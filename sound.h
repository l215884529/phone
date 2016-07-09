#include <complex.h>

#ifndef SOUND_H
#define SOUND_H

typedef short sample_t;

void cut_off(complex short *data, complex double *Y, int low, int high, long n);
void re_cut_off(complex short *data, complex double *Y, int low, int high, long n);
int is_silence(complex short *data_rec, int n);
void smoothing(sample_t *data, int n);
void hamming_window(sample_t *data, int n);
void re_hamming_window(sample_t *data, int n);

#endif
