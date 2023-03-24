#include "rpi.h"
#include "sw-uart.h"
#include "esp-extra.c"

#define BUF_SIZE 1024
enum
{
    tx_pin = 20,
    rx_pin = 21,
    baud = 115200
};
/*
 * Adapted from AT Command Examples
 * Courtesy of Expressif:
 * https://docs.espressif.com/projects/esp-at/en/latest/esp32/AT_Command_Examples/TCP-IP_AT_Examples.html#esp32-as-a-tcp-client-in-single-connection
 */

#define AT "AT\r\n"
#define AT_GMR "AT+GMR\r\n"
#define UART_CUR "AT+UART_CUR?\r\n"
#define GET_WIFI_MODE "AT+CWMODE?\r\n"
#define SET_WIFI_STATION_MODE "AT+CWMODE=2\r\n"
#define ROUTER_CONNECT "AT+CWJAP=\"Kelechi's iPhone\",\"eightchar\"\r\n"
#define GET_IP "AT+CIPSTA?\r\n"
#define GET_UART "AT+UART_CUR?\r\n"
#define SET_UART "AT+UART_CUR=115200,8,1,0,0\r\n"

void send_cmd(sw_uart_t uart_esp32, char *c)
{
    printk("Sending %s to ESP32.\n", c);
    char resp[BUF_SIZE];
    memset(resp, 0, BUF_SIZE);

    sw_uart_putk(&uart_esp32, c);
    int result = sw_uart_read_timeout(&uart_esp32, resp, BUF_SIZE, 10000);

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

void notmain(void)
{
    // init uart
    // RX on pin 21
    // TX on pin 20
    // baud rate: default (flashed to 115200)
    esp_t esp_server = esp_init(tx_pin, rx_pin, baud, "ESP32_AP_KC", "passnerd");
    printk("esp uart: %d\n", esp_server.l->u->tx);
    esp_setup_ap(&esp_server);

    // send "AT"
    char *cmd = AT;
    send_cmd(*(esp_server.l->u), cmd);
    // char *cmd2 = SET_WIFI_STATION_MODE;
    // send_cmd(u, cmd2);
    // delay_us(750);
    // dev_barrier();
    // char *cmd3 = "AT+CIPMUX=1\r\n";
    // send_cmd(u, cmd3);
    // delay_ms(1000);
    // char *cmd4 = "AT+CWSAP=\"ESP32_softAP\",\"passnerd\",5,3\r\n";
    // send_cmd(u, cmd4);
    // delay_ms(1000);
    // char *cmd5 = "AT+CWSAP?\r\n";
    // send_cmd(u, cmd5);
    // char *cmd6 = "AT+CIPAP?\r\n";
    // send_cmd(u, cmd6);
    // char *cmd7 = "AT+CIPSERVER=1,80\r\n";
    // char *cmd3 = ROUTER_CONNECT;
    // delay_us(1000 * 1000);
    // char *cmd4 = GET_IP;
    // send_cmd(u, cmd3);
    // delay_us(1000 * 1000);
    // send_cmd(u, cmd4);
    // reboot / clean up
    clean_reboot();
}

// return at_cmdv(e, "AT+CWSAP=\"%s\",\"%s\",5,3", e->wifi, e->password) && at_cmd(e, "AT+CWMODE=3", "OK") && at_cmd_extra(e, "AT+CIFSR", "OK");