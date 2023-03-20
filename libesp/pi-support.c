/*
 * put platform-specific routines for:
 *  - esp_read
 *  - exp_write_exact
 *  - esp_has_data_timeout.
 */
#include "libesp.h"
#include "gpio.h"
#include "cycle-util.h"



// called when the lex buffer has no characters.  reads in more from the esp.
int esp_read(lex_t *l, void *buf, unsigned maxbytes, unsigned usec) {
  int result = sw_uart_read_timeout(l->u, buf, maxbytes, usec);
  int n_bytes_read = strlen(buf);
  if (result >= 0) 
    return n_bytes_read;
  else
    return result;
}

// returns <n> if wrote, 0 otherwise.
int esp_write_exact(lex_t *l, const void *buf, unsigned n) {
  sw_uart_write(l->u, (uint8_t *)buf, n);
  return n;
}

int esp_has_data_timeout(lex_t *l, unsigned usec) {
  return wait_until_usec(l->u->rx, 0, usec);
}

// sleep for <usec> microseconds.
int esp_usleep(unsigned usec) {
    delay_us(usec);
    return usec;
}
