// window.c
// Circular buffer implementation. Writing is O(1): the buffer never
// shifts elements, it just wraps write_index back to 0 once it hits
// WINDOW_SIZE, naturally overwriting the oldest sample.

#include "core/window.h"

void window_init(SlidingWindow *window) {
    window->write_index = 0;
    window->count = 0;
}

void window_push(SlidingWindow *window, uint16_t value) {
    window->samples[window->write_index] = value;
    window->write_index = (window->write_index + 1) % WINDOW_SIZE;

    if (window->count < WINDOW_SIZE) {
        window->count++;
    }
}

bool window_is_full(const SlidingWindow *window) {
    return window->count == WINDOW_SIZE;
}

int window_count(const SlidingWindow *window) {
    return window->count;
}

uint16_t window_get(const SlidingWindow *window, int index_from_oldest) {
    // While the window hasn't filled up yet, samples were written
    // starting at index 0 with no wraparound, so the oldest one is
    // simply at index 0. Once it's full, write_index points exactly
    // at the oldest sample (the next one about to be overwritten).
    int oldest_index = (window->count < WINDOW_SIZE) ? 0 : window->write_index;
    int actual_index = (oldest_index + index_from_oldest) % WINDOW_SIZE;
    return window->samples[actual_index];
}