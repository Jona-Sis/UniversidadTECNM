#include <Wire.h>
#include <Servo.h>
Servo miServo;
volatile byte angulo = 90;
volatile bool hayDato = false;

void setup() {
  miServo.attach(9);
  miServo.write(90);
  Wire.begin(0x09);
  Wire.onReceive(recibir);
}

void loop() {
  if (hayDato) {
    hayDato = false;
    miServo.write(angulo);
  }
}

void recibir(int cuantos) {
  while (Wire.available()) {
    int a = Wire.read();
    if (a > 180) a = 180;
    if (a < 0) a = 0;
    angulo = a;
    hayDato = true;
  }
}
