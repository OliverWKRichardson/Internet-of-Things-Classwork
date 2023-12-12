#include <dummy.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <Wire.h>
#include <Preferences.h>

// save data set up
Preferences preferences;

// Sensors Setup
// LIGHT METER variables
BH1750 lightMeter;

// HUMIDITY SENSOR variables
#define HUMIDITY_PIN A0                       // pin that the humidity data wire is connected to
#define HUMIDITY_RESISTOR_RESISTANCE 1000000  // reistance of the resistor use when building the circuit
float average = 0;
int count = 0;
int initial = 10;
bool spike = false;

// THERMISTOR variables
#define KNOWN_TEMP 20.3
#define KNOWN_RESISTANCE 6200
#define BETA_COEFFICIENT 26367.1
#define THERMISTOR_PIN A1
#define THERMISTOR_RESISTOR_RESISTANCE 1000

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
int waterAlert;
int idealTemp;
int hoursToMillis = 3600000;
int secondsToMillis = 1000;
int uS_TO_S_FACTOR = 1000000;
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

// wake up reason method
String return_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: return "Wakeup caused by external signal using RTC_IO";
    case ESP_SLEEP_WAKEUP_EXT1: return "Wakeup caused by external signal using RTC_CNTL";
    case ESP_SLEEP_WAKEUP_TIMER: return "Wakeup caused by timer";
    case ESP_SLEEP_WAKEUP_TOUCHPAD: return "Wakeup caused by touchpad";
    case ESP_SLEEP_WAKEUP_ULP: return "Wakeup caused by ULP program";
    default: return "Wakeup was not caused by deep sleep: " + wakeup_reason;
  }
}

int readInAnger(int pin) {
  // take a few readings and average them to avoid errors from readings
  float total = 0;
  int amount = 20;
  for (int i = 0; i < amount; i++) {
    total += analogRead(pin);
    delay(5);
  }
  return total/amount;
}

void watered(){
  // if humidity spike then trip flag
  if(spike) 
  {
    pflags |= PFLAG_WATER;
  }
}

void setup() {
  // set up interrupts
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,1); // wake up if watered
  attachInterrupt(digitalPinToInterrupt(26),watered,RISING); // if watered run watered()
  // set up serial line
  Serial.begin(115200);
  Serial.flush();
  // print wake up reason
  Serial.println(return_wakeup_reason());
  // save for settings
  preferences.begin("smartPlantPot", false);
  // get settings
  if (return_wakeup_reason() == "Wakeup was not caused by deep sleep: " || preferences.getBool("setup", true)) {  // if doesn't have settings saved or is not a deep sleep loop start up
    Serial.println("Initial Start Up");
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
    waterAlert = waterRate;
    Serial.println("Ideal Temperature Of Plant In Celsius:");
    while (Serial.available() <= 0) {}
    idealTemp = Serial.readString().toInt();
    preferences.putInt("idealTemp", idealTemp);
    preferences.putBool("setup", false);
    preferences.putFloat("average", 0);
    preferences.putInt("count", 0);
    delay(2000);
  } else {  // load settings
    SSID = preferences.getString("SSID");
    WIFI_PWD = preferences.getString("WIFI_PWD");
    waterRate = preferences.getInt("waterRate");
    waterAlert = preferences.getInt("waterAlert");
    idealTemp = preferences.getInt("idealTemp");
  }
  preferences.end();
  // Setup Sensors
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
  preferences.begin("smartPlantPot", false);
  average = preferences.getFloat("average");
  count = preferences.getInt("count");
  preferences.end();
  float reading;
  reading = readInAnger(HUMIDITY_PIN);
  // convert reading to resistance
  reading = (4095 / reading) - 1;
  reading = HUMIDITY_RESISTOR_RESISTANCE / reading;
  Serial.print("    Humidity Res: ");
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
  Serial.print("    Humidity Avg: ");
  Serial.print(average);
  // TEMP
  float reading2;
  reading2 = readInAnger(THERMISTOR_PIN);
  // convert reading to resistance
  reading2 = (4095 / reading2) - 1;
  reading2 = THERMISTOR_RESISTOR_RESISTANCE / reading2;
  Serial.print("    Temperature Res: ");
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
  // check readings
  if(temperature > idealTemp + 3 || temperature < idealTemp - 3 )
  {
    pflags |= PFLAG_TEMP;
  }
  if(lux < 50)
  {
    pflags |= PFLAG_LIGHT;
  }
  if(spike)
  {
    waterAlert = waterRate;
    pflags |= PFLAG_WATER;
  }
  if(waterAlert == 0)
  {
    waterAlert = waterRate;
    pflags |= PFLAG_WATER;
  }
  // check flags
  if (pflags) {
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
    // connect to MQTT
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
    Serial.flush();
    Serial.end();
    noInterrupts();
    uint8_t cflags = pflags;
    pflags = 0x00;
    interrupts();
    if (cflags & PFLAG_WATER) {
      lastwateredTime = millis();
      nextwateredTime = lastwateredTime + (waterRate * hoursToMillis);
      String watermsg = "The Plant has been watered, water it again in ";
      watermsg += waterRate;
      watermsg += " hours.";
      client.publish(MQTT_PUBLIC_TOPIC, watermsg.c_str());
    }
    if (cflags & PFLAG_TEMP) {
      client.publish(MQTT_PUBLIC_TOPIC, "The Plant's Temperature is +/- 3 degrees celsius from it's ideal temperature, it would be a good idea to move it.");
    }
    if (cflags & PFLAG_LIGHT) {
      client.publish(MQTT_PUBLIC_TOPIC, "The Plant is in the dark, if it is not night then you might want to move it.");
    }
  }
  preferences.begin("smartPlantPot", false);
  preferences.putFloat("average", average);
  preferences.putInt("count", count);
  if(return_wakeup_reason() == "Wakeup caused by timer")
  {
    preferences.putInt("waterAlert", waterAlert-1);
  }
  preferences.end();
  esp_sleep_enable_timer_wakeup(60 * 60 * uS_TO_S_FACTOR);
  // wait
  esp_deep_sleep_start();
}

void loop() {
}
