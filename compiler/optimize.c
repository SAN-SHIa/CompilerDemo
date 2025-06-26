#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "optimize.h"

// 初始化优化器
Optimizer* init_optimizer(IRGenerator *ir_gen, int optimization_level) {
    Optimizer *opt = (Optimizer*)malloc(sizeof(Optimizer));
    opt->ir_gen = ir_gen;
    opt->optimization_level = optimization_level;
    opt->eliminated_instructions = 0;
    opt->folded_constants = 0;
    opt->propagated_constants = 0;
    
    // 根据优化级别设置启用的优化
    set_optimization_level(opt, optimization_level);
    
    return opt;
}

// 释放优化器
void free_optimizer(Optimizer *opt) {
    free(opt);
}

// 设置优化级别
void set_optimization_level(Optimizer *opt, int level) {
    // 默认关闭所有优化
    for (int i = 0; i < 6; i++) {
        opt->optimizations_enabled[i] = false;
    }
    
    switch (level) {
        case 3: // -O3: 最高优化
            opt->optimizations_enabled[OPT_COMMON_SUBEXPRESSION] = true;
            // fallthrough
        case 2: // -O2: 高优化
            opt->optimizations_enabled[OPT_COPY_PROPAGATION] = true;
            opt->optimizations_enabled[OPT_DEAD_CODE_ELIMINATION] = true;
            // fallthrough
        case 1: // -O1: 基本优化
            opt->optimizations_enabled[OPT_CONSTANT_FOLDING] = true;
            opt->optimizations_enabled[OPT_CONSTANT_PROPAGATION] = true;
            opt->optimizations_enabled[OPT_ALGEBRAIC_SIMPLIFICATION] = true;
            break;
        case 0: // -O0: 无优化
        default:
            break;
    }
}

// 主优化函数
void optimize_ir(Optimizer *opt) {
    printf("\n=== Start Optimization (Level %d) ===\n", opt->optimization_level);
    
    if (opt->optimization_level == 0) {
        printf("Optimization disabled\n");
        return;
    }
    
    // 多遍优化，直到没有更多改进
    bool changed = true;
    int pass = 1;
    
    while (changed && pass <= 5) { // 最多5遍
        changed = false;
        printf("Optimization pass %d:\n", pass);
        
        int old_eliminated = opt->eliminated_instructions;
        int old_folded = opt->folded_constants;
        int old_propagated = opt->propagated_constants;
        
        if (opt->optimizations_enabled[OPT_CONSTANT_FOLDING]) {
            constant_folding(opt);
        }
        
        if (opt->optimizations_enabled[OPT_CONSTANT_PROPAGATION]) {
            constant_propagation(opt);
        }
        
        if (opt->optimizations_enabled[OPT_ALGEBRAIC_SIMPLIFICATION]) {
            algebraic_simplification(opt);
        }
        
        if (opt->optimizations_enabled[OPT_COPY_PROPAGATION]) {
            copy_propagation(opt);
        }
        
        if (opt->optimizations_enabled[OPT_DEAD_CODE_ELIMINATION]) {
            dead_code_elimination(opt);
        }
        
        if (opt->optimizations_enabled[OPT_COMMON_SUBEXPRESSION]) {
            common_subexpression_elimination(opt);
        }
        
        // 检查是否有变化
        if (opt->eliminated_instructions > old_eliminated ||
            opt->folded_constants > old_folded ||
            opt->propagated_constants > old_propagated) {
            changed = true;
        }
        
        pass++;
    }
    
    print_optimization_stats(opt);
}

