# Irrigation Need Prediction — (Ensemble + Stacking Pipeline)

Multiclass classification pipeline predicting irrigation need (`Low` / `Medium` / `High`) using engineered agricultural/weather features, a suite of baseline and boosting models, a soft-voting ensemble, an OOF stacking layer, and multi-seed averaging.

**Final Kaggle Score:** 0.96198 | **Rank:** 992

Full writeup: see `Report Kaggle Competition.pdf` in this repo.

## Requirements

```
pip install -U pandas numpy scikit-learn matplotlib seaborn xgboost lightgbm catboost
```

Python 3.9+ recommended. CatBoost is optional — the script falls back to LightGBM + XGBoost only if it isn't installed.

## Input Files

Place these in a `data/` folder:
- `train.csv`
- `test.csv`
- `sample_submission.csv`

## How to Run

```bash
python main.py
```

No CLI arguments — configuration (seed, folds, multi-seed list) is set at the top of the script. Outputs are written to `outputs/`.

## Pipeline Steps

1. Load `train.csv` / `test.csv` / `sample_submission.csv`, encode target labels (`Low`, `Medium`, `High`)
2. Feature engineering — 18 engineered numeric features (evapotranspiration proxy, moisture stress, water balance, soil health, area-normalized, interaction terms) added to the base columns, giving 8 categorical + 29 numeric = 37 features
3. Two preprocessing branches:
   - sklearn branch: `OrdinalEncoder` + `StandardScaler`, used for Decision Tree, Naive Bayes, Logistic Regression, Linear Regression (OvR), K-Means, Random Forest
   - Tree branch: `OrdinalEncoder`, no scaling, used for XGBoost/LightGBM; CatBoost uses raw categorical columns natively
4. 5-fold `StratifiedKFold` (seed 42) with class-balanced sample weights, primary metric `balanced_accuracy_score`
5. Baseline models run with OOF CV: Decision Tree, Naive Bayes, Logistic Regression, Linear Regression (custom OvR), K-Means (unsupervised, 30k sample), Random Forest (100k subsample)
6. Strong models run with OOF CV: XGBoost, LightGBM, CatBoost
7. Soft-voting ensemble (weights 0.40 / 0.35 / 0.25 for LGBM/XGB/CatBoost) evaluated via OOF CV
8. Stacking layer: Logistic Regression meta-learner trained on 9-column OOF probability matrix from the three boosting models
9. Whichever of soft-voting or stacking has higher OOF balanced accuracy is selected for the final submission
10. Final models retrained across 5 seeds (42/52/62/72/82) per base model, test probabilities averaged across seeds
11. Final predictions generated via the selected path (stacking or soft-vote) → `submission.csv`
12. Confusion matrices, model comparison chart, feature importance chart, classification reports, and summary JSON generated

## Output Folder (`outputs/`)

| File | Description |
|---|---|
| `submission.csv` | Final Kaggle submission file |
| `confusion_matrices.png` | Row-normalized confusion matrices for all evaluated models |
| `model_comparison.png` | Balanced accuracy comparison bar chart across all models |
| `feature_importance.png` | Top 20 LightGBM feature importances (final model, last seed) |
| `classification_reports.txt` | Per-model precision/recall/F1/accuracy/balanced-accuracy reports |
| `summary.json` | Compact JSON summary of CV and OOF balanced accuracy / accuracy per model |

## Models Compared

| Model | Role |
|---|---|
| Decision Tree | Baseline supervised model |
| Gaussian Naive Bayes | Probabilistic baseline |
| Logistic Regression | Linear baseline |
| Linear Regression (custom OvR) | Linear model requirement |
| K-Means | Unsupervised baseline (cluster → majority-class mapping) |
| Random Forest | Bagged tree baseline |
| XGBoost | Gradient boosting |
| LightGBM | Gradient boosting |
| CatBoost | Gradient boosting, native categorical handling |
| Soft-Voting Ensemble | Weighted blend of LGBM/XGB/CatBoost probabilities |
| Stacking | Logistic Regression meta-learner on OOF base-model probabilities |

## Notes / Limitations

- No explicit missing-value imputation(the script assumes clean input data)
- Preprocessing (scaler/encoder) is fit once on full training data, not per-fold (unlike the later Optuna-tuned stacking pipeline, which fits per-fold to be stricter about leakage)
- No hyperparameter search (model parameters are hand-set)
- This was the earlier, coursework-oriented iteration; a later attempt added per-fold preprocessing and Optuna-based tuning for a higher score