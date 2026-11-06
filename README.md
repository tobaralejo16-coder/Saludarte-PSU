# Saludarte-PSU

### Sistema de asistencia y acompañamiento mediante aplicación Android y ESP32

**Saludarte_PSU** es un proyecto desarrollado en el contexto de PSU, que integra una aplicación móvil Android, un sistema embebido basado en ESP32 y un diseño de hardware electrónico para implementar funciones de asistencia, recordatorios y comunicación entre dispositivos.

El repositorio reúne el código fuente, los recursos multimedia y los archivos de diseño electrónico utilizados durante el desarrollo, montaje y pruebas del sistema.

## 1. Descripción del proyecto

Saludarte busca integrar herramientas de software y hardware en una solución tecnológica de apoyo a las actividades cotidianas.

El sistema integra:

- Una aplicación móvil Android para la interacción con el usuario.
- Un dispositivo basado en ESP32 para ejecutar las funciones del sistema embebido.
- Comunicación inalámbrica Bluetooth Low Energy (BLE) entre la aplicación y la ESP32.
- Recursos de audio para recordatorios, actividades y notificaciones.
- Hardware electrónico utilizado como referencia para el montaje, las conexiones y la soldadura de los componentes físicos del sistema.

## 2. Estructura del repositorio

```text
Saludarte-PSU/
│
├── App-Android/
│   └── Código fuente y recursos de la aplicación
│
├── Esp32/
│   └── Firmware, audios y configuración de ESP32
│
├── Hardware/
│   └── ESP32_Audio_3SPK/
│       ├── ESP32_Audio_3SPK.kicad_pro
│       ├── ESP32_Audio_3SPK.kicad_sch
│       ├── ESP32_Audio_3SPK.kicad_pcb
│       ├── Proyecto_Audio.pretty/
│       ├── models/
│       ├── vista/
│       └── Archivos complementarios
│
├── .gitignore
└── README.md
```

## 3. Aplicación Android

La aplicación está desarrollada en **Kotlin**, utilizando Android Studio.

Incluye componentes para la interfaz de usuario, la gestión de la comunicación Bluetooth BLE y recursos multimedia asociados con las funcionalidades del sistema.

**Contenido disponible:**

- Código fuente Kotlin.
- Interfaz y recursos gráficos.
- Archivos de audio MP3.
- Configuración de Gradle.
- Archivos necesarios para abrir y continuar desarrollando el proyecto en Android Studio.

**Ubicación:** `App-Android/`

### Apertura del proyecto

1. Descargar o clonar el repositorio.
2. Abrir Android Studio.
3. Seleccionar **Open**.
4. Buscar la carpeta `App-Android/`.
5. Esperar a que Gradle sincronice las dependencias.
6. Seleccionar un dispositivo Android compatible para ejecutar la aplicación.

## 4. Sistema ESP32

El firmware de la ESP32 implementa las funciones del sistema embebido y su comunicación con la aplicación móvil.

El proyecto incluye archivos relacionados con:

- Comunicación Bluetooth Low Energy (BLE).
- Reproducción y gestión de audio.
- Configuración de PlatformIO.
- Herramientas de diagnóstico.
- Pruebas y documentación técnica.

**Ubicación:** `Esp32/`

### Apertura del proyecto

1. Instalar Visual Studio Code.
2. Instalar la extensión PlatformIO.
3. Abrir la carpeta `Esp32/`.
4. Seleccionar la configuración correspondiente del proyecto.
5. Compilar el firmware y cargarlo en una ESP32 compatible.

Para información adicional, consultar el README incluido dentro de la carpeta del firmware.

## 5. Diseño y montaje de hardware

El hardware de Saludarte fue desarrollado utilizando **KiCad 10**, con el objetivo de organizar las conexiones electrónicas, definir los componentes del dispositivo y disponer de una referencia técnica para su implementación física.

Los archivos de diseño se utilizaron como apoyo durante el proceso de **montaje, conexión y soldadura de los componentes electrónicos** del sistema.

