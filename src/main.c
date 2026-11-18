#include "calculator.h"
#include "lcd.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <string.h>

typedef struct { const char *label,*insert; } Function;
static const Function functions[]={
    {"Square root", "sqrt("}, {"Cube root", "cbrt("}, {"Power", "^("},
    {"Reciprocal", "^(-1)"}, {"Absolute value", "abs("}, {"Percent", "%"},
    {"Factorial", "!"}, {"Pi", "pi"}, {"Euler number e", "e"},
    {"Sine", "sin("}, {"Cosine", "cos("}, {"Tangent", "tan("},
    {"Inverse sine", "asin("}, {"Inverse cosine", "acos("}, {"Inverse tangent", "atan("},
    {"Natural log", "ln("}, {"Log base 10", "log("}, {"Log base b: logb(x,b)", "logb("},
    {"Exponential", "exp("}, {"10 to a power", "10^("}, {"nth root: root(x,n)", "root("},
    {"Combinations: ncr(n,r)", "ncr("}, {"Permutations: npr(n,r)", "npr("},
    {"Greatest divisor: gcd(a,b)", "gcd("}, {"Least multiple: lcm(a,b)", "lcm("},
    {"Remainder: mod(a,b)", "mod("}, {"Minimum: min(a,b)", "min("}, {"Maximum: max(a,b)", "max("},
    {"Round down", "floor("}, {"Round up", "ceil("}, {"Round nearest", "round("},
    {"Hyperbolic sine", "sinh("}, {"Hyperbolic cosine", "cosh("}, {"Hyperbolic tangent", "tanh("},
    {"Inverse sinh", "asinh("}, {"Inverse cosh", "acosh("}, {"Inverse tanh", "atanh("},
    {"Normal CDF: norm(x)", "norm("}, {"Binomial: binom(n,k,p)", "binom("},
    {"Comma / next argument", ","}, {"Answer memory", "Ans"}
};
enum { FunctionCount=sizeof(functions)/sizeof(*functions) };
static int view,selected,help_page;
static bool done;
static key_event_t nextkey(void) {
    return getkey_opt(GETKEY_REP_ARROWS|GETKEY_REP_PROFILE,NULL);
}
static int digit(int key) {
    const int keys[]={KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9};
    for(int i=0;i<10;i++) if(keys[i]==key) return i;
    return -1;
}
static void open_view(int id) { view=id;selected=0;calc.shift=false;calc.alpha=false; }
static void render(void) {
    if(view==0) lcd_calculator();
    if(view==1) {
        const char *rows[FunctionCount]; for(int i=0;i<FunctionCount;i++) rows[i]=functions[i].label;
        lcd_menu("CATALOG",rows,FunctionCount,selected,NULL);
    }
    if(view==2) {
        const char *angle[]={"Degree","Radian","Gradian"};
        const char *format[]={"Norm (10 sig. digits)","Fix","Sci"};
        char labels[8][40];
        snprintf(labels[0],40,"Angle: %s",angle[calc.angle]);
        snprintf(labels[1],40,"Output: %s",calc.decimal?"Decimal":"Exact");
        snprintf(labels[2],40,"Number: %s",format[calc.format]);
        snprintf(labels[3],40,"Fix/Sci digits: %d",calc.precision);
        snprintf(labels[4],40,"Contrast: %d / 10",calc.contrast);
        strcpy(labels[5],"Variables / memory"); strcpy(labels[6],"Help / key guide"); strcpy(labels[7],"Clear history");
        const char *rows[8]; for(int i=0;i<8;i++) rows[i]=labels[i];
        lcd_menu("SETTINGS",rows,8,selected,"EXE change   SHIFT+EXE reverse   BACK return");
    }
    if(view==3) {
        char labels[9][40]; const char *rows[9];
        const char *names="ABCDEFXYZ";
        for(int i=0;i<9;i++) {
            if(calc.memory_set&(1<<i)) snprintf(labels[i],40,"%c = %.10g",names[i],calc.memory[i]);
            else snprintf(labels[i],40,"%c = (empty)",names[i]);
            rows[i]=labels[i];
        }
        lcd_menu("VARIABLES / ANS MEMORY",rows,9,selected,"EXE recall   SHIFT+EXE store Ans   BACK return");
    }
    if(view==4) lcd_help(help_page);
    if(view==5) lcd_contrast();
    dupdate();
}
static void calculate_key(int key,bool shift,bool alpha) {
    int number=digit(key);
    if(alpha && number>=1 && number<=9) { calculator_recall(number-1); return; }
    if(key==KEY_EXE || key==KEY_OK) { calculator_evaluate(); return; }
    if(key==KEY_BACK || key==KEY_ACON) { calculator_clear(); return; }
    if(key==KEY_LEFT || key==KEY_RIGHT) {
        calc.result=false; calc.error=false;
        if(shift) calc.cursor=key==KEY_LEFT?0:(int)strlen(calc.input);
        else calculator_move(key==KEY_LEFT?-1:1);
        return;
    }
    if(key==KEY_UP || key==KEY_DOWN) { if(!calc.result && calculator_move(key==KEY_UP?-2:2)) return; calculator_history(key==KEY_UP?1:-1); return; }
    if(key==KEY_DEL) { if(shift) calculator_clear(); else calculator_delete(); return; }
    if(key==KEY_CATALOG || key==KEY_F1) { open_view(1); return; }
    if(key==KEY_TOOLS || key==KEY_SETTINGS || key==KEY_F2) { open_view(2); return; }
    if(key==KEY_VARS || (key==KEY_STORE && shift) || key==KEY_F3) { open_view(3); return; }
    if(key==KEY_FORMAT) { if(shift) calc.angle=(calc.angle+1)%3; else calc.decimal=!calc.decimal; return; }
    if(key==KEY_F4) { open_view(4); return; }
    const char *text=NULL;
    if(number>=0) {
        if(shift && number==7) text="pi";
        else if(shift && number==8) text="e";
        else { char s[2]={'0'+number,0}; calculator_type(s); return; }
    }
    if(key==KEY_DOT) text=shift?"!":".";
    if(key==KEY_ADD) text="+";
    if(key==KEY_SUB) text="-";
    if(key==KEY_MUL) text="*";
    if(key==KEY_FRAC && !shift) {calculator_fraction();return;}
    if(key==KEY_DIV) text=shift?"%":"\034";
    if(key==KEY_FRAC && shift) text="%";
    if(key==KEY_LEFTP) text="(";
    if(key==KEY_RIGHTP) text=shift?",":")";
    if(key==KEY_COMMA) text=",";
    if(key==KEY_SQUARE) text=shift?"sqrt(":"^2";
    if(key==KEY_POWER) text=shift?"^(-1)":"^(";
    if(key==KEY_SQRT) text=shift?"cbrt(":"sqrt(";
    if(key==KEY_SIN) text=shift?"asin(":"sin(";
    if(key==KEY_COS) text=shift?"acos(":"cos(";
    if(key==KEY_TAN) text=shift?"atan(":"tan(";
    if(key==KEY_LN) text=shift?"exp(":"ln(";
    if(key==KEY_LOG) text=shift?"10^(":"log(";
    if(key==KEY_EXPFUN) text=shift?"ln(":"exp(";
    if(key==KEY_EXP) text=shift?"pi":"*10^(";
    if(key==KEY_NEG) text=shift?"e":"-";
    if(key==KEY_STORE) text="Ans";
    if(key==KEY_X || key==KEY_XOT) { calculator_recall(6); return; }
    if(text) calculator_type(text);
}
int main(void) {
    extern void __printf_enable_fp(void);
    __printf_enable_fp();
    calculator_init(); calculator_load();
    clearevents();
    while(!done) {
        render();
        key_event_t event=nextkey(); int key=event.key;
        if(key==KEY_HOME) { done=true; continue; }
        if(key==KEY_SHIFT) { calc.shift=!calc.shift; continue; }
        if(key==KEY_ALPHA) { calc.alpha=!calc.alpha; continue; }
        bool shift=calc.shift,alpha=calc.alpha; calc.shift=false;calc.alpha=false;
        if(view==0) calculate_key(key,shift,alpha);
        else if(view==5) {
            if(key==KEY_LEFT && calc.contrast>0) calc.contrast--;
            if(key==KEY_RIGHT && calc.contrast<10) calc.contrast++;
            if(key==KEY_EXE || key==KEY_OK || key==KEY_BACK || key==KEY_ACON) { view=2;selected=4; }
        }
        else if(key==KEY_BACK || key==KEY_ACON) { view=0;selected=0; }
        else if(view==4) {
            if(key==KEY_RIGHT) help_page=(help_page+1)%4;
            if(key==KEY_LEFT) help_page=(help_page+3)%4;
        } else {
            int count=view==1?FunctionCount:view==2?8:9;
            if(key==KEY_UP) selected=(selected+count-1)%count;
            if(key==KEY_DOWN) selected=(selected+1)%count;
            if(key==KEY_LEFT) selected=selected>=5?selected-5:0;
            if(key==KEY_RIGHT) selected=selected+5<count?selected+5:count-1;
            if(key==KEY_EXE || key==KEY_OK) {
                if(view==1) { calculator_type(functions[selected].insert);view=0; }
                else if(view==3) { if(shift) calculator_store(selected); else { calculator_recall(selected);view=0; } }
                else if(view==2) {
                    if(selected==0) calc.angle=(calc.angle+(shift?2:1))%3;
                    if(selected==1) calc.decimal=!calc.decimal;
                    if(selected==2) calc.format=(calc.format+(shift?2:1))%3;
                    if(selected==3) calc.precision=(calc.precision+(shift?9:1))%10;
                    if(selected==4) open_view(5);
                    else if(selected==5) open_view(3);
                    else if(selected==6) { open_view(4);help_page=0; }
                    else if(selected==7) { calc.count=0;calc.ans=0;calculator_clear();view=0; }
                }
            }
        }
    }
    // Commit settings, memory and history once, keeping ordinary entry responsive.
    if(!calculator_save()) {
        lcd_frame("SAVE ERROR");
        lcd_text(18,60,"Could not save lcdmath.dat",1,LCD_INK);
        lcd_text(18,83,"Check USB storage free space.",1,LCD_INK);
        lcd_text(18,120,"EXE or HOME exits without saving.",1,LCD_INK); dupdate();
        int key; do { key=nextkey().key; } while(key!=KEY_EXE && key!=KEY_OK && key!=KEY_HOME);
    }
    return 0;
}
