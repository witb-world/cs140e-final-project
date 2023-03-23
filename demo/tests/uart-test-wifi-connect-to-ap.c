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
#define ROUTER_CONNECT "AT+CWJAP=\"ESP32_softAP\",\"passnerd\"\r\n" // "AT+CWJAP=\"Stanford\",\"\"\r\n"
#define GET_IP "AT+CIPSTA?\r\n"
#define GET_UART "AT+UART_CUR?\r\n"
#define SET_UART "AT+UART_CUR=115200,8,1,0,0\r\n"

void send_cmd(sw_uart_t uart_esp32, char* c) {
  printk("Sending %s to ESP32.\n", c);
  char resp[BUF_SIZE];
  memset(resp, 0, BUF_SIZE);

  sw_uart_putk(&uart_esp32, c);
  int result = sw_uart_read_timeout(&uart_esp32, resp, BUF_SIZE, 10000);

  if (result == 0) {
    printk("No response over UART\n");
  }
  else {
    printk("Got response of size %d: ", result);
    for (int i = 0; i < BUF_SIZE; i++) {
      printk("%c", resp[i]);
    }
    printk("\n");
  }
}

void connect_over_TCP_test(sw_uart_t u) {  // works if KC is running TCP server from ESP32
  // nc -l 8888 (KC runs this before I run this function. Ensure the IP address and port are correct) (only works with featherboard??)
  send_cmd(u, SET_WIFI_STATION_MODE);
  send_cmd(u, ROUTER_CONNECT);
  delay_us(10000000);  // this is required
  send_cmd(u, GET_IP);
  delay_ms(1000);  
  send_cmd(u, "AT+CIPSTART=\"TCP\",\"192.168.4.4\",8888\r\n");
  delay_us(10000000);  // this is required
  send_cmd(u, "AT+CIPSEND=5\r\n");
  delay_us(1000000);
  send_cmd(u, " test");
  // KC should receive msg after this call
}

void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)
  sw_uart_t u = sw_uart_init(tx_pin, rx_pin, baud);
  connect_over_TCP_test(u);
  clean_reboot();
}
