#define PIN 26 // pin that the data wire is connected to

#define CIRCUIT_RESISTOR_RESISTANCE 1000 // reistance of the resistor use when building the circuit

void setup() {
Serial.begin(9600);
}

float average;
int count = 0;
int initial = 10;
bool spike;

void loop() {
  spike = false;
  float reading;
  reading = analogRead(PIN);
  Serial.print("Reading: "); 
  Serial.print(reading);   
  // convert reading to resistance
  reading = (4095 / reading) - 1;
  reading = CIRCUIT_RESISTOR_RESISTANCE / reading;
  Serial.print("    Resistance: "); 
  Serial.println(reading); 
  if(initial > 0)
  {
    average = ((average*count) + reading)/(count+1);
    count += 1;
    initial -= 1;
  }
  else if((reading < (average * 1.1)) && (reading > (average * 0.9)))
  {
    average = ((average*count) + reading)/(count+1);
    count += 1;
  } else
  {
    spike = true;
  }
  delay(500);
}
