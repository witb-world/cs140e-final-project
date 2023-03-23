#ifndef __ESP_EXTRA_H__
#define __ESP_EXTRA_H__

esp_t esp_init(int tx, int rx, int baud, const char *network, const char *password);

int esp_start_wifi(esp_t *e);

#endif