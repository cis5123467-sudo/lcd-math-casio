#include "keyboard.h"
#include "text.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void expect(int key,bool shift,bool alpha,KeyActionKind kind,const char *text){
 KeyboardState state={shift,alpha,false};KeyAction a=keyboard_decode(&state,key);
 if(a.kind!=kind||strcmp(a.text,text)){fprintf(stderr,"Key %x SHIFT=%d ALPHA=%d: %d %s expected %d %s\n",key,shift,alpha,a.kind,a.text,kind,text);assert(0);}
 assert(!state.shift&&!state.alpha);
}
int main(int argc,char **argv){
 if(argc>1&&!strcmp(argv[1],"--dump")){
  for(size_t i=0;i<graph_math_key_count;i++){
   const KeyBinding *k=&graph_math_keys[i];printf("%02x\t%s\t%s\t%s\n",k->code,k->label,k->shift_label,k->alpha_label);
  }return 0;
 }
 assert(graph_math_key_count==50);
 for(size_t i=0;i<graph_math_key_count;i++){
  assert(graph_math_keys[i].label&&graph_math_keys[i].shift_label&&graph_math_keys[i].alpha_label);
  for(size_t j=0;j<i;j++)assert(graph_math_keys[i].code!=graph_math_keys[j].code);
  for(int mode=0;mode<3;mode++){
   KeyboardState s={mode==1,mode==2,false};KeyAction a=keyboard_decode(&s,graph_math_keys[i].code);assert(a.text!=NULL);
   if(a.kind!=KA_MODIFIER)assert(!s.shift&&!s.alpha);
  }
 }
 // Independent transcription of red legends, in physical row order.
 const int alpha_keys[]={KEY_XOT,KEY_FRAC,KEY_SQRT,KEY_POWER,KEY_SQUARE,KEY_EXPFUN,
  KEY_COMMA,KEY_SIN,KEY_COS,KEY_TAN,KEY_LEFTP,KEY_RIGHTP,KEY_7,KEY_8,KEY_9,
  KEY_4,KEY_5,KEY_6,KEY_MUL,KEY_DIV,KEY_1,KEY_2,KEY_3,KEY_ADD,KEY_SUB,KEY_0};
 for(int i=0;i<26;i++){char letter[2]={'A'+i,0};expect(alpha_keys[i],false,true,KA_TEXT,letter);}
 expect(KEY_DOT,false,true,KA_TEXT," ");expect(KEY_EXP,false,true,KA_TEXT,"\"");
 expect(KEY_FORMAT,false,true,KA_FORMAT,"");
 expect(KEY_XOT,false,false,KA_TEXT,"x");expect(KEY_XOT,true,false,KA_TEXT,"\027");
 expect(KEY_FRAC,false,false,KA_FRACTION,"");expect(KEY_FRAC,true,false,KA_MIXED,"");
 expect(KEY_SQRT,false,false,KA_TEXT,"sqrt(");expect(KEY_SQRT,true,false,KA_ROOT,"");
 expect(KEY_POWER,false,false,KA_TEXT,"^(");expect(KEY_POWER,true,false,KA_LOGBASE,"");
 expect(KEY_SQUARE,false,false,KA_TEXT,"^2");expect(KEY_SQUARE,true,false,KA_TEXT,"log(");
 expect(KEY_EXPFUN,false,false,KA_TEXT,"exp(");expect(KEY_EXPFUN,true,false,KA_TEXT,"ln(");
 expect(KEY_SIN,true,false,KA_TEXT,"asin(");expect(KEY_COS,true,false,KA_TEXT,"acos(");expect(KEY_TAN,true,false,KA_TEXT,"atan(");
 expect(KEY_COMMA,true,false,KA_TEXT,":");expect(KEY_LEFTP,true,false,KA_TEXT,"=");expect(KEY_RIGHTP,true,false,KA_TEXT,")");
 expect(KEY_7,true,false,KA_TEXT,"pi");expect(KEY_8,true,false,KA_TEXT,"\024");expect(KEY_9,true,false,KA_TEXT,"i");
 expect(KEY_4,true,false,KA_TEXT,"[");expect(KEY_5,true,false,KA_TEXT,"]");
 expect(KEY_1,true,false,KA_TEXT,"\025");expect(KEY_2,true,false,KA_TEXT,"\026");
 expect(KEY_ADD,true,false,KA_TEXT,"\030");expect(KEY_SUB,true,false,KA_TEXT,"\031");
 expect(KEY_FORMAT,true,false,KA_TEXT,"Ans");expect(KEY_EXE,true,false,KA_DECIMAL_EXE,"");
 expect(KEY_DEL,true,false,KA_INSERT,"");expect(KEY_ACON,true,false,KA_OFF,"");
 expect(KEY_0,true,false,KA_SELECT,"");expect(KEY_DOT,true,false,KA_PASTE,"");expect(KEY_EXP,true,false,KA_CAPTURE,"");
 assert(!keyboard_binding(KEY_X)&&!keyboard_binding(KEY_LOG)&&!keyboard_binding(KEY_LN)&&!keyboard_binding(KEY_ARROW));
 KeyboardState state={0};keyboard_decode(&state,KEY_SHIFT);keyboard_decode(&state,KEY_ALPHA);assert(state.alpha_lock&&state.alpha&&!state.shift);
 assert(!strcmp(keyboard_decode(&state,KEY_XOT).text,"A"));assert(!strcmp(keyboard_decode(&state,KEY_FRAC).text,"B"));assert(state.alpha_lock);
 keyboard_decode(&state,KEY_ALPHA);assert(!state.alpha_lock&&!state.alpha);
 keyboard_decode(&state,KEY_SHIFT);keyboard_decode(&state,KEY_SHIFT);assert(!state.shift);
 // Physical scan fixture from NumAI casio_keys.h (blob 928e23f) and
 // gint's shared numeric rows. The legacy code also encodes row and column.
 static const struct { int legacy, expected; } switches[]={
  {0x91,KEY_ON},{0x92,KEY_HOME},{0x93,KEY_PREVTAB},{0x94,KEY_UP},{0x95,KEY_NEXTTAB},{0x96,KEY_PAGEUP},
  {0x81,KEY_SETTINGS},{0x82,KEY_BACK},{0x83,KEY_LEFT},{0x84,KEY_OK},{0x85,KEY_RIGHT},{0x86,KEY_PAGEDOWN},
  {0x71,KEY_SHIFT},{0x72,KEY_ALPHA},{0x73,KEY_VARS},{0x74,KEY_DOWN},{0x75,KEY_CATALOG},{0x76,KEY_TOOLS},
  {0x61,KEY_XOT},{0x62,KEY_FRAC},{0x63,KEY_SQRT},{0x64,KEY_POWER},{0x65,KEY_SQUARE},{0x66,KEY_EXPFUN},
  {0x51,KEY_COMMA},{0x52,KEY_SIN},{0x53,KEY_COS},{0x54,KEY_TAN},{0x55,KEY_LEFTP},{0x56,KEY_RIGHTP},
  {0x41,KEY_7},{0x42,KEY_8},{0x43,KEY_9},{0x44,KEY_DEL},{0x07,KEY_ACON},
  {0x31,KEY_4},{0x32,KEY_5},{0x33,KEY_6},{0x34,KEY_MUL},{0x35,KEY_DIV},
  {0x21,KEY_1},{0x22,KEY_2},{0x23,KEY_3},{0x24,KEY_ADD},{0x25,KEY_SUB},
  {0x11,KEY_0},{0x12,KEY_DOT},{0x13,KEY_EXP},{0x14,KEY_FORMAT},{0x15,KEY_EXE}
 };
 assert(sizeof(switches)/sizeof(*switches)==graph_math_key_count);
 for(size_t i=0;i<sizeof(switches)/sizeof(*switches);i++){
  int raw=switches[i].legacy,expected=switches[i].expected;
  int row=raw>>4,col=7-(raw&7);
  for(int variant=0;variant<2;variant++){
   int mapped=keyboard_event_code(variant?expected:raw,row,col);
   assert(mapped==expected && keyboard_binding(mapped));
   for(int mode=0;mode<3;mode++){
    KeyboardState actual={mode==1,mode==2,false},reference=actual;
    KeyAction action=keyboard_decode(&actual,mapped);
    KeyAction want=keyboard_decode(&reference,expected);
    assert(action.kind==want.kind && !strcmp(action.text,want.text));
    assert(actual.alpha==reference.alpha && actual.shift==reference.shift &&
        actual.alpha_lock==reference.alpha_lock);
   }
  }
 }
 // These raw legacy codes reproduced the failures on physical hardware.
 expect(keyboard_event_code(0x52,5,5),false,false,KA_TEXT,"sin(");
 expect(keyboard_event_code(0x51,5,6),false,false,KA_TEXT,",");
 expect(keyboard_event_code(0x63,6,4),false,false,KA_TEXT,"sqrt(");
 expect(keyboard_event_code(0x62,6,5),false,false,KA_FRACTION,"");
 assert(keyboard_menu_option(KEY_1,false,false)==0);
 assert(keyboard_menu_option(KEY_9,false,false)==8);
 assert(keyboard_menu_option(KEY_XOT,false,false)==9);
 assert(keyboard_menu_option(KEY_RIGHTP,false,false)==20);
 assert(keyboard_menu_option(KEY_7,false,true)==21);
 assert(keyboard_menu_option(KEY_0,false,true)==34);
 assert(keyboard_menu_option(KEY_1,true,false)==-1);
 for(int i=0;i<35;i++) {
  char label[3];keyboard_option_label(i,label);
  assert(label[0]==(i<9?'1'+i:'A'+i-9)&&!label[1]);
  if(i>=9)assert(keyboard_menu_option(alpha_keys[i-9],false,i>=21)==i);
 }
 char option[3];keyboard_option_label(0,option);assert(!strcmp(option,"1"));keyboard_option_label(9,option);assert(!strcmp(option,"A"));keyboard_option_label(34,option);assert(!strcmp(option,"Z"));
 const char *labels[]={"π","θ","√","□","²","³","⁻","¹","ⁿ","ₐ","ʳ","°","′","″","×","÷","−","→","←","↑","↓","⇤","⇥","⇈","⇊","↩","⌫","≈","↔","■","—"};
 for(unsigned i=0;i<sizeof(labels)/sizeof(*labels);i++){const char *p=labels[i];assert(text_symbol(text_codepoint(&p)));assert(!*p);}
 assert(text_characters("sin⁻¹(θ)",-1)==8);
 puts("50 physical keys × 3 layers, all 26 letters, spaces/quotes, printed SHIFT actions, ALPHA lock, both raw event formats and label glyphs passed.");
}
