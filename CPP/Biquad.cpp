/*

This solution provides digital biquad signal filtering
which have been implemented by "Direct form 2" circuit.

The difference equation can be written as:

y[n] = b0*w[n] + b1*w[n-1] - b2*w[n-2], where

w[n] = x[n] - a1*w[n-1] - a2*w[n-2]

The z-transform:

W(z)/X(z) = 1/(1 + a1*z^(-1) + a2*z^(-2))

Y(z)/V(z) = b0 + b1*z^(-1) + b2*z^(-2)

Trasnfer function for biquad filtering:

Y(z)	 b0 + b1*z^(-1) + b2*z^(-2)		(b0/a0) + (b1/a0)*z^(-1) + (b2/a0)*z^(-2)
H(z) = ------ = ---------------------------- = -------------------------------------------
X(z)	 a0 + a1*z^(-1) + a2*z^(-2)		   1 + (a1/a0)*z^(-1) + (a2/a0)*z^(-2)

Materials used:


1. Robert Bristow-Johnson. "Cookbook formulae for audio EQ biquad filter coefficients";

https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html

2. Wikipedia. "Digital biquad filter":

http://en.wikipedia.org/wiki/Digital_biquad_filter

*/

#include "Biquad.h"

void Biquad::initBPF(float f0, float g, float q)		//	Band-pass filter; 
{
	float omega0 = 2 * M_PI*(f0 / m_sr);			//		g - is gain at peak;
	float alpha = sin(omega0) / (2 * q);			//	For unity gain in skirt, make g = q;
	float a0;

	m_b0 = g*alpha;
	m_b1 = 0;
	m_b2 = -g*alpha;
	a0 = 1 + alpha;
	m_a1 = -2 * cos(omega0);
	m_a2 = 1 - alpha;

	normalize(a0);
}

void Biquad::initNotch(float f0, float q)			//	Notch filter;
{
	float omega0 = 2 * M_PI*(f0 / m_sr);
	float alpha = sin(omega0) / (2 * q);
	float a0;

	m_b0 = 1;
	m_b1 = -2 * cos(omega0);
	m_b2 = 1;
	a0 = 1 + alpha;
	m_a1 = -2 * cos(omega0);
	m_a2 = 1 - alpha;

	normalize(a0);
}
