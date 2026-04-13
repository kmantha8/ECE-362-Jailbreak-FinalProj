#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "matrix.pio.h"

// Your pin mapping
#define R1 1; G1 2; B1 3; R2 4; G2 5; B2 6;
#define A 7; B 8; C 9; D 10;
#define CLK 11; LAT 12; OE 13;

void set_addr(int row) {
    gpio_put(7, (row >> 0) & 1);
    gpio_put(8, (row >> 1) & 1);
    gpio_put(9, (row >> 2) & 1);
    gpio_put(10, (row >> 3) & 1);
}

int main() {
    stdio_init_all();
    
    // Initialize Pins 1-13 as Outputs
    for(int i=1; i<=13; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
    }

    while (true) {
        for (int row = 0; row < 16; row++) {
            gpio_put(13, 1); // OE High (Disable display)
            
            set_addr(row);
            
            // Push data for 64 pixels (Example: All White)
            for (int col = 0; col < 64; col++) {
                // Set pins 1-6 HIGH (White)
                gpio_put_masked(0x7E, 0x7E); 
                gpio_put(11, 1); // Clock High
                gpio_put(11, 0); // Clock Low
            }
            
            gpio_put(12, 1); // Latch High
            gpio_put(12, 0); // Latch Low
            gpio_put(13, 0); // OE Low (Enable display)
            
            sleep_ms(1); // Short delay to stabilize row
        }
    }
}
