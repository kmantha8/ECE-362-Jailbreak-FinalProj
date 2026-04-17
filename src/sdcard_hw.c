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


// #include "sdcard_hw.h"
// #include "hardware/gpio.h"
// #include "pico/stdlib.h"
// #include "hardware/spi.h"

// // 1. Setup the actual SPI peripheral and Pins
// void init_sdcard_io() {
//     // 1. Force SPI1 at a slow speed
//     spi_init(spi0, 400 * 1000);
    
//     // 2. Force the pins to SPI1 functionality
//     // On RP2350, Pins 12-15 are specifically SPI1
//     gpio_set_function(16, GPIO_FUNC_SPI); // MISO
//     gpio_set_function(18, GPIO_FUNC_SPI); // SCK
//     gpio_set_function(19, GPIO_FUNC_SPI); // MOSI
    
//     // 3. Chip Select as standard GPIO
//     gpio_init(17);
//     gpio_set_dir(17, GPIO_OUT);
//     gpio_put(17, 1);

//     // 4. Set format
//     spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
// }

// // 2. Speed up after the card is ready
// void sdcard_io_high_speed(void) {
//     spi_set_baudrate(SD_SPI_INSTANCE, 12 * 1000 * 1000); // 12MHz
// }

// // 3. Control the CS Pin
// void enable_sdcard(void) {
//     // Switching MOSI back to SPI function in case it was released
//     gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
//     asm volatile("nop \n nop \n nop"); 
//     gpio_put(SD_CS_PIN, 0); // Select card
//     asm volatile("nop \n nop \n nop");
// }

// void disable_sdcard(void) {
//     asm volatile("nop \n nop \n nop");
//     gpio_put(SD_CS_PIN, 1); // Deselect card
//     asm volatile("nop \n nop \n nop");
    
//     // Optional: Send 8 extra clock cycles with CS high to let the SD card finish its business
//     uint8_t fill = 0xFF;
//     spi_write_blocking(SD_SPI_INSTANCE, &fill, 1);
// }

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "matrix.pio.h"

#define SD_MISO 16
#define SD_CS 17
#define SD_SCK 18
#define SD_MOSI 19
#define SD_SPI_INSTANCE spi0

void init_spi_sdcard() {
    //Configure SCK, MOSI, MISO as SPI pins
    gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_MISO, GPIO_FUNC_SPI);

    //Configure CS as a regular GPIO pin controlled by SIO
    gpio_init(SD_CS);
    gpio_set_dir(SD_CS, GPIO_OUT);
    gpio_put(SD_CS, 1); // Set CS high (inactive initially)

    // Configure the SPI peripheral (SPI1)
    // 400 KHz baudrate, 8-bit, CPOL=0, CPHA=0
    spi_init(SD_SPI_INSTANCE, 400 * 1000);
    spi_set_format(SD_SPI_INSTANCE, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void disable_sdcard() {
    // Set CS high (inactive)
    gpio_put(SD_CS, 1);

    // Send 0xFF over SPI to give the SD card clock cycles to finish
    uint8_t dummy = 0xFF;
    spi_write_blocking(SD_SPI_INSTANCE, &dummy, 1);

    // Release MOSI: Make it a GPIO and force it high
    gpio_init(SD_MOSI);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SIO);
    gpio_set_dir(SD_MOSI, GPIO_OUT);
    gpio_put(SD_MOSI, 1);
}

void enable_sdcard() {
    // Take control of MOSI again by making it an SPI pin
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);

    // Set CS low (active)
    gpio_put(SD_CS, 0);
    //try swithcing around
}

void sdcard_io_high_speed() {
    // Change baudrate to 12 MHz for faster data transfer after init
    spi_set_baudrate(SD_SPI_INSTANCE, 12 * 1000 * 1000);
}

void init_sdcard_io() {
    // Fully initialize pins and put card in inactive state
    init_spi_sdcard();
    disable_sdcard();
}