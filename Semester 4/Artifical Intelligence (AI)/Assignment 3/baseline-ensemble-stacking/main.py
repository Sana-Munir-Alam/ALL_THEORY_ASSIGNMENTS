import warnings
warnings.filterwarnings("ignore")

import numpy as np
import pandas as pd
import matplotlib 
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path
import json, time

# Sklearn utilities
from sklearn.preprocessing      import LabelEncoder, OrdinalEncoder, StandardScaler
from sklearn.compose            import ColumnTransformer
from sklearn.model_selection    import StratifiedKFold
from sklearn.metrics            import (
    accuracy_score,
    balanced_accuracy_score,   # ← PRIMARY METRIC (v4 change)
    classification_report,
    confusion_matrix,
)
from sklearn.utils.class_weight import compute_class_weight

# Mandatory models
from sklearn.tree               import DecisionTreeClassifier
from sklearn.naive_bayes        import GaussianNB
from sklearn.linear_model       import LogisticRegression, LinearRegression
from sklearn.cluster            import KMeans
from sklearn.ensemble           import RandomForestClassifier
from sklearn.base               import BaseEstimator, ClassifierMixin

# Strong models
from xgboost  import XGBClassifier
from lightgbm import LGBMClassifier

try:
    from catboost import CatBoostClassifier
    HAS_CB = True
    print("[INFO] CatBoost available")
except ImportError:
    HAS_CB = False
    print("[WARN] CatBoost not installed — pip install catboost")
    print("       Ensemble will fall back to LGBM+XGB only.")

# ── Output directory ──────────────────────────────────────────────────────────
OUT  = Path("outputs")
OUT.mkdir(exist_ok=True)

SEED = 42
np.random.seed(SEED)

print("=" * 65)
print("  IRRIGATION NEED — HIGH-PERFORMANCE PIPELINE  (v4)")
print("  Metric: balanced_accuracy_score (competition-aligned)")
print("=" * 65)

# ─────────────────────────────────────────────────────────────────────────────
# 1.  LOAD DATA
# ─────────────────────────────────────────────────────────────────────────────
print("\n[1] Loading data ...")

train_raw  = pd.read_csv("data/train.csv")
test_raw   = pd.read_csv("data/test.csv")
sample_sub = pd.read_csv("data/sample_submission.csv")

TARGET      = "Irrigation_Need"
ID_COL      = "id"
CLASS_ORDER = ["Low", "Medium", "High"]

print(f"    Train : {train_raw.shape}")
print(f"    Test  : {test_raw.shape}")
print(f"    Target distribution:")
print(train_raw[TARGET].value_counts().to_string())

print(f"\n    Missing values (train): {train_raw.isnull().sum().sum()}")

target_enc = LabelEncoder()
target_enc.fit(CLASS_ORDER)
y   = target_enc.transform(train_raw[TARGET].values)
lev = target_enc.transform(CLASS_ORDER)
print(f"    Class map: {dict(zip(CLASS_ORDER, lev))}")

# ─────────────────────────────────────────────────────────────────────────────
# 2.  FEATURE ENGINEERING  (unchanged from v3)
# ─────────────────────────────────────────────────────────────────────────────
print("\n[2] Feature engineering ...")

def engineer(df: pd.DataFrame) -> pd.DataFrame:
    d   = df.copy()
    eps = 1e-6

    d["evapo_proxy"]          = (d["Temperature_C"] * d["Wind_Speed_kmh"] * (1 - d["Humidity"] / 100).clip(0))
    d["wind_drying"]          = d["Wind_Speed_kmh"] * d["Sunlight_Hours"]
    d["sunshine_stress"]      = d["Sunlight_Hours"] / (d["Humidity"] / 100 + eps)
    d["heat_index"]           = d["Temperature_C"] * (1 + 0.02 * d["Humidity"])

    d["moisture_deficit"]     = 100.0 - d["Soil_Moisture"]
    d["temp_moisture_stress"] = d["Temperature_C"] / (d["Soil_Moisture"] + eps)
    d["moisture_sq"]          = d["Soil_Moisture"] ** 2

    d["water_balance"]        = d["Rainfall_mm"] - d["Previous_Irrigation_mm"]
    d["net_water_avail"]      = (d["Rainfall_mm"] + d["Previous_Irrigation_mm"]) / (d["Temperature_C"] + eps)
    d["rain_moisture"]        = d["Rainfall_mm"] * d["Soil_Moisture"]

    d["soil_health"]          = (d["Soil_pH"] * d["Organic_Carbon"] / (d["Electrical_Conductivity"] + eps))
    d["salinity_stress"]      = d["Electrical_Conductivity"] / (d["Soil_pH"] + eps)

    d["rainfall_per_ha"]      = d["Rainfall_mm"] / (d["Field_Area_hectare"] + eps)
    d["prev_irr_per_ha"]      = d["Previous_Irrigation_mm"] / (d["Field_Area_hectare"] + eps)
    d["area_log"]             = np.log1p(d["Field_Area_hectare"])

    d["moisture_wind"]        = d["Soil_Moisture"] * d["Wind_Speed_kmh"]
    d["temp_wind"]            = d["Temperature_C"] * d["Wind_Speed_kmh"]
    d["need_proxy"]           = (d["temp_moisture_stress"] * d["evapo_proxy"] / (d["Rainfall_mm"] + eps))
    return d

