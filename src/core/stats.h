// stats.h
// Basic descriptive statistics computed over a sliding window.

#ifndef STATS_H
#define STATS_H

#include "core/window.h"

// Arithmetic mean of all samples currently held in the window.
float stats_mean(const SlidingWindow *window);

// Population standard deviation of all samples currently held in the
// window, given a pre-computed mean (avoids recomputing it twice).
float stats_stddev(const SlidingWindow *window, float mean);

#endif // STATS_H