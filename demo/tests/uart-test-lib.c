#include "rpi.h"
#include "sw-uart.h"
#include "esp-lex-simple.h"
#include "libesp.h"
#include "esp-commands.h"

#define BUF_SIZE 1024 



void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)


  // init uart
  sw_uart_t u = sw_uart_init(20, 21, 115200);

  lex_t *l = (lex_t *) kmalloc(sizeof(lex_t));
  lex_init_uart(l, &u);

  esp_t e = esp_mk(l, NETWORK, PASSWORD);

  // esp_drain(&e);
//  esp_hard_reset(&e);

  esp_is_up(&e);
  at_cmd(&e, "AT+GMR", "OK");
  clean_reboot();

}
  // printk("Hello, world!\n");