train_fe = engineer(train_raw)
test_fe  = engineer(test_raw)

cat_cols = [
    "Soil_Type", "Crop_Type", "Crop_Growth_Stage", "Season",
    "Irrigation_Type", "Water_Source", "Mulching_Used", "Region"
]
num_cols = [
    "Soil_pH", "Soil_Moisture", "Organic_Carbon", "Electrical_Conductivity",
    "Temperature_C", "Humidity", "Rainfall_mm", "Sunlight_Hours",
    "Wind_Speed_kmh", "Field_Area_hectare", "Previous_Irrigation_mm",
    "evapo_proxy", "wind_drying", "sunshine_stress", "heat_index",
    "moisture_deficit", "temp_moisture_stress", "moisture_sq",
    "water_balance", "net_water_avail", "rain_moisture",
    "soil_health", "salinity_stress",
    "rainfall_per_ha", "prev_irr_per_ha", "area_log",
    "moisture_wind", "temp_wind", "need_proxy",
]
print(f"    Features: {len(cat_cols)} cat + {len(num_cols)} num = {len(cat_cols)+len(num_cols)} total")

# ─────────────────────────────────────────────────────────────────────────────
# 3.  PREPROCESSING
# ─────────────────────────────────────────────────────────────────────────────
print("\n[3] Preprocessing ...")

pre_sk = ColumnTransformer([
    ("cat", OrdinalEncoder(handle_unknown="use_encoded_value", unknown_value=-1), cat_cols),
    ("num", StandardScaler(), num_cols),
])
X_sk      = pre_sk.fit_transform(train_fe[cat_cols + num_cols])
X_test_sk = pre_sk.transform(test_fe[cat_cols + num_cols])

pre_tree = ColumnTransformer([
    ("cat", OrdinalEncoder(handle_unknown="use_encoded_value", unknown_value=-1), cat_cols),
    ("num", "passthrough", num_cols),
])
X_tree      = pre_tree.fit_transform(train_fe[cat_cols + num_cols])
X_test_tree = pre_tree.transform(test_fe[cat_cols + num_cols])

print(f"    X_sk shape  : {X_sk.shape}")
print(f"    X_tree shape: {X_tree.shape}")

classes = np.unique(y)
cw      = compute_class_weight("balanced", classes=classes, y=y)
cw_dict = {int(c): float(w) for c, w in zip(classes, cw)}
sw      = np.array([cw_dict[int(yi)] for yi in y])
print(f"    Class weights: {cw_dict}")
print(f"      → 'High' class is weighted {cw[2]/cw[0]:.1f}x higher than 'Low'")

# ─────────────────────────────────────────────────────────────────────────────
# 4.  CROSS-VALIDATION SETUP
# ─────────────────────────────────────────────────────────────────────────────
print("\n[4] CV setup: 5-Fold StratifiedKFold ...")
cv = StratifiedKFold(n_splits=5, shuffle=True, random_state=SEED)
results = {}

# ── v4 CHANGE: run_cv now uses balanced_accuracy_score ───────────────────────
def run_cv(name: str, clf, Xd: np.ndarray, yd: np.ndarray, use_sw: bool = False):
    """
    5-fold stratified CV storing OOF predictions.
    PRIMARY METRIC: balanced_accuracy_score (aligns with Kaggle metric).
    Also stores regular accuracy for reference.
    """
    print(f"\n    [{name}]")
    t0 = time.time()
    oof = np.zeros(len(yd), dtype=int)
    scores_bal = []
    scores_acc = []
    for tri, vli in cv.split(Xd, yd):
        fit_kw = {"sample_weight": sw[tri]} if use_sw else {}
        clf.fit(Xd[tri], yd[tri], **fit_kw)
        pred = clf.predict(Xd[vli])
        oof[vli] = pred
        scores_bal.append(balanced_accuracy_score(yd[vli], pred))
        scores_acc.append(accuracy_score(yd[vli], pred))
    m_bal, s_bal = np.mean(scores_bal), np.std(scores_bal)
    m_acc        = np.mean(scores_acc)
    oa_bal       = balanced_accuracy_score(yd, oof)
    oa_acc       = accuracy_score(yd, oof)
    results[name] = dict(
        cv_mean=m_bal, cv_std=s_bal,        # balanced (primary)
        cv_acc=m_acc,                        # accuracy (reference)
        oof_acc=oa_bal,                      # balanced OOF (primary)
        oof_raw_acc=oa_acc,                  # accuracy OOF (reference)
        y_pred=oof, y_true=yd,
    )
    print(f"       BAL_ACC CV : {m_bal:.5f} ± {s_bal:.5f}  ({time.time()-t0:.0f}s)")
    print(f"       BAL_ACC OOF: {oa_bal:.5f}  |  ACC OOF: {oa_acc:.5f}")

