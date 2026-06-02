#include "stm32_protocol.h"
#include <string.h>
#include "main.h"
#include "AD9833.h"
#include "usbd_cdc_if.h"

extern uint8_t usb_rx_buffer[128];
extern uint16_t usb_rx_index;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim8;
extern ADC_HandleTypeDef hadc3;
extern ADC_HandleTypeDef hadc2;
//extern uint16_t adcVinBuffer[ADC_BUFFER_SIZE];
//extern uint16_t adcVoutBuffer[ADC_BUFFER_SIZE];
extern uint16_t adcBuffer[ADC_BUFFER_SIZE];

static void SetTimer1PscArr(uint16_t psc, uint16_t arr);
static void StartMeasurement(void);
static void Send_ACK(void);
static void Send_NACK(void);

uint8_t proto_checksum(uint8_t type, uint16_t length, const uint8_t *payload)
{
    uint32_t sum = type + (uint8_t)(length & 0xFFu) + (uint8_t)((length >> 8) & 0xFFu);
    for (uint16_t i = 0u; i < length; ++i)
        sum += payload[i];
    return (uint8_t)(sum & 0xFFu);
}

uint16_t proto_build_frame(uint8_t type, const uint8_t *payload, uint16_t length, uint8_t *out)
{
    uint16_t idx = 0u;
    out[idx++] = PROTO_HEADER_1;
    out[idx++] = PROTO_HEADER_2;
    out[idx++] = type;
    out[idx++] = (uint8_t)(length & 0xFFu);
    out[idx++] = (uint8_t)((length >> 8) & 0xFFu);
    for (uint16_t i = 0u; i < length; ++i)
        out[idx++] = payload[i];
    out[idx++] = proto_checksum(type, length, payload);
    return idx;
}

void proto_write_u16_le(uint8_t *buf, uint16_t value)
{
    buf[0] = (uint8_t)(value & 0xFFu);
    buf[1] = (uint8_t)((value >> 8) & 0xFFu);
}

void proto_write_u32_le(uint8_t *buf, uint32_t value)
{
    buf[0] = (uint8_t)(value & 0xFFu);
    buf[1] = (uint8_t)((value >> 8) & 0xFFu);
    buf[2] = (uint8_t)((value >> 16) & 0xFFu);
    buf[3] = (uint8_t)((value >> 24) & 0xFFu);
}

void proto_write_f32_le(uint8_t *buf, float value)
{
    memcpy(buf, &value, 4u);
}

uint16_t proto_read_u16_le(const uint8_t *buf)
{
    return (uint16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
}

uint32_t proto_read_u32_le(const uint8_t *buf)
{
    return (uint32_t)((uint32_t)buf[0] |
                     ((uint32_t)buf[1] << 8) |
                     ((uint32_t)buf[2] << 16) |
                     ((uint32_t)buf[3] << 24));
}

float proto_read_f32_le(const uint8_t *buf)
{
    float value;
    memcpy(&value, buf, 4u);
    return value;
}

void proto_process(){
	if(usb_rx_index >= 6){
		if(usb_rx_buffer[0] != 0xAA || usb_rx_buffer[1] != 0xBB){
			usb_rx_index = 0;
		}else{
			uint8_t type = usb_rx_buffer[2];
			uint16_t len = usb_rx_buffer[3] | (usb_rx_buffer[4] << 8);
			uint16_t frame_size = 2 + 1 + 2 + len + 1;

			if(usb_rx_index >= frame_size){
				uint8_t checksum = usb_rx_buffer[frame_size - 1];

				if(checksum != proto_checksum(type, len, &usb_rx_buffer[5])){
					Send_NACK();
					usb_rx_index = 0;
				}else if(type == PROTO_MSG_SET_PARAMS && len == 8){
					uint16_t psc = proto_read_u16_le(&usb_rx_buffer[5]);
					uint16_t arr = proto_read_u16_le(&usb_rx_buffer[7]);
					uint32_t freq = proto_read_u32_le(&usb_rx_buffer[9]);
					Send_ACK();
					AD9833_SetFreq(freq);
					SetTimer1PscArr(psc, arr);
					HAL_Delay(10);
					StartMeasurement();
				}
			}
			usb_rx_index = 0;
		}
	}
}

void proto_send_results(float vin_rms, float vout_rms){
	uint8_t payload[8];
	uint8_t tx[32];
	proto_write_f32_le(payload, vin_rms);
	proto_write_f32_le(&payload[4], vout_rms);

	uint16_t len = proto_build_frame(PROTO_MSG_GET_PARAMS, payload, 8, tx);
	CDC_Transmit_FS(tx, len);
}

static void SetTimer1PscArr(uint16_t psc, uint16_t arr){
	__HAL_TIM_DISABLE(&htim1);
	__HAL_TIM_SET_PRESCALER(&htim1, psc);
	__HAL_TIM_SET_AUTORELOAD(&htim1, arr);
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	htim1.Instance->EGR = TIM_EGR_UG;

	__HAL_TIM_DISABLE(&htim8);
	__HAL_TIM_SET_PRESCALER(&htim8, psc);
	__HAL_TIM_SET_AUTORELOAD(&htim8, arr);
	__HAL_TIM_SET_COUNTER(&htim8, 0);
	htim8.Instance->EGR = TIM_EGR_UG;

}
static void StartMeasurement(){
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adcBuffer, ADC_BUFFER_SIZE);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

static void Send_ACK(){
	uint8_t tx[16];
	uint16_t len = proto_build_frame(PROTO_MSG_ACK, NULL, 0, tx);
	CDC_Transmit_FS(tx, len);
}

static void Send_NACK(){
	uint8_t tx[16];
	uint16_t len = proto_build_frame(PROTO_MSG_NACK, NULL, 0, tx);
	CDC_Transmit_FS(tx, len);
}
