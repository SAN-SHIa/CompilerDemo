#include "interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 初始化解释器
Interpreter* init_interpreter(void) {
    Interpreter *interp = (Interpreter*)malloc(sizeof(Interpreter));
    if (!interp) {
        fprintf(stderr, "Memory allocation failed for interpreter\n");
        return NULL;
    }
    
    interp->variables = NULL;
    interp->pc = 0;
    interp->running = true;
    
    // 初始化返回值
    interp->return_val.type = VAL_INT;
    interp->return_val.data.int_val = 0;
    
    return interp;
}

// 释放解释器内存
void free_interpreter(Interpreter *interp) {
    if (!interp) return;
    
    // 释放变量表
    VarEntry *current = interp->variables;
    while (current) {
        VarEntry *next = current->next;
        free(current->name);
        if (current->value.type == VAL_STRING && current->value.data.str_val) {
            free(current->value.data.str_val);
        }
        free(current);
        current = next;
    }
    
    free(interp);
}

// 设置变量值
void set_variable(Interpreter *interp, const char *name, RuntimeValue value) {
    if (!interp || !name) return;
    
    // 查找是否已存在该变量
    VarEntry *entry = interp->variables;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            // 如果原来是字符串类型，先释放内存
            if (entry->value.type == VAL_STRING && entry->value.data.str_val) {
                free(entry->value.data.str_val);
            }
            entry->value = value;
            // 如果新值是字符串，需要复制
            if (value.type == VAL_STRING && value.data.str_val) {
                entry->value.data.str_val = strdup(value.data.str_val);
            }
            return;
        }
        entry = entry->next;
    }
    
    // 如果不存在，创建新的变量表项
    entry = (VarEntry*)malloc(sizeof(VarEntry));
    if (!entry) {
        fprintf(stderr, "Memory allocation failed for variable entry\n");
        return;
    }
    
    entry->name = strdup(name);
    entry->value = value;
    if (value.type == VAL_STRING && value.data.str_val) {
        entry->value.data.str_val = strdup(value.data.str_val);
    }
    entry->next = interp->variables;
    interp->variables = entry;
}

// 获取变量值
RuntimeValue get_variable(Interpreter *interp, const char *name) {
    RuntimeValue error_val = {VAL_INT, {.int_val = 0}};
    
    if (!interp || !name) return error_val;
    
    VarEntry *entry = interp->variables;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    
    // 变量未找到
    fprintf(stderr, "Variable '%s' not found\n", name);
    return error_val;
}

// 打印运行时值
void print_runtime_value(RuntimeValue value) {
    switch (value.type) {
        case VAL_INT:
            printf("%d", value.data.int_val);
            break;
        case VAL_FLOAT:
            printf("%f", value.data.float_val);
            break;
        case VAL_STRING:
            if (value.data.str_val) {
                printf("%s", value.data.str_val);
            } else {
                printf("(null)");
            }
            break;
        default:
            printf("(unknown type)");
            break;
    }
}

// 执行操作数，返回运行时值
RuntimeValue execute_operand(Interpreter *interp, Operand *operand) {
    RuntimeValue result = {VAL_INT, {.int_val = 0}};
    
    if (!operand) return result;
    
    switch (operand->type) {
        case OPERAND_CONST:
            if (operand->data_type == TYPE_INT) {
                result.type = VAL_INT;
                result.data.int_val = operand->const_val.int_val;
            } else if (operand->data_type == TYPE_FLOAT) {
                result.type = VAL_FLOAT;
                result.data.float_val = operand->const_val.float_val;
            }
            break;
            
        case OPERAND_VAR:
            result = get_variable(interp, operand->var_name);
            break;
            
        case OPERAND_TEMP:
            {
                // 临时变量名格式：t1, t2, ...
                char temp_name[32];
                snprintf(temp_name, sizeof(temp_name), "t%d", operand->temp_id);
                result = get_variable(interp, temp_name);
            }
            break;
            
        case OPERAND_LABEL:
            // 标签作为操作数时，返回标签名作为字符串
            result.type = VAL_STRING;
            result.data.str_val = operand->label_name;
            break;
            
        case OPERAND_FUNC:
            // 函数名作为操作数时，返回函数名作为字符串
            result.type = VAL_STRING;
            result.data.str_val = operand->func_name;
            break;
            
        default:
            fprintf(stderr, "Unsupported operand type in execution\n");
            break;
    }
    
    return result;
}

