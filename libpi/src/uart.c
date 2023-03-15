// implement:
//  void uart_init(void)
//
//  int uart_can_get8(void);
//  int uart_get8(void);
//
//  int uart_can_put8(void);
//  void uart_put8(uint8_t c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"
typedef enum {
  AUX_ENB = 0x20215004,
  AUX_MU_IO_REG = 0x20215040,
  AUX_MU_IER_REG = 0x20215044,
  AUX_MU_IIR_REG = 0x20215048,
  AUX_MU_LCR_REG = 0x2021504c,
  // skip MCR register
  AUX_MU_LSR_REG = 0x20215054,
  // skip MSR register
  // skip scratch register
  AUX_MU_CNTL_REG = 0x20215060,
  AUX_MU_STAT_REG = 0x20215064,
  AUX_MU_BAUD     = 0x20215068,
} mu_reg_t;

// some explanation: see manual page 11 for Baudrate formula
// baudrate = system_clock_frequency / (8 * MAGIC_BAUD_NUMER + 1)
// system_clock_frequency should be 250 MHz for our hardware.
// For baud 115200, this should be 270 (more or less)
const unsigned MAGIC_BAUD_NUMBER = 270;

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
  dev_barrier();
  // enable GPIO pins to handle Mini-UART
  gpio_set_function(14, GPIO_FUNC_ALT5);
  gpio_set_function(15, GPIO_FUNC_ALT5);
  dev_barrier();

  // then enable Mini-UART from AUX. 
  // This is a RMW operation; we do not want to clobber SPI
  unsigned aux_cur = GET32(AUX_ENB);
  aux_cur |= 0b1; // set bit 0 for MiniUART enable (pg.10);
  PUT32(AUX_ENB, aux_cur);
  dev_barrier();

  // next, disable and clear Tx/Rx buffers and flow control.
  PUT32(AUX_MU_CNTL_REG, 0x00000000); // disable tx/rx: clear bits 0 and 1 (pg. 17)
  PUT32(AUX_MU_IIR_REG,  0b110); // write 1 to bits 1 and 2 to clear tx/rx FIFOs (pg. 13)

  // now: set to 8-bit mode
  PUT32(AUX_MU_LCR_REG, 0x3); // write 0b11 (pg. 14 -- see errata)

  // lastly, disable interrupts (we will be polling instead)
  PUT32(AUX_MU_IER_REG, 0x00000000); // disable interrupts (see pg.12 and errata)

  // after that, set baud to 115200
  PUT32(AUX_MU_BAUD, MAGIC_BAUD_NUMBER); // (see pg. 11, pg 19)

  // ready! add a dev barrier in case caller doesn't.
  PUT32(AUX_MU_CNTL_REG, 0x3); // enable tx/rx
  dev_barrier();
}

// disable the uart.
void uart_disable(void) {
  // disable from AUX
  dev_barrier();

  // wait until transmitter done
  // check bit 9 of stat reg for Tx done (pg 18)
  unsigned tx_done = 0;
  uart_flush_tx();
  while (! tx_done) {
    dev_barrier();
    tx_done = GET32(AUX_MU_STAT_REG) & (1 << 9);
    dev_barrier();
  }
  PUT32(AUX_MU_CNTL_REG, 0x00000000); // disable tx/rx: clear bits 0 and 1 (pg. 17)
  PUT32(AUX_MU_IIR_REG,  0x6); // write 1 to bits 1 and 2 to clear tx/rx FIFOs (pg. 13)
  // disable tx/rx and flush FIFOs
  dev_barrier();

  // disable from AUX device
  unsigned aux_cur = GET32(AUX_ENB);
  unsigned spi_mask = 0x6; // don't clobber SPI (bits 1 and 2)
  aux_cur ^= ~spi_mask;
  PUT32(AUX_ENB, aux_cur);
  dev_barrier();

}


// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_get8(void) {
  dev_barrier();
  while(!uart_has_data()) { /*spin */ }
  dev_barrier();
  return GET32(AUX_MU_IO_REG); // get first byte here
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_put8(void) {
  dev_barrier();
  unsigned stat = GET32(AUX_MU_STAT_REG);
  return 0x2 & stat; // check bit 1 of stat register.
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
// returns < 0 on error.
int uart_put8(uint8_t c) {
  dev_barrier();
  while (!uart_can_put8()) {/* spin */}
  dev_barrier();
  PUT32(AUX_MU_IO_REG, c);
  dev_barrier();
  return 0;
}

// simple wrapper routines useful later.

// 1 = at least one byte on rx queue, 0 otherwise
int uart_has_data(void) {
  dev_barrier();
  unsigned stat = GET32(AUX_MU_LSR_REG);
  dev_barrier();
  return 0x1 & stat; // check bit 0 of stat register.

}

// return -1 if no data, otherwise the byte.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

//TODO: how empty should 'empty' mean? space on TX FIFO, or clear TX FIFO?
// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
  dev_barrier();
  return GET32(AUX_MU_STAT_REG) & (1 << 9);
}

// flush out all bytes in the uart --- we use this when 
// turning it off / on, etc.
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
