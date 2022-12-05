#ifndef __H_CALLIMPL_H_
#define __H_CALLIMPL_H_

uint32_t sys_dump_pmp();
uint32_t sys_set_protected_stack_top();
uint32_t sys_restore_protected_stack_top();
uint32_t sys_malloc();
uint32_t sys_free();
uint32_t sys_change_owner();
uint32_t sys_new_task();
uint32_t sys_post_message();
uint32_t sys_get_message();
uint32_t sys_acquire_sleep_lock();
uint32_t sys_release_sleep_lock();
uint32_t sys_wait_semaphore();
uint32_t sys_post_semaphore();
uint32_t sys_printf();
uint32_t sys_memcpy();
uint32_t sys_rand();
uint32_t sys_time();

#endif
