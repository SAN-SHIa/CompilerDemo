#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 创建复合语句节点
ASTNode *create_compound_stmt(ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_COMPOUND;
    node->left = left;
    node->right = right;
    return node;
}

// 修改变量声明节点创建函数，添加类型信息
ASTNode *create_decl(char *type, char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_DECL;
    node->decl.name = _strdup(name);  // 使用 _strdup
    node->decl.var_type = _strdup(type); // 存储类型信息
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 修改带初始化的变量声明，添加类型信息
ASTNode *create_decl_assign(char *type, char *name, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_DECL_ASSIGN;
    node->decl.name = _strdup(name);  // 使用 _strdup
    node->decl.var_type = _strdup(type); // 存储类型信息
    node->left = expr;
    node->right = NULL;
    return node;
}

// 创建赋值语句节点
ASTNode *create_assign(char *name, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_ASSIGN;
    node->assign.name = _strdup(name);  // 使用 _strdup
    node->left = expr;
    node->right = NULL;
    return node;
}

// 创建return语句节点
ASTNode *create_return_stmt(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_RETURN;
    node->left = expr;
    node->right = NULL;
    return node;
}

// 创建if语句节点
ASTNode *create_if(ASTNode *cond, ASTNode *then_stmt, ASTNode *else_stmt) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_IF;
    node->if_stmt.cond = cond;
    node->left = then_stmt;
    node->right = else_stmt;
    return node;
}

// 创建while语句节点
ASTNode *create_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_WHILE;
    node->while_stmt.cond = cond;
    node->left = body;
    node->right = NULL;
    return node;
}

// 创建二元运算节点
ASTNode *create_binop(BinOpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_BINOP;
    node->binop.op = op;
    node->left = left;
    node->right = right;
    return node;
}

