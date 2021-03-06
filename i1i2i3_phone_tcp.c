#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <math.h>
#include <complex.h>
#include <pthread.h>
#include "die.h"
#include "fft.h"
#include "sound.h"

#define SAMPLE 4096
#define FREQ_MIN 50
#define FREQ_MAX 3000
#define RES_OK 0
#define RES_SI 1
#define RES_OK_TO_SI 2

int N = 0;

typedef enum node_type {
	SERVER,
	CLIENT
} node_type_t;

typedef struct data_t {
	int *status;
	int *socket_number;
	int *s;
} data_t;

ssize_t read_n(int fd, ssize_t n, void * buf) {
	ssize_t re = 0;
	while (re < n) {
		ssize_t r = read(fd, buf + re, n - re);
		if (r == -1) die("read");
		if (r == 0) break;
		re += r;
	}
	memset(buf + re, 0, n - re);
	return re;
}

ssize_t write_n(int fd, ssize_t n, void * buf) {
	ssize_t wr = 0;
	while (wr < n) {
		ssize_t w = write(fd, buf + wr, n - wr);
		if (w == -1) die("write");
		wr += w;
	}
	return wr;
}

void *accept_connection(void *arg) {
	data_t d = *(data_t *)arg;
	int ss = *(d.socket_number);
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(struct sockaddr_in);
	
	*(d.s) = accept(ss, (struct sockaddr *)&client_addr, &len);
	if (*(d.s) == -1) die("accept");
	*(d.status) = 1;

	fprintf(stderr, "Connected to %s:%d.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	
	return (void *)NULL;
}

void calculate_sample_count() {
	int i;
	for (i = 1; i < SAMPLE / 2; i++) {
		const int freq = (int)(i * 44100.0 / SAMPLE);
		if (freq >= FREQ_MIN && freq <= FREQ_MAX) N++;
	}
}

void set_response_with_data(char *response, complex short *data, int n) {
	static int count = 0;
	if (is_silence(data, n)) {
		switch (*response) {
		case RES_OK:
			*response = RES_OK_TO_SI;
			break;
		case RES_SI:
			*response = RES_SI;
			break;
		case RES_OK_TO_SI:
			count++;
			if (count == 3) {
				count = 0;
				*response = RES_SI;
			}
			break;
		default:
			break;
		}
	} else {
		*response = RES_OK;
	}
}

int main(int argc, char *argv[]) {
	calculate_sample_count();
	node_type_t type = 0;
	int port;
	int ss;
	struct sockaddr_in addr;
	data_t d;
	int s;
	int status = 0;
	pthread_t th;
	complex double *X = (complex double *)malloc(sizeof(complex double) * SAMPLE);
	complex double *Y = (complex double *)malloc(sizeof(complex double) * SAMPLE);
	complex short *data = (complex short *)malloc(sizeof(complex short) * N);
	sample_t *buf = (sample_t *)malloc(sizeof(sample_t) * SAMPLE);
	sample_t *prev_data = (sample_t *)malloc(sizeof(sample_t) * SAMPLE / 2);
	char response_send, response_recv;
	int m, n;

	if (argc == 2) {
		type = SERVER;
	} else if (argc == 3) {
		type = CLIENT;	
	} else {
		fprintf(stderr, "Usage: %s [ip address] <port>", argv[1]);
		exit(1);
	}
	port = atoi(argv[argc - 1]);
	ss = socket(PF_INET, SOCK_STREAM, 0);
	if (ss == -1) die("socket");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	d.socket_number = &ss;
	d.s = &s;
	d.status = &status;

	switch (type) {
	case SERVER:
		addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(ss, (struct sockaddr *)&addr, sizeof(addr)) == -1) die("bind");
		if (listen(ss, 10) == -1) die("listen");
		if (pthread_create(&th, NULL, accept_connection, (void *)&d) != 0) die("pthread_create");

		fprintf(stderr, "Waiting...\n");

		break;
	case CLIENT:
		if (inet_aton(argv[1], &addr.sin_addr) == 0) die("inet_aton");
		if (connect(ss, (struct sockaddr *)&addr, sizeof(addr)) == -1) die("connect");
		s = ss;
		status = 1;
		
		fprintf(stderr, "Connected to %s:%d.\n", argv[1], port);
		
		break;
	default:
		break;
	}

	memset(prev_data, 0, sizeof(sample_t) * SAMPLE / 2);
	while (1) {
		m = read_n(0, sizeof(sample_t) * SAMPLE / 2, buf + SAMPLE / 2);
		if (m == 0) break;
		if (status == 1) {
			memcpy(buf, prev_data, sizeof(sample_t) * SAMPLE / 2);
			memcpy(prev_data, buf + SAMPLE / 2, sizeof(sample_t) * SAMPLE / 2);
			hamming_window(buf, SAMPLE);
			sample_to_complex(buf, X, SAMPLE);
			fft(X, Y, SAMPLE);
			cut_off(data, Y, FREQ_MIN, FREQ_MAX, SAMPLE);
			set_response_with_data(&response_send, data, N);
			send(s, &response_send, sizeof(char), 0);
			switch (response_send) {
			case RES_OK:
			case RES_OK_TO_SI:
				write_n(s, sizeof(complex short) * N, data);
//				n = send(s, data, sizeof(complex short) * N, 0);
				break;
			case RES_SI:
				memset(prev_data, 0, sizeof(sample_t) * SAMPLE / 2);
				break;
			default:
				break;
			}

//			n = recv(s, &response_recv, sizeof(char), 0);
			n = read_n(s, sizeof(char), &response_recv);
			if (n == -1) die("recv");
			if (n == 0) break;
			switch (response_recv) {
			case RES_OK:
			case RES_OK_TO_SI:
				n = read_n(s, sizeof(complex short) * N, data);
//				n = recv(s, data, sizeof(complex short) * N, 0);
				if (n == -1) die("recv");
				if (n == 0) break;
				re_cut_off(data, Y, FREQ_MIN, FREQ_MAX, SAMPLE);
				ifft(Y, X, SAMPLE);
				complex_to_sample(X, buf, SAMPLE);
				re_hamming_window(buf, SAMPLE);
				write_n(1, sizeof(sample_t) * SAMPLE / 2, buf + SAMPLE / 4);
				break;
			case RES_SI:
				break;
			default:
				break;
			}
		}
	}

	pthread_join(th, NULL);
	close(ss);
		
	return 0;
}
