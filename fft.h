#include <complex.h>

#ifndef FFT_H
#define FFT_H

typedef short sample_t;

void sample_to_complex(sample_t *s, complex double *X, long n);
void complex_to_sample(complex double *X, sample_t *s, long n);
void fft(complex double *x, complex double *y, long n);
void ifft(complex double *y, complex double *x, long n);
int pow2check(long N);

#endif
