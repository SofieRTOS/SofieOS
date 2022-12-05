#include "task.h"
#include "riscv.h"
#include "trap.h"
#include "umalloc.h"
#include "util.h"
#include "pmp.h"

static struct LinkedList task_list;
static struct Task* current_task = NULL;
static struct Context kernel_context;
extern void switch_mscratch(struct TrapFrame*);
static int task_count = 0;

// Static functions.
static int get_tid() {
	int i, used = 0;
	struct LinkedListNode* p = task_list.head;
	while (p != NULL) {
		struct Task* task = p->data;
		used |= 1 << (task->tid - 1);
		p = p->next;
	}

	for (i = 0; i < MAX_TASK_COUNT; i++) {
		if (!((used >> i) & 1)) {
			return i+1;
		}
	}

	return -1;
}

static struct Task* get_task_by_id(int tid) {
	struct LinkedListNode* p = task_list.head;
	while (p != NULL) {
		struct Task *task = p->data;
		if (task->tid == tid) return task;
		p = p->next;
	}

	return NULL;
}

// Public functions.
int create_task(int (*task_entry)(void)) {
	// Get a task id, reached max count when -1 is returned.
	int tid = get_tid();
	if (tid == -1) {
		panic("MAX TASK COUNT REACHED!\r\n");
	}

	// Initialize task.
	struct Task *task = (struct Task*)malloc(sizeof(struct Task));
	if (task == NULL) {
		panic("Not enough memory to create a new task!");
	}

	memset(task, 0, sizeof(struct Task));
	task->kernel_stack = (uint8_t*)malloc(KERNEL_STACK_SIZE);
	if (task->kernel_stack == NULL) {
		panic("Not enough memory to create a new task!");
	}

	task->sleep_channel = NULL;
	SET_STATE(task->state, RUNNABLE);

	// Init stack.
	task->stacks = malloc(sizeof(struct Stack));
	task->stacks->fp = 0;
	task->stacks->data = malloc(USER_STACK_SIZE);
	task->stacks->next = NULL;
	task->stacks->length = USER_STACK_SIZE;
	task->usp_bottom = (uint32_t) task->stacks->data;
	task->usp_top = (uint32_t) task->stacks->data + USER_STACK_SIZE;
	SET_USER_STACK_JUST_ALLOCATED(task->state, 0);

	task->tid = tid;
	task->trapframe.kernel_sp = (uint32_t) task->kernel_stack + KERNEL_STACK_SIZE;
	task->trapframe.sp = (uint32_t) task->stacks->data + USER_STACK_SIZE;
	task->trapframe.gp = r_gp();
	task->trapframe.epc = (uint32_t) task_entry;
	task->context.ra = (uint32_t) usertrapret;
	task->context.sp = (uint32_t) task->kernel_stack + KERNEL_STACK_SIZE;
	sleeplock_init(&task->queue_lock);
	semaphore_init(&task->queue_sema, 0);

	// Add task to linkedlist.
	struct LinkedListNode* node = (struct LinkedListNode*)malloc(sizeof(struct LinkedListNode));
	if (node == NULL) {
		panic("Not enough memory to create a new task!");
	}

	node->data = task;
	node->next = NULL;
	add_list_node(&task_list, node);

	// Init task message queue.
	init_linked_list(&task->message_queue);

	++task_count;
	return tid;
}

void init_tasks() {
	init_linked_list(&task_list);
}

struct Task* get_current_task() {
	return current_task;
}

void* get_message(int tid) {
	struct Task* task = get_task_by_id(tid);
	void* result = NULL;
	struct LinkedListNode* node = NULL;
	semaphore_wait(&task->queue_sema);
	sleeplock_acquire(&task->queue_lock);
	result = task->message_queue.head->data;
	node = task->message_queue.head;
	task->message_queue.head = node->next;
	free(node);
	sleeplock_release(&task->queue_lock);
	return result;
}

void post_message(int tid, void* msg) {
	struct Task* task = get_task_by_id(tid);
	sleeplock_acquire(&task->queue_lock);
	struct LinkedListNode* node = (struct LinkedListNode*)malloc(sizeof(struct LinkedListNode));
	if (node == NULL) {
		panic("CANNOT POST MESSAGE!\r\n");
	}

	node->data = msg;
	node->next = NULL;

	change_owner((uint32_t) msg, tid);
	add_list_node(&task->message_queue, node);
	semaphore_post(&task->queue_sema);
	sleeplock_release(&task->queue_lock);
}

void sched() {
	swtch(&current_task->context, &kernel_context);
}

void scheduler() {
	for(;;) {
		struct LinkedListNode* p = task_list.head;
		while (p != NULL) {
			struct Task* task = p->data;
			if (IS_RUNNABLE(task->state)) {
				SET_STATE(task->state, RUNNING);
				current_task = task;
				restore_pmp_settings(&task->pmp);
				switch_mscratch(&task->trapframe);
				swtch(&kernel_context, &task->context);
				current_task = NULL;
			}

			p = p->next;
		}
	}
}

