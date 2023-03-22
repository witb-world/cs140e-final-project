// simple software 8n1 UART bit-banging implemention.
//   - look for todo() and implement!
//
// NOTE: using usec is going to be too slow for high baudrate, 
// but should be ok for 115200.
//
#include "rpi.h"
#include "cycle-count.h"
#include "sw-uart.h"
#include "cycle-util.h"

// simple putk to the given <uart>
void sw_uart_putk(sw_uart_t *uart, const char *msg) {
    for(; *msg; msg++)
        sw_uart_put8(uart, *msg);
}

// helper: cleans up the code: do a write for <usec> microseconds
//
// code that uses it will have a lot of compounding error, however.  
// if you switch to using cycles for faster baud rates, you should
// instead use
//      <write_cyc_until> in libpi/include/cycle-util.h
static inline void timed_write(int pin, int v, unsigned usec) {
    gpio_write(pin,v);
    delay_us(usec);
}

// do this first: used timed_write to cleanup.
//  recall: time to write each bit (0 or 1) is in <uart->usec_per_bit>
void sw_uart_put8(sw_uart_t *uart, unsigned char c) {
    unsigned start = cycle_cnt_read();
    unsigned incr = uart->cycle_per_bit;
    write_cyc_until(uart->tx, 0, start, incr);
    for (int i = 0; i < 8; i++) {
        write_cyc_until(uart->tx, (c >> i) & 1, start, (i + 2) * incr);
    }
    write_cyc_until(uart->tx, 1, start, 10 * incr);
}

int sw_uart_get8(sw_uart_t *uart) {
    unsigned UINT_MAX = 4294967295;
    return sw_uart_get8_timeout(uart, UINT_MAX);  // does not timeout
}

// setup the GPIO pins
sw_uart_t sw_uart_mk_helper(unsigned tx, unsigned rx,
        unsigned baud,
        unsigned cyc_per_bit,
        unsigned usec_per_bit) {

    gpio_set_input(rx);
    gpio_set_output(tx);
    gpio_set_on(tx);
    unsigned mhz = 700 * 1000 * 1000;
    unsigned derived = cyc_per_bit * baud;
    // assert((mhz - baud) <= derived && derived <= (mhz + baud));
    // panic("cyc_per_bit = %d * baud = %d\n", cyc_per_bit, cyc_per_bit * baud);

    return (sw_uart_t) {
            .tx = tx,
            .rx = rx,
            .baud = baud,
            .cycle_per_bit = cyc_per_bit ,
            .usec_per_bit = usec_per_bit
    };
}

sw_uart_t sw_uart_mk(unsigned tx, unsigned rx, unsigned baud) {
    return sw_uart_mk_helper(tx, rx, baud, 100000, 100000); // 700 * 1000 * 1000UL / baud, 1000 * 1000 / baud);
}

// IMPLEMENT ALL BELOW
int sw_uart_get8_timeout(sw_uart_t *uart, uint32_t timeout_usec) {
    if (wait_until_usec(uart->rx, 0, timeout_usec) == 0) {  // waits until it finds the first 0
        return -1;
    }
    unsigned ch = 0;
    unsigned time_diff = uart->usec_per_bit;
    for (int i = 0; i < 8; i++) {
        ch += (gpio_read(uart->rx) << i);
        delay_us(time_diff);
    }
    if (wait_until_usec(uart->rx, 1, time_diff) != 1) {  // did not finish with 1, error occurred
        return -1;
    }
    return ch;
}

int sw_uart_gets(sw_uart_t *uart, char *in, unsigned nbytes) {
    for (int i = 0; i < nbytes; i++) {
        int val = sw_uart_get8(uart);  // convert to char
        if (val == 10) {  // new line character
            in[i] = 0;  // null terminate? (do we do this)
            return i;
        } else {
            in[i] = val;
        }
    }
    in[nbytes] = 0;
    return nbytes;
}

void sw_uart_write(sw_uart_t *uart, uint8_t *input, unsigned nbytes) {
    for (int i = 0; i < nbytes; i++) {
        sw_uart_put8(uart, input[i]);
    }
}

void sw_uart_read(sw_uart_t *uart, uint8_t *out, unsigned nbytes) {
    for (int i = 0; i < nbytes; i++) {
        out[i] = sw_uart_get8(uart);
    }
}

int sw_uart_read_timeout(sw_uart_t *uart, uint8_t *out, uint32_t nbytes, uint32_t usec_timeout) {
    for (int i = 0; i < nbytes; i++) {
        int start = timer_get_usec();
        int value = sw_uart_get8_timeout(uart, usec_timeout);
        int end = timer_get_usec();
        usec_timeout -= (end - start);
        if (value == -1 || usec_timeout < 0) {
            return i;
        }
        out[i] = value;
    }
    return nbytes;
}
