#pragma once
#include "Config.h"
#include <stdint.h>
struct IncomingCommand {
  char bytes[Config::COMMAND_LENGTH];
  uint8_t length;
  bool invalid;
};
namespace BleTransport {
bool begin(const char *initialStatus);
void update();
bool receive(IncomingCommand &message);
bool takeOverflow();
void publish(const char *status);
}