// 常量折叠
void constant_folding(Optimizer *opt) {
    ConstantTable *table = init_constant_table();
    IRInstruction *instr = opt->ir_gen->instructions;
    
    while (instr) {
        IRInstruction *next = instr->next;
        
        switch (instr->opcode) {
            case IR_LOAD_CONST:
            case IR_LOAD:
                if (instr->operand1->type == OPERAND_CONST) {
                    ConstantValue value;
                    if (instr->operand1->data_type == TYPE_INT) {
                        value = create_int_constant(instr->operand1->const_val.int_val);
                    } else {
                        value = create_float_constant(instr->operand1->const_val.float_val);
                    }
                    if (instr->result->type == OPERAND_TEMP) {
                        add_constant(table, instr->result->temp_id, value);
                    }
                }
                break;
                
            case IR_BINOP: {
                if (is_constant_operand(instr->operand1, table) && 
                    is_constant_operand(instr->operand2, table)) {
                    
                    ConstantValue left = get_operand_constant(instr->operand1, table);
                    ConstantValue right = get_operand_constant(instr->operand2, table);
                    
                    if (can_evaluate_binop(instr->binop, left, right)) {
                        ConstantValue result = evaluate_binop(instr->binop, left, right);
                        
                        // 替换指令为常量加载
                        instr->opcode = IR_LOAD_CONST;
                        free_operand(instr->operand1);
                        free_operand(instr->operand2);
                        
                        if (result.type == TYPE_INT) {
                            instr->operand1 = create_int_const_operand(result.value.int_val);
                        } else {
                            instr->operand1 = create_float_const_operand(result.value.float_val);
                        }
                        instr->operand2 = NULL;
                        
                        if (instr->result->type == OPERAND_TEMP) {
                            add_constant(table, instr->result->temp_id, result);
                        }
                        
                        opt->folded_constants++;
                    }
                }
                break;
            }
            
            case IR_CONVERT: {
                if (is_constant_operand(instr->operand1, table)) {
                    ConstantValue value = get_operand_constant(instr->operand1, table);
                    ConstantValue converted;
                    
                    if (instr->result->data_type == TYPE_INT && value.type == TYPE_FLOAT) {
                        converted = create_int_constant((int)value.value.float_val);
                    } else if (instr->result->data_type == TYPE_FLOAT && value.type == TYPE_INT) {
                        converted = create_float_constant((float)value.value.int_val);
                    } else {
                        converted = value;
                    }
                    
                    // 替换为常量加载
                    instr->opcode = IR_LOAD_CONST;
                    free_operand(instr->operand1);
                    
                    if (converted.type == TYPE_INT) {
                        instr->operand1 = create_int_const_operand(converted.value.int_val);
                    } else {
                        instr->operand1 = create_float_const_operand(converted.value.float_val);
                    }
                    
                    if (instr->result->type == OPERAND_TEMP) {
                        add_constant(table, instr->result->temp_id, converted);
                    }
                    
                    opt->folded_constants++;
                }
                break;
            }
            
            default:
                break;
        }
        
        instr = next;
    }
    
    free_constant_table(table);
}

// 常量传播
void constant_propagation(Optimizer *opt) {
    ConstantTable *table = init_constant_table();
    IRInstruction *instr = opt->ir_gen->instructions;
    
    while (instr) {
        // 更新常量表
        switch (instr->opcode) {
            case IR_LOAD_CONST:
                if (instr->result->type == OPERAND_TEMP && instr->operand1->type == OPERAND_CONST) {
                    ConstantValue value;
                    if (instr->operand1->data_type == TYPE_INT) {
                        value = create_int_constant(instr->operand1->const_val.int_val);
                    } else {
                        value = create_float_constant(instr->operand1->const_val.float_val);
                    }
                    add_constant(table, instr->result->temp_id, value);
                }
                break;
                
            case IR_STORE:
                if (instr->operand1->type == OPERAND_TEMP && instr->result->type == OPERAND_VAR) {
                    ConstantValue *value = lookup_temp_constant(table, instr->operand1->temp_id);
                    if (value && value->is_constant) {
                        add_var_constant(table, instr->result->var_name, *value);
                    } else {
                        remove_var_constant(table, instr->result->var_name);
                    }
                }
                break;
                
            default:
                // 如果指令修改了某个临时变量，从常量表中移除
                if (instr->result && instr->result->type == OPERAND_TEMP) {
                    remove_temp_constant(table, instr->result->temp_id);
                }
                break;
        }
        
        // 传播常量
        bool changed = false;
        
        if (instr->operand1 && instr->operand1->type == OPERAND_TEMP) {
            ConstantValue *value = lookup_temp_constant(table, instr->operand1->temp_id);
            if (value && value->is_constant) {
                free_operand(instr->operand1);
                if (value->type == TYPE_INT) {
                    instr->operand1 = create_int_const_operand(value->value.int_val);
                } else {
                    instr->operand1 = create_float_const_operand(value->value.float_val);
                }
                changed = true;
            }
        }
        
        if (instr->operand2 && instr->operand2->type == OPERAND_TEMP) {
            ConstantValue *value = lookup_temp_constant(table, instr->operand2->temp_id);
            if (value && value->is_constant) {
                free_operand(instr->operand2);
                if (value->type == TYPE_INT) {
                    instr->operand2 = create_int_const_operand(value->value.int_val);
                } else {
                    instr->operand2 = create_float_const_operand(value->value.float_val);
                }
                changed = true;
            }
        }
        
        if (changed) {
            opt->propagated_constants++;
        }
        
        instr = instr->next;
    }
    
    free_constant_table(table);
}

