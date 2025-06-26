#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// �����������ڵ�
ASTNode *create_compound_stmt(ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_COMPOUND;
    node->left = left;
    node->right = right;
    return node;
}

// �޸ı��������ڵ㴴�����������������Ϣ
ASTNode *create_decl(char *type, char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_DECL;
    node->decl.name = _strdup(name);  // ʹ�� _strdup
    node->decl.var_type = _strdup(type); // �洢������Ϣ
    node->left = NULL;
    node->right = NULL;
    return node;
}

// �޸Ĵ���ʼ���ı������������������Ϣ
ASTNode *create_decl_assign(char *type, char *name, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_DECL_ASSIGN;
    node->decl.name = _strdup(name);  // ʹ�� _strdup
    node->decl.var_type = _strdup(type); // �洢������Ϣ
    node->left = expr;
    node->right = NULL;
    return node;
}

// ������ֵ���ڵ�
ASTNode *create_assign(char *name, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_ASSIGN;
    node->assign.name = _strdup(name);  // ʹ�� _strdup
    node->left = expr;
    node->right = NULL;
    return node;
}

// ����return���ڵ�
ASTNode *create_return_stmt(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_RETURN;
    node->left = expr;
    node->right = NULL;
    return node;
}

// ����if���ڵ�
ASTNode *create_if(ASTNode *cond, ASTNode *then_stmt, ASTNode *else_stmt) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_IF;
    node->if_stmt.cond = cond;
    node->left = then_stmt;
    node->right = else_stmt;
    return node;
}

// ����while���ڵ�
ASTNode *create_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = STMT_WHILE;
    node->while_stmt.cond = cond;
    node->left = body;
    node->right = NULL;
    return node;
}

// ������Ԫ����ڵ�
ASTNode *create_binop(BinOpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_BINOP;
    node->binop.op = op;
    node->left = left;
    node->right = right;
    return node;
}

// ���������ڵ�
ASTNode *create_var(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_VAR;
    node->var.name = _strdup(name);  // ʹ�� _strdup
    node->left = NULL;
    node->right = NULL;
    return node;
}

// �������ͳ����ڵ�
ASTNode *create_int(int value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_INT;
    node->integer.value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// ���������ͳ����ڵ�
ASTNode *create_float(float value) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = EXPR_FLOAT;
    node->floating.value = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// ������������ڵ�
ASTNode *create_func_def(char *ret_type, char *name, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = FUNC_DEF;
    node->func_def.name = _strdup(name);
    node->func_def.ret_type = _strdup(ret_type);
    node->left = body;
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

// �ݹ��ӡAST
void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    
    // ��ӡ����
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch(node->type) {
        case STMT_COMPOUND:
            printf("�������\n");
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case STMT_DECL:
            printf("��������: %s (%s)\n", node->decl.name, node->decl.var_type);
            break;
        case STMT_DECL_ASSIGN:
            printf("������������ֵ: %s (%s) =\n", node->decl.name, node->decl.var_type);
            print_ast(node->left, indent + 1);
            break;
        case STMT_ASSIGN:
            printf("��ֵ: %s =\n", node->assign.name);
            print_ast(node->left, indent + 1);
            break;
        case STMT_RETURN:
            printf("�������:\n");
            print_ast(node->left, indent + 1);
            break;
        case STMT_IF:
            printf("if���:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("����:\n");
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
            printf("while���:\n");
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("����:\n");
            print_ast(node->while_stmt.cond, indent + 2);
            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("ѭ����:\n");
            print_ast(node->left, indent + 2);
            break;
        case EXPR_BINOP:
            printf("��Ԫ����: %s\n", binop_to_str(node->binop.op));
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case EXPR_VAR:
            printf("����: %s\n", node->var.name);
            break;
        case EXPR_INT:
            printf("����: %d\n", node->integer.value);
            break;
        case EXPR_FLOAT:
            printf("������: %f\n", node->floating.value);
            break;
        case FUNC_DEF:
            printf("��������: %s %s()\n", node->func_def.ret_type, node->func_def.name);
            print_ast(node->left, indent + 1);
            break;
    }
}

// �޸�free_ast�������޸��ڴ��ͷŴ���
void free_ast(ASTNode *node) {
    if (!node) return;
    
    switch(node->type) {
        case STMT_DECL:
        case STMT_DECL_ASSIGN:
            free(node->decl.name);
            free(node->decl.var_type);
            break;
        case STMT_ASSIGN:
            free(node->assign.name);  // ������ʹ��assign.name������decl.name
            break;
        case EXPR_VAR:
            free(node->var.name);     // ������ʹ��var.name������decl.name
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

// �ݹ�����DOT�ļ�����
static void export_ast_to_dot_recursive(ASTNode *node, FILE *fp, int *counter) {
    if (!node) return;
    
    int my_id = (*counter)++;
    
    // �ڵ��������ɱ�ǩ
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
            
            // Ϊ��������һ���м�ڵ�
            int cond_id = (*counter)++;
            fprintf(fp, "  node%d [label=\"Condition\"];\n", cond_id);
            fprintf(fp, "  node%d -> node%d;\n", my_id, cond_id);
            
            // �����������ʽ
            if (node->if_stmt.cond) {
                int expr_id = *counter;
                export_ast_to_dot_recursive(node->if_stmt.cond, fp, counter);
                fprintf(fp, "  node%d -> node%d;\n", cond_id, expr_id);
            }
            
            // Ϊthen��֧�����м�ڵ�
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
            
            // �����������ʽ
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
        perror("�޷�����DOT�ļ�");
        return;
    }
    
    // DOT�ļ�ͷ
    fprintf(fp, "digraph AST {\n");
    fprintf(fp, "  node [shape=box, fontname=\"Arial\", fontsize=10];\n"); 
    fprintf(fp, "  edge [fontname=\"Arial\", fontsize=9];\n");
    fprintf(fp, "  rankdir=TB;\n"); 
    
    // �ݹ����ɽڵ�ͱ�
    int node_counter = 0;
    export_ast_to_dot_recursive(node, fp, &node_counter);
    
    // DOT�ļ�β
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("AST�ѵ������ļ�: %s\n", filename);
}