# ─────────────────────────────────────────────────────────────────────────────
# 5.  MANDATORY MODEL A — DECISION TREE
# ─────────────────────────────────────────────────────────────────────────────
print("\n[5] MANDATORY A — Decision Tree")
run_cv("Decision Tree", DecisionTreeClassifier(max_depth=15, min_samples_leaf=10, class_weight="balanced", random_state=SEED), X_sk, y)

# ─────────────────────────────────────────────────────────────────────────────
# 6.  MANDATORY MODEL B — NAIVE BAYES
# ─────────────────────────────────────────────────────────────────────────────
print("\n[6] MANDATORY B — Gaussian Naive Bayes")
run_cv("Naive Bayes", GaussianNB(), X_sk, y)

# ─────────────────────────────────────────────────────────────────────────────
# 7.  MANDATORY MODEL C — LOGISTIC REGRESSION
# ─────────────────────────────────────────────────────────────────────────────
print("\n[7] MANDATORY C — Logistic Regression")
run_cv("Logistic Regression", LogisticRegression(solver="lbfgs", max_iter=1000, class_weight="balanced", C=1.0, random_state=SEED), X_sk, y)

class LinearRegressionClassifier(BaseEstimator, ClassifierMixin):
    """One-vs-Rest Linear Regression classifier (literal OvR interpretation)."""
    def fit(self, X, y):
        self.classes_ = np.unique(y)
        self.regs_    = {
            c: LinearRegression().fit(X, (y == c).astype(float))
            for c in self.classes_
        }
        return self
    def predict(self, X):
        scores = np.column_stack([self.regs_[c].predict(X) for c in self.classes_])
        return self.classes_[np.argmax(scores, axis=1)]

run_cv("Linear Reg (OvR)", LinearRegressionClassifier(), X_sk, y)

# ─────────────────────────────────────────────────────────────────────────────
# 8.  MANDATORY MODEL D — K-MEANS
# ─────────────────────────────────────────────────────────────────────────────
print("\n[8] MANDATORY D — K-Means (Unsupervised, 30k sample)")
idx_km    = np.random.RandomState(SEED).choice(len(X_sk), 30_000, replace=False)
Xkm, ykm = X_sk[idx_km], y[idx_km]

km  = KMeans(n_clusters=3, random_state=SEED, n_init=10, max_iter=300)
km.fit(Xkm)
lbl  = km.predict(Xkm)
c2c  = {c: np.bincount(ykm[lbl == c]).argmax() for c in range(3)}
yp_km = np.array([c2c[c] for c in lbl])

km_bal = balanced_accuracy_score(ykm, yp_km)
km_acc = accuracy_score(ykm, yp_km)
results["K-Means"] = dict(cv_mean=km_bal, cv_std=0.0, cv_acc=km_acc, oof_acc=km_bal, oof_raw_acc=km_acc, y_pred=yp_km, y_true=ykm,)
print(f"    K-Means balanced accuracy: {km_bal:.5f}  (acc: {km_acc:.5f})")

# ─────────────────────────────────────────────────────────────────────────────
# 9.  STRONG MODEL E — RANDOM FOREST
# ─────────────────────────────────────────────────────────────────────────────
print("\n[9] STRONG E — Random Forest (CV on 100k subsample)")
idx_rf = np.random.RandomState(SEED).choice(len(X_sk), 100_000, replace=False)
run_cv("Random Forest", RandomForestClassifier(n_estimators=200, max_depth=20, min_samples_leaf=10, max_features="sqrt", class_weight="balanced_subsample", n_jobs=-1, random_state=SEED), X_sk[idx_rf], y[idx_rf])

# ─────────────────────────────────────────────────────────────────────────────
# 10.  STRONG MODEL F — XGBOOST
# ─────────────────────────────────────────────────────────────────────────────
print("\n[10] STRONG F — XGBoost (full data)")
run_cv("XGBoost", XGBClassifier(n_estimators=500, max_depth=6, learning_rate=0.05, subsample=0.8, colsample_bytree=0.8, reg_alpha=0.1, reg_lambda=1.0, objective="multi:softmax", num_class=3, eval_metric="mlogloss", tree_method="hist", n_jobs=-1, random_state=SEED, verbosity=0), X_tree, y, use_sw=True)

# ─────────────────────────────────────────────────────────────────────────────
# 11.  STRONG MODEL G — LIGHTGBM
# ─────────────────────────────────────────────────────────────────────────────
print("\n[11] STRONG G — LightGBM (full data)")
run_cv("LightGBM", LGBMClassifier(n_estimators=1000, num_leaves=127, learning_rate=0.03, subsample=0.8, subsample_freq=1, colsample_bytree=0.8, reg_alpha=0.1, reg_lambda=1.0, min_child_samples=20, class_weight="balanced", objective="multiclass", num_class=3, n_jobs=-1, random_state=SEED, verbosity=-1), X_tree, y, use_sw=True)

