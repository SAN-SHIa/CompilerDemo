%{
#include "ast.h"
#include "semantic.h"
#include "ir.h"
#include "optimize.h"
#include "codegen.h"
#include "interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

extern FILE *yyin;
extern int yylineno;
extern int yycolumn;
void yyerror(const char *s);
int yylex(void);

ASTNode *root = NULL;
SemanticContext *semantic_context = NULL;
IRGenerator *ir_generator = NULL;
Optimizer *optimizer = NULL;
CodeGenerator *code_generator = NULL;
Interpreter *interpreter = NULL;
%}

%union {
    int num;
    float fnum;
    char *str;
    ASTNode *node;
}

%token INT FLOAT RETURN IF ELSE WHILE PRINTF
%token <num> INTEGER
%token <fnum> FLOATING
%token <str> IDENTIFIER STRING
%token EQ NE '<' '>' LE GE

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/'

%type <node> program stmt stmt_list expr decl assignment if_stmt while_stmt func_def call_stmt arg_list

%%

program : func_def { 
            root = $1; 
            printf("Syntax analysis successful!\n");
            print_ast(root, 0);
            export_ast_to_dot(root, "ast.dot");
            
            // system("dot -Tpng -Gcharset=latin1 ast.dot -o ast.png"); 
            printf("AST DOT file generated: ast.dot\n");
            
            semantic_context = init_semantic();
            if (semantic_context) {
                bool sem_ok = analyze_semantics(root, semantic_context);
                if (sem_ok) {
                    printf("Semantic analysis passed!\n");
                    
                    printf("\n=== INTERMEDIATE CODE GENERATION ===\n");
                    ir_generator = init_ir_generator(semantic_context->symbol_table);
                    if (ir_generator) {
                        generate_ir(root, ir_generator);
                        print_ir(ir_generator);
                        
                        printf("\n=== CODE OPTIMIZATION ===\n");
                        optimizer = init_optimizer(ir_generator, 2);
                        if (optimizer) {
                            optimize_ir(optimizer);
                            printf("Optimized intermediate code:\n");
                            print_ir(ir_generator);
                            
                            printf("\n=== TARGET CODE GENERATION ===\n");
                            
                            code_generator = init_code_generator(TARGET_PSEUDO, "output.s");
                            if (code_generator) {
                                generate_target_code(ir_generator, code_generator);
                                printf("Pseudo assembly code generated: output.s\n");
                                free_code_generator(code_generator);
                            }
                            
                            code_generator = init_code_generator(TARGET_C_CODE, "output.c");
                            if (code_generator) {
                                generate_target_code(ir_generator, code_generator);
                                printf("C code generated: output.c\n");
                                free_code_generator(code_generator);
                            }
                            
                            code_generator = init_code_generator(TARGET_X86_64, "output_x64.s");
                            if (code_generator) {
                                generate_target_code(ir_generator, code_generator);
                                printf("x86-64 assembly code generated: output_x64.s\n");
                                free_code_generator(code_generator);
                            }
                            
                            // 添加解释器执行
                            printf("\n=== PROGRAM INTERPRETATION ===\n");
                            interpreter = init_interpreter();
                            if (interpreter) {
                                execute_ir(interpreter, ir_generator);
                                free_interpreter(interpreter);
                            }
                            
                            free_optimizer(optimizer);
                        }
                        
                        free_ir_generator(ir_generator);
                    }
                } else {
                    printf("Semantic analysis failed!\n");
                }
            }
          }

func_def : INT IDENTIFIER '(' ')' '{' stmt_list '}' {
            $$ = create_func_def("int", $2, $6);
          }

stmt_list : stmt_list stmt { $$ = create_compound_stmt($1, $2); }
          | stmt          { $$ = $1; }

