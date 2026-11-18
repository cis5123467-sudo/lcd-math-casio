#include "mathview.h"

#include <ctype.h>
#include <string.h>
extern int casio_width(const char *, int, bool, bool);

enum { LayoutLimit = 320, LayoutChars = 112 };
typedef enum { Text, Adjacent, Fraction, Radical, Raised, Lowered, Parentheses } LayoutKind;
typedef struct {
    LayoutKind kind;
    int a,b;
    MathBox box;
    const char *text, *start, *end;
    int length;
    char single;
    bool small;
} Layout;
static Layout layouts[LayoutLimit];
static int count, fraction_level;
static bool overflow, large;
static const char *cursor;
static MathCaret caret;
static int caret_depth;

static int max(int a,int b) { return a>b ? a:b; }

static int item(Layout layout) {
    if (count == LayoutLimit-1) { overflow = true; return 0; }
    layouts[++count] = layout;
    return count;
}

static int half_height(bool small) { return small ? 7 : 11; }

static int leaf(const char *text,int length,char one,bool small) {
    int half = half_height(small);
    int width = one ? casio_width(&one,1,small,false) : casio_width(text,length,small,false);
    return item((Layout){.kind=Text,.box={width,half,half},
                          .text=text,.length=length,.single=one,.small=small,
                          .start=text,.end=text ? text+length : NULL});
}

static int join(LayoutKind kind,int a,int b) {
    if (!a || (kind != Radical && kind != Parentheses && !b)) { overflow = true; return 0; }
    MathBox left = layouts[a].box, right = layouts[b].box, box;
    switch (kind) {
        case Fraction:
            box = (MathBox){max(left.width,right.width)+8,left.above+left.below+4,right.above+right.below+5};
            break;
        case Radical: {
            int prefix=layouts[a].small?12:16;
            box=(MathBox){left.width+prefix+2,left.above+max(4,right.above+right.below),left.below+1};
            break;
        }
        case Raised:
            box = (MathBox){left.width+right.width+1,max(left.above,left.above+right.above+right.below-4),left.below};
            break;
        case Lowered:
            box = (MathBox){left.width+right.width+1,left.above,max(left.below,left.below+right.above+right.below-5)};
            break;
        case Parentheses: {
            bool small=layouts[a].small;
            int padding=small?8:11;
            box=(MathBox){left.width+2*padding,left.above+2,left.below+2};
            break;
        }
        default:
            box = (MathBox){left.width+right.width,max(left.above,right.above),max(left.below,right.below)};
            break;
    }
    return item((Layout){.kind=kind,.a=a,.b=b,.box=box,.small=layouts[a].small});
}

static const char *closing(const char *first,const char *end) {
    int depth = 0;
    for (const char *p=first;p<end;p++) {
        if (*p=='(' || *p=='{') depth++;
        if ((*p==')' || *p=='}') && --depth==0) return p;
    }
    return NULL;
}

static bool unary_mark(const char *start,const char *p,const char *end) {
    if (p==start) return true;
    const char *prev = p-1;
    while (prev>start && *prev==' ') prev--;
    if (*prev == 'e' || *prev == 'E')
        return prev>start && (isdigit((unsigned char)prev[-1]) || prev[-1]=='.') &&
               p+1<end && isdigit((unsigned char)p[1]);
    return strchr("+-*/^=(,",*prev)!=NULL;
}

static int parse_math(const char *start,const char *end,bool small,int depth);

static int fraction_term(const char *start,const char *end,bool small,int depth) {
    while (start<end && *start==' ') start++;
    while (start<end && end[-1]==' ') end--;
    if (start<end && (*start=='(' || *start=='{') && closing(start,end)==end-1) { start++; end--; }
    return parse_math(start,end,small,depth);
}

