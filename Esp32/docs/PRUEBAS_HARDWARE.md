# Pruebas de aceptación: SaludarteApp y ESP32

Requieren la placa y los tres MAX98357A. Compilar no verifica sonido, radio,
alimentación ni continuidad de los relojes. Los tests Python validan las
herramientas de archivos del ordenador; no ejecutan el firmware.

1. Preparar los trece WAV PCM8 desde `assets/audio_source/` con
   `tools/prepare_audio.py`, validar con `tools/check_assets.py`, cargar primero
   el firmware/tabla de particiones y luego SPIFFS, reiniciar y abrir Serial a
   115200 baudios.
2. Buscar `SALUDARTE_ESP32` por escaneo activo. Descubrir servicio
   `4fafc201-1fb5-459e-8fcc-c5c9c331914b` y característica
   `beb5483e-36e1-4688-b7f5-ea07361b26a8`, con READ y WRITE.
   No solicitar NOTIFY. READ inicial debe ser READY si SPIFFS/I2S iniciaron.
3. Escribir `02` como dos bytes ASCII mediante WRITE con respuesta. En Serial:

   ```text
   BLE conectado
   Comando recibido: 02
   Actividad: DESAYUNAR
   BLE estado: OK:02
   Reproduciendo audio: /audio/0002.wav
   BLE estado: PLAY:02
   Audio finalizado
   BLE estado: DONE:02
   ```

   Los registros de las tareas pueden intercalarse. Escuchar la voz completa
   en los tres parlantes. READ muestra el último estado y puede saltarse OK/PLAY.
4. Probar `01` a `12` y `99`, comprobando actividad y voz exactas. Emergencia
   debe usar 0099.wav, no 0012.wav. Enviar cada comando cuando termine el anterior.
5. Probar `V0` a `V9`: Serial confirma porcentajes 0, 11, 22, 33, 44, 55, 66,
   77, 88 y 100; READ confirma con `OK:Vx:porcentaje`. Enviar `V0` durante una
   voz larga y verificar que el volumen cambia sin reiniciar el audio. Enviar
   `Vx` y `V10`: ambos deben responder `ERR:FORMAT`.
6. Mientras suena un WAV largo, enviar dos códigos. Reproducción FIFO sin mezcla.
   Repetir un mismo código: debe sonar dos veces. `99` también sigue FIFO.
7. Enviar payload vacío, `1`, `001`, `02\n`, ` 02`, un byte binario 0x01,
   NUL embebido y `A1`: ERR:FORMAT. Enviar `00` y `13`: ERR:UNKNOWN mientras
   esos códigos no estén registrados. Ninguno debe reproducir una voz.
8. Enviar ráfagas durante un WAV largo. Serial debe informar ERR:QUEUE:<codigo>
   o ERR:INPUT_QUEUE cuando se llenen las colas. BLE debe seguir respondiendo.
9. Desconectar a mitad de pista. Los audios aceptados continúan. El dispositivo
   debe volver a anunciarse; reconectar desde Android y redescubrir el servicio.
   READ devuelve solo el último evento, no los perdidos. No reenviar por defecto.
10. Cargar una imagen sin 0002.wav. Enviar `02`: OK:02 seguido de ERR:FILE:02.
   Probar un WAV estéreo, MP3 renombrado, PCM de otra frecuencia o truncado:
   ERR:WAV:02. Después enviar otro código con WAV válido y comprobar recuperación.
11. Iniciar con SPIFFS sin imagen válida: READ ERR:PLAYER y, después de `02`,
    ERR:PLAYER:02. No se debe formatear automáticamente ni borrar archivos.
12. Medir LRC aproximadamente 16 kHz y BCLK aproximadamente 1.024 MHz antes,
    durante y después de la voz; comprobar continuidad y ausencia de recorte final.
13. Verificar +5V_SYS con tres parlantes al volumen previsto; subir gradualmente
    y observar distorsión, reinicios y temperatura. OUT− nunca se conecta a GND.

DONE confirma salida digital, no audición ni presencia de parlantes. Este firmware
no transfiere las voces de Android y no envía llamadas/alertas externas con `99`.
