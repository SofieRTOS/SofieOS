#ifndef __RISC_V_H_
#define __RISC_V_H_
#include <stdint.h>

// which hart (core) is this?
static inline uint32_t
r_mhartid()
{
  uint32_t x;
  asm volatile("csrr %0, mhartid" : "=r" (x) );
  return x;
}

// Machine Status Register, mstatus

#define MSTATUS_MPP_MASK (3L << 11) // previous mode.
#define MSTATUS_MPP_M (3L << 11)
#define MSTATUS_MPP_S (1L << 11)
#define MSTATUS_MPP_U (0L << 11)
#define MSTATUS_MIE (1L << 3)    // machine-mode interrupt enable.

static inline uint32_t
r_mcause()
{
  uint32_t x;
  asm volatile("csrr %0, mcause" : "=r" (x) );
  return x;
}

static inline uint32_t
r_mstatus()
{
  uint32_t x;
  asm volatile("csrr %0, mstatus" : "=r" (x) );
  return x;
}

static inline void
w_mstatus(uint32_t x)
{
  asm volatile("csrw mstatus, %0" : : "r" (x));
}

// machine exception program counter, holds the
// instruction address to which a return from
// exception will go.
static inline void
w_mepc(uint32_t x)
{
  asm volatile("csrw mepc, %0" : : "r" (x));
}

static inline uint32_t
r_mepc()
{
	uint32_t x;
	asm volatile("csrr %0, mepc" : "=r" (x));
	return x;
}

static inline uint32_t
r_mtval()
{
	uint32_t x;
	asm volatile("csrr %0, mtval" : "=r" (x));
	return x;
}


// Machine-mode Interrupt Enable
#define MIE_MEIE (1L << 11) // external
#define MIE_MTIE (1L << 7)  // timer
#define MIE_MSIE (1L << 3)  // software
static inline uint32_t
r_mie()
{
  uint32_t x;
  asm volatile("csrr %0, mie" : "=r" (x) );
  return x;
}

static inline void
w_mie(uint32_t x)
{
  asm volatile("csrw mie, %0" : : "r" (x));
}

// Machine-mode interrupt vector
static inline void
w_mtvec(uint32_t x)
{
  asm volatile("csrw mtvec, %0" : : "r" (x));
}


static inline void
w_mscratch(uint32_t x)
{
  asm volatile("csrw mscratch, %0" : : "r" (x));
}

// machine-mode cycle counter
static inline uint32_t
r_time()
{
  uint32_t x;
  asm volatile("csrr %0, time" : "=r" (x) );
  return x;
}

static inline uint32_t
r_fp()
{
  uint32_t x;
  asm volatile("mv %0, fp" : "=r" (x) );
  return x;
}


static inline uint32_t
r_sp()
{
  uint32_t x;
  asm volatile("mv %0, sp" : "=r" (x) );
  return x;
}

static inline uint32_t
r_gp()
{
  uint32_t x;
  asm volatile("mv %0, gp" : "=r" (x) );
  return x;
}

// read and write tp, the thread pointer, which holds
// this core's hartid (core number), the index into cpus[].
static inline uint32_t
r_tp()
{
  uint32_t x;
  asm volatile("mv %0, tp" : "=r" (x) );
  return x;
}

static inline void
w_tp(uint32_t x)
{
  asm volatile("mv tp, %0" : : "r" (x));
}

static inline uint32_t
r_ra()
{
  uint32_t x;
  asm volatile("mv %0, ra" : "=r" (x) );
  return x;
}

static inline void
intr_off()
{
	w_mstatus(r_mstatus() & ~MSTATUS_MIE);
}

static inline void
intr_on()
{
	w_mstatus(r_mstatus() | MSTATUS_MIE);
}

#endif
