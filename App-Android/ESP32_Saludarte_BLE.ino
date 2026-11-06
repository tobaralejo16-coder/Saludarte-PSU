/*
  SaludarteApp - ESP32-C6 Firmware
  Recibe códigos por BLE para activar recordatorios de audio.

  Placa: ESP32-C6-DevKitC-1
  Librería: ESP32 BLE Arduino (integrada en el core)
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// UUIDs deben coincidir con la App Android
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool deviceConnected = false;

// Clase para manejar eventos de escritura desde la App
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.print("Código recibido: ");
        Serial.println(value.c_str());

        // LÓGICA DE AUDIO
        if (value == "01") {
          Serial.println("Acción: Reproducir audio 'Despertar'");
          // reproducirAudio(1);
        } else if (value == "02") {
          Serial.println("Acción: Reproducir audio 'Desayunar'");
        } else if (value == "99") {
          Serial.println("ALERTA: ¡EMERGENCIA ACTIVADA!");
          // Bucle de sonido fuerte o alerta continua
        }
        // ... agregar el resto de actividades
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("App conectada");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("App desconectada. Reiniciando anuncios...");
      pServer->getAdvertising()->start();
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando BLE Saludarte...");

  // Crear el dispositivo BLE
  BLEDevice::init("SALUDARTE_ESP32");

  // Crear el servidor BLE
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crear el servicio BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Crear la característica para recibir datos (WRITE)
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Iniciar el servicio
  pService->start();

  // Configurar anuncios (Advertising)
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("Esperando conexión de la App...");
}

void loop() {
  // El loop queda libre para otras tareas,
  // la recepción se maneja por callbacks.
  delay(1000);
}