# ─────────────────────────────────────────────────────────────────────────────
# 12.  STRONG MODEL H — CATBOOST
# ─────────────────────────────────────────────────────────────────────────────
if HAS_CB:
    print("\n[12] STRONG H — CatBoost (full data)")
    cb_params = dict(iterations=800, depth=6, learning_rate=0.05, l2_leaf_reg=3.0, random_seed=SEED, loss_function="MultiClass", eval_metric="Accuracy", class_weights=list(cw), cat_features=list(range(len(cat_cols))), verbose=0, thread_count=-1,)

    print("\n[CatBoost]")
    t0 = time.time()
    X_cb      = train_fe[cat_cols + num_cols]
    X_test_cb = test_fe[cat_cols + num_cols]
    oof_cb    = np.zeros(len(y), dtype=int)
    scores_cb_bal = []
    scores_cb_acc = []

    for tri, vli in cv.split(X_cb, y):
        cb = CatBoostClassifier(**cb_params)
        cb.fit(X_cb.iloc[tri], y[tri], sample_weight=sw[tri], verbose=0)
        pred_cb        = cb.predict(X_cb.iloc[vli]).ravel()
        oof_cb[vli]    = pred_cb
        scores_cb_bal.append(balanced_accuracy_score(y[vli], pred_cb))
        scores_cb_acc.append(accuracy_score(y[vli], pred_cb))

    m_cb_bal  = np.mean(scores_cb_bal)
    s_cb_bal  = np.std(scores_cb_bal)
    oa_cb_bal = balanced_accuracy_score(y, oof_cb)
    oa_cb_acc = accuracy_score(y, oof_cb)

    results["CatBoost"] = dict(cv_mean=m_cb_bal, cv_std=s_cb_bal, cv_acc=np.mean(scores_cb_acc), oof_acc=oa_cb_bal, oof_raw_acc=oa_cb_acc, y_pred=oof_cb, y_true=y,)
    print(f"  BAL_ACC CV: {m_cb_bal:.5f}±{s_cb_bal:.5f}  OOF: {oa_cb_bal:.5f}  ({time.time()-t0:.0f}s)")
else:
    cb_params    = None
    X_cb         = train_fe[cat_cols + num_cols]
    X_test_cb    = test_fe[cat_cols + num_cols]

# ─────────────────────────────────────────────────────────────────────────────
# 13.  SOFT-VOTING ENSEMBLE CV  (LightGBM + XGBoost + CatBoost)
# ─────────────────────────────────────────────────────────────────────────────
print("\n[13] ENSEMBLE — Soft-Voting CV (LightGBM + XGBoost + CatBoost)")
lgbm_params_ens = dict(n_estimators=1000, num_leaves=127, learning_rate=0.03, subsample=0.8, subsample_freq=1, colsample_bytree=0.8, reg_alpha=0.1, reg_lambda=1.0, min_child_samples=20, class_weight="balanced", objective="multiclass", num_class=3, n_jobs=-1, random_state=SEED, verbosity=-1,)
xgb_params_ens = dict(n_estimators=500, max_depth=6, learning_rate=0.05, subsample=0.8, colsample_bytree=0.8, reg_alpha=0.1, reg_lambda=1.0, objective="multi:softprob", num_class=3, eval_metric="mlogloss", tree_method="hist", n_jobs=-1, random_state=SEED, verbosity=0,)

print("    Running ensemble CV (3 models × 5 folds = 15 fits) ...")
t0 = time.time()

# ── We store OOF probabilities from each model separately ────────────────────
# These will be reused as inputs to the stacking meta-learner (Section 14).
oof_proba_lgbm = np.zeros((len(y), 3))   # shape: (n_samples, n_classes)
oof_proba_xgb  = np.zeros((len(y), 3))
oof_proba_cb   = np.zeros((len(y), 3))
oof_proba_ens  = np.zeros((len(y), 3))   # weighted blend

fold_accs_ens_bal = []
fold_accs_ens_acc = []

for fold_i, (tri, vli) in enumerate(cv.split(X_tree, y)):

    # ── LightGBM ──────────────────────────────────────────────────────────
    lg = LGBMClassifier(**lgbm_params_ens)
    lg.fit(X_tree[tri], y[tri], sample_weight=sw[tri])
    p_lg = lg.predict_proba(X_tree[vli])
    oof_proba_lgbm[vli] = p_lg

    # ── XGBoost ───────────────────────────────────────────────────────────
    xg = XGBClassifier(**xgb_params_ens)
    xg.fit(X_tree[tri], y[tri], sample_weight=sw[tri], verbose=False)
    p_xg = xg.predict_proba(X_tree[vli])
    oof_proba_xgb[vli] = p_xg

    # ── CatBoost ──────────────────────────────────────────────────────────
    if HAS_CB:
        cb2 = CatBoostClassifier(**cb_params)
        cb2.fit(X_cb.iloc[tri], y[tri], sample_weight=sw[tri], verbose=0)
        p_cb = cb2.predict_proba(X_cb.iloc[vli])
        oof_proba_cb[vli] = p_cb
        p_ens = 0.40 * p_lg + 0.35 * p_xg + 0.25 * p_cb
    else:
        p_ens = 0.55 * p_lg + 0.45 * p_xg

    oof_proba_ens[vli] = p_ens
    ens_pred = np.argmax(p_ens, axis=1)

    fa_bal = balanced_accuracy_score(y[vli], ens_pred)
    fa_acc = accuracy_score(y[vli], ens_pred)
    fold_accs_ens_bal.append(fa_bal)
    fold_accs_ens_acc.append(fa_acc)
    print(f"       Fold {fold_i+1}: BAL={fa_bal:.5f}  ACC={fa_acc:.5f}  ({time.time()-t0:.0f}s)")

