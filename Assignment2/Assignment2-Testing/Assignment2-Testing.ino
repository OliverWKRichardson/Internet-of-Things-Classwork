#include <dummy.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <dht.h>
#include <BH1750.h>

#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)

#define MQTT_PUBLIC_TOPIC "uok/iot/OR83/SmartFlowerPot"
#define MQTT_SUBSCRIBE_TOPIC "uok/iot/OR83/SFPManualActivation"

#define HUMIDITY_TEMP_DATAPIN 1
dht DHT;
BH1750 lightMeter(0x23);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// print recieved string
void printRecievedString(const char* topic, byte* payload, unsigned int length)
{
  char str[length+1];
  memcpy(str, payload, length);
  str[length] = 0;
  Serial.begin(115200);
  Serial.println(str);
  Serial.end();
}

void setup() {
  // set up serial line
  Serial.begin(115200);
  // get wifi and password from serial line
  Serial.println("Enter Wifi Name:");
  SSID = Serial.readString()
  Serial.println("Enter Wifi Password:");
  WIFI_PWD = Serial.readString()

  // connect to the wifi
  Serial.print("Connecting to " SSID);
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
  client.setCallback(printRecievedString);
}

void loop() {
  // MQTT loops
  client.loop();
  // main loop
  Serial.begin(115200)
  // temp and humidity
  int readData = DHT.read22(HUMIDITY_TEMP_DATAPIN);
  float temp = DHT.temperature;
  float humid = DHT.humidity;
  Serial.print("Temperture = ");
  Serial.print(t);
  Serial.print("    Humidity = ");
  Serial.print(h);
  Serial.println("");
  Serial.end();
  // light
  lightmeter.begin();
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  delay(2000)
}