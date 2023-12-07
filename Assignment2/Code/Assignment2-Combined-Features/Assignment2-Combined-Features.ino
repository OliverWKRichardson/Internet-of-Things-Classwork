#include <dummy.h>
#include <BH1750.h>
#include <Wire.h>

// LIGHT METER variables
BH1750 lightMeter;

// HUMIDITY SENSOR variables
#define HUMIDITY_PIN A0                    // pin that the humidity data wire is connected to
#define HUMIDITY_RESISTOR_RESISTANCE 1000000  // reistance of the resistor use when building the circuit
float average = 0;
int count = 0;
int initial = 10;
bool spike = false;

// THERMISTOR variables
#define KNOWN_TEMP 6
#define KNOWN_RESISTANCE 20000
#define BETA_COEFFICIENT 3500
#define THERMISTOR_PIN A1
#define THERMISTOR_RESISTOR_RESISTANCE 1000

// Timer Variables
#define DELAY_TIME 1000

void setup() {
  // set up serial line
  Serial.begin(115200);
  Serial.flush();
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
  float reading;
  reading = analogRead(HUMIDITY_PIN);
  // convert reading to resistance
  reading = (4095 / reading) - 1;
  reading = HUMIDITY_RESISTOR_RESISTANCE / reading;
  Serial.print("    Humidity Resistance: ");
  Serial.print(reading);
  // detect spike in readings
  spike = false;
  if (initial > 0) {
    average = ((average * count) + reading) / (count + 1);
    count += 1;
    initial -= 1;
  } else if ((reading < (average * 1.15)) && (reading > (average * 0.85))) {
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
  reading2 = analogRead(THERMISTOR_PIN);
  // convert reading to resistance
  reading2 = (4095 / reading2) - 1;
  reading2 = THERMISTOR_RESISTOR_RESISTANCE / reading2;
  Serial.print("    temp resistance: ");
  Serial.print(reading2);
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
  delay(DELAY_TIME);
}
