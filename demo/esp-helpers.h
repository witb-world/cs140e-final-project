#include "rpi.h"

#define CTRL_EP_NAME_RESP                         "ctrlResp"
#define PROTO_PSER_TLV_T_EPNAME           0x01
#define PROTO_PSER_TLV_T_DATA             0x02

/*
 * Note: description and header copied from esp-hosted
 * https://github.com/espressif/esp-hosted/blob/master/esp_hosted_fg/docs/common/serial_apis.md#12-uint16_t-compose_tlvuint8_t-buf-uint8_t-data-uint16_t-data_length
 *
 * The data written on serial driver file, `SERIAL_IF_FILE` from adapter.h
 * In TLV i.e. Type Length Value format, to transfer data between host and ESP32
 *  | type | length | value |
 * Types are 0x01 : for endpoint name
 *           0x02 : for data
 * length is respective value field's data length in 16 bits
 * value is actual data to be transferred
 */
uint16_t compose_tlv(uint8_t* buf, uint8_t* data, uint16_t data_length);
