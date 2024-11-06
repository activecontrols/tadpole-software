#ifndef BIQUADLPF_H
#define BIQUADLPF_H

class BiquadLPF {

private:
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

public:
    BiquadLPF(float cutoffFreq, float sampleFreq);
	BiquadLPF();
    float biquadLPFApply(float x_n) volatile;

};

#endif