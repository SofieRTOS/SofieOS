#ifndef CPU_H__
#define CPU_H__

#include <stdint.h>

#define CLINT 0x2000000L
#define CLINT_MTIMECMP (CLINT + 0x4000)
#define CLINT_MTIME (CLINT + 0xBFF8)

uint32_t cpu_freq(void);
void delay(uint32_t counter);
void cpu_clock_init(void);
uint64_t get_mtime();
void set_mtimecmp(uint64_t value);
void init_timer();
void update_timercmp();

#endif