oof_ens     = np.argmax(oof_proba_ens, axis=1)
ens_bal_acc = balanced_accuracy_score(y, oof_ens)
ens_acc     = accuracy_score(y, oof_ens)
results["Ensemble"] = dict(cv_mean=np.mean(fold_accs_ens_bal), cv_std=np.std(fold_accs_ens_bal), cv_acc=np.mean(fold_accs_ens_acc), oof_acc=ens_bal_acc, oof_raw_acc=ens_acc, y_pred=oof_ens, y_true=y,)
print(f"\n       Ensemble CV  BAL: {np.mean(fold_accs_ens_bal):.5f} ± {np.std(fold_accs_ens_bal):.5f}")
print(f"       Ensemble OOF BAL: {ens_bal_acc:.5f}  |  OOF ACC: {ens_acc:.5f}")

# ─────────────────────────────────────────────────────────────────────────────
# 14.  STACKING LAYER  (NEW IN v4)
# ─────────────────────────────────────────────────────────────────────────────
#
# PLACEMENT: After ensemble CV, before final retraining on full dataset.
#
# HOW IT WORKS:
#   Base learners: LightGBM, XGBoost, CatBoost
#   Meta-features: the OOF predicted probabilities from each base learner.
#                  Each fold's OOF block was generated by a model that never
#                  saw that fold → zero data leakage by construction.
#   Meta-learner:  Logistic Regression (simple, fast, well-calibrated).
#                  Trained on shape (n_train, 9) — 3 models × 3 class probs.
#
# WHY NO LEAKAGE:
#   oof_proba_lgbm[i] was predicted by a model trained WITHOUT sample i.
#   Same for XGB and CatBoost. Concatenating them is safe as meta-features.
#
# EXPECTED GAIN: +0.002 to +0.012 balanced accuracy (depends on model diversity).
#
print("\n[14] STACKING — Meta-learner on OOF probabilities (v4 new)")
# Build the meta-feature matrix from OOF probabilities
# Shape: (n_train, 9)  [3 probs per model × 3 models]
if HAS_CB:
    meta_X_train = np.hstack([oof_proba_lgbm, oof_proba_xgb, oof_proba_cb])
else:
    meta_X_train = np.hstack([oof_proba_lgbm, oof_proba_xgb])

print(f"    Meta-feature matrix shape: {meta_X_train.shape}")

# ── Evaluate stacking with nested CV (5-fold on the same folds) ──────────────
# We use the SAME StratifiedKFold splits so fold boundaries are identical.
# This gives an honest estimate of stacking performance.
print("    Evaluating stacking meta-learner (5-fold CV on OOF features) ...")
t0 = time.time()

meta_model_cv = LogisticRegression( solver="lbfgs", max_iter=2000, class_weight="balanced", C=1.0, random_state=SEED,)

oof_stack     = np.zeros(len(y), dtype=int)
oof_stack_proba = np.zeros((len(y), 3))
scores_stack_bal = []
scores_stack_acc = []

for tri, vli in cv.split(meta_X_train, y):
    meta_model_cv.fit(meta_X_train[tri], y[tri], sample_weight=sw[tri])
    pred_stack          = meta_model_cv.predict(meta_X_train[vli])
    prob_stack          = meta_model_cv.predict_proba(meta_X_train[vli])
    oof_stack[vli]      = pred_stack
    oof_stack_proba[vli]= prob_stack
    scores_stack_bal.append(balanced_accuracy_score(y[vli], pred_stack))
    scores_stack_acc.append(accuracy_score(y[vli], pred_stack))

stack_bal_cv  = np.mean(scores_stack_bal)
stack_bal_std = np.std(scores_stack_bal)
stack_oof_bal = balanced_accuracy_score(y, oof_stack)
stack_oof_acc = accuracy_score(y, oof_stack)

results["Stacking"] = dict(
    cv_mean=stack_bal_cv, cv_std=stack_bal_std,
    cv_acc=np.mean(scores_stack_acc),
    oof_acc=stack_oof_bal, oof_raw_acc=stack_oof_acc,
    y_pred=oof_stack, y_true=y,
)
print(f"    BAL_ACC CV : {stack_bal_cv:.5f} ± {stack_bal_std:.5f}  ({time.time()-t0:.0f}s)")
print(f"    BAL_ACC OOF: {stack_oof_bal:.5f}  |  ACC OOF: {stack_oof_acc:.5f}")

