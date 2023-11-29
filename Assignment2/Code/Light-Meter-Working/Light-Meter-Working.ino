#include <BH1750.h>
#include <Wire.h>

BH1750 lightMeter;

void setup() {
  // set up serial wire
  Serial.begin(9600);
  // set up lightmeter
  Wire.begin();
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
}

void loop() {
  // get lightmeter reading
  while (!lightMeter.measurementReady(true)) {
    yield();
  }
  float lux = lightMeter.readLightLevel(); // 500 - bright room
  Serial.print("Light: ");
  Serial.println(lux);
  lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
}
