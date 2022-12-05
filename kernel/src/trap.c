#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "cpu.h"
#include "callimpl.h"
#include "macros.h"
#include "riscv.h"
#include "trap.h"
#include "syscall.h"
#include "pmp.h"
#include "task.h"
#include "util.h"

struct TrapFrame trapframe __attribute__ ((section(".kernel"), used));
extern void trampoline();
extern void trapret();
int current_privileges;
int tick = 0;
int call_cnt = 0;

void trap_initialization() {
	current_privileges = PRIV_MACHINE;
	trapframe.kernel_sp = r_sp();
	w_mscratch((uint32_t)&trapframe);
	w_mtvec((uint32_t)(trampoline));
}

void usertrap() {
	current_privileges = PRIV_MACHINE;
	struct Task* task = get_current_task();
	struct TrapFrame* tf = NULL;
	if (task == NULL) {
		tf = &trapframe;
	} else {
		tf = &task->trapframe;
	}

	tf->epc = r_mepc();
	uint32_t mcause = r_mcause();
	if (!IS_INTERRUPT(mcause) && mcause == TRAP_SYSCALL) {
		tf->epc += 4;
		syscall();
	} else if (!IS_INTERRUPT(mcause)
			&& (mcause == TRAP_STORE_ACCESS_FAULT || mcause == TRAP_LOAD_ACCESS_FAULT)) {
		store_access_fault_handler();
	} else if (IS_INTERRUPT(mcause)) {
		uint32_t mpp = (r_mstatus() >> 10) & 3;
		if (mpp == 0) { // intrupt from user space.
			timer_intrupt_handler();
		}
		++tick;
		update_timercmp();
	} else {
		printf("mcause = %u -->", mcause);
		for (int i = 0; i < 8; i++)	dump_pmp(i);
		panic("Not allowed trap.\r\n");
	}

ret:
	usertrapret();
}

void usertrapret() {
	current_privileges = PRIV_USER;
	trapret();
}

static uint32_t (*syscalls[])(void) = {
[SYS_dump_pmp]		                sys_dump_pmp,
[SYS_set_protected_stack_top]		sys_set_protected_stack_top,
[SYS_restore_protected_stack_top]	sys_restore_protected_stack_top,
[SYS_malloc]                        sys_malloc,
[SYS_free]                          sys_free,
[SYS_change_owner]                  sys_change_owner,
[SYS_new_task]                      sys_new_task,
[SYS_post_message]                  sys_post_message,
[SYS_get_message]                   sys_get_message,
[SYS_acquire_sleep_lock]            sys_acquire_sleep_lock,
[SYS_release_sleep_lock]            sys_release_sleep_lock,
[SYS_wait_semaphore]                sys_wait_semaphore,
[SYS_post_semaphore]                sys_post_semaphore,
[SYS_printf]                        sys_printf,
[SYS_memcpy]                        sys_memcpy,
[SYS_rand]                          sys_rand,
};

void syscall() {
	uint32_t num;
	struct Task* task = get_current_task();
	num = task->trapframe.a7;
	if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
		task->trapframe.a0 = syscalls[num]();
	} else {
		printf("unknown sys call %d\r\n", num);
		task->trapframe.a0 = -1;
	}
}

void store_access_fault_handler() {
	++call_cnt;
	uint32_t access_addr = r_mtval();
	struct Task* task = get_current_task();

	if (access_addr >= task->usp_bottom && access_addr < task->usp_top) {
		enable_buffer_access(task->usp_bottom, task->usp_top, 2);
		return;
	}

	// Case 1: fp is in the current stack frame but sp is lower than current stack bottom.
	// Should expand stack.
	if (task->trapframe.sp < (uint32_t) task->stacks->data &&
		task->trapframe.fp <= (uint32_t) task->stacks->data + task->stacks->length) {
//		printf("CASE 1: STACK EXPAND FOR TASK %d\r\n", task->tid);
		task_expand_stack();
		return;
	}

	// Case 2: fp is in the previous stack frame and sp is at the bottom of the current stack.
	// Should shrink stack.
	if (task->stacks->fp && task->trapframe.fp >= task->stacks->next->data && task->trapframe.fp <= task->stacks->next->data + task->stacks->next->length
			&& task->trapframe.sp >= (uint32_t)task->stacks->data + task->stacks->length) {
//		printf("CASE 2: SHRINK STACK FOR TASK %d\r\n", task->tid);
		task_shrink_stack();
		return;
	}

	// Corner case: If access is in the stack and between current frame pointer and previous frame pointer,
	// Then allow access.
	if (access_addr >= task->trapframe.fp && access_addr < *((uint32_t*)(task->trapframe.fp - 8)) - 8) {
//		 printf("CORNER CASE: ALLOW PREVIOUS FRAME TO ACCESS!\r\n");
		enable_buffer_access(task->trapframe.fp, *((uint32_t*)(task->trapframe.fp - 8)) - 8, 0);
		return;
	}

	struct BufferEntry *p = buffer_entries_head;
	while (p && access_addr >= p->address) {
		if (access_addr < p->address + p->length) {
			if ((p->owner == 0 && p->address >= (uint32_t)&__user_section_begin
					&& p->address + p->length < (uint32_t)&__user_section_end
					&& !has_ownership(p->address)) || (p->owner != 0 && task->tid != p->owner)
				) {
				printf("MEMORY DOESN'T HAS OWNERSHIP\r\n");
				goto deny;
			}

			enable_buffer_access(p->address, p->address + p->length, 0);
			return;
		}

		p = p->next;
	}

	// Case 3: fp is at the bottom of the current stack, but access address is greater than fp.
	// Should move current fp to the previous fp.
	if (task->stacks->fp && task->trapframe.fp == task->stacks->data + task->stacks->length
			&& access_addr >= task->trapframe.fp
			&& access_addr >= task->stacks->data) {
//		printf("CASE 3: MOVE FP TO THE PREVIOUS FP.\r\n");
		task->trapframe.fp = task->stacks->fp;
		return;
	}

	// Case 4: When fp is equal to the previous fp, and sp is in the current stack,
	// and access address is lower than fp.
	// Should move current fp to the bottom of the current stack.
	if (task->stacks->fp && task->trapframe.fp == task->stacks->fp
			&& access_addr < task->trapframe.fp) {
//		printf("CASE 4: MOVE FP TO THE CURRENT STACK BOTTOM.\r\n");
		task->trapframe.fp = task->stacks->data + task->stacks->length;
		return;
	}

deny:
	printf("TASK ID: %d\r\n", get_current_task()->tid);
	printf("STACK: BEGIN AT %x END AT %x\r\n", get_current_task()->stacks->data, get_current_task()->stacks->data + get_current_task()->stacks->length);
	printf("FP: %x, SP: %x\r\n", get_current_task()->trapframe.fp, get_current_task()->trapframe.sp);
	printf("WANTS TO ACCESS: %x\r\n", access_addr);
	for (int i = 0; i < 8; i++)	dump_pmp(i);
	panic("Store Access Fault!\r\n");
}

void timer_intrupt_handler() {
	task_yield();
}
