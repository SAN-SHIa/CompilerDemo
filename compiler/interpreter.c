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
    
    // 初始化参数栈
    interp->max_params = 10;
    interp->param_stack = (RuntimeValue*)malloc(interp->max_params * sizeof(RuntimeValue));
    interp->param_count = 0;
    
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
    
    // 释放参数栈
    if (interp->param_stack) {
        for (int i = 0; i < interp->param_count; i++) {
            if (interp->param_stack[i].type == VAL_STRING && interp->param_stack[i].data.str_val) {
                free(interp->param_stack[i].data.str_val);
            }
        }
        free(interp->param_stack);
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
            printf("%.6f", value.data.float_val);
            break;
        case VAL_STRING:
            if (value.data.str_val) {
                printf("%s", value.data.str_val);
            } else {
                printf("(null)");
            }
            break;
    }
}

// 执行printf函数
void execute_printf(Interpreter *interp) {
    if (interp->param_count == 0) {
        return;
    }
    
    // 第一个参数应该是格式字符串
    RuntimeValue format = interp->param_stack[0];
    if (format.type != VAL_STRING || !format.data.str_val) {
        printf("Error: printf format string is not valid\n");
        return;
    }
    
    char *fmt = format.data.str_val;
    int param_index = 1;
    
    printf("Output: ");
    for (int i = 0; fmt[i]; i++) {
        if (fmt[i] == '%' && fmt[i+1] && param_index < interp->param_count) {
            char specifier = fmt[i+1];
            switch (specifier) {
                case 'd':
                case 'i':
                    if (interp->param_stack[param_index].type == VAL_INT) {
                        printf("%d", interp->param_stack[param_index].data.int_val);
                    } else if (interp->param_stack[param_index].type == VAL_FLOAT) {
                        printf("%d", (int)interp->param_stack[param_index].data.float_val);
                    }
                    break;
                case 'f':
                    if (interp->param_stack[param_index].type == VAL_FLOAT) {
                        printf("%.6f", interp->param_stack[param_index].data.float_val);
                    } else if (interp->param_stack[param_index].type == VAL_INT) {
                        printf("%.6f", (float)interp->param_stack[param_index].data.int_val);
                    }
                    break;
                case 's':
                    if (interp->param_stack[param_index].type == VAL_STRING) {
                        printf("%s", interp->param_stack[param_index].data.str_val);
                    }
                    break;
                case '%':
                    printf("%%");
                    param_index--; // 不消耗参数
                    break;
                default:
                    printf("%%%c", specifier);
                    break;
            }
            i++; // 跳过格式说明符
            param_index++;
        } else {
            printf("%c", fmt[i]);
        }
    }
    printf("\n");
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
            // 检查是否是字符串字面量（以引号开始和结束）
            if (operand->var_name && operand->var_name[0] == '"') {
                result.type = VAL_STRING;
                // 去掉首尾引号并复制字符串
                int len = strlen(operand->var_name);
                if (len >= 2 && operand->var_name[len-1] == '"') {
                    char *str = malloc(len - 1);
                    strncpy(str, operand->var_name + 1, len - 2);
                    str[len - 2] = '\0';
                    // 处理转义字符
                    char *processed = malloc(len);
                    int j = 0;
                    for (int i = 0; str[i]; i++) {
                        if (str[i] == '\\' && str[i+1] == 'n') {
                            processed[j++] = '\n';
                            i++;
                        } else if (str[i] == '\\' && str[i+1] == 't') {
                            processed[j++] = '\t';
                            i++;
                        } else {
                            processed[j++] = str[i];
                        }
                    }
                    processed[j] = '\0';
                    free(str);
                    result.data.str_val = processed;
                } else {
                    result.data.str_val = strdup(operand->var_name);
                }
            } else {
                result = get_variable(interp, operand->var_name);
            }
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
            result.data.str_val = strdup(operand->label_name);
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
    int index = 0;
    instr = ir_gen->instructions;
    while (instr && index < count) {
        arr->instructions[index] = instr;
        
        // 如果是标签指令，记录位置
        if (instr->opcode == IR_LABEL && instr->operand1 && instr->operand1->type == OPERAND_LABEL) {
            if (arr->label_count < 100) {
                arr->label_names[arr->label_count] = strdup(instr->operand1->label_name);
                arr->label_positions[arr->label_count] = index;
                arr->label_count++;
                printf("Debug: Found label '%s' at position %d\n", instr->operand1->label_name, index);
            }
        }
        
        index++;
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
                    
                    printf("Debug: IF_FALSE_GOTO condition value: %d\n", cond_value);
                    
                    if (!cond_value && instr->operand2 && instr->operand2->type == OPERAND_LABEL) {
                        int pos = find_label_position(arr, instr->operand2->label_name);
                        if (pos >= 0) {
                            printf("Debug: Jumping to label '%s' at position %d\n", instr->operand2->label_name, pos);
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
                // 参数指令 - 将参数压入栈
                {
                    RuntimeValue param_value = execute_operand(interp, instr->operand1);
                    if (interp->param_count < interp->max_params) {
                        // 复制字符串值
                        if (param_value.type == VAL_STRING && param_value.data.str_val) {
                            interp->param_stack[interp->param_count].type = VAL_STRING;
                            interp->param_stack[interp->param_count].data.str_val = strdup(param_value.data.str_val);
                        } else {
                            interp->param_stack[interp->param_count] = param_value;
                        }
                        interp->param_count++;
                        printf("Debug: Added parameter %d: ", interp->param_count);
                        print_runtime_value(param_value);
                        printf("\n");
                    }
                }
                break;
                
            case IR_FUNC_BEGIN:
            case IR_FUNC_END:
                // 函数开始和结束标记，暂时不需要特殊处理
                break;
                
            case IR_CALL:
                // 函数调用处理
                if (instr->operand1 && instr->operand1->type == OPERAND_FUNC) {
                    if (strcmp(instr->operand1->func_name, "printf") == 0) {
                        // 实现简单的printf功能
                        execute_printf(interp);
                    }
                }
                // 清空参数栈
                for (int i = 0; i < interp->param_count; i++) {
                    if (interp->param_stack[i].type == VAL_STRING && interp->param_stack[i].data.str_val) {
                        free(interp->param_stack[i].data.str_val);
                    }
                }
                interp->param_count = 0;
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
