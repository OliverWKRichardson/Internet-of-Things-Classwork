/*
  SSens.cpp - Library simulating a sensor signal.
  Created by Christos Efstratiou, October 6, 2021.
  Released into the public domain.
*/

#ifndef SSENSE_H
#define SSENSE_H

#include "Arduino.h"



class SSens {
  private:
    static const int max_signal_set = 4;
    const float pi = 3.14159265359;
    bool running = false;
    float signal_set[max_signal_set];
    float signal_step[max_signal_set];  // degrees
    float x_deg[max_signal_set];
    float weights[max_signal_set];
    float baseline=0;
    float activeBaseline = 0;


    float y_signal = 0;

    float getSubSignal(int i);
    float getNoise();
    void createEvent();

    int eventcounter=0;
    int noeventcounter=0;
    int nextEventTime = 50;

    int signal_mix = 4;
    bool add_noise = true;
    float noise_range = 0.15;    // 15%

  public:
    SSens(int pin);
    void setSignal(int i, float step, float weight);
    void setSignalSetSize(int signal_mix);
    void setNoise(int noise_range);   
    void setBaseline(float baseline);
    void begin();


    float getSignal();
  
};
#endif