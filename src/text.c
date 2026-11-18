#include "text.h"
#include <string.h>
unsigned text_codepoint(const char **p) {
 const unsigned char *s=(const unsigned char *)*p;unsigned c=*s++;int n=0;
 if(c>=0xc0&&c<0xe0){c&=31;n=1;}else if(c>=0xe0&&c<0xf0){c&=15;n=2;}else if(c>=0xf0){c&=7;n=3;}
 while(n-- && (*s&0xc0)==0x80)c=(c<<6)|(*s++&63);
 *p=(const char *)s;return c;
}
int text_characters(const char *text,int bytes) {
 const char *end=text+(bytes<0?(int)strlen(text):bytes);int count=0;
 while(text<end&&*text){text_codepoint(&text);count++;}return count;
}
const unsigned char *text_symbol(unsigned c) {
 static const struct {unsigned c;unsigned char rows[7];} glyphs[]={
 {0x03c0,{0,31,10,10,10,10,17}},{0x03b8,{14,17,17,31,17,17,14}},
 {0x221a,{3,2,2,18,10,6,2}},{0x25a1,{31,17,17,17,17,17,31}},
 {0x25a0,{31,31,31,31,31,31,31}},{0x2212,{0,0,0,31,0,0,0}},
 {0x00d7,{0,17,10,4,10,17,0}},{0x00f7,{0,4,0,31,0,4,0}},
 {0x2194,{0,10,17,31,17,10,0}},{0x2192,{0,4,2,31,2,4,0}},{0x2190,{0,4,8,31,8,4,0}},
 {0x2191,{4,14,21,4,4,4,0}},{0x2193,{4,4,4,21,14,4,0}},
 {0x21a9,{0,1,9,31,8,4,0}},{0x232b,{7,9,21,25,21,9,7}},
 {0x21e4,{17,21,25,31,25,21,17}},{0x21e5,{17,21,19,31,19,21,17}},
 {0x21c8,{10,31,10,10,10,10,0}},{0x21ca,{10,10,10,10,31,10,0}},
 {0x00b2,{12,2,4,8,14,0,0}},{0x00b3,{12,2,4,2,12,0,0}},
 {0x00b9,{4,12,4,4,14,0,0}},{0x207b,{0,0,14,0,0,0,0}},
 {0x207f,{0,14,10,10,10,0,0}},{0x2090,{0,0,0,14,2,14,14}},
 {0x02b3,{12,8,8,8,0,0,0}},{0x00b0,{6,9,6,0,0,0,0}},
 {0x2032,{4,4,8,0,0,0,0}},{0x2033,{10,10,20,0,0,0,0}},
 {0x2220,{1,2,4,8,31,0,0}},{0x2248,{0,9,22,0,9,22,0}},
 {0x2014,{0,0,0,31,0,0,0}}
 };
 if(c==20)c=0x2220;
 if(c==21)c=0x02b3;
 if(c==22)c=0x03b8;
 if(c==23)c=0x2192;
 if(c==24)c=0x00b0;
 if(c==25)c=0x2212;
 for(unsigned i=0;i<sizeof(glyphs)/sizeof(*glyphs);i++)if(glyphs[i].c==c)return glyphs[i].rows;
 return 0;
}
