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

#define MAX_BOX_ROWS 10

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ff.h"
#include "diskio.h"
#include <stdio.h>
#include <string.h>

/*******************************************************************/

#define SD_MISO 19
#define SD_CS 17
#define SD_SCK 18
#define SD_MOSI 16
#define SD_SPI_INSTANCE spi0

/*******************************************************************/

void init_spi_sdcard() {
    // 1. Configure SCK, MOSI, MISO as SPI pins
    gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SD_MISO, GPIO_FUNC_SPI);

    // 2. Configure CS as a regular GPIO pin controlled by SIO
    gpio_init(SD_CS);
    gpio_set_dir(SD_CS, GPIO_OUT);
    gpio_put(SD_CS, 1); // Set CS high (inactive initially)

    // 3. Configure the SPI peripheral (SPI1)
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
    gpio_set_dir(SD_MOSI, GPIO_OUT);
    gpio_put(SD_MOSI, 1);
}

void enable_sdcard() {
    // Take control of MOSI again by making it an SPI pin
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);

    // Set CS low (active)
    gpio_put(SD_CS, 0);
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

int box_y_pos[MAX_BOX_ROWS] = {63, 66, 69, 72, 75, 78, 81, 84, 87, 90};
bool box1_on[MAX_BOX_ROWS] = {true, true, true, true, true, true, true, true, true, true};
bool box2_on[MAX_BOX_ROWS] = {true, true, true, true, true, true, true, true, true, true};

//static positions
float bx = 16.0f;
float by = 32.0f; // Centered vertically'
float ball_dx = 0.1f; //horixonal speed
float ball_dy = 0.15f; //verifucal speed
int radius_sq = 4;

int paddle_x = 8;
int paddle_y = 2; // Near the bottom
int paddle_width = 16;
int paddle_height = 2;

//initaial move varibles for moving line
//i think need in other things
int target_col = 63;
uint32_t count = 0;
uint32_t color = 0b111;
uint32_t score = 0;

bool lose_end = false;
// bool box1_exists = true;
// bool box2_exists = true;
bool btn_21_prev = false;
bool btn_26_prev = false;


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

    //having trouble with glitchy edges
    //convert all to int
    int int_bx = (int)bx;
    int int_by = (int)by;

    //calculate horixonal ditance from current pixel (x) to center of ball
    //do pythagoram theorm (a^2 + b^2 = c^2)
    int a = col - int_bx;
    //find veritcal disntace to ball center
    int b = (row + 1) - int_by;

    //circle equation 
    //a^2 + b^2 < radius ^ 2
    //then the pixel is part of ball = true
    bool ball = (((a * a) + (b * b))) < radius_sq;

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

uint32_t get_boxes(int target_col, uint32_t color, int col, int row, bool box1_exists, bool box2_exists)
{
    // uint32_t boxes = 0b0;

    //if the target_col it hsould be on is the current colum
    //then set the line to 1
    if (target_col == col)
    {
        //box sizes
        bool box1 = ((row >= 1) && (row <= 15));
        bool box2 = (((row) >= 17) && ((row) <= 30));

        //if the col and row is a  box
        if (box1 && box1_exists)
        {
            //turn on that box color for both top and bottom row
            return color;
        }
        else if (box2 && box2_exists)
        {
            return color;
        }
    }
    return 0;
}

uint32_t get_score(int col, int row)
{
    //score is the width
    //1 is the height
    //row should only happen on row 0 or 1
    //start is col 0
    //start is row 0

    uint32_t score_y = 0;
    uint32_t score_x = 0;
    uint32_t score_height = 2;

    bool score_bool = ((row >= score_y) && 
                (row < (score_y + score_height)) &&
                ((col >= score_x) && 
                (col < (score_x + (int)score)) 
                ));
    
    uint32_t bits_score = score_bool ? 0b111 : 0b000;

    // uint32_t bits_return = bits_ball | bits_paddle;

    return bits_score;
}

