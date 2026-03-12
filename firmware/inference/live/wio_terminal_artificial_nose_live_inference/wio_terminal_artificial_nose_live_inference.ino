/**
 * @file    wio_terminal_artificial_nose_live_inference.ino
 * @brief   Live odor classification on the Wio Terminal using Edge Impulse.
 *
 * Continuously collects sensor data, normalises it using the preprocessing
 * constants from your dataset curation step, runs inference via the Edge
 * Impulse library, and displays the predicted odor label on the LCD.
 *
 * ── How to customise ──────────────────────────────────────────────────────────
 * 1. Change the #include below to match your Edge Impulse library name.
 * 2. Replace mins[] and ranges[] with the values printed by the dataset
 *    curation notebook (notebooks/dataset_curation.ipynb).
 * 3. Adjust ANOMALY_THRESHOLD as needed (higher = less sensitive).
 *
 * WARNING: Allow at least 10–15 minutes of sensor preheat before use.
 *
 * Sensors (all connected via Grove I2C Hub):
 *   - Grove Multichannel Gas Sensor v2
 *   - Grove BME680 (Temperature, Humidity, Pressure)
 *   - Grove SGP30 (VOC, eCO2)
 *
 * Required libraries (in addition to the Edge Impulse library):
 *   https://github.com/Seeed-Studio/Seeed_Multichannel_Gas_Sensor/archive/master.zip
 *   https://github.com/Seeed-Studio/Seeed_BME680/archive/refs/heads/master.zip
 *   https://github.com/Seeed-Studio/SGP30_Gas_Sensor/archive/refs/heads/master.zip
 *
 * Based on the work by:
 *   Benjamin Cabé  – https://github.com/kartben/artificial-nose
 *   Shawn Hymel    – https://github.com/ShawnHymel/ai-nose
 *
 * Author:  Shivam Kumar (Scarage1)
 * License: 0BSD (https://opensource.org/licenses/0BSD)
 */

#include <Wire.h>

#include "Multichannel_Gas_GMXXX.h"
#include "seeed_bme680.h"
#include "sensirion_common.h"
#include "sgp30.h"
#include "TFT_eSPI.h"               // Bundled with Wio Terminal board package

// ── TODO: Replace with your Edge Impulse library header ──────────────────────
#include "ei-e-nose_inferencing.h"  // <─── CHANGE THIS
// ─────────────────────────────────────────────────────────────────────────────

// ─── Settings ────────────────────────────────────────────────────────────────
#define DEBUG               1                         ///< 1 = print debug to serial
#define DEBUG_NN            false                     ///< Print EI internal debug
#define ANOMALY_THRESHOLD   0.3f                      ///< Anomaly score threshold
#define SAMPLING_FREQ_HZ    4                         ///< Must match Edge Impulse project
#define SAMPLING_PERIOD_MS  (1000 / SAMPLING_FREQ_HZ)
#define NUM_SAMPLES         EI_CLASSIFIER_RAW_SAMPLE_COUNT
#define READINGS_PER_SAMPLE EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME

// ─── Hardware constants ───────────────────────────────────────────────────────
#define BME680_I2C_ADDR     uint8_t(0x76)
#define PA_IN_KPA           1000.0f

// ─── Preprocessing constants ──────────────────────────────────────────────────
// TODO: Replace with values from your dataset_curation.ipynb output.
// Column order: temp, humd, co2, voc1, voc2, no2, eth, co  (pressure DROPPED)
float mins[] = {
  27.93f, 36.17f, 400.0f, 0.0f, 1.56f, 0.81f, 1.51f, 0.61f
};
float ranges[] = {
  12.54f, 18.74f, 56930.0f, 60000.0f, 1.48f, 2.05f, 1.6f, 2.68f
};

// ─── Global sensor & display objects ─────────────────────────────────────────
GAS_GMXXX<TwoWire> gas;
Seeed_BME680 bme680(BME680_I2C_ADDR);
TFT_eSPI tft;

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
  int16_t  sgp_err;
  uint16_t sgp_eth, sgp_h2;

  Serial.begin(115200);

  // ── Configure LCD ──────────────────────────────────────────────────────────
  tft.begin();
  tft.setRotation(3);
  tft.setFreeFont(&FreeSansBoldOblique24pt7b);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("E-Nose", 40, 80);
  tft.drawString("Initialising...", 10, 130);

  // ── Initialise sensors ────────────────────────────────────────────────────
  gas.begin(Wire, 0x08);

  while (!bme680.init()) {
    Serial.println("BME680: waiting...");
    delay(1000);
  }

  while (sgp_probe() != STATUS_OK) {
    Serial.println("SGP30: waiting...");
    delay(1000);
  }

  sgp_err = sgp_measure_signals_blocking_read(&sgp_eth, &sgp_h2);
  if (sgp_err != STATUS_OK) {
    Serial.println("ERROR: SGP30 initial read failed. Halting.");
    tft.fillScreen(TFT_RED);
    tft.drawString("SGP30 Error", 10, 80);
    while (1);
  }

  tft.fillScreen(TFT_BLACK);
  Serial.println("E-Nose ready.");
}

