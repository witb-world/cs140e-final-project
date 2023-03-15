#ifndef __ARMV6_DEBUG_IMPL_H__
#define __ARMV6_DEBUG_IMPL_H__
#include "armv6-debug.h"

// all your code should go here.  implementation of the debug interface.

// bitfield struct for Data Fault Status Register
// see ARM 1176 pg. 3-64 through 3-65
struct dfsr_reg {
  uint32_t status:4, // 3-65
           domain:4, // 3-65
           :2, // always read as zero, ignore
           s:1,  //part of status field. reset val is zero
           rw:1, // read or write as abort cause. 
                 // 0 means read caused abort,
                 // 1 means write access caused abort
           sd:1, // AXI Decode or Slave error. SBZ.
           :19 // discard bits 13:31
           ;
};

// TODO: add static asserts!

// bitfield struct for Instruction Fault Status Register
// see ARM 1176 pg. 3-66 through 3-68
struct ifsr_reg {
  uint32_t status:4, // indicates type of fault. 0b0010: instruction debug
           :6, // SBZ
           :1, // part of status field, SBZ
           :1, // SBZ
           sd:1, // AXI decode or slave error. SBZ.
           :23 // SBZ
           ;
};

// bitfield struct for Debug Status and Control Register
// status and config info about debug system state
// needed to enable debugging!
// see ARM 1176 pg 13-7 through 13-11
struct dscr_reg {
  uint32_t core_halted:1,
           core_restarted:1,
           method_of_dbg_entry:4, // 0b0001 if breakpoint
                                  // 0b0010 if watchpoint
           sticky_precise_data_abort:1,
           sticky_imprecise_data_abort:1,
           sticky_undefined:1,
           powerdown_disable:1,
           dbgack:1,
           interrupts_bit:1, // 0 if enabled, 1 if disabled
           usr_mode_comms_access:1, // 0 if enabled, 1 if disabled
           exec_arm_inst_enable:1, // 0 if disabled, 1 if enabled
           mode_select:1, // 0 for monitor debug-mode, 1 for halting
           monitor_mode_enable:1, // 0 to disable, 1 to enable
           spiden:1,
           spniden:1,
           nonsecure_world_status:1,
           imprecises_data_aborts_ignored:1, // read-only
           :9, // SBZ
           w_DTR_full:1,
           r_DTR_full:1,
           :1 // SBZ
           ; 
};

// watchpoint control Register
// ARM1176 pg 13-21 through 13-23
struct wcr_reg {
  uint32_t watchpoint_enable:1,
           supervisor_access:2, // 0b11 for user and privileged
           ld_str_access:2, // 0b11 for either
           byte_addr_sel:4, // set which bytes of addr word trigger access. 
           :5, // reserved/SBZ
           secure_match:2, // set to 0b00 to match in secure or non-secure world
           linked_brp:4,
           enable_linking:1, // 0 to disable, 1 to enable
           :15; // SBZ
};

// breakpoint control register
// ARM1176 pg 13-17 through 13-19
struct bcr_reg {
  uint32_t breakpoint_enable:1, // 0 to disable, 1 to enable
           supervisor_access:2, // 0b11 to allow either user or priv
            :2, // SBZ
           byte_addr_select:4, // set to 0b1111
           :5, // SBZ
           secure:2, // 0b00 to match in secure or non-secure
           linked_brp:4,
           enable_linking:1, // 0 to disable
           bvr:2, // context ID matching
           :9 // SBZ
          ;
};

// example of how to define get and set for status registers
coproc_mk(status, p14, 0, c0, c1, 0)

coproc_mk(dfsr, p15, 0, c5, c0, 0)
coproc_mk(ifar, p15, 0, c6, c0, 2)
coproc_mk(ifsr, p15, 0, c5, c0, 1)
coproc_mk(far, p15, 0, c6, c0, 0)

coproc_mk(dscr, p14, 0, c0, c1, 0)
coproc_mk(bvr0, p14, 0, c0, c0, 4)
coproc_mk(bcr0, p14, 0, c0, c0, 5)
coproc_mk(wvr0, p14, 0, c0, c0, 6)
coproc_mk(wcr0, p14, 0, c0, c0, 7)
coproc_mk(wfar, p14, 0, c0, c6, 0)
// you'll need to define these and a bunch of other routines.
static inline uint32_t cp15_dfsr_get(void);
static inline uint32_t cp15_ifar_get(void);
static inline uint32_t cp15_ifsr_get(void);
static inline uint32_t cp14_dscr_get(void);


// static inline uint32_t cp14_wcr0_set(uint32_t r);
// create macros for dfsr, ifar, ifsr, dcsr
//


// return 1 if enabled, 0 otherwise.  
//    - we wind up reading the status register a bunch:
//      could return its value instead of 1 (since is 
//      non-zero).
static inline int cp14_is_enabled(void) {
  // install exception handlers, enable bits in status register

  uint32_t r = cp14_dscr_get();
  struct dscr_reg cp14_status_reg = *(struct dscr_reg *)&r;

  return cp14_status_reg.monitor_mode_enable;
}

