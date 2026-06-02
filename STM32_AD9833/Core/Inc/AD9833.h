#ifndef INC_AD9833_H_
#define INC_AD9833_H_

#include "main.h"

// SPI1 kullandığın için hspi1 diyoruz
extern SPI_HandleTypeDef hspi1;

// FSYNC pini için PA4 seçtiğini varsayıyorum, IOC'de farklıysa burayı değiştir
#define AD9833_PORT  GPIOA
#define AD9833_FSYNC GPIO_PIN_4

// AD9833 Komutları
#define SINE_WAVE     0x2000
#define TRIANGLE_WAVE 0x2002
#define SQUARE_WAVE   0x2028

void AD9833_SetFreq(uint32_t freq);
void AD9833_Init(uint16_t wave_type);

#endif
