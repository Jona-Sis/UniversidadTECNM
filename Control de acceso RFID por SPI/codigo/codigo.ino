#include <SPI.h>
#include <MFRC522.h>

// Pines para el lector RC522
#define SS_PIN 10
#define RST_PIN 9

// Pines para los LEDs
#define LED_VERDE_PIN 2  // Acceso Concedido
#define LED_ROJO_PIN  3  // Acceso Denegado

MFRC522 rfid(SS_PIN, RST_PIN);

// =========================================================================
// UID DE TU TARJETA AUTORIZADA
// =========================================================================
const byte UID_AUTORIZADO[] = {0x73, 0x6C, 0xBD, 0x01}; 
const byte TAMANO_UID = sizeof(UID_AUTORIZADO);

// Control de tiempos no bloqueante (millis) para ambos LEDs
bool ledVerdeEncendido = false;
bool ledRojoEncendido = false;

unsigned long tiempoInicioVerde = 0;
unsigned long tiempoInicioRojo = 0;

const unsigned long DURACION_LED = 2000; // 2 segundos

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Configuración de pines de los LEDs
  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_ROJO_PIN, OUTPUT);
  digitalWrite(LED_VERDE_PIN, LOW);
  digitalWrite(LED_ROJO_PIN, LOW);

  // Inicialización de SPI y RC522
  SPI.begin();
  rfid.PCD_Init();

  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println(F("ERROR: No hay comunicacion con el lector RC522."));
  } else {
    Serial.println(F("Lector RC522 listo. Acerque una tarjeta..."));
  }
}

void loop() {
  // 1. Apagar LED Verde si ya pasaron los 2 segundos
  if (ledVerdeEncendido && (millis() - tiempoInicioVerde >= DURACION_LED)) {
    digitalWrite(LED_VERDE_PIN, LOW);
    ledVerdeEncendido = false;
  }

  // 2. Apagar LED Rojo si ya pasaron los 2 segundos
  if (ledRojoEncendido && (millis() - tiempoInicioRojo >= DURACION_LED)) {
    digitalWrite(LED_ROJO_PIN, LOW);
    ledRojoEncendido = false;
  }

  // 3. Verificar presencia de tarjeta
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // 4. Leer UID
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // 5. Imprimir UID en el Monitor Serie
  Serial.print(F("UID:"));
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print(F(" 0"));
    } else {
      Serial.print(F(" "));
    }
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // 6. Validar UID
  bool esAutorizado = true;

  if (rfid.uid.size != TAMANO_UID) {
    esAutorizado = false;
  } else {
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] != UID_AUTORIZADO[i]) {
        esAutorizado = false;
        break;
      }
    }
  }

  // 7. Lógica de activación de LEDs sin bloquear el sistema
  if (esAutorizado) {
    Serial.println(F("ACCESO CONCEDIDO"));
    
    digitalWrite(LED_VERDE_PIN, HIGH);
    digitalWrite(LED_ROJO_PIN, LOW); // Apaga el rojo si estaba encendido
    
    ledVerdeEncendido = true;
    ledRojoEncendido = false;
    tiempoInicioVerde = millis();
  } else {
    Serial.println(F("ACCESO DENEGADO"));
    
    digitalWrite(LED_ROJO_PIN, HIGH);
    digitalWrite(LED_VERDE_PIN, LOW); // Apaga el verde si estaba encendido
    
    ledRojoEncendido = true;
    ledVerdeEncendido = false;
    tiempoInicioRojo = millis();
  }

  // Liberar lector
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