# Compare stacking vs soft-voting ensemble
print(f"\n    Ensemble (soft-vote) OOF BAL: {ens_bal_acc:.5f}")
print(f"    Stacking               OOF BAL: {stack_oof_bal:.5f}")
use_stacking_for_submission = stack_oof_bal > ens_bal_acc
print(f"    → Using {'STACKING' if use_stacking_for_submission else 'SOFT-VOTE ENSEMBLE'} for final submission")

# ── Train final meta-learner on ALL training OOF probabilities ────────────────
# This is the meta-model we will apply to test set predictions.
final_meta_model = LogisticRegression(
    solver="lbfgs", max_iter=2000,
    class_weight="balanced", C=1.0, random_state=SEED,
)
final_meta_model.fit(meta_X_train, y, sample_weight=sw)
print("    Final meta-learner fitted on full OOF meta-features.")

# ─────────────────────────────────────────────────────────────────────────────
# 15.  CONFUSION MATRICES  (updated to include Stacking)
# ─────────────────────────────────────────────────────────────────────────────
print("\n[15] Generating confusion matrices ...")
CLASS_LABELS = CLASS_ORDER

all_models = [
    "Decision Tree", "Naive Bayes", "Logistic Regression", "Linear Reg (OvR)",
    "K-Means", "Random Forest", "XGBoost", "LightGBM",
]
if HAS_CB:
    all_models.append("CatBoost")
all_models += ["Ensemble", "Stacking"]

ncols = 4
nrows = int(np.ceil(len(all_models) / ncols))
fig, axes = plt.subplots(nrows, ncols, figsize=(22, nrows * 5 + 1))
fig.suptitle(
    "Confusion Matrices — 5-Fold OOF Predictions (row-normalised recall)\n"
    "PRIMARY METRIC: Balanced Accuracy  |  Rows = True, Columns = Predicted",
    fontsize=13, fontweight="bold",
)
axes = axes.flatten()

for ax, name in zip(axes, all_models):
    r    = results[name]
    cm   = confusion_matrix(r["y_true"], r["y_pred"], labels=lev)
    cm_n = cm.astype(float) / cm.sum(axis=1, keepdims=True)
    sns.heatmap(cm_n, ax=ax, annot=True, fmt=".2f", cmap="Blues", xticklabels=CLASS_LABELS, yticklabels=CLASS_LABELS, vmin=0, vmax=1, cbar=False)
    ax.set_title(f"{name}\nBAL={results[name]['oof_acc']:.4f}  ACC={results[name].get('oof_raw_acc', results[name]['oof_acc']):.4f}", fontsize=10, fontweight="bold",)
    ax.set_xlabel("Predicted", fontsize=9)
    ax.set_ylabel("True", fontsize=9)

for ax in axes[len(all_models):]:
    ax.set_visible(False)

plt.tight_layout()
plt.savefig(OUT / "confusion_matrices.png", dpi=150, bbox_inches="tight")
plt.close()
print("    Saved: outputs/confusion_matrices.png")

# ─────────────────────────────────────────────────────────────────────────────
# 16.  MODEL COMPARISON CHART
# ─────────────────────────────────────────────────────────────────────────────
print("\n[16] Model comparison chart ...")
means   = [results[m]["cv_mean"] for m in all_models]
stds    = [results[m]["cv_std"]  for m in all_models]
palette = ["#e63946","#457b9d","#2a9d8f","#e9c46a","#888888", "#264653","#f4a261","#a8dadc","#6d6875","#023047","#9b2226"]

fig, ax = plt.subplots(figsize=(13, 7))
bars = ax.barh(all_models, means, xerr=stds,
               color=palette[:len(all_models)], capsize=4,
               height=0.55, edgecolor="white")
for b, m, s in zip(bars, means, stds):
    ax.text(b.get_width() + 0.003, b.get_y() + b.get_height() / 2,
            f"{m:.4f} ± {s:.4f}", va="center", fontsize=9)
ax.axvline(max(means), color="black", ls="--", alpha=0.4,
           label=f"Best = {max(means):.4f}")
ax.set_xlabel("Balanced Accuracy (5-Fold CV)  ← Competition Metric")
ax.set_xlim(0, 1.18)
ax.set_title("Model Comparison — 5-Fold Stratified CV\n"
             "PRIMARY: Balanced Accuracy  (error bars = ± 1 std)",
             fontweight="bold", fontsize=12)
ax.legend(fontsize=10)
plt.tight_layout()
plt.savefig(OUT / "model_comparison.png", dpi=150, bbox_inches="tight")
plt.close()
print("    Saved: outputs/model_comparison.png")

# ─────────────────────────────────────────────────────────────────────────────
# 17.  CLASSIFICATION REPORTS
# ─────────────────────────────────────────────────────────────────────────────
print("\n[17] Classification reports ...")

