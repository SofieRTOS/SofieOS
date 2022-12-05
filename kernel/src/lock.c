#include "lock.h"

void spinlock_init(struct SpinLock* lock) {
	lock->locked = 0;
}

void spinlock_acquire(struct SpinLock* lock) {
	while (__sync_lock_test_and_set(&lock->locked, 1) != 0);
	__sync_synchronize();
}

void spinlock_release(struct SpinLock* lock) {
	__sync_synchronize();
	__sync_lock_release(&lock->locked);
}

void sleeplock_init(struct SleepLock* lock) {
	spinlock_init(&lock->spinlock);
	lock->locked = 0;
}

void sleeplock_acquire(struct SleepLock* lock) {
	spinlock_acquire(&lock->spinlock);
	while (lock->locked) {
		sleep(lock, &lock->spinlock);
	}

	lock->locked = 1;
	spinlock_release(&lock->spinlock);
}

void sleeplock_release(struct SleepLock* lock) {
	spinlock_acquire(&lock->spinlock);
	lock->locked = 0;
	wakeup(lock);
	spinlock_release(&lock->spinlock);
}

void semaphore_init(struct Semaphore* semaphore, int value) {
	spinlock_init(&semaphore->spinlock);
	semaphore->value = value;
}

void semaphore_wait(struct Semaphore* semaphore) {
	spinlock_acquire(&semaphore->spinlock);
	while (semaphore->value == 0) {
		sleep(semaphore, &semaphore->spinlock);
	}

	semaphore->value --;
	spinlock_release(&semaphore->spinlock);
}

void semaphore_post(struct Semaphore* semaphore) {
	spinlock_acquire(&semaphore->spinlock);
	semaphore->value ++;
	wakeup(semaphore);
	spinlock_release(&semaphore->spinlock);
}
