#include <SSens.h>
#include <Ticker.h>
#include <dummy.h>

#define SENSOR_PIN 12
#define sample_period_ms 100
#define threshold 4.8
#define LED_BUILTIN 2
#define USB_BAUD_RATE 9600

Ticker sampler;
SSens sensor(SENSOR_PIN);


// the setup function runs once when you press reset or power the board

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(USB_BAUD_RATE);
  sensor.begin();
  sampler.attach_ms(sample_period_ms, sample_interrupt);
}

// the loop function runs over and over again forever
void loop() {
}



void sample_interrupt()
{
  float reading = sensor.getSignal();
  if (reading > threshold)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(reading);
    Serial.print("  ");
    Serial.println(true);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print(reading);
    Serial.print("  ");
    Serial.println(false);
  }
}