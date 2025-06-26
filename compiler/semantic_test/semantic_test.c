#include "semantic.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("==================== 语义分析测试 ====================\n");
    
    // 初始化语义分析上下文
    SemanticContext *context = init_semantic();
    if (!context) {
        printf("语义分析上下文初始化失败\n");
        return 1;
    }
    
    // 测试1：变量声明和使用
    printf("\n测试1：变量声明和使用\n");
    ASTNode *var_decl = create_decl("int", "x");
    ASTNode *var_use = create_var("x");
    ASTNode *assign_stmt = create_assign("x", create_int(10));
    
    // 分析变量声明
    if (check_stmt(var_decl, context)) {
        printf("✓ 变量声明分析成功\n");
    } else {
        printf("✗ 变量声明分析失败\n");
    }
    
    // 分析变量使用
    DataType var_type = check_expr_type(var_use, context);
    if (var_type == TYPE_INT) {
        printf("✓ 变量使用类型检查成功\n");
    } else {
        printf("✗ 变量使用类型检查失败\n");
    }
    
    // 分析赋值语句
    if (check_stmt(assign_stmt, context)) {
        printf("✓ 赋值语句分析成功\n");
    } else {
        printf("✗ 赋值语句分析失败\n");
    }
    
    // 测试2：类型不匹配错误
    printf("\n测试2：类型不匹配错误检测\n");
    ASTNode *float_decl = create_decl("float", "y");
    ASTNode *type_mismatch_assign = create_assign("y", create_var("x")); // int赋值给float
    
    check_stmt(float_decl, context);
    if (check_stmt(type_mismatch_assign, context)) {
        printf("✓ 类型兼容性检查通过（int->float隐式转换）\n");
    } else {
        printf("✗ 类型兼容性检查失败\n");
    }
    
    // 测试3：未声明变量错误
    printf("\n测试3：未声明变量错误检测\n");
    ASTNode *undeclared_var = create_var("undefined_var");
    DataType undeclared_type = check_expr_type(undeclared_var, context);
    if (undeclared_type == TYPE_UNKNOWN) {
        printf("✓ 未声明变量错误检测成功\n");
    } else {
        printf("✗ 未声明变量错误检测失败\n");
    }
    
    // 测试4：二元运算类型检查
    printf("\n测试4：二元运算类型检查\n");
    ASTNode *add_expr = create_binop(OP_ADD, create_var("x"), create_int(5));
    DataType add_result_type = check_expr_type(add_expr, context);
    if (add_result_type == TYPE_INT) {
        printf("✓ 二元运算类型推导成功\n");
    } else {
        printf("✗ 二元运算类型推导失败\n");
    }
    
    // 测试5：除零检测
    printf("\n测试5：除零错误检测\n");
    ASTNode *div_zero = create_binop(OP_DIV, create_var("x"), create_int(0));
    DataType div_result_type = check_expr_type(div_zero, context);
    if (div_result_type == TYPE_UNKNOWN) {
        printf("✓ 除零错误检测成功\n");
    } else {
        printf("✗ 除零错误检测失败\n");
    }
    
    // 最终结果
    printf("\n==================== 测试结果 ====================\n");
    printf("总错误数量: %d\n", context->error_count);
    
    // 打印符号表
    print_symbol_table(context->symbol_table);
    
    // 清理资源
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
