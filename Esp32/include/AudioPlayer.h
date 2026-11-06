#pragma once
#include "Commands.h"
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

struct AudioEvent { const char *status; uint16_t id; };

// Un solo trabajador posee el archivo e I2S. BLE nunca escribe audio.
class AudioPlayer {
 public:
  bool begin();
  bool enqueue(const CommandEntry *entry);
  bool poll(AudioEvent &event);
  bool ready() const { return ready_; }
  void setVolumePercent(unsigned percent) {
    if (percent > 100) percent = 100;
    volumePercent_.store(percent);
  }
  unsigned getVolumePercent() const { return volumePercent_.load(); }
 private:
  static void taskEntry(void *self);
  void run();
  bool writeBlock(const int32_t *samples, size_t frames);
  bool drain();
  void play(const CommandEntry &entry);
  void report(const char *status, uint16_t id);
  QueueHandle_t requests_ = nullptr;
  QueueHandle_t events_ = nullptr;
  std::atomic<unsigned> volumePercent_{Config::VOLUME_PERCENT};
  bool ready_ = false; // Solo se modifica antes de anunciar BLE.
  bool outputHealthy_ = true; // Solo lo usa el trabajador.
};
