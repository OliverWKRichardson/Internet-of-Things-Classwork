#include <dummy.h>
#include <BLEDevice.h>


BLEUUID serviceID("83f40d2a-0d96-42fa-b5eb-dc499ffbc314");
BLEUUID readonlyCharID("8b580f9f-fe9b-4c0e-b381-5fa381ebad5b");


void setup() {
  //Set up device
  BLEDevice::init("OR83");
  // Set up the BLE Device in Server mode
  BLEServer *pServer = BLEDevice::createServer();
  // Each BLE Server can have multiple services
  BLEService *pService = pServer->createService(serviceID);

  //An example read-only characteristic
  BLECharacteristic *readCharacteristic = pService->createCharacteristic(readonlyCharID, BLECharacteristic::PROPERTY_READ);
  readCharacteristic->setValue("Testing");

  //Start the service
  pService->start();

  //Advertising config
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceID);
  pAdvertising->setScanResponse(true);
  // Functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  // Start advertising the device
  BLEDevice::startAdvertising();
}

void loop() {
  delay(1000);
}
