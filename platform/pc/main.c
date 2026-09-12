// platform/pc/main.c
// Module 3 test: for each simulated reading, push it into the window
// and evaluate BOTH detection strategies (fixed and adaptive) side by
// side, along with the window's current mean and stddev, so we can
// compare their behavior directly on the same data.

#include <stdio.h>
#include "hal/sensor_hal.h"
#include "core/window.h"
#include "core/stats.h"
#include "core/detector.h"

#define TEST_SAMPLE_COUNT        500
#define MIN_SAMPLES_FOR_ADAPTIVE  10

int main(void) {
    sensor_hal_init();

    SlidingWindow window;
    window_init(&window);

    printf("index,value,mean,stddev,fixed_flag,adaptive_flag\n");

    for (int i = 0; i < TEST_SAMPLE_COUNT; i++) {
        uint16_t value = sensor_hal_read();
        window_push(&window, value);

        float mean = stats_mean(&window);
        float stddev = stats_stddev(&window, mean);

        bool fixed_flag = detector_fixed(value);
        bool adaptive_flag = false;

        if (window_count(&window) >= MIN_SAMPLES_FOR_ADAPTIVE) {
            adaptive_flag = detector_adaptive(&window, value);
        }

        printf("%d,%u,%.1f,%.1f,%d,%d\n",
               i, value, mean, stddev, fixed_flag, adaptive_flag);
    }

    return 0;
}