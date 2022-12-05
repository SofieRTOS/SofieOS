#include "sofie.h"
#include "trap.h"
#include <stdarg.h>
#include <stdio.h>

// delegate printf
int uprintf(const char* format, ...) {
	if (current_privileges == PRIV_USER) {
		sofie_set_protected_stack();
	}

	int ret = 0;
	va_list argptr;
	va_start(argptr, format);

	if (current_privileges == PRIV_MACHINE) {
		ret = vprintf(format, argptr);
		fflush(stdout);
	} else {
		ret = sofie_printf(format, argptr);
	}

	va_end(argptr);

	if (current_privileges == PRIV_USER) {
		sofie_restore_protected_stack();
	}
	return ret;
}
