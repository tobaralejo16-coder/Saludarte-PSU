#include <Arduino.h>
#include <driver/i2s.h>
#include "Config.h"

// Prueba independiente: genera PCM en RAM. No lee voces ni espera comandos.
// 3 pitidos de 1 kHz, de medio segundo, cada 5 segundos. Misma senal en L y R.
// Frecuencia fija 16 kHz y slots de 32 bits: BCLK = 1,024 MHz.
namespace {
constexpr i2s_port_t PORT = I2S_NUM_0;
constexpr uint32_t RATE = 16000;
constexpr size_t FRAMES = 256;
constexpr uint32_t HALF_SECOND = RATE / 2;
constexpr uint32_t CYCLE = RATE * 5;
// Seno de 16 muestras, amplitud aproximada 10 % (antes de ganancia del MAX).
constexpr int16_t SINE[] = {0, 1254, 2317, 3027, 3277, 3027, 2317, 1254,
                           0, -1254, -2317, -3027, -3277, -3027, -2317, -1254};
int32_t buffer[FRAMES * 2] = {};
uint32_t frame = 0;
bool ready = false;

bool check(esp_err_t result, const char *operation) {
  if (result == ESP_OK) return true;
  Serial.printf("DIAG-I2S ERROR %s: %s\n", operation, esp_err_to_name(result));
  return false;
}
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== SALUDARTE DIAG-I2S v1: SIN BLE NI ARCHIVOS ===");
  Serial.printf("BCLK GPIO%d | LRC GPIO%d | DIN GPIO%d\n",
                Config::BCLK, Config::WS, Config::DATA_OUT);
  Serial.println("LRC=16000 Hz | BCLK=1024000 Hz | PCM32 estereo L=R");

  i2s_config_t cfg{};
  cfg.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX);
  cfg.sample_rate = RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = FRAMES;
  cfg.tx_desc_auto_clear = true;
  if (!check(i2s_driver_install(PORT, &cfg, 0, nullptr), "driver_install")) return;

  i2s_pin_config_t pins{};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = Config::BCLK;
  pins.ws_io_num = Config::WS;
  pins.data_out_num = Config::DATA_OUT;
  pins.data_in_num = I2S_PIN_NO_CHANGE;
  if (!check(i2s_set_pin(PORT, &pins), "set_pin")) return;
  if (!check(i2s_zero_dma_buffer(PORT), "zero_dma")) return;
  ready = true;
}

void loop() {
  if (!ready) {
    Serial.println("DIAG-I2S detenido por error; revisar mensajes de arranque");
    delay(2000);
    return;
  }
  bool newCycle = false;
  for (size_t i = 0; i < FRAMES; ++i) {
    if (frame == 0) newCycle = true;
    const uint32_t segment = frame / HALF_SECOND;
    const bool beep = segment == 0 || segment == 2 || segment == 4;
    int32_t value = 0;
    if (beep) {
      const uint32_t position = frame % HALF_SECOND;
      // Fundidos de 5 ms reducen los clics al iniciar y terminar cada pitido.
      uint32_t envelope = 80;
      if (position < 80) envelope = position;
      if (HALF_SECOND - 1 - position < envelope)
        envelope = HALF_SECOND - 1 - position;
      value = int32_t(SINE[frame % 16]) * int32_t(envelope) / 80;
      value *= 65536; // Alinear PCM16 en los bits altos de PCM32, sin shift negativo.
    }
    buffer[2 * i] = buffer[2 * i + 1] = value;
    frame = (frame + 1) % CYCLE;
  }
  size_t written = 0;
  const esp_err_t result = i2s_write(PORT, buffer, sizeof(buffer), &written,
                                   pdMS_TO_TICKS(100));
  if (!check(result, "write") || written != sizeof(buffer)) {
    Serial.printf("DIAG-I2S: escritura incompleta %u/%u\n",
                  unsigned(written), unsigned(sizeof(buffer)));
    i2s_zero_dma_buffer(PORT);
    ready = false;
    return;
  }
  if (newCycle) Serial.println("DIAG-I2S: enviando 3 pitidos; despues pausa");
  // Se siguen enviando ceros durante las pausas: no se detienen BCLK ni LRC.
}