//easier to scan rows so dont have to it in additional functions
uint32_t get_data(int phys_col, int phys_row) {
    
    // We swap row/col here to match the verical orientation
    uint32_t top = ball_paddle(phys_row, phys_col);
    
    // Row 16-31 just add 16 to rows
    uint32_t bottom = ball_paddle(phys_row + 16, phys_col);

    //do the same thing for boxes
    // uint32_t top_box = get_boxes(target_col, color, phys_col, phys_row);
    // uint32_t bottom_box = get_boxes(target_col, color, phys_col, phys_row + 16);

    for (int i = 0; i < MAX_BOX_ROWS; i ++)
    {
        top |= get_boxes(box_y_pos[i], color, phys_col, phys_row, box1_on[i], box2_on[i]);
        bottom |= get_boxes(box_y_pos[i], color, phys_col, phys_row + 16, box1_on[i], box2_on[i]);
    }

    top |= get_score(phys_col, phys_row);
    bottom |= get_score(phys_col, phys_row + 16);
    
    // uint32_t top = top_ball | top_box;
    // uint32_t bottom = bottom_ball | bottom_box;

    return (bottom << 3) | top;
}

//taken from prevous lab
void init_inputs() {
    // fill in
    //configure gp21 and gp26 as inputs
    //no need to set the value
    uint32_t mask = (1u << 21) | (1u << 26);
    sio_hw->gpio_oe_clr = mask;

    // have to set he function for them to work 
    // HAVE TO ASK ABOUT IN LAB
    // THAT IS THE ONE THING THAT IS PROBS WRONG
    hw_write_masked(&pads_bank0_hw->io[21],
                   PADS_BANK0_GPIO0_IE_BITS,
                   PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
    );
    io_bank0_hw->io[21].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[21], PADS_BANK0_GPIO0_ISO_BITS);
    
    hw_write_masked(&pads_bank0_hw->io[26],
                   PADS_BANK0_GPIO0_IE_BITS,
                   PADS_BANK0_GPIO0_IE_BITS | PADS_BANK0_GPIO0_OD_BITS
    );
    io_bank0_hw->io[26].ctrl = GPIO_FUNC_SIO << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
    hw_clear_bits(&pads_bank0_hw->io[26], PADS_BANK0_GPIO0_ISO_BITS);

    // gpio_init(21);
    // gpio_init(26);

    // gpio_set_dir(21, GPIO_IN);
    // gpio_set_dir(26, GPIO_IN);
    // Enable rising edge detection
    // gpio_set_irq_enabled(21, GPIO_IRQ_EDGE_RISE, true);
    // gpio_set_irq_enabled(26, GPIO_IRQ_EDGE_RISE, true);
}

// uint32_t get_line(int target_col, uint32_t color, int col)
// {
//     //inital line == 0
//     uint32_t line = 0b0;

//     //if the target_col it hsould be on is the current colum
//     //then set the line to 1
//     if (target_col == col)
//     {
//         //i think shoudl fill
//         //top and bottom rows for that col
//         //dont need to seperate line ball and paddle becuase nothing is cut off??
//         line = (color << 3) | color;
//     }
//     return line;
// }

const uint32_t game_message[5] = {
    // 0b01110011100100101111, // Row 1
    // 0b0111_0_01110_0_1000001_1111
    0b01110011100100000101111,
    // 0b10000100010110111100, // Row 2
    // 0b1000_0_10001_0_1100011_1000
    0b10000100010110001101000,
    // 0b10110111110010101111,
    // 0b1011_0_11111_0_1010101_1111 // Row 3
    0b10110111110101010101111,
    // 0b10010100010001010000,
    // 0b1001_0_10001_0_1001001_1000 // Row 4
    0b10010100010100100101000,
    // 0b01110100010001011111  
    // 0b0111_0_10001_0_1001001_1111// Row 5
    0b01110100010100100101111
};

