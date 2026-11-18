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
 assert(keyboard_from_cg50_matrix(KEY_F1)==KEY_ON);assert(keyboard_from_cg50_matrix(KEY_F2)==KEY_HOME);
 assert(keyboard_from_cg50_matrix(KEY_ALPHA)==KEY_SHIFT);assert(keyboard_from_cg50_matrix(KEY_SQUARE)==KEY_ALPHA);
 assert(keyboard_from_cg50_matrix(KEY_LN)==KEY_SQRT);assert(keyboard_from_cg50_matrix(KEY_COS)==KEY_SQUARE);
 assert(keyboard_from_cg50_matrix(KEY_NEG)==KEY_FORMAT);
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
 puts("50 physical keys × 3 layers, all 26 letters, spaces/quotes, printed SHIFT actions, ALPHA lock, matrix translation and label glyphs passed.");
}
