#include <stdarg.h>
#include "libesp.h"
#include "esp-extra.h"

// XXX: this is a hack to get the esp to work.
esp_t esp_init(int tx, int rx, int baud, const char *network, const char *password)
{
    sw_uart_t u = sw_uart_init(tx, rx, baud);
    lex_t *l = (lex_t *)kmalloc(sizeof(lex_t));
    lex_init_uart(l, &u);
    return esp_mk(l, network, password);
}

int esp_start_wifi(esp_t *e)
{
    assert(e->wifi && e->password);
    assert(!e->active_p);

    output("setup_wifi: on my esp this prints out: +CIFSR:APIP,\"%s\"\n", ESP_SERVER_IP);
    output("\tso 192.168.4.1 is the device's address\n");
    assert(strlen(e->password) >= 8);

    return
        // removing call to at_cmdv whille porting over strlib
        at_cmdv(e, "AT+CWSAP=\"%s\",\"%s\",5,3", e->wifi, e->password)
        // at_cmd(e, "AT_CWSAP=\"abcdefgh\",\"12345678\",5,3", "OK")
        && at_cmd(e, "AT+CWMODE=3", "OK") && at_cmd_extra(e, "AT+CIFSR", "OK");
}