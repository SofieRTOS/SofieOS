// SPDX-License-Identifier: Apache-2.0
#define __STDC_WANT_LIB_EXT1__ 1 // C11's bounds-checking interface.
#include <string.h>

void memzero(void *pnt, size_t len) {
	sofie_set_protected_stack();
  volatile unsigned char *p = pnt;
  while (len--)
    *p++ = 0;
  sofie_restore_protected_stack();
}
