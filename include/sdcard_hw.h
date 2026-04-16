#ifndef SDCARD_HW_H
#define SDCARD_HW_H

#include "hardware/spi.h"

// Update these pins if your SD wiring differs.
// The default mapping assumes SPI0 on GP16-19.
#define SD_SPI_INSTANCE spi1
#define SD_MISO_PIN 28
#define SD_CS_PIN 29
#define SD_SCK_PIN 30
#define SD_MOSI_PIN 31

void init_sdcard_io(void);
void sdcard_io_high_speed(void);
void enable_sdcard(void);
void disable_sdcard(void);

#endif
