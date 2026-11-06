#pragma once
#include "Config.h"
#include <string.h>

enum class Activity : uint16_t {
#define ACTIVIDAD(symbol, code, id, path) symbol = id,
#include "Activities.def"
#undef ACTIVIDAD
};

struct CommandEntry {
  const char *command;
  const char *label;
  Activity activity;
  const char *path;
};

// Los errores de longitud/rango se detectan al compilar una nueva actividad.
#define ACTIVIDAD(symbol, code, id, path) \
  static_assert(sizeof(code) - 1 == Config::COMMAND_LENGTH, "Codigo de dos digitos requerido"); \
  static_assert(code[0] >= '0' && code[0] <= '9' && code[1] >= '0' && code[1] <= '9', "Codigo no numerico"); \
  static_assert(id > 0 && id <= 99 && id == (code[0] - '0') * 10 + code[1] - '0', "ID no coincide con codigo"); \
  static_assert(sizeof(path) - 1 <= 31, "Ruta SPIFFS larga");
#include "Activities.def"
#undef ACTIVIDAD

// Encabezado independiente de Arduino: permite pruebas en un ordenador.
inline const CommandEntry *findCommand(const char *name) {
  static const CommandEntry table[] = {
#define ACTIVIDAD(symbol, code, id, path) {code, #symbol, Activity::symbol, path},
#include "Activities.def"
#undef ACTIVIDAD
  };
  for (const auto &entry : table) {
    if (strcmp(entry.command, name) == 0) return &entry;
  }
  return nullptr;
}

// Copia solo dos digitos. No aceptar numeros binarios, NUL, espacios o saltos
// de linea: "01" significa los bytes 0x30, 0x31, no un byte con valor 1.
inline bool parseCommand(const char *input, size_t length, char *output) {
  if (length != Config::COMMAND_LENGTH) return false;
  for (size_t i = 0; i < Config::COMMAND_LENGTH; ++i) {
    if (input[i] < '0' || input[i] > '9') return false;
    output[i] = input[i];
  }
  output[Config::COMMAND_LENGTH] = '\0';
  return true;
}
