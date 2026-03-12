# Model Training Guide

This guide covers:
1. Feature scaling the raw dataset with the Colab notebook
2. Uploading data to Edge Impulse
3. Configuring and training the impulse (DSP + NN + Anomaly)

---

## 1 — Feature Scaling (Google Colab)

Raw sensor readings span very different value ranges (e.g., temperature 25–40 °C vs CO₂ 400–60000 ppm). Normalising all inputs to `[0, 1]` prevents large-magnitude features from dominating the neural network.

### Steps

1. Open the notebook:

   [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/Scarage1/E-Nose/blob/main/notebooks/dataset_curation.ipynb)

2. Create the directory `/content/dataset` in Colab and upload all raw CSV files from `datasets/raw/`.

3. Run **all cells** in order.

4. The notebook will:
   - Load and concatenate all CSVs
   - Plot scatter matrices and histograms to confirm data distributions
   - **Drop** the `pressure` column (poor discriminative power)
   - **Normalise** all remaining features (min–max to `[0, 1]`)
   - Save per-label scaled CSVs into `/content/out/`
   - Zip them as `/content/out.zip`

5. Download `out.zip` and unzip into `datasets/scaled/`.

### Preprocessing choices

| Column | Method | Reason |
|--------|--------|--------|
| `timestamp` | **Drop** | Not a predictive feature |
| `temp` | Normalise | Non-Gaussian distribution |
| `humd` | Normalise | Non-Gaussian distribution |
| `pres` | **Drop** | Low variance across odors |
| `co2` | Normalise | Wide range, non-Gaussian |
| `voc1` | Normalise | Wide range, non-Gaussian |
| `voc2` | Normalise | Voltage, bounded |
| `no2` | Normalise | Voltage, bounded |
| `eth` | Normalise | Voltage, bounded |
| `co` | Normalise | Voltage, bounded |

### Preprocessing constants (example)

After running the notebook, copy the printed **mins** and **ranges** arrays — you will need them in the inference firmware:

```
Mins:   [27.93, 36.17, 400.0, 0.0, 1.56, 0.81, 1.51, 0.61]
Ranges: [12.54, 18.74, 56930.0, 60000.0, 1.48, 2.05, 1.6, 2.68]
```

---

## 2 — Upload to Edge Impulse

1. Create a new project at [edgeimpulse.com](https://studio.edgeimpulse.com/).
2. Go to **Data Acquisition → Upload Data**.
3. Upload all CSVs from `datasets/scaled/`.
4. Select:
   - **Automatically split between Training and Test** (80/20)
   - **Infer labels from filename** (the script named files as `<label>.<ts>.csv`)

---

## 3 — Design the Impulse

Navigate to **Impulse Design** and configure:

```
Input: Time series data
  • Window size:    2000 ms
  • Window increase: 200 ms
  • Frequency:      4 Hz
  • Axes:           temp, humd, co2, voc1, voc2, no2, eth, co  (8 axes)

Processing Block: Flatten
  ✓ Mean  ✓ Minimum  ✓ Maximum  ✓ RMS
  ✗ Skewness  ✗ Kurtosis

Learning Block 1: Classification (Keras)
Learning Block 2: Anomaly Detection (K-Means)
```

Click **Save Impulse**.

---

## 4 — Generate Features (Flatten Block)

1. Go to **Flatten** under *DSP*.
2. Deselect **Skewness** and **Kurtosis**.
3. Click **Save parameters**.
4. Click **Generate features** — wait for processing to finish.
5. Review the Feature Explorer to confirm classes cluster separately.

---

## 5 — Train the Neural Network

Go to **NN Classifier** and configure:

```
Architecture:
  Dense(40)  → Dropout(0.25)
  Dense(20)  → Dropout(0.25)
  Dense(num_classes, activation=softmax)

Training settings:
  Learning rate:    0.0005
  Training cycles:  300
  Validation split: 20%
  Auto-balance:     On (if classes are imbalanced)
```

Click **Start training**. Target accuracy: > 80% on validation set.

---

## 6 — Train Anomaly Detection

Go to **Anomaly Detection**:

1. Select **RMS** features for each sensor axis (`feature 3` in the Flatten output).
2. Set **Number of clusters**: 32 (default).
3. Click **Start training**.

This allows the device to output `"Unknown"` when it detects an odor outside the trained distribution.

---

## 7 — Test the Model

Go to **Model testing → Classify all**.

Review the confusion matrix. Typical issues:
- **Spirits confusion**: different spirit types smell very similar to gas sensors.
- **Environmental drift**: if test accuracy is much lower than training accuracy, collect more data across varied temperatures/humidities.

---

## 8 — Export the Arduino Library

Go to **Deployment → Arduino library**.

1. Select **Quantised (int8)** for smallest footprint.
2. Click **Build** and download the `.zip`.
3. In Arduino IDE: **Sketch → Include Library → Add .ZIP Library…** → select the downloaded file.

The library name (e.g., `ei-e-nose_inferencing`) becomes the `#include` header in the inference sketch.

---

## Next Step

Proceed to [INFERENCE.md](INFERENCE.md) to deploy and run on the Wio Terminal.
