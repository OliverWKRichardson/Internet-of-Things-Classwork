#include <dummy.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <BH1750.h>
#include <DHT.h>

#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)

#define MQTT_PUBLIC_TOPIC "uok/iot/OR83/SmartFlowerPot"
#define MQTT_SUBSCRIBE_TOPIC "uok/iot/OR83/SFPManualActivation"

DHTNEW DHT(0);
BH1750 lightMeter;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

String SSID;
String WIFI_PWD;

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
  while(Serial.available() <= 0)
  {}
  SSID = Serial.readString();
  Serial.println("Enter Wifi Password:");
  while(Serial.available() <= 0)
  {}
  WIFI_PWD = Serial.readString();
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
  client.setCallback(printRecievedString);
  // set up light sensor
  lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
  lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
  // set up temperture and humidity sensor
  DHT.setWaitForReading(true);
  DHT.setType(22);
}

void loop() {
  // MQTT loops
  client.loop();
  // main loop
  Serial.begin(115200);
  // temp and humidity
  DHT.read();
  float temp = DHT.getTemperature();
  float humid = DHT.getHumidity();
  Serial.print("Temperture = ");
  Serial.flush();
  Serial.print(temp);
  Serial.flush();
  Serial.print("    Humidity = ");
  Serial.flush();
  Serial.print(humid);
  Serial.flush();
  Serial.print("    Sensor type: ");
  Serial.flush();
  Serial.println(DHT.getType());
  Serial.flush();
  // light
  while (!lightMeter.measurementReady(true)) 
  {}
  float lux = lightMeter.readLightLevel();
  Serial.print("    Light = ");
  Serial.flush();
  Serial.println(lux);
  Serial.flush();
  Serial.end();
  delay(2000);
}