#ifndef __H_LOCK_H_
#define __H_LOCK_H_
#include <stdint.h>
#include "task.h"

extern struct SpinLock;
extern struct SleepLock;
extern struct Semaphore;

void spinlock_init(struct SpinLock* lock);
void spinlock_acquire(struct SpinLock* lock);
void spinlock_release(struct SpinLock* lock);

void sleeplock_init(struct SleepLock* lock);
void sleeplock_acquire(struct SleepLock* lock);
void sleeplock_release(struct SleepLock* lock);

void semaphore_init(struct Semaphore* semaphore, int value);
void semaphore_wait(struct Semaphore* semaphore);
void semaphore_post(struct Semaphore* semaphore);

#endif /* __H_SPINLOCK_H_ */