// ─────────────────────────────────────────────────────────────────────────────

void loop() {
  float    gm_no2_v, gm_eth_v, gm_voc_v, gm_co_v;
  int16_t  sgp_err;
  uint16_t sgp_tvoc, sgp_co2;
  unsigned long timestamp;

  static float raw_buf[NUM_SAMPLES * READINGS_PER_SAMPLE];
  static signal_t signal;
  float temp_val;
  int   max_idx = 0;
  float max_val = 0.0f;
  char  str_buf[40];

  // ── Collect NUM_SAMPLES, each with READINGS_PER_SAMPLE features ──────────
  for (int i = 0; i < NUM_SAMPLES; i++) {

    timestamp = millis();

    gm_no2_v = gas.calcVol(gas.getGM102B());
    gm_eth_v = gas.calcVol(gas.getGM302B());
    gm_voc_v = gas.calcVol(gas.getGM502B());
    gm_co_v  = gas.calcVol(gas.getGM702B());

    if (bme680.read_sensor_data()) {
      Serial.println("ERROR: BME680 read failed.");
      return;
    }

    sgp_err = sgp_measure_iaq_blocking_read(&sgp_tvoc, &sgp_co2);
    if (sgp_err != STATUS_OK) {
      Serial.println("ERROR: SGP30 read failed.");
      return;
    }

    // ── Store raw readings ───────────────────────────────────────────────
    raw_buf[(i * READINGS_PER_SAMPLE) + 0] = bme680.sensor_result_value.temperature;
    raw_buf[(i * READINGS_PER_SAMPLE) + 1] = bme680.sensor_result_value.humidity;
    raw_buf[(i * READINGS_PER_SAMPLE) + 2] = (float)sgp_co2;
    raw_buf[(i * READINGS_PER_SAMPLE) + 3] = (float)sgp_tvoc;
    raw_buf[(i * READINGS_PER_SAMPLE) + 4] = gm_voc_v;
    raw_buf[(i * READINGS_PER_SAMPLE) + 5] = gm_no2_v;
    raw_buf[(i * READINGS_PER_SAMPLE) + 6] = gm_eth_v;
    raw_buf[(i * READINGS_PER_SAMPLE) + 7] = gm_co_v;

    // ── Normalise each reading ───────────────────────────────────────────
    for (int j = 0; j < READINGS_PER_SAMPLE; j++) {
      temp_val = raw_buf[(i * READINGS_PER_SAMPLE) + j] - mins[j];
      raw_buf[(i * READINGS_PER_SAMPLE) + j] = temp_val / ranges[j];
    }

    while (millis() < timestamp + SAMPLING_PERIOD_MS);
  }

  // ── Optional: print normalised buffer ────────────────────────────────────
#if DEBUG
  for (int i = 0; i < NUM_SAMPLES * READINGS_PER_SAMPLE; i++) {
    Serial.print(raw_buf[i], 4);
    Serial.print(i < (NUM_SAMPLES * READINGS_PER_SAMPLE - 1) ? ", " : "\n");
  }
#endif

  // ── Build signal and run classifier ──────────────────────────────────────
  int err = numpy::signal_from_buffer(raw_buf, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    ei_printf("ERROR: signal_from_buffer failed (%d)\r\n", err);
    return;
  }

  ei_impulse_result_t result = { 0 };
  err = run_classifier(&signal, &result, DEBUG_NN);
  if (err != EI_IMPULSE_OK) {
    ei_printf("ERROR: run_classifier failed (%d)\r\n", err);
    return;
  }

  // ── Print predictions to serial ───────────────────────────────────────────
  ei_printf("Predictions (DSP: %d ms, Classification: %d ms, Anomaly: %d ms)\r\n",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    ei_printf("  %-12s %.3f\r\n",
              result.classification[i].label,
              result.classification[i].value);
  }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("  anomaly score: %.3f\r\n", result.anomaly);
#endif

  // ── Find the highest-confidence label ─────────────────────────────────────
  max_idx = 0;
  max_val = 0.0f;
  for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > max_val) {
      max_val = result.classification[i].value;
      max_idx = i;
    }
  }

  // ── Update LCD ────────────────────────────────────────────────────────────
  tft.fillScreen(TFT_BLACK);
  if (result.anomaly < ANOMALY_THRESHOLD) {
    tft.drawString(result.classification[max_idx].label, 20, 60);
    snprintf(str_buf, sizeof(str_buf), "%.3f", max_val);
    tft.drawString(str_buf, 60, 120);
  } else {
    tft.drawString("Unknown", 20, 60);
    snprintf(str_buf, sizeof(str_buf), "a: %.3f", result.anomaly);
    tft.drawString(str_buf, 20, 120);
  }
}
