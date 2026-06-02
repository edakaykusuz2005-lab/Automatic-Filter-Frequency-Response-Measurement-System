#ifndef STM32_PROTOCOL_H
#define STM32_PROTOCOL_H

#include <stdint.h>

#define PROTO_HEADER_1          0xAAu
#define PROTO_HEADER_2          0xBBu
#define PROTO_MSG_ACK           0x10u
#define PROTO_MSG_NACK          0x20u
#define PROTO_MSG_SET_PARAMS    0x11u
#define PROTO_MSG_GET_PARAMS    0x12u

uint8_t proto_checksum(uint8_t type, uint16_t length, const uint8_t *payload);
uint16_t proto_build_frame(uint8_t type, const uint8_t *payload, uint16_t length, uint8_t *out);
void proto_write_u16_le(uint8_t *buf, uint16_t value);
void proto_write_u32_le(uint8_t *buf, uint32_t value);
void proto_write_f32_le(uint8_t *buf, float value);
uint16_t proto_read_u16_le(const uint8_t *buf);
uint32_t proto_read_u32_le(const uint8_t *buf);
float proto_read_f32_le(const uint8_t *buf);
void proto_process(void);
void proto_send_results(float vin_rms, float vout_rms);

#endif