// Swtch -> see swtch.S

void sleep(void* channel, struct SpinLock* lock) {
	if (current_task == NULL) return;
	spinlock_release(lock);
	current_task->sleep_channel = channel;
	SET_STATE(current_task->state, SLEEPING);
	--task_count;
	sched();
	current_task->sleep_channel = NULL;
	spinlock_acquire(lock);
}

void task_expand_stack() {
	uint32_t stack_frame_size = current_task->trapframe.fp - current_task->trapframe.sp;
	uint32_t allocated_size = (stack_frame_size + USER_STACK_SIZE - 1) / USER_STACK_SIZE * USER_STACK_SIZE;
	if (stack_frame_size == allocated_size) {
		allocated_size += USER_STACK_SIZE;
	}

	uint8_t *new_stack = (uint8_t*)malloc(allocated_size);
	if (new_stack == NULL) {
		panic("INSUFFICIENT MEMORY TO EXPAND STACK.");
	}

	struct Stack *stack = malloc(sizeof(struct Stack));
	stack->data = new_stack;
	stack->next = current_task->stacks;
	stack->fp = current_task->trapframe.fp;
	stack->sp = current_task->trapframe.sp;
	stack->p_sp = current_task->usp_top;
	stack->length = allocated_size;
	stack->extra = 0;

	// Copy current frame from previous frame
	uint8_t *ptr = (uint8_t*) current_task->trapframe.fp - 1;
	uint8_t *ptr_new_stack = new_stack + allocated_size - 1;
	while (ptr >= current_task->stacks->data) {
		*ptr_new_stack = *ptr;
		--ptr; --ptr_new_stack;
	}

	uint32_t new_fp = (uint32_t) new_stack + allocated_size;

	current_task->trapframe.fp = new_fp;
	current_task->trapframe.sp = new_fp - stack_frame_size;
	current_task->stacks = stack;

	current_task->usp_bottom = (uint32_t) new_stack;
	current_task->usp_top = (uint32_t) new_stack + allocated_size;
	SET_USER_STACK_JUST_ALLOCATED(current_task->state, 1);
}

void task_yield() {
	if (task_count <= 1) return;
	SET_STATE(current_task->state, RUNNABLE);
	log_pmp_settings(&current_task->pmp);
	sched();
}

void task_shrink_stack() {
	current_task->trapframe.sp = current_task->stacks->fp - current_task->stacks->extra;
	struct Stack* t = current_task->stacks;

	struct BufferEntry *p = buffer_entries_head;
	struct BufferEntry *pre = NULL;
	while (p != NULL && p->address < t->data + t->length) {
		if (p->address >= t->data) {
			if (pre == NULL) {
				struct BufferEntry* t = p;
				buffer_entries_head = p->next;
				free(t);
				p = buffer_entries_head;
				continue;
			} else {
				pre->next = p->next;
				free(p);
				p = pre->next;
				continue;
			}
		}

		pre = p;
		p = p->next;
	}

	current_task->stacks = t->next;
	// printf("SHRINK STACK %x to %x\r\n", t->data, t->data+t->length);
	free(t->data);
	free(t);

	if (IS_USER_STACK_JUST_ALLOCATED(current_task->state)) {
		SET_USER_STACK_JUST_ALLOCATED(current_task->state, 0);
	}

	current_task->usp_bottom = (uint32_t) current_task->stacks->data;

	if (current_task->usp_top == 0) {
		remove_buffer_entry(current_task->trapframe.sp);
		uint32_t lastfp = 0;

	//	printf("SET USER STACK PMP: %x ~ %x\r\n", (uint32_t)task->stacks->data, lastfp);
		struct BufferEntry *p = buffer_entries_head;
		while (p != NULL) {
			if (p->address >= current_task->stacks->data && p->address < current_task->stacks->data + current_task->stacks->length) {
				lastfp = p->address;
				break;
			}

			p = p->next;
		}

		if (lastfp == 0) {
			lastfp = (uint32_t) current_task->stacks->data + current_task->stacks->length;
		}

		current_task->usp_bottom = (uint32_t)current_task->stacks->data;
		current_task->usp_top = lastfp;
	} else {
		current_task->usp_top = current_task->trapframe.sp;
	}

}

void wakeup(void* channel) {
	struct LinkedListNode* p = task_list.head;
	while (p != NULL) {
		struct Task* task = p->data;
		if (IS_SLEEPING(task->state) && task->sleep_channel == channel) {
			SET_STATE(task->state, RUNNABLE);
			++task_count;
		}

		p = p->next;
	}
}
