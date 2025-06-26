#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"
#include "symbol_table.h"

// 初始化IR生成器
IRGenerator* init_ir_generator(SymbolTable *symbol_table) {
    IRGenerator *gen = (IRGenerator*)malloc(sizeof(IRGenerator));
    gen->instructions = NULL;
    gen->last_instr = NULL;
    gen->temp_counter = 0;
    gen->label_counter = 0;
    gen->symbol_table = symbol_table;
    gen->var_type_table = NULL;  // 初始化变量类型映射表
    return gen;
}

// 释放IR生成器
void free_ir_generator(IRGenerator *gen) {
    IRInstruction *current = gen->instructions;
    while (current) {
        IRInstruction *next = current->next;
        if (current->result) free_operand(current->result);
        if (current->operand1) free_operand(current->operand1);
        if (current->operand2) free_operand(current->operand2);
        free(current);
        current = next;
    }
    
    // 释放变量类型映射表
    VarTypeNode *var_current = gen->var_type_table;
    while (var_current) {
        VarTypeNode *var_next = var_current->next;
        free(var_current->var_name);
        free(var_current);
        var_current = var_next;
    }
    
    free(gen);
}

// 创建临时变量操作数
Operand* create_temp_operand(int temp_id, DataType type) {
    Operand *operand = (Operand*)malloc(sizeof(Operand));
    operand->type = OPERAND_TEMP;
    operand->data_type = type;
    operand->temp_id = temp_id;
    return operand;
}

// 创建变量操作数
Operand* create_var_operand(const char *var_name, DataType type) {
    Operand *operand = (Operand*)malloc(sizeof(Operand));
    operand->type = OPERAND_VAR;
    operand->data_type = type;
    operand->var_name = strdup(var_name);
    return operand;
}

// 创建整数常量操作数
Operand* create_int_const_operand(int value) {
    Operand *operand = (Operand*)malloc(sizeof(Operand));
    operand->type = OPERAND_CONST;
    operand->data_type = TYPE_INT;
    operand->const_val.int_val = value;
    return operand;
}

// 创建浮点常量操作数
Operand* create_float_const_operand(float value) {
    Operand *operand = (Operand*)malloc(sizeof(Operand));
    operand->type = OPERAND_CONST;
    operand->data_type = TYPE_FLOAT;
    operand->const_val.float_val = value;
    return operand;
}

// 创建标签操作数
Operand* create_label_operand(const char *label_name) {
    Operand *operand = (Operand*)malloc(sizeof(Operand));
    operand->type = OPERAND_LABEL;
    operand->data_type = TYPE_UNKNOWN;
    operand->label_name = strdup(label_name);
    return operand;
}

// 创建函数操作数
Operand* create_func_operand(const char *func_name) {
    Operand *operand = (Operand*)malloc(sizeof(Operand));
    operand->type = OPERAND_FUNC;
    operand->data_type = TYPE_UNKNOWN;
    operand->func_name = strdup(func_name);
    return operand;
}

// 释放操作数
void free_operand(Operand *operand) {
    if (!operand) return;
    
    switch (operand->type) {
        case OPERAND_VAR:
            free(operand->var_name);
            break;
        case OPERAND_LABEL:
            free(operand->label_name);
            break;
        case OPERAND_FUNC:
            free(operand->func_name);
            break;
        default:
            break;
    }
    free(operand);
}

// 创建IR指令
IRInstruction* create_ir_instruction(IROpcode opcode) {
    IRInstruction *instr = (IRInstruction*)malloc(sizeof(IRInstruction));
    instr->opcode = opcode;
    instr->result = NULL;
    instr->operand1 = NULL;
    instr->operand2 = NULL;
    instr->next = NULL;
    return instr;
}

// 添加指令到链表
void append_instruction(IRGenerator *gen, IRInstruction *instr) {
    if (!gen->instructions) {
        gen->instructions = instr;
        gen->last_instr = instr;
    } else {
        gen->last_instr->next = instr;
        gen->last_instr = instr;
    }
}

// 获取下一个临时变量ID
int get_next_temp(IRGenerator *gen) {
    return ++gen->temp_counter;
}

// 获取下一个标签
char* get_next_label(IRGenerator *gen) {
    char *label = (char*)malloc(16);
    sprintf(label, "L%d", ++gen->label_counter);
    return label;
}

