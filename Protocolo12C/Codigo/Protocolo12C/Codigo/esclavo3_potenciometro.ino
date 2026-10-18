#include <Wire.h>

void setup() {
  Wire.begin(0x0A);
  Wire.onRequest(enviar);
}

void loop() { }

void enviar() {
  int valor = analogRead(A0);
  Wire.write(highByte(valor));
  Wire.write(lowByte(valor));
}
