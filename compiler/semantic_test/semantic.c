#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 初始化语义分析上下文
SemanticContext* init_semantic() {
    SemanticContext *context = (SemanticContext*)malloc(sizeof(SemanticContext));
    if (context) {
        context->symbol_table = init_symbol_table();
        context->current_func_type = TYPE_UNKNOWN;
        context->error_count = 0;
    }
    return context;
}

// 释放语义分析上下文
void free_semantic(SemanticContext *context) {
    if (context) {
        free_symbol_table(context->symbol_table);
        free(context);
    }
}

// 报告语义错误
void report_semantic_error(SemanticErrorType error, const char *message) {
    const char *error_type = "";
    
    switch (error) {
        case SEM_UNDECLARED_VAR:
            error_type = "未声明的变量";
            break;
        case SEM_REDECLARED_VAR:
            error_type = "重复声明的变量";
            break;
        case SEM_TYPE_MISMATCH:
            error_type = "类型不匹配";
            break;
        case SEM_INVALID_RETURN_TYPE:
            error_type = "无效的返回类型";
            break;
        case SEM_DIVISION_BY_ZERO:
            error_type = "除零错误";
            break;
        case SEM_INVALID_OPERATION:
            error_type = "无效操作";
            break;
        case SEM_FUNCTION_NOT_DECLARED:
            error_type = "函数未声明";
            break;
        default:
            error_type = "未知错误";
    }
    
    fprintf(stderr, "语义错误: %s - %s\n", error_type, message);
}

// 检查表达式类型
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
                report_semantic_error(SEM_UNDECLARED_VAR, error_msg);
                context->error_count++;
                return TYPE_UNKNOWN;
            }
            return entry->type;
        }
        
        case EXPR_BINOP: {
            // 递归检查左右操作数类型
            DataType left_type = check_expr_type(node->left, context);
            DataType right_type = check_expr_type(node->right, context);
            
            // 类型兼容性检查
            if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
                return TYPE_UNKNOWN;
            }
            
            // 检查除零错误
            if (node->binop.op == OP_DIV) {
                if (node->right->type == EXPR_INT && node->right->integer.value == 0) {
                    report_semantic_error(SEM_DIVISION_BY_ZERO, "整数除零");
                    context->error_count++;
                    return TYPE_UNKNOWN;
                } else if (node->right->type == EXPR_FLOAT && node->right->floating.value == 0.0) {
                    report_semantic_error(SEM_DIVISION_BY_ZERO, "浮点数除零");
                    context->error_count++;
                    return TYPE_UNKNOWN;
                }
            }
            
            // 检查比较运算符的类型兼容性
            if (node->binop.op >= OP_EQ && node->binop.op <= OP_GE) {
                if (!check_type_compatible(left_type, right_type) && 
                    !check_type_compatible(right_type, left_type)) {
                    char error_msg[200];
                    sprintf(error_msg, "无法比较 %s 和 %s 类型", 
                           data_type_to_str(left_type), data_type_to_str(right_type));
                    report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                    context->error_count++;
                }
                return TYPE_INT; // 比较运算结果为逻辑值，用int表示
            }
            
            // 算术运算的类型提升
            return get_result_type(left_type, right_type);
        }
        
        default:
            return TYPE_UNKNOWN;
    }
}

// 推断变量类型
DataType infer_var_type(const char *type_name) {
    if (!type_name) return TYPE_UNKNOWN;
    
    if (strcmp(type_name, "int") == 0) {
        return TYPE_INT;
    } else if (strcmp(type_name, "float") == 0) {
        return TYPE_FLOAT;
    } else if (strcmp(type_name, "void") == 0) {
        return TYPE_UNKNOWN; // void类型用UNKNOWN表示
    }
    
    return TYPE_UNKNOWN;
}

// 检查类型兼容性（增加对隐式转换的支持）
bool check_type_compatible(DataType target_type, DataType expr_type) {
    // 相同类型总是兼容的
    if (target_type == expr_type) return true;
    
    // 支持一些安全的隐式类型转换
    if (target_type == TYPE_FLOAT && expr_type == TYPE_INT) {
        // int 可以安全地转换为 float
        return true;
    }
    
    if (target_type == TYPE_INT && expr_type == TYPE_FLOAT) {
        // float 转换为 int 是允许的，但可能导致精度损失
        // 在许多语言中这是允许的，尽管会产生警告
        printf("警告: 从 float 转换到 int 可能导致精度损失\n");
        return true;
    }
    
    return false;
}

