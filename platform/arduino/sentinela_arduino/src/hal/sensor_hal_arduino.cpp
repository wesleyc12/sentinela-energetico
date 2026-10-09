// sensor_hal_arduino.cpp
// Real hardware implementation of the sensor HAL: reads the NTC
// voltage divider on pin A0. Written as .cpp (not .c) because it
// needs Arduino's analogRead(), a C++-context API. The extern "C"
// block keeps it callable from the plain-C core modules, exactly
// like the simulated version was.

#include <Arduino.h>

extern "C" {
  #include "sensor_hal.h"
}

#define NTC_PIN A0

void sensor_hal_init(void) {
  // Nothing extra needed: analogRead() works out of the box on Uno.
}

uint16_t sensor_hal_read(void) {
  return (uint16_t)analogRead(NTC_PIN);
}