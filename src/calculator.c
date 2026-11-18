#include "calculator.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

Calculator calc;
static const char memory_names[]="ABCDEFGHIJKLMNOPQRSTUVWXYZr\026";
const char *calculator_memory_name(int index) {
 static char names[MEMORY_COUNT][2];
 if(index<0||index>=MEMORY_COUNT)return "?";
 names[index][0]=memory_names[index];return names[index];
}
static void fail(const char *message) {
    calc.error=true;
    snprintf(calc.message,sizeof(calc.message),"%s",message);
}
void calculator_init(void) { memset(&calc,0,sizeof(calc)); calc.browse=-1; calc.precision=6; calc.contrast=7;calc.selection_start=calc.selection_end=-1; }
void calculator_clear(void) {
    calc.input[0]=0; calc.cursor=0;calc.approximate_result=false;calc.insert_capture=false;calc.selection_start=calc.selection_end=-1; calc.result=false; calc.error=false; calc.browse=-1;
}
void calculator_insert(const char *text) {
    if(calc.result) {
        bool continuation=strchr("+-*/^!%\034\025\027\030",text[0])!=NULL;
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
    if(!calc.result && strchr("^!%\025\030",text[0]) && calc.cursor>0 && calc.input[calc.cursor-1]=='}') {
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
    if(calc.insert_capture && !calc.result) {
        calc.insert_capture=false;
        size_t length=strlen(text);
        if(length && text[length-1]=='(' && calc.input[calc.cursor]) {
            int end=calc.cursor,depth=0;
            while(calc.input[end]) {
                char ch=calc.input[end];
                if(ch=='('||ch=='{')depth++;
                if(ch==')'||ch=='}'){if(!depth)break;depth--;}
                if(!depth && end>calc.cursor && strchr("+-*,\034",ch))break;
                end++;
            }
            char wrapped[INPUT_CAP];
            if(length+(size_t)(end-calc.cursor)+2>=sizeof(wrapped)){fail("Expression full");return;}
            memcpy(wrapped,text,length);memcpy(wrapped+length,calc.input+calc.cursor,(size_t)(end-calc.cursor));
            wrapped[length+end-calc.cursor]=')';wrapped[length+end-calc.cursor+1]=0;
            int start=calc.cursor;replace_span(start,end,wrapped,start+(int)strlen(wrapped));return;
        }
    }
    if(!calc.result) replace_placeholder();
    size_t length=strlen(text);
    if(length && text[length-1]=='(') {
        int fields=(!strcmp(text,"binom(")||!strcmp(text,"mixed("))?3:
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
void calculator_mixed_fraction(void) {calculator_type("mixed(");}
void calculator_indexed(const char *function) {
 calculator_type(function);
 if(calc.error)return;
 int i=calc.cursor;while(calc.input[i] && calc.input[i]!=',')i++;
 if(calc.input[i]==',')calc.cursor=i+1;
}
static int field_commas(int open,int close,int commas[2]) {
 int depth=0,count=0;
 for(int i=open+1;i<close;i++) {
  if(calc.input[i]=='('||calc.input[i]=='{')depth++;
  else if(calc.input[i]==')'||calc.input[i]=='}')depth--;
  else if(calc.input[i]==',' && !depth && count<2)commas[count++]=i;
 }
 return count;
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
    for(int open=(int)strlen(calc.input)-1;open>=0;open--) if(calc.input[open]=='(') {
        int close=match_close(open),commas[2];if(close<0)continue;
        int count=field_commas(open,close,commas);
        bool indexed=open>=4 && (!memcmp(calc.input+open-4,"root",4)||!memcmp(calc.input+open-4,"logb",4));
        bool mixed=open>=5 && !memcmp(calc.input+open-5,"mixed",5);
        if(indexed && count==1) {
            int value=open+1,base=commas[0]+1;
            if(at>=base && at<=close) {
                if(direction==2 || (direction==1 && (at==close || calc.input[at]=='_'))) {calc.cursor=value;goto moved;}
            } else if(at>=value && at<=commas[0]) {
                if(direction==-2) {calc.cursor=base;goto moved;}
                if(direction==1 && at==commas[0]) {calc.cursor=close+1;goto moved;}
                if(direction==-1 && at==value) {calc.cursor=close;goto moved;}
            }
        }
        if(mixed && count==2 && at>=open+1 && at<=close) {
            int starts[]={open+1,commas[0]+1,commas[1]+1};
            int ends[]={commas[0],commas[1],close};
            int field=at<=commas[0]?0:at<=commas[1]?1:2;
            if(direction==2 && field<2) {calc.cursor=starts[2];goto moved;}
            if(direction==-2 && field==2) {calc.cursor=starts[1];goto moved;}
            if(direction==1 && (at==ends[field] || calc.input[at]=='_')) {calc.cursor=field<2?starts[field+1]:close+1;goto moved;}
        }
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
static bool expand_ans(const char *source,char output[512]) {
    size_t n=0;
    char answer[96];
    if(calc.count && calc.history[calc.count-1].exact[0])
        snprintf(answer,sizeof(answer),"(%s)",calc.history[calc.count-1].exact);
    else snprintf(answer,sizeof(answer),"(%.17g)",calc.ans);
    for(const char *p=source;*p;) {
        const char *part=p; size_t length=1;
        if(!strncmp(p,"Ans",3)) { part=answer; length=strlen(answer); p+=3; } else p++;
        char converted=0;
        if(length==1) {
            if(*part=='{') converted='(';
            else if(*part=='}') converted=')';
            else if(*part==28) converted='/';
            else if(*part==25) converted='-';
            else if(*part==22) converted='t';
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
    calc.error=false;calc.approximate_result=false;
    if(!calc.input[0]) return false;
    CasVariables variables={0};
    for(int i=0;i<MEMORY_COUNT;i++) if(calc.memory_set&(1u<<i)) {
        int index=i<26?26+i:i==26?'r'-'a':'t'-'a';
        variables.value[index]=calc.memory[i]; variables.set[index]=true;
        if(i==24){variables.value['y'-'a']=calc.memory[i];variables.set['y'-'a']=true;}
    }
    cas_set_variables(&variables);
    char expression[512];
    CalculationValue value;
    char source[INPUT_CAP];strcpy(source,calc.input);
    int target=-1;char *arrow=strchr(source,23);
    if(arrow) {
        char *name=arrow+1;while(*name==' ')name++;
        target=*name>='A'&&*name<='Z'?*name-'A':*name=='x'?23:*name=='y'?24:*name=='r'?26:*name==22?27:-1;
        if(target<0){fail("Use value → variable");return false;}
        name++;while(*name==' ')name++;
        if(target<0||*name){fail("Use value → variable");return false;}
        *arrow=0;
    }
    if(strchr(source,'"')){fail("Text is not a numeric expression");return false;}
    bool imaginary=strchr(source,20)!=NULL;
    for(int i=0;source[i];i++)if(source[i]=='i' &&
       (i==0||!((source[i-1]>='a'&&source[i-1]<='z')||(source[i-1]>='A'&&source[i-1]<='Z'))) &&
       !(source[i+1]>='a'&&source[i+1]<='z'))imaginary=true;
    if(imaginary){fail("Complex input is not supported");return false;}
    if(strchr(source,'[')||strchr(source,']')){fail("Matrix input is not supported");return false;}
    if(!expand_ans(source,expression)) { fail("Syntax ERROR"); return false; }
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
    if(target>=0)calculator_store(target);
    calc.result=true; calc.browse=-1;
    return true;
}
void calculator_answer(char output[RESULT_CAP]) {
    if(calc.error) { snprintf(output,RESULT_CAP,"%s",calc.message); return; }
    const Entry *e=calc.count?&calc.history[calc.count-1]:NULL;
    double v=e?e->value:0;
    if(calc.sexagesimal) {
        double a=fabs(v),degrees=floor(a);int minutes=(int)floor((a-degrees)*60);double seconds=((a-degrees)*60-minutes)*60;
        if(seconds>59.9999995){seconds=0;if(++minutes==60){minutes=0;degrees++;}}
        snprintf(output,RESULT_CAP,"%s%.0f°%d′%.6g″",v<0?"-":"",degrees,minutes,seconds);return;
    }
    if(calc.engineering && v!=0) {
        int power=(int)(floor(log10(fabs(v))/3)*3)+calc.engineering_steps;
        double mantissa=v/pow(10,power);
        if(isfinite(mantissa)){snprintf(output,RESULT_CAP,"%.10g*10^(%d)",mantissa,power);return;}
    }
    if(calc.mixed_result && !calc.decimal && !calc.approximate_result && e) {
        int n,d;
        if(sscanf(e->exact,"%d/%d",&n,&d)==2 && d>0 && abs(n)>d && abs(n)%d) {
            snprintf(output,RESULT_CAP,"mixed(%d,%d,%d)",n/d,abs(n)%d,d);return;
        }
    }
    if(!calc.decimal && !calc.approximate_result && !calc.format && e && e->exact[0]) snprintf(output,RESULT_CAP,"%s",e->exact);
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
    if(index<0 || index>=MEMORY_COUNT) return;
    calc.memory[index]=calc.ans; calc.memory_set|=1u<<index;
}
void calculator_recall(int index) {
    if(index<0 || index>=MEMORY_COUNT) return;
    if(!(calc.memory_set&(1u<<index))) { fail("Variable is not stored"); return; }
    char text[2]={memory_names[index],0}; calculator_type(text);
}

// A separate, versioned on-disk structure excludes transient UI flags.
typedef struct {
    uint32_t magic, version;
    int32_t count, angle, format, precision, decimal;
    double ans, memory[MEMORY_COUNT];
    uint32_t memory_set, contrast;
    Entry history[HISTORY_CAP];
    uint32_t checksum;
} Save;
static Save saved;
static uint32_t checksum(const Save *s) {
    const unsigned char *p=(const unsigned char *)s; uint32_t h=2166136261u;
    for(size_t i=0;i<(size_t)((const unsigned char *)&s->checksum-p);i++) h=(h^p[i])*16777619u;
    return h;
}
typedef struct {
    uint32_t magic,version;int32_t count,angle,format,precision,decimal;
    double ans,memory[9];uint16_t memory_set,contrast;Entry history[HISTORY_CAP];uint32_t checksum;
} LegacySave;
static bool read_save(const char *name) {
    union {Save current;LegacySave legacy;} data;
    int fd=open(name,O_RDONLY);if(fd<0)return false;
    size_t total=0;char *bytes=(char *)&data;
    while(total<sizeof(data)){ssize_t n=read(fd,bytes+total,sizeof(data)-total);if(n<=0)break;total+=(size_t)n;}
    close(fd);
    if(total==sizeof(LegacySave) && data.legacy.version==1) {
        LegacySave *old=&data.legacy;uint32_t h=2166136261u;
        for(size_t i=0;i<(size_t)((char *)&old->checksum-(char *)old);i++)h=(h^(unsigned char)bytes[i])*16777619u;
        if(h!=old->checksum)return false;
        memset(&saved,0,sizeof(saved));saved.magic=old->magic;saved.version=2;
        saved.count=old->count;saved.angle=old->angle;saved.format=old->format;saved.precision=old->precision;
        saved.decimal=old->decimal;saved.ans=old->ans;saved.contrast=old->contrast;
        const int indices[]={0,1,2,3,4,5,23,24,25};
        for(int i=0;i<9;i++){saved.memory[indices[i]]=old->memory[i];if(old->memory_set&(1u<<i))saved.memory_set|=1u<<indices[i];}
        memcpy(saved.history,old->history,sizeof(saved.history));saved.checksum=checksum(&saved);
    } else if(total==sizeof(Save) && data.current.version==2) saved=data.current;
    else return false;
    if(saved.magic!=0x4c434431 || saved.checksum!=checksum(&saved) ||
       saved.count<0||saved.count>HISTORY_CAP||saved.angle<0||saved.angle>2||saved.format<0||saved.format>2||
       saved.precision<0||saved.precision>9||saved.contrast>10||!isfinite(saved.ans))return false;
    for(int i=0;i<saved.count;i++)if(!memchr(saved.history[i].expression,0,INPUT_CAP)||
       !memchr(saved.history[i].exact,0,64)||!isfinite(saved.history[i].value))return false;
    for(int i=0;i<MEMORY_COUNT;i++)if(!isfinite(saved.memory[i]))return false;
    return true;
}
void calculator_load(void) {
    if(!read_save("lcdmath.dat") && !read_save("lcdmath.bak")) return;
    calc.count=saved.count; calc.angle=saved.angle; calc.format=saved.format;
    calc.contrast=saved.contrast; calc.precision=saved.precision; calc.decimal=(saved.decimal&1)!=0;calc.mixed_result=(saved.decimal&2)!=0;calc.approximate_result=(saved.decimal&4)!=0;
    calc.ans=saved.ans; calc.memory_set=saved.memory_set;
    memcpy(calc.memory,saved.memory,sizeof(calc.memory)); memcpy(calc.history,saved.history,sizeof(calc.history));
    if(calc.count) { strcpy(calc.input,calc.history[calc.count-1].expression); calc.cursor=(int)strlen(calc.input); calc.result=true; }
}
bool calculator_save(void) {
    memset(&saved,0,sizeof(saved)); saved.magic=0x4c434431; saved.version=2;
    saved.count=calc.count; saved.angle=calc.angle; saved.format=calc.format;
    saved.precision=calc.precision; saved.decimal=(calc.decimal?1:0)|(calc.mixed_result?2:0)|(calc.approximate_result?4:0); saved.ans=calc.ans;
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
