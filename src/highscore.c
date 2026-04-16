#include "highscore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

#define HIGH_SCORE_FILE "SCORES.TXT"

static FATFS highscore_fs;
static bool highscore_ready = false;
static uint32_t highscore_table[HIGH_SCORE_COUNT];
static size_t highscore_table_count = 0;

static bool insert_score(uint32_t table[], size_t *table_count, uint32_t score)
{
    size_t insert_at = 0;

    while (insert_at < *table_count && table[insert_at] >= score) {
        insert_at++;
    }

    if (*table_count == HIGH_SCORE_COUNT && insert_at == HIGH_SCORE_COUNT) {
        return false;
    }

    size_t last = *table_count;
    if (last >= HIGH_SCORE_COUNT) {
        last = HIGH_SCORE_COUNT - 1;
    }

    for (size_t i = last; i > insert_at; i--) {
        table[i] = table[i - 1];
    }

    table[insert_at] = score;
    if (*table_count < HIGH_SCORE_COUNT) {
        (*table_count)++;
    }
    return true;
}

static bool persist_scores(const uint32_t table[], size_t table_count)
{
    FIL file;
    FRESULT fr = f_open(&file, HIGH_SCORE_FILE, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        return false;
    }

    for (size_t i = 0; i < table_count; i++) {
        char line[16];
        int len = snprintf(line, sizeof(line), "%lu\n", (unsigned long)table[i]);
        UINT written = 0;

        if (len < 0 || len >= (int)sizeof(line)) {
            f_close(&file);
            return false;
        }

        fr = f_write(&file, line, (UINT)len, &written);
        if (fr != FR_OK || written != (UINT)len) {
            f_close(&file);
            return false;
        }
    }

    fr = f_sync(&file);
    f_close(&file);
    return fr == FR_OK;
}

static void print_scores(void)
{
    if (highscore_table_count == 0) {
        printf("High scores: none\r\n");
        return;
    }

    printf("High scores:");
    for (size_t i = 0; i < highscore_table_count; i++) {
        printf(" %lu", (unsigned long)highscore_table[i]);
    }
    printf("\r\n");
}

static bool load_scores(void)
{
    FIL file;
    FRESULT fr = f_open(&file, HIGH_SCORE_FILE, FA_READ);
    if (fr == FR_NO_FILE) {
        return persist_scores(highscore_table, highscore_table_count);
    }
    if (fr != FR_OK) {
        return false;
    }

    highscore_table_count = 0;
    memset(highscore_table, 0, sizeof(highscore_table));

    char line[32];
    while (f_gets(line, sizeof(line), &file) != NULL) {
        char *end = NULL;
        unsigned long parsed = strtoul(line, &end, 10);

        if (end == line) {
            continue;
        }
        insert_score(highscore_table, &highscore_table_count, (uint32_t)parsed);
    }

    f_close(&file);
    return true;
}

bool highscore_init(void)
{
    memset(highscore_table, 0, sizeof(highscore_table));
    highscore_table_count = 0;
    highscore_ready = false;

    FRESULT fr = f_mount(&highscore_fs, "", 1);
    if (fr != FR_OK) {
        printf("SD mount failed (%d)\r\n", fr);
        return false;
    }

    if (!load_scores()) {
        printf("High score load failed\r\n");
        return false;
    }

    highscore_ready = true;
    print_scores();
    return true;
}

bool highscore_is_ready(void)
{
    return highscore_ready;
}

bool highscore_submit(uint32_t score)
{
    if (!highscore_ready) {
        return false;
    }

    uint32_t updated_table[HIGH_SCORE_COUNT];
    size_t updated_count = highscore_table_count;

    memcpy(updated_table, highscore_table, sizeof(updated_table));

    if (!insert_score(updated_table, &updated_count, score)) {
        return true;
    }

    if (!persist_scores(updated_table, updated_count)) {
        return false;
    }

    memcpy(highscore_table, updated_table, sizeof(highscore_table));
    highscore_table_count = updated_count;
    return true;
}

uint32_t highscore_get_best(void)
{
    if (highscore_table_count == 0) {
        return 0;
    }
    return highscore_table[0];
}

size_t highscore_count(void)
{
    return highscore_table_count;
}

uint32_t highscore_get_score(size_t index)
{
    if (index >= highscore_table_count) {
        return 0;
    }
    return highscore_table[index];
}
