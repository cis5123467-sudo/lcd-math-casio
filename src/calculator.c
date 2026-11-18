#include "calculator.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

Calculator calc;
static const char memory_names[] = "ABCDEFXYZ";
static void fail(const char *message) {
    calc.error=true;
    snprintf(calc.message,sizeof(calc.message),"%s",message);
}
void calculator_init(void) { memset(&calc,0,sizeof(calc)); calc.browse=-1; calc.precision=6; calc.contrast=7; }
void calculator_clear(void) {
    calc.input[0]=0; calc.cursor=0; calc.result=false; calc.error=false; calc.browse=-1;
}
void calculator_insert(const char *text) {
    if(calc.result) {
        bool continuation=strchr("+-*/^!%\034",text[0])!=NULL;
        calculator_clear();
        if(continuation) { strcpy(calc.input,"Ans"); calc.cursor=3; }
    }
    calc.error=false;
    size_t n=strlen(calc.input), m=strlen(text);
    if(n+m>=INPUT_CAP) { fail("Expression full"); return; }
    memmove(calc.input+calc.cursor+m,calc.input+calc.cursor,n-calc.cursor+1);
    memcpy(calc.input+calc.cursor,text,m); calc.cursor+=(int)m;
    calc.browse=-1;
}

// Curly braces mark the two fields of a fraction; evaluation maps them to
// parentheses, preserving grouping independently of the display layout.
static void replace_span(int start,int end,const char *text,int cursor) {
    char updated[INPUT_CAP];
    size_t tail=strlen(calc.input+end),length=strlen(text);
    if(start+length+tail>=INPUT_CAP) { fail("Expression full"); return; }
    memcpy(updated,calc.input,start); memcpy(updated+start,text,length);
    memcpy(updated+start+length,calc.input+end,tail+1);
    strcpy(calc.input,updated);calc.cursor=cursor;calc.result=false;calc.error=false;calc.browse=-1;
}
static void replace_placeholder(void) {
    if(calc.input[calc.cursor]=='_')
        memmove(calc.input+calc.cursor,calc.input+calc.cursor+1,strlen(calc.input+calc.cursor));
}
static int match_close(int start);
void calculator_type(const char *text) {
    // Closing a function or group moves out of its existing closing bracket.
    if(!calc.result && !strcmp(text,")") && calc.input[calc.cursor]==')') {
        calc.cursor++;calc.error=false;return;
    }
    if(!calc.result && strchr("^!%",text[0]) && calc.cursor>0 && calc.input[calc.cursor-1]=='}') {
        // A postfix operator after leaving a fraction applies to the fraction as a whole.
        for(int i=0;i<calc.cursor;i++) if(calc.input[i]=='{') {
            int num=match_close(i);
            if(num>=0 && calc.input[num+1]=='/' && calc.input[num+2]=='{' && match_close(num+2)==calc.cursor-1) {
                int length=calc.cursor-i;
                if(strlen(calc.input)+2>=INPUT_CAP) {fail("Expression full");return;}
                char grouped[INPUT_CAP];grouped[0]='(';memcpy(grouped+1,calc.input+i,length);grouped[length+1]=')';grouped[length+2]=0;
                replace_span(i,calc.cursor,grouped,calc.cursor+2);break;
            }
        }
    }
    if(!calc.result) replace_placeholder();
    size_t length=strlen(text);
    if(length && text[length-1]=='(') {
        int fields=!strcmp(text,"binom(")?3:
            (!strcmp(text,"ncr(")||!strcmp(text,"npr(")||!strcmp(text,"gcd(")||
             !strcmp(text,"lcm(")||!strcmp(text,"mod(")||!strcmp(text,"min(")||
             !strcmp(text,"max(")||!strcmp(text,"root(")||!strcmp(text,"logb("))?2:1;
        char expanded[64];snprintf(expanded,sizeof(expanded),"%s%s)",text,fields==3?"_,_,_":fields==2?"_,_":"_");
        calculator_insert(expanded);
        if(!calc.error) calc.cursor-=(int)(strlen(expanded)-length);
    } else if(!strcmp(text,",") && calc.input[calc.cursor]==',') calc.cursor++;
    else calculator_insert(text);
}
static int match_close(int start) {
    int depth=0;
    for(int i=start;calc.input[i];i++) {
        if(calc.input[i]=='{' || calc.input[i]=='(') depth++;
        if(calc.input[i]=='}' || calc.input[i]==')') if(--depth==0) return i;
    }
    return -1;
}
void calculator_fraction(void) {
    if(calc.result) { calculator_clear();calculator_insert("Ans"); }
    replace_placeholder();
    int end=calc.cursor,start=end,depth=0;
    while(start>0) {
        char ch=calc.input[start-1];
        if(ch==')'||ch=='}') depth++;
        else if(ch=='('||ch=='{') { if(!depth) break;depth--; }
        else if(!depth && strchr("+-*/,\034",ch)) break;
        start--;
    }
    char numerator[INPUT_CAP],text[INPUT_CAP];
    int n=end-start;
    if(n) { memcpy(numerator,calc.input+start,n);numerator[n]=0; }
    else strcpy(numerator,"_");
    if(strlen(numerator)+6>=sizeof(text)) {fail("Expression full");return;}
    snprintf(text,sizeof(text),"{%s}/{_}",numerator);
    int target=n?start+n+4:start+1;
    replace_span(start,end,text,target);
}
// direction: -1 left, +1 right, -2 up, +2 down. Vertical movement chooses
// the innermost fraction first and keeps the horizontal field offset.
bool calculator_move(int direction) {
    int left[INPUT_CAP],right[INPUT_CAP],count=0;
    for(int i=0;calc.input[i];i++) if(calc.input[i]=='{') {
        int end=match_close(i);
        if(end>=0 && calc.input[end+1]=='/' && calc.input[end+2]=='{') {
            int den=match_close(end+2);
            if(den>=0) {left[count]=i;right[count++]=den;}
        }
    }
    int at=calc.cursor;
    for(int f=count-1;f>=0;f--) {
        int start=left[f],last=right[f],num_end=match_close(start),den_start=num_end+3;
        if(at<start+1 || at>last) continue;
        bool num=at<=num_end,den=at>=den_start;
        int offset=num?at-start-1:at-den_start;
        if(direction==2 && num) {int len=last-den_start;calc.cursor=den_start+(offset<len?offset:len);if(calc.input[den_start]=='_')calc.cursor=den_start;goto moved;}
        if(direction==-2 && den) {int len=num_end-start-1;calc.cursor=start+1+(offset<len?offset:len);if(calc.input[start+1]=='_')calc.cursor=start+1;goto moved;}
        if(direction==1 && (at==num_end || (at==num_end-1 && calc.input[at]=='_'))) {calc.cursor=den_start;goto moved;}
        if(direction==1 && (at==last || (at==last-1 && calc.input[at]=='_'))) {calc.cursor=last+1;goto moved;}
        if(direction==-1 && at==den_start) {calc.cursor=num_end;goto moved;}
        if(direction==-1 && at==start+1) {calc.cursor=start;goto moved;}
    }
    for(int i=1;calc.input[i];i++) if(calc.input[i]=='(' && calc.input[i-1]=='^') {
        int last=match_close(i);
        if(last<0) continue;
        if(direction==2 && at>=i+1 && at<=last) {calc.cursor=last+1;goto moved;}
        if(direction==-2 && (at==i-1 || at==last+1)) {calc.cursor=i+1;goto moved;}
    }
    if(direction==-1 && at>0 && calc.input[at-1]=='(') {
        int start=at-1;
        while(start>0 && calc.input[start-1]>='a' && calc.input[start-1]<='z') start--;
        if(start>0 && calc.input[start-1]=='^') start--;
        calc.cursor=start;goto moved;
    }
    if(direction==1) {
        if(calc.input[at]=='_' && calc.input[at+1]==')') {calc.cursor=at+2;goto moved;}
        int next=at;
        while(calc.input[next]>='a' && calc.input[next]<='z') next++;
        if(next>at && calc.input[next]=='(') {calc.cursor=next+1;goto moved;}
        if(calc.input[at]=='^' && calc.input[at+1]=='(') {calc.cursor=at+2;goto moved;}
    }
    if(direction==1 || direction==-1) {
        int length=(int)strlen(calc.input);
        if(direction==1 && at<length) calc.cursor++;
        if(direction==-1 && at>0) calc.cursor--;
        goto moved;
    }
    return false;
moved:
    calc.result=false;calc.error=false;return true;
}
void calculator_delete(void) {
    calc.result=false; calc.error=false;
    if(calc.cursor>0) {
        int i=calc.cursor;
        if(calc.input[i]=='_' && i>0 && calc.input[i-1]=='{') {
            int num_end=match_close(i-1);
            if(num_end>=0 && calc.input[num_end+1]=='/' && calc.input[num_end+2]=='{') {
                int den_end=match_close(num_end+2);
                if(den_end>=0 && den_end==num_end+4 && calc.input[num_end+3]=='_') {
                    replace_span(i-1,den_end+1,"",i-1);return;
                }
            }
        }
        if(calc.input[i]=='_' && i>0 && calc.input[i-1]=='(') {
            int start=i-1;while(start>0 && ((calc.input[start-1]>='a' && calc.input[start-1]<='z'))) start--;
            if(start>0 && calc.input[start-1]=='^') start--;
            if(calc.input[i+1]==')') {replace_span(start,i+2,"",start);return;}
        }
        if(calc.input[i-1]=='(' || calc.input[i-1]=='{' || calc.input[i-1]=='}') {calculator_move(-1);return;}
        if((calc.input[i]=='}'||calc.input[i]==')'||calc.input[i]==',') && i>=2 &&
           (calc.input[i-2]=='{'||calc.input[i-2]=='('||calc.input[i-2]==',')) {
            replace_span(i-1,i,"_",i-1);return;
        }
        memmove(calc.input+calc.cursor-1,calc.input+calc.cursor,strlen(calc.input+calc.cursor)+1);
        calc.cursor--;
    }
}
void calculator_history(int direction) {
    if(!calc.count) return;
    int next=calc.browse+direction;
    if(next<0) { calculator_clear(); return; }
    if(next>=calc.count) return;
    calc.browse=next;
    strcpy(calc.input,calc.history[calc.count-1-next].expression);
    calc.cursor=(int)strlen(calc.input); calc.result=false; calc.error=false;
}
static bool expand_ans(char output[512]) {
    size_t n=0;
    char answer[96];
    if(calc.count && calc.history[calc.count-1].exact[0])
        snprintf(answer,sizeof(answer),"(%s)",calc.history[calc.count-1].exact);
    else snprintf(answer,sizeof(answer),"(%.17g)",calc.ans);
    for(const char *p=calc.input;*p;) {
        const char *part=p; size_t length=1;
        if(!strncmp(p,"Ans",3)) { part=answer; length=strlen(answer); p+=3; } else p++;
        char converted=0;
        if(length==1) {
            if(*part=='{') converted='(';
            else if(*part=='}') converted=')';
            else if(*part==28) converted='/';
            if(converted) part=&converted;
        }
        if(n+length>=480) return false;
        memcpy(output+n,part,length); n+=length;
    }
    int nesting=0;
    for(size_t i=0;i<n;i++) { if(output[i]=='(') nesting++; if(output[i]==')') nesting--; if(nesting<0) return false; }
    while(nesting-->0 && n<510) output[n++]=')';
    output[n]=0; return true;
}
bool calculator_evaluate(void) {
    calc.error=false;
    if(!calc.input[0]) return false;
    CasVariables variables={0};
    for(int i=0;i<9;i++) if(calc.memory_set&(1<<i)) {
        int index=26+memory_names[i]-'A';
        variables.value[index]=calc.memory[i]; variables.set[index]=true;
    }
    cas_set_variables(&variables);
    char expression[512];
    CalculationValue value;
    if(!expand_ans(expression)) { fail("Syntax ERROR"); return false; }
    cas_set_gradians(calc.angle==2);
    if(!cas_calculation(expression,calc.angle!=1,&value)) {
        fail(value.syntax_valid?"Math ERROR":"Syntax ERROR"); calc.result=false; return false;
    }
    if(calc.count==HISTORY_CAP) { memmove(calc.history,calc.history+1,sizeof(Entry)*(HISTORY_CAP-1)); calc.count--; }
    Entry *entry=&calc.history[calc.count++];
    memset(entry,0,sizeof(*entry));
    strcpy(entry->expression,calc.input);
    snprintf(entry->exact,sizeof(entry->exact),"%s",value.exact);
    entry->value=value.value; calc.ans=value.value;
    calc.result=true; calc.browse=-1;
    return true;
}
void calculator_answer(char output[RESULT_CAP]) {
    if(calc.error) { snprintf(output,RESULT_CAP,"%s",calc.message); return; }
    const Entry *e=calc.count?&calc.history[calc.count-1]:NULL;
    double v=e?e->value:0;
    if(!calc.decimal && !calc.format && e && e->exact[0]) snprintf(output,RESULT_CAP,"%s",e->exact);
    else if(calc.format==1) snprintf(output,RESULT_CAP,"%.*f",calc.precision,v);
    else if(calc.format==2) snprintf(output,RESULT_CAP,"%.*e",calc.precision,v);
    else snprintf(output,RESULT_CAP,"%.10g",v==0?0:v);
    // Make scientific notation a textbook power, without interpreting Euler's e.
    char *exponent=strchr(output,'e');
    if(exponent) {
        int power=atoi(exponent+1); *exponent=0;
        char mantissa[64]; snprintf(mantissa,sizeof(mantissa),"%.55s",output);
        snprintf(output,RESULT_CAP,"%s*10^(%d)",mantissa,power);
    }
}
void calculator_store(int index) {
    if(index<0 || index>=9) return;
    calc.memory[index]=calc.ans; calc.memory_set|=1<<index;
}
void calculator_recall(int index) {
    if(index<0 || index>=9) return;
    if(!(calc.memory_set&(1<<index))) { fail("Variable is not stored"); return; }
    char text[2]={memory_names[index],0}; calculator_type(text);
}

