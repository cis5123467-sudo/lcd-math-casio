#include "lcd.h"
#include "calculator.h"
#include "mathview.h"
#include "font.h"
#include "digits.h"
#include <gint/display.h>
#include <stdio.h>
#include <string.h>

uint16_t lcd_ink=0x2185,lcd_paper=0xd6f8,lcd_mid=0x742d;
static uint16_t rgb565(int r,int g,int b) { return ((r>>3)<<11)|((g>>2)<<5)|(b>>3); }
static uint16_t blend(uint16_t a,uint16_t b,int amount) {
    int r=(((a>>11)&31)*(10-amount)+((b>>11)&31)*amount)/10;
    int g=(((a>>5)&63)*(10-amount)+((b>>5)&63)*amount)/10;
    int blue=((a&31)*(10-amount)+(b&31)*amount)/10;
    return (r<<11)|(g<<5)|blue;
}
static uint16_t paper_pixels[256],ink_pixels[256],rule_pixels[256],rule_ink;
static int palette_contrast=-1;
static uint16_t shifted(uint16_t color,int step) {
    int r=((color>>11)&31)+step,g=((color>>5)&63)+step,b=(color&31)+step;
    if(r<0)r=0;
    if(r>31)r=31;
    if(g<0)g=0;
    if(g>63)g=63;
    if(b<0)b=0;
    if(b>31)b=31;
    return (r<<11)|(g<<5)|b;
}
static void palette(void) {
    int c=calc.contrast;
    if(c==palette_contrast)return;
    palette_contrast=c;
    lcd_paper=rgb565(187-c/2,201-c/2,158-c/2);
    lcd_ink=rgb565(126-12*c,139-12*c,94-8*c);
    lcd_mid=blend(lcd_paper,lcd_ink,6);
    rule_ink=blend(lcd_paper,lcd_ink,9);
    // A fixed screen-space pattern models the glass/pixel lattice. No random
    // frame state, missing pixels or changes to font advances are involved.
    for(int y=0;y<16;y++)for(int x=0;x<16;x++) {
        unsigned index=y*16+x;
        uint32_t hash=(index+17u)*2654435761u;
        hash^=hash>>13;hash*=2246822519u;hash^=hash>>16;
        int grain=(hash&7)==0?-1:(hash&7)==1?1:0;
        int dot=((x&1)||(y&1))?1:0;
        if((hash&31)==3)dot++;
        paper_pixels[index]=shifted(lcd_paper,grain);
        ink_pixels[index]=shifted(lcd_ink,dot);
        rule_pixels[index]=shifted(rule_ink,dot);
    }
}
void lcd_finish(void) {
    // Texture only LCD shades. Bezel colors and all geometric bounds remain
    // untouched; shade lookup keeps redraw cost independent of expression size.
    for(int y=0;y<DHEIGHT;y++)for(int x=0;x<DWIDTH;x++) {
        uint16_t *pixel=&gint_vram[y*DWIDTH+x];
        unsigned index=(y&15)*16+(x&15);
        if(*pixel==LCD_PAPER)*pixel=paper_pixels[index];
        else if(*pixel==LCD_INK)*pixel=ink_pixels[index];
        else if(*pixel==rule_ink)*pixel=rule_pixels[index];
    }
}
static int clip_left=0,clip_top=0,clip_right=395,clip_bottom=223;
static void rectangle(int x,int y,int w,int h,uint16_t ink) {
    int r=x+w-1,b=y+h-1;
    if(x<clip_left) x=clip_left;
    if(y<clip_top) y=clip_top;
    if(r>clip_right) r=clip_right;
    if(b>clip_bottom) b=clip_bottom;
    if(x<=r && y<=b) drect(x,y,r,b,ink);
}
void lcd_text(int x,int y,const char *text,int scale,uint16_t ink) {
    for(;*text;text++,x+=6*scale) {
        unsigned ch=(unsigned char)*text;
        if(ch==28) {
            rectangle(x,y+3*scale,5*scale,scale,ink);
            rectangle(x+2*scale,y+scale,scale,scale,ink);
            rectangle(x+2*scale,y+5*scale,scale,scale,ink);
        } else if(ch==MathMultiply) {
            for(int k=0;k<5;k++) { rectangle(x+k*scale,y+(k+1)*scale,scale,scale,ink); rectangle(x+(4-k)*scale,y+(k+1)*scale,scale,scale,ink); }
        } else if(ch==MathPi) {
            rectangle(x,y+scale,5*scale,scale,ink);
            rectangle(x+scale,y+2*scale,scale,4*scale,ink);
            rectangle(x+3*scale,y+2*scale,scale,4*scale,ink);
            rectangle(x,y+6*scale,scale,scale,ink);
            rectangle(x+4*scale,y+6*scale,scale,scale,ink);
        } else if(ch==MathApprox) {
            for(int r=0;r<2;r++) for(int k=0;k<5;k++) rectangle(x+k*scale,y+(2+r*3+(k==0||k==4))*scale,scale,scale,ink);
        } else {
            if(ch>127) ch='?';
            for(int row=0;row<7;row++) for(int col=0;col<5;col++)
                if(lcd_font[ch][row]&(1<<(4-col))) rectangle(x+col*scale,y+row*scale,scale,scale,ink);
        }
    }
}
int casio_width(const char *text,int length,bool small,bool bold) {
    (void)bold;
    int n=length<0?(int)strlen(text):length;
    return n?(n*(small?12:18)-(small?2:3)):0;
}
static void math_text(int x,int y,const char *text,uint16_t ink,uint16_t paper,bool small) {
    (void)paper;
    if(small) { lcd_text(x,y,text,2,ink); return; }
    // Taller CW-like numerals retain the existing 18-pixel formula advance.
    for(;*text;text++,x+=18) {
        if(*text>='0' && *text<='9') {
            for(int row=0;row<11;row++) for(int col=0;col<7;col++)
                if(lcd_digits[*text-'0'][row]&(1<<(6-col)))
                    rectangle(x+col*2,y+row*2,2,2,ink);
        } else if(*text=='+' || *text=='-' || *text=='=' || *text==28 || *text==MathMultiply) {
            if(*text=='+' || *text=='-' || *text==28) rectangle(x+1,y+10,13,2,ink);
            if(*text=='+') rectangle(x+6,y+4,2,14,ink);
            if(*text==28) {rectangle(x+6,y+4,2,2,ink);rectangle(x+6,y+16,2,2,ink);}
            if(*text=='=') {rectangle(x+1,y+7,13,2,ink);rectangle(x+1,y+14,13,2,ink);}
            if(*text==MathMultiply) for(int i=0;i<11;i++) {rectangle(x+2+i,y+5+i,2,2,ink);rectangle(x+12-i,y+5+i,2,2,ink);}
        } else { char glyph[2]={*text,0}; lcd_text(x,y,glyph,3,ink); }
    }
}
static void noop_text(int x,int y,const char *t,uint16_t i,uint16_t p,bool s) {(void)x;(void)y;(void)t;(void)i;(void)p;(void)s;}
static void noop_rect(int x,int y,int w,int h,uint16_t i) {(void)x;(void)y;(void)w;(void)h;(void)i;}
static void math_rectangle(int x,int y,int w,int h,uint16_t ink) {
    rectangle(x,y,w,h,ink==LCD_INK?blend(LCD_PAPER,LCD_INK,9):ink);
}
static MathCanvas canvas={math_text,math_rectangle,0,0};
static const MathCanvas measure_canvas={noop_text,noop_rect,0,0};
void lcd_frame(const char *title) {
    palette(); canvas.ink=LCD_INK; canvas.paper=LCD_PAPER;
    clip_left=0; clip_top=0; clip_right=395; clip_bottom=223;
    if(!title) {
        dclear(LCD_PAPER);
        drect_border(0,0,395,223,LCD_PAPER,2,blend(LCD_PAPER,LCD_INK,5));
        return;
    }
    int bottom=202;
    dclear(0x8c90);
    drect(3,3,392,bottom,0x4a69);
    drect(5,5,390,bottom-2,0xb5b5);
    drect(7,7,388,bottom-4,LCD_PAPER);
    // A shallow inner rim keeps the screen recognisably LCD, without a fake keypad.
    dline(7,7,388,7,0x9d31); dline(7,7,7,bottom-4,0x9d31);
    if(title) drect(0,205,395,223,0xc69a);
    if(title) lcd_text(16,17,title,1,LCD_INK);
}
void lcd_calculator(void) {
    lcd_frame(NULL);
    const char *angles[]={"D","R","G"};
    lcd_text(16,17,calc.shift?"S":" ",1,LCD_INK);
    lcd_text(30,17,calc.alpha?"A":" ",1,LCD_INK);
    // Natural-input symbol and boxed angle indicator, like the CW status row.
    dline(47,21,49,24,LCD_INK); dline(49,24,52,15,LCD_INK);
    dline(52,15,62,15,LCD_INK); dline(54,19,61,19,LCD_INK);
    dline(56,17,60,17,LCD_INK); dline(56,22,60,22,LCD_INK);
    drect_border(80,14,89,26,LCD_PAPER,1,LCD_INK);
    lcd_text(82,17,angles[calc.angle],1,LCD_INK);
    lcd_text(101,17,calc.memory_set?"M":" ",1,LCD_INK);
    if(calc.format) lcd_text(121,17,calc.format==1?"FIX":"SCI",1,LCD_INK);
    if(calc.count) for(int row=0;row<5;row++) dline(369-row,16+row,369+row,16+row,LCD_INK);
    clip_left=16; clip_right=379; clip_top=32; clip_bottom=114;
    MathBox box=measure_math(calc.input,4096);
    int mid=54;
    if(mid-box.above<clip_top) mid=clip_top+box.above;
    if(mid+box.below>clip_bottom) mid=clip_bottom-box.below;
    MathCaret caret=paint_math_input(calc.input,4096,16,mid,&measure_canvas,calc.cursor,false);
    if(caret.y<clip_top) mid+=clip_top-caret.y;
    if(caret.y+caret.height>clip_bottom) mid-=caret.y+caret.height-clip_bottom;
    int offset=caret.x>369?caret.x-369:0;
    caret=paint_math_input(calc.input,4096,16-offset,mid,&canvas,calc.cursor,false);
    if(!calc.result && !calc.error) rectangle(caret.x,caret.y,2,caret.height,LCD_INK);
    clip_top=122; clip_bottom=208;
    char answer[RESULT_CAP]; calculator_answer(answer);
    if(calc.error) {
        lcd_text(22,132,calc.message,1,LCD_INK);

    } else {
        box=measure_math(answer,4096);
        int x=379-box.width;
        if(x<16) x=16;
        int center=179;
        if(center+box.below>208) center=208-box.below;
        paint_math(answer,4096,x,center,&canvas);

    }
    clip_left=0;clip_right=395;clip_top=0;clip_bottom=223;

}
void lcd_menu(const char *title,const char *const *rows,int count,int selected,const char *hint) {
    lcd_frame(title);
    dline(15,31,380,31,0xadb4);
    int start=selected/5*5;
    for(int i=start;i<count && i<start+5;i++) {
        int y=43+(i-start)*29;
        if(i==selected) { drect(14,y-4,381,y+20,LCD_INK); lcd_text(22,y,rows[i],2,LCD_PAPER); }
        else lcd_text(22,y,rows[i],2,LCD_INK);
    }
    char position[20]; snprintf(position,sizeof(position),"%d/%d",selected+1,count);
    lcd_text(338,18,position,1,LCD_INK);
    lcd_text(10,212,hint?hint:"UP/DOWN select   EXE choose   BACK return",1,LCD_INK);
}
void lcd_help(int page) {
    static const char *pages[][8]={
        {"CALCULATING", "EXE / OK evaluates", "Arrows move the cursor", "UP / DOWN recalls history", "DEL deletes; BACK clears", "FORMAT changes exact/decimal", "HOME saves and exits", "LEFT/RIGHT changes help page"},
        {"SCIENTIFIC KEYS", "SHIFT + SIN/COS/TAN: inverse", "SHIFT + LN: exp", "SHIFT + LOG: 10 to a power", "SHIFT + square: square root", "SHIFT + dot: factorial", "SHIFT + EXP: pi", "SHIFT + (-): e"},
        {"EDITING & MEMORY", "Fraction: numerator/denominator", "SHIFT + left/right: start/end", "SHIFT + DEL: clear expression", "ANS inserts last answer", "STO opens memory menu", "ALPHA + 1..9 recalls memory", "SHIFT + FORMAT: angle unit"},
        {"CATALOG & SETUP", "CATALOG lists all functions", "Arguments separated by comma", "Close brackets auto-complete", "TOOLS opens display settings", "DEG / RAD / GRAD angle modes", "NORM / FIX / SCI number modes", "Contrast adjusts the LCD shade"}
    };
    lcd_frame("LCD MATH / HELP");
    for(int i=0;i<8;i++) lcd_text(18,39+i*19,pages[page][i],1,LCD_INK);
    lcd_text(10,212,"LEFT/RIGHT page   BACK return",1,LCD_INK);
}

void lcd_contrast(void) {
    lcd_frame("SYSTEM SETTINGS / CONTRAST");
    for(int i=0;i<4;i++) drect(70+i*64,43,116+i*64,65,blend(LCD_PAPER,LCD_INK,i*10/3));
    lcd_text(71,74,"4 LCD SHADES",1,LCD_INK);
    const char *sample="(1+sqrt(5))/2";
    MathBox box=measure_math(sample,4096);
    paint_math(sample,4096,(396-box.width)/2,112,&canvas);
    dline(36,161,360,161,LCD_INK);
    for(int i=0;i<=10;i++) dline(36+i*32,157,36+i*32,166,LCD_INK);
    drect(32+calc.contrast*32,153,40+calc.contrast*32,170,LCD_INK);
    lcd_text(24,181,"LIGHT",1,LCD_INK);lcd_text(336,181,"DARK",1,LCD_INK);
    char level[16];snprintf(level,sizeof(level),"%d / 10",calc.contrast);
    lcd_text(177,181,level,1,LCD_INK);
    lcd_text(10,212,"LEFT lighter   RIGHT darker   EXE done",1,LCD_INK);
}