El proyecto incluye el esquemático electrónico, el diseño de PCB, bibliotecas de componentes, modelos tridimensionales y archivos complementarios de verificación.

### 5.1. Esquemático electrónico

El archivo `ESP32_Audio_3SPK.kicad_sch` contiene la representación de las conexiones eléctricas del sistema.

Permite consultar los componentes, las señales y la organización de los circuitos utilizados como referencia para el montaje físico.

### 5.2. Diseño de PCB

El archivo `ESP32_Audio_3SPK.kicad_pcb` contiene el diseño de la tarjeta electrónica, incluyendo la distribución de componentes y las conexiones entre ellos.

Este diseño forma parte de la documentación técnica utilizada durante el desarrollo e implementación del hardware.

### 5.3. Modelos tridimensionales

La carpeta `models/` contiene recursos tridimensionales asociados con el diseño electrónico.

Estos archivos permiten complementar la representación física de los componentes dentro del entorno de KiCad.

### 5.4. Bibliotecas y archivos complementarios

El proyecto también incluye:

- `Proyecto_Audio.pretty/`: biblioteca de huellas de componentes.
- `BOM_preliminar.csv`: lista preliminar de materiales.
- `ERC.json` y `DRC.json`: archivos relacionados con las verificaciones eléctricas y de diseño.
- `MONTAJE.md`: información relacionada con el montaje.
- `VALIDACION.md`: documentación de validación.
- `ESTADO_TRABAJO.md`: registro del estado del desarrollo.
- `LEEME.md`: información complementaria del proyecto.

### Apertura del proyecto de hardware

1. Instalar KiCad 10.
2. Descargar o clonar el repositorio.
3. Abrir la carpeta `Hardware/ESP32_Audio_3SPK/`.
4. Ejecutar el archivo `ESP32_Audio_3SPK.kicad_pro`.
5. Desde KiCad, abrir el editor de esquemáticos o el editor de PCB según corresponda.

Se recomienda conservar la estructura de carpetas original para mantener las referencias a bibliotecas, huellas y modelos tridimensionales.

**Ubicación:** `Hardware/ESP32_Audio_3SPK/`

## 6. Descarga del proyecto

El repositorio es público y puede consultarse sin necesidad de solicitar acceso.

Para descargar los archivos:

1. Abrir el repositorio Saludarte-PSU en GitHub.
2. Seleccionar el botón **Code**.
3. Elegir **Download ZIP**.
4. Extraer el archivo descargado.
5. Abrir el componente del proyecto que se desea consultar o modificar.

También puede clonarse mediante Git:

```bash
git clone https://github.com/tobaralejo16-coder/Saludarte-PSU.git
```

## 7. Estado del proyecto

| Componente | Estado |
|---|---|
| Aplicación Android | Código fuente disponible |
| Firmware ESP32 | Código fuente disponible |
| Comunicación Bluetooth BLE | Implementada en el código |
| Esquemático electrónico | Disponible |
| Diseño PCB | Disponible |
| Modelos 3D | Archivos disponibles |
| Montaje y soldadura | Realizados utilizando el diseño de hardware como referencia |
| Documentación técnica | Incluida en las carpetas correspondientes |

La disponibilidad de los archivos de diseño permite consultar y continuar desarrollando el sistema. Para realizar nuevas fabricaciones o modificaciones de hardware, se recomienda revisar previamente los documentos de validación y verificar las conexiones y componentes.

## 8. Desarrollo y colaboración

Saludarte-PSU es un repositorio destinado a centralizar los diferentes componentes del proyecto y facilitar su consulta, mantenimiento y desarrollo.

El código fuente y los archivos publicados pueden consultarse y descargarse públicamente.

Las personas interesadas en colaborar pueden proponer mejoras mediante los mecanismos de contribución disponibles en GitHub.

---

**Proyecto Saludarte — PSU**

Repositorio mantenido en GitHub: [Saludarte-PSU](https://github.com/tobaralejo16-coder/Saludarte-PSU).
