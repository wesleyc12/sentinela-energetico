// platform/pc/main.c
// Module 2 test: push simulated sensor readings into the sliding
// window and periodically print its contents (oldest to newest) to
// visually confirm the circular buffer is sliding correctly.

#include <stdio.h>
#include "hal/sensor_hal.h"
#include "core/window.h"

#define TEST_SAMPLE_COUNT 60
#define PRINT_EVERY        5

static void print_window(const SlidingWindow *window) {
    printf("  window (%d samples): ", window_count(window));
    for (int i = 0; i < window_count(window); i++) {
        printf("%u ", window_get(window, i));
    }
    printf("\n");
}

int main(void) {
    sensor_hal_init();

    SlidingWindow window;
    window_init(&window);

    for (int i = 0; i < TEST_SAMPLE_COUNT; i++) {
        uint16_t value = sensor_hal_read();
        window_push(&window, value);

        if ((i + 1) % PRINT_EVERY == 0) {
            printf("read #%d, value=%u\n", i, value);
            print_window(&window);
        }
    }

    return 0;
}