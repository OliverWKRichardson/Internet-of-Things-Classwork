#include "Arduino.h"
#include "SSens.h"

SSens::SSens(int pin)  {
  running = false;

  for (int i=0; i < max_signal_set; i++) {
    signal_set[i] = 0;
    x_deg[i] = 0;
    weights[i] = 1;
  }
  signal_step[0] = 7.0;
  signal_step[1] = 26.0;
  signal_step[2] = 13.0;
  signal_step[2] = 22.0;

  // Configure
  setSignalSetSize(4);
  setNoise(1.0);
  setSignal(0, 3.5, 2.5);
  setSignal(1, 32, 0.8);
  setSignal(2, 12, 0.6);
  setSignal(3, 21, 1.0);
  setBaseline(3);
}
    
void SSens::begin() {
  // Start sensing
  running = true;

}

void SSens::setBaseline(float baseline) {
  this->baseline = baseline;
}
void SSens::setSignalSetSize(int signal_mix) {
  if (signal_mix > max_signal_set)
    signal_mix = max_signal_set;

  this->signal_mix = signal_mix;
}

void SSens::setNoise(int noise_range) {
  this->noise_range = noise_range;
  this->add_noise = true;
  if (noise_range == 0)
    add_noise = false;
}

void SSens::setSignal(int i, float step, float weight) {
  if (i >= max_signal_set)
    return;
  signal_step[i] = step;
  weights[i] = weight;
}

void SSens::createEvent() {
  if (eventcounter == 0) {
    activeBaseline = baseline;
    noeventcounter++;
    nextEventTime = 60 + random(30);
  }

  if (noeventcounter > nextEventTime) {
    // start event
    noeventcounter = 0;
    eventcounter = 10;
  }
  if (eventcounter > 0) {
    eventcounter--;
    activeBaseline = baseline * 2;
  }
}


float SSens::getSubSignal(int i) {
  float rads = x_deg[i] * pi / 180;
  float sig = sin(rads);
  x_deg[i] = x_deg[i] + signal_step[i];
  if (x_deg[i] > 360) {
    x_deg[i] = 0;
  }
  return sig*weights[i];
}

float SSens::getNoise() {
  float rand01 = (float)random(10000)/10000;
  rand01 = rand01 - 0.5;
  rand01 = rand01 * noise_range;
  return rand01;
}


float SSens::getSignal() {

  float sig = 0;
  for (int i=0; i < signal_mix; i++) {
    sig += getSubSignal(i);
  }
  sig = sig / signal_mix;

  if (add_noise)
    sig += getNoise();
  createEvent();
  return sig + activeBaseline;
}



  