static int parse_span(const char *start,const char *end,bool small,int depth) {
    while (start<end && *start==' ') start++;
    while (start<end && end[-1]==' ') end--;
    if (overflow || depth>24) return leaf(start,(int)(end-start),0,small);
    if(start==end) return leaf(start,0,0,small);
    if(end-start==1 && *start=='_') return leaf(start,1,27,small);
    const char *level1 = NULL, *level2 = NULL, *power = NULL;
    int nesting = 0;
    for (const char *p=start;p<end;p++) {
        if (end-p >= 3 && !memcmp(p, "+/-", 3)) { p += 2; continue; }
        if (*p=='{' || *p=='[') { nesting++; continue; }
        if (*p=='}' || *p==']') { nesting--; continue; }
        if (*p=='(') {
            if(p>start && (isdigit((unsigned char)p[-1]) || p[-1]==')' || p[-1]=='}')) level2=p;
            nesting++; continue;
        }
        if(p>start && isalpha((unsigned char)*p) &&
           (isdigit((unsigned char)p[-1]) || p[-1]==')' || p[-1]=='}') &&
           !((*p=='e'||*p=='E') && p+1<end && (isdigit((unsigned char)p[1])||p[1]=='+'||p[1]=='-')))
            level2=p;
        if (*p==')') { nesting--; continue; }
        if (nesting) continue;
        if ((*p=='+' || *p=='-') && !unary_mark(start,p,end)) {
            if (!level1 || (*level1!='=' && *level1!=MathApprox)) level1=p;
        } else if (*p=='=' || *p==MathApprox || *p==',') level1=p;
        else if (*p=='*' || *p=='/' || *p==28) level2=p;
        else if (*p=='^' && !power && !level1 && !level2) power=p;
    }
    if (level1 || level2 || power) {
        const char *op = level1 ? level1 : level2 ? level2 : power;
        if (*op=='(' || isalpha((unsigned char)*op)) {
            return join(Adjacent,parse_math(start,op,small,depth+1),parse_math(op,end,small,depth+1));
        }
        if (*op=='/') {
            bool compact=small || fraction_level>0;
            fraction_level++;
            int a = fraction_term(start,op,compact,depth+1);
            int b = fraction_term(op+1,end,compact,depth+1);
            fraction_level--;
            return join(Fraction,a,b);
        }
        if (*op=='^') {
            int a = parse_math(start,op,small,depth+1);
            int b = fraction_term(op+1,end,true,depth+1);
            return join(Raised,a,b);
        }
        int a = parse_math(start,op,small,depth+1);
        int b = parse_math(op+1,end,small,depth+1);
        if (*op=='*' && ((end-op>5 && !strncmp(op+1,"sqrt(",5)) ||
                         (end-op==3 && !strncmp(op+1,"pi",2)))) return join(Adjacent,a,b);
        int symbol = leaf(NULL,0,*op=='*'?MathMultiply:*op,small);
        return join(Adjacent,join(Adjacent,a,symbol),b);
    }
    if (end-start >= 3 && !memcmp(start, "+/-", 3)) {
        int sign = leaf(start,3,0,small);
        return join(Adjacent,sign,parse_math(start+3,end,small,depth+1));
    }
    for (const char *p=start;p<end;p++) {
        if (*p!='(' && *p!='{') continue;
        const char *close=closing(p,end);
        bool complete=close!=NULL;
        if(!close) close=end;
        bool root=p-start>=4 && (!strncmp(p-4,"sqrt",4)||!strncmp(p-4,"cbrt",4));
        bool cube=root && !strncmp(p-4,"cbrt",4);
        bool nth=p-start>=4 && !strncmp(p-4,"root",4);
        bool logbase=p-start==4 && !strncmp(start,"logb",4);
        bool exponential=p-start==3 && !strncmp(start,"exp",3);
        const char *prefix_end=(root||nth)?p-4:p;
        const char *comma=NULL;
        if(nth||logbase) {
            int level=0;
            for(const char *q=p+1;q<close;q++) {
                if(*q=='('||*q=='{')level++;
                if(*q==')'||*q=='}')level--;
                if(*q==',' && !level) {comma=q;break;}
            }
        }
        int child=parse_math(p+1,comma?comma:close,small,depth+1);
        int index=cube?leaf("3",1,0,true):comma?parse_math(comma+1,close,true,depth+1):0;
        int group=join(root||nth?Radical:Parentheses,child,root||nth?index:0);
        if(logbase && index) {
            int label=join(Lowered,leaf(start,3,0,small),index);
            group=join(Adjacent,label,group);prefix_end=start;
        }
        if(exponential) {
            group=join(Raised,leaf(start,1,'e',small),parse_math(p+1,close,true,depth+1));
            prefix_end=start;
        }
        if (*p=='{' && !root && !nth) group=child;
        if (prefix_end>start) {
            int prefix;
            int length=(int)(prefix_end-start);
            bool inverse=(length==4 && (!strncmp(start,"asin",4)||!strncmp(start,"acos",4)||!strncmp(start,"atan",4))) ||
                (length==5 && (!strncmp(start,"asinh",5)||!strncmp(start,"acosh",5)||!strncmp(start,"atanh",5)));
            if(inverse) prefix=join(Raised,leaf(start+1,length-1,0,small),leaf("-1",2,0,true));
            else prefix=parse_math(start,prefix_end,small,depth+1);
            group=join(Adjacent,prefix,group);
        }
        if(complete && close+1<end) group=join(Adjacent,group,parse_math(close+1,end,small,depth+1));
        return group;
    }
    for (const char *p=start+1;p+2<end;p++) {
        if (*p != '_' || p[1] != '{') continue;
        const char *close = memchr(p+2, '}', (size_t)(end-p-2));
        if (!close) break;
        int base = parse_math(start,p,small,depth+1);
        int sub = leaf(p+2,(int)(close-p-2),0,true);
        int value = join(Lowered,base,sub);
        if (close+1<end) value=join(Adjacent,value,parse_math(close+1,end,small,depth+1));
        return value;
    }
    for (const char *p=start;p+1<end;p++) {
        if (p[0]!='p' || p[1]!='i') continue;
        if (p>start && isalpha((unsigned char)p[-1])) continue;
        if (p+2<end && isalpha((unsigned char)p[2])) continue;
        int value = leaf(NULL,0,MathPi,small);
        if (p>start) value=join(Adjacent,parse_math(start,p,small,depth+1),value);
        if (p+2<end) value=join(Adjacent,value,parse_math(p+2,end,small,depth+1));
        return value;
    }
    return leaf(start,(int)(end-start),0,small);
}

