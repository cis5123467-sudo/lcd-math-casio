#include "mathview.h"
#include "text.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static int left,top,right,bottom,primitives;
static char text_seen[1024];
int casio_width(const char *text,int length,bool small,bool bold) {
    (void)bold;int n=text_characters(text,length);
    return n?n*(small?12:18)-(small?2:3):0;
}
static void rect(int x,int y,int w,int h,uint16_t ink) {
    (void)ink;if(w<=0||h<=0)return;
    if(x<left)left=x;if(y<top)top=y;
    if(x+w-1>right)right=x+w-1;if(y+h-1>bottom)bottom=y+h-1;
    primitives++;
}
static void text(int x,int y,const char *value,uint16_t ink,uint16_t paper,bool small) {
    (void)paper;strncat(text_seen,value,sizeof(text_seen)-strlen(text_seen)-1);
    if(*value)rect(x,y,casio_width(value,-1,small,false),small?14:22,ink);
}
static MathCanvas canvas={text,rect,1,0};
static void reset(void) {left=top=INT_MAX;right=bottom=INT_MIN;primitives=0;text_seen[0]=0;}
static void verify(const char *value) {
    MathBox box=measure_math(value,4096);reset();paint_math(value,4096,0,0,&canvas);
    if(left<0||right>=box.width||top < -box.above ||bottom>=box.below) {
        fprintf(stderr,"Bounds %s: box %d,%d,%d actual %d,%d,%d,%d\n",value,box.width,box.above,box.below,left,top,right,bottom);assert(0);
    }
    assert(primitives>0);
}
int main(void) {
    const char *cases[]={"{_}/{_}","{1}/{3}","{{1}/{2}}/{3}","{1}/{{2}/{3}}","({1}/{2})^2",
      "sqrt({_}/{_})","sqrt(sqrt(2))","sqrt(8)","cbrt(27)","root(32,5)","root(32,_)",
      "2^(_)","2^(3^(_))","9^({1}/{2})","2^(-3)","asin(0.5)","asinh(2)","logb(8,2)","exp(2)",
      "2*pi","8\0342*(1+1)","(1/2+3)*4","1/2pi","1/2(3+4)","2^3^2","-1/2","{_}/{2}","mixed(_,_,_)","mixed(2,1,3)","42\027A","12°30′0″","\"ABC DEF\""};
    for(unsigned i=0;i<sizeof(cases)/sizeof(*cases);i++)verify(cases[i]);
    reset();paint_math("",4096,0,0,&canvas);assert(primitives==0);
    reset();MathCaret exponent=paint_math_input("2^(_)",4096,0,0,&canvas,3,false);
    assert(exponent.height==14 && exponent.y<-11);
    reset();MathCaret numerator=paint_math_input("{_}/{_}",4096,0,0,&canvas,1,false);
    reset();MathCaret denominator=paint_math_input("{_}/{_}",4096,0,0,&canvas,5,false);
    assert(numerator.y<0 && denominator.y>0 && numerator.x==denominator.x);
    verify("sqrt(8)");assert(!strstr(text_seen,"sqrt"));
    verify("asin(0.5)");assert(strstr(text_seen,"sin") && strstr(text_seen,"-1") && !strstr(text_seen,"asin"));
    verify("exp(2)");assert(strstr(text_seen,"e") && !strstr(text_seen,"exp"));
    MathBox f=measure_math("1/2",4096),pi=measure_math("pi",4096),product=measure_math("1/2pi",4096);
    assert(product.width==f.width+pi.width);
    puts("32 layout bounds, superscript/field carets, symbolic function rendering and implicit-product grouping checks passed.");
}
