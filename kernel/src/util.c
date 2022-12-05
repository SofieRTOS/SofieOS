#include "util.h"

void panic(const char *s) {
	fprintf(stdout, s);
	fflush(stdout);
	while(1);
}
