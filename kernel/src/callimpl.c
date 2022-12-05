#include <stdint.h>
#include "callimpl.h"
#include "util.h"
#include "pmp.h"
#include "macros.h"
#include "umalloc.h"
#include "task.h"

extern int tick;

static uint32_t
argraw(int n)
{
  struct Task* task = get_current_task();
  switch (n) {
  case 0:
    return task->trapframe.a0;
  case 1:
    return task->trapframe.a1;
  case 2:
    return task->trapframe.a2;
  case 3:
    return task->trapframe.a3;
  case 4:
    return task->trapframe.a4;
  case 5:
    return task->trapframe.a5;
  }
  panic("argraw");
  return -1;
}

int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

uint32_t
arguint32(int n, uint32_t *ip)
{
	*ip = argraw(n);
	return 0;
}

int
argaddr(int n, uint32_t *ip)
{
  *ip = argraw(n);
  return 0;
}

uint32_t sys_dump_pmp() {
	int i;
	for (i = 0; i < 8; i++)
		dump_pmp(i);
	return 0;
}

uint32_t sys_set_protected_stack_top() {
	struct Task* task = get_current_task();

	if (task->trapframe.fp == task->stacks->fp && task->trapframe.sp >= (uint32_t)task->stacks->data
			&& task->trapframe.sp < (uint32_t)task->stacks->data + task->stacks->length) {
		task->trapframe.fp = (uint32_t)task->stacks->data + task->stacks->length;
	}


	if (IS_USER_STACK_JUST_ALLOCATED(task->state)) {
		uint32_t extra = 0;
		if (task->stacks->sp + (task->trapframe.fp - task->trapframe.sp) < task->stacks->p_sp) {
//			printf("EXTRA SPACE RESERVED!\r\n");
			extra = task->stacks->p_sp - task->stacks->sp - (task->trapframe.fp - task->trapframe.sp);
		}

		if (task->trapframe.fp != (uint32_t) task->stacks->data + task->stacks->length) {
//			printf("REARRANGE OVERCOPIED STACK!\r\n");
			*((uint32_t*)(task->trapframe.fp - 8)) = task->stacks->fp;
			task->stacks->fp -= (uint32_t) task->stacks->data + task->stacks->length - task->trapframe.fp - extra;
			// task->stacks->length = task->trapframe.fp - (uint32_t) task->stacks->data;
			uint8_t* ptr1 = (uint8_t*) task->trapframe.fp + extra - 1;
			uint8_t* ptr2 = (uint8_t*) task->stacks->data + task->stacks->length - 1;
			while ((uint32_t) ptr1 >= task->trapframe.sp) {
				*ptr2 = *ptr1;
				--ptr2; --ptr1;
			}

			task->trapframe.sp = ptr2 + 1;
			task->trapframe.fp = task->stacks->data + task->stacks->length - extra;

			if (extra) {
//				printf("ADD1 BUFFER ENTRY: %x to %x\r\n", task->trapframe.fp, task->trapframe.fp + extra);
				add_buffer_entry(task->trapframe.fp, extra, task->tid);
			}

//			printf("ADD3 BUFFER ENTRY: %x to %x\r\n", task->stacks->fp, task->stacks->fp + *((uint32_t*)(task->trapframe.fp - 8)) - 8 - task->stacks->fp);
			add_buffer_entry(task->stacks->fp, *((uint32_t*)(task->trapframe.fp - 8)) - 8 - task->stacks->fp, task->tid);
		} else {
			task->stacks->extra = extra;
		}

		SET_USER_STACK_JUST_ALLOCATED(task->state, 0);
	}

	if (task->trapframe.sp < (uint32_t) task->stacks->data) {
//		 printf("CASE 5: EXPAND TASK FROM SYSCALL.\r\n");
		if (task->stacks->fp == 0 && task->trapframe.fp == task->stacks->data + task->stacks->length) {
			panic("ERR2: STACK SIZE IS NOT ENOUGH, PLEASE TRY A LARGER STACK SIZE!\r\n");
		}

		task_expand_stack();
		SET_USER_STACK_JUST_ALLOCATED(task->state, 0);
	}

	uint32_t sp = task->trapframe.sp;
	uint32_t fp = task->trapframe.fp;

    // printf("SP: %x, FP: %x\r\n", sp, fp);

	task->usp_bottom = (uint32_t)task->stacks->data;
	task->usp_top = sp;
//	set_user_stack_pmp((uint32_t)task->stacks->data, sp, 1);

	uint32_t len = fp - sp - 8;
//	printf("ADD2 BUFFER ENTRY: %x to %x\r\n", sp, sp + len);
	add_buffer_entry(sp, len, task->tid);
	return 0;
}

