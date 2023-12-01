#include <dummy.h>
#include <WiFi.h>
#include <PubSubClient.h>
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

// MQTT set up
#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)

// MQTT topic set up
#define MQTT_PUBLIC_TOPIC "uok/iot/OR83/SmartFlowerPot"
#define MQTT_SUBSCRIBE_TOPIC "uok/iot/OR83/SFPManualActivation"

// wifi set up
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// wifi variables
String SSID;
String WIFI_PWD;

// main plant variables
int waterRate;
int idealTemp;
int hoursToMillis = 3600000;
int secondsToMillis = 1000;
unsigned long lastwateredTime;
unsigned long nextwateredTime;

// flags for sensors
static volatile uint8_t pflags = 0x00;
#define PFLAG_WATER (0x01)
#define PFLAG_TEMP (0x02)
#define PFLAG_LIGHT (0x03)

// manually set variables with mqtt msgs
void manualActivation(const char* topic, byte* payload, unsigned int length) {
  char str[length + 1];
  memcpy(str, payload, length);
  str[length] = 0;
  if (str == "water") {
    pflags |= PFLAG_WATER;
  } else if (str == "temp") {
    pflags |= PFLAG_TEMP;
  } else if (str == "light") {
    pflags |= PFLAG_LIGHT;
  }
}

void setup() {
  // set up serial line
  Serial.begin(115200);
  Serial.flush();
  // set up save for settings
  preferences.begin("smartPlantPot", false);
  if (!preferences.getBool("setup", false)) 
  {
    // get wifi and password from serial line
    Serial.println("Enter Wifi Name:");
    while (Serial.available() <= 0) {}
    SSID = Serial.readString();
    preferences.putString("SSID", SSID);
    Serial.println("Enter Wifi Password:");
    while (Serial.available() <= 0) {}
    WIFI_PWD = Serial.readString();
    preferences.putString("WIFI_PWD", WIFI_PWD);
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
    SSID = preferences.getString("SSID");
    WIFI_PWD = preferences.getString("WIFI_PWD");
    waterRate = preferences.getInt("waterRate");
    idealTemp = preferences.getInt("idealTemp");
  }
  preferences.end();
  // connect to the wifi
  Serial.print("Connecting to ");
  Serial.print(SSID);
  WiFi.begin(SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(" . ");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // conect to MQTT
  client.setServer(MQTT_BROKER, MQTT_PORT);
  while (!client.connected()) {
    if (client.connect(("ESP32-" + String(random(0xffff), HEX)).c_str())) {
      Serial.println("MQTT connected.");
    } else {
      Serial.printf(" failed , rc=%d try again in 5 seconds", client.state());
      delay(5000);
    }
  }
  // subscribe to topic and set callback function to call when msg arrives
  client.subscribe(MQTT_SUBSCRIBE_TOPIC);
  client.setCallback(manualActivation);
  //Setup Sensors
  Wire.begin();
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
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
  Serial.end();
  // check flags WIP temp removed
  //for (;;) {
  //  if (!pflags) {
  //    continue;
  //  }
  //  noInterrupts();
  //  uint8_t cflags = pflags;
  //  pflags = 0x00;
  //  interrupts();
  //  if (cflags & PFLAG_WATER) {
  //    lastwateredTime = millis();
  //    nextwateredTime = lastwateredTime + (waterRate * hoursToMillis);
  //    String watermsg = "The Plant has been watered, water it again in ";
  //    watermsg += waterRate;
  //    watermsg += " hours.";
  //    client.publish(MQTT_PUBLIC_TOPIC, watermsg.c_str());
  //  }
  //  if (cflags & PFLAG_TEMP) {
  //    client.publish(MQTT_PUBLIC_TOPIC, "The Plant's Temperature is +/- 3 degrees celsius from it's ideal temperature, it would be a good idea to move it.");
  //  }
  //  if (cflags & PFLAG_LIGHT) {
  //    client.publish(MQTT_PUBLIC_TOPIC, "The Plant is in the dark, if it is not night then you might want to move it.");
  //  }
  //}
  // wait
  delay(1 * secondsToMillis);  //replace with sleep
}

void loop()
{

}
