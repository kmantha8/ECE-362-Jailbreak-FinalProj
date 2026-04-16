#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HIGH_SCORE_COUNT 5

bool highscore_init(void);
bool highscore_is_ready(void);
bool highscore_submit(uint32_t score);
uint32_t highscore_get_best(void);
size_t highscore_count(void);
uint32_t highscore_get_score(size_t index);

#endif