// 获取表达式类型 - 简化版本
DataType get_expr_type(ASTNode *node, SymbolTable *symbol_table) {
    switch (node->type) {
        case EXPR_INT:
            return TYPE_INT;
        case EXPR_FLOAT:
            return TYPE_FLOAT;
        case EXPR_VAR:
            // 简化：假设所有变量都是int类型
            return TYPE_INT;
        case EXPR_BINOP: {
            DataType left_type = get_expr_type(node->left, symbol_table);
            DataType right_type = get_expr_type(node->right, symbol_table);
            // 如果有浮点数，结果为浮点数
            if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
                return TYPE_FLOAT;
            }
            return TYPE_INT;
        }
        default:
            return TYPE_UNKNOWN;
    }
}

// 检查是否需要类型转换
bool need_type_conversion(DataType from, DataType to) {
    return from != to && from != TYPE_UNKNOWN && to != TYPE_UNKNOWN;
}

// 添加变量类型到映射表
void add_var_type(IRGenerator *gen, const char *var_name, DataType type) {
    if (!gen || !var_name) return;
    
    VarTypeNode *new_node = malloc(sizeof(VarTypeNode));
    new_node->var_name = strdup(var_name);
    new_node->type = type;
    new_node->next = gen->var_type_table;
    gen->var_type_table = new_node;
}

// 从映射表获取变量类型
DataType get_var_type(IRGenerator *gen, const char *var_name) {
    if (!gen || !var_name) return TYPE_INT;
    
    VarTypeNode *current = gen->var_type_table;
    while (current) {
        if (strcmp(current->var_name, var_name) == 0) {
            return current->type;
        }
        current = current->next;
    }
    
    return TYPE_INT;  // 默认返回int类型
}

// 生成类型转换
Operand* generate_type_conversion(IRGenerator *gen, Operand *operand, DataType target_type) {
    if (!need_type_conversion(operand->data_type, target_type)) {
        return operand;
    }
    
    int temp_id = get_next_temp(gen);
    Operand *result = create_temp_operand(temp_id, target_type);
    
    IRInstruction *instr = create_ir_instruction(IR_CONVERT);
    instr->result = result;
    instr->operand1 = operand;
    append_instruction(gen, instr);
    
    return result;
}

// 生成表达式的中间代码
Operand* generate_expr_ir(ASTNode *node, IRGenerator *gen) {
    if (!node) {
        return NULL;
    }
    
    switch (node->type) {
        case EXPR_INT:
            return create_int_const_operand(node->integer.value);
            
        case EXPR_FLOAT:
            return create_float_const_operand(node->floating.value);
            
        case EXPR_VAR: {
            // 从变量类型映射表中查找变量的实际类型
            DataType var_type = get_var_type(gen, node->var.name);
            
            int temp_id = get_next_temp(gen);
            Operand *result = create_temp_operand(temp_id, var_type);
            Operand *var_operand = create_var_operand(node->var.name, var_type);
            
            IRInstruction *instr = create_ir_instruction(IR_LOAD);
            instr->result = result;
            instr->operand1 = var_operand;
            append_instruction(gen, instr);
            
            return result;
        }
        
        case EXPR_BINOP: {
            Operand *left_operand = generate_expr_ir(node->left, gen);
            Operand *right_operand = generate_expr_ir(node->right, gen);
            
            // 确定结果类型
            DataType result_type = TYPE_INT;
            if (left_operand->data_type == TYPE_FLOAT || right_operand->data_type == TYPE_FLOAT) {
                result_type = TYPE_FLOAT;
            }
            
            // 类型转换
            if (need_type_conversion(left_operand->data_type, result_type)) {
                left_operand = generate_type_conversion(gen, left_operand, result_type);
            }
            if (need_type_conversion(right_operand->data_type, result_type)) {
                right_operand = generate_type_conversion(gen, right_operand, result_type);
            }
            
            int temp_id = get_next_temp(gen);
            Operand *result = create_temp_operand(temp_id, result_type);
            
            IRInstruction *instr = create_ir_instruction(IR_BINOP);
            instr->result = result;
            instr->operand1 = left_operand;
            instr->operand2 = right_operand;
            instr->binop = node->binop.op;
            append_instruction(gen, instr);
            
            return result;
        }
        
        case EXPR_CALL: {
            // 函数调用表达式
            return generate_call_expr_ir(node, gen);
        }
        
        default:
            return NULL;
    }
}

