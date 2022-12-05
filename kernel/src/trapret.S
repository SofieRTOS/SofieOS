.global trapret
.align 4
trapret:
    csrrw a0, mscratch, a0

    lw t0, 0(a0)
    csrw mepc, t0

    lw t0, 44(a0)
    csrw mscratch, t0

    lw ra, 8(a0)
    lw sp, 12(a0)
    lw gp, 16(a0)
    lw tp, 20(a0)
    lw t0, 24(a0)
    lw t1, 28(a0)
    lw t2, 32(a0)
    lw fp, 36(a0)
    lw s1, 40(a0)
    lw a1, 48(a0)
    lw a2, 52(a0)
    lw a3, 56(a0)
    lw a4, 60(a0)
    lw a5, 64(a0)
    lw a6, 68(a0)
    lw a7, 72(a0)
    lw s2, 76(a0)
    lw s3, 80(a0)
    lw s4, 84(a0)
    lw s5, 88(a0)
    lw s6, 92(a0)
    lw s7, 96(a0)
    lw s8, 100(a0)
    lw s9, 104(a0)
    lw s10, 108(a0)
    lw s11, 112(a0)
    lw t3, 116(a0)
    lw t4, 120(a0)
    lw t5, 124(a0)
    lw t6, 128(a0)

    csrrw a0, mscratch, a0
    mret
