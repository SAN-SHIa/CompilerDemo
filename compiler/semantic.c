#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize semantic analysis context
SemanticContext* init_semantic() {
    SemanticContext *context = (SemanticContext*)malloc(sizeof(SemanticContext));
    if (context) {
        context->symbol_table = init_symbol_table();
        context->current_func_type = TYPE_UNKNOWN;
        context->error_count = 0;
    }
    return context;
}

// Free semantic analysis context
void free_semantic(SemanticContext *context) {
    if (context) {
        free_symbol_table(context->symbol_table);
        free(context);
    }
}

// Report semantic errors
void report_semantic_error(SemanticErrorType error, const char *message) {
    const char *error_type = "";
    
    switch (error) {
        case SEM_UNDECLARED_VAR:
            error_type = "Undeclared variable";
            break;
        case SEM_REDECLARED_VAR:
            error_type = "Variable redeclaration";
            break;
        case SEM_TYPE_MISMATCH:
            error_type = "Type mismatch";
            break;
        case SEM_INVALID_RETURN_TYPE:
            error_type = "Invalid return type";
            break;
        case SEM_DIVISION_BY_ZERO:
            error_type = "Division by zero";
            break;
        case SEM_INVALID_OPERATION:
            error_type = "Invalid operation";
            break;
        case SEM_FUNCTION_NOT_DECLARED:
            error_type = "Function not declared";
            break;
        default:
            error_type = "Unknown error";
    }
    
    fprintf(stderr, "Semantic Error: %s - %s\n", error_type, message);
}

// Report semantic errors with location information
void report_semantic_error_with_location(SemanticErrorType error, const char *message, int line, int column) {
    const char *error_type = "";
    
    switch (error) {
        case SEM_UNDECLARED_VAR:
            error_type = "Undeclared variable";
            break;
        case SEM_REDECLARED_VAR:
            error_type = "Variable redeclaration";
            break;
        case SEM_TYPE_MISMATCH:
            error_type = "Type mismatch";
            break;
        case SEM_INVALID_RETURN_TYPE:
            error_type = "Invalid return type";
            break;
        case SEM_DIVISION_BY_ZERO:
            error_type = "Division by zero";
            break;
        case SEM_INVALID_OPERATION:
            error_type = "Invalid operation";
            break;
        case SEM_FUNCTION_NOT_DECLARED:
            error_type = "Function not declared";
            break;
        default:
            error_type = "Unknown error";
    }
    
    fprintf(stderr, "Semantic Error at line %d, column %d: %s - %s\n", line, column, error_type, message);
}

