#include "calculator.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static int checks;
static void value(const char *expression,double expected) {
    calculator_clear(); calculator_insert(expression);
    if(!calculator_evaluate()) { fprintf(stderr,"FAILED evaluation: %s\n",expression); assert(0); }
    if(fabs(calc.ans-expected)>1e-10*fmax(1,fabs(expected))) {
        fprintf(stderr,"FAILED %s: %.17g expected %.17g\n",expression,calc.ans,expected); assert(0);
    }
    checks++;
}
static void invalid(const char *expression) {
    calculator_clear(); calculator_insert(expression); assert(!calculator_evaluate()); assert(calc.error); checks++;
}
int main(void) {
    calculator_init();
    value("2+3*4",14); value("(2+3)*4",20); value("2(3+4)",14);
    value("-2^2",-4); value("(-2)^2",4); value("2^3^2",512); value("2^-3",.125);
    value("1/3+1/6",.5); assert(!strcmp(calc.history[calc.count-1].exact,"1/2"));
    value("sqrt(8)",sqrt(8)); assert(!strcmp(calc.history[calc.count-1].exact,"2*sqrt(2)"));
    value("sin(30)",.5); value("cos(60)",.5); value("tan(45)",1);
    value("asin(0.5)",30); value("acos(0.5)",60); value("atan(1)",45);
    calc.angle=1; value("sin(pi/6)",.5); value("asin(1)",acos(-1)/2);
    calc.angle=2; value("sin(100)",1); value("acos(0)",100);
    calc.angle=0;
    value("ln(e)",1); value("log(1000)",3); value("logb(8,2)",3); value("exp(2)",exp(2));
    value("5!/2!",60); value("200*15%",30); value("ncr(10,3)",120); value("npr(5,3)",60);
    value("gcd(84,30)",6); value("lcm(12,18)",36); value("mod(17,5)",2);
    value("min(2,3)",2); value("max(2,3)",3); value("abs(-8)",8);
    value("cbrt(-27)",-3); value("root(-32,5)",-2); value("root(16,4)",2);
    value("floor(-1.2)",-2); value("ceil(-1.2)",-1); value("round(2.6)",3);
    value("sinh(1)",sinh(1)); value("cosh(1)",cosh(1)); value("tanh(1)",tanh(1));
    value("asinh(1)",asinh(1)); value("acosh(2)",acosh(2)); value("atanh(0.5)",atanh(.5));
    value("norm(0)",.5); value("binom(5,2,0.5)",.3125);
    value("2*(3+4",14); // Auto-complete missing closing parentheses.
    value("1/3",1./3); value("Ans*3",1); assert(!strcmp(calc.history[calc.count-1].exact,"1"));
    calculator_store(0); value("A+2",3);
    invalid("1/0"); invalid("sqrt(-1)"); invalid("tan(90)"); invalid("logb(8,1)");
    invalid("(-3)!"); invalid("171!"); invalid("2++*"); invalid(")("); invalid("ncr(2,4)"); invalid("B+1");
    value("25",25); calculator_insert("+"); calculator_insert("5"); assert(!strcmp(calc.input,"Ans+5")); assert(calculator_evaluate() && calc.ans==30);
    calculator_insert("7"); assert(!strcmp(calc.input,"7"));
    calculator_insert("+1"); calc.cursor=1; calculator_insert("0"); assert(!strcmp(calc.input,"70+1"));
    calculator_delete(); assert(!strcmp(calc.input,"7+1"));
    calculator_history(1); assert(!strcmp(calc.input,calc.history[calc.count-1].expression));
    calculator_history(-1); assert(!calc.input[0]);
    value("1/3",1./3); char out[RESULT_CAP]; calculator_answer(out); assert(!strcmp(out,"1/3"));
    calc.decimal=true; calculator_answer(out); assert(!strcmp(out,"0.3333333333"));
    calc.format=1; calc.precision=3; calculator_answer(out); assert(!strcmp(out,"0.333"));
    calc.format=2; calculator_answer(out); assert(!strcmp(out,"3.333*10^(-1)"));
    calc.format=0; calc.decimal=false;
    calculator_clear(); calculator_fraction(); assert(!strcmp(calc.input,"{_}/{_}") && calc.cursor==1);
    calculator_type("1"); assert(calculator_move(2)); calculator_type("3");
    assert(!strcmp(calc.input,"{1}/{3}")); assert(calculator_evaluate() && calc.ans==1./3);
    calculator_clear();calculator_type("1");calculator_fraction();calculator_type("2");
    assert(calculator_move(1));calculator_type("+");calculator_type("3");
    assert(calculator_evaluate() && calc.ans==3.5);
    calculator_clear();calculator_fraction();calculator_fraction();calculator_type("1");
    calculator_move(2);calculator_type("2");calculator_move(1);calculator_move(2);calculator_type("3");
    assert(!strcmp(calc.input,"{{1}/{2}}/{3}"));assert(calculator_evaluate() && calc.ans==1./6);
    calculator_clear();calculator_fraction();calculator_delete();assert(!calc.input[0]);
    calculator_clear();calculator_type("sqrt(");calculator_move(-1);assert(calc.cursor==0);calculator_move(1);assert(calc.cursor==5);
    calculator_clear();calculator_type("sqrt(");assert(!strcmp(calc.input,"sqrt(_)") && calc.cursor==5);
    calculator_fraction();calculator_type("1");calculator_move(2);calculator_type("2");calculator_move(1);calculator_type(")");
    assert(calculator_evaluate() && fabs(calc.ans-sqrt(.5))<1e-12);
    calculator_clear();calculator_type("2");calculator_type("^(");
    assert(!strcmp(calc.input,"2^(_)") && calc.cursor==3);calculator_type("-");calculator_type("3");
    assert(calculator_move(2) && calc.cursor==6);assert(calculator_evaluate() && calc.ans==.125);
    assert(calculator_move(-2) && calc.cursor==3);
    calculator_clear();calculator_type("2");calculator_type("^(");calculator_fraction();
    calculator_type("1");calculator_move(2);calculator_type("2");calculator_move(1);calculator_type(")");
    assert(calculator_evaluate() && fabs(calc.ans-sqrt(2))<1e-12);
    calculator_clear();calculator_fraction();calculator_type("1");calculator_move(2);calculator_type("2");calculator_move(1);calculator_type("^2");
    assert(!strcmp(calc.input,"({1}/{2})^2"));assert(calculator_evaluate() && calc.ans==.25);
    calculator_clear();calculator_fraction();calculator_type("3");calculator_move(2);calculator_type("2");calculator_move(1);calculator_type("!");
    assert(!calculator_evaluate() && calc.error);
    calculator_clear();calculator_type("2");calculator_type("^(");calculator_delete();assert(!strcmp(calc.input,"2"));
    calculator_clear();calculator_type("logb(");calculator_type("8");calculator_type(",");calculator_type("2");
    assert(calculator_evaluate() && calc.ans==3);
    value("6",6);calculator_store(0);calculator_clear();calculator_fraction();calculator_recall(0);calculator_move(2);calculator_type("3");
    assert(calculator_evaluate() && calc.ans==2);
    calculator_clear();calculator_fraction();assert(!calculator_evaluate() && calc.error);
    value("1/3",1./3);
    calc.contrast=3; assert(calculator_save()); int count=calc.count;
    calculator_init(); calculator_load(); assert(calc.count==count && calc.result && calc.contrast==3); assert(calc.ans==1./3);
    value("42",42); assert(calculator_save());
    FILE *f=fopen("lcdmath.dat","wb"); assert(f); fputs("broken",f); fclose(f);
    calculator_init(); calculator_load(); assert(calc.count==count && calc.ans==1./3); // Backup recovery.
    assert(calculator_save()); calculator_init(); calculator_load(); assert(calc.ans==1./3);
    unlink("lcdmath.dat"); unlink("lcdmath.bak"); unlink("lcdmath.tmp");
    printf("%d numerical/domain checks, exact forms, editing, memory, history, formats and save recovery passed.\n",checks);
}