report_path = OUT / "classification_reports.txt"
with open(report_path, "w") as f:
    f.write("=" * 65 + "\n")
    f.write("  IRRIGATION NEED v4 — CLASSIFICATION REPORTS\n")
    f.write("  Primary metric: balanced_accuracy_score\n")
    f.write("=" * 65 + "\n")

    for name in all_models:
        r      = results[name]
        report = classification_report(
            r["y_true"], r["y_pred"],
            target_names=CLASS_LABELS, labels=lev, zero_division=0,
        )
        block = (f"\n{'='*50}\n{name}\n"
                 f"  BAL_ACC: {r['oof_acc']:.5f}  |  ACC: {r.get('oof_raw_acc', r['oof_acc']):.5f}\n"
                 f"{'='*50}\n{report}")
        print(block)
        f.write(block)

print(f"    Saved: {report_path}")

# ─────────────────────────────────────────────────────────────────────────────
# 18.  BEST MODEL SELECTION
# ─────────────────────────────────────────────────────────────────────────────
print("\n[18] Model ranking (by balanced accuracy) ...")
sup_res   = {k: v for k, v in results.items() if k != "K-Means"}
best_name = max(sup_res, key=lambda k: sup_res[k]["cv_mean"])
best_cv   = sup_res[best_name]["cv_mean"]
print(f"    Best single model : {best_name}  (BAL_CV={best_cv:.5f})")
print(f"    Ensemble (soft)   : {results['Ensemble']['cv_mean']:.5f}")
print(f"    Stacking          : {results['Stacking']['cv_mean']:.5f}")
print(f"    Submitting        : {'STACKING' if use_stacking_for_submission else 'SOFT-VOTE ENSEMBLE'}")

# ─────────────────────────────────────────────────────────────────────────────
# 19.  FINAL ENSEMBLE — MULTI-SEED AVERAGING (NEW IN v4)
# ─────────────────────────────────────────────────────────────────────────────
SEEDS = [42, 52, 62, 72, 82]

print(f"\n[19] Final ensemble — Multi-seed averaging ({len(SEEDS)} seeds × 3 models = {len(SEEDS)*3} fits) ...")

# Accumulators for test probabilities
p_lgbm_test_acc = np.zeros((len(test_raw), 3))
p_xgb_test_acc  = np.zeros((len(test_raw), 3))
p_cb_test_acc   = np.zeros((len(test_raw), 3))

# Accumulators for meta-feature generation on test set
# (needed if we use stacking for submission)
p_lgbm_test_meta = np.zeros((len(test_raw), 3))
p_xgb_test_meta  = np.zeros((len(test_raw), 3))
p_cb_test_meta   = np.zeros((len(test_raw), 3))

lgbm_params_final = dict(
    n_estimators=1500, num_leaves=127, learning_rate=0.025,
    subsample=0.8, subsample_freq=1, colsample_bytree=0.8,
    reg_alpha=0.1, reg_lambda=1.0, min_child_samples=20,
    class_weight="balanced", objective="multiclass", num_class=3,
    n_jobs=-1, verbosity=-1,
)
xgb_params_final = dict(
    n_estimators=600, max_depth=6, learning_rate=0.05,
    subsample=0.8, colsample_bytree=0.8,
    reg_alpha=0.1, reg_lambda=1.0,
    objective="multi:softprob", num_class=3,
    eval_metric="mlogloss", tree_method="hist",
    n_jobs=-1, verbosity=0,
)

for i, seed in enumerate(SEEDS):
    print(f"\n    Seed {seed}  ({i+1}/{len(SEEDS)}) ...")

    # ── LightGBM ──────────────────────────────────────────────────────────
    t0 = time.time()
    lg_f = LGBMClassifier(**{**lgbm_params_final, "random_state": seed})
    lg_f.fit(X_tree, y, sample_weight=sw)
    p_lgbm_test_acc  += lg_f.predict_proba(X_test_tree)
    p_lgbm_test_meta += lg_f.predict_proba(X_test_tree)
    print(f"      LGBM done  ({time.time()-t0:.0f}s)")

    # ── XGBoost ───────────────────────────────────────────────────────────
    t0 = time.time()
    xg_f = XGBClassifier(**{**xgb_params_final, "random_state": seed})
    xg_f.fit(X_tree, y, sample_weight=sw, verbose=False)
    p_xgb_test_acc  += xg_f.predict_proba(X_test_tree)
    p_xgb_test_meta += xg_f.predict_proba(X_test_tree)
    print(f"      XGB  done  ({time.time()-t0:.0f}s)")

    # ── CatBoost ──────────────────────────────────────────────────────────
    if HAS_CB:
        t0 = time.time()
        cb_f = CatBoostClassifier(**{**cb_params, "random_seed": seed})
        cb_f.fit(X_cb, y, sample_weight=sw, verbose=0)
        p_cb_test_acc  += cb_f.predict_proba(X_test_cb)
        p_cb_test_meta += cb_f.predict_proba(X_test_cb)
        print(f"      CB   done  ({time.time()-t0:.0f}s)")

