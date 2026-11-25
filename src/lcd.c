#include "lcd.h"
#include "calculator.h"
#include "mathview.h"
#include "font.h"
#include "digits.h"
#include "text.h"
#include "keyboard.h"
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
static uint16_t paper_pixels[256],ink_pixels[256],rule_pixels[256],status_pixels[256],ghost_pixels[256];
static uint16_t rule_ink,status_paper,ghost_ink;
static bool calculation_glass;
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
    lcd_paper=rgb565(166-c,178-c,119-c);
    lcd_ink=rgb565(124-9*c,137-9*c,91-6*c);
    lcd_mid=blend(lcd_paper,lcd_ink,6);
    rule_ink=blend(lcd_paper,lcd_ink,9);
    status_paper=shifted(lcd_paper,-1);
    ghost_ink=shifted(lcd_paper,-2);
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
        status_pixels[index]=shifted(status_paper,grain);
        ghost_pixels[index]=shifted(ghost_ink,grain);
        ink_pixels[index]=shifted(lcd_ink,dot);
        rule_pixels[index]=shifted(rule_ink,dot);
    }
}
void lcd_finish(void) {
    for(int y=0;y<DHEIGHT;y++) {
        // A fixed diagonal lighting slope across the glass. Ordered dithering
        // blends a single RGB565 step without wide bands or animated noise.
        int vertical_shade=(DHEIGHT-1-y)*32/(DHEIGHT-1);
        int rim_y=y< DHEIGHT-1-y ? y : DHEIGHT-1-y;
        for(int x=0;x<DWIDTH;x++) {
            uint16_t *pixel=&gint_vram[y*DWIDTH+x];
            unsigned index=(y&15)*16+(x&15);
            if(*pixel==LCD_INK) {*pixel=ink_pixels[index];continue;}
            if(*pixel==rule_ink) {*pixel=rule_pixels[index];continue;}
            if(*pixel==status_paper) *pixel=status_pixels[index];
            else if(*pixel==ghost_ink) *pixel=ghost_pixels[index];
            else if(*pixel==LCD_PAPER) *pixel=paper_pixels[index];
            else continue;
            if(calculation_glass) {
                unsigned phase=(index*73u+19u)&255;
                int shade=vertical_shade+x*96/(DWIDTH-1);
                int rim_x=x<DWIDTH-1-x ? x : DWIDTH-1-x;
                int rim=rim_x<rim_y ? rim_x : rim_y;
                int shadow=rim<10 ? (10-rim)*24 : 0;
                if(phase<(unsigned)(shade+shadow))
                    *pixel=shifted(*pixel,-1);
            }
        }
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
    int units=0;
    for(;*text;x+=6*scale) {
        unsigned ch=text_codepoint(&text);
        if(ch==24)ch=units++==0?0x00b0:units==2?0x2032:0x2033;
        const unsigned char *symbol=text_symbol(ch);
        if(symbol) {
            for(int row=0;row<7;row++)for(int col=0;col<5;col++)
                if(symbol[row]&(1<<(4-col)))rectangle(x+col*scale,y+row*scale,scale,scale,ink);
            continue;
        }
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
    int n=text_characters(text,length);
    return n?(n*(small?12:18)-(small?2:3)):0;
}
static void math_text(int x,int y,const char *text,uint16_t ink,uint16_t paper,bool small) {
    (void)paper;
    if(small) { lcd_text(x,y,text,2,ink); return; }
    // Taller CW-like numerals retain the existing 18-pixel formula advance.
    int units=0;
    for(;*text;x+=18) {
        if((unsigned char)*text==24){const char *unit=units++==0?"°":units==2?"′":"″";lcd_text(x,y,unit,3,ink);text++;continue;}
        if((unsigned char)*text>=128 || ((unsigned char)*text>=20 && (unsigned char)*text<=25)) {
            const char *start=text;text_codepoint(&text);char glyph[5]={0};memcpy(glyph,start,(size_t)(text-start));lcd_text(x,y,glyph,3,ink);continue;
        }
        unsigned ch=(unsigned char)*text++;
        if(ch>='0' && ch<='9') {
            for(int row=0;row<11;row++) for(int col=0;col<7;col++)
                if(lcd_digits[ch-'0'][row]&(1<<(6-col)))
                    rectangle(x+col*2,y+row*2,2,2,ink);
        } else if(ch=='+' || ch=='-' || ch=='=' || ch==28 || ch==MathMultiply) {
            if(ch=='+' || ch=='-' || ch==28) rectangle(x+1,y+10,13,2,ink);
            if(ch=='+') rectangle(x+6,y+4,2,14,ink);
            if(ch==28) {rectangle(x+6,y+4,2,2,ink);rectangle(x+6,y+16,2,2,ink);}
            if(ch=='=') {rectangle(x+1,y+7,13,2,ink);rectangle(x+1,y+14,13,2,ink);}
            if(ch==MathMultiply) for(int i=0;i<11;i++) {rectangle(x+2+i,y+5+i,2,2,ink);rectangle(x+12-i,y+5+i,2,2,ink);}
        } else { char glyph[2]={(char)ch,0}; lcd_text(x,y,glyph,3,ink); }
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
    palette(); calculation_glass=title==NULL; canvas.ink=LCD_INK; canvas.paper=LCD_PAPER;
    clip_left=0; clip_top=0; clip_right=395; clip_bottom=223;
    if(!title) {
        dclear(LCD_PAPER);
        drect_border(0,0,395,223,LCD_PAPER,2,blend(LCD_PAPER,LCD_INK,3));
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
static void status_label(int x,const char *label,bool active) {
    int width=(int)strlen(label)*6-1;
    if(active) drect(x-2,12,x+width+1,24,status_paper);
    lcd_text(x,15,label,1,active?LCD_INK:ghost_ink);
}
void lcd_calculator(void) {
    lcd_frame(NULL);
    const char *angles[]={"D","R","G"};
    // Real indicators share a baseline. Unlit positions retain their glyph
    // silhouettes in the glass, with a weaker shade than active symbols.
    status_label(15,"S",calc.shift);
    status_label(26,"A",calc.alpha||calc.alpha_lock);
    if(calc.alpha_lock) lcd_text(32,15,"*",1,LCD_INK);
    drect(40,12,58,24,status_paper);
    dline(42,19,44,22,LCD_INK);
    dline(44,22,47,14,LCD_INK);
    dline(47,14,56,14,LCD_INK);
    dline(49,17,55,17,LCD_INK);
    dline(50,20,55,20,LCD_INK);
    drect_border(63,12,72,24,status_paper,1,LCD_INK);
    lcd_text(65,15,angles[calc.angle],1,LCD_INK);
    status_label(80,"M",calc.memory_set);
    status_label(93,"FIX",calc.format==1);
    status_label(115,"SCI",calc.format==2);
    uint16_t history_ink=calc.count?LCD_INK:ghost_ink;
    if(calc.count) drect(365,12,378,24,status_paper);
    for(int row=0;row<4;row++)
        dline(372-row,16+row,372+row,16+row,history_ink);
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
    if(calc.selection_start>=0 && calc.selection_end>=0) {
        int a=calc.selection_start,b=calc.selection_end;if(a>b){int swap=a;a=b;b=swap;}
        for(int i=a;i<b;i++) {
            MathCaret c=paint_math_input(calc.input,4096,16-offset,mid,&measure_canvas,i,false);
            MathCaret n=paint_math_input(calc.input,4096,16-offset,mid,&measure_canvas,i+1,false);
            int width=c.y==n.y && n.x>c.x?n.x-c.x:8;
            rectangle(c.x,c.y+c.height-1,width,1,LCD_INK);
        }
    }
    if(!calc.result && !calc.error) {
        if(calc.insert_capture)rectangle(caret.x,caret.y+caret.height-2,10,2,LCD_INK);
        else rectangle(caret.x,caret.y,2,caret.height,LCD_INK);
    }
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
        uint16_t ink=i==selected?LCD_PAPER:LCD_INK;
        if(i==selected)drect(14,y-4,381,y+20,LCD_INK);
        char option[3];keyboard_option_label(i,option);
        lcd_text(21,y+3,option,1,ink);lcd_text(46,y,rows[i],2,ink);
    }
    char position[20]; snprintf(position,sizeof(position),"%d/%d",selected+1,count);
    lcd_text(338,18,position,1,LCD_INK);
    lcd_text(10,212,hint?hint:"↑/↓ select   OK choose   ↩ return",1,LCD_INK);
}
void lcd_help(int page) {
    lcd_frame("GRAPH MATH+ / KEY GUIDE");
    lcd_text(18,38,"Key",1,LCD_INK);lcd_text(155,38,"SHIFT",1,LCD_INK);lcd_text(300,38,"ALPHA",1,LCD_INK);
    int start=page*5;
    for(size_t i=(size_t)start;i<graph_math_key_count && i<(size_t)start+5;i++) {
        int y=57+((int)i-start)*26;const KeyBinding *key=&graph_math_keys[i];
        lcd_text(18,y,key->label,1,LCD_INK);lcd_text(155,y,key->shift_label,1,LCD_INK);lcd_text(300,y,key->alpha_label,1,LCD_INK);
    }
    lcd_text(10,212,"←/→ page   ↩ return",1,LCD_INK);
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
    lcd_text(10,212,"← lighter   → darker   OK done",1,LCD_INK);
}
