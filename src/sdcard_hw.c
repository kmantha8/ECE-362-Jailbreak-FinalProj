#include "sdcard_hw.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

static void set_mosi_to_spi(void)
{
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
}

static void release_mosi_high(void)
{
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(SD_MOSI_PIN, GPIO_OUT);
    gpio_put(SD_MOSI_PIN, 1);
}

static void init_spi_sdcard(void)
{
    gpio_set_function(SD_SCK_PIN, GPIO_FUNC_SPI);
    set_mosi_to_spi();
    gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
    gpio_pull_up(SD_MISO_PIN);

    gpio_init(SD_CS_PIN);
    gpio_set_dir(SD_CS_PIN, GPIO_OUT);
    gpio_put(SD_CS_PIN, 1);

    spi_init(SD_SPI_INSTANCE, 400 * 1000);
    spi_set_format(SD_SPI_INSTANCE, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

void disable_sdcard(void)
{
    uint8_t idle = 0xff;

    gpio_put(SD_CS_PIN, 1);
    spi_write_blocking(SD_SPI_INSTANCE, &idle, 1);
    release_mosi_high();
}

void enable_sdcard(void)
{
    set_mosi_to_spi();
    gpio_put(SD_CS_PIN, 0);
}

void sdcard_io_high_speed(void)
{
    spi_set_baudrate(SD_SPI_INSTANCE, 12 * 1000 * 1000);
}

void init_sdcard_io(void)
{
    init_spi_sdcard();
    disable_sdcard();
}
