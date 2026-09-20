// fsm.c
// See fsm.h for the hysteresis rationale.

#include "app/fsm.h"

void fsm_init(Fsm *fsm) {
    fsm->state = STATE_NORMAL;
    fsm->consecutive_count = 0;
}

bool fsm_update(Fsm *fsm, bool anomaly_detected) {
    bool wants_anomalia = anomaly_detected;
    bool currently_anomalia = (fsm->state == STATE_ANOMALIA);

    if (wants_anomalia == currently_anomalia) {
        // This reading agrees with where we already are.
        // Reset any streak that was building toward the opposite state.
        fsm->consecutive_count = 0;
        return false;
    }

    // This reading disagrees with the current state: count it toward
    // a possible transition.
    fsm->consecutive_count++;

    if (fsm->consecutive_count >= FSM_HYSTERESIS_COUNT) {
        fsm->state = wants_anomalia ? STATE_ANOMALIA : STATE_NORMAL;
        fsm->consecutive_count = 0;
        return true;
    }

    return false;
}

SystemState fsm_get_state(const Fsm *fsm) {
    return fsm->state;
}