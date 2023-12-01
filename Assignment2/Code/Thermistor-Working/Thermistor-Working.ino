#define KNOWN_TEMP 12
#define KNOWN_RESISTANCE 1950
#define BETA_COEFFICIENT 3500

int ThermistorPin = 26;
float FixedResistor = 1000;

void setup() {
Serial.begin(9600);
}

void loop() {
  float reading2;
  reading2 = analogRead(ThermistorPin); 
  // convert reading to resistance
  reading2 = (4095 / reading2) - 1;
  reading2 = FixedResistor / reading2;
  // get temperature
  float steinhart;
  steinhart = reading2/KNOWN_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= BETA_COEFFICIENT;
  steinhart += 1.0 / (KNOWN_TEMP + 273.15);
  steinhart = 1.0 / steinhart;
  float temperature = steinhart - 273.15;
  Serial.print("    Temperature: "); 
  Serial.println(temperature); 

  delay(500);
}