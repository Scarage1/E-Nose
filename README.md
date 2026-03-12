#  E-Nose — AI-Powered Artificial Nose

> A TinyML-powered electronic nose that fuses readings from multiple gas sensors on a **Seeed Wio Terminal** to classify odors in real time using a neural network trained with **Edge Impulse**.

[![License: 0BSD](https://img.shields.io/badge/License-0BSD-blue.svg)](https://opensource.org/licenses/0BSD)
[![Arduino](https://img.shields.io/badge/Platform-Arduino-teal.svg)](https://www.arduino.cc/)
[![Edge Impulse](https://img.shields.io/badge/ML-Edge%20Impulse-orange.svg)](https://edgeimpulse.com/)
[![Python](https://img.shields.io/badge/Python-3.8%2B-blue.svg)](https://www.python.org/)

---

## 📖 Table of Contents

- [Overview](#-overview)
- [How It Works](#-how-it-works)
- [Hardware](#-hardware)
- [Repository Structure](#-repository-structure)
- [Quickstart](#-quickstart)
- [Documentation](#-documentation)
- [Results](#-results)
- [Credits & References](#-credits--references)
- [License](#-license)

---

## 🔍 Overview

The **E-Nose** performs **sensor fusion** — combining raw readings from three different Grove gas sensors — and feeds them into a lightweight neural network to identify odors (e.g., coffee, tea, spirits). The entire ML pipeline runs on-device with no cloud dependency at inference time.

| Feature | Detail |
|---|---|
| Microcontroller | Seeed Wio Terminal (ATSAMD51 + RTL8720DN) |
| Sensors | BME680 · SGP30 · Multichannel Gas Sensor v2 |
| ML Framework | Edge Impulse (Keras classifier + K-means anomaly) |
| Inference | On-device TinyML, result shown on LCD |
| Data pipeline | Python serial collector → Google Colab curation → Edge Impulse |

---

## ⚙️ How It Works

```
┌──────────────────────────────────────────────────────────────────┐
│                         E-Nose System                            │
│                                                                  │
│  ┌──────────┐  ┌──────────┐  ┌────────────────────┐              │
│  │  BME680  │  │  SGP30   │  │ Multichannel Gas v2 │             │
│  │ Temp     │  │ CO2      │  │ VOC · NO2           │             │
│  │ Humidity │  │ TVOC     │  │ Ethanol · CO        │             │
│  └────┬─────┘  └────┬─────┘  └─────────┬──────────┘              │
│       └─────────────┴──────────────────┘                         │
│                     Grove I2C Hub                                │
│                          │                                       │
│               ┌──────────▼──────────┐                            │
│               │    Wio Terminal     │                            │
│               │  (ATSAMD51 @ 120MHz)│                            │
│               │                     │                            │
│               │  ┌───────────────┐  │                            │
│               │  │  Edge Impulse │  │                            │
│               │  │  Impulse      │  │  ──► LCD Display           │
│               │  │  (Flatten DSP │  │  ──► Serial Monitor        │
│               │  │  + NN + Anomaly) │                            │
│               │  └───────────────┘  │                            │
│               └─────────────────────┘                            │
└──────────────────────────────────────────────────────────────────┘
```

### Sensor Features (8 inputs per reading)

| # | Sensor | Measurement | Unit |
|---|--------|-------------|------|
| 1 | BME680 | Temperature | °C |
| 2 | BME680 | Humidity | % RH |
| 3 | SGP30  | CO₂ equivalent | ppm |
| 4 | SGP30  | TVOC | ppb |
| 5 | Gas v2 | VOC | V |
| 6 | Gas v2 | NO₂ | V |
| 7 | Gas v2 | Ethanol | V |
| 8 | Gas v2 | CO | V |

---

## 🔧 Hardware

### Bill of Materials

| Component | Seeed Part # | Approx. Cost |
|---|---|---|
| [Wio Terminal](https://www.seeedstudio.com/Wio-Terminal-p-4509.html) | 102991299 | $30 |
| [Grove Multichannel Gas Sensor v2](https://www.seeedstudio.com/Grove-Multichannel-Gas-Sensor-v2-p-4569.html) | 101020820 | $38 |
| [Grove SGP30 VOC & eCO₂ Sensor](https://www.seeedstudio.com/Grove-VOC-and-eCO2-Gas-Sensor-for-Arduino-SGP30.html) | 101020512 | $18 |
| [Grove BME680 Env. Sensor](https://www.seeedstudio.com/Grove-Temperature-Humidity-Pressure-and-Gas-Sensor-for-Arduino-BME680.html) | 101020513 | $23 |
| [Grove I²C Hub](https://www.seeedstudio.com/Grove-I2C-Hub.html) | 103020006 | $3 |

### Wiring

Connect all three sensors to the **Grove I²C Hub**, then plug the hub into the **Grove I²C port** on the left side of the Wio Terminal.

```
Wio Terminal (Left I²C)
        │
   Grove I²C Hub
   ┌────┼────┐
   │    │    │
BME680 SGP30 Gas v2
```

> See [docs/SETUP.md](docs/SETUP.md) for detailed wiring and Arduino IDE configuration.

---

## 📁 Repository Structure

```
E-Nose/
├── README.md                            # ← You are here
├── LICENSE
├── .gitignore
│
├── docs/
│   ├── SETUP.md                         # Hardware & Arduino IDE setup
│   ├── DATA_COLLECTION.md               # How to collect odor samples
│   ├── MODEL_TRAINING.md                # Edge Impulse training guide
│   └── INFERENCE.md                     # Deploy & run on Wio Terminal
│
├── firmware/
│   ├── data_collection/
│   │   └── wio_terminal_odor_data_collection/
│   │       └── wio_terminal_odor_data_collection.ino
│   └── inference/
│       ├── live/
│       │   └── wio_terminal_artificial_nose_live_inference/
│       │       └── wio_terminal_artificial_nose_live_inference.ino
│       └── static/
│           └── wio_terminal_artificial_nose_static_inference/
│               └── wio_terminal_artificial_nose_static_inference.ino
│
├── scripts/
│   ├── serial_data_collect_csv.py       # Captures serial CSV data to files
│   └── requirements.txt                 # Python dependencies
│
├── notebooks/
│   └── dataset_curation.ipynb           # Feature scaling (Google Colab ready)
│
└── datasets/
    └── README.md                        # Dataset instructions & download links
```

---

## 🚀 Quickstart

### 1 — Hardware Setup
Follow [docs/SETUP.md](docs/SETUP.md) to install the Arduino board package and required libraries.

### 2 — Collect Data
```bash
# Upload firmware/data_collection sketch to Wio Terminal first
pip install -r scripts/requirements.txt
python scripts/serial_data_collect_csv.py -p <PORT> -b 115200 -d datasets/raw -l coffee
```
Repeat for each odor label. See [docs/DATA_COLLECTION.md](docs/DATA_COLLECTION.md).

### 3 — Curate Dataset
Open `notebooks/dataset_curation.ipynb` in [Google Colab](https://colab.research.google.com/), upload your raw CSVs, and run all cells to produce normalized data.

### 4 — Train Model
Upload scaled CSVs to [Edge Impulse](https://edgeimpulse.com/). See [docs/MODEL_TRAINING.md](docs/MODEL_TRAINING.md) for the full impulse configuration.

### 5 — Deploy & Infer
Download the Edge Impulse Arduino library, add it to the IDE, then upload the live inference sketch. See [docs/INFERENCE.md](docs/INFERENCE.md).

---

## 📊 Results

The model was trained to distinguish between several odor categories:

| Odor | Classification Accuracy |
|------|------------------------|
| Coffee | ~85% |
| Tea | ~80% |
| Spirits (broad) | ~75% |

> **Note:** Accuracy depends heavily on sensor preheat time and environmental conditions. Collect data at consistent temperature and humidity for best results.

---

## 📚 Documentation

| Document | Description |
|---|---|
| [docs/SETUP.md](docs/SETUP.md) | Hardware wiring + Arduino IDE setup |
| [docs/DATA_COLLECTION.md](docs/DATA_COLLECTION.md) | Odor sample collection workflow |
| [docs/MODEL_TRAINING.md](docs/MODEL_TRAINING.md) | Edge Impulse training pipeline |
| [docs/INFERENCE.md](docs/INFERENCE.md) | Deploying the model for live inference |

---

## 🙏 Credits & References

- Original concept: [Benjamin Cabé's Artificial Nose](https://blog.benjamin-cabe.com/2021/08/03/how-i-built-a-connected-artificial-nose) ([GitHub](https://github.com/kartben/artificial-nose))
- Tutorial basis: [ShawnHymel — AI-Powered Artificial Nose (DigiKey Maker)](https://www.digikey.com/en/maker/projects/how-to-make-an-ai-powered-artificialnose/3fcf88a89efa47a1b231c5ad2097716a)
- Reference code: [ShawnHymel/ai-nose](https://github.com/ShawnHymel/ai-nose)
- ML platform: [Edge Impulse](https://edgeimpulse.com/)

---

## 📄 License

Unless otherwise noted, all code is licensed under the [Zero-Clause BSD License (0BSD)](https://opensource.org/licenses/0BSD).

> Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted. THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES.