// 代数简化
void algebraic_simplification(Optimizer *opt) {
    ConstantTable *table = init_constant_table();
    IRInstruction *instr = opt->ir_gen->instructions;
    
    while (instr) {
        IRInstruction *next = instr->next;
        
        if (instr->opcode == IR_BINOP) {
            ConstantValue left_const = get_operand_constant(instr->operand1, table);
            ConstantValue right_const = get_operand_constant(instr->operand2, table);
            
            bool simplified = false;
            
            // x + 0 = x, x - 0 = x
            if ((instr->binop == OP_ADD || instr->binop == OP_SUB) && 
                right_const.is_constant && is_zero_const_value(right_const)) {
                
                // 替换为简单赋值
                instr->opcode = IR_ASSIGN;
                free_operand(instr->operand2);
                instr->operand2 = NULL;
                simplified = true;
            }
            // 0 + x = x
            else if (instr->binop == OP_ADD && 
                     left_const.is_constant && is_zero_const_value(left_const)) {
                
                instr->opcode = IR_ASSIGN;
                free_operand(instr->operand1);
                instr->operand1 = instr->operand2;
                instr->operand2 = NULL;
                simplified = true;
            }
            // x * 1 = x, x / 1 = x
            else if ((instr->binop == OP_MUL || instr->binop == OP_DIV) && 
                     right_const.is_constant && is_one_const_value(right_const)) {
                
                instr->opcode = IR_ASSIGN;
                free_operand(instr->operand2);
                instr->operand2 = NULL;
                simplified = true;
            }
            // 1 * x = x
            else if (instr->binop == OP_MUL && 
                     left_const.is_constant && is_one_const_value(left_const)) {
                
                instr->opcode = IR_ASSIGN;
                free_operand(instr->operand1);
                instr->operand1 = instr->operand2;
                instr->operand2 = NULL;
                simplified = true;
            }
            // x * 0 = 0, 0 * x = 0
            else if (instr->binop == OP_MUL && 
                     ((left_const.is_constant && is_zero_const_value(left_const)) ||
                      (right_const.is_constant && is_zero_const_value(right_const)))) {
                
                instr->opcode = IR_LOAD_CONST;
                free_operand(instr->operand1);
                free_operand(instr->operand2);
                instr->operand1 = create_int_const_operand(0);
                instr->operand2 = NULL;
                simplified = true;
            }
            
            if (simplified) {
                opt->folded_constants++;
            }
        }
        
        instr = next;
    }
    
    free_constant_table(table);
}

// 死代码消除
void dead_code_elimination(Optimizer *opt) {
    IRInstruction *instr = opt->ir_gen->instructions;
    
    while (instr) {
        IRInstruction *next = instr->next;
        
        if (is_dead_instruction(instr, opt->ir_gen)) {
            remove_instruction(opt->ir_gen, instr);
            opt->eliminated_instructions++;
        }
        
        instr = next;
    }
}

// 复制传播
void copy_propagation(Optimizer *opt) {
    IRInstruction *instr = opt->ir_gen->instructions;
    
    while (instr) {
        // 查找形如 t1 = t2 的赋值
        if (instr->opcode == IR_ASSIGN && 
            instr->result->type == OPERAND_TEMP &&
            instr->operand1->type == OPERAND_TEMP) {
            
            int target_temp = instr->result->temp_id;
            int source_temp = instr->operand1->temp_id;
            
            // 在后续指令中替换对target_temp的使用
            IRInstruction *current = instr->next;
            bool can_eliminate = true;
            
            while (current && can_eliminate) {
                // 检查source_temp是否被重新定义
                if (current->result && current->result->type == OPERAND_TEMP &&
                    current->result->temp_id == source_temp) {
                    can_eliminate = false;
                    break;
                }
                
                // 替换对target_temp的使用
                if (current->operand1 && current->operand1->type == OPERAND_TEMP &&
                    current->operand1->temp_id == target_temp) {
                    current->operand1->temp_id = source_temp;
                    opt->propagated_constants++;
                }
                
                if (current->operand2 && current->operand2->type == OPERAND_TEMP &&
                    current->operand2->temp_id == target_temp) {
                    current->operand2->temp_id = source_temp;
                    opt->propagated_constants++;
                }
                
                current = current->next;
            }
        }
        
        instr = instr->next;
    }
}

