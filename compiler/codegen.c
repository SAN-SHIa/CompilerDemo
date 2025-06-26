#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"

// 初始化代码生成器
CodeGenerator* init_code_generator(TargetArch target_arch, const char *output_filename) {
    CodeGenerator *gen = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    gen->target_arch = target_arch;
    gen->output_file = fopen(output_filename, "w");
    if (!gen->output_file) {
        fprintf(stderr, "Failed to create output file: %s\n", output_filename);
        free(gen);
        return NULL;
    }
    
    gen->var_locations = NULL;
    gen->stack_offset = 0;
    gen->label_counter = 0;
    gen->optimization_enabled = true;
    gen->instructions_generated = 0;
    gen->registers_used = 0;
    gen->stack_space_used = 0;
    
    init_registers(gen);
    
    return gen;
}

// 释放代码生成器
void free_code_generator(CodeGenerator *gen) {
    if (gen->output_file) {
        fclose(gen->output_file);
    }
    
    // 释放变量位置列表
    VarLocation *var_loc = gen->var_locations;
    while (var_loc) {
        VarLocation *next = var_loc->next;
        free(var_loc->var_name);
        if (var_loc->location.type == MEM_GLOBAL) {
            free(var_loc->location.global.label);
        }
        free(var_loc);
        var_loc = next;
    }
    
    if (gen->registers) {
        for (int i = 0; i < gen->register_count; i++) {
            free(gen->registers[i].name);
        }
        free(gen->registers);
    }
    
    free(gen);
}

// 初始化寄存器
void init_registers(CodeGenerator *gen) {
    switch (gen->target_arch) {
        case TARGET_X86_64:
            gen->register_count = 12;
            gen->registers = (Register*)malloc(gen->register_count * sizeof(Register));
            
            // 通用寄存器
            gen->registers[0] = (Register){REG_RAX, strdup("rax"), true, -1, TYPE_UNKNOWN};
            gen->registers[1] = (Register){REG_RBX, strdup("rbx"), true, -1, TYPE_UNKNOWN};
            gen->registers[2] = (Register){REG_RCX, strdup("rcx"), true, -1, TYPE_UNKNOWN};
            gen->registers[3] = (Register){REG_RDX, strdup("rdx"), true, -1, TYPE_UNKNOWN};
            gen->registers[4] = (Register){REG_RSI, strdup("rsi"), true, -1, TYPE_UNKNOWN};
            gen->registers[5] = (Register){REG_RDI, strdup("rdi"), true, -1, TYPE_UNKNOWN};
            gen->registers[6] = (Register){REG_R8, strdup("r8"), true, -1, TYPE_UNKNOWN};
            gen->registers[7] = (Register){REG_R9, strdup("r9"), true, -1, TYPE_UNKNOWN};
            
            // 浮点寄存器
            gen->registers[8] = (Register){REG_XMM0, strdup("xmm0"), true, -1, TYPE_UNKNOWN};
            gen->registers[9] = (Register){REG_XMM1, strdup("xmm1"), true, -1, TYPE_UNKNOWN};
            gen->registers[10] = (Register){REG_XMM2, strdup("xmm2"), true, -1, TYPE_UNKNOWN};
            gen->registers[11] = (Register){REG_XMM3, strdup("xmm3"), true, -1, TYPE_UNKNOWN};
            break;
            
        case TARGET_PSEUDO:
            gen->register_count = 8;
            gen->registers = (Register*)malloc(gen->register_count * sizeof(Register));
            
            // 通用寄存器
            gen->registers[0] = (Register){REG_RAX, strdup("R0"), true, -1, TYPE_UNKNOWN};
            gen->registers[1] = (Register){REG_RBX, strdup("R1"), true, -1, TYPE_UNKNOWN};
            gen->registers[2] = (Register){REG_RCX, strdup("R2"), true, -1, TYPE_UNKNOWN};
            gen->registers[3] = (Register){REG_RDX, strdup("R3"), true, -1, TYPE_UNKNOWN};
            
            // 浮点寄存器
            gen->registers[4] = (Register){REG_XMM0, strdup("F0"), true, -1, TYPE_UNKNOWN};
            gen->registers[5] = (Register){REG_XMM1, strdup("F1"), true, -1, TYPE_UNKNOWN};
            gen->registers[6] = (Register){REG_XMM2, strdup("F2"), true, -1, TYPE_UNKNOWN};
            gen->registers[7] = (Register){REG_XMM3, strdup("F3"), true, -1, TYPE_UNKNOWN};
            break;
            
        default:
            gen->register_count = 0;
            gen->registers = NULL;
            break;
    }
}