// 生成语句的中间代码
void generate_stmt_ir(ASTNode *node, IRGenerator *gen) {
    if (!node) return;
    
    switch (node->type) {
        case STMT_COMPOUND:
            if (node->left) generate_stmt_ir(node->left, gen);
            if (node->right) generate_stmt_ir(node->right, gen);
            break;
            
        case STMT_DECL:
            // 处理变量声明，记录类型到映射表
            if (node->decl.name && node->decl.var_type) {
                DataType var_type = TYPE_INT; // 默认为int
                if (strcmp(node->decl.var_type, "float") == 0) {
                    var_type = TYPE_FLOAT;
                }
                add_var_type(gen, node->decl.name, var_type);
            }
            break;
            
        case STMT_DECL_ASSIGN: {
            Operand *expr_operand = generate_expr_ir(node->left, gen);  // 修改为left
            
            // 从AST节点获取变量类型信息，而不是从符号表
            DataType var_type = TYPE_INT; // 默认为int
            if (node->decl.var_type) {
                if (strcmp(node->decl.var_type, "float") == 0) {
                    var_type = TYPE_FLOAT;
                }
            }
            
            // 记录变量类型到映射表
            add_var_type(gen, node->decl.name, var_type);
            
            // 类型转换
            if (need_type_conversion(expr_operand->data_type, var_type)) {
                expr_operand = generate_type_conversion(gen, expr_operand, var_type);
            }
            
            Operand *var_operand = create_var_operand(node->decl.name, var_type);
            
            IRInstruction *instr = create_ir_instruction(IR_STORE);
            instr->result = var_operand;
            instr->operand1 = expr_operand;
            append_instruction(gen, instr);
            break;
        }
        
        case STMT_ASSIGN: {
            Operand *expr_operand = generate_expr_ir(node->left, gen);  // 修改为left
            
            // 从变量类型映射表中查找变量的实际类型
            DataType var_type = get_var_type(gen, node->assign.name);
            
            // 类型转换 - 只有当真正需要时才转换
            if (need_type_conversion(expr_operand->data_type, var_type)) {
                expr_operand = generate_type_conversion(gen, expr_operand, var_type);
            }
            
            Operand *var_operand = create_var_operand(node->assign.name, var_type);
            
            IRInstruction *instr = create_ir_instruction(IR_STORE);
            instr->result = var_operand;
            instr->operand1 = expr_operand;
            append_instruction(gen, instr);
            break;
        }
        
        case STMT_IF: {
            Operand *cond_operand = generate_expr_ir(node->if_stmt.cond, gen);
            
            char *else_label = get_next_label(gen);
            char *end_label = get_next_label(gen);
            
            // 条件跳转到else分支
            IRInstruction *if_false_instr = create_ir_instruction(IR_IF_FALSE_GOTO);
            if_false_instr->operand1 = cond_operand;
            if_false_instr->operand2 = create_label_operand(else_label);
            append_instruction(gen, if_false_instr);
            
            // then分支
            if (node->left) generate_stmt_ir(node->left, gen);
            
            // 跳转到结束
            IRInstruction *goto_end_instr = create_ir_instruction(IR_GOTO);
            goto_end_instr->operand1 = create_label_operand(end_label);
            append_instruction(gen, goto_end_instr);
            
            // else标签
            IRInstruction *else_label_instr = create_ir_instruction(IR_LABEL);
            else_label_instr->operand1 = create_label_operand(else_label);
            append_instruction(gen, else_label_instr);
            
            // else分支
            if (node->right) generate_stmt_ir(node->right, gen);
            
            // 结束标签
            IRInstruction *end_label_instr = create_ir_instruction(IR_LABEL);
            end_label_instr->operand1 = create_label_operand(end_label);
            append_instruction(gen, end_label_instr);
            
            free(else_label);
            free(end_label);
            break;
        }
        
        case STMT_WHILE: {
            char *loop_label = get_next_label(gen);
            char *end_label = get_next_label(gen);
            
            // 循环开始标签
            IRInstruction *loop_label_instr = create_ir_instruction(IR_LABEL);
            loop_label_instr->operand1 = create_label_operand(loop_label);
            append_instruction(gen, loop_label_instr);
            
            // 条件判断
            Operand *cond_operand = generate_expr_ir(node->while_stmt.cond, gen);
            
            // 条件为假时跳出循环
            IRInstruction *if_false_instr = create_ir_instruction(IR_IF_FALSE_GOTO);
            if_false_instr->operand1 = cond_operand;
            if_false_instr->operand2 = create_label_operand(end_label);
            append_instruction(gen, if_false_instr);
            
            // 循环体
            if (node->left) generate_stmt_ir(node->left, gen);
            
            // 跳回循环开始
            IRInstruction *goto_loop_instr = create_ir_instruction(IR_GOTO);
            goto_loop_instr->operand1 = create_label_operand(loop_label);
            append_instruction(gen, goto_loop_instr);
            
            // 循环结束标签
            IRInstruction *end_label_instr = create_ir_instruction(IR_LABEL);
            end_label_instr->operand1 = create_label_operand(end_label);
            append_instruction(gen, end_label_instr);
            
            free(loop_label);
            free(end_label);
            break;
        }
        
        case STMT_RETURN: {
            if (node->left) {
                Operand *return_operand = generate_expr_ir(node->left, gen);
                
                IRInstruction *instr = create_ir_instruction(IR_RETURN);
                instr->operand1 = return_operand;
                append_instruction(gen, instr);
            } else {
                IRInstruction *instr = create_ir_instruction(IR_RETURN);
                append_instruction(gen, instr);
            }
            break;
        }
        
        case STMT_CALL: {
            // 函数调用语句
            generate_call_ir(node, gen);
            break;
        }
        
        // 处理表达式语句（如函数调用表达式）
        case EXPR_CALL: {
            generate_call_expr_ir(node, gen);
            break;
        }
        
        case EXPR_BINOP:
        case EXPR_VAR:
        case EXPR_INT:
        case EXPR_FLOAT: {
            // 对于其他表达式，我们只是生成代码但不保存结果
            generate_expr_ir(node, gen);
            break;
        }
        
        default:
            break;
    }
}

