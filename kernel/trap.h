#ifndef __TRAP_H_kernel_stack
#define __TRAP_H_kernel_stack

#include <stdint.h>
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
#define IS_INTERRUPT(x) ((x >> 31) & 1)

#define TRAP_LOAD_ACCESS_FAULT  5
#define TRAP_STORE_ACCESS_FAULT 7
#define TRAP_SYSCALL 8
#define PRIV_MACHINE 0
#define PRIV_USER    1

extern int current_privileges;

struct TrapFrame {
	uint32_t epc;        /*  0  */
	uint32_t kernel_sp;  /*  4  */
	uint32_t ra;         /*  8  */
	uint32_t sp;         /* 12  */
	uint32_t gp;         /* 16  */
	uint32_t tp;         /* 20  */
	uint32_t t0;         /* 24  */
	uint32_t t1;         /* 28  */
	uint32_t t2;         /* 32  */
	uint32_t fp;         /* 36  */
	uint32_t s1;         /* 40  */
	uint32_t a0;         /* 44  */
	uint32_t a1;         /* 48  */
	uint32_t a2;         /* 52  */
	uint32_t a3;         /* 56  */
	uint32_t a4;         /* 60  */
	uint32_t a5;         /* 64  */
	uint32_t a6;         /* 68  */
	uint32_t a7;         /* 72  */
	uint32_t s2;         /* 76  */
	uint32_t s3;         /* 80  */
	uint32_t s4;         /* 84  */
	uint32_t s5;         /* 88  */
	uint32_t s6;         /* 92  */
	uint32_t s7;         /* 96  */
	uint32_t s8;         /* 100 */
	uint32_t s9;         /* 104 */
	uint32_t s10;        /* 108 */
	uint32_t s11;        /* 112 */
	uint32_t t3;         /* 116 */
	uint32_t t4;         /* 120 */
	uint32_t t5;         /* 124 */
	uint32_t t6;         /* 128 */
};

void trap_initialization();
void usertrap();
void usertrapret();
void syscall();
void store_access_fault_handler();
void timer_intrupt_handler();

#endif
