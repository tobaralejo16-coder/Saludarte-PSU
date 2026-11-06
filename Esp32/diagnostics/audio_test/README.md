# Prueba independiente sin BLE ni WAV

Este firmware emite tres pitidos automáticamente cada cinco segundos. No lee
SPIFFS ni necesita el teléfono. Usa GPIO26/BCLK, GPIO25/LRC y GPIO22/DIN,
I2S estándar, PCM32 estéreo con L=R y 16000 Hz. Amplitud digital aproximada: 10 %.

Cerrar el monitor y ejecutar desde la raíz del proyecto:

```powershell
$env:PLATFORMIO_CORE_DIR = Join-Path (Get-Location) '.pio-core'
$pio = 'C:\Users\User\.platformio\penv\Scripts\pio.exe'
& $pio run -c platformio.audio_test.ini -e audio_test -t upload --upload-port COM5
& $pio device monitor --port COM5 --baud 115200
```

Presionar EN/RESET y comprobar el identificador:
`=== SALUDARTE DIAG-I2S v1: SIN BLE NI ARCHIVOS ===`.
Debe repetirse `DIAG-I2S: enviando 3 pitidos; despues pausa`. Con este firmware
no se anuncia BLE. No ejecutar uploadfs: no es necesario ni se modifican voces.

- Si suenan los pitidos, existe una ruta de audio funcional con esta configuración;
  comparar después el firmware normal, frecuencia y contenido de sus WAV.
- Si no suenan, revisar conexiones/modelo/parlante y medir las señales I2S en
  el MAX. El registro confirma escrituras aceptadas por el driver, no sonido real.
  En esta prueba LRC = 16 kHz y BCLK = 1,024 MHz.
- Si aparece un error DIAG-I2S, conservar el log desde RESET para identificar
  la operación que falla. Si aparece NTP u otro identificador, no corre esta prueba.

Para volver a Saludarte, cerrar el monitor y cargar el proyecto normal:

```powershell
& $pio run -c platformio.ini -e esp32dev -t upload --upload-port COM5
```

La prueba usa la misma tabla de particiones actual y no escribe SPIFFS. El código
principal no se reemplaza: la configuración alternativa selecciona otra carpeta
de fuentes. La compilación no sustituye la comprobación en la placa.
