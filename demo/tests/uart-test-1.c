#include "rpi.h"
#include "sw-uart.h"

#define BUF_SIZE 1024 
enum {
  tx_pin = 20,
  rx_pin = 21,
  baud = 115200
};

#define  AT  "AT",
#define  AT_GMR  "AT+GMR"
#define UART_CUR "AT+UART_CUR?"
#define GET_WIFI_MODE "AT+CWMODE?"
#define SET_WIFI_STATION_MODE "AT+CWMODE=1"
#define CWLAP "AT+CWLAP=\"Wi-Fi\""


void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)
  sw_uart_t uart_esp32 = sw_uart_init(tx_pin, rx_pin, baud);

  // send "AT"
  char* cmd = AT_GMR;
  strcat(cmd, "\r\n");
  printk("Sending %s to ESP32.\n", cmd);
  sw_uart_putk(&uart_esp32, cmd);

  // should receive "OK"
  char resp[BUF_SIZE];
  memset(resp, 0, BUF_SIZE);
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
