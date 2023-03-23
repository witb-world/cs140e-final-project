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
#define ROUTER_CONNECT "AT+CWJAP=\"Stanford\",\"\"\r\n" // "AT+CWJAP=\"ESP32_softAP\",\"passnerd\"\r\n" 
#define GET_IP "AT+CIPSTA?\r\n"
#define GET_UART "AT+UART_CUR?\r\n"
#define SET_UART "AT+UART_CUR=115200,8,1,0,0\r\n"

// TODOS:
// creating the server/client relationship with the feathers and not the computers (make feather be a server)

char* send_cmd(sw_uart_t uart_esp32, char* c) {
  printk("Sending %s to ESP32.\n", c);

  char* resp = kmalloc(BUF_SIZE);
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
  return resp;
}

void connect_over_TCP_test(sw_uart_t u) {  // works if KC is running TCP server from ESP32
  // nc -l 8888 (KC runs this before I run this function. Ensure the IP address and port are correct) (only works with featherboard??)
  send_cmd(u, SET_WIFI_STATION_MODE);
  send_cmd(u, ROUTER_CONNECT);
  delay_us(10000000);  // this is required
  send_cmd(u, GET_IP);
  delay_ms(1000);  
  send_cmd(u, "AT+CIPSTART=\"TCP\",\"192.168.4.1\",333\r\n");  // become a client
  delay_us(10000000);  // this is required
  send_cmd(u, "AT+CIPSEND=5\r\n");
  delay_us(1000000);
  send_cmd(u, "blink");
  // KC should receive msg after this call
}

void ESP32_as_TCP_server_multiple_connections(sw_uart_t server_u, sw_uart_t client_u) {
  send_cmd(server_u, "AT+CWMODE=2\r\n");  // Wifi mode to softAP
  send_cmd(server_u, "AT+CIPMUX=1\r\n"); // enable multiple connections
  send_cmd(server_u, "AT+CWSAP=\"ESP32_softAP\",\"passnerd\",5,3\r\n"); // set softAP
  delay_us(10000000);
  send_cmd(server_u, "AT+CIPAP?\r\n"); // Query softAP information
  send_cmd(server_u, "AT+CIPSERVER=1\r\n"); // set up TCP server (port is 333 by default)

  // ensure PC is connected to ESP32_softAP
  printk("Connect the PC to ESP32_softAP (10 sec to do so)");  // unsure if these lines are needed
  delay_ms(10000000);


  // get a client to connect to the TCP server (use second feather here)
  connect_over_TCP_test(client_u);  // make sure the IP address and port are right (they probably aren't)

  send_cmd(server_u, "AT+CIPSEND=0,5\r\n");  // sending 5 bytes to connection link 0
  delay_us(1000000);
  send_cmd(server_u, "_test\r\n");  // the 4 bytes are " test"
  delay_us(10000000);
  send_cmd(server_u, "AT+CIPCLOSE=0\r\n"); // close the TCP connection.
}

void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)
  #if 0
  sw_uart_t server_u = sw_uart_init(tx_pin, rx_pin, baud);


  send_cmd(server_u, "AT+CWMODE=2\r\n");  // Wifi mode to softAP
  send_cmd(server_u, "AT+CIPMUX=1\r\n"); // enable multiple connections
  send_cmd(server_u, "AT+CWSAP=\"ESP32_softAP_Isaac\",\"passnerd\",5,3\r\n"); // set softAP
  delay_us(10000000);
  send_cmd(server_u, "AT+CIPAP?\r\n"); // Query softAP information
  send_cmd(server_u, "AT+CIPSERVER=1\r\n"); // set up TCP server (port is 333 by default)

  // ensure PC is connected to ESP32_softAP
  dev_barrier();
  printk("Connect the PC to ESP32_softAP (10 sec to do so)\n");  // unsure if these lines are needed
  delay_us(10000000);
  dev_barrier();

  // get a client to connect to the TCP server (use second feather here)
  // connect_over_TCP_test(client_u);  // make sure the IP address and port are right (they probably aren't)
  send_cmd(server_u, "AT+CIPSEND=0,33\r\n");  // sending 5 bytes to connection link 0
  delay_us(1000000);
  send_cmd(server_u, "This is server to client message!\r\n");  // the 4 bytes are " test"
  delay_us(10000000);
  send_cmd(server_u, "AT+CIPRECVMODE=1\r\n");
  dev_barrier();
  printk("Send data now?\r\n");
  delay_us(10000000);
  dev_barrier();
  char* data = send_cmd(server_u, "AT+CIPRECVDATA=0,5\r\n");
  data += 35;
  data[5] = 0;
  // dev_barrier();
  // printk("DATA RECEIVED:\n");
  // printk(data);
  // printk("\n");
  // dev_barrier();
  dev_barrier();
  #endif
  char* data = "blink";
  if (!strcmp(data, "blink")) {
    gpio_set_output(22);
    delay_us(10000);
    printk("Will start blinking!");
    for (int i = 0; i < 100; i++) {
      gpio_set_on(22);
      delay_ms(1000);
      gpio_set_off(22);
      delay_ms(1000);
    }
  }
  // dev_barrier();
  // delay_us(10000000);
  // send_cmd(server_u, "AT+CIPCLOSE=0\r\n"); // close the TCP connection.


  // sw_uart_t client_u = sw_uart_init(15, 16, baud); 
  // ESP32_as_TCP_server_multiple_connections(server_u, client_u);
  // connect_over_TCP_test(u);
  clean_reboot();
}
