// sensor_hal.h
// Public interface for the sensor Hardware Abstraction Layer (HAL).
// Code above this layer talks ONLY to these two functions, never to
// hardware registers or simulation internals directly. This is what
// lets the sensor implementation be swapped later (simulated NTC now,
// real NTC next, ACS712 even later) without touching anything else.

#ifndef SENSOR_HAL_H
#define SENSOR_HAL_H

#include <stdint.h>

// Must be called once, before any call to sensor_hal_read().
void sensor_hal_init(void);

// Returns one raw sample from the sensor.
// Range (0-1023) mirrors Arduino's analogRead() output for a 10-bit ADC.
uint16_t sensor_hal_read(void);

#endif // SENSOR_HAL_H