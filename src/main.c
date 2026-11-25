#include "calculator.h"
#include "keyboard.h"
#include "lcd.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/hardware.h>
#include <gint/gint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

typedef struct {const char *label,*insert;} Function;
static const Function functions[]={
 {"√□","sqrt("},{"³√□","cbrt("},{"□^□","^("},{"□⁻¹","^(-1)"},{"Abs(□)","abs("},
 {"%","%"},{"!","!"},{"π","pi"},{"e","e"},{"sin","sin("},{"cos","cos("},{"tan","tan("},
 {"sin⁻¹","asin("},{"cos⁻¹","acos("},{"tan⁻¹","atan("},{"ln","ln("},{"log","log("},
 {"logₐ□","logb("},{"e^□","exp("},{"10^□","10^("},{"ⁿ√□","root("},
 {"nCr(n,r)","ncr("},{"nPr(n,r)","npr("},{"gcd(a,b)","gcd("},{"lcm(a,b)","lcm("},
 {"mod(a,b)","mod("},{"min(a,b)","min("},{"max(a,b)","max("},
 {"floor(□)","floor("},{"ceil(□)","ceil("},{"round(□)","round("},
 {"sinh","sinh("},{"cosh","cosh("},{"tanh","tanh("},{"sinh⁻¹","asinh("},
 {"cosh⁻¹","acosh("},{"tanh⁻¹","atanh("},{"norm(□)","norm("},{"binom(n,k,p)","binom("},
 {",",","},{"Ans","Ans"},{"=","="},{"[","["},{"]","]"},{":",":"},{"Space"," "},{"\"","\""}
};
enum {FunctionCount=sizeof(functions)/sizeof(*functions)};
static int view,selected,help_page;
static bool done;
static KeyboardState keyboard;
static char clipboard[INPUT_CAP];
static int selection_start=-1,selection_end=-1;
static int nextkey(void){
 for(;;){
  key_event_t event=getkey_opt(GETKEY_REP_ALL|GETKEY_REP_PROFILE,NULL);
  int key=keyboard_event_code(event.key,event.row,event.col);
  if(event.type==KEYEV_DOWN || (event.type==KEYEV_HOLD &&
      (key==KEY_UP || key==KEY_DOWN || key==KEY_LEFT || key==KEY_RIGHT)))
   return key;
 }
}
static void open_view(int id){view=id;selected=0;}
static void insert_function(const char *text){
 if(!strcmp(text,"root(")||!strcmp(text,"logb("))calculator_indexed(text);
 else calculator_type(text);
}
static void render(void){
 if(view==0)lcd_calculator();
 if(view==1){const char *rows[35];for(int i=0;i<34;i++)rows[i]=functions[i].label;rows[34]="More";lcd_menu("CATALOG",rows,35,selected,NULL);}
 if(view==9){const char *rows[FunctionCount-34];for(int i=34;i<FunctionCount;i++)rows[i-34]=functions[i].label;lcd_menu("CATALOG / MORE",rows,FunctionCount-34,selected,NULL);}
 if(view==2){
  const char *angles[]={"Degrees","Radians","Gradians"};const char *formats[]={"Norm","Fix","Sci"};
  char labels[8][40];snprintf(labels[0],40,"Angle: %s",angles[calc.angle]);
  snprintf(labels[1],40,"Output: %s",calc.decimal?"Decimal":"Standard");
  snprintf(labels[2],40,"Number: %s",formats[calc.format]);snprintf(labels[3],40,"Fix/Sci digits: %d",calc.precision);
  snprintf(labels[4],40,"Contrast: %d / 10",calc.contrast);strcpy(labels[5],"VARIABLE");strcpy(labels[6],"Graph Math+ key guide");strcpy(labels[7],"Clear history");
  const char *rows[8];for(int i=0;i<8;i++)rows[i]=labels[i];lcd_menu("SETTINGS",rows,8,selected,"↑/↓ select   OK change   ↩ return");
 }
 if(view==3){
  char labels[MEMORY_COUNT][48];const char *rows[MEMORY_COUNT];
  for(int i=0;i<MEMORY_COUNT;i++){
   if(calc.memory_set&(1u<<i))snprintf(labels[i],48,"%s = %.10g",calculator_memory_name(i),calc.memory[i]);
   else snprintf(labels[i],48,"%s = (unset)",calculator_memory_name(i));
   rows[i]=labels[i];
  }
  lcd_menu("VARIABLE",rows,MEMORY_COUNT,selected,"OK insert   Store: value → variable EXE");
 }
 if(view==4)lcd_help(help_page);
 if(view==5)lcd_contrast();
 if(view==6){const char *rows[]={"Standard ↔ Decimal","Mixed ↔ Improper","°′″","Engineering","Engineering reverse"};lcd_menu("FORMAT",rows,5,selected,NULL);}
 if(view==7){const char *rows[]={"Copy","Cut"};lcd_menu("SELECT",rows,2,selected,"←/→ set range before OK   ↩ cancel");}
 if(view==8){const char *rows[]={"Graph Math+ key guide","Copy expression","Paste","Clear history"};lcd_menu("TOOLS",rows,4,selected,NULL);}
 lcd_finish();dupdate();
}
static void put32(unsigned char *p,uint32_t n){for(int i=0;i<4;i++)p[i]=(unsigned char)(n>>(8*i));}
static bool capture(void){
 unsigned char header[54]={0},row[DWIDTH*3];header[0]='B';header[1]='M';put32(header+2,54+DWIDTH*DHEIGHT*3);
 put32(header+10,54);put32(header+14,40);put32(header+18,DWIDTH);put32(header+22,DHEIGHT);header[26]=1;header[28]=24;
 int fd=open("lcdshot.bmp",O_WRONLY|O_CREAT|O_TRUNC,0666);if(fd<0)return false;
 bool ok=write(fd,header,54)==54;
 for(int y=DHEIGHT-1;y>=0&&ok;y--){for(int x=0;x<DWIDTH;x++){
   uint16_t c=gint_vram[y*DWIDTH+x];row[3*x]=(c&31)*255/31;row[3*x+1]=((c>>5)&63)*255/63;row[3*x+2]=(c>>11)*255/31;
  }ok=write(fd,row,sizeof(row))==(int)sizeof(row);
 }
 return close(fd)==0&&ok;
}
static void copy_selection(bool cut){
 int start=selection_start,end=selection_end;
 if(start>end){int swap=start;start=end;end=swap;}
 if(start<0||end<start||end>(int)strlen(calc.input))return;
 memcpy(clipboard,calc.input+start,(size_t)(end-start));clipboard[end-start]=0;
 if(cut){memmove(calc.input+start,calc.input+end,strlen(calc.input+end)+1);calc.cursor=start;calc.result=false;}
 selection_start=selection_end=-1;calc.selection_start=calc.selection_end=-1;
}
static void calculate_action(KeyAction action){
 switch(action.kind){
 case KA_TEXT:calculator_type(action.text);break;
 case KA_FRACTION:calculator_fraction();break;
 case KA_MIXED:calculator_mixed_fraction();break;
 case KA_ROOT:calculator_indexed("root(");break;
 case KA_LOGBASE:calculator_indexed("logb(");break;
 case KA_OK:
  if(selection_start>=0){selection_end=calc.cursor;open_view(7);}else calculator_evaluate();break;
 case KA_EXE:calculator_evaluate();break;
 case KA_DECIMAL_EXE:if(calculator_evaluate())calc.approximate_result=true;break;
 case KA_CLEAR:calculator_clear();selection_start=selection_end=-1;break;
 case KA_BACK:if(selection_start>=0){selection_start=selection_end=-1;calc.selection_start=calc.selection_end=-1;}else calculator_clear();break;
 case KA_LEFT:case KA_RIGHT:calc.result=false;calc.error=false;calculator_move(action.kind==KA_LEFT?-1:1);break;
 case KA_UP:case KA_DOWN:
  if(!calc.result&&calculator_move(action.kind==KA_UP?-2:2))break;
  calculator_history(action.kind==KA_UP?1:-1);break;
 case KA_PAGEUP:calculator_history(1);break;
 case KA_PAGEDOWN:calculator_history(-1);break;
 case KA_DELETE:calculator_delete();break;
 case KA_INSERT:calc.insert_capture=true;break;
 case KA_VARIABLE:open_view(3);break;
 case KA_SETTINGS:open_view(2);break;
 case KA_TOOLS:open_view(8);break;
 case KA_CATALOG:open_view(1);break;
 case KA_FORMAT:open_view(6);break;
 case KA_SELECT:selection_start=selection_end=calc.cursor;calc.selection_start=calc.selection_end=calc.cursor;break;
 case KA_PASTE:if(clipboard[0])calculator_type(clipboard);break;
 case KA_CAPTURE:render();if(!capture()){calc.error=true;strcpy(calc.message,"Cannot write lcdshot.bmp");}break;
 default:break;
 }
 if(selection_start>=0&&view==0){selection_end=calc.cursor;calc.selection_start=selection_start;calc.selection_end=selection_end;}
}
static void format_result(void){
 if(selected==0){calc.decimal=!(calc.decimal||calc.approximate_result);calc.approximate_result=false;calc.sexagesimal=false;calc.engineering=false;}
 if(selected==1){calc.mixed_result=!calc.mixed_result;calc.decimal=false;calc.sexagesimal=false;calc.engineering=false;}
 if(selected==2){calc.sexagesimal=true;calc.engineering=false;}
 if(selected==3||selected==4){if(!calc.engineering)calc.engineering_steps=0;else calc.engineering_steps+=selected==3?-3:3;calc.engineering=true;calc.sexagesimal=false;}
 view=0;
}
int main(void){
 extern void __printf_enable_fp(void);__printf_enable_fp();calculator_init();calculator_load();clearevents();
 while(!done){
  render();int key=nextkey();
  bool had_shift=keyboard.shift,had_alpha=keyboard.alpha||keyboard.alpha_lock;
  KeyAction action=keyboard_decode(&keyboard,key);calc.shift=keyboard.shift;calc.alpha=keyboard.alpha;calc.alpha_lock=keyboard.alpha_lock;
  if(action.kind==KA_MODIFIER)continue;
  if(action.kind==KA_HOME){done=true;continue;}
  if(action.kind==KA_OFF){calculator_save();if(gint[HWCALC]==HWCALC_FXCG100)gint_poweroff(true);else done=true;continue;}
  if(action.kind==KA_ON)continue;
  if(view==0){calculate_action(action);continue;}
  if(view==5){
   if(action.kind==KA_LEFT&&calc.contrast>0)calc.contrast--;
   if(action.kind==KA_RIGHT&&calc.contrast<10)calc.contrast++;
   if(action.kind==KA_OK||action.kind==KA_EXE||action.kind==KA_BACK||action.kind==KA_CLEAR){view=2;selected=4;}continue;
  }
  if(action.kind==KA_BACK||action.kind==KA_CLEAR){view=0;selection_start=selection_end=-1;calc.selection_start=calc.selection_end=-1;continue;}
  if(view==4){
   int pages=(int)((graph_math_key_count+4)/5);
   if(action.kind==KA_RIGHT||action.kind==KA_NEXTTAB||action.kind==KA_PAGEDOWN)help_page=(help_page+1)%pages;
   if(action.kind==KA_LEFT||action.kind==KA_PREVTAB||action.kind==KA_PAGEUP)help_page=(help_page+pages-1)%pages;
   continue;
  }
  int count=view==1?35:view==9?FunctionCount-34:view==2?8:view==3?MEMORY_COUNT:view==6?5:view==7?2:4;
  int shortcut=keyboard_menu_option(key,had_shift,had_alpha);
  if(shortcut>=0 && shortcut<count){selected=shortcut;action=(KeyAction){KA_OK,""};}
  if(action.kind==KA_UP)selected=(selected+count-1)%count;
  if(action.kind==KA_DOWN)selected=(selected+1)%count;
  if(action.kind==KA_LEFT||action.kind==KA_PAGEUP)selected=selected>=5?selected-5:0;
  if(action.kind==KA_RIGHT||action.kind==KA_PAGEDOWN)selected=selected+5<count?selected+5:count-1;
  if(action.kind==KA_OK||action.kind==KA_EXE||action.kind==KA_DECIMAL_EXE){
   if(view==1){if(selected==34)open_view(9);else {insert_function(functions[selected].insert);view=0;}}
   else if(view==9){insert_function(functions[selected+34].insert);view=0;}
   else if(view==3){calculator_type(calculator_memory_name(selected));view=0;}
   else if(view==6)format_result();
   else if(view==7){copy_selection(selected==1);view=0;}
   else if(view==8){
    if(selected==0){open_view(4);help_page=0;}
    else if(selected==1){strcpy(clipboard,calc.input);view=0;}
    else if(selected==2){if(clipboard[0])calculator_type(clipboard);view=0;}
    else {calc.count=0;calc.ans=0;calculator_clear();view=0;}
   }else if(view==2){
    if(selected==0)calc.angle=(calc.angle+1)%3;
    else if(selected==1)calc.decimal=!calc.decimal;
    else if(selected==2)calc.format=(calc.format+1)%3;
    else if(selected==3)calc.precision=(calc.precision+1)%10;
    else if(selected==4)open_view(5);
    else if(selected==5)open_view(3);
    else if(selected==6){open_view(4);help_page=0;}
    else {calc.count=0;calc.ans=0;calculator_clear();view=0;}
   }
  }
 }
 if(!calculator_save()){
  lcd_frame("SAVE ERROR");lcd_text(18,60,"Could not save lcdmath.dat",1,LCD_INK);lcd_text(18,83,"Check USB storage free space.",1,LCD_INK);
  lcd_text(18,120,"EXE or HOME exits without saving.",1,LCD_INK);lcd_finish();dupdate();
  int key;do{key=nextkey();}while(key!=KEY_EXE&&key!=KEY_OK&&key!=KEY_HOME);
 }
 return 0;
}
