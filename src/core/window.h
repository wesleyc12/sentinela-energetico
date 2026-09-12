// window.h
// Fixed-size circular buffer holding the most recent N sensor samples
// (the "sliding window"). Statistics modules read from this instead
// of talking to the sensor HAL directly.

#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include <stdbool.h>

#define WINDOW_SIZE 20

typedef struct {
    uint16_t samples[WINDOW_SIZE];
    int write_index;   // position where the next sample will be written
    int count;         // valid samples held so far (caps at WINDOW_SIZE)
} SlidingWindow;

void window_init(SlidingWindow *window);
void window_push(SlidingWindow *window, uint16_t value);
bool window_is_full(const SlidingWindow *window);
int window_count(const SlidingWindow *window);

// Returns the sample at position `index_from_oldest` (0 = oldest sample
// currently held, count-1 = most recent). Caller must ensure
// index_from_oldest is within [0, window_count(window) - 1].
uint16_t window_get(const SlidingWindow *window, int index_from_oldest);

#endif // WINDOW_H