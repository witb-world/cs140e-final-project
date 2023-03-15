#include "rpi.h"
#include "spi.h"
#include "gpio.h"
#include "nrf-hw-support.h"

enum {GET_MAC_ADDRESS = 0x65};

void notmain(void) {
  spi_t spi_if = pin_init(8, 0);

  uint8_t mac = GET_MAC_ADDRESS;
  uint8_t res = 0x00;

  while (1) {
    spi_n_transfer(spi_if, &mac, &res, 1);
    if (res != 0) {
      printk("%c", res);
    }
  }
}
  // printk("Hello, world!\n");
