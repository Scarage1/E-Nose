# Data Collection Guide

This guide explains how to collect labeled odor samples from the E-Nose sensors.

---

## Important: Sensor Preheat

> ⚠️ **The gas sensors require a preheat period before their readings stabilize.**
>
> - Datasheet recommendation: **24 hours**
> - Practical minimum for relative classification: **10–15 minutes**
>
> Always preheat sensors before collecting any data. Collect all labels in the same session whenever possible to minimise environmental drift.

---

## 1 — Upload the Data Collection Firmware

1. Open `firmware/data_collection/wio_terminal_odor_data_collection/wio_terminal_odor_data_collection.ino` in Arduino IDE.
2. Upload to the Wio Terminal.
3. **Close** the Serial Monitor — the Python collection script will open the port.

---

## 2 — Understand the CSV Format

Each sample consists of **8 readings** captured at **4 Hz over 2 seconds**. The header row is:

```
timestamp,temp,humd,pres,co2,voc1,voc2,no2,eth,co
```

| Column | Sensor | Unit | Notes |
|--------|--------|------|-------|
| `timestamp` | Internal | ms | Milliseconds since boot |
| `temp` | BME680 | °C | Ambient temperature |
| `humd` | BME680 | % RH | Relative humidity |
| `pres` | BME680 | kPa | Atmospheric pressure |
| `co2` | SGP30 | ppm | CO₂ equivalent |
| `voc1` | SGP30 | ppb | Total VOC |
| `voc2` | Gas v2 | V | Raw VOC voltage |
| `no2` | Gas v2 | V | Raw NO₂ voltage |
| `eth` | Gas v2 | V | Raw ethanol voltage |
| `co` | Gas v2 | V | Raw CO voltage |

A sample ends with a blank line (`\r\n\r\n`), which the collection script uses as a record terminator.

---

## 3 — Collect Samples

### Step-by-step

```bash
# From the project root
pip install -r scripts/requirements.txt

# Collect coffee samples (repeat ~10+ times)
python scripts/serial_data_collect_csv.py \
    -p <PORT> \
    -b 115200 \
    -d datasets/raw \
    -l coffee
```

Replace `<PORT>` with your serial port, for example:
- macOS / Linux: `/dev/cu.usbmodem1101` or `/dev/ttyACM0`
- Windows: `COM9`

### Workflow per label

1. Position sensors **inside / above** the odor source.
2. Run the script with the appropriate `-l <label>`.
3. Press the **5-way joystick centre button** on the Wio Terminal to trigger each sample.
4. Repeat at least **10 times** per label for a reasonable dataset (50+ is better).
5. Ctrl+C to stop the script, then switch to the next odor.

### Suggested labels

```
coffee
tea
spirits
```

> You can add any label you like. Keep label names lowercase with no spaces.

---

## 4 — Recommended Collection Tips

| Tip | Reason |
|-----|--------|
| Collect all labels in one session | Reduces temperature/humidity drift between classes |
| Keep sensor distance consistent | ~2 cm above the open container |
| Let the sensor stabilize for 30 s between samples | Allows sensors to return to baseline |
| Note the ambient temp & humidity | Useful context if you need to debug accuracy |
| Aim for ≥ 50 samples per class | Better generalisation |

---

## 5 — Dataset File Naming

The script saves files as:

```
datasets/raw/<label>.<epoch_ms>.csv
```

Example:
```
datasets/raw/coffee.1741789234000.csv
datasets/raw/coffee.1741789298000.csv
datasets/raw/tea.1741789350000.csv
```

---

## Next Step

Proceed to [MODEL_TRAINING.md](MODEL_TRAINING.md) to curate the dataset and train the model.
