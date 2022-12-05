.global trampoline
.align 4
trampoline:
    csrrw a0, mscratch, a0
    sw ra, 8(a0)
    sw sp, 12(a0)
    sw gp, 16(a0)
    sw tp, 20(a0)
    sw t0, 24(a0)
    sw t1, 28(a0)
    sw t2, 32(a0)
    sw fp, 36(a0)
    sw s1, 40(a0)
    sw a1, 48(a0)
    sw a2, 52(a0)
    sw a3, 56(a0)
    sw a4, 60(a0)
    sw a5, 64(a0)
    sw a6, 68(a0)
    sw a7, 72(a0)
    sw s2, 76(a0)
    sw s3, 80(a0)
    sw s4, 84(a0)
    sw s5, 88(a0)
    sw s6, 92(a0)
    sw s7, 96(a0)
    sw s8, 100(a0)
    sw s9, 104(a0)
    sw s10, 108(a0)
    sw s11, 112(a0)
    sw t3, 116(a0)
    sw t4, 120(a0)
    sw t5, 124(a0)
    sw t6, 128(a0)

    # save the user a0 in trapframe->a0
    csrr t0, mscratch
    sw t0, 44(a0)

    # restore kernel stack pointer from trapframe->kernel_sp
    lw sp, 4(a0)

    csrrw a0, mscratch, a0
    # jump to usertrap()
    la t0, usertrap
    jr t0

