#include "rpi.h"
#include "sw-uart.h"
#include "at-constants.h"
// #include "esp-parse.h"

#define BUF_SIZE 1024

#define AT  "AT"
#define AT_GMR  "AT+GMR"
#define UART_CUR "AT+UART_CUR?"

typedef struct {
  sw_uart_t *u;
 // lex_t *l;

} at_dev_t;

/*
 * Library providing wrappers for basic AT commands
 * sent over SW-UART
 */

/*
 * initialize ESP32 for AT commands over UART
 */
at_dev_t at_init() {
  sw_uart_t *u = kmalloc(sizeof (sw_uart_t));
  *u = sw_uart_init(tx_pin, rx_pin, baud);
  return (at_dev_t) {
    .u = u,
 //   .l =l
  };
}

/*
 * send command to AT device, receive response
 * will print response and save it to buffer.
 */
int send_at_cmd(at_dev_t a, char* cmd, char* res) {
  debug("Sending %s to ESP32.\n", cmd);
  strcat(cmd, "\r\n");
  sw_uart_putk(a.u, cmd);

  memset(res, 0, BUF_SIZE);
  delay_us(TIMEOUT);

  int result = sw_uart_read_timeout(a.u, res, BUF_SIZE, TIMEOUT);
  // int match = match_line(a.l, cmd);
  // printk("Match? %d\n", match);

  if (result == 0) {
    printk("No response over UART\n");
  }
  else {
    printk("Got response: ");
    for (int i = 0; i < BUF_SIZE; i++) {
      printk("%c", res[i]);
    }
    printk("\n");
  }
  return result;
}

int get_gmr(at_dev_t a, char* res) {
  char* at = AT_GMR;
  return send_at_cmd(a, AT_GMR, res);
}

int get_at(at_dev_t a, char* res) {
  char* at = AT;
  return send_at_cmd(a, at, res);
}
