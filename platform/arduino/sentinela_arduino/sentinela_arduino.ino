// sentinela_arduino.ino
// Final Arduino firmware: full pipeline (sensor -> window -> fixed
// threshold detection -> FSM with hysteresis -> output: LED + buzzer).
// The Serial Monitor stays quiet: state ('E') and temperature ('T')
// are printed only on request. The temperature is also printed
// automatically, but only when an anomaly is detected.

extern "C" {
  #include "src/hal/sensor_hal.h"
  #include "src/core/window.h"
  #include "src/core/stats.h"
  #include "src/core/detector.h"
  #include "src/app/fsm.h"
  #include "src/output/output_hal.h"
}

#include <math.h>

#define SAMPLE_INTERVAL_MS 200

// Assumed NTC parameters, used ONLY for the temperature estimate.
// Anomaly detection never uses these: it works on the raw reading.
#define NTC_FIXED_RESISTOR 10000.0f
#define NTC_NOMINAL_R      10000.0f
#define NTC_NOMINAL_TEMP_K 298.15f
#define NTC_BETA           3950.0f

SlidingWindow window;
Fsm fsm;
uint16_t last_value = 0;

const char *state_name(SystemState state) {
  return (state == STATE_ANOMALIA) ? "ANOMALIA" : "NORMAL";
}

// Converts a raw ADC reading to degrees Celsius (Beta equation).
// Wiring assumed: NTC between 5V and A0, fixed resistor from A0 to GND.
float raw_to_celsius(uint16_t raw) {
  float r_ntc = NTC_FIXED_RESISTOR * (1023.0f / (float)raw - 1.0f);
  float inv_t = 1.0f / NTC_NOMINAL_TEMP_K + log(r_ntc / NTC_NOMINAL_R) / NTC_BETA;
  return (1.0f / inv_t) - 273.15f;
}

void print_temperature() {
  // 0 or 1023 usually means a loose wire or a missing component.
  if (last_value == 0 || last_value >= 1023) {
    Serial.println(">> Leitura fora da faixa (0 ou 1023): confira a fiacao do NTC.");
    return;
  }
  Serial.print(">> Temperatura estimada: ");
  Serial.print(raw_to_celsius(last_value), 1);
  Serial.print(" C (leitura: ");
  Serial.print(last_value);
  Serial.println(")");
}

void print_state() {
  Serial.print(">> Estado atual: ");
  Serial.println(state_name(fsm_get_state(&fsm)));
}

// Reads whatever the user typed: 'T' = temperature, 'E' = state.
void handle_serial_command() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'T' || c == 't') {
      print_temperature();
    } else if (c == 'E' || c == 'e') {
      print_state();
    }
  }
}

void setup() {
  Serial.begin(9600);

  sensor_hal_init();
  output_hal_init();
  window_init(&window);
  fsm_init(&fsm);

  Serial.println("Sentinela Energetico - Arduino build OK");
  Serial.println("Digite T (temperatura) ou E (estado) e Enter.");
}

void loop() {
  uint16_t value = sensor_hal_read();
  last_value = value;
  window_push(&window, value);

  bool fixed_flag = detector_fixed(value);
  bool transitioned = fsm_update(&fsm, fixed_flag);

  if (transitioned) {
    SystemState new_state = fsm_get_state(&fsm);
    Serial.print("TRANSICAO -> ");
    output_hal_update(new_state);

    if (new_state == STATE_ANOMALIA) {
      print_temperature();
    }
  }

  handle_serial_command();
  delay(SAMPLE_INTERVAL_MS);
}