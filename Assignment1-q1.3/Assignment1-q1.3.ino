#include <dummy.h>
#include <BLEDevice.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN (2)  // LED Pin on DevKit V1 boards
#endif

// See https://www.uuidgenerator.net/ for generating UUIDs:
BLEUUID serviceID("83f40d2a-0d96-42fa-b5eb-dc499ffbc314");
BLEUUID readonlyCharID("8b580f9f-fe9b-4c0e-b381-5fa381ebad5b");
BLEUUID writableCharID("1cecd40b-0c07-4ea9-8a97-620b0370bedb");
BLEUUID dynamicReadonlyCharID("1b734f88-1f69-4a6b-bcc0-475681b9f8ad");
BLEUUID notifyReadonlyCharID("1b98708d-3302-4264-b0da-5325ea877d4f");

//A global store for the data in our writable characteristic
uint8_t writable_store[1];

//A class definition for handling callbacks .
class MyCallbacks : public BLECharacteristicCallbacks {
  // This method will be call to perform writes to characteristic
  void onWrite(BLECharacteristic *pCharacteristic) {
    if (writableCharID.equals(pCharacteristic->getUUID())) {  //is it our characteristic ?
      uint8_t *value = pCharacteristic->getData();            //get the data associated with it .
      digitalWrite(LED_BUILTIN, value[0] ? HIGH : LOW);       //turn LED off if 0; on otherwise.
    }
  }
};
//Create a callback handler
MyCallbacks cb;

//incrementing value
uint8_t incrementalValue = 0;
// pointers to characteristics
BLECharacteristic *notifyReadCharacteristic;
BLECharacteristic *dynamicReadCharacteristic;


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

  //A Dynamic read-only characteristic
  dynamicReadCharacteristic = pService->createCharacteristic(dynamicReadonlyCharID, BLECharacteristic::PROPERTY_READ);

  // A Notify Read Only Characteristic
  notifyReadCharacteristic = pService->createCharacteristic(notifyReadonlyCharID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);

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
  // notify characteristic
  notifyReadCharacteristic->setValue((uint8_t*)&incrementalValue, 4);
  notifyReadCharacteristic->notify();
  incrementalValue++;

  // dynamic characteristc
  long rawtime = millis();
  uint8_t *time = (uint8_t*) &rawtime;
  dynamicReadCharacteristic->setValue((uint8_t*)&time, 4);

  delay(1000);
}