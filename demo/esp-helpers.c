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
uint16_t compose_tlv(uint8_t* buf, uint8_t* data, uint16_t data_length)
{
	char* ep_name = CTRL_EP_NAME_RESP;
	uint16_t ep_length = strlen(ep_name);
	uint16_t count = 0;
	buf[count] = PROTO_PSER_TLV_T_EPNAME;
	count++;
	buf[count] = (ep_length & 0xFF);
	count++;
	buf[count] = ((ep_length >> 8) & 0xFF);
	count++;
	strcpy((char *)&buf[count], ep_name);
	count = count + ep_length;
	buf[count]= PROTO_PSER_TLV_T_DATA;
	count++;
	buf[count] = (data_length & 0xFF);
	count++;
	buf[count] = ((data_length >> 8) & 0xFF);
	count++;
	memcpy(&buf[count], data, data_length);
	count = count + data_length;
	return count;
}
