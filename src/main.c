////////////////////////////////////////////////////////
// ALL WHITE
/////////////////////////////////////////////////////
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pio.h"
// #include "matrix.pio.h"

// #define DATA_BASE_PIN 1 
// #define CLK_PIN 11
// #define LAT_PIN 12
// #define OE_PIN 13
// #define ADDR_BASE_PIN 7 

// void setup_pio(PIO pio, uint sm, uint offset) {
//     pio_sm_config c = matrix_pio_program_get_default_config(offset);
    
//     sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
//     sm_config_set_sideset_pins(&c, CLK_PIN);

//     for(int i=0; i<6; i++) pio_gpio_init(pio, DATA_BASE_PIN + i);
//     pio_gpio_init(pio, CLK_PIN);

//     pio_sm_set_consecutive_pindirs(pio, sm, DATA_BASE_PIN, 6, true);
//     pio_sm_set_consecutive_pindirs(pio, sm, CLK_PIN, 1, true);

//     // SHIFT RIGHT: This is the standard for Pico PIO.
//     // Autopull at 30 bits (5 pixels * 6 bits). 
//     // This uses bits 0-29 of your 32-bit word.
//     sm_config_set_out_shift(&c, true, true, 30); 

//     // Slow clock for high-capacitance matrix cables
//     sm_config_set_clkdiv(&c, 50.0f); 

//     pio_sm_init(pio, sm, offset, &c);
//     pio_sm_set_enabled(pio, sm, true);
// }

// int main() {
//     stdio_init_all();

//     for(int i=7; i<=13; i++) {
//         gpio_init(i);
//         gpio_set_dir(i, GPIO_OUT);
//     }

//     PIO pio = pio0;
//     uint offset = pio_add_program(pio, &matrix_pio_program);
//     uint sm = pio_claim_unused_sm(pio, true);
//     setup_pio(pio, sm, offset);

//     while (true) {
//         for (int row = 0; row < 16; row++) {
//             // 1. Prepare for data
//             gpio_put(OE_PIN, 1); 

//             // 2. Set Row Address
//             gpio_put_masked(0xF << ADDR_BASE_PIN, row << ADDR_BASE_PIN);

//             // 3. Fill the row
//             // We need 64 pixels. 64 pixels / 5 per word = 12.8 words.
//             // We will send 13 words to ensure the row is full.
//             // 0x3FFFFFFF = Binary 0011 1111... (30 ones)
//             for (int i = 0; i < 13; i++) {
//                 pio_sm_put_blocking(pio, sm, 0x3FFFFFFF);
//             }

//             // 4. THE CRITICAL SYNC: 
//             // Wait for the FIFO to empty AND the OSR (Output Shift Register) to empty
//             while (!pio_sm_is_tx_fifo_empty(pio, sm));
            
//             // This is the "Reset Fix": Clear anything stuck in the shifter
//             // by forcing a state machine restart if it hangs
//             busy_wait_us(10); 

//             // 5. Latch and Show
//             gpio_put(LAT_PIN, 1);
//             busy_wait_us(2);
//             gpio_put(LAT_PIN, 0);
            
//             gpio_put(OE_PIN, 0);
//             busy_wait_us(400); 
//         }
//     }
// }

// /////////////////////////////////////////////////
// ALTERNATE RED / GREEN LEDS
////////////////////////////////////////////////
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pio.h"
// #include "matrix.pio.h"

// // --- Configuration ---
// #define DATA_BASE_PIN 1  // R1, G1, B1, R2, G2, B2 (Pins 1-6)
// #define ADDR_BASE_PIN 7  // A, B, C, D (Pins 7-10)
// #define CLK_PIN 11
// #define LAT_PIN 12
// #define OE_PIN 13

// void setup_pio(PIO pio, uint sm, uint offset) {
//     pio_sm_config c = matrix_pio_program_get_default_config(offset);
    
