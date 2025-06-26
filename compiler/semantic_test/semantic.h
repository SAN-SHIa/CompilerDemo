#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

// �����������
typedef enum {
    SEM_OK,                    // �޴���
    SEM_UNDECLARED_VAR,        // δ��������
    SEM_REDECLARED_VAR,        // �ظ���������
    SEM_TYPE_MISMATCH,         // ���Ͳ�ƥ��
    SEM_INVALID_RETURN_TYPE,   // ��Ч�ķ�������
    SEM_DIVISION_BY_ZERO,      // �������
    SEM_INVALID_OPERATION,     // ��Ч����
    SEM_FUNCTION_NOT_DECLARED  // ����δ����
} SemanticErrorType;

// �������������
typedef struct {
    SymbolTable *symbol_table;  // ���ű�
    DataType current_func_type;  // ��ǰ������������
    int error_count;            // �������
} SemanticContext;

// ��������ӿں���
SemanticContext* init_semantic();
void free_semantic(SemanticContext *context);
bool analyze_semantics(ASTNode *root, SemanticContext *context);
void report_semantic_error(SemanticErrorType error, const char *message);

// ���ͼ�麯��
DataType check_expr_type(ASTNode *node, SemanticContext *context);
bool check_type_compatible(DataType target_type, DataType expr_type);
bool check_stmt(ASTNode *node, SemanticContext *context);

// ��������
DataType infer_var_type(const char *type_name);
bool is_arithmetic_op(BinOpType op);
bool is_relational_op(BinOpType op);
bool is_zero_constant(ASTNode *node);
void type_conversion_warning(DataType from_type, DataType to_type, const char *context_msg);

#endif
