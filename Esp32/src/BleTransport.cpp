#include "BleTransport.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

namespace {
QueueHandle_t incoming = nullptr;
SemaphoreHandle_t statusMutex = nullptr;
NimBLECharacteristic *characteristic = nullptr;
NimBLEAdvertising *advertising = nullptr;
std::atomic<bool> connected{false}, disconnected{false}, overflow{false};
char lastStatus[21] = "READY";
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *, NimBLEConnInfo &) override {
    connected.store(true);
    Serial.println("BLE conectado");
  }
  void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int) override {
    connected.store(false);
    disconnected.store(true);
    Serial.println("BLE desconectado; esperando reconexion");
  }
};
class CommandCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *source, NimBLEConnInfo &) override {
    // Callback corta: copiar y encolar; sin acceso al disco ni reproduccion.
    const std::string value = source->getValue();
    IncomingCommand message{};
    message.invalid = value.size() != Config::COMMAND_LENGTH;
    if (!message.invalid) {
      message.length = value.size();
      memcpy(message.bytes, value.data(), value.size());
    }
    if (xQueueSend(incoming, &message, 0) != pdTRUE) overflow.store(true);
  }
  void onRead(NimBLECharacteristic *source, NimBLEConnInfo &) override {
    // WRITE reemplaza el valor GATT. READ devuelve el estado publicado,
    // incluso antes de que loop() procese el comando entrante.
    xSemaphoreTake(statusMutex, portMAX_DELAY);
    source->setValue(lastStatus);
    xSemaphoreGive(statusMutex);
  }
};
}
bool BleTransport::begin(const char *initialStatus) {
  incoming = xQueueCreate(16, sizeof(IncomingCommand));
  statusMutex = xSemaphoreCreateMutex();
  if (!incoming || !statusMutex) return false;
  snprintf(lastStatus, sizeof(lastStatus), "%s", initialStatus);
  if (!NimBLEDevice::init(Config::DEVICE_NAME)) return false;
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Potencia máxima de transmisión BLE (+9dBm)
  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  NimBLEService *service = server->createService(Config::SERVICE_UUID);
  characteristic = service->createCharacteristic(
      Config::CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ |
      NIMBLE_PROPERTY::WRITE);
  // Protocolo de SaludarteApp: READ | WRITE, sin NOTIFY ni suscripcion CCCD.
  characteristic->setCallbacks(new CommandCallbacks());
  characteristic->setValue(lastStatus);
  service->start();
  advertising = NimBLEDevice::getAdvertising();
  // UUID de 128 bits + flags en advertising; nombre en scan response separado.
  NimBLEAdvertisementData advertisement, response;
  advertisement.setFlags(0x06);
  advertisement.setCompleteServices(NimBLEUUID(Config::SERVICE_UUID));
  response.setName(Config::DEVICE_NAME);
  advertising->setAdvertisementData(advertisement);
  advertising->setScanResponseData(response);
  advertising->enableScanResponse(true);
  advertising->setMinInterval(0x20); // Anuncios rápidos (20 ms)
  advertising->setMaxInterval(0x40); // 40 ms
  if (!advertising->start()) return false;
  Serial.printf("BLE listo; anunciando %s a máxima potencia\n", Config::DEVICE_NAME);
  return true;
}
void BleTransport::update() {
  static bool pending = false;
  static uint32_t disconnectedAt = 0;
  if (disconnected.exchange(false)) { pending = true; disconnectedAt = millis(); }
  // Dejar terminar la desconexion antes de reiniciar advertising.
  if (pending && millis() - disconnectedAt >= 250) {
    if (connected.load() || advertising->start()) pending = false;
    else {
      disconnectedAt = millis();
      Serial.println("Error: reintentando advertising BLE");
    }
  }
}
bool BleTransport::receive(IncomingCommand &message) {
  return xQueueReceive(incoming, &message, 0) == pdTRUE;
}
bool BleTransport::takeOverflow() { return overflow.exchange(false); }
void BleTransport::publish(const char *status) {
  xSemaphoreTake(statusMutex, portMAX_DELAY);
  snprintf(lastStatus, sizeof(lastStatus), "%s", status);
  xSemaphoreGive(statusMutex);
  // onRead() restaura este estado desde la tarea BLE. No modificar el valor
  // GATT aqui: podria reemplazar una escritura antes de que onWrite() la copie.
  Serial.printf("BLE estado: %s\n", status);
}

