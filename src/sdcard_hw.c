// #include "sdcard_hw.h"

// #include "hardware/gpio.h"
// #include "pico/stdlib.h"

// static void set_mosi_to_spi(void)
// {
//     gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
// }

// static void release_mosi_high(void)
// {
//     gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SIO);
//     gpio_set_dir(SD_MOSI_PIN, GPIO_OUT);
//     gpio_put(SD_MOSI_PIN, 1);
// }

// static void init_spi_sdcard(void)
// {
//     gpio_set_function(SD_SCK_PIN, GPIO_FUNC_SPI);
//     set_mosi_to_spi();
//     gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
//     gpio_pull_up(SD_MISO_PIN);

//     gpio_init(SD_CS_PIN);
//     gpio_set_dir(SD_CS_PIN, GPIO_OUT);
//     gpio_put(SD_CS_PIN, 1);

//     spi_init(SD_SPI_INSTANCE, 400 * 1000);
//     spi_set_format(SD_SPI_INSTANCE, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
// }

// // void disable_sdcard(void)
// // {
// //     uint8_t idle = 0xff;

// //     gpio_put(SD_CS_PIN, 1);
// //     spi_write_blocking(SD_SPI_INSTANCE, &idle, 1);
// //     release_mosi_high();
// // }

// // void enable_sdcard(void)
// // {
// //     set_mosi_to_spi();
// //     gpio_put(SD_CS_PIN, 0);
// // }

// // void sdcard_io_high_speed(void)
// // {
// //     spi_set_baudrate(SD_SPI_INSTANCE, 12 * 1000 * 1000);
// // }

// // void init_sdcard_io(void)
// // {
// //     init_spi_sdcard();
// //     disable_sdcard();
// // }








// // 1. Setup the actual SPI peripheral and Pins
// void init_sdcard_io() {
//     // SD cards MUST start slow (400kHz) to initialize
//     spi_init(SD_SPI_INSTANCE, 400 * 1000);
    
//     gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
//     gpio_set_function(SD_SCK_PIN, GPIO_FUNC_SPI);
//     gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
    
//     // CS is handled as a regular GPIO
//     gpio_init(SD_CS_PIN);
//     gpio_set_dir(SD_CS_PIN, GPIO_OUT);
//     gpio_put(SD_CS_PIN, 1); // Deselect by default
// }

// // 2. Speed up after the card is ready
// void sdcard_io_high_speed(void) {
//     spi_set_baudrate(SD_SPI_INSTANCE, 12 * 1000 * 1000); // 12MHz
// }

// // 3. Control the CS Pin
// void enable_sdcard(void) {
//     asm volatile("nop \n nop \n nop"); // Tiny delay
//     gpio_put(SD_CS_PIN, 0);
//     asm volatile("nop \n nop \n nop");
// }

// void disable_sdcard(void) {
//     asm volatile("nop \n nop \n nop");
//     gpio_put(SD_CS_PIN, 1);
//     asm volatile("nop \n nop \n nop");
// }


#include "sdcard_hw.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "hardware/resets.h"

void init_sdcard_io() {
    // 1. Reset the SPI1 peripheral to ensure a clean state
    reset_block(RESETS_RESET_SPI1_BITS);
    unreset_block_wait(RESETS_RESET_SPI1_BITS);

    // 2. Initialize SPI1 at 400kHz (Standard SD Init Speed)
    // We call this twice to ensure the peripheral clock is locked
    spi_init(spi1, 400 * 1000);
    
    // 3. FORCE Pin Multiplexing
    // We must ensure the RP2350 switches these pins from SIO (GPIO) to SPI
    gpio_set_function(12, GPIO_FUNC_SPI); // MISO
    gpio_set_function(14, GPIO_FUNC_SPI); // SCK
    gpio_set_function(15, GPIO_FUNC_SPI); // MOSI
    
    // 4. Manual Pull-up on MISO
    // Many SD cards need this to stay out of a floating state
    gpio_pull_up(12);

    // 5. CS is handled as a standard GPIO
    gpio_init(13);
    gpio_set_dir(13, GPIO_OUT);
    gpio_put(13, 1); // Keep high (deselected)

    // 6. Explicitly set the SPI format
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
    printf("SPI1 Hardware Initialized on Pins 12-15\n");
}

// 2. Speed up after the card is ready
void sdcard_io_high_speed(void) {
    spi_set_baudrate(SD_SPI_INSTANCE, 12 * 1000 * 1000); // 12MHz
}

// 3. Control the CS Pin
void enable_sdcard(void) {
    // Switching MOSI back to SPI function in case it was released
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
    asm volatile("nop \n nop \n nop"); 
    gpio_put(SD_CS_PIN, 0); // Select card
    asm volatile("nop \n nop \n nop");
}

void disable_sdcard(void) {
    asm volatile("nop \n nop \n nop");
    gpio_put(SD_CS_PIN, 1); // Deselect card
    asm volatile("nop \n nop \n nop");
    
    // Optional: Send 8 extra clock cycles with CS high to let the SD card finish its business
    uint8_t fill = 0xFF;
    spi_write_blocking(SD_SPI_INSTANCE, &fill, 1);
}