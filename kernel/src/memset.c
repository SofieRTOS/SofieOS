#include "sofie.h"
#include <trap.h>
#include <stdint.h>
#include <stdio.h>

void* umemset(void *dst, int val, size_t size) {
	if (current_privileges == PRIV_USER) {
		sofie_set_protected_stack();
	}

	int i;
	uint8_t *buf = dst;
	for (i = 0; i < size; i++) {
		buf[i] = val;
	}

	if (current_privileges == PRIV_USER) {
		sofie_restore_protected_stack();
	}

	return dst;
}