// 主代码生成函数
void generate_target_code(IRGenerator *ir_gen, CodeGenerator *code_gen) {
    printf("\n=== Start Target Code Generation ===\n");
    printf("Target architecture: %d\n", code_gen->target_arch);
    
    emit_file_header(code_gen);
    
    switch (code_gen->target_arch) {
        case TARGET_X86_64:
        case TARGET_X86_32:
            printf("Generating assembly code...\n");
            generate_assembly_code(ir_gen, code_gen);
            break;
        case TARGET_C_CODE:
            printf("Generating C code...\n");
            generate_c_code(ir_gen, code_gen);
            break;
        case TARGET_PSEUDO:
            printf("Generating pseudo code...\n");
            generate_pseudo_code(ir_gen, code_gen);
            break;
        default:
            printf("Unsupported target architecture\n");
            return;
    }
    
    emit_file_footer(code_gen);
    
    print_codegen_stats(code_gen);
}

// 生成汇编代码
void generate_assembly_code(IRGenerator *ir_gen, CodeGenerator *code_gen) {
    IRInstruction *instr = ir_gen->instructions;
    
    while (instr) {
        switch (code_gen->target_arch) {
            case TARGET_X86_64:
                generate_x86_64_instruction(code_gen, instr);
                break;
            case TARGET_X86_32:
                generate_x86_32_instruction(code_gen, instr);
                break;
            default:
                break;
        }
        instr = instr->next;
    }
}

// 收集所有临时变量及其类型
void collect_temp_variable_types(IRGenerator *ir_gen, 
                                 int *float_temps, int *float_count,
                                 int *int_temps, int *int_count,
                                 int *string_temps, int *string_count) {
    IRInstruction *instr = ir_gen->instructions;
    *float_count = *int_count = *string_count = 0;
    
    // 用于标记已处理的临时变量
    bool processed[50] = {false}; // 假设最多50个临时变量
    
    while (instr) {
        // 检查结果操作数
        if (instr->result && instr->result->type == OPERAND_TEMP) {
            int temp_id = instr->result->temp_id;
            if (temp_id < 50 && !processed[temp_id]) {
                processed[temp_id] = true;
                
                // 根据指令类型和操作数类型推断临时变量类型
                DataType temp_type = instr->result->data_type;
                
                // 特殊处理：比较操作结果总是int类型
                if (instr->opcode == IR_BINOP && 
                    (instr->binop == OP_GT || instr->binop == OP_LT || 
                     instr->binop == OP_EQ || instr->binop == OP_NE)) {
                    temp_type = TYPE_INT;
                }
                
                // 特殊处理：字符串常量赋值 - 通过检查值是否以引号开头
                bool is_string = false;
                if (instr->opcode == IR_LOAD_CONST && instr->operand1 && 
                    instr->operand1->type == OPERAND_CONST && 
                    instr->operand1->var_name && instr->operand1->var_name[0] == '"') {
                    is_string = true;
                }
                
                // 另一种检查方式：如果是ASSIGN指令且operand1是字符串常量
                if (instr->opcode == IR_ASSIGN && instr->operand1 && 
                    instr->operand1->type == OPERAND_CONST && 
                    instr->operand1->var_name && instr->operand1->var_name[0] == '"') {
                    is_string = true;
                }
                
                // 特殊处理：函数调用结果（printf返回int）
                if (instr->opcode == IR_CALL) {
                    temp_type = TYPE_INT;
                }
                
                // 分类存储
                if (is_string) {
                    string_temps[(*string_count)++] = temp_id;
                } else if (temp_type == TYPE_FLOAT) {
                    float_temps[(*float_count)++] = temp_id;
                } else {
                    int_temps[(*int_count)++] = temp_id;
                }
            }
        }
        instr = instr->next;
    }
}