// 公共子表达式消除 (简化版本)
void common_subexpression_elimination(Optimizer *opt) {
    // 这里实现一个简化的版本，只处理相邻的相同表达式
    IRInstruction *instr = opt->ir_gen->instructions;
    
    while (instr && instr->next) {
        IRInstruction *next = instr->next;
        
        // 检查两个连续的二元运算指令是否相同
        if (instr->opcode == IR_BINOP && next->opcode == IR_BINOP &&
            instr->binop == next->binop &&
            operands_equal(instr->operand1, next->operand1) &&
            operands_equal(instr->operand2, next->operand2)) {
            
            // 将第二个指令替换为赋值
            next->opcode = IR_ASSIGN;
            free_operand(next->operand1);
            free_operand(next->operand2);
            next->operand1 = copy_operand(instr->result);
            next->operand2 = NULL;
            
            opt->eliminated_instructions++;
        }
        
        instr = instr->next;
    }
}

// 常量表操作函数
ConstantTable* init_constant_table() {
    ConstantTable *table = (ConstantTable*)malloc(sizeof(ConstantTable));
    table->entries = NULL;
    return table;
}

void free_constant_table(ConstantTable *table) {
    ConstantEntry *entry = table->entries;
    while (entry) {
        ConstantEntry *next = entry->next;
        if (entry->var_name) free(entry->var_name);
        free(entry);
        entry = next;
    }
    free(table);
}

void add_constant(ConstantTable *table, int temp_id, ConstantValue value) {
    ConstantEntry *entry = (ConstantEntry*)malloc(sizeof(ConstantEntry));
    entry->temp_id = temp_id;
    entry->var_name = NULL;
    entry->constant = value;
    entry->next = table->entries;
    table->entries = entry;
}

void add_var_constant(ConstantTable *table, const char *var_name, ConstantValue value) {
    ConstantEntry *entry = (ConstantEntry*)malloc(sizeof(ConstantEntry));
    entry->temp_id = -1;
    entry->var_name = strdup(var_name);
    entry->constant = value;
    entry->next = table->entries;
    table->entries = entry;
}

ConstantValue* lookup_temp_constant(ConstantTable *table, int temp_id) {
    ConstantEntry *entry = table->entries;
    while (entry) {
        if (entry->temp_id == temp_id && !entry->var_name) {
            return &entry->constant;
        }
        entry = entry->next;
    }
    return NULL;
}

ConstantValue* lookup_var_constant(ConstantTable *table, const char *var_name) {
    ConstantEntry *entry = table->entries;
    while (entry) {
        if (entry->var_name && strcmp(entry->var_name, var_name) == 0) {
            return &entry->constant;
        }
        entry = entry->next;
    }
    return NULL;
}

// 常量值操作
ConstantValue create_int_constant(int value) {
    ConstantValue constant;
    constant.is_constant = true;
    constant.type = TYPE_INT;
    constant.value.int_val = value;
    return constant;
}

ConstantValue create_float_constant(float value) {
    ConstantValue constant;
    constant.is_constant = true;
    constant.type = TYPE_FLOAT;
    constant.value.float_val = value;
    return constant;
}

ConstantValue create_unknown_constant() {
    ConstantValue constant;
    constant.is_constant = false;
    constant.type = TYPE_UNKNOWN;
    return constant;
}

bool is_constant_operand(Operand *operand, ConstantTable *table) {
    if (operand->type == OPERAND_CONST) {
        return true;
    }
    if (operand->type == OPERAND_TEMP) {
        ConstantValue *value = lookup_temp_constant(table, operand->temp_id);
        return value && value->is_constant;
    }
    if (operand->type == OPERAND_VAR) {
        ConstantValue *value = lookup_var_constant(table, operand->var_name);
        return value && value->is_constant;
    }
    return false;
}

