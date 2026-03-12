# Inference & Deployment Guide

This guide explains how to deploy the trained Edge Impulse model to the Wio Terminal for live odor classification.

---

## Prerequisites

- Completed [MODEL_TRAINING.md](MODEL_TRAINING.md) — Edge Impulse Arduino library downloaded & installed.
- Noted the **mins** and **ranges** preprocessing arrays from the dataset curation step.

---

## 1 — Configure the Live Inference Sketch

Open `firmware/inference/live/wio_terminal_artificial_nose_live_inference/wio_terminal_artificial_nose_live_inference.ino` in Arduino IDE.

### 1a — Update the library include

Change the `#include` line to match **your** Edge Impulse library name:

```cpp
// Replace this with your library name
#include "ei-e-nose_inferencing.h"
```

The library name appears in Arduino IDE under **Sketch → Include Library**.

### 1b — Update preprocessing constants

Replace the `mins[]` and `ranges[]` arrays with the values printed by the Colab notebook:

```cpp
// From your dataset_curation notebook output
float mins[] = {
  27.93, 36.17, 400.0, 0.0, 1.56, 0.81, 1.51, 0.61
};
float ranges[] = {
  12.54, 18.74, 56930.0, 60000.0, 1.48, 2.05, 1.6, 2.68
};
```

> ⚠️ Using someone else's constants will produce wrong predictions — your environment's temperature/humidity will differ.

### 1c — (Optional) Tune the anomaly threshold

```cpp
#define ANOMALY_THRESHOLD   0.3   // Increase to be less sensitive to unknowns
```

---

## 2 — Upload to Wio Terminal

1. Select **Tools → Board → Seeeduino Wio Terminal**.
2. Select the correct **Port**.
3. Click **Upload** (Ctrl + U).

---

## 3 — Using the Device

### Sensor Preheat

> ⚠️ Allow at least **10–15 minutes** of preheat time after powering on.

### LCD Display

Once running, the Wio Terminal LCD shows:
- **Predicted odor label** (e.g., `coffee`)
- **Confidence score** (0.0 – 1.0)
- **`Unknown`** when anomaly score exceeds `ANOMALY_THRESHOLD`

### Serial Monitor (115200 baud)

Open the Serial Monitor to see detailed output:

```
Predictions (DSP: 12 ms., Classification: 4 ms., Anomaly: 1 ms.)
    coffee: 0.921
    tea: 0.043
    spirits: 0.036
    anomaly score: 0.012
```

---

## 4 — Static Inference (Testing Only)

The static inference sketch (`firmware/inference/static/`) is used for quick validation — it runs inference on a hard-coded data sample without any sensors connected.

1. Copy **Raw features** from a sample in Edge Impulse's Live Classification tab.
2. Paste them into the `raw_buf[]` array in the static sketch.
3. Upload and check the Serial Monitor for predictions.

---

## 5 — Troubleshooting

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| LCD always shows `Unknown` | Anomaly threshold too low, or sensors not preheated | Increase `ANOMALY_THRESHOLD` or wait longer |
| Compile error: header not found | Library name mismatch | Check installed library name |
| All predictions near 0.33 | Wrong `mins`/`ranges` values | Re-run Colab notebook for your dataset |
| Sensor reads 0 | I²C connection issue | Check Grove cable connections |
| BME680 init loop | Wrong I²C address | Confirm `BME680_I2C_ADDR` is `0x76` |

---

## 6 — Performance Reference

| Metric | Value |
|--------|-------|
| DSP (Flatten) time | ~12 ms |
| NN inference time | ~4 ms |
| Total latency | ~2.0 s (dominated by sampling window) |
| Flash usage | ~250 KB (varies by model) |
| RAM usage | ~90 KB |

---

## Further Improvements

- Collect data across a wider range of temperatures and humidities to improve robustness.
- Add more odor classes (e.g., specific spirits varieties, herbs, cleaning products).
- Implement Bluetooth / Wi-Fi (Wio Terminal RTL8720DN) to log predictions to a dashboard.
- Perform hardware temperature/humidity compensation on the gas sensor readings.