// 简化版本：直接从生成的中间代码检测字符串临时变量
void analyze_temp_variables_simple(IRGenerator *ir_gen, 
                                   int *float_temps, int *float_count,
                                   int *int_temps, int *int_count,
                                   int *string_temps, int *string_count) {
    *float_count = *int_count = *string_count = 0;
    bool processed[50] = {false};
    bool is_string_temp[50] = {false};
    
    IRInstruction *instr = ir_gen->instructions;
    
    // 第一遍：检测字符串临时变量
    while (instr) {
        if (instr->opcode == IR_LOAD && instr->result && 
            instr->result->type == OPERAND_TEMP && instr->operand1 &&
            instr->operand1->type == OPERAND_VAR && instr->operand1->var_name &&
            instr->operand1->var_name[0] == '"') {
            int temp_id = instr->result->temp_id;
            if (temp_id < 50) {
                is_string_temp[temp_id] = true;
            }
        }
        instr = instr->next;
    }
    
    // 第二遍：分类所有临时变量
    instr = ir_gen->instructions;
    while (instr) {
        if (instr->result && instr->result->type == OPERAND_TEMP) {
            int temp_id = instr->result->temp_id;
            if (temp_id < 50 && !processed[temp_id]) {
                processed[temp_id] = true;
                
                if (is_string_temp[temp_id]) {
                    string_temps[(*string_count)++] = temp_id;
                } else if (instr->result->data_type == TYPE_FLOAT) {
                    float_temps[(*float_count)++] = temp_id;
                } else {
                    // 默认为int（包括比较结果、函数调用结果等）
                    int_temps[(*int_count)++] = temp_id;
                }
            }
        }
        instr = instr->next;
    }
}