// Expression type checking
DataType check_expr_type(ASTNode *node, SemanticContext *context) {
    if (!node) return TYPE_UNKNOWN;
    
    switch (node->type) {
        case EXPR_INT:
            return TYPE_INT;
            
        case EXPR_FLOAT:
            return TYPE_FLOAT;
            
        case EXPR_VAR: {
            // 查找变量
            SymbolEntry *entry = lookup_symbol(context->symbol_table, node->var.name);
            if (!entry) {
                char error_msg[100];
                sprintf(error_msg, "'%s'", node->var.name);
                report_semantic_error_with_location(SEM_UNDECLARED_VAR, error_msg, node->line_number, node->column);
                context->error_count++;
                return TYPE_UNKNOWN;
            }
            return entry->type;
        }
        
        case EXPR_BINOP: {
            // Recursively search parameters
            DataType left_type = check_expr_type(node->left, context);
            DataType right_type = check_expr_type(node->right, context);
            
            // ���ͼ����Լ��
            if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
                return TYPE_UNKNOWN;
            }
            
            // Check division by zero
            if (node->binop.op == OP_DIV) {
                if (node->right->type == EXPR_INT && node->right->integer.value == 0) {
                    report_semantic_error_with_location(SEM_DIVISION_BY_ZERO, "Integer division by zero", 
                                                       node->line_number, node->column);
                    context->error_count++;
                    return TYPE_UNKNOWN;
                } else if (node->right->type == EXPR_FLOAT && node->right->floating.value == 0.0) {
                    report_semantic_error_with_location(SEM_DIVISION_BY_ZERO, "Float division by zero", 
                                                       node->line_number, node->column);
                    context->error_count++;
                    return TYPE_UNKNOWN;
                }
            }
            
            // ���Ƚ�����������ͼ�����
            if (node->binop.op >= OP_EQ && node->binop.op <= OP_GE) {
                if (!check_type_compatible(left_type, right_type) && 
                    !check_type_compatible(right_type, left_type)) {
                    char error_msg[200];
                    sprintf(error_msg, "Cannot compare %s and %s types", 
                           data_type_to_str(left_type), data_type_to_str(right_type));
                    report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                    context->error_count++;
                }
                return TYPE_INT; // �Ƚ�������Ϊ�߼�ֵ����int��ʾ
            }
            
            // 返回运算结果类型
            return get_result_type(left_type, right_type);
        }
        
        case EXPR_CALL: {
            // 函数调用表达式
            if (strcmp(node->call.name, "printf") == 0) {
                // printf 函数特殊处理
                if (node->call.arg_count < 1) {
                    report_semantic_error_with_location(SEM_INVALID_OPERATION, 
                                                       "printf requires at least one argument", 
                                                       node->line_number, node->column);
                    context->error_count++;
                    return TYPE_UNKNOWN;
                }
                
                // 检查第一个参数是否为字符串字面量（在这里简化处理）
                printf("INFO: printf function call detected at line %d, column %d\n", 
                       node->line_number, node->column);
                
                // printf 返回 int 类型（打印的字符数）
                return TYPE_INT;
            } else {
                // 其他函数调用
                char error_msg[100];
                sprintf(error_msg, "Function '%s' not declared", node->call.name);
                report_semantic_error_with_location(SEM_FUNCTION_NOT_DECLARED, error_msg, 
                                                   node->line_number, node->column);
                context->error_count++;
                return TYPE_UNKNOWN;
            }
        }
        
        default:
            return TYPE_UNKNOWN;
    }
}

// �ƶϱ�������
DataType infer_var_type(const char *type_name) {
    if (!type_name) return TYPE_UNKNOWN;
    
    if (strcmp(type_name, "int") == 0) {
        return TYPE_INT;
    } else if (strcmp(type_name, "float") == 0) {
        return TYPE_FLOAT;
    } else if (strcmp(type_name, "void") == 0) {
        return TYPE_UNKNOWN; // void������UNKNOWN��ʾ
    }
    
    return TYPE_UNKNOWN;
}

// ������ͼ����ԣ����Ӷ���ʽת����֧�֣�
bool check_type_compatible(DataType target_type, DataType expr_type) {
    // ��ͬ�������Ǽ��ݵ�
    if (target_type == expr_type) return true;
    
    // ֧��һЩ��ȫ����ʽ����ت��
    if (target_type == TYPE_FLOAT && expr_type == TYPE_INT) {
        // int ���԰�ȫ��ت��Ϊ float
        return true;
    }
    
    if (target_type == TYPE_INT && expr_type == TYPE_FLOAT) {
        // float ת��Ϊ int �������ģ������ܵ��¾�����ʧ
        // ���������������������ģ����ܻ��������
        printf("����: �� float ת���� int ���ܵ��¾�����ʧ\n");
        return true;
    }
    
    return false;
}

// ����Ƿ�Ϊ���������
bool is_arithmetic_op(BinOpType op) {
    return (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV);
}

// ����Ƿ�Ϊ��ϵ�����
bool is_relational_op(BinOpType op) {
    return (op >= OP_EQ && op <= OP_GE);
}

// ��鳣������ʽ��ֵ
bool is_zero_constant(ASTNode *node) {
    if (!node) return false;
    
    if (node->type == EXPR_INT) {
        return node->integer.value == 0;
    } else if (node->type == EXPR_FLOAT) {
        return node->floating.value == 0.0;
    }
    
    return false;
}

// ����ת������
void type_conversion_warning(DataType from_type, DataType to_type, const char *context_msg) {
    if (from_type == TYPE_FLOAT && to_type == TYPE_INT) {
        printf("����: %s - �� float ת���� int ���ܵ��¾�����ʧ\n", context_msg);
    }
}

