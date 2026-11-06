#include "AudioPlayer.h"
#include "WavReader.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <driver/i2s.h>

namespace {
constexpr i2s_port_t PORT = I2S_NUM_0;
const int32_t SILENCE[Config::BLOCK_FRAMES * 2] = {};
}

bool AudioPlayer::begin() {
  // Un fallo de montaje no debe borrar las voces: sin formateo automatico.
  if (!SPIFFS.begin(false)) {
    Serial.println("Error: SPIFFS no montado; cargar imagen con uploadfs");
    return false;
  }
  requests_ = xQueueCreate(Config::QUEUE_SIZE, sizeof(const CommandEntry *));
  events_ = xQueueCreate(32, sizeof(AudioEvent));
  if (!requests_ || !events_) {
    if (requests_) vQueueDelete(requests_);
    if (events_) vQueueDelete(events_);
    requests_ = events_ = nullptr;
    return false;
  }
  i2s_config_t cfg{};
  cfg.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX);
  cfg.sample_rate = Config::I2S_SAMPLE_RATE;
  // El tono de diagnostico que ya fue probado usa 16 kHz y slots PCM32.
  // La voz de 8 kHz se remuestrea duplicando cada muestra para conservar
  // duracion y tono con la misma configuracion I2S validada en hardware.
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = Config::DMA_BUFFERS;
  cfg.dma_buf_len = Config::BLOCK_FRAMES;
  cfg.use_apll = false;
  cfg.tx_desc_auto_clear = true; // Silencio ante underrun, no repetir audio viejo.
  esp_err_t error = i2s_driver_install(PORT, &cfg, 0, nullptr);
  if (error != ESP_OK) {
    Serial.printf("Error instalando I2S: %s\n", esp_err_to_name(error));
    return false;
  }
  i2s_pin_config_t pins{};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = Config::BCLK;
  pins.ws_io_num = Config::WS;
  pins.data_out_num = Config::DATA_OUT;
  pins.data_in_num = I2S_PIN_NO_CHANGE;
  error = i2s_set_pin(PORT, &pins);
  if (error != ESP_OK) {
    Serial.printf("Error configurando pines I2S: %s\n", esp_err_to_name(error));
    i2s_driver_uninstall(PORT);
    return false;
  }
  i2s_zero_dma_buffer(PORT);
  if (xTaskCreate(taskEntry, "saludarte_audio", 8192, this, 2, nullptr) != pdPASS) {
    // No detener relojes ya conectados al MAX; auto-clear conserva silencio.
    Serial.println("Error: no se pudo crear tarea de audio");
    return false;
  }
  ready_ = true;
  Serial.printf("I2S: WS=%u Hz, BCLK=%u Hz, 32 bits/canal; WAV=%u Hz; BCLK=%d WS=%d DIN=%d\n",
                unsigned(Config::I2S_SAMPLE_RATE),
                unsigned(Config::I2S_SAMPLE_RATE * 64),
                unsigned(Config::WAV_SAMPLE_RATE),
                Config::BCLK, Config::WS, Config::DATA_OUT);
  Serial.printf("I2S listo; SPIFFS: %u/%u bytes\n",
                unsigned(SPIFFS.usedBytes()), unsigned(SPIFFS.totalBytes()));
  return true;
}

