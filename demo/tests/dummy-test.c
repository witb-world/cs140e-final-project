#include "rpi.h"
#include "spi.h"
#include "gpio.h"
#include "nrf-hw-support.h"
#include "esp-helpers.h"

enum {GET_MAC_ADDRESS = 0x65};

void notmain(void) {
  spi_t spi_if = pin_init(8, 0);

  uint8_t mac = GET_MAC_ADDRESS;

  // TODO: create buf for TLV structure, use `compose_tlv` helper to build
  //
  char buf[1024];
  char res[1024];
  compose_tlv(buf, &mac, 1);
  printk("Composed TLV\n");
  while (1) {

    spi_n_transfer(spi_if, &mac, res, 1024);
    if (res != 0) {
      printk("Got response: \n");
      break;
    }
  }
  for (int i = 0; i < 1024 / sizeof(uint32_t); i++) {
    printk("%x", res[i*4]);
  }
  printk("\n");
  clean_reboot();

}
  // printk("Hello, world!\n");
