# Irrigation Need Prediction — Optuna-Tuned OOF Stacking Pipeline

Multiclass classification pipeline predicting irrigation need (`Low` / `Medium` / `High`) using a leakage-safe out-of-fold (OOF) stacking ensemble of LightGBM, XGBoost, and CatBoost, combined with a Logistic Regression meta-learner tuned via Optuna.

**Final Kaggle Score:** 0.97144 | **Rank:** 766

Full writeup: see `Report Optuna Stacking.pdf` in this repo.

## Requirements

```
pip install -U pandas numpy scikit-learn optuna lightgbm xgboost catboost joblib
```

Python 3.9+ recommended.

## Input Files

Place these in a `data/` folder (or pass `--data-dir`):
- `train.csv`
- `test.csv`
- `sample_submission.csv`

## How to Run

```bash
python main.py
```

Default settings: 5-fold `StratifiedKFold`, seed 42, 30 Optuna trials per base model, strict full-stack CV enabled.

### Useful arguments

| Flag | Purpose |
|---|---|
| `--data-dir` | Folder containing train/test/sample_submission CSVs |
| `--output-dir` | Output folder (default: `outputs_optuna_stack`) |
| `--n-splits` | Number of CV folds (default: 5) |
| `--n-trials-lightgbm/xgboost/catboost` | Optuna trials per model (default: 30 each) |
| `--no-strict-stacking-cv` | Skip the expensive strict full-stack CV evaluation |
| `--no-optuna-sqlite-storage` | Disable resumable Optuna study storage |

Run `python stacking_pipeline.py --help` for the full list.

## Pipeline Steps

1. Load `train.csv` / `test.csv` / `sample_submission.csv`
2. Deterministic feature engineering (no target leakage) — evapotranspiration, moisture stress, water balance, soil health, and interaction features
3. Fold-specific `ColumnTransformer` preprocessing (median/most-frequent imputation + ordinal encoding), fitted only on each fold's training rows
4. Optuna (TPE + MedianPruner) tuning of LightGBM, XGBoost, CatBoost on shared `StratifiedKFold` splits, optimizing `balanced_accuracy_score`
5. Strictly out-of-fold probability generation per base model
6. Logistic Regression meta-learner trained on OOF probabilities only
7. Optional strict full-stack CV (inner-OOF meta-training per outer fold) for a stricter, leakage-safe performance estimate
8. Retrain base models on full training data with best params
9. Generate test probabilities → stack → write `submission.csv`

## Output Folder (`outputs_optuna_stack/`)

| File | Description |
|---|---|
| `config.json` | Run configuration (seed, folds, target/id columns, etc.) |
| `cv_folds.csv` | Row-to-fold mapping used across the whole pipeline |
| `optuna_studies.db` | Resumable Optuna SQLite storage |
| `optuna_trials_<model>.csv` | Per-trial Optuna results for each base model |
| `best_params.json` | Best hyperparameters per base model |
| `optuna_best_results.json` | Best Optuna value + params per base model |
| `base_oof_probabilities.npz` | OOF predicted probabilities per base model |
| `base_oof_metrics.json` | Per-fold and OOF balanced accuracy/accuracy per base model |
| `meta_train_oof_features.csv` | 9-column OOF meta-feature matrix (3 models × 3 classes) |
| `meta_feature_names.json` | Meta-feature column names |
| `meta_fast_cv_oof_predictions.npz` | Meta-learner CV predictions on OOF features |
| `strict_stack_cv_oof_predictions.npz` | Strict full-stack CV predictions (if enabled) |
| `cv_results.csv` | Summary CV metrics — base models, fast meta-CV, strict stack CV |
| `stacking_classification_report.txt` | Classification report + confusion matrix |
| `feature_importances.csv` | Feature importances from base models |
| `test_probabilities.npz` | Test-set probabilities per base model + final stacked probabilities |
| `final_stacking_model.joblib` | Full model bundle: preprocessor, base models, meta-model, class maps |
| `submission.csv` | Final Kaggle submission |
| `summary.json` | Consolidated run summary and artifact paths |

## Leakage Safety

- Feature engineering uses only input columns (no target)
- Preprocessing (`ColumnTransformer`) is fit per-fold, only on that fold's training rows
- Base-model meta-features are strictly out-of-fold
- Meta-learner never sees in-sample base predictions
- Base models are retrained on full data only *after* validation is complete

## Note on Approach

The advanced modeling choices (Random Forest, XGBoost, and eventually the full stacking setup) came from reading tips left by top scorers in the competition's discussion/comments section. Optuna-based hyperparameter tuning was then added on top of that based on AI assistance. This combination `OOF stacking + Optuna tuning` is what produced the final rank of 766.