#ifndef __ESP_EXTRA_H__
#define __ESP_EXTRA_H__
#include "esp-run.h"
esp_t esp_init(int tx, int rx, int baud, const char *network, const char *password);

int esp_start_wifi(esp_t *e);
void lib_send_cmd(sw_uart_t *uart_esp32, char *c);
#endif