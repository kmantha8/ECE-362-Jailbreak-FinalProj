#include "hardware/uart.h" 
#include "hardware/gpio.h" 
#include <stdio.h>  
#include <string.h>
#include "pico/stdlib.h"
#include "sdcard.h"

#define BUFSIZE 100
char serbuf[BUFSIZE];
int seridx = 0;
int newline_seen = 0;

// add this here so that compiler does not complain about implicit function
// in init_uart_irq
void uart_rx_handler();

/*******************************************************************/

void init_uart() {
    // fill in.\// fill in
    //conifgure the uart0 peripheral 
    //to communicate 115200 bps 
    //8 data bits, no partity, one stop
    
    //set the gpio pin muz to the uart
    //pin 0 is TX, 1 is RX
    //cal this before uart_init to avoid loosing data
    gpio_set_function(0, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(uart0, 1));

    //initlaiat uart 0
    uart_init(uart0, 115200);

    //set the uart data format
    //the first is uart, data_bits, stop, and partity
    //might have to tset partity to constant though
    uart_set_format(uart0, 8, 1, 0);
}

void init_uart_irq() {
    // fill in.
    // fill in.

    //configure the same uart did in step two
    //diable the interal FIFO
    //enable nexxessary bits to mask the recieve interuppt
    //set up the uart_rx_handler to be exclcued handle
    //enable the uart interuppt

    // Field       : UART_UARTLCR_H_FEN
    // Description : Enable FIFOs: 0 = FIFOs are disabled (character mode) that is,
    //               the FIFOs become 1-byte-deep holding registers 1 = transmit and
    //               receive FIFO buffers are enabled (FIFO mode).
    //set he fifo to 0 (FEN is FIFO enable)
    //write to line control registser
    //as seen in datasheet 977
    uart_get_hw(uart0)->lcr_h &= ~UART_UARTLCR_H_FEN_BITS;

    //enablet eh revcieve interuppt
    // Field       : UART_UARTIMSC_RXIM
    // Description : Receive interrupt mask. A read returns the current mask for the
    //               UARTRXINTR interrupt. On a write of 1, the mask of the
    //               UARTRXINTR interrupt is set. A write of 0 clears the mask.
    //sett eh mask of the recieve interuppt using the UARTIMSC reguister
    //see datasheet 979
    uart_get_hw(uart0)->imsc |= UART_UARTIMSC_RXIM_BITS;
    
    //set uart_rx_handler as exlcucse
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);

    //eanbelt eh interuppt
    irq_set_enabled(UART0_IRQ, true);
}

void uart_rx_handler() {
    // fill in.
    // fill in.

    //acknowledge the interuupt by set bit in register
    //seridx reaches BUFSIZE --> return
    //read the character from UART data register into c
    //if chacter is newline (0x0A) set newline_seen to 1
    //if not else if the chacter is backspace + characters revieveed
    //write backspace-sapce-backspace
    //derement seridz is >0 
    //set serbuf[seridx to ]\0 and return
    //if char is not backspace print out uart
    //write to serbuf[seridx] 
    //increment seridx

    //acknowldge the unterupt
    //// Field       : UART_UARTICR_RXIC
    // De scription : Receive interrupt clear. Clears the UARTRXINTR interrupt.
    //icr is the interupt clear register
    //RXIC is reviceb inteupt clear
    uart_get_hw(uart0)->icr = UART_UARTICR_RXIC_BITS;

    //if seridx reaches the bufsize
    if (seridx >= BUFSIZE)
    {
        //then return
        return;
    }

    //read the character into c via the data register
    //UARTDR
    char c = uart_get_hw(uart0)->dr;

    //if character is newline
    if (c == 0x0A)
    {
        //set new line to 1
        newline_seen = 1;
    }

    //if new character is backspace
    //and chacters have been reviced (seridx is > 0)
    if ((c == 8) && (seridx > 0))
    {
        //write backspace
        uart_putc(uart0, 8);
        //space
        uart_putc(uart0, 32);
        //backspace
        uart_putc(uart0, 8);

        //deincrement seridx
        //already chacked >0
        seridx--;
        //set this as shown in instructions
        serbuf[seridx] = '\0';

        return;
    }

    //otherwise print character
    uart_putc(uart0, c);

    //write to serbuf[seridex]
    serbuf[seridx] = c;

    //increment seridx
    seridx++;
}

