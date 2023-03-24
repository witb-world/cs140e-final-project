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
  // printk("Sending %s to ESP32.\n", c);

  char* resp = kmalloc(BUF_SIZE);
  memset(resp, 0, BUF_SIZE);

  sw_uart_putk(&uart_esp32, c);
  int result = sw_uart_read_timeout(&uart_esp32, resp, BUF_SIZE, 10000);

  if (result == 0) {
    printk("No response over UART\n");
  }
  else {
    // printk("Got response of size %d: ", result);
    // for (int i = 0; i < BUF_SIZE; i++) {
    //   printk("%c", resp[i]);
    // }
    // printk("\n");
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

void do_blink(uint32_t gpio_pin){
  gpio_set_output(gpio_pin);
  delay_us(10000);
  printk("Will start blinking!\n");
  for (int i = 0; i < 10; i++) {
    gpio_set_on(gpio_pin);
    delay_ms(500);
    gpio_set_off(gpio_pin);
    delay_ms(500);
  }
}

void notmain(void) {
  // init uart
  // RX on pin 21
  // TX on pin 20
  // baud rate: default (flashed to 115200)
  sw_uart_t server_u = sw_uart_init(tx_pin, rx_pin, baud);


  send_cmd(server_u, "AT+CWMODE=2\r\n");  // Wifi mode to softAP
  send_cmd(server_u, "AT+CIPMUX=1\r\n"); // enable multiple connections
  send_cmd(server_u, "AT+CWSAP=\"ESP32_softAP_Eli\",\"passnerd\",5,3\r\n"); // set softAP
  delay_us(10000000);
  send_cmd(server_u, "AT+CIPAP?\r\n"); // Query softAP information
  send_cmd(server_u, "AT+CIPSERVER=1\r\n"); // set up TCP server (port is 333 by default)

  // ensure PC is connected to ESP32_softAP
  // dev_barrier();
  // printk("Connect to server now.\n");  // unsure if these lines are needed
  // delay_us(10000000);
  // dev_barrier();

  // get a client to connect to the TCP server (use second feather here)
  // connect_over_TCP_test(client_u);  // make sure the IP address and port are right (they probably aren't)
  // send_cmd(server_u, "AT+CIPSEND=0,33\r\n");  // sending 5 bytes to connection link 0
  // delay_us(1000000);
  // send_cmd(server_u, "This is server to client message!\r\n");  // the 4 bytes are " test"
  // delay_us(10000000);
  send_cmd(server_u, "AT+CIPRECVMODE=1\r\n");
  // send_cmd(server_u, "AT+CIPSEND=0,21\r\n");
  // send_cmd(server_u, "Will begin listening!\r\n");
  delay_us(10000000);
  dev_barrier();
  printk("Ready to receive commands!\n");
  dev_barrier();
  while (1) {
    char* temp = send_cmd(server_u, "AT+CIPRECVLEN?\r\n") + 28;
    // printk("temp: %c\n", *temp);
    if (temp[0] != '-' || temp[0] != '0') {  // received data from connection link 0
      char* data = send_cmd(server_u, "AT+CIPRECVDATA=0,1024\r\n");  // data from connection link 0
      data += 38;
      int end_of_command = strlen(data);
      data[end_of_command - 25] = 0;  // data now contains what connection link 0 sent

      if (!strncmp(data, "blink19", 7)) {  // if the blink command is sent
        dev_barrier();
        do_blink(19);
        dev_barrier();

      }
      else if (!strncmp(data, "blink22", 7)){
        dev_barrier();
        do_blink(22);
        dev_barrier();
      }
    else if (!strncmp(data, "kill", 4)) {  // client kills server
                                               //
        break;
      } else {  // otherwise continue
       // printk("Data: %s\t%d\n", data, strlen(data));
        continue;
      }
    }
  }
  delay_us(10000000);
  send_cmd(server_u, "AT+CIPCLOSE=0\r\n"); // close the TCP connection.
  clean_reboot();
}
