#ifndef __H_SOFIE_H_
#define __H_SOFIE_H_
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

extern struct SleepLock;
extern struct Semaphore;

#define dispose_message(x) ufree(x)
#define sofie_init_semaphore(x, y) semaphore_init(x, y)
#define sofie_init_mutex(x) sleeplock_init(x)
#define Mutex SleepLock

void sofie_dump_pmp();
void sofie_set_protected_stack();
void sofie_restore_protected_stack();
void* umalloc(uint32_t size);
void ufree(void* addr);
void sofie_change_owner(void* addr, int tid);
int sofie_new_task(int (*entry)(void));
void sofie_post_message(int tid, void* msg);
void* sofie_get_message();
void sofie_acquire_mutex(struct Mutex *lock);
void sofie_release_mutex(struct Mutex *lock);
void sofie_wait_semaphore(struct Semaphore* sem);
void sofie_post_semaphore(struct Semaphore* sem);
int sofie_printf(const char* format, va_list argptr);
void* sofie_memcpy(void* dst, const void* src, size_t size);
int sofie_rand();

#endif /* __H_SOFIE_H_ */
