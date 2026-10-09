// detector.c
// Implementations of the two detection strategies declared in
// detector.h. Both are intentionally simple — the point of this
// module is to compare their behavior, not to optimize either one.

#include "detector.h"
#include "stats.h"

bool detector_fixed(uint16_t value) {
    return (float)value > FIXED_THRESHOLD;
}

bool detector_adaptive(const SlidingWindow *window, uint16_t value) {
    float mean = stats_mean(window);
    float stddev = stats_stddev(window, mean);
    float upper_limit = mean + K_SIGMA * stddev;

    return (float)value > upper_limit;
}