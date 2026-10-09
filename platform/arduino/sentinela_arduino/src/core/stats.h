// stats.h
// Basic descriptive statistics computed over a sliding window.

#ifndef STATS_H
#define STATS_H

#include "window.h"

#ifdef __cplusplus
extern "C" {
#endif

float stats_mean(const SlidingWindow *window);
float stats_stddev(const SlidingWindow *window, float mean);

#ifdef __cplusplus
}
#endif

#endif // STATS_H