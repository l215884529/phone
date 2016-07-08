#include <stdio.h>
#include <stdlib.h>

void die(char *s) {
	perror(s);
	exit(1);
}
