// detector.h
// Two anomaly-decision strategies over the same sliding window data:
// a fixed threshold and an adaptive (z-score based) threshold.

#ifndef DETECTOR_H
#define DETECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "core/window.h"

// Fixed threshold: chosen from known signal behavior (normal ~200-260,
// anomaly plateau ~630). Sits comfortably between the two.
#define FIXED_THRESHOLD 400.0f

// Adaptive threshold: how many standard deviations above the window's
// own mean counts as anomalous. 3.0 is a common starting point for
// z-score outlier detection.
#define K_SIGMA 3.0f

// True if the raw value exceeds the fixed threshold.
bool detector_fixed(uint16_t value);

// True if the raw value exceeds (window mean + K_SIGMA * window stddev).
bool detector_adaptive(const SlidingWindow *window, uint16_t value);

#endif // DETECTOR_H