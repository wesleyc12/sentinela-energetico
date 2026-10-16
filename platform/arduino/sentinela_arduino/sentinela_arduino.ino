// sentinela_arduino.ino
// Final Arduino firmware: full pipeline (sensor -> window -> fixed
// threshold detection -> FSM with hysteresis -> output: LED + buzzer).
//
// The Serial Monitor stays quiet by default. Commands (type + Enter):
//   E = print current state
//   T = print estimated temperature
//   L = toggle CSV logging (one line per sample)
//   A = set label to 1 (an anomaly is really happening)
//   N = set label to 0 (everything is normal)
// The temperature is also printed automatically when an anomaly starts.

extern "C" {
  #include "src/hal/sensor_hal.h"
  #include "src/core/window.h"
  #include "src/core/stats.h"
  #include "src/core/detector.h"
  #include "src/app/fsm.h"
  #include "src/output/output_hal.h"
}

#include <math.h>

#define SAMPLE_INTERVAL_MS       200
#define MIN_SAMPLES_FOR_ADAPTIVE 10

// Assumed NTC parameters, used ONLY for the temperature estimate.
// Anomaly detection never uses these: it works on the raw reading.
#define NTC_FIXED_RESISTOR 10000.0f
#define NTC_NOMINAL_R      10000.0f
#define NTC_NOMINAL_TEMP_K 298.15f
#define NTC_BETA           3950.0f

SlidingWindow window;
Fsm fsm;
uint16_t last_value = 0;
bool logging_enabled = false;
uint8_t current_label = 0;  // ground truth, set by the user: 0 normal, 1 anomaly

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

void print_log_header() {
  Serial.println("ms,raw,mean,stddev,fixed_flag,adaptive_flag,state,label");
}

// One CSV line per sample: the data the classifier will train on.
void print_log_line(uint16_t value, bool fixed_flag, bool adaptive_flag) {
  float mean = stats_mean(&window);
  float stddev = stats_stddev(&window, mean);

  Serial.print(millis());
  Serial.print(',');
  Serial.print(value);
  Serial.print(',');
  Serial.print(mean, 1);
  Serial.print(',');
  Serial.print(stddev, 1);
  Serial.print(',');
  Serial.print(fixed_flag ? 1 : 0);
  Serial.print(',');
  Serial.print(adaptive_flag ? 1 : 0);
  Serial.print(',');
  Serial.print(fsm_get_state(&fsm) == STATE_ANOMALIA ? 1 : 0);
  Serial.print(',');
  Serial.println(current_label);
}

// Reads whatever the user typed and runs the matching command.
void handle_serial_command() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'T' || c == 't') {
      print_temperature();
    } else if (c == 'E' || c == 'e') {
      print_state();
    } else if (c == 'L' || c == 'l') {
      logging_enabled = !logging_enabled;
      if (logging_enabled) {
        print_log_header();
      } else {
        Serial.println("# log desligado");
      }
    } else if (c == 'A' || c == 'a') {
      current_label = 1;
      Serial.println("# rotulo = 1 (anomalia)");
    } else if (c == 'N' || c == 'n') {
      current_label = 0;
      Serial.println("# rotulo = 0 (normal)");
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
  Serial.println("Comandos: E estado | T temperatura | L log CSV | A anomalia | N normal");
}

void loop() {
  uint16_t value = sensor_hal_read();
  last_value = value;
  window_push(&window, value);

  bool fixed_flag = detector_fixed(value);
  bool adaptive_flag = false;
  if (window_count(&window) >= MIN_SAMPLES_FOR_ADAPTIVE) {
    adaptive_flag = detector_adaptive(&window, value);
  }

  bool transitioned = fsm_update(&fsm, fixed_flag);

  if (transitioned) {
    SystemState new_state = fsm_get_state(&fsm);
    output_hal_update(new_state);

    // While logging, stay silent so text never mixes into the CSV.
    if (!logging_enabled) {
      if (new_state == STATE_ANOMALIA) {
        Serial.println("TRANSICAO -> ANOMALIA: LED e buzzer ligados.");
        print_temperature();
      } else {
        Serial.println("TRANSICAO -> NORMAL: LED e buzzer desligados.");
      }
    }
  }

  if (logging_enabled) {
    print_log_line(value, fixed_flag, adaptive_flag);
  }

  handle_serial_command();
  delay(SAMPLE_INTERVAL_MS);
}