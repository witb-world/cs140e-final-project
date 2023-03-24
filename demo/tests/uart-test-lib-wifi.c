#include "rpi.h"
#include "sw-uart.h"
#include "esp-lex-simple.h"
#include "libesp.h"
#include "esp-commands.h"
#include <stdlib.h>

#define BUF_SIZE 1024 



void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)


  // init uart
  sw_uart_t *u = (sw_uart_t*) kmalloc(sizeof(sw_uart_t));
  *u = sw_uart_init(20, 21, 115200);

  lex_t *l = (lex_t *) kmalloc(sizeof(lex_t));
  lex_init_uart(l, u);

  esp_t e = esp_mk(l, NETWORK, PASSWORD);

  // esp_drain(&e);
//  esp_hard_reset(&e);
//
  dev_barrier();
  debug("Checking if ESP is up\n");
  esp_is_up(&e);
  dev_barrier();
  delay_us(1000 * 1000);
  dev_barrier();

  debug("ESP is up! getting IP info...\n");
  dev_barrier();
  delay_us(1000 * 1000);
  dev_barrier();
  at_cmd_extra(&e, "AT+CIPSTA?", "OK");
  dev_barrier();
  debug("getting UART info...\n");
  delay_us(1000 * 1000);
  at_cmd_extra(&e, "AT+UART_CUR?", "OK");
  dev_barrier();
  delay_us(1000 * 1000);
  esp_setup_wifi(&e);
  dev_barrier();
  clean_reboot();
}
  // printk("Hello, world!\n");