stmt : decl ';'         { $$ = $1; }
     | assignment ';'   { $$ = $1; }
     | expr ';'         { $$ = $1; }
     | if_stmt          { $$ = $1; }
     | while_stmt       { $$ = $1; }
     | call_stmt ';'    { $$ = $1; }
     | RETURN expr ';'  { $$ = create_return_stmt($2); }
     | '{' stmt_list '}' { $$ = $2; }

decl : INT IDENTIFIER      { $$ = create_decl("int", $2); }
     | INT IDENTIFIER '=' expr { $$ = create_decl_assign("int", $2, $4); }
     | FLOAT IDENTIFIER    { $$ = create_decl("float", $2); }
     | FLOAT IDENTIFIER '=' expr { $$ = create_decl_assign("float", $2, $4); }

assignment : IDENTIFIER '=' expr { $$ = create_assign($1, $3); }

if_stmt : IF '(' expr ')' stmt %prec LOWER_THAN_ELSE { $$ = create_if($3, $5, NULL); }
        | IF '(' expr ')' stmt ELSE stmt { $$ = create_if($3, $5, $7); }

while_stmt : WHILE '(' expr ')' stmt { $$ = create_while($3, $5); }

expr : expr '+' expr  { $$ = create_binop(OP_ADD, $1, $3); }
     | expr '-' expr  { $$ = create_binop(OP_SUB, $1, $3); }
     | expr '*' expr  { $$ = create_binop(OP_MUL, $1, $3); }
     | expr '/' expr  { $$ = create_binop(OP_DIV, $1, $3); }
     | expr EQ expr   { $$ = create_binop(OP_EQ, $1, $3); }
     | expr NE expr   { $$ = create_binop(OP_NE, $1, $3); }
     | expr '<' expr  { $$ = create_binop(OP_LT, $1, $3); }
     | expr '>' expr  { $$ = create_binop(OP_GT, $1, $3); }
     | expr LE expr   { $$ = create_binop(OP_LE, $1, $3); }
     | expr GE expr   { $$ = create_binop(OP_GE, $1, $3); }
     | IDENTIFIER    { $$ = create_var($1); }
     | INTEGER       { $$ = create_int($1); }
     | FLOATING      { $$ = create_float($1); }
     | '(' expr ')'  { $$ = $2; }

call_stmt : PRINTF '(' arg_list ')' { 
            // 计算参数数量
            int arg_count = 0;
            ASTNode *curr = $3;
            while (curr) {
                arg_count++;
                if (curr->type == STMT_COMPOUND) {
                    curr = curr->right;
                } else {
                    break;
                }
            }
            
            // 创建参数数组
            ASTNode **args = NULL;
            if (arg_count > 0) {
                args = malloc(sizeof(ASTNode*) * arg_count);
                curr = $3;
                for (int i = 0; i < arg_count; i++) {
                    if (curr->type == STMT_COMPOUND) {
                        args[i] = curr->left;
                        curr = curr->right;
                    } else {
                        args[i] = curr;
                        break;
                    }
                }
            }
            
            $$ = create_call("printf", args, arg_count); 
            set_ast_location($$, yylineno, yycolumn);
          }

arg_list : STRING           { $$ = create_var($1); }
         | expr             { $$ = $1; }
         | arg_list ',' STRING { $$ = create_compound_stmt($1, create_var($3)); }
         | arg_list ',' expr { $$ = create_compound_stmt($1, $3); }
         | /* empty */       { $$ = NULL; }

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        fopen_s(&yyin, argv[1], "r");
        if (!yyin) {
            perror("Cannot open file");
            return 1;
        }
    }
    
    printf("=== COMPILER FRONTEND ===\n");
    printf("Start compilation...\n");
    
    yyparse();
    
    if (root) {
        free_ast(root);
    }
    
    if (semantic_context) {
        free_semantic(semantic_context);
    }
    
    if (interpreter) {
        free_interpreter(interpreter);
    }
    
    if (argc > 1) fclose(yyin);
    
    printf("\n=== COMPILATION COMPLETED ===\n");
    return 0;
}
