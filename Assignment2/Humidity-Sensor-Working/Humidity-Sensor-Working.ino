#define PIN 26 // pin that the data wire is connected to

#define CIRCUIT_RESISTOR_RESISTANCE 1000 // reistance of the resistor use when building the circuit

void setup() {
Serial.begin(9600);
}

void loop() {
  float reading;
  reading = analogRead(PIN);
  Serial.print("Reading: "); 
  Serial.print(reading);   
  // convert reading to resistance
  reading = (4095 / reading) - 1;
  reading = CIRCUIT_RESISTOR_RESISTANCE / reading;
  Serial.print("    Resistance: "); 
  Serial.println(reading); 
  delay(500);
}