const uint32_t over_message[5] = {
    0b01110010001011110111100, // O  V  E  R
    0b10001010001010000100010,
    0b10001001010011110111100,
    0b10001001010010000100010,
    0b01110000100011110100010
};

uint32_t game_over(int col, int row)
{
    //rows 0 - 16
    // uint32_t shape = 0;
    // if (col == row || (col == row * 2))
    // {
    //     shape = (color << 3) | color;
    // }
    // else 
    // {
    //     shape = 0;
    // }
    // return shape;

    uint32_t top = 0;
    uint32_t bottom = 0;


    int text_start_col = 19;
    int text_start_row = 5;

    //game displayed on top half
    //(check to see if the row is in between the specied start and end row)
    if (row >= text_start_row && row < text_start_row + 5) {
        //start the index at the column it is at minus the start
        //the bit_index start where the col is
        //ex: our start is 15 so if 15-15 bit index is 0
        int bit_index = (col - text_start_col);
        //then the bit_index has to be between 0-19
        //because that is how long the messahe is
        if (bit_index >= 0 && bit_index < 23) {
            // Check the specific bit in the bitmap
            //since the goes to game_message and picsk the row
            //then picsk the index in that row for that bit
            //read from left to right
            //EX: 1 << (19 - 0) shift the data over 19 to read that bit
            if (game_message[row - text_start_row] & (1 << (22 - bit_index))) {
                top = 0b111; // Red
            }
            // else 
            // {
            //     top = 0b001;
            // }

            if (over_message[row - text_start_row] & (1 << (22 - bit_index))) {
                bottom = 0b111; // red
            }
            // else 
            // {
            //     bottom = 0b001;
            // }
        }
    }

    return (bottom << 3) | top;
}

void init_uart();
void init_uart_irq();
void date(int argc, char *argv[]);
void command_shell();


