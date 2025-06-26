#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

// 语义错误类型
typedef enum {
    SEM_OK,                    // 无错误
    SEM_UNDECLARED_VAR,        // 未声明变量
    SEM_REDECLARED_VAR,        // 重复声明变量
    SEM_TYPE_MISMATCH,         // 类型不匹配
    SEM_INVALID_RETURN_TYPE,   // 无效的返回类型
    SEM_DIVISION_BY_ZERO,      // 除零错误
    SEM_INVALID_OPERATION,     // 无效操作
    SEM_FUNCTION_NOT_DECLARED  // 函数未声明
} SemanticErrorType;

// 语义分析上下文
typedef struct {
    SymbolTable *symbol_table;  // 符号表
    DataType current_func_type;  // 当前函数返回类型
    int error_count;            // 错误计数
} SemanticContext;

// 语义分析接口函数
SemanticContext* init_semantic();
void free_semantic(SemanticContext *context);
bool analyze_semantics(ASTNode *root, SemanticContext *context);
void report_semantic_error(SemanticErrorType error, const char *message);

// 类型检查函数
DataType check_expr_type(ASTNode *node, SemanticContext *context);
bool check_type_compatible(DataType target_type, DataType expr_type);
bool check_stmt(ASTNode *node, SemanticContext *context);

// 辅助函数
DataType infer_var_type(const char *type_name);
bool is_arithmetic_op(BinOpType op);
bool is_relational_op(BinOpType op);
bool is_zero_constant(ASTNode *node);
void type_conversion_warning(DataType from_type, DataType to_type, const char *context_msg);

#endif
