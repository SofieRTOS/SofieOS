#include <buffer.h>
#include <cpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <riscv.h>
#include <trap.h>
#include <metal/machine.h>
#include <metal/pmp.h>
#include <metal/privilege.h>
#include <macros.h>
#include <pmp.h>
#include <task.h>
#include <uart.h>
#include <umalloc.h>

#define METAL_MSTATUS_MIE_OFFSET 3
#define METAL_MSTATUS_MPIE_OFFSET 7
#define METAL_MSTATUS_SIE_OFFSET 1
#define METAL_MSTATUS_SPIE_OFFSET 5
#define METAL_MSTATUS_UIE_OFFSET 0
#define METAL_MSTATUS_UPIE_OFFSET 4

#define METAL_MSTATUS_MPP_OFFSET 11
#define METAL_MSTATUS_MPP_MASK 3

extern int real_main();

/* User heap use for umalloc */
unsigned char __user_heap[USER_HEAP_SIZE] __attribute__ ((section(".user"), used));

void init_privilege() {
	uintptr_t mstatus = r_mstatus();

	/* Set xPIE bits based on current xIE bits */
	if (mstatus & (1 << METAL_MSTATUS_MIE_OFFSET)) {
	    mstatus |= (1 << METAL_MSTATUS_MPIE_OFFSET);
	} else {
	    mstatus &= ~(1 << METAL_MSTATUS_MPIE_OFFSET);
	}
	if (mstatus & (1 << METAL_MSTATUS_SIE_OFFSET)) {
	    mstatus |= (1 << METAL_MSTATUS_SPIE_OFFSET);
	} else {
	    mstatus &= ~(1 << METAL_MSTATUS_SPIE_OFFSET);
	}
	if (mstatus & (1 << METAL_MSTATUS_UIE_OFFSET)) {
	    mstatus |= (1 << METAL_MSTATUS_UPIE_OFFSET);
	} else {
	    mstatus &= ~(1 << METAL_MSTATUS_UPIE_OFFSET);
	}

	/* Set MPP to the requested privilege mode */
	mstatus &= ~(METAL_MSTATUS_MPP_MASK << METAL_MSTATUS_MPP_OFFSET);
	w_mstatus(mstatus);
}

int main()
{
	printf("User section begin: %x\r\n", &__user_section_begin);
	printf("User section end  : %x\r\n", &__user_section_end);
	printf("heap   section begin: %x\r\n", &metal_segment_heap_target_start);
	printf("heap   section end  : %x\r\n", &metal_segment_heap_target_end);
	fflush(stdout);
	trap_initialization();
	init_tasks();
	cpu_clock_init();
	uart_init(BAUDRATE_115200);
	init_free_list((uint32_t) __user_heap, (uint32_t) USER_HEAP_SIZE);
	pmp_initialization();
	init_privilege();
	create_task(real_main);
	init_timer();
	scheduler();
	/* Execution should never return here */
	return 6;
}
