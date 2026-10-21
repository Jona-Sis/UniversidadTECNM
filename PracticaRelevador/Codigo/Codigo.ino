/*
  Práctica 4.1.1-A: Driver de software para un relevador
  Arduino UNO R4 WiFi

  - Motor (canal del L298N, IN1) en D7  -> activo en ALTO
  - LED "al revés" (imita optoacoplador) en D6 -> activo en BAJO

  Comandos por Monitor Serie (115200 baudios):
    M1 / M0 -> encender / apagar motor
    L1 / L0 -> encender / apagar LED
    E       -> mostrar estado de ambos
*/

// ===================== Clase Relevador =====================
class Relevador {
  private:
    uint8_t pin;                 // Pin que controla
    bool activoEnBajo;           // true = se activa con LOW
    bool encendido;              // Estado actual
    unsigned long ultimoCambio;  // Momento del último cambio (millis)
    static const unsigned long TIEMPO_MINIMO = 1000;  // ms entre cambios

    // Traduce "encendido/apagado" al nivel eléctrico correcto
    void escribirPin(bool estado) {
      bool nivel = activoEnBajo ? !estado : estado;
      digitalWrite(pin, nivel ? HIGH : LOW);
    }

    // Regla física: no conmutar demasiado rápido.
    // La resta con unsigned long funciona aunque millis() se desborde.
    bool puedeCambiar() const {
      return (millis() - ultimoCambio) >= TIEMPO_MINIMO;
    }

    // Cambio común para encender() y apagar()
    bool cambiarA(bool nuevoEstado) {
      if (encendido == nuevoEstado) return true;  // Ya estaba así: nada que mover
      if (!puedeCambiar()) return false;          // Rechazada: el pin NO cambia
      encendido = nuevoEstado;
      escribirPin(encendido);
      ultimoCambio = millis();
      return true;
    }

  public:
    Relevador(uint8_t pinControl, bool esActivoEnBajo)
      : pin(pinControl), activoEnBajo(esActivoEnBajo),
        encendido(false), ultimoCambio(0) {}

    // Configura el pin y garantiza el estado seguro: APAGADO
    void iniciar() {
      encendido = false;
      escribirPin(false);   // Nivel de "apagado" antes de habilitar la salida
      pinMode(pin, OUTPUT);
      escribirPin(false);   // Se reafirma ya como salida
      // Cuenta el arranque como un cambio: tras un RESET hay que esperar 1 s
      // antes de volver a encender (protege los contactos de un ciclo rápido).
      ultimoCambio = millis();
    }

    bool encender() { return cambiarA(true); }
    bool apagar()   { return cambiarA(false); }
    bool estaEncendido() const { return encendido; }
};

// ===================== Objetos =====================
Relevador motor(7, false);  // D7, activo en ALTO
Relevador led(6, true);     // D6, activo en BAJO

// ===================== Lectura de comandos (sin bloquear) =====================
String bufferCmd = "";
unsigned long ultimoCaracter = 0;
const unsigned long FIN_CMD_MS = 50;  // Si no llega "fin de línea", se procesa tras 50 ms

void mostrarEstado() {
  Serial.print("Motor: ");
  Serial.println(motor.estaEncendido() ? "ENCENDIDO" : "APAGADO");
  Serial.print("LED:   ");
  Serial.println(led.estaEncendido() ? "ENCENDIDO" : "APAGADO");
}

void ordenar(Relevador &r, const char *nombre, bool encender) {
  bool yaEstaba = (r.estaEncendido() == encender);
  bool ok = encender ? r.encender() : r.apagar();

  if (!ok) {
    Serial.println("Orden rechazada: espere 1 s entre cambios");
    return;
  }
  Serial.print(nombre);
  Serial.print(yaEstaba ? " ya estaba " : " ");
  Serial.println(encender ? "ENCENDIDO" : "APAGADO");
}

void procesarComando(String cmd) {
  cmd.trim();
  cmd.toUpperCase();
  if (cmd.length() == 0) return;

  if      (cmd == "M1") ordenar(motor, "Motor", true);
  else if (cmd == "M0") ordenar(motor, "Motor", false);
  else if (cmd == "L1") ordenar(led, "LED", true);
  else if (cmd == "L0") ordenar(led, "LED", false);
  else if (cmd == "E")  mostrarEstado();
  else {
    Serial.print("Comando desconocido: ");
    Serial.println(cmd);
    Serial.println("Use M1, M0, L1, L0 o E");
  }
}

void leerSerie() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      procesarComando(bufferCmd);
      bufferCmd = "";
    } else if (bufferCmd.length() < 10) {
      bufferCmd += c;
    }
    ultimoCaracter = millis();
  }
  // Por si el Monitor Serie está en "Sin ajuste de línea"
  if (bufferCmd.length() > 0 && millis() - ultimoCaracter >= FIN_CMD_MS) {
    procesarComando(bufferCmd);
    bufferCmd = "";
  }
}

// ===================== Programa principal =====================
void setup() {
  // Lo primero: actuadores en estado seguro
  motor.iniciar();
  led.iniciar();

  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 2000) { }  // Espera breve al USB, sin delay()

  Serial.println("Sistema iniciado: todo APAGADO (estado seguro)");
  Serial.println("Comandos: M1, M0, L1, L0, E");
}

void loop() {
  leerSerie();
}
