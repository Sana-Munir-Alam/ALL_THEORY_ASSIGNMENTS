#!/usr/bin/env python3
"""
Optuna-tuned, leakage-safe OOF stacking pipeline for multiclass irrigation need.

What this script does:
1. Loads train.csv, test.csv, sample_submission.csv.
2. Applies deterministic feature engineering with no target leakage.
3. Builds a ColumnTransformer preprocessing pipeline.
4. Tunes LightGBM, XGBoost, and CatBoost with Optuna using the same
   StratifiedKFold splits and balanced_accuracy_score.
5. Generates strictly out-of-fold predicted probabilities for each base model.
6. Trains a swappable meta-learner, default LogisticRegression, on OOF
   probabilities only.
7. Optionally runs a strict full-stack CV evaluation where each outer fold's
   meta-learner is trained from inner OOF base predictions that never use the
   outer validation fold.
8. Retrains base models on all training data with best parameters.
9. Generates test probabilities, applies the trained meta-learner, and writes
   submission.csv.

Install dependencies:
- pip install -U pandas numpy scikit-learn optuna lightgbm xgboost catboost joblib

Expected input files:
- train.csv
- test.csv
- sample_submission.csv

Default target/id columns are configured for the provided irrigation dataset.
"""

from __future__ import annotations

import argparse
import json
import time
import warnings
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Mapping, Optional, Sequence, Tuple

warnings.filterwarnings("ignore")

import joblib
import numpy as np
import pandas as pd

from sklearn.compose import ColumnTransformer
from sklearn.impute import SimpleImputer
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import (
    accuracy_score,
    balanced_accuracy_score,
    classification_report,
    confusion_matrix,
)
from sklearn.model_selection import StratifiedKFold
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import OrdinalEncoder
from sklearn.utils.class_weight import compute_class_weight

try:
    import optuna
    from optuna.pruners import MedianPruner
    from optuna.samplers import TPESampler
    from optuna.trial import TrialState
except ImportError as exc:
    raise ImportError(
        "Optuna is required. Install dependencies with: "
        "pip install optuna lightgbm xgboost catboost"
    ) from exc

try:
    from lightgbm import LGBMClassifier
except ImportError as exc:
    raise ImportError(
        "LightGBM is required. Install dependencies with: pip install lightgbm"
    ) from exc

try:
    from xgboost import XGBClassifier
except ImportError as exc:
    raise ImportError(
        "XGBoost is required. Install dependencies with: pip install xgboost"
    ) from exc

try:
    from catboost import CatBoostClassifier
except ImportError as exc:
    raise ImportError(
        "CatBoost is required. Install dependencies with: pip install catboost"
    ) from exc


# =============================================================================
# Configuration
# =============================================================================

BASE_MODEL_NAMES = ("lightgbm", "xgboost", "catboost")


@dataclass
class Config:
    # Data and output paths.
    data_dir: Optional[str] = None
    output_dir: str = "outputs_optuna_stack"
    train_file: str = "train.csv"
    test_file: str = "test.csv"
    sample_submission_file: str = "sample_submission.csv"

    # Dataset schema.
    target_col: str = "Irrigation_Need"
    id_col: str = "id"
    class_order: Tuple[str, ...] = ("Low", "Medium", "High")
    sample_weight_col: Optional[str] = None

    # Cross-validation and reproducibility.
    seed: int = 42
    n_splits: int = 5
    n_jobs: int = -1

    # Optuna. These are target total trials per study when SQLite storage is on.
    n_trials_lightgbm: int = 30
    n_trials_xgboost: int = 30
    n_trials_catboost: int = 30
    timeout_lightgbm: Optional[int] = None
    timeout_xgboost: Optional[int] = None
    timeout_catboost: Optional[int] = None
    use_optuna_sqlite_storage: bool = True

    # Sample-weight strategy. If sample_weight_col is provided, class-balanced
    # weights are multiplied into the user-provided weights when this is True.
    use_class_balanced_sample_weight: bool = True

    # Evaluation. Strict full-stack CV is the safest estimate, but expensive:
    # for 5 folds it performs 75 additional base-model fits after tuning.
    run_strict_stacking_cv: bool = True

    # Meta-learner. Kept simple so this can be swapped in make_meta_learner().
    meta_c: float = 1.0
    meta_max_iter: int = 3000


@dataclass
class FoldCache:
    fold: int
    train_idx: np.ndarray
    valid_idx: np.ndarray
    x_train: np.ndarray
    x_valid: np.ndarray
    y_train: np.ndarray
    y_valid: np.ndarray
    sample_weight_train: np.ndarray


# =============================================================================
# Utilities
# =============================================================================


def log(message: str) -> None:
    print(message, flush=True)


def elapsed(start: float) -> str:
    return f"{time.time() - start:.1f}s"


def save_json(path: Path, payload: Mapping) -> None:
    def convert(obj):
        if isinstance(obj, (np.integer,)):
            return int(obj)
        if isinstance(obj, (np.floating,)):
            return float(obj)
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        raise TypeError(f"Object of type {type(obj)} is not JSON serializable")

    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2, default=convert)


def find_data_file(filename: str, data_dir: Optional[str]) -> Path:
    candidates: List[Path] = []
    if data_dir:
        candidates.append(Path(data_dir) / filename)
    candidates.extend(
        [
            Path("data") / filename,
            Path(filename),
            Path("/mnt/data") / filename,
        ]
    )
    for path in candidates:
        if path.exists():
            return path
    searched = "\n".join(f"  - {p}" for p in candidates)
    raise FileNotFoundError(f"Could not find {filename}. Searched:\n{searched}")


def as_float32(matrix) -> np.ndarray:
    if hasattr(matrix, "toarray"):
        matrix = matrix.toarray()
    return np.asarray(matrix, dtype=np.float32)


def validate_required_columns(
    train_df: pd.DataFrame,
    test_df: pd.DataFrame,
    required_cols: Sequence[str],
    target_col: str,
) -> None:
    missing_train = sorted(set(required_cols + [target_col]) - set(train_df.columns))
    missing_test = sorted(set(required_cols) - set(test_df.columns))
    if missing_train:
        raise ValueError(f"Missing columns in train.csv: {missing_train}")
    if missing_test:
        raise ValueError(f"Missing columns in test.csv: {missing_test}")


# =============================================================================
# Feature engineering
# =============================================================================


CATEGORICAL_COLUMNS = [
    "Soil_Type",
    "Crop_Type",
    "Crop_Growth_Stage",
    "Season",
    "Irrigation_Type",
    "Water_Source",
    "Mulching_Used",
    "Region",
]

NUMERIC_BASE_COLUMNS = [
    "Soil_pH",
    "Soil_Moisture",
    "Organic_Carbon",
    "Electrical_Conductivity",
    "Temperature_C",
    "Humidity",
    "Rainfall_mm",
    "Sunlight_Hours",
    "Wind_Speed_kmh",
    "Field_Area_hectare",
    "Previous_Irrigation_mm",
]

ENGINEERED_NUMERIC_COLUMNS = [
    "evapo_proxy",
    "wind_drying",
    "sunshine_stress",
    "heat_index",
    "moisture_deficit",
    "temp_moisture_stress",
    "moisture_sq",
    "water_balance",
    "net_water_avail",
    "rain_moisture",
    "soil_health",
    "salinity_stress",
    "rainfall_per_ha",
    "prev_irr_per_ha",
    "area_log",
    "moisture_wind",
    "temp_wind",
    "need_proxy",
]

