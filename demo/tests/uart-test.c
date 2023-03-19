#include "rpi.h"
#include "sw-uart.h"

#define BUF_SIZE 512 
enum {
  tx_pin = 20,
  rx_pin = 21,
  baud = 115200
};



void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)
  sw_uart_t uart_esp32 = sw_uart_init(tx_pin, rx_pin, baud);

  // send "AT"
  printk("Sending \"AT\" to ESP32.\n");
  sw_uart_putk(&uart_esp32, "AT\r\n");

  // should receive "OK"
  char resp[BUF_SIZE];
  int result = sw_uart_read_timeout(&uart_esp32, resp, BUF_SIZE, 1000);

  if (result == 0) {
    printk("No response over UART\n");
  }
  else {
    printk("Got response: ");
    for (int i = 0; i < BUF_SIZE; i++) {
      printk("%c", resp[i]);
    }
    printk("\n");
  }

  // reboot / clean up
  clean_reboot();

}
  // printk("Hello, world!\n");
