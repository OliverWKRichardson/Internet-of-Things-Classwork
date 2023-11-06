#include <dummy.h>
#include <BLEDevice.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN (2)  // LED Pin on DevKit V1 boards
#endif

// See https://www.uuidgenerator.net/ for generating UUIDs:
BLEUUID serviceID("83f40d2a-0d96-42fa-b5eb-dc499ffbc314");
BLEUUID readonlyCharID("8b580f9f-fe9b-4c0e-b381-5fa381ebad5b");
BLEUUID writableCharID("1cecd40b-0c07-4ea9-8a97-620b0370bedb");

//A global store for the data in our writable characteristic
uint8_t writable_store[1];

//Create a callback handler
MyCallbacks cb;


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

  //An example writable characteristic
  BLECharacteristic *writeCharacteristic = pService->createCharacteristic(writableCharID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);  //bit-wise OR is used to enable both properties.
  writeCharacteristic->setValue(writable_store, 1);
  writeCharacteristic->setCallbacks(&cb);


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

  //Enable the LED
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  delay(1000);
}

//A class definition for handling callbacks .
class MyCallbacks : public BLECharacteristicCallbacks {
  // This method will be call to perform writes to characteristic
  void onWrite(BLECharacteristic *pCharacteristic) {
    if (writableCharID.equals(pCharacteristic->getUUID())) {  //is it our characteristic ?
      uint8_t *value = pCharacteristic->getData();            //get the data associated with it .
      digitalWrite(LED_BUILTIN, value[0]?HIGH:LOW);       //turn LED off if 0; on otherwise.
    }
  }
};