#include "BiquadLPF.h"
#include "math.h"

BiquadLPF::BiquadLPF(float cutoffFreq, float sampleFreq)
{
	this->cutoff_freq = cutoffFreq;
	this->sample_freq = sampleFreq;

    // calculates filter parameters based on cutoff and sample frequency
    float Q = 1.0/sqrt(2.0);
	float omega = 2 * M_PI * this->cutoff_freq / this->sample_freq;
	float alpha = sin(omega) / (2 * Q);
	float a0 = 1 + alpha;

	this->b0 = (1 - cos(omega)) / 2.0;
	this->b1 = 1 - cos(omega);
	this->b2 = this->b0;
	this->a1 = -2.0 * cos(omega);
	this->a2 = 1 - alpha;

	this->b0 /= a0;
	this->b1 /= a0;
	this->b2 /= a0;
	this->a1 /= a0;
	this->a2 /= a0;
}

// compiler was claiming about default constructor
BiquadLPF::BiquadLPF()
{
	this->cutoff_freq = 100;
	this->sample_freq = 100;

    // calculates filter parameters based on cutoff and sample frequency
    float Q = 1.0/sqrt(2.0);
	float omega = 2 * M_PI * this->cutoff_freq / this->sample_freq;
	float alpha = sin(omega) / (2 * Q);
	float a0 = 1 + alpha;

	this->b0 = (1 - cos(omega)) / 2.0;
	this->b1 = 1 - cos(omega);
	this->b2 = this->b0;
	this->a1 = -2.0 * cos(omega);
	this->a2 = 1 - alpha;

	this->b0 /= a0;
	this->b1 /= a0;
	this->b2 /= a0;
	this->a1 /= a0;
	this->a2 /= a0;
}

// returns filtered input
float BiquadLPF::biquadLPFApply(float x_n) volatile
{
    float y_n = 0;
    y_n = this->b0 * x_n + this->b1 * this->x_n1 + this->b2 * this->x_n2 - this->a1 * this->y_n1 - this->a2 * this->y_n2;
	
    this->y_n2 = this->y_n1;
    this->y_n1 = y_n;
    this->x_n2 = this->x_n2;
    this->x_n1 = x_n;
    
    return y_n;
}