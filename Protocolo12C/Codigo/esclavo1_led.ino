#include <Wire.h>
const uint8_t PIN_LED = 13;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Wire.begin(0x08);
  Wire.onReceive(recibir);
}

void loop() { }

void recibir(int cuantos) {
  while (Wire.available()) {
    byte dato = Wire.read();
    digitalWrite(PIN_LED, dato ? HIGH : LOW);
  }
}