NUMERIC_COLUMNS = NUMERIC_BASE_COLUMNS + ENGINEERED_NUMERIC_COLUMNS
FEATURE_COLUMNS = CATEGORICAL_COLUMNS + NUMERIC_COLUMNS


def safe_div(numerator, denominator, eps: float = 1e-6):
    return numerator / (denominator + eps)


def engineer_features(df: pd.DataFrame) -> pd.DataFrame:
    """Create deterministic features without using target information."""
    d = df.copy()
    eps = 1e-6

    humidity_ratio = (d["Humidity"] / 100.0).clip(lower=0.0, upper=1.5)
    drying_factor = (1.0 - humidity_ratio).clip(lower=0.0)

    # Weather and evapotranspiration proxies.
    d["evapo_proxy"] = d["Temperature_C"] * d["Wind_Speed_kmh"] * drying_factor
    d["wind_drying"] = d["Wind_Speed_kmh"] * d["Sunlight_Hours"]
    d["sunshine_stress"] = safe_div(d["Sunlight_Hours"], humidity_ratio, eps)
    d["heat_index"] = d["Temperature_C"] * (1.0 + 0.02 * d["Humidity"])

    # Moisture stress.
    d["moisture_deficit"] = 100.0 - d["Soil_Moisture"]
    d["temp_moisture_stress"] = safe_div(d["Temperature_C"], d["Soil_Moisture"], eps)
    d["moisture_sq"] = d["Soil_Moisture"] ** 2

    # Water availability.
    d["water_balance"] = d["Rainfall_mm"] - d["Previous_Irrigation_mm"]
    d["net_water_avail"] = safe_div(
        d["Rainfall_mm"] + d["Previous_Irrigation_mm"], d["Temperature_C"], eps
    )
    d["rain_moisture"] = d["Rainfall_mm"] * d["Soil_Moisture"]

    # Soil health and salinity proxies.
    d["soil_health"] = safe_div(
        d["Soil_pH"] * d["Organic_Carbon"], d["Electrical_Conductivity"], eps
    )
    d["salinity_stress"] = safe_div(d["Electrical_Conductivity"], d["Soil_pH"], eps)

    # Field-area normalized quantities.
    d["rainfall_per_ha"] = safe_div(d["Rainfall_mm"], d["Field_Area_hectare"], eps)
    d["prev_irr_per_ha"] = safe_div(
        d["Previous_Irrigation_mm"], d["Field_Area_hectare"], eps
    )
    d["area_log"] = np.log1p(d["Field_Area_hectare"].clip(lower=0.0))

    # Interactions.
    d["moisture_wind"] = d["Soil_Moisture"] * d["Wind_Speed_kmh"]
    d["temp_wind"] = d["Temperature_C"] * d["Wind_Speed_kmh"]
    d["need_proxy"] = safe_div(d["temp_moisture_stress"] * d["evapo_proxy"], d["Rainfall_mm"], eps)

    return d


# =============================================================================
# Preprocessing
# =============================================================================


def build_preprocessor(
    categorical_cols: Sequence[str], numeric_cols: Sequence[str]
) -> ColumnTransformer:
    """
    Preprocess heterogeneous columns with ColumnTransformer.

    Important leakage rule:
    During CV, this preprocessor is fitted only on each fold's training rows and
    then used to transform that fold's validation rows.
    """
    categorical_pipeline = Pipeline(
        steps=[
            ("imputer", SimpleImputer(strategy="most_frequent")),
            (
                "ordinal_encoder",
                OrdinalEncoder(handle_unknown="use_encoded_value", unknown_value=-1),
            ),
        ]
    )

    numeric_pipeline = Pipeline(steps=[("imputer", SimpleImputer(strategy="median"))])

    return ColumnTransformer(
        transformers=[
            ("cat", categorical_pipeline, list(categorical_cols)),
            ("num", numeric_pipeline, list(numeric_cols)),
        ],
        remainder="drop",
        sparse_threshold=0.0,
        verbose_feature_names_out=False,
    )


def fit_transform_for_indices(
    x_df: pd.DataFrame,
    train_idx: np.ndarray,
    predict_idx: np.ndarray,
    categorical_cols: Sequence[str],
    numeric_cols: Sequence[str],
) -> Tuple[np.ndarray, np.ndarray, ColumnTransformer]:
    preprocessor = build_preprocessor(categorical_cols, numeric_cols)
    x_train = as_float32(preprocessor.fit_transform(x_df.iloc[train_idx]))
    x_predict = as_float32(preprocessor.transform(x_df.iloc[predict_idx]))
    return x_train, x_predict, preprocessor


def build_fold_caches(
    x_df: pd.DataFrame,
    y: np.ndarray,
    sample_weight: np.ndarray,
    cv_splits: Sequence[Tuple[np.ndarray, np.ndarray]],
    categorical_cols: Sequence[str],
    numeric_cols: Sequence[str],
) -> List[FoldCache]:
    """Cache fold-specific preprocessed matrices once for Optuna and OOF."""
    caches: List[FoldCache] = []
    for fold, (train_idx, valid_idx) in enumerate(cv_splits):
        x_train, x_valid, _ = fit_transform_for_indices(
            x_df, train_idx, valid_idx, categorical_cols, numeric_cols
        )
        caches.append(
            FoldCache(
                fold=fold,
                train_idx=train_idx,
                valid_idx=valid_idx,
                x_train=x_train,
                x_valid=x_valid,
                y_train=y[train_idx],
                y_valid=y[valid_idx],
                sample_weight_train=sample_weight[train_idx],
            )
        )
    return caches


# =============================================================================
# Target and sample weights
# =============================================================================


def encode_target(
    target: pd.Series, configured_class_order: Sequence[str]
) -> Tuple[np.ndarray, List[str], Dict[str, int], Dict[int, str]]:
    labels_in_data = list(pd.unique(target.dropna()))
    configured = list(configured_class_order)

    if set(labels_in_data).issubset(set(configured)):
        class_order = configured
    else:
        # Fallback for new datasets; stable deterministic ordering.
        class_order = sorted(labels_in_data)

    class_to_int = {label: idx for idx, label in enumerate(class_order)}
    int_to_class = {idx: label for label, idx in class_to_int.items()}

    encoded = target.map(class_to_int)
    if encoded.isna().any():
        unknown = sorted(set(target.dropna()) - set(class_to_int))
        raise ValueError(f"Target contains labels not in class mapping: {unknown}")

    return encoded.astype(int).to_numpy(), class_order, class_to_int, int_to_class


