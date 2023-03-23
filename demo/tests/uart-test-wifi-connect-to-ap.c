#include "rpi.h"
#include "sw-uart.h"

#define BUF_SIZE 1024 
enum {
  tx_pin = 20,
  rx_pin = 21,
  baud = 115200
};
/*
 * Adapted from AT Command Examples
 * Courtesy of Expressif:
 * https://docs.espressif.com/projects/esp-at/en/latest/esp32/AT_Command_Examples/TCP-IP_AT_Examples.html#esp32-as-a-tcp-client-in-single-connection
 */

#define AT  "AT\r\n"
#define AT_GMR  "AT+GMR\r\n"
#define UART_CUR "AT+UART_CUR?\r\n"
#define GET_WIFI_MODE "AT+CWMODE?\r\n"
#define SET_WIFI_STATION_MODE "AT+CWMODE=1\r\n"
#define ROUTER_CONNECT "AT+CWJAP=\"<YOUR SSID>\",\"<YOUR_PW>\"\r\n"
#define GET_IP "AT+CIPSTA?\r\n"
#define GET_UART "AT+UART_CUR?\r\n"
#define SET_UART "AT+UART_CUR=115200,8,1,0,0\r\n"

void send_cmd(sw_uart_t uart_esp32, char* c) {
  printk("Sending %s to ESP32.\n", c);
  sw_uart_putk(&uart_esp32, c);

  // should receive "OK"
  char resp[BUF_SIZE];
  memset(resp, 0, BUF_SIZE);

  delay_us(1000);

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

}


void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)
  sw_uart_t u = sw_uart_init(tx_pin, rx_pin, baud);

  // send "AT"
  char* cmd = AT;
  send_cmd(u, cmd);
  char* cmd2 = SET_WIFI_STATION_MODE;
  send_cmd(u, cmd2);
  delay_us(750);
  dev_barrier();
  char* cmd3 = ROUTER_CONNECT;
  delay_us(1000 * 1000);
  char* cmd4 = GET_IP;
  send_cmd(u, cmd3);
  delay_us(1000 * 1000);
  send_cmd(u, cmd4);
  // reboot / clean up
  clean_reboot();

}