//     // Map the 'out' pins for data and 'side-set' for the clock
//     sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
//     sm_config_set_sideset_pins(&c, CLK_PIN);

//     // Initialize the GPIOs for PIO use
//     for(int i=0; i<6; i++) pio_gpio_init(pio, DATA_BASE_PIN + i);
//     pio_gpio_init(pio, CLK_PIN);

//     pio_sm_set_consecutive_pindirs(pio, sm, DATA_BASE_PIN, 6, true);
//     pio_sm_set_consecutive_pindirs(pio, sm, CLK_PIN, 1, true);

//     // Shift Right, Autopull at 24 bits (6 bits * 4 pixels = 24 bits)
//     sm_config_set_out_shift(&c, true, true, 24); 

//     // Clock divider: 20.0f is a good balance of speed and stability
//     sm_config_set_clkdiv(&c, 20.0f); 

//     pio_sm_init(pio, sm, offset, &c);
//     pio_sm_set_enabled(pio, sm, true);
// }

// int main() {
//     stdio_init_all();

//     // Initialize Address and Control pins as standard GPIO
//     for(int i=7; i<=13; i++) {
//         gpio_init(i);
//         gpio_set_dir(i, GPIO_OUT);
//     }

//     // PIO Setup
//     PIO pio = pio0;
//     uint offset = pio_add_program(pio, &matrix_pio_program);
//     uint sm = pio_claim_unused_sm(pio, true);
//     setup_pio(pio, sm, offset);

//     while (true) {
//         for (int row = 0; row < 16; row++) {
//             // 1. Disable display while shifting new data
//             gpio_put(OE_PIN, 1); 

//             // 2. Set Row Address (A, B, C, D)
//             gpio_put_masked(0xF << ADDR_BASE_PIN, row << ADDR_BASE_PIN);

//             // 3. Define the colors for this specific row
//             uint32_t p_bits;
//             if (row % 2 == 0) {
//                 // Even Rows: Red (R1 is bit 0, R2 is bit 3)
//                 p_bits = 0b001001; // 0x09
//             } else {
//                 // Odd Rows: Green (G1 is bit 1, G2 is bit 4)
//                 p_bits = 0b010010; // 0x12
//             }

//             // Pack 4 pixels into one 32-bit word (4 * 6 bits = 24 bits used)
//             uint32_t packed_word = (p_bits << 0) | (p_bits << 6) | (p_bits << 12) | (p_bits << 18);

//             // 4. Send 64 pixels worth of data
//             // 64 pixels / 4 pixels per word = 16 words
//             for (int i = 0; i < 16; i++) {
//                 pio_sm_put_blocking(pio, sm, packed_word);
//             }

//             // 5. Wait for the PIO hardware to finish the physical shift
//             while (!pio_sm_is_tx_fifo_empty(pio, sm));
//             busy_wait_us(2); // Safety buffer for the final clock pulse

//             // 6. Latch the data into the row's shift registers
//             gpio_put(LAT_PIN, 1);
//             busy_wait_us(1); 
//             gpio_put(LAT_PIN, 0);
            
//             // 7. Re-enable display and wait (Glow time)
//             gpio_put(OE_PIN, 0);
//             busy_wait_us(500); 
//         }
//     }
// }


///////////////////////////////////////////////////
// TRANSITION red -> green -> blue
// ////////////////////////////////////////////////
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pio.h"
// #include "matrix.pio.h"

// #define DATA_BASE_PIN 1  
// #define ADDR_BASE_PIN 7  
// #define CLK_PIN 11
// #define LAT_PIN 12
// #define OE_PIN 13

// void setup_pio(PIO pio, uint sm, uint offset) {
//     pio_sm_config c = matrix_pio_program_get_default_config(offset);
//     sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
//     sm_config_set_sideset_pins(&c, CLK_PIN);

//     for(int i=0; i<6; i++) pio_gpio_init(pio, DATA_BASE_PIN + i);
//     pio_gpio_init(pio, CLK_PIN);

