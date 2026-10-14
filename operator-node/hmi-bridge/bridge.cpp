// ============================================================================
// bridge.cpp — USB <-> Nextion UART passthrough (dev tool)
//
// Lets the laptop talk to the Nextion directly through the ESP32:
//   USB serial (115200) <-> Serial2 (GPIO16/17, 9600)
// Flash: pio run -e nextion-bridge -t upload
// ============================================================================

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
}

void loop() {
  while (Serial.available())  Serial2.write(Serial.read());
  while (Serial2.available()) Serial.write(Serial2.read());
}