// 检查是否为算术运算符
bool is_arithmetic_op(BinOpType op) {
    return (op == OP_ADD || op == OP_SUB || op == OP_MUL || op == OP_DIV);
}

// 检查是否为关系运算符
bool is_relational_op(BinOpType op) {
    return (op >= OP_EQ && op <= OP_GE);
}

// 检查常量表达式的值
bool is_zero_constant(ASTNode *node) {
    if (!node) return false;
    
    if (node->type == EXPR_INT) {
        return node->integer.value == 0;
    } else if (node->type == EXPR_FLOAT) {
        return node->floating.value == 0.0;
    }
    
    return false;
}

// 类型转换警告
void type_conversion_warning(DataType from_type, DataType to_type, const char *context_msg) {
    if (from_type == TYPE_FLOAT && to_type == TYPE_INT) {
        printf("警告: %s - 从 float 转换到 int 可能导致精度损失\n", context_msg);
    }
}

// 检查语句
bool check_stmt(ASTNode *node, SemanticContext *context) {
    if (!node) return true;
    
    switch(node->type) {
        case STMT_COMPOUND: {
            // 不为复合语句创建新作用域，只分析其子语句
            bool left_result = check_stmt(node->left, context);
            bool right_result = check_stmt(node->right, context);
            return left_result && right_result;
        }
        
        case STMT_DECL: {
            // 处理变量声明
            const char *var_name = node->decl.name;
            const char *type_str = node->decl.var_type;
            
            // 根据类型字符串确定数据类型
            DataType var_type = infer_var_type(type_str);
            if (var_type == TYPE_UNKNOWN) {
                char error_msg[200];
                sprintf(error_msg, "未知的类型 '%s'", type_str);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            // 查找变量是否已存在于当前作用域
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
            // 处理带初始值的变量声明
            const char *var_name = node->decl.name;
            const char *type_str = node->decl.var_type;
            
            // 确定变量类型
            DataType var_type = infer_var_type(type_str);
            if (var_type == TYPE_UNKNOWN) {
                char error_msg[200];
                sprintf(error_msg, "未知的类型 '%s'", type_str);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            // 检查初始值表达式类型
            DataType expr_type = check_expr_type(node->left, context);
            
            // 查找变量是否已存在
            if (lookup_symbol_current_scope(context->symbol_table, var_name)) {
                char error_msg[100];
                sprintf(error_msg, "'%s'", var_name);
                report_semantic_error(SEM_REDECLARED_VAR, error_msg);
                context->error_count++;
                return false;
            }
            
            // 类型兼容性检查
            if (!check_type_compatible(var_type, expr_type)) {
                char error_msg[200];
                sprintf(error_msg, "无法将 %s 类型赋值给 %s 类型变量 '%s'", 
                        data_type_to_str(expr_type), data_type_to_str(var_type), var_name);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            return add_symbol(context->symbol_table, var_name, SYM_VARIABLE, var_type);
        }
        
        case STMT_ASSIGN: {
            // 处理赋值语句
            const char *var_name = node->assign.name;
            SymbolEntry *entry = lookup_symbol(context->symbol_table, var_name);
            
            if (!entry) {
                char error_msg[100];
                sprintf(error_msg, "'%s'", var_name);
                report_semantic_error(SEM_UNDECLARED_VAR, error_msg);
                context->error_count++;
                return false;
            }
            
            // 检查右侧表达式类型
            DataType expr_type = check_expr_type(node->left, context);
            
            // 类型兼容性检查
            if (!check_type_compatible(entry->type, expr_type)) {
                char error_msg[200];
                sprintf(error_msg, "无法将 %s 类型赋值给 %s 类型变量 '%s'", 
                       data_type_to_str(expr_type), data_type_to_str(entry->type), var_name);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            return true;
        }
        
        case STMT_RETURN: {
            // 处理返回语句
            DataType expr_type = check_expr_type(node->left, context);
            
            // 检查返回值类型与函数声明一致性，使用增强的类型兼容性检查
            if (context->current_func_type != TYPE_UNKNOWN && 
                expr_type != TYPE_UNKNOWN && 
                !check_type_compatible(context->current_func_type, expr_type)) {
                
                char error_msg[100];
                sprintf(error_msg, "函数返回类型为 %s，但返回值类型为 %s", 
                       data_type_to_str(context->current_func_type), 
                       data_type_to_str(expr_type));
                report_semantic_error(SEM_INVALID_RETURN_TYPE, error_msg);
                context->error_count++;
                return false;
            }
            
            return true;
        }
        
        case STMT_IF: {
            // 处理if语句
            DataType cond_type = check_expr_type(node->if_stmt.cond, context);
            
            // 检查条件表达式类型
            if (cond_type != TYPE_INT && cond_type != TYPE_FLOAT && cond_type != TYPE_UNKNOWN) {
                report_semantic_error(SEM_TYPE_MISMATCH, "if条件表达式必须是数值类型");
                context->error_count++;
                return false;
            }
            
            // 检查then分支
            enter_scope(context->symbol_table);
            bool then_result = check_stmt(node->left, context);
            leave_scope(context->symbol_table);
            
            // 检查else分支
            bool else_result = true;
            if (node->right) {
                enter_scope(context->symbol_table);
                else_result = check_stmt(node->right, context);
                leave_scope(context->symbol_table);
            }
            
            return then_result && else_result;
        }
        
        case STMT_WHILE: {
            // 处理while语句
            DataType cond_type = check_expr_type(node->while_stmt.cond, context);
            
            // 检查条件表达式类型
            if (cond_type != TYPE_INT && cond_type != TYPE_FLOAT && cond_type != TYPE_UNKNOWN) {
                report_semantic_error(SEM_TYPE_MISMATCH, "while条件表达式必须是数值类型");
                context->error_count++;
                return false;
            }
            
            // 检查循环体
            enter_scope(context->symbol_table);
            bool body_result = check_stmt(node->left, context);
            leave_scope(context->symbol_table);
            
            return body_result;
        }
        
        case FUNC_DEF: {
            // 处理函数定义
            const char *func_name = node->func_def.name;
            const char *ret_type_str = node->func_def.ret_type;
            
            DataType ret_type = infer_var_type(ret_type_str);
            if (ret_type == TYPE_UNKNOWN) {
                char error_msg[200];
                sprintf(error_msg, "函数 '%s' 的返回类型 '%s' 未知", func_name, ret_type_str);
                report_semantic_error(SEM_TYPE_MISMATCH, error_msg);
                context->error_count++;
                return false;
            }
            
            // 检查函数是否已声明
            if (lookup_symbol_current_scope(context->symbol_table, func_name)) {
                char error_msg[200];
                sprintf(error_msg, "函数 '%s' 重复定义", func_name);
                report_semantic_error(SEM_REDECLARED_VAR, error_msg);
                context->error_count++;
                return false;
            }
            
            // 设置当前函数返回类型
            DataType previous_func_type = context->current_func_type;
            context->current_func_type = ret_type;
            
            // 添加函数到符号表
            if (!add_symbol(context->symbol_table, func_name, SYM_FUNCTION, ret_type)) {
                context->error_count++;
                return false;
            }
            
            // 函数体创建新作用域
            enter_scope(context->symbol_table);
            bool body_result = check_stmt(node->left, context);
            leave_scope(context->symbol_table);
            
            // 恢复之前的函数类型
            context->current_func_type = previous_func_type;
            
            return body_result;
        }
        
        default:
            return true;
    }
}

// 对整个AST进行语义分析
bool analyze_semantics(ASTNode *root, SemanticContext *context) {
    if (!root || !context) {
        printf("语义分析失败：无效的输入参数\n");
        return false;
    }
    
    printf("==================== 开始语义分析 ====================\n");
    
    // 重置错误计数
    context->error_count = 0;
    
    // 递归分析AST
    check_stmt(root, context);
    
    printf("==================== 语义分析完成 ====================\n");
    
    // 打印分析结果
    if (context->error_count == 0) {
        printf("? 语义分析成功：未发现错误\n");
    } else {
        printf("? 语义分析失败：发现 %d 个错误\n", context->error_count);
    }
    
    // 打印符号表
    printf("\n==================== 符号表信息 ====================\n");
    print_symbol_table(context->symbol_table);
    
    return (context->error_count == 0);
}