// enable debug coprocessor 
static inline void cp14_enable(void) {
  // if it's already enabled, just return?
  if(cp14_is_enabled())
    panic("already enabled\n");

  // for the core to take a debug exception, monitor debug mode has to be both 
  // selected and enabled --- bit 14 clear and bit 15 set.

  uint32_t r = cp14_dscr_get();
  struct dscr_reg cp14_status_reg = *(struct dscr_reg *)&r;
  cp14_status_reg.mode_select = 0b0;
  cp14_status_reg.monitor_mode_enable = 0b1; 
  cp14_dscr_set(*(uint32_t *)&cp14_status_reg);
  printk("set DCSR to: %x\n", *(uint32_t *)&cp14_status_reg);

  assert(cp14_is_enabled());
}

// disable debug coprocessor
static inline void cp14_disable(void) {
  if(!cp14_is_enabled())
    return;

  uint32_t r = cp14_dscr_get();
  struct dscr_reg cp14_status_reg = *(struct dscr_reg *)&r;
  // write zero to disable monitor debug-mode
  cp14_status_reg.monitor_mode_enable = 0b0;
  cp14_dscr_set(*(uint32_t *)&cp14_status_reg);
  assert(!cp14_is_enabled());
}


static inline int cp14_bcr0_is_enabled(void) {
  uint32_t r = cp14_bcr0_get();
  struct bcr_reg b_reg = *(struct bcr_reg *)&r;
  return (b_reg.breakpoint_enable == 1 
      && b_reg.byte_addr_select == 0b1111);
}

static inline void cp14_bcr0_enable(void) {
  printk("calling enable bcr\n");
  uint32_t r = cp14_bcr0_get();
  struct bcr_reg b_reg = *(struct bcr_reg *)&r;
  b_reg.breakpoint_enable = 0b1;
  b_reg.supervisor_access = 0b11;
  b_reg.byte_addr_select = 0b1111;
  b_reg.secure = 0b00;
  b_reg.enable_linking = 0b0;
  b_reg.bvr = 0b00;
  cp14_bcr0_set(*(uint32_t *)&b_reg);

}
static inline void cp14_bcr0_disable(void) {
  uint32_t r = cp14_bcr0_get();
  struct bcr_reg b_reg = *(struct bcr_reg *)&r;
  b_reg.breakpoint_enable = 0b0;
  b_reg.byte_addr_select = 0b0000;
  cp14_bcr0_set(*(uint32_t *)&b_reg);
}

// was this a brkpt fault?
static inline int was_brkpt_fault(void) {
  // use IFSR and then DSCR
  uint32_t i_r = cp15_ifsr_get();
  struct ifsr_reg i_reg = *(struct ifsr_reg *) &i_r; 

  uint32_t d_r = cp14_dscr_get();
  struct dscr_reg d_reg = *(struct dscr_reg *) &d_r;

  return (i_reg.status == 0b0010 && d_reg.method_of_dbg_entry == 0b0001);
}

// was watchpoint debug fault caused by a load?
static inline int datafault_from_ld(void) {
  return bit_isset(cp15_dfsr_get(), 11) == 0;
}
// ...  by a store?
static inline int datafault_from_st(void) {
  return !datafault_from_ld();
}


// 13-33: tabl 13-23
static inline int was_watchpt_fault(void) {
  // use DFSR then DSCR
  uint32_t dfsr = cp15_dfsr_get();
  struct dfsr_reg df = *(struct dfsr_reg *) &dfsr;

  uint32_t dscr = cp14_dscr_get();
  struct dscr_reg ds = *(struct dscr_reg *) &dscr;
  printk("DSCR entry method: %x\n", ds.method_of_dbg_entry);

  return (df.status == 0b0010 
      && ds.method_of_dbg_entry == 0b0010);
}

static inline int cp14_wcr0_is_enabled(void) {
  uint32_t r = cp14_wcr0_get();
  struct wcr_reg w_reg = *(struct wcr_reg *) &r; 
  return w_reg.watchpoint_enable;
}
static inline void cp14_wcr0_enable(void) {
  uint32_t r = cp14_wcr0_get();
  struct wcr_reg w_reg = *(struct wcr_reg *)&r;
  w_reg.watchpoint_enable = 0b1;
  w_reg.supervisor_access = 0b11;
  w_reg.ld_str_access = 0b11;
  w_reg.byte_addr_sel = 0b1111;
  w_reg.secure_match = 0b00;
  w_reg.enable_linking = 0b0;
  cp14_wcr0_set(*(uint32_t*) &w_reg);
  printk("Set WCR0 to: %x\n", *(uint32_t*) &w_reg);
}
static inline void cp14_wcr0_disable(void) {
  uint32_t r = cp14_wcr0_get();
  struct wcr_reg w_reg = *(struct wcr_reg *)&r;
  w_reg.watchpoint_enable = 0b0;
  w_reg.byte_addr_sel = 0b0000;
  cp14_wcr0_set(*(uint32_t*) &w_reg);
}

// Get watchpoint fault using WFAR
static inline uint32_t watchpt_fault_pc(void) {
  return cp14_wfar_get() - 8;
}
static inline uint32_t watchpt_fault_addr(void) {
  return cp15_far_get();

}

#endif
