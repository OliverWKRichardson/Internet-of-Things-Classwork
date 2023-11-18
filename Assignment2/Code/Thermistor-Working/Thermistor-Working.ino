#define KNOWN_TEMP 12
#define KNOWN_RESISTANCE 1950
#define BETA_COEFFICIENT 3500

int ThermistorPin = 26;
float FixedResistor = 1000;

void setup() {
Serial.begin(9600);
}

void loop() {
  float reading;
  reading = analogRead(ThermistorPin);
  Serial.print("Reading: "); 
  Serial.print(reading);   
  // convert reading to resistance
  reading = (4095 / reading) - 1;
  reading = FixedResistor / reading;
  Serial.print("    Resistance: "); 
  Serial.print(reading); 
  // get temperature
  float steinhart;
  steinhart = reading/KNOWN_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= BETA_COEFFICIENT;
  steinhart += 1.0 / (KNOWN_TEMP + 273.15);
  steinhart = 1.0 / steinhart;
  float temperature = steinhart - 273.15;
  Serial.print("    Temperature: "); 
  Serial.println(temperature); 

  delay(500);
}