// A separate, versioned on-disk structure excludes transient UI flags.
typedef struct {
    uint32_t magic, version;
    int32_t count, angle, format, precision, decimal;
    double ans, memory[9];
    uint16_t memory_set, contrast;
    Entry history[HISTORY_CAP];
    uint32_t checksum;
} Save;
static Save saved;
static uint32_t checksum(const Save *s) {
    const unsigned char *p=(const unsigned char *)s; uint32_t h=2166136261u;
    for(size_t i=0;i<(size_t)((const unsigned char *)&s->checksum-p);i++) h=(h^p[i])*16777619u;
    return h;
}
static bool read_save(const char *name) {
    int fd=open(name,O_RDONLY); if(fd<0) return false;
    size_t left=sizeof(saved); char *p=(char *)&saved;
    while(left) { ssize_t n=read(fd,p,left); if(n<=0) break; p+=n; left-=n; }
    close(fd);
    if(left || saved.magic!=0x4c434431 || saved.version!=1 || saved.checksum!=checksum(&saved) ||
       saved.count<0 || saved.count>HISTORY_CAP || saved.angle<0 || saved.angle>2 || saved.format<0 || saved.format>2 ||
       saved.precision<0 || saved.precision>9 || saved.contrast>10 || !isfinite(saved.ans)) return false;
    for(int i=0;i<saved.count;i++) if(!memchr(saved.history[i].expression,0,INPUT_CAP) ||
       !memchr(saved.history[i].exact,0,64) || !isfinite(saved.history[i].value)) return false;
    for(int i=0;i<9;i++) if(!isfinite(saved.memory[i])) return false;
    return true;
}
void calculator_load(void) {
    if(!read_save("lcdmath.dat") && !read_save("lcdmath.bak")) return;
    calc.count=saved.count; calc.angle=saved.angle; calc.format=saved.format;
    calc.contrast=saved.contrast; calc.precision=saved.precision; calc.decimal=saved.decimal!=0;
    calc.ans=saved.ans; calc.memory_set=saved.memory_set;
    memcpy(calc.memory,saved.memory,sizeof(calc.memory)); memcpy(calc.history,saved.history,sizeof(calc.history));
    if(calc.count) { strcpy(calc.input,calc.history[calc.count-1].expression); calc.cursor=(int)strlen(calc.input); calc.result=true; }
}
bool calculator_save(void) {
    memset(&saved,0,sizeof(saved)); saved.magic=0x4c434431; saved.version=1;
    saved.count=calc.count; saved.angle=calc.angle; saved.format=calc.format;
    saved.precision=calc.precision; saved.decimal=calc.decimal; saved.ans=calc.ans;
    saved.memory_set=calc.memory_set; saved.contrast=calc.contrast;
    memcpy(saved.memory,calc.memory,sizeof(calc.memory)); memcpy(saved.history,calc.history,sizeof(calc.history));
    saved.checksum=checksum(&saved);
    int fd=open("lcdmath.tmp",O_WRONLY|O_CREAT|O_TRUNC,0666); if(fd<0) return false;
    size_t left=sizeof(saved); const char *p=(const char *)&saved;
    while(left) { ssize_t n=write(fd,p,left); if(n<=0) break; p+=n; left-=n; }
    int status=close(fd);
    if(left || status<0) return false;
    // Do not discard a valid backup when recovering from a damaged primary.
    if(read_save("lcdmath.dat")) { unlink("lcdmath.bak"); if(rename("lcdmath.dat","lcdmath.bak")<0) return false; }
    else unlink("lcdmath.dat");
    return rename("lcdmath.tmp","lcdmath.dat")==0;
}
