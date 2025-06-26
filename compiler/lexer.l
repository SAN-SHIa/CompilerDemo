%{
#include "ast.h"        
#include "parser.tab.h"  
#include <string.h>  
#include <stdlib.h>

void yyerror(const char *s);

// 跟踪行号和列号
int yylineno = 1;
int yycolumn = 1;
%}

%option noinput
%option nounput
%option noyywrap  

digit       [0-9]
letter      [a-zA-Z]
id          ({letter}|_)({letter}|{digit}|_)*

%%

"int"       { yycolumn += yyleng; return INT; }
"float"     { yycolumn += yyleng; return FLOAT; }
"return"    { yycolumn += yyleng; return RETURN; }
"if"        { yycolumn += yyleng; return IF; }
"else"      { yycolumn += yyleng; return ELSE; }
"while"     { yycolumn += yyleng; return WHILE; }
"printf"    { yycolumn += yyleng; yylval.str = _strdup(yytext); return PRINTF; }
"{"         { yycolumn += yyleng; return '{'; }
"}"         { yycolumn += yyleng; return '}'; }
"("         { yycolumn += yyleng; return '('; }
")"         { yycolumn += yyleng; return ')'; }
";"         { yycolumn += yyleng; return ';'; }
","         { yycolumn += yyleng; return ','; }
"="         { yycolumn += yyleng; return '='; }
"+"         { yycolumn += yyleng; return '+'; }
"-"         { yycolumn += yyleng; return '-'; }
"*"         { yycolumn += yyleng; return '*'; }
"/"         { yycolumn += yyleng; return '/'; }
"=="        { yycolumn += yyleng; return EQ; }
"!="        { yycolumn += yyleng; return NE; }
"<"         { yycolumn += yyleng; return '<'; }
">"         { yycolumn += yyleng; return '>'; }
"<="        { yycolumn += yyleng; return LE; }
">="        { yycolumn += yyleng; return GE; }
\"[^\"]*\"  { yycolumn += yyleng; yylval.str = _strdup(yytext); return STRING; }
{id}        { yycolumn += yyleng; yylval.str = _strdup(yytext); return IDENTIFIER; }
{digit}+    { yycolumn += yyleng; yylval.num = atoi(yytext); return INTEGER; }
{digit}+"."{digit}* { yycolumn += yyleng; yylval.fnum = atof(yytext); return FLOATING; }
"//".*      { yycolumn += yyleng; } /* 单行注释 */
"/*"([^*]|\*+[^*/])*\*+"/" { yycolumn += yyleng; } /* 多行注释 */
[ \t]       { yycolumn += yyleng; } /* 忽略空白 */
\n          { yylineno++; yycolumn = 1; } /* 换行处理 */
.           { yycolumn += yyleng; yyerror("非法字符"); }

%%