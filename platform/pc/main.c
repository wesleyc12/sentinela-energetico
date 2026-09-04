// platform/pc/main.c
// Entry point for the PC simulation build.
// Module 1 test: read the simulated sensor repeatedly and print each
// value, so we can visually confirm the signal drifts slowly and
// occasionally spikes into a sustained anomaly.

#include <stdio.h>
#include "hal/sensor_hal.h"

#define TEST_SAMPLE_COUNT 500

int main(void) {
    sensor_hal_init();

    for (int i = 0; i < TEST_SAMPLE_COUNT; i++) {
        uint16_t value = sensor_hal_read();
        printf("%d,%u\n", i, value);
    }

    return 0;
}