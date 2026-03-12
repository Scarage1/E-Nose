#!/usr/bin/env python3
"""
plot_dataset.py — E-Nose Local Dataset Visualiser

Reads all raw CSV files from a directory and plots:
  1. Time-series of every sensor feature per file
  2. Per-label overlay — compare the same feature across different odors
  3. Box plots — distribution per label per feature

Usage
-----
    python scripts/plot_dataset.py -d datasets/raw
    python scripts/plot_dataset.py -d datasets/raw --feature temp humd co2
    python scripts/plot_dataset.py -d datasets/scaled --no-overlay

Dependencies
------------
    pip install pandas matplotlib seaborn

Author:  Shivam Kumar (Scarage1)
License: 0BSD (https://opensource.org/licenses/0BSD)
"""

import argparse
import os
import sys
from collections import defaultdict
from pathlib import Path

try:
    import pandas as pd
    import matplotlib.pyplot as plt
    import matplotlib.cm as cm
except ImportError as e:
    print(f"ERROR: Missing dependency — {e}")
    print("Run:  pip install pandas matplotlib")
    sys.exit(1)


# ─── Helpers ──────────────────────────────────────────────────────────────────

def parse_label(filename: str) -> str:
    """Extract label from '<label>.<epoch_ms>.csv' or return the stem."""
    stem = Path(filename).stem        # e.g. coffee.1741789234000
    parts = stem.split(".")
    return parts[0] if len(parts) >= 2 else stem


def load_directory(directory: str) -> dict[str, list[pd.DataFrame]]:
    """Load all CSVs in *directory* grouped by label."""
    data: dict[str, list[pd.DataFrame]] = defaultdict(list)
    csv_files = sorted(Path(directory).glob("*.csv"))

    if not csv_files:
        print(f"No .csv files found in '{directory}'")
        sys.exit(1)

    for path in csv_files:
        try:
            df = pd.read_csv(path)
            label = parse_label(path.name)
            data[label].append(df)
        except Exception as exc:
            print(f"Warning: could not read {path.name} — {exc}")

    print(f"Loaded {sum(len(v) for v in data.values())} file(s) across {len(data)} label(s):")
    for label, frames in data.items():
        rows = sum(len(f) for f in frames)
        print(f"  '{label}': {len(frames)} file(s), {rows} total rows")
    return data


# ─── Plot functions ───────────────────────────────────────────────────────────

def plot_timeseries(data: dict, features: list[str], directory: str) -> None:
    """Plot time-series for each individual CSV file."""
    all_files = [(label, df) for label, frames in data.items() for df in frames]
    n = len(all_files)
    if n == 0:
        return

    print(f"\nPlotting time-series for {n} file(s)...")
    cols = min(3, n)
    rows = (n + cols - 1) // cols
    fig, axes = plt.subplots(rows, cols, figsize=(cols * 5, rows * 3), squeeze=False)

    for idx, (label, df) in enumerate(all_files):
        ax = axes[idx // cols][idx % cols]
        valid = [f for f in features if f in df.columns]
        x = df.get("timestamp", pd.Series(range(len(df))))
        for feat in valid:
            ax.plot(x, df[feat], label=feat, linewidth=1)
        ax.set_title(f"{label} #{idx + 1}", fontsize=9)
        ax.legend(fontsize=7, ncol=2)
        ax.set_xlabel("timestamp" if "timestamp" in df.columns else "sample")

    # Hide unused subplots
    for idx in range(n, rows * cols):
        axes[idx // cols][idx % cols].set_visible(False)

    plt.suptitle(f"Time-series — {directory}", fontsize=11)
    plt.tight_layout()
    plt.show()


def plot_label_overlay(data: dict, features: list[str]) -> None:
    """Overlay all labels on the same axes per feature."""
    print("\nPlotting per-feature label overlays...")
    n = len(features)
    if n == 0:
        return

    colors = cm.tab10.colors
    label_list = sorted(data.keys())
    cols = min(4, n)
    rows = (n + cols - 1) // cols
    fig, axes = plt.subplots(rows, cols, figsize=(cols * 4.5, rows * 3), squeeze=False)

    for feat_idx, feat in enumerate(features):
        ax = axes[feat_idx // cols][feat_idx % cols]
        for lbl_idx, label in enumerate(label_list):
            color = colors[lbl_idx % len(colors)]
            for df in data[label]:
                if feat in df.columns:
                    ax.plot(df[feat].values, color=color, alpha=0.6, linewidth=1,
                            label=label if df is data[label][0] else "")
        ax.set_title(feat, fontsize=9)
        ax.legend(fontsize=7)

    for idx in range(n, rows * cols):
        axes[idx // cols][idx % cols].set_visible(False)

    plt.suptitle("Feature Overlay by Label", fontsize=11)
    plt.tight_layout()
    plt.show()


def plot_boxplots(data: dict, features: list[str]) -> None:
    """Box-plot distribution of each feature per label."""
    print("\nPlotting box plots...")
    label_list = sorted(data.keys())
    n = len(features)
    if n == 0:
        return

    cols = min(4, n)
    rows = (n + cols - 1) // cols
    fig, axes = plt.subplots(rows, cols, figsize=(cols * 4, rows * 3.5), squeeze=False)

    for feat_idx, feat in enumerate(features):
        ax = axes[feat_idx // cols][feat_idx % cols]
        box_data = []
        for label in label_list:
            combined = pd.concat(data[label], ignore_index=True)
            if feat in combined.columns:
                box_data.append(combined[feat].dropna().values)
            else:
                box_data.append([])
        bp = ax.boxplot(box_data, patch_artist=True, labels=label_list)
        colors = cm.Pastel1.colors
        for patch, color in zip(bp["boxes"], colors):
            patch.set_facecolor(color)
        ax.set_title(feat, fontsize=9)
        ax.set_xticklabels(label_list, rotation=20, ha="right", fontsize=8)

    for idx in range(n, rows * cols):
        axes[idx // cols][idx % cols].set_visible(False)

    plt.suptitle("Feature Distribution by Label", fontsize=11)
    plt.tight_layout()
    plt.show()


# ─── Main ─────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Visualise E-Nose raw or scaled CSV datasets.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-d", "--directory",
        default="datasets/raw",
        help="Directory containing labelled CSV files",
    )
    parser.add_argument(
        "--feature", "-f",
        nargs="+",
        default=None,
        help="Features to plot (default: all numeric columns except timestamp)",
    )
    parser.add_argument(
        "--no-timeseries",
        action="store_true",
        help="Skip individual file time-series plots",
    )
    parser.add_argument(
        "--no-overlay",
        action="store_true",
        help="Skip per-feature label overlay plots",
    )
    parser.add_argument(
        "--no-boxplot",
        action="store_true",
        help="Skip box plots",
    )
    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        print(f"ERROR: Directory not found: '{args.directory}'")
        sys.exit(1)

    data = load_directory(args.directory)

    # Determine features to plot
    all_dfs = [df for frames in data.values() for df in frames]
    all_cols = list(all_dfs[0].columns) if all_dfs else []
    numeric_cols = [c for c in all_cols if c != "timestamp"]

    features = args.feature if args.feature else numeric_cols
    missing = [f for f in features if f not in all_cols]
    if missing:
        print(f"Warning: requested features not found in data: {missing}")
        features = [f for f in features if f in all_cols]

    print(f"\nPlotting features: {features}")

    if not args.no_timeseries:
        plot_timeseries(data, features, args.directory)

    if not args.no_overlay:
        plot_label_overlay(data, features)

    if not args.no_boxplot:
        plot_boxplots(data, features)


if __name__ == "__main__":
    main()
