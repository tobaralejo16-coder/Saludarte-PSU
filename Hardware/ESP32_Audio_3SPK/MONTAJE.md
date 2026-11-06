# Conexiones externas previstas

Esta guía describe el **prototipo**; confirmar las piezas y la polaridad antes de energizar.

| Conector | Cableado |
|---|---|
| J1.1 / J1.2 | Jack hembra externo para la fuente 5 V: centro + / exterior −. |
| J2.1 / J2.2 | Dos terminales de un interruptor ON/OFF externo, en serie con el positivo. |
| JP1 | Shunt de 2 contactos para entregar +5 V a la ESP32 desde la base. |
| J3.1 / J3.2 | Parlante 1: SPK+ / SPK− del módulo U2. |
| J4.1 / J4.2 | Parlante 2: SPK+ / SPK− del módulo U3. |
| J5.1 / J5.2 | Parlante 3: SPK+ / SPK− del módulo U4. |

Cada U2–U4 requiere un socket hembra THT 1×7. Los siete pads `JP1.1` a `JP1.7` de la huella corresponden al header homónimo **del módulo Adafruit 3006**: VIN, GND, SD_MODE, GAIN, DIN, BCLK y LRC. `JP1.3` queda sin conexión externa para conservar la configuración del módulo. Soldar un cable corto desde `SPK+` del borne del módulo al pad de la base `X1.2`, y otro desde `SPK−` al pad `X1.1`. Los pads `X1.*` son agujeros de cable: el módulo no encaja directamente en ellos. Sujetar mecánicamente los módulos y aliviar la tracción de los cables; el socket y los cables no son un anclaje validado.

No conectar ningún polo de J3–J5 a GND ni unir salidas de dos amplificadores. Verificar que la ESP32 real coincide contacto por contacto con la huella antes de insertarla. Con la fuente externa apagada, comprobar continuidad, ausencia de cortos y polaridad; realizar la primera puesta en marcha con límite de corriente y sin parlantes, y añadirlos tras verificar las salidas.