// 生成C代码
void generate_c_code(IRGenerator *ir_gen, CodeGenerator *code_gen) {
    emit_instruction(code_gen, "// Auto-generated C code");
    emit_instruction(code_gen, "");
    emit_instruction(code_gen, "#include <stdio.h>");
    emit_instruction(code_gen, "#include <string.h>");
    emit_instruction(code_gen, "");
    
    IRInstruction *instr = ir_gen->instructions;
    bool in_function = false;
    
    // 用于跟踪printf参数的状态
    char printf_params[10][64];  // 最多10个参数
    int param_count = 0;
    bool collecting_params = false;
    
    // 收集临时变量类型 - 使用简化版本
    int float_temps[50], int_temps[50], string_temps[50];
    int float_count, int_count, string_count;
    analyze_temp_variables_simple(ir_gen, float_temps, &float_count, 
                                  int_temps, &int_count, string_temps, &string_count);
    
    while (instr) {
        switch (instr->opcode) {
            case IR_FUNC_BEGIN:
                if (instr->result && instr->result->type == OPERAND_FUNC) {
                    emit_instruction(code_gen, "int %s() {", instr->result->func_name);
                } else {
                    emit_instruction(code_gen, "int main() {");
                }
                in_function = true;
                
                // 动态声明变量
                emit_instruction(code_gen, "    int x;");
                emit_instruction(code_gen, "    float y, result;");
                
                // 动态生成float临时变量声明
                if (float_count > 0) {
                    char float_decl[1024] = "    float ";
                    for (int i = 0; i < float_count; i++) {
                        char temp_name[16];
                        snprintf(temp_name, sizeof(temp_name), "t%d", float_temps[i]);
                        strcat(float_decl, temp_name);
                        if (i < float_count - 1) strcat(float_decl, ", ");
                    }
                    strcat(float_decl, ";");
                    emit_instruction(code_gen, "%s", float_decl);
                }
                
                // 动态生成int临时变量声明
                if (int_count > 0) {
                    char int_decl[1024] = "    int ";
                    for (int i = 0; i < int_count; i++) {
                        char temp_name[16];
                        snprintf(temp_name, sizeof(temp_name), "t%d", int_temps[i]);
                        strcat(int_decl, temp_name);
                        if (i < int_count - 1) strcat(int_decl, ", ");
                    }
                    strcat(int_decl, ";");
                    emit_instruction(code_gen, "%s", int_decl);
                }
                
                // 动态生成string临时变量声明
                if (string_count > 0) {
                    char string_decl[1024] = "    char ";
                    for (int i = 0; i < string_count; i++) {
                        char temp_name[16];
                        snprintf(temp_name, sizeof(temp_name), "*t%d", string_temps[i]);
                        strcat(string_decl, temp_name);
                        if (i < string_count - 1) strcat(string_decl, ", ");
                    }
                    strcat(string_decl, ";  // string temporaries");
                    emit_instruction(code_gen, "%s", string_decl);
                }
                
                emit_instruction(code_gen, "");
                break;
                
            case IR_FUNC_END:
                emit_instruction(code_gen, "}");
                emit_instruction(code_gen, "");
                in_function = false;
                break;
                
            case IR_LOAD_CONST:
                if (instr->result && instr->operand1) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    
                    if (instr->result->type == OPERAND_TEMP) {
                        emit_instruction(code_gen, "    t%d = %s;", 
                            instr->result->temp_id, operand_str);
                    } else {
                        emit_instruction(code_gen, "    %s = %s;", 
                            instr->result->var_name, operand_str);
                    }
                }
                break;
                
            case IR_LOAD:
                if (instr->result && instr->operand1) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    
                    if (instr->result->type == OPERAND_TEMP) {
                        emit_instruction(code_gen, "    t%d = %s;", 
                            instr->result->temp_id, operand_str);
                    } else {
                        emit_instruction(code_gen, "    %s = %s;", 
                            instr->result->var_name, operand_str);
                    }
                }
                break;
                
            case IR_STORE:
            case IR_ASSIGN:
                if (instr->result && instr->operand1) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    
                    if (instr->result->type == OPERAND_TEMP) {
                        emit_instruction(code_gen, "    t%d = %s;", 
                            instr->result->temp_id, operand_str);
                    } else {
                        emit_instruction(code_gen, "    %s = %s;", 
                            instr->result->var_name, operand_str);
                    }
                }
                break;
                
            case IR_BINOP: {
                if (instr->result && instr->operand1 && instr->operand2) {
                    char left_str[64], right_str[64];
                    generate_operand_code(code_gen, instr->operand1, left_str, sizeof(left_str));
                    generate_operand_code(code_gen, instr->operand2, right_str, sizeof(right_str));
                    
                    const char *op_str = "";
                    switch (instr->binop) {
                        case OP_ADD: op_str = "+"; break;
                        case OP_SUB: op_str = "-"; break;
                        case OP_MUL: op_str = "*"; break;
                        case OP_DIV: op_str = "/"; break;
                        case OP_EQ: op_str = "=="; break;
                        case OP_NE: op_str = "!="; break;
                        case OP_LT: op_str = "<"; break;
                        case OP_GT: op_str = ">"; break;
                        case OP_LE: op_str = "<="; break;
                        case OP_GE: op_str = ">="; break;
                    }
                    
                    if (instr->result->type == OPERAND_TEMP) {
                        emit_instruction(code_gen, "    t%d = %s %s %s;", 
                            instr->result->temp_id, left_str, op_str, right_str);
                    } else {
                        emit_instruction(code_gen, "    %s = %s %s %s;", 
                            instr->result->var_name, left_str, op_str, right_str);
                    }
                }
                break;
            }
            
            case IR_CONVERT:
                if (instr->result && instr->operand1) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    
                    const char *cast_type = "";
                    if (instr->result->data_type == TYPE_FLOAT) {
                        cast_type = "(float)";
                    } else if (instr->result->data_type == TYPE_INT) {
                        cast_type = "(int)";
                    }
                    
                    if (instr->result->type == OPERAND_TEMP) {
                        emit_instruction(code_gen, "    t%d = %s%s;", 
                            instr->result->temp_id, cast_type, operand_str);
                    } else {
                        emit_instruction(code_gen, "    %s = %s%s;", 
                            instr->result->var_name, cast_type, operand_str);
                    }
                }
                break;
                
            case IR_PARAM:
                // 收集printf参数
                if (instr->operand1) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    
                    if (param_count < 10) {
                        strcpy(printf_params[param_count], operand_str);
                        param_count++;
                    }
                    collecting_params = true;
                }
                break;
                
            case IR_CALL:
                if (instr->operand1 && instr->operand1->type == OPERAND_FUNC) {
                    if (strcmp(instr->operand1->func_name, "printf") == 0) {
                        // 生成printf调用
                        if (param_count == 1) {
                            // 只有格式字符串，添加换行符
                            char fixed_format[128];
                            strcpy(fixed_format, printf_params[0]);
                            
                            // 如果是字符串常量且没有换行符，添加换行符
                            if (fixed_format[0] == '"') {
                                size_t len = strlen(fixed_format);
                                if (len > 1 && fixed_format[len-1] == '"') {
                                    fixed_format[len-1] = '\0';  // 移除结尾引号
                                    emit_instruction(code_gen, "    printf(%s\\n\");", fixed_format);
                                } else {
                                    emit_instruction(code_gen, "    printf(%s);", printf_params[0]);
                                }
                            } else {
                                emit_instruction(code_gen, "    printf(%s);", printf_params[0]);
                            }
                        } else if (param_count == 2) {
                            // 格式字符串 + 一个参数
                            emit_instruction(code_gen, "    printf(%s, %s);", 
                                printf_params[0], printf_params[1]);
                        } else if (param_count == 3) {
                            // 格式字符串 + 两个参数
                            emit_instruction(code_gen, "    printf(%s, %s, %s);", 
                                printf_params[0], printf_params[1], printf_params[2]);
                        } else {
                            // 更多参数的情况
                            char printf_call[256] = "    printf(";
                            for (int i = 0; i < param_count; i++) {
                                if (i > 0) strcat(printf_call, ", ");
                                strcat(printf_call, printf_params[i]);
                            }
                            strcat(printf_call, ");");
                            emit_instruction(code_gen, printf_call);
                        }
                        
                        // 重置参数收集状态
                        param_count = 0;
                        collecting_params = false;
                    }
                }
                break;
                
            case IR_RETURN:
                if (instr->operand1) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    emit_instruction(code_gen, "    return %s;", operand_str);
                } else {
                    emit_instruction(code_gen, "    return 0;");
                }
                break;
                
            case IR_LABEL:
                if (instr->operand1 && instr->operand1->type == OPERAND_LABEL) {
                    emit_instruction(code_gen, "%s:", instr->operand1->label_name);
                }
                break;
                
            case IR_GOTO:
                if (instr->operand1 && instr->operand1->type == OPERAND_LABEL) {
                    emit_instruction(code_gen, "    goto %s;", instr->operand1->label_name);
                }
                break;
                
            case IR_IF_GOTO:
                if (instr->operand1 && instr->operand2 && instr->operand2->type == OPERAND_LABEL) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    emit_instruction(code_gen, "    if (%s) goto %s;", 
                        operand_str, instr->operand2->label_name);
                }
                break;
                
            case IR_IF_FALSE_GOTO:
                if (instr->operand1 && instr->operand2 && instr->operand2->type == OPERAND_LABEL) {
                    char operand_str[64];
                    generate_operand_code(code_gen, instr->operand1, operand_str, sizeof(operand_str));
                    emit_instruction(code_gen, "    if (!%s) goto %s;", 
                        operand_str, instr->operand2->label_name);
                }
                break;
                
            default:
                break;
        }
        
        instr = instr->next;
    }
}

