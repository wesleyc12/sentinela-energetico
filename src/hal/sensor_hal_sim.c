// sensor_hal_sim.c
// Simulated implementation of the sensor HAL. Generates a synthetic
// signal mimicking an NTC thermistor near a load: normal operation
// follows a mean-reverting random walk (simplified Ornstein-Uhlenbeck
// process) around a resting point, with rare sustained anomalies
// (e.g. overheating).

#include "sensor_hal.h"
#include <stdlib.h>
#include <time.h>

static float current_value;
static int anomaly_ticks_left;

#define BASELINE_MIN         200.0f
#define BASELINE_MAX         260.0f
#define DRIFT_STEP              2.0f
#define REVERSION_RATE          0.07f  // pull strength toward center each read
#define NOISE_AMPLITUDE         3.0f
#define ANOMALY_PROBABILITY     0.002f // 0.2% chance per read (portable)
#define ANOMALY_DURATION_MIN   30
#define ANOMALY_DURATION_MAX   80
#define ANOMALY_RISE          400.0f

static float random_bipolar(void) {
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

void sensor_hal_init(void) {
    srand((unsigned int)time(NULL));
    current_value = (BASELINE_MIN + BASELINE_MAX) / 2.0f;
    anomaly_ticks_left = 0;
}

uint16_t sensor_hal_read(void) {
    if (anomaly_ticks_left == 0) {
        float center = (BASELINE_MIN + BASELINE_MAX) / 2.0f;

        // Mean-reverting step: pull toward center + random drift.
        current_value += (center - current_value) * REVERSION_RATE
                        + random_bipolar() * DRIFT_STEP;

        if (((double)rand() / ((double)RAND_MAX + 1.0)) < ANOMALY_PROBABILITY) {
            anomaly_ticks_left = ANOMALY_DURATION_MIN +
                (rand() % (ANOMALY_DURATION_MAX - ANOMALY_DURATION_MIN));
        }
    } else {
        current_value = (BASELINE_MIN + BASELINE_MAX) / 2.0f + ANOMALY_RISE;
        anomaly_ticks_left--;
    }

    float noisy_value = current_value + random_bipolar() * NOISE_AMPLITUDE;

    if (noisy_value < 0.0f) noisy_value = 0.0f;
    if (noisy_value > 1023.0f) noisy_value = 1023.0f;

    return (uint16_t)noisy_value;
}