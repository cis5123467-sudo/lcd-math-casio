#ifndef LCD_TEXT_H
#define LCD_TEXT_H
#include <stdint.h>
unsigned text_codepoint(const char **text);
int text_characters(const char *text,int bytes);
const unsigned char *text_symbol(unsigned codepoint);
#endif
