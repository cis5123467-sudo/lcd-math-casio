#ifndef LCD_SCREEN_H
#define LCD_SCREEN_H
#include <stdint.h>
#include <stdbool.h>
extern uint16_t lcd_ink, lcd_paper, lcd_mid;
#define LCD_INK lcd_ink
#define LCD_PAPER lcd_paper
#define LCD_MID lcd_mid
void lcd_text(int x,int y,const char *text,int scale,uint16_t ink);
void lcd_finish(void);
void lcd_frame(const char *title);
void lcd_calculator(void);
void lcd_menu(const char *title,const char *const *rows,int count,int selected,const char *hint);
void lcd_help(int page);
void lcd_contrast(void);
#endif