bool AudioPlayer::enqueue(const CommandEntry *entry) {
  return ready_ && xQueueSend(requests_, &entry, 0) == pdTRUE;
}
bool AudioPlayer::poll(AudioEvent &event) {
  return events_ && xQueueReceive(events_, &event, 0) == pdTRUE;
}
void AudioPlayer::taskEntry(void *self) { static_cast<AudioPlayer *>(self)->run(); }
void AudioPlayer::report(const char *status, uint16_t id) {
  const AudioEvent event{status, id}; // status siempre es un literal estatico.
  // loop() drena esta cola aunque no haya cliente BLE conectado.
  xQueueSend(events_, &event, portMAX_DELAY);
}
bool AudioPlayer::writeBlock(const int32_t *samples, size_t frames) {
  // Los bloques de escritura son cortos para alimentar DMA sin bloquear tareas.
  size_t written = 0;
  const size_t expected = frames * 2 * sizeof(int32_t);
  const esp_err_t error = i2s_write(PORT, samples, expected, &written,
                                  pdMS_TO_TICKS(100));
  if (error == ESP_OK && written == expected) return true;
  Serial.printf("Error I2S: %s, %u/%u bytes\n", esp_err_to_name(error),
                unsigned(written), unsigned(expected));
  outputHealthy_ = false;
  i2s_zero_dma_buffer(PORT);
  return false;
}
bool AudioPlayer::drain() {
  // Mas silencio que toda la capacidad DMA fuerza la salida de la ultima
  // muestra antes de DONE. BCLK y WS permanecen continuos entre pistas.
  for (int i = 0; i < Config::DMA_BUFFERS + 2; ++i)
    if (!writeBlock(SILENCE, Config::BLOCK_FRAMES)) return false;
  return true;
}
void AudioPlayer::play(const CommandEntry &entry) {
  const uint16_t id = uint16_t(entry.activity);
  if (!outputHealthy_) { report("ERR:I2S", id); return; }
  File file = SPIFFS.open(entry.path, FILE_READ);
  if (!file || file.isDirectory()) { report("ERR:FILE", id); return; }
  WavInfo info;
  if (!parseWav(file, info)) { report("ERR:WAV", id); return; }
  Serial.printf("Reproduciendo audio: %s\n", entry.path);
  report("PLAY", id); // Inicio de streaming, no medicion acustica.
  const size_t bytesPerSample = info.bitsPerSample / 8;
  uint8_t mono[Config::BLOCK_FRAMES * 2];
  int32_t stereo[Config::BLOCK_FRAMES * 4];
  uint32_t remaining = info.bytes;
  const char *result = "DONE";
  while (remaining) {
    const size_t maxRead = Config::BLOCK_FRAMES * bytesPerSample;
    const size_t count = remaining < maxRead ? remaining : maxRead;
    if (file.read(mono, count) != count) { result = "ERR:READ"; break; }
    const size_t frames = count / bytesPerSample;
    const unsigned volumePercent = volumePercent_.load();
    for (size_t i = 0; i < frames; ++i) {
      int16_t raw;
      if (info.bitsPerSample == 8) {
        // PCM8 WAV usa muestras sin signo, con 128 como nivel de silencio.
        raw = int16_t((int(mono[i]) - 128) * 256);
      } else {
        raw = int16_t(le16(mono + 2 * i));
      }
      const int16_t sample =
          raw * int32_t(volumePercent) / 100;
      // PCM16 ocupa los 16 bits superiores de la palabra PCM32. Multiplicar
      // evita desplazar a la izquierda un entero negativo (UB en C++).
      const int32_t output = int32_t(sample) * 65536;
      // 8 kHz -> 16 kHz: repetir cada muestra dos veces; L y R reciben la voz.
      for (size_t repeat = 0; repeat < 2; ++repeat) {
        const size_t out = (i * 2 + repeat) * 2;
        stereo[out] = stereo[out + 1] = output;
      }
    }
    if (!writeBlock(stereo, frames * 2)) { result = "ERR:I2S"; break; }
    remaining -= count;
  }
  file.close();
  if (outputHealthy_ && !drain()) result = "ERR:I2S";
  if (strcmp(result, "DONE") == 0) Serial.println("Audio finalizado");
  report(result, id);
}
void AudioPlayer::run() {
  for (;;) {
    const CommandEntry *entry = nullptr;
    if (xQueueReceive(requests_, &entry, 0) == pdTRUE) play(*entry);
    else if (outputHealthy_) writeBlock(SILENCE, Config::BLOCK_FRAMES);
    else vTaskDelay(pdMS_TO_TICKS(20));
  }
}
