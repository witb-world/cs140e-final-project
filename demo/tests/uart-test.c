#include "rpi.h"
#include "sw-uart.h"

#define BUF_SIZE 1024 
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
  char* cmd = "AT+GMR";
  strcat(cmd, "\r\n");
  printk("Sending \"AT\" to ESP32.\n");
  sw_uart_putk(&uart_esp32, cmd);
  // sw_uart_put8(&uart_esp32, 'A');
  // sw_uart_put8(&uart_esp32, 'T');
  // sw_uart_put8(&uart_esp32, '\r');
  // sw_uart_put8(&uart_esp32, '\n');

  // should receive "OK"
  char resp[BUF_SIZE];
  memset(resp, 0, BUF_SIZE);
  int result = sw_uart_read_timeout(&uart_esp32, resp, BUF_SIZE, 1000);

  // int result = 1;
  // sw_uart_read(&uart_esp32, resp, BUF_SIZE);
  // int num_bytes = sw_uart_gets(&uart_esp32, resp, BUF_SIZE);
  // num_bytes = sw_uart_gets(&uart_esp32, resp+ num_bytes, BUF_SIZE);

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
