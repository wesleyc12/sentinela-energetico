// output_hal_sim.c
// PC simulation of the output layer: prints instead of driving real
// LED/buzzer hardware. Same interface the Arduino implementation will
// use later, so the application layer above never changes.

#include "output/output_hal.h"
#include <stdio.h>

void output_hal_init(void) {
    printf("[OUTPUT] Output HAL (simulated) initialized.\n");
}

void output_hal_update(SystemState state) {
    if (state == STATE_ANOMALIA) {
        printf("[OUTPUT] LED ON, BUZZER BEEP -- ANOMALIA detected.\n");
    } else {
        printf("[OUTPUT] LED OFF -- back to NORMAL.\n");
    }
}