// 主中间代码生成函数
void generate_ir(ASTNode *node, IRGenerator *gen) {
    if (!node) return;
    
    switch (node->type) {
        case FUNC_DEF: {
            // 函数开始
            IRInstruction *func_begin = create_ir_instruction(IR_FUNC_BEGIN);
            func_begin->operand1 = create_func_operand(node->func_def.name);
            append_instruction(gen, func_begin);
            
            // 函数体
            if (node->left) generate_stmt_ir(node->left, gen);
            
            // 函数结束
            IRInstruction *func_end = create_ir_instruction(IR_FUNC_END);
            append_instruction(gen, func_end);
            break;
        }
        
        default:
            generate_stmt_ir(node, gen);
            break;
    }
}

// 生成函数调用的中间代码
void generate_call_ir(ASTNode *node, IRGenerator *gen) {
    if (!node || node->type != STMT_CALL) return;
    
    // 为每个参数生成PARAM指令
    for (int i = 0; i < node->call.arg_count; i++) {
        if (node->call.args[i]) {
            Operand *arg_operand = generate_expr_ir(node->call.args[i], gen);
            
            IRInstruction *param_instr = create_ir_instruction(IR_PARAM);
            param_instr->operand1 = arg_operand;
            append_instruction(gen, param_instr);
        }
    }
    
    // 生成CALL指令
    IRInstruction *call_instr = create_ir_instruction(IR_CALL);
    call_instr->operand1 = create_func_operand(node->call.name);
    
    // 对于printf这样的函数，我们可能不需要保存返回值
    // 但为了完整性，我们创建一个临时变量来存储返回值
    int temp_id = get_next_temp(gen);
    Operand *result_temp = create_temp_operand(temp_id, TYPE_INT);
    call_instr->result = result_temp;
    
    append_instruction(gen, call_instr);
}

// 生成函数调用表达式的中间代码
Operand* generate_call_expr_ir(ASTNode *node, IRGenerator *gen) {
    if (!node || node->type != EXPR_CALL) return NULL;
    
    // 为每个参数生成PARAM指令
    for (int i = 0; i < node->call.arg_count; i++) {
        if (node->call.args[i]) {
            Operand *arg_operand = generate_expr_ir(node->call.args[i], gen);
            
            IRInstruction *param_instr = create_ir_instruction(IR_PARAM);
            param_instr->operand1 = arg_operand;
            append_instruction(gen, param_instr);
        }
    }
    
    // 生成CALL指令
    IRInstruction *call_instr = create_ir_instruction(IR_CALL);
    call_instr->operand1 = create_func_operand(node->call.name);
    
    // 创建临时变量来存储返回值
    int temp_id = get_next_temp(gen);
    Operand *result_temp = create_temp_operand(temp_id, TYPE_INT);
    call_instr->result = result_temp;
    
    append_instruction(gen, call_instr);
    
    return result_temp;
}

