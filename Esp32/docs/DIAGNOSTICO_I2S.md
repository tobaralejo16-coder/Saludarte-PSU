# Diagnóstico de salida I2S y voces

La prueba independiente de pitido transmite PCM32 estéreo a 16000 Hz:

- LRC/WS: 16 kHz.
- BCLK: 16000 x 2 x 32 = 1,024 MHz.
- GPIO26 BCLK, GPIO25 WS y GPIO22 DIN.

El firmware de voz se configuró para usar la misma frecuencia y formato I2S que
esa prueba. Las voces se almacenan como PCM mono a 8000 Hz y cada muestra se
duplica al transmitir, manteniendo duración y tono. Esto evita atribuir a los
archivos de voz una diferencia de reloj que el pitido no tenía.

## Causa verificada de la imagen SPIFFS incompleta

Con los WAV PCM16 originales, `pio run --target buildfs` imprimía
`SPIFFS_write error(-10001): File system is full.` al generar la imagen, pero
PlatformIO terminaba con `SUCCESS`. Esa imagen no contenía todas las actividades,
por lo que una carga no podía dejar el catálogo completo disponible.

Se conservaron los originales PCM16 en `assets/audio_source/` y se generaron
copias PCM8 mono 8000 Hz en `data/audio/`. La conversión reduce el tamaño a la
mitad; las 13 voces ahora están por debajo del presupuesto seguro de SPIFFS.
Volver a generar/cargar el filesystem cuando cambie el catálogo:

```powershell
$env:PLATFORMIO_CORE_DIR = Join-Path (Get-Location) '.pio-core'
$python = 'C:\Users\User\.platformio\penv\Scripts\python.exe'
$pio = 'C:\Users\User\.platformio\penv\Scripts\pio.exe'
& $python tools/prepare_audio.py
if ($LASTEXITCODE -ne 0) { throw 'Falló la conversión de audio' }
& $python tools/check_assets.py
if ($LASTEXITCODE -ne 0) { throw 'Los audios no pasan el preflight' }
& $pio run --target uploadfs
```

El verificador impone un máximo conservador del 65 % del tamaño de la partición,
porque el generador SPIFFS no siempre propaga el error de falta de espacio.

Si el pitido independiente suena pero una voz no, observar Serial desde RESET.
`ERR:PLAYER` indica que SPIFFS/I2S no inició; `ERR:FILE` apunta a una pista
ausente; `ERR:WAV` indica cabecera/formato incompatible; `ERR:I2S` indica fallo
de transmisión. Al reproducir una voz, Serial debe informar I2S a 16 kHz,
BCLK 1,024 MHz, reproducción de la ruta WAV y `Audio finalizado`.

Una medición con multímetro no confirma actividad de datos I2S. Si después de
verificar el filesystem y esos mensajes sigue sin escucharse audio, revisar
WS/BCLK/DIN con analizador lógico, SD/EN, alimentación y conexiones diferenciales
del parlante. `OUT-` no debe conectarse a GND.
