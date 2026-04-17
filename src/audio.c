#include <stdio.h>
#include "audio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

extern uint32_t sound_off_time;
extern uint16_t current_volume;

#define SPEAKER_PIN 20
#define POT_ADC_CH 1    
#define POT_GPIO 41     

void init_audio() {
    // FIXED: Removed reset_block_mask which was causing the system to freeze/reset.
    // Replaced with standard robust initialization.
    adc_init();
    adc_gpio_init(POT_GPIO); 
    adc_select_input(POT_ADC_CH); 
    
    // --- Speaker (PWM) Setup ---
    gpio_set_function(SPEAKER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SPEAKER_PIN);
    
    pwm_set_clkdiv(slice_num, 150.0f);
    pwm_set_wrap(slice_num, 2272); 
    pwm_set_enabled(slice_num, true);
    
    // Ensure speaker starts at 0 volume
    pwm_set_gpio_level(SPEAKER_PIN, 0);
}

// // In audio.c — remove the adc_read() call entirely:
// void play_brick_break_sound() {
//     uint32_t min_duty = 200;
//     uint32_t max_duty = 1136;
//     uint32_t current_duty = min_duty + ((current_volume * (max_duty - min_duty)) / 4095);
//     pwm_set_gpio_level(SPEAKER_PIN, current_duty);  
// }

void play_brick_break_sound() {
    // Ensure we are using the external variable from main.c
    uint32_t min_duty = 0;    // Quiet
    uint32_t max_duty = 2272; // Full volume (Match your wrap value)
    
    // Map 0-4095 to 0-2272
    uint32_t current_duty = (uint32_t)current_volume * max_duty / 4095;
    
    pwm_set_gpio_level(SPEAKER_PIN, current_duty);  
}