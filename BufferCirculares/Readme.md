# PracticaInterrupciones — Interrupciones Externas y Buffers Circulares

**Materia:** Sistemas Programables  
**Alumno:** Morales Ramirez Jonathan Alexis  
**Fecha:** Septiembre 2026

---

## 📋 Descripción del Proyecto

Esta práctica implementa un sistema de detección de eventos asíncronos en Arduino UNO R4 WiFi usando **interrupciones externas** y un **buffer circular (ring buffer)**. Un pulsador simula el sensor de una banda transportadora industrial que detecta piezas en cualquier momento, mientras el sistema realiza simultáneamente una animación en la matriz de LEDs integrada.

---

## 🎯 Objetivos

- Configurar y usar una interrupción externa para detectar eventos asíncronos.
- Implementar un buffer circular para desacoplar la detección del procesamiento.
- Aplicar un filtro de rebote (debounce) dentro de la ISR.
- Demostrar que dos tareas pueden coexistir sin bloquearse mutuamente.

---

## 🛠️ Herramientas y Componentes

| Componente | Descripción | Cantidad |
|---|---|---|
| Arduino UNO R4 WiFi | Microcontrolador con matriz LED integrada | 1 |
| Pulsador táctil | Simula el sensor de piezas | 1 |
| Resistencia 10 kΩ | Pull-down para el pin de interrupción | 1 |
| Cables jumper | Conexiones en protoboard | varios |
| Protoboard | Plataforma de prototipado | 1 |
| Arduino IDE 2.x | Entorno de desarrollo y Monitor Serial | — |

---

## 📁 Estructura del Repositorio

---

## ⚙️ ¿Cómo funciona?

### Interrupciones externas
El pulsador está conectado a un pin de interrupción externa. Al presionarlo, el Arduino pausa momentáneamente lo que esté haciendo y salta a la ISR (Interrupt Service Routine), que anota el evento en el buffer circular y regresa de inmediato.

### Buffer circular
Es una estructura de tamaño fijo con dos índices: uno de escritura (ISR) y uno de lectura (loop()). Permite que ambas partes operen a ritmos distintos sin bloquearse. Cuando el loop() tiene un momento libre, lee los eventos pendientes y los muestra en el Monitor Serial.

### Filtro de rebote
Dentro de la ISR se verifica que hayan pasado al menos 50 ms desde el último evento registrado, evitando duplicados por rebote mecánico del pulsador.

---

## 📸 Evidencia del circuito

### Vista general del montaje
![Vista general](Diagrama/ImageInterrupciones1.jpeg)

### Detalle del pulsador en protoboard
![Detalle pulsador](Diagrama/ImageInterrupciones2.jpeg)

### Circuito completo en prueba
![Circuito completo](Diagrama/ImageInterrupciones3.jpeg)

### Circuito funcionando con LED activo
![Circuito funcionando](Diagrama/Image4Interrupciones.jpeg)

### Monitor Serial — detección de piezas
![Monitor Serial](Diagrama/Image5Interrupciones.jpeg)

---

## 📊 Resultados

| Aspecto evaluado | Resultado |
|---|---|
| Detección sin pérdidas | 100% de eventos registrados |
| Filtro de rebote | Sin duplicados detectados |
| Animación continua | Sin pausas visibles |
| Pulsaciones rápidas (< 150 ms) | Registradas correctamente |
| Reset del buffer | Reinicio limpio confirmado |

---

## ✅ Conclusión

La combinación de interrupciones externas con buffer circular demostró ser una solución confiable para detectar eventos asíncronos sin perder ninguno, mientras el sistema mantiene otra tarea ejecutándose en paralelo sin interrupciones perceptibles.
