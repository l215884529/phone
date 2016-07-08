#include <math.h>
#include <complex.h>
#include "sound.h"

#define VOLUME_THRESHOLD 700
#define SMOOTHING_NUMBER 200

typedef short sample_t;

void cut_off(complex double *data, complex double *Y, int low, int high, long n) {
	long i;
	int count = 0;
	for (i = 1; i <= n / 2; i++) {
		const int freq = (int)(i * 44100.0 / n);
		if (freq >= low && freq <= high) {
			data[count++] = Y[i];
		}
	}
}

void re_cut_off(complex double *data, complex double *Y, int low, int high, long n) {
	long i;
	int count = 0;
	for (i = 1; i < n / 2; i++) {
		const int freq = (int)(i * 44100.0 / n);
		if (freq >= low && freq <= high) {
			Y[i] = data[count++];
			Y[n - i] = conj(Y[i]);
		}
	}
}

int is_silence(complex double *data_rec, int n) {
	const int min = VOLUME_THRESHOLD;
	int i;
	double sum = 0.0;
	for (i = 0; i < n; i++) {
		sum += cabs(data_rec[i]);
	}
	
	return sum < min;
}

void smoothing(sample_t *data, int n) {
	static int start = 0;
	static sample_t prev_data;
	int i;
	sample_t work[SMOOTHING_NUMBER + 1];
	sample_t diff;
		
	if (start == 1) {
		// copy data
		work[0] = prev_data;
		for (i = 1; i < 1 + SMOOTHING_NUMBER; i++) {
			work[i] = data[i - 1];
		}
		// smoothing
		diff = work[0] - work[1];
		for (i = 1; i < 1 + SMOOTHING_NUMBER; i++) {
			work[i] += diff * (SMOOTHING_NUMBER - i + 1) / SMOOTHING_NUMBER; // not safe, may overflow
		}
		// writeback
		for (i = 1; i < 1 + SMOOTHING_NUMBER; i++) {
			data[i - 1] = work[i];
		}
	}

	prev_data = data[n - 1];
	
	if (start == 0) {
		start = 1;
	}
	
	return;
}

void hamming_window(sample_t *data, int n) {
	int i;
	for (i = 0; i < n; i++) {
		const double w = 0.54 - 0.46 * cos(2 * M_PI * i / n);
		data[i] = (sample_t)((double)data[i] * w);
	}

	return;
}

void re_hamming_window(sample_t *data, int n) {
	int i;
	for (i = 0; i < n; i++) {
		const double w = 0.54 - 0.46 * cos(2 * M_PI * i / n);
		data[i] = (sample_t)((double)data[i] / w);
	}	
}

