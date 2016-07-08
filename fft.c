#include <assert.h>
#include <complex.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fft.h"

/* 標本(整数)を複素数へ変換 */
void sample_to_complex(sample_t * s, 
					   complex double * X, 
					   long n) {
	long i;
	for (i = 0; i < n; i++) X[i] = s[i];
}

/* 複素数を標本(整数)へ変換. 虚数部分は無視 */
void complex_to_sample(complex double * X, 
					   sample_t * s, 
					   long n) {
	long i;
	for (i = 0; i < n; i++) {
		s[i] = creal(X[i]);
	}
}

/* 高速(逆)フーリエ変換;
   w は1のn乗根.
   フーリエ変換の場合   偏角 -2 pi / n
   逆フーリエ変換の場合 偏角  2 pi / n
   xが入力でyが出力.
   xも破壊される
*/
void fft_r(complex double * x, 
		   complex double * y, 
		   long n, 
		   complex double w) {
	if (n == 1) { y[0] = x[0]; }
	else {
		complex double W = 1.0; 
		long i;
		for (i = 0; i < n/2; i++) {
			y[i]     =     (x[i] + x[i+n/2]); /* 偶数行 */
			y[i+n/2] = W * (x[i] - x[i+n/2]); /* 奇数行 */
			W *= w;
		}
		fft_r(y,     x,     n/2, w * w);
		fft_r(y+n/2, x+n/2, n/2, w * w);
		for (i = 0; i < n/2; i++) {
			y[2*i]   = x[i];
			y[2*i+1] = x[i+n/2];
		}
	}
}

void fft(complex double * x, 
		 complex double * y, 
		 long n) {
	long i;
	double arg = 2.0 * M_PI / n;
	complex double w = cos(arg) - 1.0j * sin(arg);
	fft_r(x, y, n, w);
	for (i = 0; i < n; i++) y[i] /= n;
}

void ifft(complex double * y, 
		  complex double * x, 
		  long n) {
	double arg = 2.0 * M_PI / n;
	complex double w = cos(arg) + 1.0j * sin(arg);
	fft_r(y, x, n, w);
}

int pow2check(long N) {
	long n = N;
	while (n > 1) {
		if (n % 2) return 0;
		n = n / 2;
	}
	return 1;
}