ConstantValue get_operand_constant(Operand *operand, ConstantTable *table) {
    if (operand->type == OPERAND_CONST) {
        if (operand->data_type == TYPE_INT) {
            return create_int_constant(operand->const_val.int_val);
        } else {
            return create_float_constant(operand->const_val.float_val);
        }
    }
    if (operand->type == OPERAND_TEMP) {
        ConstantValue *value = lookup_temp_constant(table, operand->temp_id);
        return value ? *value : create_unknown_constant();
    }
    if (operand->type == OPERAND_VAR) {
        ConstantValue *value = lookup_var_constant(table, operand->var_name);
        return value ? *value : create_unknown_constant();
    }
    return create_unknown_constant();
}

// 常量运算
ConstantValue evaluate_binop(BinOpType op, ConstantValue left, ConstantValue right) {
    if (!left.is_constant || !right.is_constant) {
        return create_unknown_constant();
    }
    
    // 类型提升
    if (left.type == TYPE_FLOAT || right.type == TYPE_FLOAT) {
        float left_val = (left.type == TYPE_FLOAT) ? left.value.float_val : (float)left.value.int_val;
        float right_val = (right.type == TYPE_FLOAT) ? right.value.float_val : (float)right.value.int_val;
        
        switch (op) {
            case OP_ADD: return create_float_constant(left_val + right_val);
            case OP_SUB: return create_float_constant(left_val - right_val);
            case OP_MUL: return create_float_constant(left_val * right_val);
            case OP_DIV: 
                if (right_val != 0.0f) {
                    return create_float_constant(left_val / right_val);
                }
                break;
            case OP_EQ: return create_int_constant(left_val == right_val ? 1 : 0);
            case OP_NE: return create_int_constant(left_val != right_val ? 1 : 0);
            case OP_LT: return create_int_constant(left_val < right_val ? 1 : 0);
            case OP_GT: return create_int_constant(left_val > right_val ? 1 : 0);
            case OP_LE: return create_int_constant(left_val <= right_val ? 1 : 0);
            case OP_GE: return create_int_constant(left_val >= right_val ? 1 : 0);
        }
    } else {
        int left_val = left.value.int_val;
        int right_val = right.value.int_val;
        
        switch (op) {
            case OP_ADD: return create_int_constant(left_val + right_val);
            case OP_SUB: return create_int_constant(left_val - right_val);
            case OP_MUL: return create_int_constant(left_val * right_val);
            case OP_DIV:
                if (right_val != 0) {
                    return create_int_constant(left_val / right_val);
                }
                break;
            case OP_EQ: return create_int_constant(left_val == right_val ? 1 : 0);
            case OP_NE: return create_int_constant(left_val != right_val ? 1 : 0);
            case OP_LT: return create_int_constant(left_val < right_val ? 1 : 0);
            case OP_GT: return create_int_constant(left_val > right_val ? 1 : 0);
            case OP_LE: return create_int_constant(left_val <= right_val ? 1 : 0);
            case OP_GE: return create_int_constant(left_val >= right_val ? 1 : 0);
        }
    }
    
    return create_unknown_constant();
}

bool can_evaluate_binop(BinOpType op, ConstantValue left, ConstantValue right) {
    if (!left.is_constant || !right.is_constant) {
        return false;
    }
    
    // 检查除零
    if (op == OP_DIV) {
        if (right.type == TYPE_INT && right.value.int_val == 0) {
            return false;
        }
        if (right.type == TYPE_FLOAT && right.value.float_val == 0.0f) {
            return false;
        }
    }
    
    return true;
}

// 辅助函数
bool is_zero_const_value(ConstantValue value) {
    if (!value.is_constant) return false;
    if (value.type == TYPE_INT) return value.value.int_val == 0;
    if (value.type == TYPE_FLOAT) return value.value.float_val == 0.0f;
    return false;
}

bool is_one_const_value(ConstantValue value) {
    if (!value.is_constant) return false;
    if (value.type == TYPE_INT) return value.value.int_val == 1;
    if (value.type == TYPE_FLOAT) return value.value.float_val == 1.0f;
    return false;
}

bool is_dead_instruction(IRInstruction *instr, IRGenerator *gen) {
    // 简化的死代码检测：检查结果是否被使用
    if (!instr->result || has_side_effects(instr)) {
        return false;
    }
    
    if (instr->result->type == OPERAND_TEMP) {
        return !is_temp_used(instr->result->temp_id, instr->next);
    }
    
    return false;
}