def build_sample_weight(
    train_df: pd.DataFrame,
    y: np.ndarray,
    n_classes: int,
    cfg: Config,
) -> Tuple[np.ndarray, Dict[str, object]]:
    sample_weight = np.ones(len(y), dtype=np.float64)
    details: Dict[str, object] = {"base": "ones"}

    if cfg.sample_weight_col:
        if cfg.sample_weight_col not in train_df.columns:
            raise ValueError(
                f"sample_weight_col='{cfg.sample_weight_col}' not found in train.csv"
            )
        user_weight = pd.to_numeric(train_df[cfg.sample_weight_col], errors="coerce")
        if user_weight.isna().any():
            raise ValueError(f"Sample weight column {cfg.sample_weight_col} contains NaN")
        sample_weight *= user_weight.to_numpy(dtype=np.float64)
        details["base"] = cfg.sample_weight_col

    if cfg.use_class_balanced_sample_weight:
        classes = np.arange(n_classes)
        class_weights = compute_class_weight("balanced", classes=classes, y=y)
        sample_weight *= class_weights[y]
        details["class_balanced_multiplier"] = {
            int(cls): float(weight) for cls, weight in zip(classes, class_weights)
        }

    if not np.all(np.isfinite(sample_weight)) or np.any(sample_weight <= 0):
        raise ValueError("Sample weights must be positive finite values")

    details["min"] = float(np.min(sample_weight))
    details["max"] = float(np.max(sample_weight))
    details["mean"] = float(np.mean(sample_weight))
    return sample_weight, details


# =============================================================================
# Base model definitions and Optuna spaces
# =============================================================================


def suggest_base_params(model_name: str, trial: optuna.Trial) -> Dict[str, object]:
    if model_name == "lightgbm":
        return {
            "n_estimators": trial.suggest_int("n_estimators", 400, 1800, step=100),
            "learning_rate": trial.suggest_float("learning_rate", 0.01, 0.12, log=True),
            "num_leaves": trial.suggest_int("num_leaves", 16, 256, log=True),
            "max_depth": trial.suggest_categorical(
                "max_depth", [-1, 3, 4, 5, 6, 7, 8, 10, 12]
            ),
            "min_child_samples": trial.suggest_int("min_child_samples", 5, 120),
            "subsample": trial.suggest_float("subsample", 0.60, 1.00),
            "colsample_bytree": trial.suggest_float("colsample_bytree", 0.60, 1.00),
            "reg_alpha": trial.suggest_float("reg_alpha", 1e-8, 10.0, log=True),
            "reg_lambda": trial.suggest_float("reg_lambda", 1e-4, 30.0, log=True),
            "min_split_gain": trial.suggest_float("min_split_gain", 0.0, 0.30),
        }

    if model_name == "xgboost":
        return {
            "n_estimators": trial.suggest_int("n_estimators", 300, 1500, step=100),
            "learning_rate": trial.suggest_float("learning_rate", 0.01, 0.15, log=True),
            "max_depth": trial.suggest_int("max_depth", 3, 10),
            "min_child_weight": trial.suggest_float(
                "min_child_weight", 0.5, 30.0, log=True
            ),
            "subsample": trial.suggest_float("subsample", 0.60, 1.00),
            "colsample_bytree": trial.suggest_float("colsample_bytree", 0.60, 1.00),
            "gamma": trial.suggest_float("gamma", 0.0, 5.0),
            "reg_alpha": trial.suggest_float("reg_alpha", 1e-8, 10.0, log=True),
            "reg_lambda": trial.suggest_float("reg_lambda", 1e-4, 30.0, log=True),
            "max_bin": trial.suggest_categorical("max_bin", [128, 256, 512]),
        }

    if model_name == "catboost":
        return {
            "iterations": trial.suggest_int("iterations", 300, 1500, step=100),
            "learning_rate": trial.suggest_float("learning_rate", 0.01, 0.15, log=True),
            "depth": trial.suggest_int("depth", 4, 10),
            "l2_leaf_reg": trial.suggest_float("l2_leaf_reg", 1e-2, 30.0, log=True),
            "random_strength": trial.suggest_float("random_strength", 0.0, 10.0),
            "bagging_temperature": trial.suggest_float("bagging_temperature", 0.0, 10.0),
            "border_count": trial.suggest_categorical("border_count", [32, 64, 128, 254]),
        }

    raise ValueError(f"Unknown model_name: {model_name}")


def complete_base_params(
    model_name: str,
    tuned_params: Mapping[str, object],
    seed: int,
    n_classes: int,
    n_jobs: int,
) -> Dict[str, object]:
    params = dict(tuned_params)

    if model_name == "lightgbm":
        params.update(
            {
                "objective": "multiclass",
                "num_class": n_classes,
                "subsample_freq": 1,
                "random_state": seed,
                "n_jobs": n_jobs,
                "verbosity": -1,
            }
        )
        return params

    if model_name == "xgboost":
        params.update(
            {
                "objective": "multi:softprob",
                "num_class": n_classes,
                "eval_metric": "mlogloss",
                "tree_method": "hist",
                "random_state": seed,
                "n_jobs": n_jobs,
                "verbosity": 0,
            }
        )
        return params

    if model_name == "catboost":
        params.update(
            {
                "loss_function": "MultiClass",
                "eval_metric": "MultiClass",
                "bootstrap_type": "Bayesian",
                "random_seed": seed,
                "thread_count": n_jobs,
                "verbose": False,
                "allow_writing_files": False,
            }
        )
        return params

    raise ValueError(f"Unknown model_name: {model_name}")


def make_base_model(
    model_name: str,
    tuned_params: Mapping[str, object],
    seed: int,
    n_classes: int,
    n_jobs: int,
):
    params = complete_base_params(model_name, tuned_params, seed, n_classes, n_jobs)
    if model_name == "lightgbm":
        return LGBMClassifier(**params)
    if model_name == "xgboost":
        return XGBClassifier(**params)
    if model_name == "catboost":
        return CatBoostClassifier(**params)
    raise ValueError(f"Unknown model_name: {model_name}")


def fit_estimator(model, x: np.ndarray, y: np.ndarray, sample_weight: np.ndarray):
    model.fit(x, y, sample_weight=sample_weight)
    return model


def predict_proba_aligned(model, x: np.ndarray, n_classes: int) -> np.ndarray:
    """Return proba columns in class order 0..n_classes-1."""
    raw = np.asarray(model.predict_proba(x), dtype=np.float64)
    classes = getattr(model, "classes_", np.arange(raw.shape[1]))
    aligned = np.zeros((raw.shape[0], n_classes), dtype=np.float64)
    for source_col, cls in enumerate(classes):
        cls_int = int(cls)
        if 0 <= cls_int < n_classes:
            aligned[:, cls_int] = raw[:, source_col]
    row_sum = aligned.sum(axis=1, keepdims=True)
    row_sum[row_sum == 0.0] = 1.0
    return aligned / row_sum


# =============================================================================
# Optuna tuning
# =============================================================================


def score_model_cv(
    model_name: str,
    tuned_params: Mapping[str, object],
    fold_caches: Sequence[FoldCache],
    n_classes: int,
    n_jobs: int,
    seed: int,
    trial: Optional[optuna.Trial] = None,
) -> Tuple[float, float, List[float]]:
    scores: List[float] = []
    for fold_cache in fold_caches:
        model_seed = seed + 1009 * fold_cache.fold
        model = make_base_model(
            model_name, tuned_params, model_seed, n_classes=n_classes, n_jobs=n_jobs
        )
        fit_estimator(
            model,
            fold_cache.x_train,
            fold_cache.y_train,
            fold_cache.sample_weight_train,
        )
        proba = predict_proba_aligned(model, fold_cache.x_valid, n_classes)
        pred = np.argmax(proba, axis=1)
        score = balanced_accuracy_score(fold_cache.y_valid, pred)
        scores.append(float(score))

        if trial is not None:
            trial.report(float(np.mean(scores)), step=fold_cache.fold)
            if trial.should_prune():
                raise optuna.TrialPruned()

    return float(np.mean(scores)), float(np.std(scores)), scores


