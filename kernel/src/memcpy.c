#include "sofie.h"
#include <trap.h>
#include <stdint.h>
#include <stdio.h>

void* memcpy(void *dst, const void *src, size_t size) {
	if (current_privileges == PRIV_USER) {
		sofie_set_protected_stack();
	}

	size_t i;
	uint8_t* dst_t = (uint8_t*) dst;
	uint8_t* src_t = (uint8_t*) src;
	for (i = 0; i < size; i++) {
		dst_t[i] = src_t[i];
	}

	if (current_privileges == PRIV_USER) {
		sofie_restore_protected_stack();
	}

	return dst;
}