bool has_side_effects(IRInstruction *instr) {
    switch (instr->opcode) {
        case IR_STORE:
        case IR_CALL:
        case IR_RETURN:
        case IR_GOTO:
        case IR_IF_GOTO:
        case IR_IF_FALSE_GOTO:
        case IR_LABEL:
        case IR_FUNC_BEGIN:
        case IR_FUNC_END:
            return true;
        default:
            return false;
    }
}

bool is_temp_used(int temp_id, IRInstruction *start_instr) {
    IRInstruction *instr = start_instr;
    
    while (instr) {
        if ((instr->operand1 && instr->operand1->type == OPERAND_TEMP && 
             instr->operand1->temp_id == temp_id) ||
            (instr->operand2 && instr->operand2->type == OPERAND_TEMP && 
             instr->operand2->temp_id == temp_id)) {
            return true;
        }
        
        // 如果临时变量被重新定义，则停止搜索
        if (instr->result && instr->result->type == OPERAND_TEMP && 
            instr->result->temp_id == temp_id) {
            return false;
        }
        
        instr = instr->next;
    }
    
    return false;
}

bool operands_equal(Operand *op1, Operand *op2) {
    if (!op1 || !op2 || op1->type != op2->type) {
        return false;
    }
    
    switch (op1->type) {
        case OPERAND_TEMP:
            return op1->temp_id == op2->temp_id;
        case OPERAND_VAR:
            return strcmp(op1->var_name, op2->var_name) == 0;
        case OPERAND_CONST:
            if (op1->data_type != op2->data_type) return false;
            if (op1->data_type == TYPE_INT) {
                return op1->const_val.int_val == op2->const_val.int_val;
            } else {
                return op1->const_val.float_val == op2->const_val.float_val;
            }
        default:
            return false;
    }
}

Operand* copy_operand(Operand *operand) {
    if (!operand) return NULL;
    
    switch (operand->type) {
        case OPERAND_TEMP:
            return create_temp_operand(operand->temp_id, operand->data_type);
        case OPERAND_VAR:
            return create_var_operand(operand->var_name, operand->data_type);
        case OPERAND_CONST:
            if (operand->data_type == TYPE_INT) {
                return create_int_const_operand(operand->const_val.int_val);
            } else {
                return create_float_const_operand(operand->const_val.float_val);
            }
        case OPERAND_LABEL:
            return create_label_operand(operand->label_name);
        case OPERAND_FUNC:
            return create_func_operand(operand->func_name);
        default:
            return NULL;
    }
}

void remove_temp_constant(ConstantTable *table, int temp_id) {
    ConstantEntry **entry = &table->entries;
    while (*entry) {
        if ((*entry)->temp_id == temp_id && !(*entry)->var_name) {
            ConstantEntry *to_remove = *entry;
            *entry = (*entry)->next;
            free(to_remove);
            return;
        }
        entry = &(*entry)->next;
    }
}

void remove_var_constant(ConstantTable *table, const char *var_name) {
    ConstantEntry **entry = &table->entries;
    while (*entry) {
        if ((*entry)->var_name && strcmp((*entry)->var_name, var_name) == 0) {
            ConstantEntry *to_remove = *entry;
            *entry = (*entry)->next;
            free(to_remove->var_name);
            free(to_remove);
            return;
        }
        entry = &(*entry)->next;
    }
}

void remove_instruction(IRGenerator *gen, IRInstruction *instr) {
    // 这里需要实现从链表中移除指令的逻辑
    // 简化实现，实际应该更完善
    if (gen->instructions == instr) {
        gen->instructions = instr->next;
    } else {
        IRInstruction *prev = gen->instructions;
        while (prev && prev->next != instr) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = instr->next;
        }
    }
    
    if (gen->last_instr == instr) {
        gen->last_instr = gen->instructions;
        while (gen->last_instr && gen->last_instr->next) {
            gen->last_instr = gen->last_instr->next;
        }
    }
    
    // 释放指令
    if (instr->result) free_operand(instr->result);
    if (instr->operand1) free_operand(instr->operand1);
    if (instr->operand2) free_operand(instr->operand2);
    free(instr);
}

void print_optimization_stats(Optimizer *opt) {
    printf("Optimization Statistics:\n");
    printf("  Eliminated instructions: %d\n", opt->eliminated_instructions);
    printf("  Folded constants: %d\n", opt->folded_constants);
    printf("  Propagated constants: %d\n", opt->propagated_constants);
    printf("=========================\n");
}