// 执行二元运算
RuntimeValue execute_binop(RuntimeValue left, RuntimeValue right, BinOpType op) {
    RuntimeValue result = {VAL_INT, {.int_val = 0}};
    
    // 类型转换：如果其中一个是float，结果为float
    if (left.type == VAL_FLOAT || right.type == VAL_FLOAT) {
        float left_val = (left.type == VAL_FLOAT) ? left.data.float_val : (float)left.data.int_val;
        float right_val = (right.type == VAL_FLOAT) ? right.data.float_val : (float)right.data.int_val;
        
        result.type = VAL_FLOAT;
        switch (op) {
            case OP_ADD:
                result.data.float_val = left_val + right_val;
                break;
            case OP_SUB:
                result.data.float_val = left_val - right_val;
                break;
            case OP_MUL:
                result.data.float_val = left_val * right_val;
                break;
            case OP_DIV:
                if (right_val != 0.0) {
                    result.data.float_val = left_val / right_val;
                } else {
                    fprintf(stderr, "Division by zero\n");
                    result.data.float_val = 0.0;
                }
                break;
            case OP_EQ:
                result.type = VAL_INT;
                result.data.int_val = (fabs(left_val - right_val) < 1e-6) ? 1 : 0;
                break;
            case OP_NE:
                result.type = VAL_INT;
                result.data.int_val = (fabs(left_val - right_val) >= 1e-6) ? 1 : 0;
                break;
            case OP_LT:
                result.type = VAL_INT;
                result.data.int_val = (left_val < right_val) ? 1 : 0;
                break;
            case OP_GT:
                result.type = VAL_INT;
                result.data.int_val = (left_val > right_val) ? 1 : 0;
                break;
            case OP_LE:
                result.type = VAL_INT;
                result.data.int_val = (left_val <= right_val) ? 1 : 0;
                break;
            case OP_GE:
                result.type = VAL_INT;
                result.data.int_val = (left_val >= right_val) ? 1 : 0;
                break;
        }
    } else {
        // 都是整数
        int left_val = left.data.int_val;
        int right_val = right.data.int_val;
        
        result.type = VAL_INT;
        switch (op) {
            case OP_ADD:
                result.data.int_val = left_val + right_val;
                break;
            case OP_SUB:
                result.data.int_val = left_val - right_val;
                break;
            case OP_MUL:
                result.data.int_val = left_val * right_val;
                break;
            case OP_DIV:
                if (right_val != 0) {
                    result.data.int_val = left_val / right_val;
                } else {
                    fprintf(stderr, "Division by zero\n");
                    result.data.int_val = 0;
                }
                break;
            case OP_EQ:
                result.data.int_val = (left_val == right_val) ? 1 : 0;
                break;
            case OP_NE:
                result.data.int_val = (left_val != right_val) ? 1 : 0;
                break;
            case OP_LT:
                result.data.int_val = (left_val < right_val) ? 1 : 0;
                break;
            case OP_GT:
                result.data.int_val = (left_val > right_val) ? 1 : 0;
                break;
            case OP_LE:
                result.data.int_val = (left_val <= right_val) ? 1 : 0;
                break;
            case OP_GE:
                result.data.int_val = (left_val >= right_val) ? 1 : 0;
                break;
        }
    }
    
    return result;
}

// 将指令列表转换为数组以便跳转
typedef struct {
    IRInstruction **instructions;
    int count;
    int *label_positions;  // 标签位置映射
    char **label_names;    // 标签名称列表
    int label_count;
} InstructionArray;

