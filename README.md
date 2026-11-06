# Saludarte-PSU

### Sistema de asistencia y acompañamiento mediante aplicación Android y ESP32

**Saludarte** es un proyecto desarrollado en el contexto de PSU, que integra una aplicación móvil Android, un sistema embebido basado en ESP32 y un diseño de hardware para implementar funciones de asistencia, recordatorios y comunicación entre dispositivos.

El repositorio reúne el código fuente, los recursos multimedia y los archivos de diseño necesarios para consultar, estudiar y continuar desarrollando el sistema.

## 1. Descripción del proyecto

Saludarte busca integrar herramientas de software y hardware en una solución tecnológica de apoyo a las actividades cotidianas.

El sistema contempla:

- Una aplicación móvil Android para la interacción con el usuario.
- Un dispositivo basado en ESP32 para ejecutar funciones del sistema embebido.
- Comunicación inalámbrica Bluetooth Low Energy (BLE) entre la aplicación y la ESP32.
- Recursos de audio para recordatorios, actividades y notificaciones.
- Hardware electrónico diseñado para integrar los componentes físicos del sistema.

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
│   ├── Esquematico/
│   ├── PCB/
│   └── Modelo-3D/
│
└── README.md
```

## 3. Aplicación Android

La aplicación está desarrollada en **Kotlin**, utilizando Android Studio.

Incluye componentes para la interfaz de usuario, la gestión de la comunicación Bluetooth BLE y recursos multimedia asociados con las funcionalidades del sistema.

**Contenido:**
- Código fuente Kotlin.
- Interfaz y recursos gráficos.
- Archivos de audio MP3.
- Configuración de Gradle.
- Archivos necesarios para abrir el proyecto en Android Studio.

**Ubicación:** `App-Android/`

## 4. Sistema ESP32

El firmware de la ESP32 permite implementar funciones del sistema embebido y su comunicación con la aplicación móvil.

El proyecto incluye archivos relacionados con:

- Comunicación Bluetooth BLE.
- Reproducción y gestión de audio.
- Configuración de PlatformIO.
- Herramientas de diagnóstico.
- Pruebas y documentación técnica del firmware.

**Ubicación:** `Esp32/`

Para información adicional, consultar el README incluido dentro de esa carpeta.

## 5. Diseño de hardware

El diseño de hardware de Saludarte integra los componentes electrónicos y mecánicos necesarios para el funcionamiento físico del dispositivo.

### Esquemático electrónico

Contiene los diagramas de conexiones, componentes electrónicos y organización de los circuitos utilizados en el dispositivo.

### Diseño de PCB

Incluye los archivos de diseño de la tarjeta de circuito impreso, distribución de componentes, pistas y archivos asociados a su fabricación.

### Modelo 3D

Contiene los modelos tridimensionales del dispositivo, permitiendo visualizar su estructura física, la disposición de componentes y el diseño mecánico.

**Ubicación:** `Hardware/`

## 6. Descarga del proyecto

El repositorio es público y puede consultarse sin necesidad de solicitar acceso.

Para descargar los archivos:

1. Abrir el repositorio Saludarte-PSU en GitHub.
2. Seleccionar el botón **Code**.
3. Elegir **Download ZIP**.
4. Extraer el archivo descargado.

También puede clonarse mediante Git:

```bash
git clone https://github.com/tobaralejo16-coder/Saludarte-PSU.git
```

Para trabajar con la aplicación Android, abrir `App-Android/` como proyecto en Android Studio.

Para trabajar con el firmware, consultar las instrucciones y la configuración de PlatformIO dentro de `Esp32/`.

Para consultar y modificar el diseño electrónico, acceder a los archivos disponibles en `Hardware/`.

## 7. Estado del proyecto

| Componente | Estado |
|---|---|
| Aplicación Android | Código fuente disponible |
| Firmware ESP32 | Código fuente disponible |
| Esquemático electrónico | Disponible |
| Diseño PCB | Disponible |
| Modelo 3D | Disponible |

## 8. Desarrollo y colaboración

Saludarte-PSU es un repositorio destinado a centralizar los diferentes componentes del proyecto y facilitar su consulta, mantenimiento y desarrollo.

El código y los archivos publicados pueden consultarse y descargarse públicamente.

---

**Proyecto Saludarte — PSU**

Repositorio mantenido en GitHub: [Saludarte-PSU](https://github.com/tobaralejo16-coder/Saludarte-PSU).
