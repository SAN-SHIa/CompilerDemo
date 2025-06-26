#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Set AST node location information
void set_ast_location(ASTNode *node, int line, int column) {
    if (node) {
        node->line_number = line;
        node->column = column;
    }
}

// Create compound statement node
ASTNode *create_compound_stmt(ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_COMPOUND;
    node->left = left;
    node->right = right;
    return node;
}

// Modified variable declaration node creation to include location information
ASTNode *create_decl(char *type, char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_DECL;
    node->decl.name = _strdup(name);  // Use _strdup
    node->decl.var_type = _strdup(type); // Store type information
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Modified variable declaration with initialization to include location information
ASTNode *create_decl_assign(char *type, char *name, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_DECL_ASSIGN;
    node->decl.name = _strdup(name);  // Use _strdup
    node->decl.var_type = _strdup(type); // Store type information
    node->left = expr;
    node->right = NULL;
    return node;
}

// Create assignment node
ASTNode *create_assign(char *name, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_ASSIGN;
    node->assign.name = _strdup(name);  // Use _strdup
    node->left = expr;
    node->right = NULL;
    return node;
}

// Create return statement node
ASTNode *create_return_stmt(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_RETURN;
    node->left = expr;
    node->right = NULL;
    return node;
}

// Create if statement node
ASTNode *create_if(ASTNode *cond, ASTNode *then_stmt, ASTNode *else_stmt) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_IF;
    node->if_stmt.cond = cond;
    node->left = then_stmt;
    node->right = else_stmt;
    return node;
}

// Create while statement node
ASTNode *create_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_WHILE;
    node->while_stmt.cond = cond;
    node->left = body;
    node->right = NULL;
    return node;
}

// Create binary operation node
ASTNode *create_binop(BinOpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_BINOP;
    node->binop.op = op;
    node->left = left;
    node->right = right;
    return node;
}