// 创建变量节点
ASTNode *create_var(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_VAR;
    node->var.name = _strdup(name);  // 使用 _strdup
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 创建整型常量节点
ASTNode *create_int(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_INT;
    node->integer.value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 创建浮点型常量节点
ASTNode *create_float(float value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_FLOAT;
    node->floating.value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 创建函数定义节点
ASTNode *create_func_def(char *ret_type, char *name, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = FUNC_DEF;
    node->func_def.name = _strdup(name);
    node->func_def.ret_type = _strdup(ret_type);
    node->left = body;
    node->right = NULL;
    return node;
}

// 打印AST的辅助函数
static const char *binop_to_str(BinOpType op) {
    switch(op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_EQ:  return "==";
        case OP_NE:  return "!=";
        case OP_LT:  return "<";
        case OP_GT:  return ">";
        case OP_LE:  return "<=";
        case OP_GE:  return ">=";
        default: return "?";
    }
}

// 递归打印AST
void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    
    // 打印缩进
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch(node->type) {
        case STMT_COMPOUND:
            printf("复合语句\n");
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case STMT_DECL:
            printf("变量声明: %s (%s)\n", node->decl.name, node->decl.var_type);
            break;
        case STMT_DECL_ASSIGN:
            printf("变量声明并赋值: %s (%s) =\n", node->decl.name, node->decl.var_type);
            print_ast(node->left, indent + 1);
            break;
        case STMT_ASSIGN:
            printf("赋值: %s =\n", node->assign.name);
            print_ast(node->left, indent + 1);
            break;
        case STMT_RETURN:
            printf("返回语句:\n");
            print_ast(node->left, indent + 1);
            break;
        case STMT_IF:
            printf("if语句:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("条件:\n");
            print_ast(node->if_stmt.cond, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("then:\n");
            print_ast(node->left, indent + 2);
            if (node->right) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("else:\n");
                print_ast(node->right, indent + 2);
            }
            break;
        case STMT_WHILE:
            printf("while语句:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("条件:\n");
            print_ast(node->while_stmt.cond, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("循环体:\n");
            print_ast(node->left, indent + 2);
            break;
        case EXPR_BINOP:
            printf("二元运算: %s\n", binop_to_str(node->binop.op));
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case EXPR_VAR:
            printf("变量: %s\n", node->var.name);
            break;
        case EXPR_INT:
            printf("整数: %d\n", node->integer.value);
            break;
        case EXPR_FLOAT:
            printf("浮点数: %f\n", node->floating.value);
            break;
        case FUNC_DEF:
            printf("函数定义: %s %s()\n", node->func_def.ret_type, node->func_def.name);
            print_ast(node->left, indent + 1);
            break;
    }
}

// 修改free_ast函数，修复内存释放错误
void free_ast(ASTNode *node) {
    if (!node) return;
    
    switch(node->type) {
        case STMT_DECL:
        case STMT_DECL_ASSIGN:
            free(node->decl.name);
            free(node->decl.var_type);
            break;
        case STMT_ASSIGN:
            free(node->assign.name);  // 修正：使用assign.name而不是decl.name
            break;
        case EXPR_VAR:
            free(node->var.name);     // 修正：使用var.name而不是decl.name
            break;
        case FUNC_DEF:
            free(node->func_def.name);
            free(node->func_def.ret_type);
            break;
        default:
            break;
    }
    
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

// 递归生成DOT文件内容
static void export_ast_to_dot_recursive(ASTNode *node, FILE *fp, int *counter) {
    if (!node) return;
    
    int my_id = (*counter)++;
    
    // 节点类型生成标签
    switch(node->type) {
        case STMT_COMPOUND:
            fprintf(fp, "  node%d [label=\"Compound Statement\"];\n", my_id);
            break;
        case STMT_DECL:
            fprintf(fp, "  node%d [label=\"Variable Declaration: %s (%s)\"];\n", my_id, 
                    node->decl.name, node->decl.var_type);
            break;
        case STMT_DECL_ASSIGN:
            fprintf(fp, "  node%d [label=\"Variable Declaration with Init: %s (%s)\"];\n", my_id, 
                    node->decl.name, node->decl.var_type);
            break;
        case STMT_ASSIGN:
            fprintf(fp, "  node%d [label=\"Assignment: %s\"];\n", my_id, node->assign.name);
            break;
        case STMT_RETURN:
            fprintf(fp, "  node%d [label=\"Return Statement\"];\n", my_id);
            break;
        case STMT_IF:
            fprintf(fp, "  node%d [label=\"If Statement\"];\n", my_id);
            
            // 为条件创建一个中间节点
            int cond_id = (*counter)++;
            fprintf(fp, "  node%d [label=\"Condition\"];\n", cond_id);
            fprintf(fp, "  node%d -> node%d;\n", my_id, cond_id);
            
            // 处理条件表达式
            if (node->if_stmt.cond) {
                int expr_id = *counter;
                export_ast_to_dot_recursive(node->if_stmt.cond, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", cond_id, expr_id);
            }
            
            // 为then分支创建中间节点
            if (node->left) {
                int then_id = (*counter)++;
                fprintf(fp, "  node%d [label=\"Then\"];\n", then_id);
                fprintf(fp, "  node%d -> node%d;\n", my_id, then_id);
                
                int stmt_id = *counter;
                export_ast_to_dot_recursive(node->left, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", then_id, stmt_id);
            }
            
            // 为else分支创建中间节点
            if (node->right) {
                int else_id = (*counter)++;
                fprintf(fp, "  node%d [label=\"Else\"];\n", else_id);
                fprintf(fp, "  node%d -> node%d;\n", my_id, else_id);
                
                int stmt_id = *counter;
                export_ast_to_dot_recursive(node->right, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", else_id, stmt_id);
            }
            return; // 特殊处理已完成，不走下面通用逻辑
            
        case STMT_WHILE:
            fprintf(fp, "  node%d [label=\"While Statement\"];\n", my_id);
            
            // 为条件创建一个中间节点
            int while_cond_id = (*counter)++;
            fprintf(fp, "  node%d [label=\"Condition\"];\n", while_cond_id);
            fprintf(fp, "  node%d -> node%d;\n", my_id, while_cond_id);
            
            // 处理条件表达式
            if (node->while_stmt.cond) {
                int expr_id = *counter;
                export_ast_to_dot_recursive(node->while_stmt.cond, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", while_cond_id, expr_id);
            }
            
            // 为循环体创建中间节点
            if (node->left) {
                int body_id = (*counter)++;
                fprintf(fp, "  node%d [label=\"Body\"];\n", body_id);
                fprintf(fp, "  node%d -> node%d;\n", my_id, body_id);
                
                int stmt_id = *counter;
                export_ast_to_dot_recursive(node->left, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", body_id, stmt_id);
            }
            return; // 特殊处理已完成，不走下面通用逻辑
            
        case EXPR_BINOP:
            fprintf(fp, "  node%d [label=\"Binary Op: %s\"];\n", my_id, binop_to_str(node->binop.op));
            break;
        case EXPR_VAR:
            fprintf(fp, "  node%d [label=\"Variable: %s\"];\n", my_id, node->var.name);
            break;
        case EXPR_INT:
            fprintf(fp, "  node%d [label=\"Integer: %d\"];\n", my_id, node->integer.value);
            break;
        case EXPR_FLOAT:
            fprintf(fp, "  node%d [label=\"Float: %.2f\"];\n", my_id, node->floating.value);
            break;
        case FUNC_DEF:
            fprintf(fp, "  node%d [label=\"Function Definition: %s %s()\"];\n", my_id, node->func_def.ret_type, node->func_def.name);
            break;
    }
    
    // 递归处理左右子树
    if (node->left) {
        int left_id = *counter;
        export_ast_to_dot_recursive(node->left, fp, counter);
        fprintf(fp, "  node%d -> node%d;\n", my_id, left_id);
    }
    
    if (node->right) {
        int right_id = *counter;
        export_ast_to_dot_recursive(node->right, fp, counter);
        fprintf(fp, "  node%d -> node%d;\n", my_id, right_id);
    }
}

// 输出AST到DOT文件
void export_ast_to_dot(ASTNode *node, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("无法创建DOT文件");
        return;
    }
    
    // DOT文件头
    fprintf(fp, "digraph AST {\n");
    fprintf(fp, "  node [shape=box, fontname=\"Arial\", fontsize=10];\n"); 
    fprintf(fp, "  edge [fontname=\"Arial\", fontsize=9];\n");
    fprintf(fp, "  rankdir=TB;\n"); 
    
    // 递归生成节点和边
    int node_counter = 0;
    export_ast_to_dot_recursive(node, fp, &node_counter);
    
    // DOT文件尾
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("AST已导出到文件: %s\n", filename);
}