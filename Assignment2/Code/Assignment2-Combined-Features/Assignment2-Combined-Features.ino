#include <dummy.h>
#include <BH1750.h>
#include <Wire.h>
#include <Preferences.h>

// save data set up
Preferences preferences;

// Sensors Setup
// LIGHT METER
BH1750 lightMeter;
// HUMIDITY SENSOR
#define HUMIDITY_PIN 25                    // pin that the humidity data wire is connected to
#define CIRCUIT_RESISTOR_RESISTANCE 10000  // reistance of the resistor use when building the circuit
float average;
int count = 0;
int initial = 10;
bool spike;
// THERMISTOR
#define KNOWN_TEMP 12
#define KNOWN_RESISTANCE 1950
#define BETA_COEFFICIENT 3500
int ThermistorPin = 26;
float FixedResistor = 1000;

// main plant variables
int waterRate;
int idealTemp;
int hoursToMillis = 3600000;
int secondsToMillis = 1000;
unsigned long lastwateredTime;
unsigned long nextwateredTime;

void setup() {
  // set up serial line
  Serial.begin(115200);
  Serial.flush();
  // set up save for settings
  preferences.begin("smartPlantPot", false);
  if (!preferences.getBool("setup", false)) 
  {
    // get main variables for the plant
    Serial.println("How Often Does The Plant Need Watering In Hours:");
    while (Serial.available() <= 0) {}
    waterRate = Serial.readString().toInt();
    preferences.putInt("waterRate", waterRate);
    Serial.println("Ideal Temperature Of Plant In Celsius:");
    while (Serial.available() <= 0) {}
    idealTemp = Serial.readString().toInt();
    preferences.putInt("idealTemp", idealTemp);
    preferences.putBool("setup", true);
  }
  else
  {
    waterRate = preferences.getInt("waterRate");
    idealTemp = preferences.getInt("idealTemp");
  }
  preferences.end();
  //Setup Sensors
  Wire.begin();
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
}

void loop()
{
  // check sensors
  // LIGHT
  while (!lightMeter.measurementReady(true)) {
    yield();
  }
  float lux = lightMeter.readLightLevel();  // 500 - bright room
  Serial.print("Light: ");
  Serial.print(lux);
  lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
  // HUMIDITY
  spike = false;
  float reading;
  reading = analogRead(HUMIDITY_PIN);
  Serial.print("    Humidity Raw: ");
  Serial.print(reading);
  if (initial > 0) {
    average = ((average * count) + reading) / (count + 1);
    count += 1;
    initial -= 1;
  } else if ((reading < (average * 1.1)) && (reading > (average * 0.9))) {
    average = ((average * count) + reading) / (count + 1);
    count += 1;
  } else {
    spike = true;
  }
  Serial.print("    Humidity Spike?: ");
  Serial.print(spike);
  Serial.print("    Humidity Average: ");
  Serial.print(average);
  // TEMP
  float reading2;
  reading2 = analogRead(ThermistorPin);
  // convert reading to resistance
  reading2 = (4095 / reading2) - 1;
  reading2 = FixedResistor / reading2;
  // get temperature
  float steinhart;
  steinhart = reading2 / KNOWN_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= BETA_COEFFICIENT;
  steinhart += 1.0 / (KNOWN_TEMP + 273.15);
  steinhart = 1.0 / steinhart;
  float temperature = steinhart - 273.15;
  Serial.print("    Temperature: ");
  Serial.println(temperature);
  Serial.flush();
  // wait
  delay(1 * secondsToMillis);  //replace with sleep
}