static int parse_math(const char *start,const char *end,bool small,int depth) {
    int result = parse_span(start,end,small,depth);
    if (result) {
        layouts[result].start = start;
        layouts[result].end = end;
    }
    return result;
}

static int prepare(const char *text,int width,bool big) {
    count=0;fraction_level=0;large=big;overflow=false;
    int root=parse_math(text,text+strlen(text),false,0);
    if(root && !overflow && layouts[root].box.above+layouts[root].box.below>80) {
        count=0;fraction_level=0;overflow=false;
        root=parse_math(text,text+strlen(text),true,0);
    }
    if(overflow || !root) {
        count=0;
        int length=(int)strlen(text);
        if(length>LayoutChars) length=LayoutChars;
        root=leaf(text,length,0,false);
    }
    (void)width;
    return root;
}

MathBox measure_math(const char *text,int width) {
    return layouts[prepare(text,width,false)].box;
}

MathBox measure_calculation(const char *text,int width) {
    return layouts[prepare(text,width,true)].box;
}

static void paint(int id,int x,int mid,const MathCanvas *c,int depth) {
    if (!id) return;
    const Layout *l=&layouts[id], *a=&layouts[l->a], *b=&layouts[l->b];
    if (cursor && l->start && l->end && cursor >= l->start && cursor <= l->end && depth >= caret_depth) {
        int offset = cursor == l->end ? l->box.width : 0;
        bool match = cursor == l->start || cursor == l->end;
        if (l->kind == Text && l->text && cursor >= l->text && cursor <= l->text + l->length && l->single!=27) {
            offset = casio_width(l->text, (int)(cursor - l->text), l->small, false);
            match = true;
        }
        if (match) {
            caret = (MathCaret){x+offset,mid-l->box.above,l->box.above+l->box.below};
            caret_depth = depth;
        }
    }
    switch (l->kind) {
        case Text: {
            char text[LayoutChars+1];
            if (l->single) { text[0]=l->single; text[1]=0; }
            else {
                int length=l->length<LayoutChars?l->length:LayoutChars;
                memcpy(text,l->text,(size_t)length);
                text[length]=0;
            }
            if(l->single==27) {
                int w=l->small?8:12,h=l->small?12:18,top=mid-h/2;
                c->rect(x+1,top,w,1,c->ink);c->rect(x+1,top+h-1,w,1,c->ink);
                c->rect(x+1,top,1,h,c->ink);c->rect(x+w,top,1,h,c->ink);
            } else c->text(x,mid-l->box.above,text,c->ink,c->paper,l->small);
            break;
        }
        case Adjacent:
            paint(l->a,x,mid,c,depth+1);
            paint(l->b,x+a->box.width,mid,c,depth+1);
            break;
        case Fraction:
            c->rect(x,mid,l->box.width,2,c->ink);
            paint(l->a,x+(l->box.width-a->box.width)/2,mid-a->box.below-3,c,depth+1);
            paint(l->b,x+(l->box.width-b->box.width)/2,mid+b->box.above+4,c,depth+1);
            break;
        case Raised:
            paint(l->a,x,mid,c,depth+1);
            paint(l->b,x+a->box.width+1,mid-a->box.above-b->box.below+4,c,depth+1);
            break;
        case Lowered:
            paint(l->a,x,mid,c,depth+1);
            paint(l->b,x+a->box.width+1,mid+a->box.below+b->box.above-5,c,depth+1);
            break;
        case Radical: {
            int prefix=l->small?12:16;
            int top=mid-a->box.above-4,bottom=mid+a->box.below;
            int tick=mid+2;
            // Joined tick, descending leg, rising stem and continuous overbar.
            for(int y=tick;y<=bottom;y++) {
                int xx=x+2+(y-tick)*4/max(bottom-tick,1);
                c->rect(xx,y,2,1,c->ink);
            }
            for(int y=top;y<=bottom;y++) {
                int xx=x+6+(bottom-y)*(prefix-10)/max(bottom-top,1);
                c->rect(xx,y,2,1,c->ink);
            }
            c->rect(x+prefix-4,top,l->box.width-prefix+3,2,c->ink);
            if(l->b) paint(l->b,x,top+4-b->box.below,c,depth+1);
            paint(l->a,x+prefix,mid,c,depth+1);
            break;
        }
        case Parentheses: {
            int padding=l->small?8:11;
            paint(l->a,x+padding,mid,c,depth+1);
            int top=mid-l->box.above,bottom=mid+l->box.below-1;
            int bend=l->small?3:4;
            int close_x=x+l->box.width-padding+2;
            for(int y=top;y<=bottom;y++) {
                int distance=y-top<bottom-y?y-top:bottom-y;
                int half=(bottom-top)/2;
                int edge=half-distance;
                int curve=bend*edge*edge/max(half*half,1);
                c->rect(x+2+curve,y,2,1,c->ink);
                c->rect(close_x+bend-curve,y,2,1,c->ink);
            }
            break;
        }
    }
}

void paint_math(const char *text,int width,int x,int mid,const MathCanvas *canvas) {
    paint(prepare(text,width,false),x,mid,canvas,0);
}

void paint_calculation(const char *text,int width,int x,int mid,const MathCanvas *canvas) {
    paint(prepare(text,width,true),x,mid,canvas,0);
}

MathCaret paint_math_input(const char *text,int width,int x,int mid,const MathCanvas *canvas,
                           int position,bool big) {
    int id = prepare(text,width,big);
    MathBox box = layouts[id].box;
    caret = (MathCaret){x+box.width,mid-box.above,box.above+box.below};
    caret_depth = -1;
    cursor = text+position;
    paint(id,x,mid,canvas,0);
    cursor = NULL;
    return caret;
}