def tune_base_model(
    model_name: str,
    fold_caches: Sequence[FoldCache],
    cfg: Config,
    n_classes: int,
    output_dir: Path,
) -> Dict[str, object]:
    n_trials_map = {
        "lightgbm": cfg.n_trials_lightgbm,
        "xgboost": cfg.n_trials_xgboost,
        "catboost": cfg.n_trials_catboost,
    }
    timeout_map = {
        "lightgbm": cfg.timeout_lightgbm,
        "xgboost": cfg.timeout_xgboost,
        "catboost": cfg.timeout_catboost,
    }

    n_trials = n_trials_map[model_name]
    timeout = timeout_map[model_name]

    storage_url = None
    if cfg.use_optuna_sqlite_storage:
        storage_path = output_dir / "optuna_studies.db"
        storage_url = f"sqlite:///{storage_path.as_posix()}"

    study_name = f"{model_name}_balanced_accuracy_seed{cfg.seed}_folds{cfg.n_splits}"
    study = optuna.create_study(
        direction="maximize",
        study_name=study_name,
        storage=storage_url,
        load_if_exists=bool(storage_url),
        sampler=TPESampler(seed=cfg.seed),
        pruner=MedianPruner(n_startup_trials=max(5, n_trials // 5), n_warmup_steps=1),
    )

    finished_states = {TrialState.COMPLETE, TrialState.PRUNED}
    attempted = len([t for t in study.trials if t.state in finished_states])
    remaining = max(0, n_trials - attempted)

    log(
        f"\n[Optuna] {model_name}: target_trials={n_trials}, "
        f"already_attempted={attempted}, remaining={remaining}"
    )

    def objective(trial: optuna.Trial) -> float:
        tuned_params = suggest_base_params(model_name, trial)
        mean_score, _, _ = score_model_cv(
            model_name=model_name,
            tuned_params=tuned_params,
            fold_caches=fold_caches,
            n_classes=n_classes,
            n_jobs=cfg.n_jobs,
            seed=cfg.seed,
            trial=trial,
        )
        return mean_score

    if remaining > 0:
        start = time.time()
        study.optimize(objective, n_trials=remaining, timeout=timeout, n_jobs=1, gc_after_trial=True)
        log(f"[Optuna] {model_name}: optimization finished in {elapsed(start)}")
    else:
        log(f"[Optuna] {model_name}: using existing completed study")

    if not study.best_trial:
        raise RuntimeError(f"No completed Optuna trial found for {model_name}")

    best_params = dict(study.best_params)
    best_value = float(study.best_value)
    log(f"[Optuna] {model_name}: best BAL_ACC={best_value:.6f}")
    log(f"[Optuna] {model_name}: best params={best_params}")

    # Persist a human-readable trial table for diagnostics.
    trials_df = study.trials_dataframe(attrs=("number", "value", "params", "state"))
    trials_df.to_csv(output_dir / f"optuna_trials_{model_name}.csv", index=False)

    return {"best_params": best_params, "best_value": best_value}


# =============================================================================
# OOF base probabilities and stacking
# =============================================================================


def generate_base_oof_predictions(
    model_name: str,
    tuned_params: Mapping[str, object],
    fold_caches: Sequence[FoldCache],
    n_samples: int,
    n_classes: int,
    cfg: Config,
) -> Tuple[np.ndarray, Dict[str, object]]:
    oof_proba = np.zeros((n_samples, n_classes), dtype=np.float64)
    fold_scores: List[float] = []
    fold_accs: List[float] = []

    log(f"\n[OOF] Generating OOF probabilities for {model_name}")
    start = time.time()
    for fold_cache in fold_caches:
        model_seed = cfg.seed + 1009 * fold_cache.fold
        model = make_base_model(
            model_name, tuned_params, model_seed, n_classes=n_classes, n_jobs=cfg.n_jobs
        )
        fit_estimator(
            model,
            fold_cache.x_train,
            fold_cache.y_train,
            fold_cache.sample_weight_train,
        )
        proba = predict_proba_aligned(model, fold_cache.x_valid, n_classes)
        oof_proba[fold_cache.valid_idx] = proba
        pred = np.argmax(proba, axis=1)
        bal = balanced_accuracy_score(fold_cache.y_valid, pred)
        acc = accuracy_score(fold_cache.y_valid, pred)
        fold_scores.append(float(bal))
        fold_accs.append(float(acc))
        log(f"  fold {fold_cache.fold + 1}: BAL_ACC={bal:.6f}, ACC={acc:.6f}")

    oof_pred = np.argmax(oof_proba, axis=1)
    y_all = np.zeros(n_samples, dtype=int)
    for cache in fold_caches:
        y_all[cache.valid_idx] = cache.y_valid

    metrics = {
        "fold_balanced_accuracy": fold_scores,
        "fold_accuracy": fold_accs,
        "cv_balanced_accuracy_mean": float(np.mean(fold_scores)),
        "cv_balanced_accuracy_std": float(np.std(fold_scores)),
        "oof_balanced_accuracy": float(balanced_accuracy_score(y_all, oof_pred)),
        "oof_accuracy": float(accuracy_score(y_all, oof_pred)),
        "elapsed_seconds": float(time.time() - start),
    }
    log(
        f"[OOF] {model_name}: BAL_ACC={metrics['oof_balanced_accuracy']:.6f} "
        f"+/- {metrics['cv_balanced_accuracy_std']:.6f} ({elapsed(start)})"
    )
    return oof_proba, metrics


def make_meta_feature_names(
    base_model_names: Sequence[str], class_order: Sequence[str]
) -> List[str]:
    return [
        f"{model_name}_proba_{class_label}"
        for model_name in base_model_names
        for class_label in class_order
    ]


def build_meta_matrix(
    base_oof_probas: Mapping[str, np.ndarray], base_model_names: Sequence[str]
) -> np.ndarray:
    return np.hstack([base_oof_probas[name] for name in base_model_names])


def make_meta_learner(cfg: Config):
    """
    Default meta-learner. Swap this function to use a different second-level model.

    The meta-learner only sees OOF predicted probabilities from base models.
    """
    return LogisticRegression(
        C=cfg.meta_c,
        solver="lbfgs",
        max_iter=cfg.meta_max_iter,
        random_state=cfg.seed,
        n_jobs=cfg.n_jobs,
    )


def evaluate_meta_cv_on_oof_features(
    meta_x: np.ndarray,
    y: np.ndarray,
    sample_weight: np.ndarray,
    cv_splits: Sequence[Tuple[np.ndarray, np.ndarray]],
    cfg: Config,
    n_classes: int,
) -> Tuple[np.ndarray, np.ndarray, Dict[str, object]]:
    """
    Fast meta-level CV on the already-created OOF features.

    This evaluates only the meta-learner's behavior on OOF features. For the
    strict full-stack CV evaluation, use evaluate_strict_full_stack_cv().
    """
    oof_pred = np.zeros(len(y), dtype=int)
    oof_proba = np.zeros((len(y), n_classes), dtype=np.float64)
    fold_scores: List[float] = []
    fold_accs: List[float] = []

    log("\n[Meta CV] Evaluating LogisticRegression on OOF meta-features")
    for fold, (train_idx, valid_idx) in enumerate(cv_splits):
        meta_model = make_meta_learner(cfg)
        meta_model.fit(meta_x[train_idx], y[train_idx], sample_weight=sample_weight[train_idx])
        proba = predict_proba_aligned(meta_model, meta_x[valid_idx], n_classes)
        pred = np.argmax(proba, axis=1)
        oof_pred[valid_idx] = pred
        oof_proba[valid_idx] = proba
        bal = balanced_accuracy_score(y[valid_idx], pred)
        acc = accuracy_score(y[valid_idx], pred)
        fold_scores.append(float(bal))
        fold_accs.append(float(acc))
        log(f"  fold {fold + 1}: BAL_ACC={bal:.6f}, ACC={acc:.6f}")

    metrics = {
        "fold_balanced_accuracy": fold_scores,
        "fold_accuracy": fold_accs,
        "cv_balanced_accuracy_mean": float(np.mean(fold_scores)),
        "cv_balanced_accuracy_std": float(np.std(fold_scores)),
        "oof_balanced_accuracy": float(balanced_accuracy_score(y, oof_pred)),
        "oof_accuracy": float(accuracy_score(y, oof_pred)),
    }
    log(
        f"[Meta CV] OOF BAL_ACC={metrics['oof_balanced_accuracy']:.6f}, "
        f"ACC={metrics['oof_accuracy']:.6f}"
    )
    return oof_pred, oof_proba, metrics


def fit_all_base_models_predict_meta_features(
    x_df: pd.DataFrame,
    y: np.ndarray,
    sample_weight: np.ndarray,
    train_idx: np.ndarray,
    predict_idx: np.ndarray,
    tuned_params_by_model: Mapping[str, Mapping[str, object]],
    categorical_cols: Sequence[str],
    numeric_cols: Sequence[str],
    base_model_names: Sequence[str],
    n_classes: int,
    cfg: Config,
    seed_offset: int,
) -> np.ndarray:
    """Fit all base models on train_idx and predict probabilities for predict_idx."""
    x_train, x_predict, _ = fit_transform_for_indices(
        x_df, train_idx, predict_idx, categorical_cols, numeric_cols
    )
    blocks: List[np.ndarray] = []
    for model_position, model_name in enumerate(base_model_names):
        model_seed = cfg.seed + seed_offset + 1009 * model_position
        model = make_base_model(
            model_name,
            tuned_params_by_model[model_name],
            model_seed,
            n_classes=n_classes,
            n_jobs=cfg.n_jobs,
        )
        fit_estimator(model, x_train, y[train_idx], sample_weight[train_idx])
        blocks.append(predict_proba_aligned(model, x_predict, n_classes))
    return np.hstack(blocks)


def evaluate_strict_full_stack_cv(
    x_df: pd.DataFrame,
    y: np.ndarray,
    sample_weight: np.ndarray,
    cv_splits: Sequence[Tuple[np.ndarray, np.ndarray]],
    tuned_params_by_model: Mapping[str, Mapping[str, object]],
    categorical_cols: Sequence[str],
    numeric_cols: Sequence[str],
    base_model_names: Sequence[str],
    n_classes: int,
    cfg: Config,
) -> Tuple[np.ndarray, np.ndarray, Dict[str, object]]:
    """
    Leakage-safe full-stack CV evaluation.

    For each outer fold k:
    - Meta validation features are base probabilities from base models trained on
      all non-k rows.
    - Meta training features are inner OOF base probabilities generated only
      from non-k rows. The outer validation fold is never used to create meta
      training features.
    - The same global StratifiedKFold fold boundaries are reused for base and
      meta levels.

    This is intentionally more expensive than fitting a meta-model directly on
    global OOF features, but it gives a stricter full-pipeline CV estimate.
    """
    if len(cv_splits) < 3:
        raise ValueError("Strict full-stack CV requires at least 3 folds")

    log("\n[Strict Stack CV] Starting leakage-safe full-stack CV evaluation")
    start = time.time()
    fold_valid_indices = [valid_idx for _, valid_idx in cv_splits]
    oof_pred = np.zeros(len(y), dtype=int)
    oof_proba = np.zeros((len(y), n_classes), dtype=np.float64)
    fold_scores: List[float] = []
    fold_accs: List[float] = []

    for outer_fold, (outer_train_idx, outer_valid_idx) in enumerate(cv_splits):
        fold_start = time.time()
        log(f"  outer fold {outer_fold + 1}/{len(cv_splits)}")

        # Build inner OOF features for rows in outer_train_idx.
        meta_train = np.zeros(
            (len(outer_train_idx), len(base_model_names) * n_classes), dtype=np.float64
        )
        outer_position = {int(idx): pos for pos, idx in enumerate(outer_train_idx)}

        for inner_fold, inner_valid_idx in enumerate(fold_valid_indices):
            if inner_fold == outer_fold:
                continue
            inner_valid_idx = np.asarray(inner_valid_idx)
            inner_train_idx = np.setdiff1d(outer_train_idx, inner_valid_idx, assume_unique=True)
            rows = np.array([outer_position[int(idx)] for idx in inner_valid_idx], dtype=int)

            meta_block = fit_all_base_models_predict_meta_features(
                x_df=x_df,
                y=y,
                sample_weight=sample_weight,
                train_idx=inner_train_idx,
                predict_idx=inner_valid_idx,
                tuned_params_by_model=tuned_params_by_model,
                categorical_cols=categorical_cols,
                numeric_cols=numeric_cols,
                base_model_names=base_model_names,
                n_classes=n_classes,
                cfg=cfg,
                seed_offset=10_000 + outer_fold * 100 + inner_fold,
            )
            meta_train[rows] = meta_block

        # Build validation meta-features from base models trained only on outer train.
        meta_valid = fit_all_base_models_predict_meta_features(
            x_df=x_df,
            y=y,
            sample_weight=sample_weight,
            train_idx=outer_train_idx,
            predict_idx=outer_valid_idx,
            tuned_params_by_model=tuned_params_by_model,
            categorical_cols=categorical_cols,
            numeric_cols=numeric_cols,
            base_model_names=base_model_names,
            n_classes=n_classes,
            cfg=cfg,
            seed_offset=20_000 + outer_fold,
        )

        meta_model = make_meta_learner(cfg)
        meta_model.fit(meta_train, y[outer_train_idx], sample_weight=sample_weight[outer_train_idx])
        proba = predict_proba_aligned(meta_model, meta_valid, n_classes)
        pred = np.argmax(proba, axis=1)
        oof_pred[outer_valid_idx] = pred
        oof_proba[outer_valid_idx] = proba

        bal = balanced_accuracy_score(y[outer_valid_idx], pred)
        acc = accuracy_score(y[outer_valid_idx], pred)
        fold_scores.append(float(bal))
        fold_accs.append(float(acc))
        log(
            f"    BAL_ACC={bal:.6f}, ACC={acc:.6f}, elapsed={elapsed(fold_start)}"
        )

    metrics = {
        "fold_balanced_accuracy": fold_scores,
        "fold_accuracy": fold_accs,
        "cv_balanced_accuracy_mean": float(np.mean(fold_scores)),
        "cv_balanced_accuracy_std": float(np.std(fold_scores)),
        "oof_balanced_accuracy": float(balanced_accuracy_score(y, oof_pred)),
        "oof_accuracy": float(accuracy_score(y, oof_pred)),
        "elapsed_seconds": float(time.time() - start),
    }
    log(
        f"[Strict Stack CV] OOF BAL_ACC={metrics['oof_balanced_accuracy']:.6f}, "
        f"ACC={metrics['oof_accuracy']:.6f}, elapsed={elapsed(start)}"
    )
    return oof_pred, oof_proba, metrics


# =============================================================================
# Final training and submission
# =============================================================================


def train_final_base_models_and_predict_test(
    x_train_df: pd.DataFrame,
    x_test_df: pd.DataFrame,
    y: np.ndarray,
    sample_weight: np.ndarray,
    tuned_params_by_model: Mapping[str, Mapping[str, object]],
    categorical_cols: Sequence[str],
    numeric_cols: Sequence[str],
    base_model_names: Sequence[str],
    n_classes: int,
    cfg: Config,
) -> Tuple[ColumnTransformer, Dict[str, object], Dict[str, np.ndarray], np.ndarray, np.ndarray]:
    log("\n[Final] Fitting preprocessor on full training data")
    final_preprocessor = build_preprocessor(categorical_cols, numeric_cols)
    x_full = as_float32(final_preprocessor.fit_transform(x_train_df))
    x_test = as_float32(final_preprocessor.transform(x_test_df))

    final_base_models: Dict[str, object] = {}
    test_probas_by_model: Dict[str, np.ndarray] = {}

    for pos, model_name in enumerate(base_model_names):
        log(f"[Final] Training full-data {model_name}")
        start = time.time()
        model = make_base_model(
            model_name,
            tuned_params_by_model[model_name],
            seed=cfg.seed + 50_000 + pos,
            n_classes=n_classes,
            n_jobs=cfg.n_jobs,
        )
        fit_estimator(model, x_full, y, sample_weight)
        final_base_models[model_name] = model
        test_probas_by_model[model_name] = predict_proba_aligned(model, x_test, n_classes)
        log(f"[Final] {model_name} done in {elapsed(start)}")

    meta_x_test = build_meta_matrix(test_probas_by_model, base_model_names)
    return final_preprocessor, final_base_models, test_probas_by_model, meta_x_test, x_full


def create_submission(
    sample_submission: pd.DataFrame,
    test_df: pd.DataFrame,
    encoded_predictions: np.ndarray,
    int_to_class: Mapping[int, str],
    cfg: Config,
) -> pd.DataFrame:
    predicted_labels = pd.Series(encoded_predictions).map(int_to_class).to_numpy()
    submission = sample_submission.copy()

    if cfg.id_col in submission.columns:
        if cfg.id_col in test_df.columns:
            submission[cfg.id_col] = test_df[cfg.id_col].to_numpy()
    else:
        log(f"[WARN] id column {cfg.id_col!r} not found in sample submission")

    if cfg.target_col in submission.columns:
        submission[cfg.target_col] = predicted_labels
    else:
        # Common competition fallback: target is the last column in sample submission.
        target_like_col = submission.columns[-1]
        log(
            f"[WARN] target column {cfg.target_col!r} not found in sample submission; "
            f"writing predictions to {target_like_col!r}"
        )
        submission[target_like_col] = predicted_labels

    return submission


def save_reports(
    output_dir: Path,
    y: np.ndarray,
    class_order: Sequence[str],
    base_metrics: Mapping[str, Mapping[str, object]],
    meta_fast_metrics: Mapping[str, object],
    strict_metrics: Optional[Mapping[str, object]],
    meta_oof_pred: np.ndarray,
    strict_oof_pred: Optional[np.ndarray],
) -> None:
    rows = []
    for model_name, metrics in base_metrics.items():
        rows.append(
            {
                "model": model_name,
                "cv_balanced_accuracy_mean": metrics["cv_balanced_accuracy_mean"],
                "cv_balanced_accuracy_std": metrics["cv_balanced_accuracy_std"],
                "oof_balanced_accuracy": metrics["oof_balanced_accuracy"],
                "oof_accuracy": metrics["oof_accuracy"],
            }
        )

    rows.append(
        {
            "model": "stacking_meta_cv_fast",
            "cv_balanced_accuracy_mean": meta_fast_metrics["cv_balanced_accuracy_mean"],
            "cv_balanced_accuracy_std": meta_fast_metrics["cv_balanced_accuracy_std"],
            "oof_balanced_accuracy": meta_fast_metrics["oof_balanced_accuracy"],
            "oof_accuracy": meta_fast_metrics["oof_accuracy"],
        }
    )
    if strict_metrics is not None:
        rows.append(
            {
                "model": "stacking_full_cv_strict",
                "cv_balanced_accuracy_mean": strict_metrics["cv_balanced_accuracy_mean"],
                "cv_balanced_accuracy_std": strict_metrics["cv_balanced_accuracy_std"],
                "oof_balanced_accuracy": strict_metrics["oof_balanced_accuracy"],
                "oof_accuracy": strict_metrics["oof_accuracy"],
            }
        )

    pd.DataFrame(rows).to_csv(output_dir / "cv_results.csv", index=False)

    report_pred = strict_oof_pred if strict_oof_pred is not None else meta_oof_pred
    report_name = "strict full-stack CV" if strict_oof_pred is not None else "fast meta CV"
    report = classification_report(
        y,
        report_pred,
        labels=np.arange(len(class_order)),
        target_names=list(class_order),
        zero_division=0,
    )
    cm = confusion_matrix(y, report_pred, labels=np.arange(len(class_order)))
    with (output_dir / "stacking_classification_report.txt").open("w", encoding="utf-8") as f:
        f.write(f"Stacking report source: {report_name}\n\n")
        f.write(report)
        f.write("\nConfusion matrix, rows=true, cols=pred:\n")
        f.write(np.array2string(cm))


def save_feature_importance(
    output_dir: Path,
    final_preprocessor: ColumnTransformer,
    final_base_models: Mapping[str, object],
) -> None:
    try:
        feature_names = list(final_preprocessor.get_feature_names_out())
    except Exception:
        feature_names = FEATURE_COLUMNS

    importance_frames = []
    for model_name, model in final_base_models.items():
        if hasattr(model, "feature_importances_"):
            imp = np.asarray(model.feature_importances_, dtype=float)
            if len(imp) == len(feature_names):
                importance_frames.append(
                    pd.DataFrame(
                        {
                            "model": model_name,
                            "feature": feature_names,
                            "importance": imp,
                        }
                    )
                )
    if importance_frames:
        pd.concat(importance_frames, ignore_index=True).to_csv(
            output_dir / "feature_importances.csv", index=False
        )


# =============================================================================
# Main
# =============================================================================


def parse_args() -> Config:
    parser = argparse.ArgumentParser(description="Optuna OOF stacking pipeline")
    parser.add_argument("--data-dir", default=None)
    parser.add_argument("--output-dir", default="outputs_optuna_stack")
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--n-splits", type=int, default=5)
    parser.add_argument("--n-jobs", type=int, default=-1)
    parser.add_argument("--target-col", default="Irrigation_Need")
    parser.add_argument("--id-col", default="id")
    parser.add_argument("--sample-weight-col", default=None)
    parser.add_argument("--n-trials-lightgbm", type=int, default=30)
    parser.add_argument("--n-trials-xgboost", type=int, default=30)
    parser.add_argument("--n-trials-catboost", type=int, default=30)
    parser.add_argument("--timeout-lightgbm", type=int, default=None)
    parser.add_argument("--timeout-xgboost", type=int, default=None)
    parser.add_argument("--timeout-catboost", type=int, default=None)
    parser.add_argument("--no-optuna-sqlite-storage", action="store_true")
    parser.add_argument("--no-class-balanced-sample-weight", action="store_true")
    parser.add_argument("--no-strict-stacking-cv", action="store_true")
    parser.add_argument("--meta-c", type=float, default=1.0)
    args = parser.parse_args()

    return Config(
        data_dir=args.data_dir,
        output_dir=args.output_dir,
        target_col=args.target_col,
        id_col=args.id_col,
        sample_weight_col=args.sample_weight_col,
        seed=args.seed,
        n_splits=args.n_splits,
        n_jobs=args.n_jobs,
        n_trials_lightgbm=args.n_trials_lightgbm,
        n_trials_xgboost=args.n_trials_xgboost,
        n_trials_catboost=args.n_trials_catboost,
        timeout_lightgbm=args.timeout_lightgbm,
        timeout_xgboost=args.timeout_xgboost,
        timeout_catboost=args.timeout_catboost,
        use_optuna_sqlite_storage=not args.no_optuna_sqlite_storage,
        use_class_balanced_sample_weight=not args.no_class_balanced_sample_weight,
        run_strict_stacking_cv=not args.no_strict_stacking_cv,
        meta_c=args.meta_c,
    )


def main() -> None:
    cfg = parse_args()
    np.random.seed(cfg.seed)
    output_dir = Path(cfg.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    log("=" * 80)
    log("OPTUNA-TUNED OOF STACKING PIPELINE")
    log("Base models: LightGBM, XGBoost, CatBoost")
    log("Meta learner: LogisticRegression on OOF predicted probabilities")
    log("Primary metric: balanced_accuracy_score")
    log("=" * 80)
    save_json(output_dir / "config.json", asdict(cfg))

    # -------------------------------------------------------------------------
    # 1. Load data
    # -------------------------------------------------------------------------
    log("\n[1] Loading data")
    train_path = find_data_file(cfg.train_file, cfg.data_dir)
    test_path = find_data_file(cfg.test_file, cfg.data_dir)
    sample_path = find_data_file(cfg.sample_submission_file, cfg.data_dir)

    train_raw = pd.read_csv(train_path)
    test_raw = pd.read_csv(test_path)
    sample_submission = pd.read_csv(sample_path)

    log(f"  train: {train_raw.shape} from {train_path}")
    log(f"  test : {test_raw.shape} from {test_path}")
    log(f"  sample_submission: {sample_submission.shape} from {sample_path}")

    validate_required_columns(
        train_raw,
        test_raw,
        required_cols=CATEGORICAL_COLUMNS + NUMERIC_BASE_COLUMNS,
        target_col=cfg.target_col,
    )

    # -------------------------------------------------------------------------
    # 2. Feature engineering
    # -------------------------------------------------------------------------
    log("\n[2] Feature engineering")
    train_fe = engineer_features(train_raw)
    test_fe = engineer_features(test_raw)
    validate_required_columns(train_fe, test_fe, FEATURE_COLUMNS, cfg.target_col)

    x_train_df = train_fe[FEATURE_COLUMNS].copy()
    x_test_df = test_fe[FEATURE_COLUMNS].copy()

    # Ensure categorical columns are treated consistently by imputers/encoders.
    for col in CATEGORICAL_COLUMNS:
        x_train_df[col] = x_train_df[col].astype("object")
        x_test_df[col] = x_test_df[col].astype("object")

    log(f"  categorical features: {len(CATEGORICAL_COLUMNS)}")
    log(f"  numeric features    : {len(NUMERIC_COLUMNS)}")
    log(f"  total features      : {len(FEATURE_COLUMNS)}")

    # -------------------------------------------------------------------------
    # 3. Target encoding and sample weights
    # -------------------------------------------------------------------------
    log("\n[3] Target encoding and sample weights")
    y, class_order, class_to_int, int_to_class = encode_target(
        train_raw[cfg.target_col], cfg.class_order
    )
    n_classes = len(class_order)
    if n_classes < 2:
        raise ValueError("Need at least two classes for classification")

    min_class_count = int(np.min(np.bincount(y, minlength=n_classes)))
    if cfg.n_splits > min_class_count:
        raise ValueError(
            f"n_splits={cfg.n_splits} is greater than the smallest class count "
            f"({min_class_count}). Reduce n_splits."
        )

    sample_weight, weight_details = build_sample_weight(train_raw, y, n_classes, cfg)
    log(f"  class map: {class_to_int}")
    log(f"  class counts: {dict(zip(class_order, np.bincount(y, minlength=n_classes)))}")
    log(f"  sample weight details: {weight_details}")

    # -------------------------------------------------------------------------
    # 4. Shared CV splits
    # -------------------------------------------------------------------------
    log("\n[4] Creating shared StratifiedKFold splits")
    cv = StratifiedKFold(n_splits=cfg.n_splits, shuffle=True, random_state=cfg.seed)
    cv_splits = list(cv.split(x_train_df, y))
    fold_assignment = np.full(len(y), fill_value=-1, dtype=int)
    for fold, (_, valid_idx) in enumerate(cv_splits):
        fold_assignment[valid_idx] = fold
    pd.DataFrame({"row_index": np.arange(len(y)), "fold": fold_assignment}).to_csv(
        output_dir / "cv_folds.csv", index=False
    )
    log(f"  saved fold assignments to {output_dir / 'cv_folds.csv'}")

    # -------------------------------------------------------------------------
    # 5. Precompute fold matrices for leak-free base-model CV and Optuna
    # -------------------------------------------------------------------------
    log("\n[5] Precomputing fold-specific preprocessing caches")
    start = time.time()
    fold_caches = build_fold_caches(
        x_df=x_train_df,
        y=y,
        sample_weight=sample_weight,
        cv_splits=cv_splits,
        categorical_cols=CATEGORICAL_COLUMNS,
        numeric_cols=NUMERIC_COLUMNS,
    )
    log(f"  built {len(fold_caches)} fold caches in {elapsed(start)}")

    # -------------------------------------------------------------------------
    # 6. Tune base models with Optuna
    # -------------------------------------------------------------------------
    log("\n[6] Optuna tuning")
    optuna_results: Dict[str, Dict[str, object]] = {}
    tuned_params_by_model: Dict[str, Dict[str, object]] = {}
    for model_name in BASE_MODEL_NAMES:
        result = tune_base_model(model_name, fold_caches, cfg, n_classes, output_dir)
        optuna_results[model_name] = result
        tuned_params_by_model[model_name] = dict(result["best_params"])

    save_json(output_dir / "best_params.json", tuned_params_by_model)
    save_json(output_dir / "optuna_best_results.json", optuna_results)

    # -------------------------------------------------------------------------
    # 7. Generate base-model OOF predicted probabilities using best params
    # -------------------------------------------------------------------------
    log("\n[7] Base-model OOF probabilities")
    base_oof_probas: Dict[str, np.ndarray] = {}
    base_metrics: Dict[str, Dict[str, object]] = {}
    for model_name in BASE_MODEL_NAMES:
        oof_proba, metrics = generate_base_oof_predictions(
            model_name=model_name,
            tuned_params=tuned_params_by_model[model_name],
            fold_caches=fold_caches,
            n_samples=len(y),
            n_classes=n_classes,
            cfg=cfg,
        )
        base_oof_probas[model_name] = oof_proba
        base_metrics[model_name] = metrics

    np.savez_compressed(
        output_dir / "base_oof_probabilities.npz",
        **{name: proba for name, proba in base_oof_probas.items()},
    )
    save_json(output_dir / "base_oof_metrics.json", base_metrics)

    # -------------------------------------------------------------------------
    # 8. Build OOF meta-features and train/evaluate meta-learner
    # -------------------------------------------------------------------------
    log("\n[8] Stacking meta-features from OOF probabilities")
    meta_x_train = build_meta_matrix(base_oof_probas, BASE_MODEL_NAMES)
    meta_feature_names = make_meta_feature_names(BASE_MODEL_NAMES, class_order)
    pd.DataFrame(meta_x_train, columns=meta_feature_names).to_csv(
        output_dir / "meta_train_oof_features.csv", index=False
    )
    save_json(output_dir / "meta_feature_names.json", {"features": meta_feature_names})
    log(f"  meta feature matrix shape: {meta_x_train.shape}")

    meta_oof_pred, meta_oof_proba, meta_fast_metrics = evaluate_meta_cv_on_oof_features(
        meta_x=meta_x_train,
        y=y,
        sample_weight=sample_weight,
        cv_splits=cv_splits,
        cfg=cfg,
        n_classes=n_classes,
    )
    np.savez_compressed(
        output_dir / "meta_fast_cv_oof_predictions.npz",
        pred=meta_oof_pred,
        proba=meta_oof_proba,
    )

    strict_oof_pred = None
    strict_oof_proba = None
    strict_metrics = None
    if cfg.run_strict_stacking_cv:
        strict_oof_pred, strict_oof_proba, strict_metrics = evaluate_strict_full_stack_cv(
            x_df=x_train_df,
            y=y,
            sample_weight=sample_weight,
            cv_splits=cv_splits,
            tuned_params_by_model=tuned_params_by_model,
            categorical_cols=CATEGORICAL_COLUMNS,
            numeric_cols=NUMERIC_COLUMNS,
            base_model_names=BASE_MODEL_NAMES,
            n_classes=n_classes,
            cfg=cfg,
        )
        np.savez_compressed(
            output_dir / "strict_stack_cv_oof_predictions.npz",
            pred=strict_oof_pred,
            proba=strict_oof_proba,
        )

    # Fit final meta-learner on all OOF meta-features.
    # Each row's base probabilities were generated by base models that did not
    # see that row during training.
    log("\n[9] Fitting final meta-learner on full OOF meta-feature matrix")
    final_meta_model = make_meta_learner(cfg)
    final_meta_model.fit(meta_x_train, y, sample_weight=sample_weight)

    # -------------------------------------------------------------------------
    # 9. Final full-data base training and test prediction
    # -------------------------------------------------------------------------
    log("\n[10] Final full-data base training and test probabilities")
    (
        final_preprocessor,
        final_base_models,
        test_probas_by_model,
        meta_x_test,
        x_full_transformed,
    ) = train_final_base_models_and_predict_test(
        x_train_df=x_train_df,
        x_test_df=x_test_df,
        y=y,
        sample_weight=sample_weight,
        tuned_params_by_model=tuned_params_by_model,
        categorical_cols=CATEGORICAL_COLUMNS,
        numeric_cols=NUMERIC_COLUMNS,
        base_model_names=BASE_MODEL_NAMES,
        n_classes=n_classes,
        cfg=cfg,
    )

    stack_test_proba = predict_proba_aligned(final_meta_model, meta_x_test, n_classes)
    test_pred_encoded = np.argmax(stack_test_proba, axis=1)

    np.savez_compressed(
        output_dir / "test_probabilities.npz",
        **{name: proba for name, proba in test_probas_by_model.items()},
        stacking=stack_test_proba,
    )

    # -------------------------------------------------------------------------
    # 10. Submission and artifacts
    # -------------------------------------------------------------------------
    log("\n[11] Writing submission and artifacts")
    submission = create_submission(
        sample_submission=sample_submission,
        test_df=test_raw,
        encoded_predictions=test_pred_encoded,
        int_to_class=int_to_class,
        cfg=cfg,
    )
    submission_path = output_dir / "submission.csv"
    submission.to_csv(submission_path, index=False)
    log(f"  saved submission: {submission_path}")
    log("  prediction distribution:")
    log(submission.iloc[:, -1].value_counts().to_string())

    save_reports(
        output_dir=output_dir,
        y=y,
        class_order=class_order,
        base_metrics=base_metrics,
        meta_fast_metrics=meta_fast_metrics,
        strict_metrics=strict_metrics,
        meta_oof_pred=meta_oof_pred,
        strict_oof_pred=strict_oof_pred,
    )

    save_feature_importance(output_dir, final_preprocessor, final_base_models)

    model_bundle = {
        "config": asdict(cfg),
        "class_order": class_order,
        "class_to_int": class_to_int,
        "int_to_class": int_to_class,
        "categorical_columns": CATEGORICAL_COLUMNS,
        "numeric_columns": NUMERIC_COLUMNS,
        "feature_columns": FEATURE_COLUMNS,
        "preprocessor": final_preprocessor,
        "base_models": final_base_models,
        "meta_model": final_meta_model,
        "meta_feature_names": meta_feature_names,
        "best_params": tuned_params_by_model,
    }
    joblib.dump(model_bundle, output_dir / "final_stacking_model.joblib")

    summary = {
        "config": asdict(cfg),
        "class_order": class_order,
        "sample_weight_details": weight_details,
        "base_metrics": base_metrics,
        "meta_fast_metrics": meta_fast_metrics,
        "strict_stack_metrics": strict_metrics,
        "submission_path": str(submission_path),
        "artifacts": {
            "best_params": str(output_dir / "best_params.json"),
            "cv_results": str(output_dir / "cv_results.csv"),
            "model_bundle": str(output_dir / "final_stacking_model.joblib"),
            "base_oof_probabilities": str(output_dir / "base_oof_probabilities.npz"),
            "test_probabilities": str(output_dir / "test_probabilities.npz"),
        },
    }
    save_json(output_dir / "summary.json", summary)

    log("\n" + "=" * 80)
    log("PIPELINE COMPLETE")
    log(f"Submission: {submission_path}")
    log(f"Best params: {output_dir / 'best_params.json'}")
    log(f"CV results: {output_dir / 'cv_results.csv'}")
    log(f"Model bundle: {output_dir / 'final_stacking_model.joblib'}")
    log("=" * 80)


if __name__ == "__main__":
    main()
