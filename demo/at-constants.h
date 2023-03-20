// a timeout of 750 usec between sending AT commands and getting response
// seems to be a sweet spot in my setup. May not hold for more than basic polling.
#define TIMEOUT 750 

/* constants for SW-UART setup */
enum {
  tx_pin = 20,
  rx_pin = 21,
  baud = 115200
};
