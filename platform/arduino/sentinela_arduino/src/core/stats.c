// stats.c
// Straightforward two-pass computation: one pass for the mean
// (needed by the caller anyway for detector logic), one pass for
// the variance using that mean.

#include "stats.h"
#include <math.h>

float stats_mean(const SlidingWindow *window) {
    int n = window_count(window);
    if (n == 0) {
        return 0.0f;
    }

    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += (float)window_get(window, i);
    }
    return sum / (float)n;
}

float stats_stddev(const SlidingWindow *window, float mean) {
    int n = window_count(window);
    if (n == 0) {
        return 0.0f;
    }

    float sum_sq_diff = 0.0f;
    for (int i = 0; i < n; i++) {
        float diff = (float)window_get(window, i) - mean;
        sum_sq_diff += diff * diff;
    }
    return sqrtf(sum_sq_diff / (float)n);
}