// Create variable node
ASTNode *create_var(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_VAR;
    node->var.name = _strdup(name);  // ʹ�� _strdup
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Create integer constant node
ASTNode *create_int(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_INT;
    node->integer.value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Create floating point constant node
ASTNode *create_float(float value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_FLOAT;
    node->floating.value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Create function definition node
ASTNode *create_func_def(char *ret_type, char *name, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = FUNC_DEF;
    node->func_def.name = _strdup(name);
    node->func_def.ret_type = _strdup(ret_type);
    node->left = body;
    node->right = NULL;
    return node;
}

// 创建函数调用节点
ASTNode *create_call(char *name, ASTNode **args, int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_CALL;
    node->call.name = _strdup(name);
    node->call.arg_count = arg_count;
    
    if (arg_count > 0 && args) {
        node->call.args = malloc(sizeof(ASTNode*) * arg_count);
        for (int i = 0; i < arg_count; i++) {
            node->call.args[i] = args[i];
        }
    } else {
        node->call.args = NULL;
    }
    
    node->left = NULL;
    node->right = NULL;
    return node;
}

// ��ӡAST�ĸ�������
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

// Recursively print AST
void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    
    // Print indentation
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch(node->type) {
        case STMT_COMPOUND:
            printf("Compound Statement\n");
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case STMT_DECL:
            printf("Variable Declaration: %s (%s)\n", node->decl.name, node->decl.var_type);
            break;
        case STMT_DECL_ASSIGN:
            printf("Variable Declaration with Assignment: %s (%s) =\n", node->decl.name, node->decl.var_type);
            print_ast(node->left, indent + 1);
            break;
        case STMT_ASSIGN:
            printf("Assignment: %s =\n", node->assign.name);
            print_ast(node->left, indent + 1);
            break;
        case STMT_RETURN:
            printf("Return Statement\n");
            print_ast(node->left, indent + 1);
            break;
        case STMT_IF:
            printf("If Statement:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            print_ast(node->if_stmt.cond, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Then:\n");
            print_ast(node->left, indent + 2);
            if (node->right) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("Else:\n");
                print_ast(node->right, indent + 2);
            }
            break;
        case STMT_WHILE:
            printf("While Statement:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Condition:\n");
            print_ast(node->while_stmt.cond, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Body:\n");
            print_ast(node->left, indent + 2);
            break;
        case STMT_CALL:
            printf("Function Call Statement: %s\n", node->call.name);
            for (int i = 0; i < node->call.arg_count; i++) {
                for (int j = 0; j < indent + 1; j++) printf("  ");
                printf("Argument %d:\n", i);
                print_ast(node->call.args[i], indent + 2);
            }
            break;
        case EXPR_BINOP:
            printf("Binary Operation: %s\n", binop_to_str(node->binop.op));
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case EXPR_VAR:
            printf("Variable: %s\n", node->var.name);
            break;
        case EXPR_INT:
            printf("Integer: %d\n", node->integer.value);
            break;
        case EXPR_FLOAT:
            printf("Float: %f\n", node->floating.value);
            break;
        case EXPR_CALL:
            printf("Function Call: %s(", node->call.name);
            for (int i = 0; i < node->call.arg_count; i++) {
                if (i > 0) printf(", ");
                printf("arg%d", i);
            }
            printf(")\n");
            for (int i = 0; i < node->call.arg_count; i++) {
                for (int j = 0; j < indent + 1; j++) printf("  ");
                printf("Argument %d:\n", i);
                print_ast(node->call.args[i], indent + 2);
            }
            break;
        case FUNC_DEF:
            printf("Function Definition: %s %s()\n", node->func_def.ret_type, node->func_def.name);
            print_ast(node->left, indent + 1);
            break;
    }
}

// Modified free_ast function to fix memory release errors
void free_ast(ASTNode *node) {
    if (!node) return;
    
    switch(node->type) {
        case STMT_DECL:
        case STMT_DECL_ASSIGN:
            free(node->decl.name);
            free(node->decl.var_type);
            break;
        case STMT_ASSIGN:
            free(node->assign.name);  // Should use assign.name, not decl.name
            break;
        case STMT_CALL:
        case EXPR_CALL:
            free(node->call.name);
            if (node->call.args) {
                for (int i = 0; i < node->call.arg_count; i++) {
                    free_ast(node->call.args[i]);
                }
                free(node->call.args);
            }
            break;
        case EXPR_VAR:
            free(node->var.name);     // 这里应该使用var.name，不是decl.name
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

// 辅助函数：转义DOT标签中的特殊字符
static char* escape_dot_label(const char* str) {
    if (!str) return NULL;
    
    int len = strlen(str);
    char* escaped = malloc(len * 2 + 1); // 最坏情况下每个字符都需要转义
    int j = 0;
    
    for (int i = 0; i < len; i++) {
        switch (str[i]) {
            case '"':
                escaped[j++] = '\\';
                escaped[j++] = '"';
                break;
            case '\\':
                escaped[j++] = '\\';
                escaped[j++] = '\\';
                break;
            case '\n':
                escaped[j++] = '\\';
                escaped[j++] = 'n';
                break;
            case '\t':
                escaped[j++] = '\\';
                escaped[j++] = 't';
                break;
            default:
                escaped[j++] = str[i];
                break;
        }
    }
    escaped[j] = '\0';
    return escaped;
}

// Recursively export DOT file content
static void export_ast_to_dot_recursive(ASTNode *node, FILE *fp, int *counter) {
    if (!node) return;
    
    int my_id = (*counter)++;
    
    // Node content generates labels
    switch(node->type) {
        case STMT_COMPOUND:
            fprintf(fp, "  node%d [label=\"Compound Statement\"];\n", my_id);
            break;
        case STMT_DECL: {
            char* escaped_name = escape_dot_label(node->decl.name);
            char* escaped_type = escape_dot_label(node->decl.var_type);
            fprintf(fp, "  node%d [label=\"Variable Declaration: %s (%s)\"];\n", my_id, 
                    escaped_name, escaped_type);
            free(escaped_name);
            free(escaped_type);
            break;
        }
        case STMT_DECL_ASSIGN: {
            char* escaped_name = escape_dot_label(node->decl.name);
            char* escaped_type = escape_dot_label(node->decl.var_type);
            fprintf(fp, "  node%d [label=\"Variable Declaration with Init: %s (%s)\"];\n", my_id, 
                    escaped_name, escaped_type);
            free(escaped_name);
            free(escaped_type);
            break;
        }
        case STMT_ASSIGN: {
            char* escaped_name = escape_dot_label(node->assign.name);
            fprintf(fp, "  node%d [label=\"Assignment: %s\"];\n", my_id, escaped_name);
            free(escaped_name);
            break;
        }
        case STMT_RETURN:
            fprintf(fp, "  node%d [label=\"Return Statement\"];\n", my_id);
            break;
        case STMT_IF:
            fprintf(fp, "  node%d [label=\"If Statement\"];\n", my_id);
            
            // Create an intermediate node for the condition
            int cond_id = (*counter)++;
            fprintf(fp, "  node%d [label=\"Condition\"];\n", cond_id);
            fprintf(fp, "  node%d -> node%d;\n", my_id, cond_id);
            
            // Export condition expression
            if (node->if_stmt.cond) {
                int expr_id = *counter;
                export_ast_to_dot_recursive(node->if_stmt.cond, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", cond_id, expr_id);
            }
            
            // Create intermediate node for then branch
            if (node->left) {
                int then_id = (*counter)++;
                fprintf(fp, "  node%d [label=\"Then\"];\n", then_id);
                fprintf(fp, "  node%d -> node%d;\n", my_id, then_id);
                
                int stmt_id = *counter;
                export_ast_to_dot_recursive(node->left, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", then_id, stmt_id);
            }
            
            // Ϊelse��֧�����м�ڵ�
            if (node->right) {
                int else_id = (*counter)++;
                fprintf(fp, "  node%d [label=\"Else\"];\n", else_id);
                fprintf(fp, "  node%d -> node%d;\n", my_id, else_id);
                
                int stmt_id = *counter;
                export_ast_to_dot_recursive(node->right, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", else_id, stmt_id);
            }
            return; // ���⴦������ɣ���������ͨ���߼�
            
        case STMT_WHILE:
            fprintf(fp, "  node%d [label=\"While Statement\"];\n", my_id);
            
            // Ϊ��������һ���м�ڵ�
            int while_cond_id = (*counter)++;
            fprintf(fp, "  node%d [label=\"Condition\"];\n", while_cond_id);
            fprintf(fp, "  node%d -> node%d;\n", my_id, while_cond_id);
            
            // ������������ʽ
            if (node->while_stmt.cond) {
                int expr_id = *counter;
                export_ast_to_dot_recursive(node->while_stmt.cond, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", while_cond_id, expr_id);
            }
            
            // Ϊѭ���崴���м�ڵ�
            if (node->left) {
                int body_id = (*counter)++;
                fprintf(fp, "  node%d [label=\"Body\"];\n", body_id);
                fprintf(fp, "  node%d -> node%d;\n", my_id, body_id);
                
                int stmt_id = *counter;
                export_ast_to_dot_recursive(node->left, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", body_id, stmt_id);
            }
            return; // ���⴦������ɣ���������ͨ���߼�
            
        case EXPR_BINOP:
            fprintf(fp, "  node%d [label=\"Binary Op: %s\"];\n", my_id, binop_to_str(node->binop.op));
            break;
        case EXPR_VAR: {
            char* escaped_name = escape_dot_label(node->var.name);
            fprintf(fp, "  node%d [label=\"Variable: %s\"];\n", my_id, escaped_name);
            free(escaped_name);
            break;
        }
        case EXPR_INT:
            fprintf(fp, "  node%d [label=\"Integer: %d\"];\n", my_id, node->integer.value);
            break;
        case EXPR_FLOAT:
            fprintf(fp, "  node%d [label=\"Float: %.2f\"];\n", my_id, node->floating.value);
            break;
        case FUNC_DEF: {
            char* escaped_ret_type = escape_dot_label(node->func_def.ret_type);
            char* escaped_name = escape_dot_label(node->func_def.name);
            fprintf(fp, "  node%d [label=\"Function Definition: %s %s()\"];\n", my_id, escaped_ret_type, escaped_name);
            free(escaped_ret_type);
            free(escaped_name);
            break;
        }
        case STMT_CALL:
        case EXPR_CALL: {
            char* escaped_name = escape_dot_label(node->call.name);
            fprintf(fp, "  node%d [label=\"Function Call: %s\"];\n", my_id, escaped_name);
            free(escaped_name);
            if (node->call.arg_count > 0) {
                for (int i = 0; i < node->call.arg_count; i++) {
                    fprintf(fp, "  node%d -> node%d;\n", my_id, (*counter));
                    export_ast_to_dot_recursive(node->call.args[i], fp, counter);
                }
            }
            return; // 特殊处理完成，不走通用逻辑
        }
    }
    
    // �ݹ鴦����������
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

// ���AST��DOT�ļ�
void export_ast_to_dot(ASTNode *node, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Cannot create DOT file");
        return;
    }
    
    // DOT file header
    fprintf(fp, "digraph AST {\n");
    fprintf(fp, "  node [shape=box, fontname=\"Arial\", fontsize=10];\n"); 
    fprintf(fp, "  edge [fontname=\"Arial\", fontsize=9];\n");
    fprintf(fp, "  rankdir=TB;\n"); 
    
    // Recursively generate nodes and edges
    int node_counter = 0;
    export_ast_to_dot_recursive(node, fp, &node_counter);
    
    // DOT file footer
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("AST exported to file: %s\n", filename);
}