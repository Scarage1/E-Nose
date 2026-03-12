/**
 * @file    wio_terminal_odor_data_collection.ino
 * @brief   Odor data collection for the E-Nose project.
 *
 * Press the 5-way joystick centre button to trigger one 2-second recording
 * session (8 samples at 4 Hz). Sensor data is printed over USB serial as CSV.
 * Use scripts/serial_data_collect_csv.py to save the data into labelled files.
 *
 * WARNING: Allow at least 10–15 minutes of sensor preheat time before
 *          collecting data. The datasheet recommends 24 hours for full
 *          accuracy, but relative readings stabilise sooner.
 *
 * Sensors (all connected via Grove I2C Hub):
 *   - Grove Multichannel Gas Sensor v2 (GM102B, GM302B, GM502B, GM702B)
 *     https://wiki.seeedstudio.com/Grove-Multichannel-Gas-Sensor-V2/
 *   - Grove BME680 (Temperature, Humidity, Pressure)
 *     https://wiki.seeedstudio.com/Grove-Temperature_Humidity_Pressure_Gas_Sensor_BME680/
 *   - Grove SGP30 (VOC, eCO2)
 *     https://wiki.seeedstudio.com/Grove-VOC_and_eCO2_Gas_Sensor-SGP30/
 *
 * Required libraries (install as .ZIP):
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

// ─── Settings ────────────────────────────────────────────────────────────────
#define BTN_START           0                         ///< 1 = button-triggered, 0 = free-running
#define BTN_PIN             WIO_5S_PRESS              ///< 5-way joystick centre button
#define SAMPLING_FREQ_HZ    4                         ///< Sampling frequency in Hz
#define SAMPLING_PERIOD_MS  (1000 / SAMPLING_FREQ_HZ) ///< Derived sampling period
#define NUM_SAMPLES         8                         ///< Samples per recording (2 s at 4 Hz)

// ─── Hardware constants ───────────────────────────────────────────────────────
#define BME680_I2C_ADDR     uint8_t(0x76)             ///< Default I2C address of BME680
#define PA_IN_KPA           1000.0f                   ///< Conversion: Pa → kPa

// ─── Global sensor objects ────────────────────────────────────────────────────
GAS_GMXXX<TwoWire> gas;         ///< Multichannel gas sensor v2
Seeed_BME680 bme680(BME680_I2C_ADDR);  ///< Environmental sensor

// ─────────────────────────────────────────────────────────────────────────────

void setup() {
  int16_t  sgp_err;
  uint16_t sgp_eth, sgp_h2;

  // Configure button
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Initialise serial port
  Serial.begin(115200);
  while (!Serial);  // Wait for CDC connection (optional, remove for standalone)

  Serial.println("E-Nose: Odor Data Collection");
  Serial.println("Initialising sensors...");

  // Initialise Multichannel Gas Sensor
  gas.begin(Wire, 0x08);

  // Initialise BME680
  while (!bme680.init()) {
    Serial.println("  BME680: waiting...");
    delay(1000);
  }
  Serial.println("  BME680: OK");

  // Initialise SGP30
  while (sgp_probe() != STATUS_OK) {
    Serial.println("  SGP30: waiting...");
    delay(1000);
  }
  // Discard initial unstable reading
  sgp_err = sgp_measure_signals_blocking_read(&sgp_eth, &sgp_h2);
  if (sgp_err != STATUS_OK) {
    Serial.println("ERROR: SGP30 initial read failed. Halting.");
    while (1);
  }
  Serial.println("  SGP30: OK");

  Serial.println();
  Serial.println("Ready. Press the 5-way centre button to record a sample.");
}

// ─────────────────────────────────────────────────────────────────────────────

void loop() {
  float    gm_no2_v, gm_eth_v, gm_voc_v, gm_co_v;
  int16_t  sgp_err;
  uint16_t sgp_tvoc, sgp_co2;
  unsigned long timestamp;

  // ── Wait for trigger ──────────────────────────────────────────────────────
#if BTN_START
  while (digitalRead(BTN_PIN) == HIGH);   // Wait for button press (active low)
#endif

  // ── Print CSV header ──────────────────────────────────────────────────────
  Serial.println("timestamp,temp,humd,pres,co2,voc1,voc2,no2,eth,co");

  // ── Collect NUM_SAMPLES rows ──────────────────────────────────────────────
  for (int i = 0; i < NUM_SAMPLES; i++) {

    timestamp = millis();

    // Read Multichannel Gas Sensor v2
    gm_no2_v = gas.calcVol(gas.getGM102B());
    gm_eth_v = gas.calcVol(gas.getGM302B());
    gm_voc_v = gas.calcVol(gas.getGM502B());
    gm_co_v  = gas.calcVol(gas.getGM702B());

    // Read BME680
    if (bme680.read_sensor_data()) {
      Serial.println("ERROR: BME680 read failed.");
      return;
    }

    // Read SGP30
    sgp_err = sgp_measure_iaq_blocking_read(&sgp_tvoc, &sgp_co2);
    if (sgp_err != STATUS_OK) {
      Serial.println("ERROR: SGP30 read failed.");
      return;
    }

    // ── Print one CSV row ────────────────────────────────────────────────
    Serial.print(timestamp);                                          Serial.print(',');
    Serial.print(bme680.sensor_result_value.temperature, 2);         Serial.print(',');
    Serial.print(bme680.sensor_result_value.humidity, 2);            Serial.print(',');
    Serial.print(bme680.sensor_result_value.pressure / PA_IN_KPA, 2);Serial.print(',');
    Serial.print(sgp_co2);                                            Serial.print(',');
    Serial.print(sgp_tvoc);                                           Serial.print(',');
    Serial.print(gm_voc_v, 4);                                        Serial.print(',');
    Serial.print(gm_no2_v, 4);                                        Serial.print(',');
    Serial.print(gm_eth_v, 4);                                        Serial.print(',');
    Serial.println(gm_co_v, 4);

    // Hold until next sampling period
    while (millis() < timestamp + SAMPLING_PERIOD_MS);
  }

  // ── Empty line signals end-of-sample to collection script ────────────────
  Serial.println();

  // ── Wait for button release before allowing next trigger ─────────────────
#if BTN_START
  while (digitalRead(BTN_PIN) == LOW);
  delay(50);  // Debounce
#endif
}