// 生成伪汇编代码
void generate_pseudo_code(IRGenerator *ir_gen, CodeGenerator *code_gen) {
    emit_instruction(code_gen, "; Pseudo assembly code");
    emit_instruction(code_gen, "; Target architecture: Educational pseudo instruction set");
    emit_instruction(code_gen, "");
    
    IRInstruction *instr = ir_gen->instructions;
    
    while (instr) {
        generate_pseudo_instruction(code_gen, instr);
        instr = instr->next;
    }
}

// 生成x86-64指令
void generate_x86_64_instruction(CodeGenerator *gen, IRInstruction *instr) {
    switch (instr->opcode) {
        case IR_FUNC_BEGIN:
            emit_function_prologue(gen, instr->operand1->func_name);
            break;
            
        case IR_FUNC_END:
            emit_function_epilogue(gen);
            break;
            
        case IR_LOAD: {
            RegisterType reg = allocate_register(gen, instr->result->data_type);
            if (reg != REG_NONE) {
                const char *reg_name = get_register_name(reg, instr->result->data_type, gen->target_arch);
                emit_instruction(gen, "    mov %s, [%s]", reg_name, instr->operand1->var_name);
                assign_temp_to_register(gen, instr->result->temp_id, reg, instr->result->data_type);
            }
            break;
        }
        
        case IR_STORE: {
            if (instr->operand1->type == OPERAND_TEMP) {
                RegisterType reg = get_temp_register(gen, instr->operand1->temp_id);
                if (reg != REG_NONE) {
                    const char *reg_name = get_register_name(reg, instr->operand1->data_type, gen->target_arch);
                    emit_instruction(gen, "    mov [%s], %s", instr->result->var_name, reg_name);
                }
            } else {
                char operand_str[64];
                generate_operand_code(gen, instr->operand1, operand_str, sizeof(operand_str));
                emit_instruction(gen, "    mov [%s], %s", instr->result->var_name, operand_str);
            }
            break;
        }
        
        case IR_BINOP: {
            RegisterType left_reg = load_operand_to_register(gen, instr->operand1);
            RegisterType right_reg = load_operand_to_register(gen, instr->operand2);
            
            if (left_reg != REG_NONE && right_reg != REG_NONE) {
                const char *left_name = get_register_name(left_reg, instr->operand1->data_type, gen->target_arch);
                const char *right_name = get_register_name(right_reg, instr->operand2->data_type, gen->target_arch);
                const char *op_instr = get_binop_instruction(instr->binop, gen->target_arch, instr->result->data_type);
                
                emit_instruction(gen, "    %s %s, %s", op_instr, left_name, right_name);
                
                // 结果存储在left_reg中
                assign_temp_to_register(gen, instr->result->temp_id, left_reg, instr->result->data_type);
                free_register(gen, right_reg);
            }
            break;
        }
        
        case IR_RETURN: {
            if (instr->operand1) {
                RegisterType reg = load_operand_to_register(gen, instr->operand1);
                if (reg != REG_NONE && reg != REG_RAX) {
                    const char *reg_name = get_register_name(reg, instr->operand1->data_type, gen->target_arch);
                    emit_instruction(gen, "    mov rax, %s", reg_name);
                }
            }
            emit_instruction(gen, "    ret");
            break;
        }
        
        case IR_LABEL:
            emit_label(gen, instr->operand1->label_name);
            break;
            
        case IR_GOTO:
            emit_instruction(gen, "    jmp %s", instr->operand1->label_name);
            break;
            
        case IR_IF_FALSE_GOTO: {
            RegisterType reg = load_operand_to_register(gen, instr->operand1);
            if (reg != REG_NONE) {
                const char *reg_name = get_register_name(reg, instr->operand1->data_type, gen->target_arch);
                emit_instruction(gen, "    test %s, %s", reg_name, reg_name);
                emit_instruction(gen, "    jz %s", instr->operand2->label_name);
            }
            break;
        }
        
        default:
            emit_comment(gen, "Unsupported instruction");
            break;
    }
}

