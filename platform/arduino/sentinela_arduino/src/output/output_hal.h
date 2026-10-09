// output_hal.h
// Interface for reacting to state changes (LED, buzzer, serial log).
// Called only when the FSM actually transitions, not on every read.

#ifndef OUTPUT_HAL_H
#define OUTPUT_HAL_H

#include "../app/fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

void output_hal_init(void);
void output_hal_update(SystemState state);

#ifdef __cplusplus
}
#endif

#endif // OUTPUT_HAL_H