#include <dummy.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_BROKER "test.mosquitto.org"
#define MQTT_PORT (1883)

#define MQTT_PUBLIC_TOPIC "uok/iot/OR83/SmartFlowerPot"
#define MQTT_SUBSCRIBE_TOPIC "uok/iot/OR83/SFPManualActivation"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

String SSID;
String WIFI_PWD;

// print recieved string callback method
void printRecievedString(const char* topic, byte* payload, unsigned int length)
{
  char str[length+1];
  memcpy(str, payload, length);
  str[length] = 0;
  Serial.begin(115200);
  Serial.println(str);
  Serial.end();
}

// publish recieved string to topic
void publishRecievedString(const char* topic, byte* payload, unsigned int length)
{
  char str[length+1];
  memcpy(str, payload, length);
  str[length] = 0;
  client.publish(MQTT_PUBLIC_TOPIC, str);
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
  Serial.println("Enter Frequency Of Watering Required In Hours:");
  while(Serial.available() <= 0)
  {}
  Serial.readString();
  Serial.println("Enter Ideal Temperature of the plant In Celsius:");
  while(Serial.available() <= 0)
  {}
  Serial.readString();
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
  client.setCallback(publishRecievedString);
}

void loop() {
  // MQTT loops
  client.loop();
  delay(1000);
}