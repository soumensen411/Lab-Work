%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX 100

char output[1000];   // postfix expression buffer
int out_pos = 0;

/* stack for evaluation */
double stack[MAX];
int top = -1;

void push(double v) { stack[++top] = v; }
double pop() { return stack[top--]; }

void append(char c) {
    output[out_pos++] = c;
    output[out_pos++] = ' ';
}

/* precedence */
int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

/* operator stack for conversion */
char op_stack[MAX];
int op_top = -1;

void push_op(char c) { op_stack[++op_top] = c; }
char pop_op() { return op_stack[op_top--]; }
char peek_op() { return op_stack[op_top]; }

void process_op(char op) {
    while (op_top != -1 && precedence(peek_op()) >= precedence(op)) {
        append(pop_op());
    }
    push_op(op);
}
%}

%%

[0-9]+(\.[0-9]+)?   {
    for (int i = 0; yytext[i] != '\0'; i++)
        output[out_pos++] = yytext[i];
    output[out_pos++] = ' ';
}

[+\-*/] {
    process_op(yytext[0]);
}

"(" {
    push_op('(');
}

")" {
    while (op_top != -1 && peek_op() != '(')
        append(pop_op());
    pop_op(); // remove '('
}

[\t\r ]+   ;

\n {
    while (op_top != -1)
        append(pop_op());

    output[out_pos] = '\0';

    /* ===== Evaluate postfix ===== */
    char *p = output;
    char temp[50];
    int ti = 0;

    while (*p) {
        if (isdigit(*p) || *p == '.') {
            ti = 0;
            while (*p != ' ') {
                temp[ti++] = *p++;
            }
            temp[ti] = '\0';
            push(atof(temp));
        }
        else if (*p == ' ') {
            p++;
        }
        else {
            double b = pop();
            double a = pop();

            switch (*p) {
                case '+': push(a + b); break;
                case '-': push(a - b); break;
                case '*': push(a * b); break;
                case '/': push(a / b); break;
            }
            p++;
        }
    }

    printf("Result = %lf\n", pop());

    /* reset */
    out_pos = 0;
    op_top = -1;
    top = -1;
}

.   ;

%%

int main() {
    printf("Desk Calculator (Postfix Version)\n");
    yylex();
    return 0;
}
