#include <stdarg.h>
#include "libesp.h"
#include "esp-extra.h"
#define BUF_SIZE 1024
// XXX: this is a hack to get the esp to work.
esp_t esp_init(int tx, int rx, int baud, const char *network, const char *password)
{
    // very scuffed, change later
    // need to make sure both structs are on the heap
    sw_uart_t *u = (sw_uart_t *)kmalloc(sizeof(sw_uart_t));
    *u = sw_uart_init(tx, rx, 115200);
    lex_t *l = (lex_t *)kmalloc(sizeof(lex_t));
    lex_init_uart(l, u);
    printk("uart tx: %d\tuart rx: %d\n", u->tx, u->rx);
    return esp_mk(l, network, password);
}

void esp_setup_ap(esp_t *e)
{
    int tx = e->l->u->tx;
    printk("tx: %d\n", tx);
    printk("e->l->u->tx: %u\n", e->l->u->tx);
    assert(e->wifi && e->password);
    assert(!e->active_p);
    printk("wifi: \"%s\"\tpassword: \"%s\"\n", e->wifi, e->password);

    output("setup_wifi: on my esp this prints out: +CIFSR:APIP,\"%s\"\n", ESP_SERVER_IP);
    output("\tso 192.168.4.1 is the device's address\n");
    assert(strlen(e->password) >= 8);

    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    memcpy(buf, "AT+CWSAP=\"", 10);
    memcpy(buf + 10, e->wifi, strlen(e->wifi));
    memcpy(buf + 10 + strlen(e->wifi), "\",\"", 3);
    memcpy(buf + 13 + strlen(e->wifi), e->password, strlen(e->password));
    memcpy(buf + 13 + strlen(e->wifi) + strlen(e->password), "\",5,3\r\n", 7);

    lib_send_cmd(e->l->u, buf);
    delay_ms(100);
    // send_cmdf(e, "AT+CWSAP=\"%s\",\"%s\",5,3\r\n", e->wifi, e->password);
    // return
    //     // removing call to at_cmdv whille porting over strlib
    //     at_cmdv(e, "AT+CWSAP=\"%s\",\"%s\",5,3", e->wifi, e->password)
    //     // at_cmd(e, "AT_CWSAP=\"abcdefgh\",\"12345678\",5,3", "OK")
    //     && at_cmd(e, "AT+CWMODE=3", "OK") && at_cmd_extra(e, "AT+CIFSR", "OK");
}

void lib_send_cmd(sw_uart_t *uart_esp32, char *c)
{
    printk("Sending %s to ESP32.\n", c);
    char resp[BUF_SIZE];
    memset(resp, 0, BUF_SIZE);

    sw_uart_putk(uart_esp32, c);
    int result = sw_uart_read_timeout(uart_esp32, resp, BUF_SIZE, 10000);

    if (result == 0)
    {
        printk("No response over UART\n");
    }
    else
    {
        printk("Got response of size %d: ", result);
        for (int i = 0; i < BUF_SIZE; i++)
        {
            printk("%c", resp[i]);
        }
        printk("\n");
    }
}