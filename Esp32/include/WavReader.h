#pragma once
#include "Config.h"
#include <stdint.h>
#include <string.h>

struct WavInfo {
  uint32_t offset = 0;
  uint32_t bytes = 0;
  uint16_t bitsPerSample = 0;
};
inline uint16_t le16(const uint8_t *p) { return p[0] | uint16_t(p[1]) << 8; }
inline uint32_t le32(const uint8_t *p) {
  return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 |
         uint32_t(p[3]) << 24;
}

// Reader ofrece size(), seek(pos), read(buf,n). No asume una cabecera de 44 bytes:
// recorre chunks RIFF, salta metadatos y comprueba limites antes de cada lectura.
// PCM entero mono, 8 o 16 bits little endian, a Config::WAV_SAMPLE_RATE Hz.
template <typename Reader> bool parseWav(Reader &file, WavInfo &info) {
  uint8_t header[16];
  const uint64_t size = file.size();
  if (size < 12 || !file.seek(0) || file.read(header, 12) != 12 ||
      memcmp(header, "RIFF", 4) || memcmp(header + 8, "WAVE", 4)) return false;
  const uint64_t end = uint64_t(le32(header + 4)) + 8;
  if (end > size || end < 12) return false;
  bool formatFound = false, dataFound = false;
  uint64_t pos = 12;
  while (pos < end) {
    if (end - pos < 8 || !file.seek(uint32_t(pos)) ||
        file.read(header, 8) != 8) return false;
    const uint32_t bytes = le32(header + 4);
    const uint64_t payload = pos + 8;
    const uint64_t next = payload + bytes + (bytes & 1U);
    if (next > end) return false;
    if (memcmp(header, "fmt ", 4) == 0) {
      if (formatFound || bytes < 16 || file.read(header, 16) != 16) return false;
      const uint16_t bitsPerSample = le16(header + 14);
      const uint16_t bytesPerSample = bitsPerSample / 8;
      if (le16(header) != 1 || le16(header + 2) != 1 ||
          le32(header + 4) != Config::WAV_SAMPLE_RATE ||
          (bitsPerSample != 8 && bitsPerSample != 16) ||
          le32(header + 8) != Config::WAV_SAMPLE_RATE * bytesPerSample ||
          le16(header + 12) != bytesPerSample) return false;
      info.bitsPerSample = bitsPerSample;
      formatFound = true;
    } else if (memcmp(header, "data", 4) == 0) {
      if (dataFound || !bytes) return false;
      info.offset = uint32_t(payload);
      info.bytes = bytes;
      dataFound = true;
    }
    pos = next;
  }
  return formatFound && dataFound &&
         info.bytes % (info.bitsPerSample / 8) == 0 &&
         file.seek(info.offset);
}
