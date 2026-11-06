# Validación CAD — revisión 0.4

27 de septiembre de 2026. KiCad 10.0.6. Proyecto guardado, sin reconstruir el esquema ni cambiar el footprint importado.

| Comprobación | Resultado |
|---|---|
| Edge.Cuts | Un rectángulo cerrado de 102 × 100 mm; 4 segmentos |
| Visor 3D nativo | Sin mensaje de contorno ausente/malformado |
| ERC, todas las severidades | 0 infracciones |
| DRC normal, zonas rellenadas y paridad | 0 errores, 0 warnings, 0 conexiones pendientes, 0 diferencias |
| Clearance global | 0,25 mm explícito; cobre/borde 0,5 mm |
| Huellas | 31; la ESP32 conserva la huella exacta del ZIP y los 38 pads |
| Ruteo | 122 segmentos, 4 vías; direcciones 0/45/90° respecto a ejes, sin giros de esquina de 90° entre dos segmentos |
| GND | Plano B.Cu común, conectado a entrada, ESP32, amplificadores, capacitores y auxiliares |
| Alimentación ESP32 | +5V_SYS → JP1 (shunt colocado) → +5V_ESP → U1.19 (5V/VIN); U1.1 permanece en +3V3 |
| Salidas | Tres pares BTL independientes; ningún polo del parlante a GND |
| Ganancia | R5–R7 de 100 kΩ a VIN, configuración de 3 dB conservada |

Se acortaron recorridos I2S y derivaciones de alimentación. En la última revisión únicamente se conectó C2 localmente a C1, se fijó el mínimo de clearance y se eliminó un escape de GND redundante. Se conservó el puente corto de GND de U4 a una vía: era necesario para unir su región de cobre al plano principal. Cuatro transiciones cortas de 0,5–0,707 mm quedan junto a pads/vía; no hay microsegmentos artificiales de 0,04–0,16 mm.

## Revisión ampliada de avisos

Además del DRC normal se ejecutó una copia de auditoría con las cinco comprobaciones desactivadas por defecto habilitadas como warning. No aparecieron defectos geométricos ni conexiones pendientes. Aparecieron seis avisos `footprint_filters_mismatch`: J1–J5 usan borneras Phoenix en símbolos genéricos de conector, cuyo filtro espera una biblioteca llamada `Connector*`; JP1 usa dos pines con shunt en un símbolo cuyo filtro espera `Jumper*`. Se revisaron sus dos contactos y conexiones. Son filtros de búsqueda de biblioteca, no errores de pinout, clearance ni circuito; no se sustituyeron huellas correctas para satisfacer un nombre de filtro. El informe ampliado conserva los seis avisos. No se añadieron exclusiones DRC ni se bajó la severidad de ninguna regla.

## Anchos y corriente

- Entrada, fusible e interruptor: pistas de 2,2 mm. Troncal +5V_SYS hacia amplificadores: 2,0 mm, sin tramos estrechos en serie entre fuente y amplificadores.
- +5V_ESP y su alimentación: 1,0 mm; este ramal alimenta solo la ESP32, no los tres amplificadores.
- Señales I2S/GPIO y selección de ganancia: 0,3 mm. 3V3: 0,5 mm. Salidas BTL: 0,8 mm. Derivaciones de capacitores/test: 0,6–1,0 mm.
- Para cobre exterior de 35 µm, la aproximación IPC-2221 usada por la calculadora de KiCad da unos 1,37 mm para 3 A y aumento de 10 °C. La troncal de 2,0 mm supera ese ancho. Esta estimación no valida temperatura de contactos, fusible, MOSFET, sockets, soldaduras ni la placa dentro de una caja. Especificar al fabricante cobre terminado de al menos 35 µm y comprobar el prototipo con carga.
- El fusible preliminar T1,25 A no permite asumir servicio a 3 A: elegir calibre/curva, jack, interruptor y cableado adecuados antes de utilizar la fuente mayor. La fuente inicial de 1 A y el nivel de audio requieren comprobación de consumo.

## Revisión mecánica y límites

El modelo ESP32 del ZIP se muestra sobre dos sockets hembra THT 1×19, paso 2,54 mm y separación de hileras 25,4 mm. Se corrigió solo la orientación del modelo 3D; la huella, su numeración y sus pads no cambiaron. USB y botones accesibles en el modelo; espacio vertical para extraerla. Antena sin cobre en ambas capas y sin tornillos bajo ella. Los cuatro agujeros M3 mantienen espacio reservado de 3,4 mm de radio alrededor del centro. Borneras hacia el exterior y capacitores sin colisiones visibles con el modelo ESP32.

Los docks MAX98357A conservan la hilera 1×7 a 2,54 mm y sus dos pads de cable de salida. Se revisaron orientación, espacio reservado y acceso; no se rediseñaron. Los cuerpos 3D de los amplificadores no están disponibles: se muestran sus sockets. No se certifica el encaje físico del módulo real ni su sujeción. OUT+/OUT− se llevan por cables cortos desde el borne del módulo a los pads del dock, como indica MONTAJE.md.

Antes de fabricar: medir placa ESP32 real frente al modelo del ZIP, encaje/altura de sockets, tamaño y orientación de módulos, fijación y acceso a sus bornes, jack de barril externo y resto de piezas. Validar límite de audio para 8 Ω/1 W, integridad I2S, tensión y temperatura bajo carga; la ganancia baja no limita automáticamente a 1 W. No se generaron Gerbers.

Fuentes: [Calculadora de KiCad 10](https://docs.kicad.org/10.0/en/pcb_calculator/pcb_calculator.html), [pinout y ganancia de Adafruit 3006](https://learn.adafruit.com/adafruit-max98357-i2s-class-d-mono-amp/pinouts).