//     pio_sm_set_consecutive_pindirs(pio, sm, DATA_BASE_PIN, 6, true);
//     pio_sm_set_consecutive_pindirs(pio, sm, CLK_PIN, 1, true);

//     // TRY SHIFT LEFT (false) instead of RIGHT (true)
//     // Sometimes the way the FIFO handles the 32-bit word requires a Left shift
//     sm_config_set_out_shift(&c, false, true, 6); 

//     sm_config_set_clkdiv(&c, 50.0f); // Even slower for testing
//     pio_sm_init(pio, sm, offset, &c);
//     pio_sm_set_enabled(pio, sm, true);
// }

// int main() {
//     stdio_init_all();
//     for(int i=7; i<=13; i++) {
//         gpio_init(i);
//         gpio_set_dir(i, GPIO_OUT);
//     }

//     PIO pio = pio0;
//     uint offset = pio_add_program(pio, &matrix_pio_program);
//     uint sm = pio_claim_unused_sm(pio, true);
//     setup_pio(pio, sm, offset);

//     uint32_t test_colors[] = {
//         0b001001, // Should be RED (Pins 1 & 4)
//         0b010010, // Should be GREEN (Pins 2 & 5)
//         0b100100  // Should be BLUE (Pins 3 & 6)
//     };
//     int color_index = 0;
//     uint32_t last_switch = to_ms_since_boot(get_absolute_time());

//     while (true) {
//         // Switch color every 1000ms
//         if (to_ms_since_boot(get_absolute_time()) - last_switch > 1000) {
//             color_index = (color_index + 1) % 3;
//             last_switch = to_ms_since_boot(get_absolute_time());
//         }

//         for (int row = 0; row < 16; row++) {
//             gpio_put(OE_PIN, 1); 
//             gpio_put_masked(0xF << ADDR_BASE_PIN, row << ADDR_BASE_PIN);

//             for (int col = 0; col < 64; col++) {
//                 // We shift the color to the MSB (top of the 32-bit word) 
//                 // because we are using SHIFT LEFT in the config above.
//                 pio_sm_put_blocking(pio, sm, test_colors[color_index] << 26);
//             }

//             while (!pio_sm_is_tx_fifo_empty(pio, sm));
//             busy_wait_us(2); 
//             gpio_put(LAT_PIN, 1);
//             busy_wait_us(1); 
//             gpio_put(LAT_PIN, 0);
//             gpio_put(OE_PIN, 0);
//             busy_wait_us(400); 
//         }
//     }
// }

///////////////////////////////////////////////////
// RED & GREEN STRIPES
////////////////////////////////////////////////////
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pio.h"
// #include "matrix.pio.h"

// #define DATA_BASE_PIN 1 
// #define ADDR_BASE_PIN 7 
// #define CLK_PIN 11
// #define LAT_PIN 12
// #define OE_PIN 13

// void setup_pio(PIO pio, uint sm, uint offset) {
//     pio_sm_config c = matrix_pio_program_get_default_config(offset);
//     sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
//     sm_config_set_sideset_pins(&c, CLK_PIN);

//     for(int i=0; i<6; i++) pio_gpio_init(pio, DATA_BASE_PIN + i);
//     pio_gpio_init(pio, CLK_PIN);

//     pio_sm_set_consecutive_pindirs(pio, sm, DATA_BASE_PIN, 6, true);
//     pio_sm_set_consecutive_pindirs(pio, sm, CLK_PIN, 1, true);

//     // Using your working 24-bit pull threshold
//     sm_config_set_out_shift(&c, true, true, 24); 
    
//     // Slowing down to 30.0f to ensure the Red signals have time to rise
//     sm_config_set_clkdiv(&c, 30.0f); 

//     pio_sm_init(pio, sm, offset, &c);
//     pio_sm_set_enabled(pio, sm, true);
// }

