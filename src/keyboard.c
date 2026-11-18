/* Graph Math+ physical legends: Casio product photo and OS 2.00 manual,
 * pp. 5, 9, 18–20, 217. See KEYBINDS.md for the complete source-linked audit.
 * Unprinted SHIFT legends fall back to the primary action; no invented shifts.
 */
#include "keyboard.h"
#define A(kind) {KA_##kind,""}
#define T(text) {KA_TEXT,text}
#define CONTROL(code,label,kind) {code,label,"—","—",A(kind),A(kind),NULL}
#define KEY(code,label,kind,text,sl,sk,st,alpha) {code,label,sl,alpha,{KA_##kind,text},{KA_##sk,st},alpha}
const KeyBinding graph_math_keys[]={
 CONTROL(KEY_ON,"ON",ON),CONTROL(KEY_HOME,"HOME",HOME),CONTROL(KEY_PREVTAB,"⇤",PREVTAB),
 CONTROL(KEY_UP,"↑",UP),CONTROL(KEY_NEXTTAB,"⇥",NEXTTAB),CONTROL(KEY_PAGEUP,"⇈",PAGEUP),
 CONTROL(KEY_SETTINGS,"SETTINGS",SETTINGS),CONTROL(KEY_BACK,"↩",BACK),CONTROL(KEY_LEFT,"←",LEFT),
 CONTROL(KEY_OK,"OK",OK),CONTROL(KEY_RIGHT,"→",RIGHT),CONTROL(KEY_PAGEDOWN,"⇊",PAGEDOWN),
 {KEY_SHIFT,"SHIFT","—","—",A(MODIFIER),A(MODIFIER),NULL},
 {KEY_ALPHA,"ALPHA","A-LOCK","—",A(MODIFIER),A(MODIFIER),NULL},
 CONTROL(KEY_VARS,"VARIABLE",VARIABLE),CONTROL(KEY_DOWN,"↓",DOWN),CONTROL(KEY_CATALOG,"CATALOG",CATALOG),CONTROL(KEY_TOOLS,"TOOLS",TOOLS),
 KEY(KEY_XOT,"x,θ,T,Y",TEXT,"x","→",TEXT,"\027","A"),
 KEY(KEY_FRAC,"□/□",FRACTION,"","■ □/□",MIXED,"","B"),
 KEY(KEY_SQRT,"√□",TEXT,"sqrt(","ⁿ√□",ROOT,"","C"),
 KEY(KEY_POWER,"□^□",TEXT,"^(","logₐ□",LOGBASE,"","D"),
 KEY(KEY_SQUARE,"□²",TEXT,"^2","log",TEXT,"log(","E"),
 KEY(KEY_EXPFUN,"e^□",TEXT,"exp(","ln",TEXT,"ln(","F"),
 KEY(KEY_COMMA,",",TEXT,",",":",TEXT,":","G"),
 KEY(KEY_SIN,"sin",TEXT,"sin(","sin⁻¹",TEXT,"asin(","H"),
 KEY(KEY_COS,"cos",TEXT,"cos(","cos⁻¹",TEXT,"acos(","I"),
 KEY(KEY_TAN,"tan",TEXT,"tan(","tan⁻¹",TEXT,"atan(","J"),
 KEY(KEY_LEFTP,"(",TEXT,"(","=",TEXT,"=","K"),
 KEY(KEY_RIGHTP,")",TEXT,")","—",TEXT,")","L"),
 KEY(KEY_7,"7",TEXT,"7","π",TEXT,"pi","M"),
 KEY(KEY_8,"8",TEXT,"8","∠",TEXT,"\024","N"),
 KEY(KEY_9,"9",TEXT,"9","i",TEXT,"i","O"),
 {KEY_DEL,"⌫","INS","—",A(DELETE),A(INSERT),NULL},
 {KEY_ACON,"AC","OFF","—",A(CLEAR),A(OFF),NULL},
 KEY(KEY_4,"4",TEXT,"4","[",TEXT,"[","P"),
 KEY(KEY_5,"5",TEXT,"5","]",TEXT,"]","Q"),
 KEY(KEY_6,"6",TEXT,"6","—",TEXT,"6","R"),
 KEY(KEY_MUL,"×",TEXT,"*","—",TEXT,"*","S"),
 KEY(KEY_DIV,"÷",TEXT,"\034","—",TEXT,"\034","T"),
 KEY(KEY_1,"1",TEXT,"1","ʳ",TEXT,"\025","U"),
 KEY(KEY_2,"2",TEXT,"2","θ",TEXT,"\026","V"),
 KEY(KEY_3,"3",TEXT,"3","—",TEXT,"3","W"),
 KEY(KEY_ADD,"+",TEXT,"+","°′″",TEXT,"\030","X"),
 KEY(KEY_SUB,"−",TEXT,"-","(-)",TEXT,"\031","Y"),
 {KEY_0,"0","Select","Z",T("0"),A(SELECT),"Z"},
 {KEY_DOT,".","Paste","Space",T("."),A(PASTE)," "},
 {KEY_EXP,"×10^□","Capture","\"",T("*10^("),A(CAPTURE),"\""},
 {KEY_FORMAT,"FORMAT","Ans","—",A(FORMAT),T("Ans"),NULL},
 {KEY_EXE,"EXE","≈","—",A(EXE),A(DECIMAL_EXE),NULL}
};
const size_t graph_math_key_count=sizeof(graph_math_keys)/sizeof(*graph_math_keys);
const KeyBinding *keyboard_binding(int key) {
 for(size_t i=0;i<graph_math_key_count;i++)if(graph_math_keys[i].code==key)return &graph_math_keys[i];
 return NULL;
}
KeyAction keyboard_decode(KeyboardState *s,int key) {
 if(key==KEY_SHIFT){s->shift=!s->shift;s->alpha=false;return (KeyAction)A(MODIFIER);}
 if(key==KEY_ALPHA){
  if(s->shift){s->alpha_lock=true;s->alpha=true;s->shift=false;}
  else if(s->alpha_lock){s->alpha_lock=false;s->alpha=false;}
  else s->alpha=!s->alpha;
  return (KeyAction)A(MODIFIER);
 }
 const KeyBinding *binding=keyboard_binding(key);
 KeyAction action=A(NONE);
 if(binding)action=s->shift?binding->shift:
     (s->alpha||s->alpha_lock)&&binding->alpha?(KeyAction)T(binding->alpha):binding->primary;
 s->shift=false;s->alpha=s->alpha_lock;
 return action;
}
int keyboard_from_cg50_matrix(int key) {
 // The emulator supplies fx-CG50 matrix positions. Translate the same physical
 // switches through gint's Math+ map, so tests exercise this release's real map.
 static const int rows[]={KEY_ON,KEY_HOME,KEY_PREVTAB,KEY_UP,KEY_NEXTTAB,KEY_PAGEUP,
  KEY_SETTINGS,KEY_BACK,KEY_LEFT,KEY_OK,KEY_RIGHT,KEY_PAGEDOWN,
  KEY_SHIFT,KEY_ALPHA,KEY_VARS,KEY_DOWN,KEY_CATALOG,KEY_TOOLS,
  KEY_XOT,KEY_FRAC,KEY_SQRT,KEY_POWER,KEY_SQUARE,KEY_EXPFUN,
  KEY_COMMA,KEY_SIN,KEY_COS,KEY_TAN,KEY_LEFTP,KEY_RIGHTP};
 int row=key>>4,col=(key&15)-1;
 if(row>=5&&row<=9&&col>=0&&col<6)return rows[(9-row)*6+col];
 if(key==KEY_NEG)return KEY_FORMAT;
 return key;
}

void keyboard_option_label(int index,char label[3]) {
 label[0]=index<9?'1'+index:index<35?'A'+index-9:'?';label[1]=0;
}
int keyboard_menu_option(int key,bool shift,bool alpha) {
 if(shift)return -1;
 const KeyBinding *b=keyboard_binding(key);if(!b)return -1;
 if(b->alpha && b->alpha[0]>='A' && b->alpha[0]<='Z' && !b->alpha[1] && (alpha||b->alpha[0]<='L'))return 9+b->alpha[0]-'A';
 if(!alpha && b->primary.kind==KA_TEXT && b->primary.text[0]>='1' && b->primary.text[0]<='9' && !b->primary.text[1])return b->primary.text[0]-'1';
 return -1;
}
