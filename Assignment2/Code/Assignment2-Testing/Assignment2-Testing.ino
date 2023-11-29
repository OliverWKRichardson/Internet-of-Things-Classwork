#include <dummy.h>
#include <WiFi.h>
#include <PubSubClient.h>

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
int secondsToMillis = 1000 ;
unsigned long lastwateredTime;
unsigned long nextwateredTime;

// flags for sensors
static volatile uint8_t pflags = 0x00;
#define PFLAG_WATER (0x01)
#define PFLAG_TEMP (0x02)
#define PFLAG_LIGHT (0x03)

// manually set variables with mqtt msgs
void manualActivation() {
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
  // get wifi and password from serial line
  Serial.println("Enter Wifi Name:");
  while (Serial.available() <= 0) {}
  SSID = Serial.readString();
  Serial.println("Enter Wifi Password:");
  while (Serial.available() <= 0) {}
  WIFI_PWD = Serial.readString();
  // get main variables for the plant
  Serial.println("How Often Does The Plant Need Watering In Hours:");
  while (Serial.available() <= 0) {}
  waterRate = stoi(Serial.readString());
  Serial.println("Ideal Temperature Of Plant In Celsius:");
  while (Serial.available() <= 0) {}
  idealTemp = stoi(Serial.readString());
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
  Serial.end();
  // subscribe to topic and set callback function to call when msg arrives
  client.subscribe(MQTT_SUBSCRIBE_TOPIC);
  client.setCallback(manualActivation);
}

void loop() {
  // MQTT loops
  client.loop();
  // check flags
  for (;;) {
    if (!pflags) {
      continue;
    }
    noInterrupts();
    uint8_t cflags = pflags;
    pflags = 0x00;
    interrupts();
    if (cflags & PFLAG_WATER) {
      lastwateredTime = millis();
      nextwateredTime = lastwateredTime + (waterRate * hoursToMillis);
      client.publish(MQTT_PUBLIC_TOPIC, "The Plant has been watered, water it again in" + to_string(waterRate) + " hours.");
    }
    if (cflags & PFLAG_TEMP) {
      client.publish(MQTT_PUBLIC_TOPIC, "The Plant's Temperature is +/- 3 degrees celsius from it's ideal temperature, it would be a good idea to move it.");
    }
    if (cflags & PFLAG_LIGHT) {
      client.publish(MQTT_PUBLIC_TOPIC, "The Plant is in the dark, if it is not night then you might want to move it.");
    }
  }
  // wait 
  delay(10*secondsToMillis); //replace with sleep once interupts are added
}


