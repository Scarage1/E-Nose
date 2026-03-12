/**
 * @file    wio_terminal_artificial_nose_static_inference.ino
 * @brief   Static inference test — no sensors required.
 *
 * Use this sketch to quickly verify your Edge Impulse model works before
 * connecting hardware. Copy "Raw features" from a sample in the Edge Impulse
 * Live Classification tab and paste them into raw_buf[] below.
 *
 * Steps:
 *   1. In Edge Impulse Studio, go to Live Classification.
 *   2. Open a sample and click "Load sample".
 *   3. Copy the "Raw features" array.
 *   4. Paste the values into raw_buf[] below.
 *   5. Upload and open Serial Monitor at 115200 baud.
 *
 * Author:  Shivam Kumar (Scarage1)
 * License: 0BSD (https://opensource.org/licenses/0BSD)
 */

// ── TODO: Replace with your Edge Impulse library header ──────────────────────
#include "ei-e-nose_inferencing.h"   // <─── CHANGE THIS
// ─────────────────────────────────────────────────────────────────────────────

// ── Settings ──────────────────────────────────────────────────────────────────
const bool DEBUG_NN = false;   ///< Set true to print EI internal timing

// ── TODO: Paste your "Raw features" from Edge Impulse here ───────────────────
// Example values (coffee sample from the reference dataset):
float raw_buf[] = {
  0.7706, 0.5977, 0.0771, 1.0000, 0.7297, 0.2049, 0.6497, 0.4179,
  0.7697, 0.5950, 0.0693, 0.9616, 0.7230, 0.2049, 0.6497, 0.4664,
  0.7697, 0.5939, 0.0624, 0.9041, 0.7230, 0.2049, 0.6497, 0.4963,
  0.7679, 0.5891, 0.0648, 0.8681, 0.7162, 0.2049, 0.6433, 0.4627
};
// ─────────────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("E-Nose: Static Inference Test");
}

void loop() {

  // ── Build signal from buffer ───────────────────────────────────────────────
  signal_t signal;
  int err = numpy::signal_from_buffer(raw_buf, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    ei_printf("ERROR: signal_from_buffer failed (%d)\r\n", err);
    delay(2000);
    return;
  }

  // ── Run classifier ─────────────────────────────────────────────────────────
  ei_impulse_result_t result = { 0 };
  err = run_classifier(&signal, &result, DEBUG_NN);
  if (err != EI_IMPULSE_OK) {
    ei_printf("ERROR: run_classifier failed (%d)\r\n", err);
    delay(2000);
    return;
  }

  // ── Print predictions ──────────────────────────────────────────────────────
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

  ei_printf("\r\n");
  delay(2000);
}
