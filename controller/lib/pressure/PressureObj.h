#ifndef PRSSUREOBJ_H
#define PRSSUREOBJ_H

#include "BiquadLPF.h"
#include <TeensyThreads.h>

/*
    * Pressure.h
    *
    *  Created on: 2024-6-19 by Vincent Palmerio
    *  Description: This file contains the declaration of the PressureObj class, 
    *  in which each object is used to read from a specific pressure sensor.
*/

// Implementation of TeensyThreads based on https://github.com/ftrias/TeensyThreads/tree/master/examples/Runnable
// Runnable example

class RunnablePressure{
private:
protected:
	virtual void checkPressure(void *arg) = 0;
public:
	virtual ~RunnablePressure(){}

	static void runThread(void *arg)
	{
		RunnablePressure *_runnable = static_cast<RunnablePressure*> (arg);
		_runnable->checkPressure(arg);
	}
};

class PressureObj : public RunnablePressure {

private:
    unsigned int pin;
    float pressureMin;
    float pressureMax;
    float voltageMin;
    float voltageMax;
    float sample_time;
    BiquadLPF filter;

    std::thread* checkPressureThread;
    int threadID;
protected:
  // Runnable function that we need to implement
  void checkPressure(void *arg);

public:
    PressureObj(unsigned int pin, float pressureMin, float pressureMax, float voltageMin, float voltageMax, float filter_cutoff_freq, float filter_sample_freq);
    float getPressure() volatile;
    void PressureObj::startPressureCheckThread();
    volatile float pressure;

};


#endif // PRSSUREOBJ_H