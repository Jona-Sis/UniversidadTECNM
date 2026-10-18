#include <Wire.h>
const uint8_t DIR_LED = 0x08;
const uint8_t DIR_SERVO = 0x09;
const uint8_t DIR_POT = 0x0A;
unsigned long ultimaLectura = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000) { }
  Wire.begin();
  Serial.println("Maestro listo. Escribe 1 (LED encendido) o 0 (LED apagado)");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1' || c == '0') {
      byte orden = (c == '1') ? 1 : 0;
      Wire.beginTransmission(DIR_LED);
      Wire.write(orden);
      if (Wire.endTransmission() != 0)
        Serial.println("Esclavo 1 (LED) no responde");
    }
  }
  if (millis() - ultimaLectura >= 500) {
    ultimaLectura = millis();
    if (Wire.requestFrom(DIR_POT, 2) == 2) {
      byte alto = Wire.read();
      byte bajo = Wire.read();
      int valor = (alto << 8) | bajo;
      byte angulo = map(valor, 0, 1023, 0, 180);
      Wire.beginTransmission(DIR_SERVO);
      Wire.write(angulo);
      if (Wire.endTransmission() != 0)
        Serial.println("Esclavo 2 (servo) no responde");
      Serial.print("Potenciometro: ");
      Serial.print(valor);
      Serial.print(" -> servo a ");
      Serial.print(angulo);
      Serial.println(" grados");
    } else {
      Serial.println("Esclavo 3 (potenciometro) no responde");
    }
  }
}