# Average across seeds
n_seeds = len(SEEDS)
p_lgbm_test_acc  /= n_seeds
p_xgb_test_acc   /= n_seeds
p_lgbm_test_meta /= n_seeds
p_xgb_test_meta  /= n_seeds

if HAS_CB:
    p_cb_test_acc  /= n_seeds
    p_cb_test_meta /= n_seeds

print(f"\n    All {len(SEEDS)*3} model fits complete. Probabilities averaged.")

# ─────────────────────────────────────────────────────────────────────────────
# 20.  GENERATE FINAL TEST PREDICTIONS
# ─────────────────────────────────────────────────────────────────────────────
print("\n[20] Generating final test predictions ...")

if use_stacking_for_submission:
    # ── Path A: Stacking via meta-learner ─────────────────────────────────
    print("    Using STACKING path ...")
    if HAS_CB:
        meta_X_test = np.hstack([p_lgbm_test_meta, p_xgb_test_meta, p_cb_test_meta])
    else:
        meta_X_test = np.hstack([p_lgbm_test_meta, p_xgb_test_meta])
    y_test_enc = final_meta_model.predict(meta_X_test)
else:
    # ── Path B: Soft-voting ensemble ──────────────────────────────────────
    print("    Using SOFT-VOTE ENSEMBLE path ...")
    if HAS_CB:
        p_final = 0.40 * p_lgbm_test_acc + 0.35 * p_xgb_test_acc + 0.25 * p_cb_test_acc
    else:
        p_final = 0.55 * p_lgbm_test_acc + 0.45 * p_xgb_test_acc
    y_test_enc = np.argmax(p_final, axis=1)

y_test_labels = target_enc.inverse_transform(y_test_enc)

# ─────────────────────────────────────────────────────────────────────────────
# 21.  SUBMISSION FILE
# ─────────────────────────────────────────────────────────────────────────────
print("\n[21] Creating submission.csv ...")
submission = pd.DataFrame({
    "id"              : test_raw[ID_COL].values,
    "Irrigation_Need" : y_test_labels,
})

assert list(submission.columns) == list(sample_sub.columns), \
    f"Column mismatch! Got {list(submission.columns)}"
assert len(submission) == len(sample_sub), \
    f"Row count mismatch! Got {len(submission)}, expected {len(sample_sub)}"

submission.to_csv(OUT / "submission.csv", index=False)
print(f"    Saved: outputs/submission.csv")
print(f"    Prediction distribution:")
print(submission["Irrigation_Need"].value_counts().to_string())

# ─────────────────────────────────────────────────────────────────────────────
# 22.  FEATURE IMPORTANCE CHART
# ─────────────────────────────────────────────────────────────────────────────
print("\n[22] Feature importance (last LightGBM seed) ...")
feat_names = [f"cat_{c}" for c in cat_cols] + num_cols
imp        = lg_f.feature_importances_
top_idx    = np.argsort(imp)[-20:]

fig, ax = plt.subplots(figsize=(10, 7))
ax.barh([feat_names[i] for i in top_idx], imp[top_idx], color="#264653")
ax.set_title("Top 20 Feature Importances — LightGBM (final model, last seed)", fontweight="bold", fontsize=12)
ax.set_xlabel("Importance (total gain across all splits)")
plt.tight_layout()
plt.savefig(OUT / "feature_importance.png", dpi=150, bbox_inches="tight")
plt.close()
print("    Saved: outputs/feature_importance.png")

# ─────────────────────────────────────────────────────────────────────────────
# 23.  SUMMARY JSON
# ─────────────────────────────────────────────────────────────────────────────
summary = {
    n: {
        "bal_acc_cv_mean" : round(float(v["cv_mean"]),      5),
        "bal_acc_cv_std"  : round(float(v["cv_std"]),       5),
        "bal_acc_oof"     : round(float(v["oof_acc"]),      5),
        "acc_oof"         : round(float(v.get("oof_raw_acc", v["oof_acc"])), 5),
    }
    for n, v in results.items()
}
(OUT / "summary.json").write_text(json.dumps(summary, indent=2))

print("\nFINAL RESULTS SUMMARY (primary: balanced accuracy):")
print(json.dumps(summary, indent=2))

print("\n" + "=" * 65)
print(f"  PIPELINE v4 COMPLETE")
print(f"  Best single model : {best_name}  (BAL_CV={best_cv:.5f})")
print(f"  Ensemble CV BAL   : {results['Ensemble']['cv_mean']:.5f}")
print(f"  Stacking CV BAL   : {results['Stacking']['cv_mean']:.5f}")
print(f"  Submission method : {'STACKING' if use_stacking_for_submission else 'SOFT-VOTE ENSEMBLE'}")
print(f"  Multi-seed        : {len(SEEDS)} seeds × 3 models = {len(SEEDS)*3} final fits")
print(f"  Submission        : outputs/submission.csv")
print(f"  Charts            : outputs/confusion_matrices.png")
print(f"                      outputs/model_comparison.png")
print(f"                      outputs/feature_importance.png")
print(f"  Reports           : outputs/classification_reports.txt")
print("=" * 65)