# Datasets

This directory holds sensor CSV data collected by the E-Nose.

```
datasets/
├── raw/       ← Raw CSV files from the Wio Terminal (created by serial_data_collect_csv.py)
└── scaled/    ← Normalised CSV files output by the dataset curation notebook
```

Both directories are excluded from git (see `.gitignore`) because dataset files can be large and are environment-specific — sensor readings will vary between hardware units and environmental conditions.

---

## Collecting Your Own Data

Follow [docs/DATA_COLLECTION.md](../docs/DATA_COLLECTION.md) to capture labelled odor samples with your hardware.

```bash
python scripts/serial_data_collect_csv.py \
    -p <PORT> -b 115200 -d datasets/raw -l <label>
```

## Reference Dataset

The reference dataset from Shawn Hymel's tutorial is available at:
https://github.com/ShawnHymel/ai-nose/tree/main/datasets

> **Note:** It was collected in a specific environment and will likely not transfer directly to your setup. Collecting your own data in your environment will always yield better results.

## Tracking Datasets with Git LFS (optional)

If you want to version-control your CSVs, consider [Git Large File Storage](https://git-lfs.com/):

```bash
git lfs install
git lfs track "datasets/raw/*.csv"
git lfs track "datasets/scaled/*.csv"
git add .gitattributes
```