// int main() {
//     stdio_init_all();

//     for(int i=7; i<=13; i++) {
//         gpio_init(i);
//         gpio_set_dir(i, GPIO_OUT);
//         gpio_set_drive_strength(i, GPIO_DRIVE_STRENGTH_12MA);
//     }

//     PIO pio = pio0;
//     uint offset = pio_add_program(pio, &matrix_pio_program);
//     uint sm = pio_claim_unused_sm(pio, true);
//     setup_pio(pio, sm, offset);

//     // Explicitly define the 6-bit masks
//     const uint32_t RED   = 0b001001; // R1 and R2
//     const uint32_t GREEN = 0b010010; // G1 and G2

//     while (true) {
//         for (int row = 0; row < 16; row++) {
//             // 1. OE High (Display Off)
//             gpio_put(OE_PIN, 1); 

//             // 2. Set Address
//             gpio_put_masked(0xF << ADDR_BASE_PIN, row << ADDR_BASE_PIN);

//             // 3. Build the alternating 24-bit word
//             // This creates: [Green][Red][Green][Red] (4 pixels total)
//             uint32_t stripe_word = 0;
//             stripe_word |= (RED   << 0);
//             stripe_word |= (GREEN << 6);
//             stripe_word |= (RED   << 12);
//             stripe_word |= (GREEN << 18);
//             // Ensure bits 24-31 are strictly 0
//             stripe_word &= 0x00FFFFFF;

//             // 4. Push data (16 words * 4 pixels/word = 64 pixels)
//             for (int i = 0; i < 16; i++) {
//                 pio_sm_put_blocking(pio, sm, stripe_word);
//             }

//             // 5. Hardware Sync
//             while (!pio_sm_is_tx_fifo_empty(pio, sm));
//             busy_wait_us(5); // Increased wait for the slow RP2350 -> Panel link

//             // 6. Latch
//             gpio_put(LAT_PIN, 1);
//             busy_wait_us(2); 
//             gpio_put(LAT_PIN, 0);
            
//             // 7. OE Low (Display On)
//             gpio_put(OE_PIN, 0);
//             busy_wait_us(400); 
//         }
//     }
// }


//////////////////////////////////////////////////////////
// WHITE CIRCLE
// // // /////////////////////////////////////////////////////////
// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/pio.h"
// #include "hardware/gpio.h"
// #include "matrix.pio.h"

// //Pin mapping
// #define DATA_BASE_PIN 1 
// #define ADDR_BASE_PIN 7 
// #define CLK_PIN 11
// #define LAT_PIN 12
// #define OE_PIN 13

// #define WHITE 0b111111 
// #define BLACK 0b000000

// void setup_pio(PIO pio, uint sm, uint offset) {
//     //lead default configuration for led matrix
//     pio_sm_config c = matrix_pio_program_get_default_config(offset);
//     //define 6 data pins as out for RGB data
//     sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
//     //define clk find as the side-set pins to toggle automattically
//     sm_config_set_sideset_pins(&c, CLK_PIN);
    
//     //initlaize gpio for pio use
//     for (int i = 0; i < 6; i++) 
//     {
//         pio_gpio_init(pio, DATA_BASE_PIN + i);
//     }
//     pio_gpio_init(pio, CLK_PIN);
    
//     //set direction of pin to output in statemachine
//     pio_sm_set_consecutive_pindirs(pio, sm, DATA_BASE_PIN, 6, true);
//     pio_sm_set_consecutive_pindirs(pio, sm, CLK_PIN, 1, true);
    
//     //shift out 24 bits at a time, shift right
//     // 24-bit packing (4 pixels * 6 bits)
//     sm_config_set_out_shift(&c, true, true, 24); 
//     //set clk divider
//     sm_config_set_clkdiv(&c, 20.0f); 
    
//     //apply configuration and enable state machien
//     pio_sm_init(pio, sm, offset, &c);
//     pio_sm_set_enabled(pio, sm, true);
// }