// 生成x86-32指令
void generate_x86_32_instruction(CodeGenerator *gen, IRInstruction *instr) {
    // 类似x86-64，但使用32位寄存器
    generate_x86_64_instruction(gen, instr);
}

// 生成伪指令
void generate_pseudo_instruction(CodeGenerator *gen, IRInstruction *instr) {
    switch (instr->opcode) {
        case IR_FUNC_BEGIN:
            emit_instruction(gen, "FUNC_BEGIN %s", instr->operand1->func_name);
            break;
            
        case IR_FUNC_END:
            emit_instruction(gen, "FUNC_END");
            break;
            
        case IR_LOAD: {
            emit_instruction(gen, "    LOAD temp_%d, %s", 
                instr->result->temp_id, instr->operand1->var_name);
            break;
        }
        
        case IR_STORE: {
            char operand_str[64];
            generate_operand_code(gen, instr->operand1, operand_str, sizeof(operand_str));
            emit_instruction(gen, "    STORE %s, %s", instr->result->var_name, operand_str);
            break;
        }
        
        case IR_BINOP: {
            char left_str[64], right_str[64];
            generate_operand_code(gen, instr->operand1, left_str, sizeof(left_str));
            generate_operand_code(gen, instr->operand2, right_str, sizeof(right_str));
            
            const char *op_str = "";
            switch (instr->binop) {
                case OP_ADD: op_str = "ADD"; break;
                case OP_SUB: op_str = "SUB"; break;
                case OP_MUL: op_str = "MUL"; break;
                case OP_DIV: op_str = "DIV"; break;
                case OP_EQ: op_str = "EQ"; break;
                case OP_NE: op_str = "NE"; break;
                case OP_LT: op_str = "LT"; break;
                case OP_GT: op_str = "GT"; break;
                case OP_LE: op_str = "LE"; break;
                case OP_GE: op_str = "GE"; break;
            }
            
            emit_instruction(gen, "    %s temp_%d, %s, %s", 
                op_str, instr->result->temp_id, left_str, right_str);
            break;
        }
        
        case IR_RETURN: {
            if (instr->operand1) {
                char operand_str[64];
                generate_operand_code(gen, instr->operand1, operand_str, sizeof(operand_str));
                emit_instruction(gen, "    RETURN %s", operand_str);
            } else {
                emit_instruction(gen, "    RETURN");
            }
            break;
        }
        
        case IR_LABEL:
            emit_instruction(gen, "%s:", instr->operand1->label_name);
            break;
            
        case IR_GOTO:
            emit_instruction(gen, "    JUMP %s", instr->operand1->label_name);
            break;
            
        case IR_IF_FALSE_GOTO: {
            char operand_str[64];
            generate_operand_code(gen, instr->operand1, operand_str, sizeof(operand_str));
            emit_instruction(gen, "    JUMPZ %s, %s", operand_str, instr->operand2->label_name);
            break;
        }
        
        case IR_CONVERT: {
            char operand_str[64];
            generate_operand_code(gen, instr->operand1, operand_str, sizeof(operand_str));
            
            const char *type_str = (instr->result->data_type == TYPE_INT) ? "INT" : "FLOAT";
            emit_instruction(gen, "    CONVERT_%s temp_%d, %s", 
                type_str, instr->result->temp_id, operand_str);
            break;
        }
        
        default:
            emit_comment(gen, "Unknown instruction");
            break;
    }
}

