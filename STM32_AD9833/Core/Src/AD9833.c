#include "AD9833.h"

void AD9833_Write(uint16_t data) {
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&data, 1, HAL_MAX_DELAY);
}

void AD9833_SetFreq(uint32_t freq) {
    // 25MHz MCLK için frekans hesaplama
    uint32_t freq_val = (uint32_t)((double)freq * 268435456.0 / 25000000.0);

    uint16_t LSB = (uint16_t)(freq_val & 0x3FFF) | 0x4000;
    uint16_t MSB = (uint16_t)((freq_val >> 14) & 0x3FFF) | 0x4000;
    AD9833_Write(LSB);
    AD9833_Write(MSB);

}

void AD9833_Init(uint16_t wave_type) {
    AD9833_Write(0x2100); // Kontrol register (Reset aktif)
    AD9833_SetFreq(2000); // Varsayılan 1kHz
    AD9833_Write(0xC000); // Phase 0
    AD9833_Write(wave_type); // Dalga formunu seç ve Resetten çıkar
}