uint32_t sys_restore_protected_stack_top() {
	struct Task* task = get_current_task();
	uint32_t sp = task->trapframe.sp;
	remove_buffer_entry(sp);
	uint32_t lastfp = 0;

//	printf("SET USER STACK PMP: %x ~ %x\r\n", (uint32_t)task->stacks->data, lastfp);
	if ((uint32_t)task->stacks->data < task->trapframe.fp) {
		struct BufferEntry *p = buffer_entries_head;
		while (p != NULL) {
			if (p->address >= task->stacks->data && p->address < task->stacks->data + task->stacks->length) {
				lastfp = p->address;
				break;
			}

			p = p->next;
		}

		if (lastfp == 0) {
			lastfp = task->trapframe.fp;
			if (lastfp >= (uint32_t) task->stacks->data
					&& lastfp <= (uint32_t) task->stacks->data + task->stacks->length) {
				lastfp = (uint32_t) task->stacks->data + task->stacks->length;
			}
		}

		if (lastfp >= (uint32_t) task->stacks->data
				&& lastfp <= (uint32_t) task->stacks->data + task->stacks->length) {
			task->usp_bottom = (uint32_t)task->stacks->data;
			task->usp_top = lastfp;
		} else {
			task->usp_bottom = 0;
			task->usp_top = 0;
		}
	} else {
		task->usp_bottom = 0;
		task->usp_top = 0;
	}

	return 0;
}

uint32_t sys_malloc() {
	uint32_t size_expected, result;
	arguint32(0, &size_expected);
	result = allocate_memory(size_expected + sizeof(struct MemoryBlock));
	return result;
}

uint32_t sys_free() {
	uint32_t address;
	arguint32(0, &address);
	free_memory(address);
	return 0;
}

uint32_t sys_change_owner() {
	uint32_t address;
	int tid;
	arguint32(0, &address);
	argint(1, &tid);
	int owner = get_owner(address);
	struct Task* task = get_current_task();
	if (owner != task->tid) {
		return -1;
	}

	change_owner(address, tid);
	return 0;
}

uint32_t sys_new_task() {
	uint32_t entry;
	arguint32(0, &entry);
	return create_task((int (*)(void))entry);
}

uint32_t sys_post_message() {
	int tid;
	uint32_t message;
	argint(0, &tid);
	arguint32(1, &message);
	post_message(tid, (void*)message);
	return 0;
}

uint32_t sys_get_message() {
	struct Task* task = get_current_task();
	return (uint32_t) get_message(task->tid);
}

uint32_t sys_acquire_sleep_lock() {
	uint32_t address;
	arguint32(0, &address);
	sleeplock_acquire((struct SleepLock*)address);
	return 0;
}

uint32_t sys_release_sleep_lock() {
	uint32_t address;
	arguint32(0, &address);
	sleeplock_release((struct SleepLock*)address);
	return 0;
}

uint32_t sys_wait_semaphore() {
	uint32_t address;
	arguint32(0, &address);
	semaphore_wait((struct Semaphore*)address);
	return 0;
}

uint32_t sys_post_semaphore() {
	uint32_t address;
	arguint32(0, &address);
	semaphore_post((struct Semaphore*)address);
	return 0;
}

uint32_t sys_printf() {
	uint32_t format, arglist, ret;
	arguint32(0, &format);
	arguint32(1, &arglist);
	struct Task* task = get_current_task();
	if (arglist > task->stacks->data + task->stacks->length) {
		arglist = task->stacks->fp + (arglist - ((uint32_t) task->stacks->data + task->stacks->length));
	}
	ret = vprintf((const char*)format, (va_list)arglist);
	fflush(stdout);
	return ret;
}

uint32_t sys_memcpy() {
	uint32_t dst, src, size;
	arguint32(0, &dst);
	arguint32(1, &src);
	arguint32(2, &size);
	return (uint32_t) memcpy(dst, src, size);
}

uint32_t sys_rand() {
	return (uint32_t) rand();
}
