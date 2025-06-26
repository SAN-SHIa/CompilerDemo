#include "semantic.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("==================== Semantic Analysis Test ====================\n");
    
    // Initialize semantic context
    SemanticContext *context = init_semantic();
    if (!context) {
        printf("Failed to initialize semantic context\n");
        return 1;
    }
    
    // Test 1: Variable declaration and usage
    printf("\nTest 1: Variable Declaration and Usage\n");
    ASTNode *var_decl = create_decl("int", "x");
    ASTNode *var_use = create_var("x");
    ASTNode *assign_stmt = create_assign("x", create_int(10));
    
    // Analyze variable declaration
    if (check_stmt(var_decl, context)) {
        printf("✓ Variable declaration analysis successful\n");
    } else {
        printf("✗ Variable declaration analysis failed\n");
    }
    
    // Analyze variable usage
    DataType var_type = check_expr_type(var_use, context);
    if (var_type == TYPE_INT) {
        printf("✓ Variable usage type check successful\n");
    } else {
        printf("✗ Variable usage type check failed\n");
    }
    
    // Analyze assignment statement
    if (check_stmt(assign_stmt, context)) {
        printf("✓ Assignment statement analysis successful\n");
    } else {
        printf("✗ Assignment statement analysis failed\n");
    }
    
    // Test 2: Type mismatch detection
    printf("\nTest 2: Type Compatibility Check\n");
    ASTNode *float_decl = create_decl("float", "y");
    ASTNode *type_mismatch_assign = create_assign("y", create_var("x")); // int assigned to float
    
    check_stmt(float_decl, context);
    if (check_stmt(type_mismatch_assign, context)) {
        printf("✓ Type compatibility check passed (int->float implicit conversion)\n");
    } else {
        printf("✗ Type compatibility check failed\n");
    }
    
    // Test 3: Undeclared variable error
    printf("\nTest 3: Undeclared Variable Error Detection\n");
    ASTNode *undeclared_var = create_var("undefined_var");
    DataType undeclared_type = check_expr_type(undeclared_var, context);
    if (undeclared_type == TYPE_UNKNOWN) {
        printf("✓ Undeclared variable error detection successful\n");
    } else {
        printf("✗ Undeclared variable error detection failed\n");
    }
    
    // Test 4: Binary operation type check
    printf("\nTest 4: Binary Operation Type Check\n");
    ASTNode *add_expr = create_binop(OP_ADD, create_var("x"), create_int(5));
    DataType add_result_type = check_expr_type(add_expr, context);
    if (add_result_type == TYPE_INT) {
        printf("✓ Binary operation type inference successful\n");
    } else {
        printf("✗ Binary operation type inference failed\n");
    }
    
    // Test 5: Division by zero detection
    printf("\nTest 5: Division by Zero Error Detection\n");
    ASTNode *div_zero = create_binop(OP_DIV, create_var("x"), create_int(0));
    DataType div_result_type = check_expr_type(div_zero, context);
    if (div_result_type == TYPE_UNKNOWN) {
        printf("✓ Division by zero error detection successful\n");
    } else {
        printf("✗ Division by zero error detection failed\n");
    }
    
    // Final results
    printf("\n==================== Test Results ====================\n");
    printf("Total error count: %d\n", context->error_count);
    
    // Print symbol table
    print_symbol_table(context->symbol_table);
    
    // Clean up resources
    free_ast(var_decl);
    free_ast(var_use);
    free_ast(assign_stmt);
    free_ast(float_decl);
    free_ast(type_mismatch_assign);
    free_ast(undeclared_var);
    free_ast(add_expr);
    free_ast(div_zero);
    free_semantic(context);
    
    return 0;
}
