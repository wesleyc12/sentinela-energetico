// platform/pc/main.c
// Module 4 final test: the complete pipeline (sensor -> window ->
// fixed threshold detection -> FSM with hysteresis -> output layer).
// Only transitions are printed, driven through output_hal, not by
// main.c directly.

#include <stdio.h>
#include "hal/sensor_hal.h"
#include "core/window.h"
#include "core/stats.h"
#include "core/detector.h"
#include "app/fsm.h"
#include "output/output_hal.h"

#define TEST_SAMPLE_COUNT 1000

int main(void) {
    sensor_hal_init();
    output_hal_init();

    SlidingWindow window;
    window_init(&window);

    Fsm fsm;
    fsm_init(&fsm);

    for (int i = 0; i < TEST_SAMPLE_COUNT; i++) {
        uint16_t value = sensor_hal_read();
        window_push(&window, value);

        bool fixed_flag = detector_fixed(value);
        bool transitioned = fsm_update(&fsm, fixed_flag);

        if (transitioned) {
            printf("[%d] ", i);
            output_hal_update(fsm_get_state(&fsm));
        }
    }

    return 0;
}