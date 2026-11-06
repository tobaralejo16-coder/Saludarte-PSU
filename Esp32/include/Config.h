#pragma once
#include <stddef.h>
#include <stdint.h>

// Hardware confirmado por el proyecto. ESP32 clasica (esp32dev), flash 4 MiB.
namespace Config {
constexpr uint32_t FLASH_BYTES = 0x400000;
constexpr uint32_t APP_PARTITION_OFFSET = 0x10000;
constexpr uint32_t APP_PARTITION_BYTES = 0x0F0000;
constexpr uint32_t SPIFFS_PARTITION_OFFSET = 0x100000;
constexpr uint32_t SPIFFS_PARTITION_BYTES = 0x300000;
constexpr int BCLK = 26;
constexpr int WS = 25;
constexpr int DATA_OUT = 22;
constexpr uint32_t WAV_SAMPLE_RATE = 8000;
constexpr uint32_t I2S_SAMPLE_RATE = 16000;
constexpr unsigned VOLUME_PERCENT = 35; // Atenuacion digital inicial (0..100).
constexpr size_t BLOCK_FRAMES = 256;
constexpr int DMA_BUFFERS = 4;
constexpr size_t COMMAND_LENGTH = 2; // Protocolo Android: exactamente dos digitos ASCII.
constexpr unsigned QUEUE_SIZE = 8;
constexpr char DEVICE_NAME[] = "SALUDARTE_ESP32";
constexpr char SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char CHARACTERISTIC_UUID[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
static_assert(VOLUME_PERCENT <= 100, "Volumen fuera de rango");
static_assert(APP_PARTITION_OFFSET + APP_PARTITION_BYTES <=
                  SPIFFS_PARTITION_OFFSET,
              "Las particiones de aplicacion y SPIFFS se solapan");
static_assert(SPIFFS_PARTITION_OFFSET + SPIFFS_PARTITION_BYTES <=
                  FLASH_BYTES,
              "Las particiones exceden la flash configurada");
}