// 构建指令数组
InstructionArray* build_instruction_array(IRGenerator *ir_gen) {
    InstructionArray *arr = (InstructionArray*)malloc(sizeof(InstructionArray));
    if (!arr) return NULL;
    
    // 计算指令数量
    int count = 0;
    IRInstruction *instr = ir_gen->instructions;
    while (instr) {
        count++;
        instr = instr->next;
    }
    
    arr->instructions = (IRInstruction**)malloc(count * sizeof(IRInstruction*));
    arr->label_positions = (int*)malloc(100 * sizeof(int));  // 假设最多100个标签
    arr->label_names = (char**)malloc(100 * sizeof(char*));
    arr->count = count;
    arr->label_count = 0;
    
    if (!arr->instructions || !arr->label_positions || !arr->label_names) {
        free(arr->instructions);
        free(arr->label_positions);
        free(arr->label_names);
        free(arr);
        return NULL;
    }
    
    // 填充指令数组并记录标签位置
    instr = ir_gen->instructions;
    for (int i = 0; i < count; i++) {
        arr->instructions[i] = instr;
        
        // 如果是标签指令，记录位置
        if (instr->opcode == IR_LABEL && instr->result && instr->result->type == OPERAND_LABEL) {
            arr->label_names[arr->label_count] = strdup(instr->result->label_name);
            arr->label_positions[arr->label_count] = i;
            arr->label_count++;
        }
        
        instr = instr->next;
    }
    
    return arr;
}

// 释放指令数组
void free_instruction_array(InstructionArray *arr) {
    if (!arr) return;
    
    for (int i = 0; i < arr->label_count; i++) {
        free(arr->label_names[i]);
    }
    
    free(arr->instructions);
    free(arr->label_positions);
    free(arr->label_names);
    free(arr);
}

// 查找标签位置
int find_label_position(InstructionArray *arr, const char *label_name) {
    for (int i = 0; i < arr->label_count; i++) {
        if (strcmp(arr->label_names[i], label_name) == 0) {
            return arr->label_positions[i];
        }
    }
    return -1;  // 未找到
}

// 设置操作数变量值
void set_operand_variable(Interpreter *interp, Operand *operand, RuntimeValue value) {
    if (!operand) return;
    
    if (operand->type == OPERAND_VAR) {
        set_variable(interp, operand->var_name, value);
    } else if (operand->type == OPERAND_TEMP) {
        // 临时变量名格式：t1, t2, ...
        char temp_name[32];
        snprintf(temp_name, sizeof(temp_name), "t%d", operand->temp_id);
        set_variable(interp, temp_name, value);
    }
}

