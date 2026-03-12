# Hardware & Software Setup

This guide walks you through setting up all hardware components and the Arduino development environment for the E-Nose project.

---

## Prerequisites

| Tool | Version | Link |
|------|---------|------|
| Arduino IDE | 2.x (recommended) | https://www.arduino.cc/en/software |
| Python | 3.8 + | https://www.python.org/ |

---

## 1 — Hardware Assembly

### Components

| Component | Interface | I²C Address |
|-----------|-----------|-------------|
| Wio Terminal | USB / Host | — |
| BME680 Environmental Sensor | I²C | `0x76` |
| SGP30 VOC & eCO₂ Sensor | I²C | `0x58` |
| Multichannel Gas Sensor v2 | I²C | `0x08` |
| Grove I²C Hub | — | — |

### Wiring Diagram

```
Wio Terminal
└── Left Grove I²C Port
        │
   Grove I²C Hub (4× Grove ports)
    ├── Port A → BME680
    ├── Port B → SGP30
    └── Port C → Multichannel Gas Sensor v2
```

> **Tip:** The Grove I²C Hub is purely a passive splitter — no additional configuration is needed. All sensors communicate on the same I²C bus with different addresses.

---

## 2 — Arduino IDE: Board Package

1. Open Arduino IDE → **File → Preferences**.
2. In *Additional Board Manager URLs*, add:
   ```
   https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
   ```
3. Go to **Tools → Board → Boards Manager**, search **Seeed SAMD**, and install.
4. Select **Tools → Board → Seeed SAMD Boards → Seeeduino Wio Terminal**.

---

## 3 — Required Arduino Libraries

Install each library via **Sketch → Include Library → Add .ZIP Library…** or the Library Manager.

| Library | Source |
|---------|--------|
| Seeed Multichannel Gas Sensor | [GitHub ZIP](https://github.com/Seeed-Studio/Seeed_Multichannel_Gas_Sensor/archive/master.zip) |
| Seeed BME680 | [GitHub ZIP](https://github.com/Seeed-Studio/Seeed_BME680/archive/refs/heads/master.zip) |
| SGP30 Gas Sensor | [GitHub ZIP](https://github.com/Seeed-Studio/SGP30_Gas_Sensor/archive/refs/heads/master.zip) |
| TFT_eSPI | Bundled with Wio Terminal board package |

> **Note:** TFT_eSPI is required for the inference sketch (LCD display) and comes pre-configured with the Wio Terminal board package. If you see display issues, ensure the board package is up to date.

---

## 4 — Verify Hardware

1. Connect the Wio Terminal via USB.
2. Open **Tools → Port** and select the correct COM/tty port.
3. Upload the data collection sketch from `firmware/data_collection/`.
4. Open **Serial Monitor** at **115200 baud**.
5. Press the 5-way joystick **centre button** on the Wio Terminal.
6. You should see CSV rows printed:
   ```
   timestamp,temp,humd,pres,co2,voc1,voc2,no2,eth,co
   1234,27.5,45.2,101.3,420,12,1.23,0.85,1.45,0.72
   ...
   ```

If no data appears, check:
- Grove I²C Hub is firmly connected to the **left** Grove port.
- All sensor cables are fully seated.
- Correct COM port is selected.

---

## 5 — Python Environment

```bash
cd scripts/
pip install -r requirements.txt
```

Proceed to [DATA_COLLECTION.md](DATA_COLLECTION.md).