// uint32_t get_clean_circle(int x, int row) {
//     //center of circles x and y
//     int cx = 32;
//     int cy = 16;
//     //easier calcuation (12^2)
//     int radius_sq = 144;

//     //calculate distance fro top half (rows 0-15)
//     int dx = x - cx;
//     int dy1 = row - cy;
//     int dist1 = (dx * dx) + (dy1 * dy1);
//     //if distance < radius 
//     //set rbg to white or else black
//     uint32_t top = (dist1 < radius_sq) ? 0b111 : 0b000;

//     //calculate distance fro top half (rows 16-31)
//     int dy2 = (row + 16) - cy;
//     int dist2 = (dx * dx) + (dy2 * dy2);
//     //same thing
//     uint32_t bottom = (dist2 < radius_sq) ? 0b111 : 0b000;

//     //combine bottom (upper 3 bits) and top (lower 3 bits)
//     //into 6-bit pixal data
//     return (bottom << 3) | top;
// }



// int main() {
//     stdio_init_all();

//     for(int i = 7; i <= 13; i++) {
//         //set adress pins
//         gpio_init(i);
//         //set as output
//         gpio_set_dir(i, GPIO_OUT);
//         //high current for LEDS
//         gpio_set_drive_strength(i, GPIO_DRIVE_STRENGTH_12MA);
//     }

//     //use first PIO block
//     PIO pio = pio0;
//     //load asm into pio memory
//     uint offset = pio_add_program(pio, &matrix_pio_program);
//     //find an aviable state machine
//     uint sm = pio_claim_unused_sm(pio, true);
//     //run conifguration
//     setup_pio(pio, sm, offset);

//     while (true) {
//         for (int row = 0; row < 16; row++) {
//             //disable display to prevent ghosting while swittching rows
//             gpio_put(OE_PIN, 1); 

//             // Set Address Pins (A, B, C, D)
//             //to select row
//             gpio_put_masked(0xF << ADDR_BASE_PIN, row << ADDR_BASE_PIN);

//             // Shift in 64 pixels (16 words of 4 pixels)
//             for (int i = 0; i < 16; i++) {
//                 uint32_t packed_word = 0;
//                 for (int p = 0; p < 4; p++) {
//                     int col = (i * 4) + p;
//                     //get color for the specific coordinate
//                     uint32_t mask = get_clean_circle(col, row);
//                     //pack 6-bit color into 32-bit word for PIO
//                     packed_word |= (mask << (p * 6));
//                 }
//                 //send 4-pixel workd to pio fifo
//                 pio_sm_put_blocking(pio, sm, packed_word);
//             }

//             //wait for shifting in the bits
//             while (!pio_sm_is_tx_fifo_empty(pio, sm));
            
//             //puslse the latch to move shifted data to led driver
//             gpio_put(LAT_PIN, 1);
//             busy_wait_us(1); 
//             gpio_put(LAT_PIN, 0);
            
//             //enable display and wait so leds hvae time to light up
//             gpio_put(OE_PIN, 0);
//             busy_wait_us(200); 
//         }
//     }
// }


///////////////////////////////////////////////////
// Ball and paddle
//////////////////////////////////////////////

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "matrix.pio.h"

//Pin mapping
#define DATA_BASE_PIN 1 
#define ADDR_BASE_PIN 7 
#define CLK_PIN 11
#define LAT_PIN 12
#define OE_PIN 13

#define WHITE 0b111111 
#define BLACK 0b000000

//static positions
int bx = 16;
int by = 32; // Centered vertically
int radius_sq = 9;

int paddle_x = 8;
int paddle_y = 2; // Near the bottom
int paddle_width = 16;
int paddle_height = 2;

//initaial move varibles for moving line
//i think need in other things
int target_col = 0;
uint32_t count = 0;
uint32_t color = 0b111111;

