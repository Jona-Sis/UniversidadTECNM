/*
  PRÁCTICA: Interrupciones externas + Buffer circular + Animación no bloqueante
  Arduino UNO R4 WiFi (matriz LED integrada)

  Simula una banda transportadora: cada vez que se presiona el pulsador
  (el "sensor de piezas"), se debe anotar el evento SIN perder ninguno,
  mientras el Arduino sigue animando la matriz de LEDs sin detenerse.

  Conexión del pulsador:
    - Un extremo del botón al pin 2
    - El otro extremo a GND
    - Usamos INPUT_PULLUP, así que en reposo el pin está en HIGH
      y al presionar cae a LOW -> disparamos con FALLING.
*/

#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

// ---------- Configuración del pulsador / interrupción ----------
const uint8_t PIN_BOTON = 2;
const unsigned long DEBOUNCE_US = 50000UL;   // 50 ms en microsegundos

// ---------- "Libreta circular" (buffer circular) ----------
// Guardamos el instante (millis) en que ocurrió cada evento.
const uint8_t TAM_BUFFER = 16;               // tamaño fijo de la libreta
volatile unsigned long bufferEventos[TAM_BUFFER];

// TRUCO CLAVE para no confundir "lleno" con "vacío":
// en vez de mover un puntero circular que puede dar la vuelta y coincidir
// consigo mismo, usamos dos CONTADORES que solo avanzan hacia adelante
// (nunca se resetean). La posición real en el arreglo se obtiene con % TAM_BUFFER.
//   - escritos: cuántos eventos ha anotado la interrupción en total
//   - leidos:   cuántos eventos ha procesado ya el loop()
// Así:
//   escritos == leidos           -> no hay nada nuevo que leer
//   escritos - leidos == TAM_BUFFER -> la libreta está llena (se sobrescribirá lo viejo)
volatile uint16_t escritos = 0;   // solo lo toca la interrupción
uint16_t leidos = 0;              // solo lo toca el loop()

volatile unsigned long ultimoTiempoValido = 0; // para el filtro de rebote

unsigned long piezasContadas = 0;
uint16_t overflowsDetectados = 0;

// ---------- Rutina de interrupción (ISR) ----------
// Debe ser MUY rápida: nada de Serial.print, nada de delay(), nada de cálculos pesados.
void piezaDetectada() {
  unsigned long ahora = micros();

  // Filtro de rebote adaptado a interrupciones: si no ha pasado suficiente
  // tiempo desde el último evento válido, ignoramos este "brinco" del botón.
  if (ahora - ultimoTiempoValido < DEBOUNCE_US) {
    return;
  }
  ultimoTiempoValido = ahora;

  // Anotar en el siguiente lugar libre de la libreta circular
  bufferEventos[escritos % TAM_BUFFER] = millis();
  escritos++; // avanzar el marcador de escritura (nunca se reinicia)
}

// ---------- Animación no bloqueante de la matriz ----------
// Un punto que recorre las 12 columnas, fila por fila, sin usar delay().
uint8_t frame[8][12];
uint8_t filaAnim = 0;
uint8_t colAnim = 0;
unsigned long ultimoPasoAnim = 0;
const unsigned long INTERVALO_ANIM_MS = 120;

void actualizarAnimacion() {
  unsigned long ahora = millis();
  if (ahora - ultimoPasoAnim < INTERVALO_ANIM_MS) {
    return; // aún no toca avanzar el siguiente cuadro
  }
  ultimoPasoAnim = ahora;

  // Apagar todo el cuadro
  for (uint8_t f = 0; f < 8; f++) {
    for (uint8_t c = 0; c < 12; c++) {
      frame[f][c] = 0;
    }
  }

  // Encender el pixel actual y dibujar el frame
  frame[filaAnim][colAnim] = 1;
  matrix.renderBitmap(frame, 8, 12);

  // Avanzar a la siguiente posición (serpenteando por la matriz)
  colAnim++;
  if (colAnim >= 12) {
    colAnim = 0;
    filaAnim = (filaAnim + 1) % 8;
  }
}

void setup() {
  Serial.begin(9600);
  // Espera breve opcional para ver los primeros mensajes en el monitor serie
  delay(1000);

  matrix.begin();

  pinMode(PIN_BOTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_BOTON), piezaDetectada, FALLING);

  Serial.println("Sistema listo. Presiona el pulsador para simular piezas.");
}

void loop() {
  // 1) La animación NUNCA se detiene, sin importar lo que pase con el botón.
  actualizarAnimacion();

  // 2) Leer de forma segura cuántos eventos hay pendientes.
  //    Deshabilitamos interrupciones solo un instante brevísimo para
  //    copiar el contador 'escritos' sin riesgo de que cambie a la mitad.
  noInterrupts();
  uint16_t escritosSnapshot = escritos;
  interrupts();

  // Detectar si hubo overflow (se llenó la libreta antes de leerla)
  if ((uint16_t)(escritosSnapshot - leidos) > TAM_BUFFER) {
    overflowsDetectados++;
    // Si se desbordó, saltamos leidos hasta el dato más antiguo aún válido
    leidos = escritosSnapshot - TAM_BUFFER;
    Serial.println("¡Atención! Se perdieron eventos por desbordamiento del buffer.");
  }

  // 3) Procesar uno por uno todos los eventos nuevos sin bloquear la animación.
  while (leidos != escritosSnapshot) {
    unsigned long tiempoEvento = bufferEventos[leidos % TAM_BUFFER];
    leidos++;
    piezasContadas++;

    Serial.print("Pieza #");
    Serial.print(piezasContadas);
    Serial.print(" detectada en t = ");
    Serial.print(tiempoEvento);
    Serial.println(" ms");
  }
}
