#ifndef BIQUADLPF_H
#define BIQUADLPF_H

// initialize the struct and run biquadLPFInit on the object to initialize the filter
struct BiquadLPF{

	/* Controller gains */
	float x_n1;
	float x_n2;
	float y_n1;
	float y_n2;

	/* Derivative low-pass filter time constant */
	float cutoff_freq;
	float sample_freq;

	float b0;
	float b1;
	float b2;
	float a1;
	float a2;
};

void biquadLPFInit(BiquadLPF* filter, float cutoffFreq, float sampleFreq);
float biquadLPFApply(BiquadLPF* filter, float x_n);

#endif