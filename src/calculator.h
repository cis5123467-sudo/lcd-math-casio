#ifndef LCD_CALCULATOR_H
#define LCD_CALCULATOR_H
#include "cas.h"
#include <stdint.h>
#define INPUT_CAP 112
#define HISTORY_CAP 20
#define RESULT_CAP 96
typedef struct { char expression[INPUT_CAP], exact[64]; double value; } Entry;
typedef struct {
    char input[INPUT_CAP];
    int cursor, count, browse, angle, format, precision, contrast;
    bool decimal, result, error, shift, alpha;
    char message[48];
    double ans, memory[9];
    uint16_t memory_set;
    Entry history[HISTORY_CAP];
} Calculator;
extern Calculator calc;
void calculator_init(void);
void calculator_clear(void);
void calculator_insert(const char *text);
void calculator_delete(void);
void calculator_type(const char *text);
void calculator_fraction(void);
bool calculator_move(int direction);
void calculator_history(int direction);
bool calculator_evaluate(void);
void calculator_answer(char output[RESULT_CAP]);
void calculator_store(int index);
void calculator_recall(int index);
bool calculator_save(void);
void calculator_load(void);
#endif
