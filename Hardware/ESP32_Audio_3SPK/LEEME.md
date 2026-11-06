# ESP32 Audio 3SPK — esquema y PCB, revisión 0.4

Proyecto editable para KiCad 10. Incluye esquema, placa de dos caras con pistas y plano de masa, bibliotecas locales, lista de materiales, informes ERC/DRC y vistas PNG. Los tres parlantes reproducen el mismo audio mediante tres amplificadores I2S independientes; cada parlante se conecta solo a su amplificador.

## Archivo del ZIP utilizado

Se utilizó `ESP32-DEVKITC-32D-F`, porque el ZIP aportado contiene su símbolo de 38 contactos, huella `MODULE_ESP32-DEVKITC` de dos hileras THT 1×19 y modelo STEP. Es la coincidencia más cercana disponible para la placa ESP32 de 38 pines mostrada, **pero el anuncio genérico no prueba que tenga idéntico pinout ni dimensiones**. El símbolo y la huella importados están en las bibliotecas locales `Proyecto.kicad_sym` y `Proyecto_Audio.pretty`. El STEP está asociado y orientado sobre dos sockets hembra en la PCB. Su encaje visual no certifica las medidas de la placa genérica que se comprará.

Se asignaron footprints a las 31 referencias del esquema. La placa de 102 × 100 mm contiene las 31 huellas, cuatro agujeros M3, pistas de señal/alimentación, plano de masa posterior y una zona sin cobre debajo de la ubicación prevista de antena. El proyecto es un **prototipo de ingeniería**, pendiente de comprobación física y de alimentación; no se entregan Gerbers para fabricación.

## Abrir en KiCad

Extraer el ZIP completo sin separar sus archivos. Abrir `ESP32_Audio_3SPK.kicad_pro` en KiCad 10; desde el proyecto abrir `.kicad_sch` y `.kicad_pcb`. Las tablas `sym-lib-table` y `fp-lib-table` usan `${KIPRJMOD}` para hallar las bibliotecas locales. Si se cambia de carpeta, mantener juntos proyecto, tablas y `Proyecto_Audio.pretty`.

## Arquitectura y montaje previsto

La fuente entra por J1, pasa por F1, interruptor externo J2 y protección de polaridad Q1. JP1 entrega 5 V a la placa ESP32. GPIO26, GPIO25 y GPIO22 llevan BCLK, WS y DATA I2S por R2–R4 de 33 Ω a U2–U4. El firmware deberá transmitir audio mono idéntico en ambos canales I2S. R5–R7 ajustan ganancia de los módulos; **no limitan la potencia de los parlantes**. Hace falta limitar y comprobar el nivel máximo por firmware.

Los módulos U2–U4 se basan en el **Adafruit 3006 MAX98357A ya ensamblado**, no en el circuito integrado desnudo. Cada huella `AMP_3006_DOCK_1x07_2WIRE` combina una hilera de socket hembra THT 1×7 y dos taladros para cables cortos de salida desde el borne del módulo. La huella del dock es provisional: hay que medir módulo, socket, altura y sujeción antes de pedir PCB. Los terminales de J3, J4 y J5 llevan las salidas puenteadas de cada amplificador; ninguno de sus polos va a GND.

J1 es una bornera THT, porque los datos del adaptador (barril «2,1–2,5 mm») no identifican un jack de PCB. Se necesita una hembra de barril **externa** compatible con el enchufe real: centro positivo a J1.1 y exterior negativo a J1.2, comprobados con multímetro. J2 recibe los dos cables del interruptor externo y corta solo el positivo.

La placa ESP32 se monta mediante **dos sockets hembra THT 1×19**. El pin 19 del modelo del ZIP va a `+5V_ESP`, el pin 1 a `+3V3` (punto de prueba), y los pines 14/32/38 a GND. Hay que comparar uno por uno esos números con la serigrafía y documentación de la placa ESP32 real. No enchufar una placa genérica solo porque también tenga 38 pines.

## Límites eléctricos que quedan por cerrar

- El adaptador anunciado es 5 V / 1 A. Una estimación conservadora para tres canales a 1 W más ESP32/radio ronda 1,21 A. **No está validado a plena potencia**. Medir tensión y corriente durante reproducción y Wi-Fi/Bluetooth; considerar una fuente regulada de 5 V / 2–3 A con jack, interruptor, cables y fusible adecuados.
- El fusible T1,25 A es una selección preliminar. Revisar curva, capacidad DC, corriente de arranque y coordinación con la fuente antes de comprarlo. La PCB no protege frente a conectar 9/12 V o sobretensión sostenida.
- JP1 separa solo el positivo de 5 V de la ESP32; no aísla las líneas I2S. Hasta verificar el circuito USB de la placa concreta, programar la ESP32 desmontada de la base y no alimentar simultáneamente por USB y adaptador.
- Verificar dimensiones de **todas** las piezas físicas antes de fabricar: ESP32, sockets, módulos, borneras, portafusible, capacitores, resistencias y gabinete. Revisar también posición real de antena, acceso a USB/botones, altura, anclaje de módulos y corriente admisible de pistas con el cobre elegido.
- Wi-Fi frente a Bluetooth, formato/duración de audio y almacenamiento siguen sin definirse. No se añadió microSD ni un protocolo de aplicación.

## Comprobación CAD

KiCad 10: ERC **0 infracciones**; DRC **0 infracciones, 0 conexiones sin enrutar y 0 diferencias esquema/PCB** con zonas rellenadas. Ver `ERC.json`, `DRC.json` y `VALIDACION.md`. Esa comprobación es geométrica/eléctrica de CAD y no sustituye la verificación de componentes ni pruebas en banco.

## Referencias técnicas

- [Espressif: guía ESP32-DevKitC](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html) — pinout y advertencia de alimentar por una sola de sus entradas de alimentación.
- [Adafruit: conexiones del amplificador MAX98357A](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp/pinouts) — entradas I2S, alimentación y salidas de parlante.
- [Vishay: SUP90P06-09L](https://www.vishay.com/doc/?73010=) — MOSFET THT propuesto para protección inversa.

## Revisión final 0.4

Clearance global mínimo: 0,25 mm. Troncal de 5 V: 2,0–2,2 mm; ramal de ESP32: 1,0 mm; señales: 0,3 mm. Las derivaciones a capacitores, ganancia y puntos de prueba no transportan toda la corriente de los amplificadores. Pistas principales dimensionadas preliminarmente para 3 A con cobre exterior de 35 µm; confirmar material, contactos y temperatura en banco. Ver `VALIDACION.md`, `VERIFICACION_FINAL.json` y `ESTADO_TRABAJO.md`.
