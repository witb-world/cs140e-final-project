#include "rpi.h"
#include "spi.h"
#include "gpio.h"
#include "nrf-hw-support.h"
#include "esp-helpers.h"

#define BUF_SIZE 1024
enum {GET_MAC_ADDRESS = 0x65,
GET_WIFI_MODE = 0x67};

void notmain(void) {
  spi_t spi_if = pin_init(8, 0);

  uint8_t mac = GET_MAC_ADDRESS;
  uint8_t w_mode = GET_WIFI_MODE;

  // TODO: create buf for TLV structure, use `compose_tlv` helper to build
  //
  char buf[BUF_SIZE];
  char res[BUF_SIZE];
  memset(buf, 0, sizeof(buf));
  memset(res, 0, sizeof(res));
  compose_tlv(buf, &mac, 2);
  printk("Composed TLV\n");
  printk("-----Current TLV buffer----\n");
  for (int i = 0; i < sizeof(buf) / sizeof(uint32_t); i++) {
    printk("%x", buf[i*4]);
  }
  printk("\n---------------------------\n");
  spi_n_transfer(spi_if, res, buf, BUF_SIZE);
  // while (1) {
  //   if (*res != 0) {
  //     printk("Got response: \n");
  //     break;
  //   }
  // }
  for (int i = 0; i < sizeof(res) / sizeof(uint32_t); i++) {
    printk("%x", res[i*4]);
  }
  printk("\n");
  clean_reboot();

}
  // printk("Hello, world!\n");
