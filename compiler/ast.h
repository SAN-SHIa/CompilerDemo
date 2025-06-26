#ifndef AST_H
#define AST_H

typedef enum {
    STMT_COMPOUND,
    STMT_DECL,
    STMT_DECL_ASSIGN,
    STMT_ASSIGN,
    STMT_RETURN,
    STMT_IF,
    STMT_WHILE,
    STMT_CALL,          // 函数调用语句
    EXPR_BINOP,
    EXPR_VAR,
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_CALL,          // 函数调用表达式
    FUNC_DEF            // 函数定义
} NodeType; 

// ��Ԫ���������
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE
} BinOpType;

typedef struct ASTNode {
    NodeType type;
    int line_number;        // 行号
    int column;            // 列号
    struct ASTNode *left;
    struct ASTNode *right;
    union {
        // ��������
        struct { 
            char *name;
            char *var_type;  // ���������ֶ�
        } decl;
        // ��ֵ���
        struct { char *name; } assign;
        // ��Ԫ����
        struct { BinOpType op; } binop;
        // ����
        struct { char *name; } var;
        // ��������
        struct { int value; } integer;
        // ����������
        struct { float value; } floating;
        // if���
        struct { struct ASTNode *cond; } if_stmt;
        // while语句
        struct { struct ASTNode *cond; } while_stmt;
        // 函数定义
        struct { char *name; char *ret_type; } func_def;
        // 函数调用
        struct { 
            char *name; 
            struct ASTNode **args;  // 参数列表
            int arg_count;          // 参数个数
        } call;
    };
} ASTNode;

// AST�ڵ㴴������
ASTNode *create_compound_stmt(ASTNode *left, ASTNode *right);
ASTNode *create_decl(char *type, char *name);
ASTNode *create_decl_assign(char *type, char *name, ASTNode *expr);
ASTNode *create_assign(char *name, ASTNode *expr);
ASTNode *create_return_stmt(ASTNode *expr);
ASTNode *create_if(ASTNode *cond, ASTNode *then_stmt, ASTNode *else_stmt);
ASTNode *create_while(ASTNode *cond, ASTNode *body);
ASTNode *create_binop(BinOpType op, ASTNode *left, ASTNode *right);
ASTNode *create_var(char *name);
ASTNode *create_int(int value);
ASTNode *create_float(float value); 
ASTNode *create_func_def(char *ret_type, char *name, ASTNode *body);
ASTNode *create_call(char *name, ASTNode **args, int arg_count);  // 函数调用创建
void set_ast_location(ASTNode *node, int line, int column);   // 设置位置信息

// AST��������
void print_ast(ASTNode *node, int indent);
void free_ast(ASTNode *node);
void export_ast_to_dot(ASTNode *node, const char *filename); // DOT�ļ�����

#endif