// 寄存器分配
RegisterType allocate_register(CodeGenerator *gen, DataType data_type) {
    // 根据数据类型选择合适的寄存器
    bool need_float = needs_float_register(data_type);
    
    for (int i = 0; i < gen->register_count; i++) {
        if (gen->registers[i].is_available) {
            bool is_float_reg = (gen->registers[i].type >= REG_XMM0 && gen->registers[i].type <= REG_XMM3);
            
            if (need_float == is_float_reg) {
                gen->registers[i].is_available = false;
                gen->registers[i].temp_id = -1;
                gen->registers[i].data_type = data_type;
                gen->registers_used++;
                return gen->registers[i].type;
            }
        }
    }
    
    return REG_NONE; // 没有可用寄存器
}

void free_register(CodeGenerator *gen, RegisterType reg) {
    for (int i = 0; i < gen->register_count; i++) {
        if (gen->registers[i].type == reg) {
            gen->registers[i].is_available = true;
            gen->registers[i].temp_id = -1;
            gen->registers[i].data_type = TYPE_UNKNOWN;
            break;
        }
    }
}

RegisterType get_temp_register(CodeGenerator *gen, int temp_id) {
    for (int i = 0; i < gen->register_count; i++) {
        if (gen->registers[i].temp_id == temp_id) {
            return gen->registers[i].type;
        }
    }
    return REG_NONE;
}

void assign_temp_to_register(CodeGenerator *gen, int temp_id, RegisterType reg, DataType data_type) {
    for (int i = 0; i < gen->register_count; i++) {
        if (gen->registers[i].type == reg) {
            gen->registers[i].temp_id = temp_id;
            gen->registers[i].data_type = data_type;
            gen->registers[i].is_available = false;
            break;
        }
    }
}

RegisterType load_operand_to_register(CodeGenerator *gen, Operand *operand) {
    if (operand->type == OPERAND_TEMP) {
        return get_temp_register(gen, operand->temp_id);
    }
    
    RegisterType reg = allocate_register(gen, operand->data_type);
    if (reg != REG_NONE) {
        char operand_str[64];
        const char *reg_name = get_register_name(reg, operand->data_type, gen->target_arch);
        generate_operand_code(gen, operand, operand_str, sizeof(operand_str));
        
        if (gen->target_arch == TARGET_PSEUDO) {
            emit_instruction(gen, "    LOAD %s, %s", reg_name, operand_str);
        } else {
            emit_instruction(gen, "    mov %s, %s", reg_name, operand_str);
        }
    }
    
    return reg;
}

// 生成操作数代码
void generate_operand_code(CodeGenerator *gen, Operand *operand, char *buffer, size_t buffer_size) {
    if (!operand || !buffer) {
        if (buffer) snprintf(buffer, buffer_size, "null");
        return;
    }
    
    switch (operand->type) {
        case OPERAND_TEMP:
            snprintf(buffer, buffer_size, "t%d", operand->temp_id);
            break;
            
        case OPERAND_CONST:
            if (operand->data_type == TYPE_INT) {
                snprintf(buffer, buffer_size, "%d", operand->const_val.int_val);
            } else if (operand->data_type == TYPE_FLOAT) {
                snprintf(buffer, buffer_size, "%.2f", operand->const_val.float_val);
            } else {
                snprintf(buffer, buffer_size, "0");
            }
            break;
            
        case OPERAND_VAR:
            if (operand->var_name) {
                snprintf(buffer, buffer_size, "%s", operand->var_name);
            } else {
                snprintf(buffer, buffer_size, "unknown_var");
            }
            break;
            
        case OPERAND_LABEL:
            if (operand->label_name) {
                snprintf(buffer, buffer_size, "%s", operand->label_name);
            } else {
                snprintf(buffer, buffer_size, "unknown_label");
            }
            break;
            
        case OPERAND_FUNC:
            if (operand->func_name) {
                snprintf(buffer, buffer_size, "%s", operand->func_name);
            } else {
                snprintf(buffer, buffer_size, "unknown_func");
            }
            break;
            
        default:
            snprintf(buffer, buffer_size, "unknown");
            break;
    }
}

