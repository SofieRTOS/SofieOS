#include "syscall.h"

.global sofie_dump_pmp
.global sofie_set_protected_stack
.global sofie_restore_protected_stack
.global umalloc
.global ufree
.global sofie_change_owner
.global sofie_new_task
.global sofie_post_message
.global sofie_get_message
.global sofie_acquire_mutex
.global sofie_release_mutex
.global sofie_wait_semaphore
.global sofie_post_semaphore
.global sofie_printf
.global sofie_memcpy
.global sofie_rand
.global sofie_time

.align 4
sofie_dump_pmp:
    li a7, SYS_dump_pmp
    ecall
    nop
    ret

.align 4
sofie_set_protected_stack:
    li a7, SYS_set_protected_stack_top
    ecall
    nop
    ret

.align 4
sofie_restore_protected_stack:
    li a7, SYS_restore_protected_stack_top
    ecall
    nop
    ret

.align 4
umalloc:
    li a7, SYS_malloc
    ecall
    nop
    ret

.align 4
ufree:
    li a7, SYS_free
    ecall
    nop
    ret

.align 4
sofie_change_owner:
    li a7, SYS_change_owner
    ecall
    nop
    ret

.align 4
sofie_new_task:
    li a7, SYS_new_task
    ecall
    nop
    ret

.align 4
sofie_post_message:
    li a7, SYS_post_message
    ecall
    nop
    ret

.align 4
sofie_get_message:
    li a7, SYS_get_message
    ecall
    nop
    ret

.align 4
sofie_acquire_mutex:
    li a7, SYS_acquire_sleep_lock
    ecall
    nop
    ret

.align 4
sofie_release_mutex:
    li a7, SYS_release_sleep_lock
    ecall
    nop
    ret

.align 4
sofie_wait_semaphore:
    li a7, SYS_wait_semaphore
    ecall
    nop
    ret

.align 4
sofie_post_semaphore:
    li a7, SYS_post_semaphore
    ecall
    nop
    ret

.align 4
sofie_printf:
    li a7, SYS_printf
    ecall
    nop
    ret

.align 4
sofie_memcpy:
    li a7, SYS_memcpy
    ecall
    nop
    ret

.align 4
sofie_rand:
    li a7, SYS_rand
    ecall
    nop
    ret
