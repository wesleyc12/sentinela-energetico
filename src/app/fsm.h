// fsm.h
// Two-state machine (NORMAL / ANOMALIA) with hysteresis: a state
// change only happens after FSM_HYSTERESIS_COUNT consecutive readings
// agree, in either direction — this absorbs isolated noisy samples
// without needing a full statistical filter.

#ifndef FSM_H
#define FSM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_NORMAL,
    STATE_ANOMALIA
} SystemState;

typedef struct {
    SystemState state;
    int consecutive_count;
} Fsm;

#define FSM_HYSTERESIS_COUNT 3

void fsm_init(Fsm *fsm);

// Feeds one new detection result into the FSM.
// Returns true if the state changed as a result of this call.
bool fsm_update(Fsm *fsm, bool anomaly_detected);

SystemState fsm_get_state(const Fsm *fsm);

#ifdef __cplusplus
}
#endif

#endif // FSM_H