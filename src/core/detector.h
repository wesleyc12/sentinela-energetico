// detector.h
// Two anomaly-decision strategies over the same sliding window data:
// a fixed threshold and an adaptive (z-score based) threshold.

#ifndef DETECTOR_H
#define DETECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "core/window.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIXED_THRESHOLD 570.0f
#define K_SIGMA 3.0f

bool detector_fixed(uint16_t value);
bool detector_adaptive(const SlidingWindow *window, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif // DETECTOR_H