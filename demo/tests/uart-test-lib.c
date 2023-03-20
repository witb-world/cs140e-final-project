#include "rpi.h"
#include "sw-uart.h"
#include "libesp.h"

#define BUF_SIZE 1024 



void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)


  // TODO: add back calls to AT cmds
  // This is mostly a test to make sure we can compile
  printk("Hello, world!\n");
  // reboot / clean up
  clean_reboot();

}
  // printk("Hello, world!\n");
