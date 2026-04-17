#ifndef AUDIO_H
#define AUDIO_H

// Include standard types so the header knows what uint16_t, etc., are
#include "pico/stdlib.h"

extern uint16_t current_volume;

// Declare the functions you want to be visible to main.c
void init_audio();
void play_brick_break_sound();

#endif