int _read(__unused int handle, char *buffer, int length) {
    while (newline_seen == 0) {
        sleep_ms(5);
    }
    newline_seen = 0;

    // Copy the smaller of the two: what we have vs what was requested
    int bytes_to_copy = (seridx < length) ? seridx : length;

    for (int i = 0; i < bytes_to_copy; i++) {
        buffer[i] = serbuf[i];
    }
    
    int actual_read = bytes_to_copy;
    seridx = 0; // Reset for next command

    return actual_read; // Return how many bytes were actually put into the buffer
}

// int _read(__unused int handle, char *buffer, int length) {
//     // fill in.
//     // fill in.

//     //until newline is seen sleep 5 milliseconds
//     //once seen, newline set to 0
//     //copy contents of serbuf into buffer # of characters to copy is seridx
//     //reset seridx to 0
//     //return lenght not seridx

//     //if the new_line isn't seen or 0
//     while (newline_seen == 0)
//     {
//         //seep 5ms
//         sleep_ms(5);
//     }

//     //once it has been seen (exits loop)
//     //set to 0
//     newline_seen = 0;

//     //jave to compy the contest of serbuf into buffer
//     //the # of chaacters to copy is seridx
//     //would just use as reg seridx in loop but have to test
//     int seridx_length = seridx;

//     //if length >= seridx then copu_length = serdix
//     //if length < seridx only copy length
//     if (length > seridx)
//     {
//         seridx_length = length;
//     }

//     //copy chacters from buffer
//     for (int i = 0; i < seridx_length; i++)
//     {
//         //copy the contents of serbuf into buffer
//         buffer[i] = serbuf[i];
//     }
    
//     //reset seridx
//     seridx = 0;

//     //return lenght
//     return length;
// }

int _write(__unused int handle, char *buffer, int length) {
    // fill in.
    // fill in.

    //from step 3 no comments
    for (int i = 0; i <length; i++)
    {
        uart_putc(uart0, buffer[i]);
    }
    return length;
}

/*******************************************************************/

struct commands_t {
    const char *cmd;
    void      (*fn)(int argc, char *argv[]);
};

struct commands_t cmds[] = {
        { "append", append },
        { "cat", cat },
        { "cd", cd },
        { "date", date },
        { "input", input },
        { "ls", ls },
        { "mkdir", mkdir },
        { "mount", mount },
        { "pwd", pwd },
        { "rm", rm },
        { "restart", restart }
};

// This function inserts a string into the input buffer and echoes it to the UART
// but whatever is "typed" by this function can be edited by the user.
void insert_echo_string(const char* str) {
    // Print the string and copy it into serbuf, allowing editing
    seridx = 0;
    newline_seen = 0;
    memset(serbuf, 0, BUFSIZE);

    // Print and fill serbuf with the initial string
    for (int i = 0; str[i] != '\0' && seridx < BUFSIZE - 1; i++) {
        char c = str[i];
        uart_write_blocking(uart0, (uint8_t*)&c, 1);
        serbuf[seridx++] = c;
    }
}

void parse_command(const char* input) {
    char *token = strtok(input, " ");
    int argc = 0;
    char *argv[10];
    while (token != NULL && argc < 10) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    
    int i = 0;
    for(; i<sizeof cmds/sizeof cmds[0]; i++) {
        if (strcmp(cmds[i].cmd, argv[0]) == 0) {
            cmds[i].fn(argc, argv);
            break;
        }
    }
    if (i == (sizeof cmds/sizeof cmds[0])) {
        printf("Unknown command: %s\n", argv[0]);
    }
}

void command_shell() {
    char input[100];
    memset(input, 0, sizeof(input));

    // Disable buffering for stdout
    setbuf(stdout, NULL);

    printf("\nEnter current ");
    insert_echo_string("date 20250701120000");
    fgets(input, 99, stdin);
    input[strcspn(input, "\r")] = 0; // Remove CR character
    input[strcspn(input, "\n")] = 0; // Remove newline character
    parse_command(input);
    
    printf("SD Card Command Shell");
    printf("\r\nType 'mount' to mount the SD card.\n");
    while (1) {
        printf("\r\n> ");
        fgets(input, sizeof(input), stdin);
        fflush(stdin);
        input[strcspn(input, "\r")] = 0; // Remove CR character
        input[strcspn(input, "\n")] = 0; // Remove newline character
        
        parse_command(input);
    }
}