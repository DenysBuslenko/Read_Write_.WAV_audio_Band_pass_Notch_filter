#ifndef _Biquad_
#define _Biquad_

#include <cmath>

#define M_SQRT1_2 	0.707106781186547524401
#define M_PI 		3.14159265358979323846

class Biquad
{
	float m_sr;			//	Sampling rate;

	float m_b0;			//	Coefficients of Transfer function's H(z) numerator:
	float m_b1;			//	these determine the zero positions;
	float m_b2;

	float m_a1;			//	Coefficients of of Transfer function's H(z) denominator:
	float m_a2;			//	these determine the pole positions;

	float m_w1;			//	w delayed by 1 sample;
	float m_w2;			//	w delayed by 2 samples;

	void normalize(float a0)	//	Transfer function's H(z) coefficients normalization;
	{
		m_b0 /= a0;
		m_b1 /= a0;
		m_b2 /= a0;
		m_a1 /= a0;
		m_a2 /= a0;
	}

public:

	Biquad(float sr)
	{
		m_sr = sr;

		clear();
	}

	~Biquad() {}

	void clear()
	{
		m_w1 = 0.0;
		m_w2 = 0.0;
	}

	void initBPF(float f0, float g = 1, float q = M_SQRT1_2);	//	Band-pass filter function initialization:

									//		f0 - center frequency;
									//		g - gain at peak;
									//		q - -3dB bandwidth at a gain of 0,707;				

	void initNotch(float f0, float q = M_SQRT1_2);			//	Notch filter function initialization:

									//		f0 - center frequency;
									//		q - -3dB bandwidth at a gain of 0,707;

	void process(const float* x, float* y, int n)			//	Signal processing function;			
	{
		for (int i = 0; i < n; i++)
		{
			*y++ = tick(*x++);
		}
	}

	inline float tick(float x)
	{
	/*	float y = b0*x + b1*xd1 + b2*xd2 - a0*yd1 - a2*yd2;	//	Diffrence equations for Direct form 1
		yd2 = yd1;	yd1 = y;				//	circuit implementation;
		xd2 = xd1;	xd1 = x;	*/

		float w = x - m_a1*m_w1 - m_a2*m_w2;			//	Diffrence equations for Direct form 2
		float y = m_b0*w + m_b1*m_w1 + m_b2*m_w2;		//	circuit implementation;

		m_w2 = m_w1;
		m_w1 = w;

		return y;
	}
};

#endif
