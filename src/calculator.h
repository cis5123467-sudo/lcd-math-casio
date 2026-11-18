#ifndef LCD_CALCULATOR_H
#define LCD_CALCULATOR_H
#include "cas.h"
#include <stdint.h>
#define INPUT_CAP 112
#define HISTORY_CAP 20
#define RESULT_CAP 96
#define MEMORY_COUNT 28
typedef struct { char expression[INPUT_CAP], exact[64]; double value; } Entry;
typedef struct {
    char input[INPUT_CAP];
    int cursor, count, browse, angle, format, precision, contrast;
    bool decimal, result, error, shift, alpha, alpha_lock;
    bool approximate_result,mixed_result,sexagesimal,engineering,insert_capture;
    int engineering_steps,selection_start,selection_end;
    char message[48];
    double ans, memory[MEMORY_COUNT];
    uint32_t memory_set;
    Entry history[HISTORY_CAP];
} Calculator;
extern Calculator calc;
void calculator_init(void);
void calculator_clear(void);
void calculator_insert(const char *text);
void calculator_delete(void);
void calculator_type(const char *text);
void calculator_fraction(void);
void calculator_mixed_fraction(void);
void calculator_indexed(const char *function);
const char *calculator_memory_name(int index);
bool calculator_move(int direction);
void calculator_history(int direction);
bool calculator_evaluate(void);
void calculator_answer(char output[RESULT_CAP]);
void calculator_store(int index);
void calculator_recall(int index);
bool calculator_save(void);
void calculator_load(void);
#endif