// ������
bool check_stmt(ASTNode *node, SemanticContext *context) {
    if (!node) return true;
    
    switch(node->type) {
        case STMT_COMPOUND: {
            // ��Ϊ������䴴����������ֻ�����������
            bool left_result = check_stmt(node->left, context);
            bool right_result = check_stmt(node->right, context);
            return left_result && right_result;
        }
        
        case STMT_DECL: {
            // ������������
            const char *var_name = node->decl.name;
            const char *type_str = node->decl.var_type;
            
            // ���������ַ���ȷ����������
            DataType var_type = infer_var_type(type_str);
            if (var_type == TYPE_UNKNOWN) {
                char error_msg[200];
                sprintf(error_msg, "δ֪������ '%s'", type_str);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            // ���ұ����Ƿ��Ѵ����ڵ�ǰ������
            if (lookup_symbol_current_scope(context->symbol_table, var_name)) {
                char error_msg[100];
                sprintf(error_msg, "'%s'", var_name);
                report_semantic_error(SEM_REDECLARED_VAR, error_msg);
                context->error_count++;
                return false;
            }
            
            return add_symbol(context->symbol_table, var_name, SYM_VARIABLE, var_type);
        }
        
        case STMT_DECL_ASSIGN: {
            // ��������ʼֵ�ı�������
            const char *var_name = node->decl.name;
            const char *type_str = node->decl.var_type;
            
            // ȷ����������
            DataType var_type = infer_var_type(type_str);
            if (var_type == TYPE_UNKNOWN) {
                char error_msg[200];
                sprintf(error_msg, "δ֪������ '%s'", type_str);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            // ����ʼֵ����ʽ����
            DataType expr_type = check_expr_type(node->left, context);
            
            // ���ұ����Ƿ��Ѵ���
            if (lookup_symbol_current_scope(context->symbol_table, var_name)) {
                char error_msg[100];
                sprintf(error_msg, "'%s'", var_name);
                report_semantic_error(SEM_REDECLARED_VAR, error_msg);
                context->error_count++;
                return false;
            }
            
            // ���ͼ����Լ��
            if (!check_type_compatible(var_type, expr_type)) {
                char error_msg[200];
                sprintf(error_msg, "�޷��� %s ���͸�ֵ�� %s ���ͱ��� '%s'", 
                        data_type_to_str(expr_type), data_type_to_str(var_type), var_name);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            return add_symbol(context->symbol_table, var_name, SYM_VARIABLE, var_type);
        }
        
        case STMT_ASSIGN: {
            // ������ֵ���
            const char *var_name = node->assign.name;
            SymbolEntry *entry = lookup_symbol(context->symbol_table, var_name);
            
            if (!entry) {
                char error_msg[100];
                sprintf(error_msg, "'%s'", var_name);
                report_semantic_error(SEM_UNDECLARED_VAR, error_msg);
                context->error_count++;
                return false;
            }
            
            // ����Ҳ����ʽ����
            DataType expr_type = check_expr_type(node->left, context);
            
            // ���ͼ����Լ��
            if (!check_type_compatible(entry->type, expr_type)) {
                char error_msg[200];
                sprintf(error_msg, "�޷��� %s ���͸�ֵ�� %s ���ͱ��� '%s'", 
                       data_type_to_str(expr_type), data_type_to_str(entry->type), var_name);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            return true;
        }
        
        case STMT_RETURN: {
            // �����������
            DataType expr_type = check_expr_type(node->left, context);
            
            // ��鷵��ֵ�����뺯������һ���ԣ�ʹ����ǿ�����ͼ����Լ��
            if (context->current_func_type != TYPE_UNKNOWN && 
                expr_type != TYPE_UNKNOWN && 
                !check_type_compatible(context->current_func_type, expr_type)) {
                
                char error_msg[100];
                sprintf(error_msg, "������������Ϊ %s��������ֵ����Ϊ %s", 
                       data_type_to_str(context->current_func_type), 
                       data_type_to_str(expr_type));
                report_semantic_error(SEM_INVALID_RETURN_TYPE, error_msg);
                context->error_count++;
                return false;
            }
            
            return true;
        }
        
        case STMT_CALL: {
            // 函数调用语句
            check_expr_type(node, context);  // 复用表达式类型检查中的函数调用逻辑
            return true;
        }
        
        case STMT_IF: {
            // ����if���
            DataType cond_type = check_expr_type(node->if_stmt.cond, context);
            
            // �����������ʽ����
            if (cond_type != TYPE_INT && cond_type != TYPE_FLOAT && cond_type != TYPE_UNKNOWN) {
                report_semantic_error(SEM_TYPE_MISMATCH, "if��������ʽ��������ֵ����");
                context->error_count++;
                return false;
            }
            
            // ���then��֧
            enter_scope(context->symbol_table);
            bool then_result = check_stmt(node->left, context);
            leave_scope(context->symbol_table);
            
            // ���else��֧
            bool else_result = true;
            if (node->right) {
                enter_scope(context->symbol_table);
                else_result = check_stmt(node->right, context);
                leave_scope(context->symbol_table);
            }
            
            return then_result && else_result;
        }
        
        case STMT_WHILE: {
            // ����while���
            DataType cond_type = check_expr_type(node->while_stmt.cond, context);
            
            // �����������ʽ����
            if (cond_type != TYPE_INT && cond_type != TYPE_FLOAT && cond_type != TYPE_UNKNOWN) {
                report_semantic_error(SEM_TYPE_MISMATCH, "while��������ʽ��������ֵ����");
                context->error_count++;
                return false;
            }
            
            // ���ѭ����
            enter_scope(context->symbol_table);
            bool body_result = check_stmt(node->left, context);
            leave_scope(context->symbol_table);
            
            return body_result;
        }
        
        case FUNC_DEF: {
            // ������������
            const char *func_name = node->func_def.name;
            const char *ret_type_str = node->func_def.ret_type;
            
            DataType ret_type = infer_var_type(ret_type_str);
            if (ret_type == TYPE_UNKNOWN) {
                char error_msg[200];
                sprintf(error_msg, "���� '%s' �ķ������� '%s' δ֪", func_name, ret_type_str);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            // ��麯���Ƿ�������
            if (lookup_symbol_current_scope(context->symbol_table, func_name)) {
                char error_msg[200];
                sprintf(error_msg, "���� '%s' �ظ�����", func_name);
                report_semantic_error(SEM_REDECLARED_VAR, error_msg);
                context->error_count++;
                return false;
            }
            
            // ���õ�ǰ������������
            DataType previous_func_type = context->current_func_type;
            context->current_func_type = ret_type;
            
            // ���Ӻ��������ű�
            if (!add_symbol(context->symbol_table, func_name, SYM_FUNCTION, ret_type)) {
                context->error_count++;
                return false;
            }
            
            // �����崴����������
            enter_scope(context->symbol_table);
            bool body_result = check_stmt(node->left, context);
            leave_scope(context->symbol_table);
            
            // �ָ�֮ǰ�ĺ�������
            context->current_func_type = previous_func_type;
            
            return body_result;
        }
        
        default:
            return true;
    }
}

// Analyze AST for semantic correctness
bool analyze_semantics(ASTNode *root, SemanticContext *context) {
    if (!root || !context) {
        printf("Semantic analysis failed: invalid input parameters\n");
        return false;
    }
    
    printf("==================== BEGIN SEMANTIC ANALYSIS ====================\n");
    
    // Reset error count
    context->error_count = 0;
    
    // Recursively analyze AST
    check_stmt(root, context);
    
    printf("==================== END SEMANTIC ANALYSIS ====================\n");
    
    // Print analysis results
    if (context->error_count == 0) {
        printf("? Semantic analysis successful, no errors found\n");
    } else {
        printf("? Semantic analysis failed, found %d errors\n", context->error_count);
    }
    
    // Print symbol table
    printf("\n==================== SYMBOL TABLE INFORMATION ====================\n");
    print_symbol_table(context->symbol_table);
    
    return (context->error_count == 0);
}
