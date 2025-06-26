#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

// Semantic error types
typedef enum {
    SEM_OK,                    // No error
    SEM_UNDECLARED_VAR,        // Undeclared variable
    SEM_REDECLARED_VAR,        // Variable redeclaration
    SEM_TYPE_MISMATCH,         // Type mismatch
    SEM_INVALID_RETURN_TYPE,   // Invalid return type
    SEM_DIVISION_BY_ZERO,      // Division by zero
    SEM_INVALID_OPERATION,     // Invalid operation
    SEM_FUNCTION_NOT_DECLARED  // Function not declared
} SemanticErrorType;

// Semantic analysis context
typedef struct {
    SymbolTable *symbol_table;  // Symbol table
    DataType current_func_type;  // Current function return type
    int error_count;            // Error count
} SemanticContext;

// Semantic analysis interface functions
SemanticContext* init_semantic();
void free_semantic(SemanticContext *context);
bool analyze_semantics(ASTNode *root, SemanticContext *context);
// Report semantic errors with location
void report_semantic_error(SemanticErrorType error, const char *message);
void report_semantic_error_with_location(SemanticErrorType error, const char *message, int line, int column);

// Type checking functions
DataType check_expr_type(ASTNode *node, SemanticContext *context);
bool check_type_compatible(DataType target_type, DataType expr_type);
bool check_stmt(ASTNode *node, SemanticContext *context);

// Helper functions
DataType infer_var_type(const char *type_name);
bool is_arithmetic_op(BinOpType op);
bool is_relational_op(BinOpType op);
bool is_zero_constant(ASTNode *node);
void type_conversion_warning(DataType from_type, DataType to_type, const char *context_msg);

#endif