// 辅助函数
const char* get_register_name(RegisterType reg, DataType type, TargetArch arch) {
    switch (arch) {
        case TARGET_X86_64:
            switch (reg) {
                case REG_RAX: return (type == TYPE_FLOAT) ? "xmm0" : "rax";
                case REG_RBX: return (type == TYPE_FLOAT) ? "xmm1" : "rbx";
                case REG_RCX: return (type == TYPE_FLOAT) ? "xmm2" : "rcx";
                case REG_RDX: return (type == TYPE_FLOAT) ? "xmm3" : "rdx";
                case REG_XMM0: return "xmm0";
                case REG_XMM1: return "xmm1";
                case REG_XMM2: return "xmm2";
                case REG_XMM3: return "xmm3";
                default: return "unknown";
            }
            break;
            
        case TARGET_PSEUDO:
            // 在init_registers中已经设置了名称
            for (int i = 0; i < 8; i++) {
                if (i < 8 && reg == REG_RAX + i) {
                    return needs_float_register(type) ? 
                           (i < 4 ? "F0" : "F1") : 
                           (i < 4 ? "R0" : "R1");
                }
            }
            return "R0";
            
        default:
            return "unknown";
    }
}

bool needs_float_register(DataType type) {
    return type == TYPE_FLOAT;
}

const char* get_binop_instruction(BinOpType op, TargetArch arch, DataType type) {
    if (arch == TARGET_PSEUDO) {
        switch (op) {
            case OP_ADD: return "ADD";
            case OP_SUB: return "SUB";
            case OP_MUL: return "MUL";
            case OP_DIV: return "DIV";
            default: return "BINOP";
        }
    } else {
        switch (op) {
            case OP_ADD: return (type == TYPE_FLOAT) ? "addss" : "add";
            case OP_SUB: return (type == TYPE_FLOAT) ? "subss" : "sub";
            case OP_MUL: return (type == TYPE_FLOAT) ? "mulss" : "imul";
            case OP_DIV: return (type == TYPE_FLOAT) ? "divss" : "idiv";
            default: return "unknown";
        }
    }
}

// 发出指令
void emit_instruction(CodeGenerator *gen, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(gen->output_file, format, args);
    va_end(args);
    fprintf(gen->output_file, "\n");
    gen->instructions_generated++;
}

void emit_comment(CodeGenerator *gen, const char *comment) {
    if (gen->target_arch == TARGET_PSEUDO) {
        fprintf(gen->output_file, "; %s\n", comment);
    } else {
        fprintf(gen->output_file, "    # %s\n", comment);
    }
}

void emit_label(CodeGenerator *gen, const char *label) {
    fprintf(gen->output_file, "%s:\n", label);
}

void emit_function_prologue(CodeGenerator *gen, const char *func_name) {
    emit_instruction(gen, ".globl %s", func_name);
    emit_label(gen, func_name);
    if (gen->target_arch == TARGET_X86_64) {
        emit_instruction(gen, "    push rbp");
        emit_instruction(gen, "    mov rbp, rsp");
    } else if (gen->target_arch == TARGET_PSEUDO) {
        emit_instruction(gen, "    PUSH FP");
        emit_instruction(gen, "    MOVE FP, SP");
    }
}

void emit_function_epilogue(CodeGenerator *gen) {
    if (gen->target_arch == TARGET_X86_64) {
        emit_instruction(gen, "    mov rsp, rbp");
        emit_instruction(gen, "    pop rbp");
        emit_instruction(gen, "    ret");
    } else if (gen->target_arch == TARGET_PSEUDO) {
        emit_instruction(gen, "    MOVE SP, FP");
        emit_instruction(gen, "    POP FP");
        emit_instruction(gen, "    RETURN");
    }
}

void emit_file_header(CodeGenerator *gen) {
    switch (gen->target_arch) {
        case TARGET_X86_64:
            emit_instruction(gen, ".section .text");
            break;
        case TARGET_PSEUDO:
            emit_instruction(gen, "; Assembly code");
            emit_instruction(gen, "; Generated automatically");
            emit_instruction(gen, "");
            break;
        case TARGET_C_CODE:
            emit_instruction(gen, "// Auto-generated C code");
            emit_instruction(gen, "");
            break;
        default:
            break;
    }
}

void emit_file_footer(CodeGenerator *gen) {
    switch (gen->target_arch) {
        case TARGET_PSEUDO:
            emit_instruction(gen, "");
            emit_instruction(gen, "; Code generation completed");
            break;
        default:
            break;
    }
}

void print_codegen_stats(CodeGenerator *gen) {
    printf("Code Generation Statistics:\n");
    printf("  Generated instructions: %d\n", gen->instructions_generated);
    printf("  Registers used: %d\n", gen->registers_used);
    printf("  Stack space used: %d bytes\n", gen->stack_space_used);
    printf("===============================\n");
}
