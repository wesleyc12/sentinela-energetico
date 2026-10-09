// window.h
// Fixed-size circular buffer holding the most recent N sensor samples
// (the "sliding window"). Statistics modules read from this instead
// of talking to the sensor HAL directly.

#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WINDOW_SIZE 20

typedef struct {
    uint16_t samples[WINDOW_SIZE];
    int write_index;
    int count;
} SlidingWindow;

void window_init(SlidingWindow *window);
void window_push(SlidingWindow *window, uint16_t value);
bool window_is_full(const SlidingWindow *window);
int window_count(const SlidingWindow *window);
uint16_t window_get(const SlidingWindow *window, int index_from_oldest);

#ifdef __cplusplus
}
#endif

#endif // WINDOW_H