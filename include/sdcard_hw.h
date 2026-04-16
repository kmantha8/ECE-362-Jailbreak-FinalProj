#ifndef SDCARD_HW_H
#define SDCARD_HW_H

#include "hardware/spi.h"

// Update these pins if your SD wiring differs.
// The default mapping assumes SPI0 on GP16-19.
#define SD_SPI_INSTANCE spi1
#define SD_MISO_PIN 12
#define SD_CS_PIN 13
#define SD_SCK_PIN 14
#define SD_MOSI_PIN 15

void init_sdcard_io(void);
void sdcard_io_high_speed(void);
void enable_sdcard(void);
void disable_sdcard(void);

#endif
