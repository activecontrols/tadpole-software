#include "BiquadLPF.h"
#include "math.h"

void biquadLPFInit(BiquadLPF* filter, float cutoffFreq, float sampleFreq)
{
	filter->cutoff_freq = cutoffFreq;
	filter->sample_freq = sampleFreq;

    // calculates filter parameters based on cutoff and sample frequency
    float Q = 1.0/sqrt(2.0);
	float omega = 2 * M_PI * cutoffFreq / sampleFreq;
	float alpha = sin(omega) / (2 * Q);
	float a0 = 1 + alpha;

	filter->b0 = (1 - cos(omega)) / 2.0;
	filter->b1 = 1 - cos(omega);
	filter->b2 = filter->b0;
	filter->a1 = -2.0 * cos(omega);
	filter->a2 = 1 - alpha;

	filter->b0 /= a0;
	filter->b1 /= a0;
	filter->b2 /= a0;
	filter->a1 /= a0;
	filter->a2 /= a0;
}

// returns float of filtered input
float biquadLPFApply(BiquadLPF* filter, float x_n)
{
    float y_n = 0;
    y_n = filter->b0 * x_n + filter->b1 * filter->x_n1 + filter->b2 * filter->x_n2 - filter->a1 * filter->y_n1 - filter->a2 * filter->y_n2;
	
    filter->y_n2 = filter->y_n1;
    filter->y_n1 = y_n;
    filter->x_n2 = filter->x_n2;
    filter->x_n1 = x_n;
    
    return y_n;
}