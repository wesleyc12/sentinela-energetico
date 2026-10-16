// output_hal_arduino.cpp
// Real hardware implementation of the output layer: drives an LED
// and a buzzer based on the FSM state. It prints nothing: messages
// are the sketch's job, so they never mix into the CSV log.

#include <Arduino.h>

extern "C" {
  #include "output_hal.h"
}

#define LED_PIN    8
#define BUZZER_PIN 9

void output_hal_init(void) {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void output_hal_update(SystemState state) {
  if (state == STATE_ANOMALIA) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}