void setup_pio(PIO pio, uint sm, uint offset) {
    //lead default configuration for led matrix
    pio_sm_config c = matrix_pio_program_get_default_config(offset);
    //define 6 data pins as out for RGB data
    sm_config_set_out_pins(&c, DATA_BASE_PIN, 6);
    //define clk find as the side-set pins to toggle automattically
    sm_config_set_sideset_pins(&c, CLK_PIN);
    
    //initlaize gpio for pio use
    for (int i = 0; i < 6; i++) 
    {
        pio_gpio_init(pio, DATA_BASE_PIN + i);
    }
    pio_gpio_init(pio, CLK_PIN);
    
    //set direction of pin to output in statemachine
    pio_sm_set_consecutive_pindirs(pio, sm, DATA_BASE_PIN, 6, true);
    pio_sm_set_consecutive_pindirs(pio, sm, CLK_PIN, 1, true);
    
    //shift out 24 bits at a time, shift right
    // 24-bit packing (4 pixels * 6 bits)
    sm_config_set_out_shift(&c, true, true, 24); 
    //set clk divider
    sm_config_set_clkdiv(&c, 20.0f); 
    
    //apply configuration and enable state machien
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

uint32_t ball_paddle(int col, int row) {

    //is the pixel greater than the starting point of the paddle
    //and is the pixel less than (paddle start + width)
    //esssentially asking if it is supposed to be on the padle line
    //checks to see if the row is also the 31 row can make bigger is need be
    // int paddle = ((col >= paddle_x) && 
    //             (col < (paddle_x + paddle_width)) 
    //             && ((row + 16) == 29));

    //check top half (1-15)
    bool paddle = ((row >= paddle_y) && 
                (row < (paddle_y + paddle_height)) &&
                ((col >= paddle_x) && 
                (col < (paddle_x + paddle_width)) 
                ));

    //calculate horixonal ditance from current pixel (x) to center of ball
    //do pythagoram theorm (a^2 + b^2 = c^2)
    int a = col - bx;
    //find veritcal disntace to ball center
    int b = (row + 1) - by;

    //circle equation 
    //a^2 + b^2 < radius ^ 2
    //then the pixel is part of ball = true
    bool ball = ((a * a) + (b * b)) < radius_sq;

    //split into two in additional function 
    //because habe to scan for top and bottom already
    //easier ot compelte in a seprate function

    //convert the boolean into 3-bit format
    //if it is a part of the ball then it is white (1) else off
    // uint32_t ball_top = (ball || paddle_top) ? 0b100 : 0b000;
    // uint32_t ball_top = (ball) ? 0b100 : 0b000;
    // uint32_t paddle_top = (paddle1) ? 0b100 : 0b000;
    // uint32_t bits_top = ball_top | paddle_top;
    // //same thing for bottom
    // //the paddle will also show up on the bottom rows though
    // //so or with paddle
    // // uint32_t ball_bot = (ball2 || paddle_bot) ? 0b001 : 0b000;
    // uint32_t ball_bot = (ball2) ? 0b001 : 0b000;
    // uint32_t paddle_bot = (paddle2) ? 0b001 : 0b000;
    // uint32_t bits_bottom = ball_bot | paddle_bot;

    //hub75 expects 6 bits in the order [b2 g2 r2 b1 g1 r1]
    //shift bottom bits into first three them with top bits
    // uint32_t bits_return = (bits_bottom << 3) | bits_top;
    uint32_t bits_ball = ball ? 0b001 : 0b000;
    uint32_t bits_paddle = paddle ? 0b100 : 0b000;

    // uint32_t bits_return = bits_ball | bits_paddle;

    return bits_ball | bits_paddle;
}

//easier to scan rows so dont have to it in additional functions
uint32_t get_data(int phys_col, int phys_row) {
    // We swap row/col here to match the verical orientation
    uint32_t top = ball_paddle(phys_row, phys_col);
    
    // Row 16-31 just add 16 to rows
    uint32_t bottom = ball_paddle(phys_row + 16, phys_col);
    
    return (bottom << 3) | top;
}

// uint32_t get_line(int target_col, uint32_t color, int col)
// {
//     //inital line == 0
//     uint32_t line = 0b0;

//     //if the target_col it hsould be on is the current colum
//     //then set the line to 1
//     if (target_col = col)
//     {
//         //i think shoudl fill
//         //top and bottom rows for that col
//         //dont need to seperate line ball and paddle becuase nothing is cut off??
//         line = (color << 3) | color;
//     }
//     return line;
// }

int main() {
    stdio_init_all();

    for(int i = 7; i <= 13; i++) {
        //set adress pins
        gpio_init(i);
        //set as output
        gpio_set_dir(i, GPIO_OUT);
        //high current for LEDS
        gpio_set_drive_strength(i, GPIO_DRIVE_STRENGTH_12MA);
    }

    //use first PIO block
    PIO pio = pio0;
    //load asm into pio memory
    uint offset = pio_add_program(pio, &matrix_pio_program);
    //find an aviable state machine
    uint sm = pio_claim_unused_sm(pio, true);
    //run conifguration
    setup_pio(pio, sm, offset);

    while (true) {
        // //move the line down 
        // if (count++ % 100 == 0)
        // {
        //     //update the target_col to icnrease by 1
        //     //wraps around every 64 columns
        //     target_col = (target_col + 1) % 64;
        // }
        //ball physics
        bx += 1;
        by += 1;

        // if (by == target_col)
        // {
        //     by += -1;
        // }

        //bound off side of walls rows 0 and 31
        if (bx <= 0 || bx >= 31)
        {
            bx += -1;
        }

        //bound of top 
        if (by >= 63 || by <= 0)
        {
            by += -1;
        }

        //bound off the paddle
        // int paddle_x = 8;
        // int paddle_y = 2; // Near the bottom
        // int paddle_width = 16;
        // int paddle_height = 2;
        //static positions
        // int bx = 16;
        // int by = 32; // Centered vertically
        // int radius_sq = 9;
        
        if ((paddle_y <= by) && 
            (by <= (paddle_y + paddle_height)) && 
            (paddle_x <= bx) && 
            (bx <= (paddle_width + paddle_x)))
        {
            by += 1;
        }

        for (int row = 0; row < 16; row++) {
            //disable display to prevent ghosting while swittching rows
            gpio_put(OE_PIN, 1); 

            // Set Address Pins (A, B, C, D)
            //to select row
            gpio_put_masked(0xF << ADDR_BASE_PIN, row << ADDR_BASE_PIN);

            // Shift in 64 pixels (16 words of 4 pixels)
            for (int i = 0; i < 16; i++) {
                uint32_t packed_word = 0;
                for (int p = 0; p < 4; p++) {
                    int col = (i * 4) + p;
                    //get color for the specific coordinate
                    //only for ball and paddle!!!!
                    uint32_t mask = get_data(col, row);

                    // uint32_t line = get_line(target_col, color, col);

                    //find mask with added line
                    // mask |= line;
                    
                    //ensure mask is only 6 bits
                    //having trouble lining up the ball and paddle
                    mask &= 0x3F;

                    //pack 6-bit color into 32-bit word for PIO
                    packed_word |= (mask << (p * 6));
                }
                //send 4-pixel workd to pio fifo
                pio_sm_put_blocking(pio, sm, packed_word);
            }

            //wait for shifting in the bits
            while (!pio_sm_is_tx_fifo_empty(pio, sm));
            
            //puslse the latch to move shifted data to led driver
            gpio_put(LAT_PIN, 1);
            busy_wait_us(1); 
            gpio_put(LAT_PIN, 0);
            
            //enable display and wait so leds hvae time to light up
            gpio_put(OE_PIN, 0);
            busy_wait_us(200); 
        }
    }
}