// 执行中间代码
void execute_ir(Interpreter *interp, IRGenerator *ir_gen) {
    if (!interp || !ir_gen || !ir_gen->instructions) {
        return;
    }
    
    // 构建指令数组
    InstructionArray *arr = build_instruction_array(ir_gen);
    if (!arr) {
        fprintf(stderr, "Failed to build instruction array\n");
        return;
    }
    
    interp->pc = 0;
    interp->running = true;
    
    while (interp->running && interp->pc < arr->count) {
        IRInstruction *instr = arr->instructions[interp->pc];
        
        switch (instr->opcode) {
            case IR_LOAD_CONST:
                {
                    RuntimeValue value;
                    if (instr->operand1->data_type == TYPE_INT) {
                        value.type = VAL_INT;
                        value.data.int_val = instr->operand1->const_val.int_val;
                    } else {
                        value.type = VAL_FLOAT;
                        value.data.float_val = instr->operand1->const_val.float_val;
                    }
                    set_operand_variable(interp, instr->result, value);
                }
                break;
                
            case IR_LOAD:
                {
                    RuntimeValue value = execute_operand(interp, instr->operand1);
                    set_operand_variable(interp, instr->result, value);
                }
                break;
                
            case IR_STORE:
                {
                    RuntimeValue value = execute_operand(interp, instr->operand1);
                    set_operand_variable(interp, instr->result, value);
                }
                break;
                
            case IR_ASSIGN:
                {
                    RuntimeValue value = execute_operand(interp, instr->operand1);
                    set_operand_variable(interp, instr->result, value);
                }
                break;
                
            case IR_BINOP:
                {
                    RuntimeValue left = execute_operand(interp, instr->operand1);
                    RuntimeValue right = execute_operand(interp, instr->operand2);
                    RuntimeValue result = execute_binop(left, right, instr->binop);
                    set_operand_variable(interp, instr->result, result);
                }
                break;
                
            case IR_GOTO:
                {
                    if (instr->operand1 && instr->operand1->type == OPERAND_LABEL) {
                        int pos = find_label_position(arr, instr->operand1->label_name);
                        if (pos >= 0) {
                            interp->pc = pos;
                            continue;  // 跳过pc自增
                        } else {
                            fprintf(stderr, "Label '%s' not found\n", instr->operand1->label_name);
                        }
                    }
                }
                break;
                
            case IR_IF_GOTO:
                {
                    RuntimeValue cond = execute_operand(interp, instr->operand1);
                    int cond_value = 0;
                    if (cond.type == VAL_INT) {
                        cond_value = cond.data.int_val;
                    } else if (cond.type == VAL_FLOAT) {
                        cond_value = (cond.data.float_val != 0.0) ? 1 : 0;
                    }
                    
                    if (cond_value && instr->operand2 && instr->operand2->type == OPERAND_LABEL) {
                        int pos = find_label_position(arr, instr->operand2->label_name);
                        if (pos >= 0) {
                            interp->pc = pos;
                            continue;  // 跳过pc自增
                        } else {
                            fprintf(stderr, "Label '%s' not found\n", instr->operand2->label_name);
                        }
                    }
                }
                break;
                
            case IR_IF_FALSE_GOTO:
                {
                    RuntimeValue cond = execute_operand(interp, instr->operand1);
                    int cond_value = 0;
                    if (cond.type == VAL_INT) {
                        cond_value = cond.data.int_val;
                    } else if (cond.type == VAL_FLOAT) {
                        cond_value = (cond.data.float_val != 0.0) ? 1 : 0;
                    }
                    
                    if (!cond_value && instr->operand2 && instr->operand2->type == OPERAND_LABEL) {
                        int pos = find_label_position(arr, instr->operand2->label_name);
                        if (pos >= 0) {
                            interp->pc = pos;
                            continue;  // 跳过pc自增
                        } else {
                            fprintf(stderr, "Label '%s' not found\n", instr->operand2->label_name);
                        }
                    }
                }
                break;
                
            case IR_RETURN:
                {
                    if (instr->operand1) {
                        interp->return_val = execute_operand(interp, instr->operand1);
                    }
                    interp->running = false;
                }
                break;
                
            case IR_LABEL:
                // 标签指令不需要执行，只是占位
                break;
                
            case IR_CONVERT:
                {
                    RuntimeValue value = execute_operand(interp, instr->operand1);
                    RuntimeValue converted_value;
                    
                    // 根据结果操作数的数据类型进行转换
                    if (instr->result && instr->result->data_type == TYPE_FLOAT) {
                        converted_value.type = VAL_FLOAT;
                        if (value.type == VAL_INT) {
                            converted_value.data.float_val = (float)value.data.int_val;
                        } else {
                            converted_value.data.float_val = value.data.float_val;
                        }
                    } else {
                        converted_value.type = VAL_INT;
                        if (value.type == VAL_FLOAT) {
                            converted_value.data.int_val = (int)value.data.float_val;
                        } else {
                            converted_value.data.int_val = value.data.int_val;
                        }
                    }
                    
                    set_operand_variable(interp, instr->result, converted_value);
                }
                break;
                
            case IR_PARAM:
                // 参数指令暂时不处理，实际的函数调用会处理参数
                break;
                
            case IR_FUNC_BEGIN:
            case IR_FUNC_END:
                // 函数开始和结束标记，暂时不需要特殊处理
                break;
                
            case IR_CALL:
                // 简单的函数调用处理（printf等）
                if (instr->operand1 && instr->operand1->type == OPERAND_FUNC) {
                    if (strcmp(instr->operand1->func_name, "printf") == 0) {
                        // 这里可以添加printf的简单实现
                        printf("printf called\n");
                    }
                }
                break;
                
            default:
                fprintf(stderr, "Unsupported IR instruction: %d\n", instr->opcode);
                break;
        }
        
        interp->pc++;
    }
    
    free_instruction_array(arr);
}

// 创建运行时值的辅助函数
RuntimeValue create_int_value(int value) {
    RuntimeValue val;
    val.type = VAL_INT;
    val.data.int_val = value;
    return val;
}

RuntimeValue create_float_value(float value) {
    RuntimeValue val;
    val.type = VAL_FLOAT;
    val.data.float_val = value;
    return val;
}

RuntimeValue create_string_value(const char *str) {
    RuntimeValue val;
    val.type = VAL_STRING;
    val.data.str_val = str ? strdup(str) : NULL;
    return val;
}
