#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "libc/bit-support.h"
#include "asm-helpers.h"

/*
 * vector base address register:
 *   3-121 --- let's us control where the exception jump table is!
 *
 * defines: 
 *  - vector_base_set  
 *  - vector_base_get
 */

// return the current value vector base is set to.
static inline void *vector_base_get(void) {
  void* base;
  asm volatile("MRC p15, #0, %0, c12, c0, #0": "=r" (base));
  dev_barrier();
  return base;
}

// check that not null and alignment is good.
static inline int vector_base_chk(void *vector_base) {
  // check that vector base address ptr is not null, 
  // also check alignment: should be divisible by 32 (word-aligned).
  // alignment spec (3.2.43) specifies 5 LSB must be zero.
  return (vector_base != 0 && (unsigned)vector_base % 32 == 0);
}

// set vector base: must not have been set already.
static inline void vector_base_set(void *vec) {
  if(!vector_base_chk(vec))
    panic("illegal vector base %p\n", vec);

  void *v = vector_base_get();
  if(v) 
    panic("vector base register already set=%p\n", v);

  dev_barrier();
  asm volatile("MCR p15, #0, %0, c12, c0, #0" : :  "r" (vec));

  dev_barrier();
  v = vector_base_get();
  if(v != vec)
    panic("set vector=%p, but have %p\n", vec, v);
}

// reset vector base and return old value: could have
// been previously set.
static inline void *
vector_base_reset(void *vec) {
  if(!vector_base_chk(vec))
    panic("illegal vector base %p\n", vec);

  volatile void *old_vec = vector_base_get();
  asm volatile("MCR p15, #0, %0, c12, c0, #0": : "r" (vec));
  // printk("Called reset. should have updated from %p to %p\n", old_vec, vec);
  dev_barrier();

  return (void*) old_vec;
}
#endif
