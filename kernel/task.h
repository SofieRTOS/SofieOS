#ifndef __H_TASK_H_
#define __H_TASK_H_
#include <stdint.h>
#include <string.h>
#include "linkedlist.h"
#include "lock.h"
#include "macros.h"
#include "sofie.h"
#include "trap.h"
#include "umalloc.h"
#include "pmp.h"

#define IS_RUNNABLE(x)                       (x & 1)
#define IS_RUNNING(x)                        ((x >> 1) & 1)
#define IS_SLEEPING(x)                       ((x >> 2) & 1)
#define IS_ZOMBIE(x)                         ((x >> 3) & 1)
#define IS_USER_STACK_JUST_ALLOCATED(x)      ((x >> 4) & 1)
#define SET_STATE(x, y)                      do { \
                                 	            x &= ~((1 << 4) - 1);  \
                                 	            x |= (1 << y); \
                                             } while (0)
#define SET_USER_STACK_JUST_ALLOCATED(x, y)  do { \
	                                            x &= ~(1 << 4); \
										        x |= (y << 4); \
                                             } while (0)

struct Context {
	uint32_t ra;    // 0
	uint32_t sp;    // 4
	uint32_t s0;    // 8
	uint32_t s1;    // 12
	uint32_t s2;    // 16
	uint32_t s3;    // 20
	uint32_t s4;    // 24
	uint32_t s5;    // 28
	uint32_t s6;    // 32
	uint32_t s7;    // 36
	uint32_t s8;    // 40
	uint32_t s9;    // 44
	uint32_t s10;   // 48
	uint32_t s11;   // 52
};

enum TaskState {
	RUNNABLE,
	RUNNING,
	SLEEPING,
	ZOMBIE,
};

struct SpinLock {
	uint32_t locked;
};

struct SleepLock {
	uint32_t locked;
	struct SpinLock spinlock;
};

struct Semaphore {
	uint32_t value;
	struct SpinLock spinlock;
};

struct Stack {
	uint32_t length;
	uint32_t fp;
	uint32_t sp;
	uint32_t extra;
	uint32_t p_sp;
	uint8_t *data;
	struct Stack* next;
};

struct Task {
	struct Context context;
	uint8_t *kernel_stack;
	struct LinkedList message_queue;
	struct SleepLock queue_lock;
	struct Semaphore queue_sema;
	uint8_t tid;
	struct TrapFrame trapframe;
	void *sleep_channel;
	struct Stack *stacks;
	int state; // bit [0, 3] are TaskState enum values, bit 4 is the just allocated a new stack
	uint32_t usp_top;
	uint32_t usp_bottom;
	struct PmpSettings pmp;
};

int create_task(int (*task_entry)(void));
void init_tasks();
struct Task* get_current_task();
void* get_message(int tid);
void post_message(int tid, void* msg);
void sched();
void scheduler();
void sleep(void* channel, struct SpinLock* lock);
void swtch(struct Context* out, struct Context* in);
void task_expand_stack();
void task_yield();
void task_shrink_stack();
void wakeup(void* channel);

#endif /* __H_TASK_H_ */