// 打印操作数
void print_operand(Operand *operand) {
    if (!operand) {
        printf("NULL");
        return;
    }
    
    switch (operand->type) {
        case OPERAND_TEMP:
            printf("t%d", operand->temp_id);
            break;
        case OPERAND_VAR:
            printf("%s", operand->var_name);
            break;
        case OPERAND_CONST:
            if (operand->data_type == TYPE_INT) {
                printf("%d", operand->const_val.int_val);
            } else {
                printf("%.2f", operand->const_val.float_val);
            }
            break;
        case OPERAND_LABEL:
            printf("%s", operand->label_name);
            break;
        case OPERAND_FUNC:
            printf("%s", operand->func_name);
            break;
    }
}

// 打印指令
void print_instruction(IRInstruction *instr) {
    switch (instr->opcode) {
        case IR_ASSIGN:
            print_operand(instr->result);
            printf(" = ");
            print_operand(instr->operand1);
            break;
            
        case IR_BINOP:
            print_operand(instr->result);
            printf(" = ");
            print_operand(instr->operand1);
            switch (instr->binop) {
                case OP_ADD: printf(" + "); break;
                case OP_SUB: printf(" - "); break;
                case OP_MUL: printf(" * "); break;
                case OP_DIV: printf(" / "); break;
                case OP_EQ: printf(" == "); break;
                case OP_NE: printf(" != "); break;
                case OP_LT: printf(" < "); break;
                case OP_GT: printf(" > "); break;
                case OP_LE: printf(" <= "); break;
                case OP_GE: printf(" >= "); break;
            }
            print_operand(instr->operand2);
            break;
            
        case IR_LOAD:
            print_operand(instr->result);
            printf(" = ");
            print_operand(instr->operand1);
            break;
            
        case IR_STORE:
            print_operand(instr->result);
            printf(" = ");
            print_operand(instr->operand1);
            break;
            
        case IR_LOAD_CONST:
            print_operand(instr->result);
            printf(" = ");
            print_operand(instr->operand1);
            break;
            
        case IR_LABEL:
            print_operand(instr->operand1);
            printf(":");
            break;
            
        case IR_GOTO:
            printf("goto ");
            print_operand(instr->operand1);
            break;
            
        case IR_IF_GOTO:
            printf("if ");
            print_operand(instr->operand1);
            printf(" goto ");
            print_operand(instr->operand2);
            break;
            
        case IR_IF_FALSE_GOTO:
            printf("if !");
            print_operand(instr->operand1);
            printf(" goto ");
            print_operand(instr->operand2);
            break;
            
        case IR_RETURN:
            printf("return");
            if (instr->operand1) {
                printf(" ");
                print_operand(instr->operand1);
            }
            break;
            
        case IR_FUNC_BEGIN:
            printf("func_begin ");
            print_operand(instr->operand1);
            break;
            
        case IR_FUNC_END:
            printf("func_end");
            break;
            
        case IR_CONVERT:
            print_operand(instr->result);
            printf(" = (");
            printf(instr->result->data_type == TYPE_INT ? "int" : "float");
            printf(") ");
            print_operand(instr->operand1);
            break;
            
        case IR_PARAM:
            printf("param ");
            print_operand(instr->operand1);
            break;
            
        case IR_CALL:
            if (instr->result) {
                print_operand(instr->result);
                printf(" = ");
            }
            printf("call ");
            print_operand(instr->operand1);
            break;
            
        default:
            printf("unknown instruction");
            break;
    }
}

// 打印所有中间代码
void print_ir(IRGenerator *gen) {
    printf("\n=== Intermediate Code ===\n");
    IRInstruction *current = gen->instructions;
    int line_num = 1;
    
    while (current) {
        printf("%3d: ", line_num++);
        print_instruction(current);
        printf("\n");
        current = current->next;
    }
    printf("=========================\n");
}