int main() {
    stdio_init_all();

    //call button function
    init_inputs();

    init_uart();
    init_uart_irq();
    
    init_sdcard_io();

    // SD card functions will initialize everything.
    command_shell();

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

        //BUTTON LOGIC
        //want to have it so one button press
        //moves it one pixel
        //need to track prevous state

        bool btn_21_cur = (sio_hw->gpio_in & (1u << 21));
        bool btn_26_cur = (sio_hw->gpio_in & (1u << 26));

        //again just taken from lab1
        //adjusted to change color for testing
        // if (sio_hw->gpio_in & (1u << 21))
        // if (gpio_get_irq_event_mask(21) & GPIO_IRQ_EDGE_RISE)
        if (btn_21_cur && !btn_21_prev)
        {
            //acknowledge IRQ
            // gpio_acknowledge_irq(21, GPIO_IRQ_EDGE_RISE);
            //change the color to red
            color = 0b001;
            //boundaries so the paddle doesnt disappear
            if (paddle_x > 0)
            {
                paddle_x--;
            }
            // paddle_x--;
        }

        // if (sio_hw->gpio_in & (1u << 26))
        // if (gpio_get_irq_event_mask(26) & GPIO_IRQ_EDGE_RISE)
        if (btn_26_cur && !btn_26_prev) 
        {
            //acknowledge IRQ
            // gpio_acknowledge_irq(26, GPIO_IRQ_EDGE_RISE);
            //change the color to red
            color = 0b010;
            // paddle_x++;
            //boundaries so the paddle doesnt disappear
            if (paddle_x < (32 - paddle_width))
            {
                paddle_x++;
            }
        }

        btn_21_prev = btn_21_cur;
        btn_26_prev = btn_26_cur;

        // //move the line down 
        if (count++ % 200 == 0)
        {
            //update the target_col to icnrease by 1
            //wraps around every 64 columns
            // target_col = (target_col - 1);

            // if ((target_col == 0)
            //     && (box1_exists || box2_exists)
            //     )
            // {
            //     // target_col = 63;
            //     lose_end = true;
            // }

            //do the same thing but for all box rows
            for (int i = 0; i < MAX_BOX_ROWS; i++)
            {
                box_y_pos[i]--;

                //if boxes hit the paddle
                if (box_y_pos[i] <= 2)
                {
                    if (box1_on[i] || box2_on[i])
                    {
                        lose_end = true;
                        // box_y_pos[i] = 63;
                    }
                    else
                    {
                        //box should "respawn at the top"
                        box_y_pos[i] = 63; 
                        box1_on[i] = true;
                        box2_on[i] = true;
                    }
                }
            }
        }
        //ball physics
        bx += ball_dx;
        by += ball_dy;

        int int_bx = (int)bx;
        int int_by = (int)by;

        // if (((int_by - 2) <= (target_col))
        //     && (int_by >= target_col)
        //     && (int_bx <= 15)
        //     && (int_bx >= 1)
        //     && box1_exists
        //     )
        // {
        //     ball_dy *= -1;
        //     by -= 0.1f;
        //     box1_exists = false;
        // }

        // if (((int_by - 2) <= (target_col))
        //     && (int_by >= target_col)
        //     && ((int_bx) <= 30)
        //     && ((int_bx - 2) >= 17)
        //     && box2_exists
        //     )
        // {
        //     ball_dy *= -1;
        //     by -= 0.1f;
        //     box2_exists = false;
        // }


        //change the endges because is bx and by were
        //exactly inbetween the boxes it was going thorugh it
        for (int i = 0; i < MAX_BOX_ROWS; i++)
        {
            if (((int_by - 2) <= (box_y_pos[i]))
                && ((int_by + 2) >= box_y_pos[i])
                && ((int_bx - 1) <= 16)
                && (int_bx >= 0)
                && box1_on[i]
                )
            {
                ball_dy *= -1;
                by -= 0.1f;
                box1_on[i] = false;
                score++;
            }

            if (((int_by - 2) <= (box_y_pos[i]))
                && (int_by + 2 >= box_y_pos[i])
                && ((int_bx) <= 31)
                && ((int_bx + 1) >= 17)
                && box2_on[i]
                )
            {
                ball_dy *= -1;
                by -= 0.1f;
                box2_on[i] = false;
                score++;
            }
        }

        //bound off side of walls rows 0 and 31
        //take into account radius of 2
        if (int_bx <= 2 || int_bx >= 29)
        {
            //revserve the speed of the ball so
            //that it continus on in that direction
            ball_dx *= -1;
        }

        //bound of top 
        if (int_by >= 61)
        {
            ball_dy *= -1;
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
        
        if (((paddle_y - 2) <= int_by) && 
            (int_by <= (paddle_y + paddle_height + 2)) && 
            ((paddle_x - 2) <= int_bx) && 
            (int_bx <= (paddle_width + paddle_x + 2)))
        {
            ball_dy *= -1;
            //ball gets stuck rolling on paddle
            //have to move it off
            by += 0.2f;
        }

        if ((int_by - 2) < (paddle_y))
        {
            // bx = 16.0f;
            // by = 32.0f; // Centered vertically'
            // ball_dx = -0.1f; //horixonal speed
            // ball_dy = 0.15f; //verifucal speed
            lose_end = true;
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
                    uint32_t mask = 0;
                    // uint32_t line = get_line(target_col, color, col);
                    // uint32_t boxes = get_boxes(target_col, color, col, row);

                    //find mask with added line
                    // mask |= line;
                    // mask |= boxes;

                    //if the ball has either went off screen
                    //or if the line has gotten to the end 
                    //the game will be lost
                    if (lose_end)
                    {
                        mask |= game_over(col, row);
                    }
                    else
                    {
                        mask |= get_data(col, row);
                    }
                    
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
            //having trouble waiting
            //it is glithcing out a lot
            //adding an additional wait to give it time to process
            busy_wait_us(2); 
            gpio_put(LAT_PIN, 1);
            busy_wait_us(1);
            gpio_put(LAT_PIN, 0);
            
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



