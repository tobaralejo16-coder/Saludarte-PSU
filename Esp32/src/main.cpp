#include <Arduino.h>
#include "AudioPlayer.h"
#include "BleTransport.h"

namespace {
AudioPlayer player;

bool parseVolumeCommand(const IncomingCommand &message, char &step) {
  if (message.invalid || message.length != Config::COMMAND_LENGTH ||
      message.bytes[0] != 'V' || message.bytes[1] < '0' ||
      message.bytes[1] > '9') {
    return false;
  }
  step = message.bytes[1];
  return true;
}

void publishActivityStatus(const char *status, uint16_t id) {
  // READ devuelve el ultimo estado; los codigos conservan dos digitos.
  // Ejemplo para DESAYUNAR: OK:02, PLAY:02, DONE:02.
  char message[21];
  snprintf(message, sizeof(message), "%s:%02u", status, unsigned(id));
  BleTransport::publish(message);
}

void handleCommand(const IncomingCommand &message) {
  if (message.invalid || message.length != Config::COMMAND_LENGTH) {
    BleTransport::publish("ERR:FORMAT");
    return;
  }

  if (message.bytes[0] == 'V') {
    char step = '\0';
    if (!parseVolumeCommand(message, step)) {
      BleTransport::publish("ERR:FORMAT");
      return;
    }
    unsigned level = unsigned(step - '0') * 11;
    if (step == '9') level = 100;
    player.setVolumePercent(level);
    Serial.printf("Comando recibido: V%c\n", step);
    Serial.printf("Volumen actualizado: %u%%\n", level);
    BleTransport::publish("OK:VOL");
    return;
  }

  char name[Config::COMMAND_LENGTH + 1];
  if (!parseCommand(message.bytes, message.length, name)) {
    BleTransport::publish("ERR:FORMAT");
    return;
  }
  Serial.printf("Comando recibido: %s\n", name);
  const CommandEntry *entry = findCommand(name);
  if (!entry) { BleTransport::publish("ERR:UNKNOWN"); return; }
  Serial.printf("Actividad: %s\n", entry->label);
  const uint16_t id = uint16_t(entry->activity);
  if (!player.ready()) { publishActivityStatus("ERR:PLAYER", id); return; }
  if (!player.enqueue(entry)) { publishActivityStatus("ERR:QUEUE", id); return; }
  // Los eventos del trabajador se publican mas tarde en loop(): OK siempre
  // precede a PLAY/DONE, aunque la tarea de audio empiece inmediatamente.
  publishActivityStatus("OK", id);
}
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nIniciando SALUDARTE (I2S MAX98357A x3)...");
  const bool ready = player.begin();
  // BLE sigue disponible si falta SPIFFS, para comunicar el error al cliente.
  if (!BleTransport::begin(ready ? "READY" : "ERR:PLAYER")) {
    Serial.println("Error fatal: no se pudo inicializar BLE; revisar registros previos");
    for (;;) delay(1000);
  }
}

void loop() {
  BleTransport::update();
  // Limitar el trabajo por vuelta evita inanicion bajo escrituras continuas.
  IncomingCommand message{};
  for (unsigned i = 0; i < Config::QUEUE_SIZE && BleTransport::receive(message); ++i)
    handleCommand(message);
  if (BleTransport::takeOverflow()) BleTransport::publish("ERR:INPUT_QUEUE");
  AudioEvent event{};
  for (unsigned i = 0; i < 32 && player.poll(event); ++i)
    publishActivityStatus(event.status, event.id);
  delay(2);
}
