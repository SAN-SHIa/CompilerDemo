#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include "ir.h"

// 优化类型
typedef enum {
    OPT_CONSTANT_FOLDING,      // 常量折叠
    OPT_CONSTANT_PROPAGATION,  // 常量传播
    OPT_DEAD_CODE_ELIMINATION, // 死代码消除
    OPT_ALGEBRAIC_SIMPLIFICATION, // 代数简化
    OPT_COPY_PROPAGATION,      // 复制传播
    OPT_COMMON_SUBEXPRESSION   // 公共子表达式消除
} OptimizationType;

// 优化器上下文
typedef struct {
    IRGenerator *ir_gen;
    int optimization_level;    // 优化级别 (0-3)
    bool optimizations_enabled[6]; // 各种优化是否启用
    int eliminated_instructions;   // 消除的指令数
    int folded_constants;         // 折叠的常量数
    int propagated_constants;     // 传播的常量数
} Optimizer;

// 常量值结构
typedef struct {
    bool is_constant;
    DataType type;
    union {
        int int_val;
        float float_val;
    } value;
} ConstantValue;

// 常量表项
typedef struct ConstantEntry {
    int temp_id;
    char *var_name;
    ConstantValue constant;
    struct ConstantEntry *next;
} ConstantEntry;

// 常量表
typedef struct {
    ConstantEntry *entries;
} ConstantTable;

// 优化器初始化和清理
Optimizer* init_optimizer(IRGenerator *ir_gen, int optimization_level);
void free_optimizer(Optimizer *opt);

// 主优化函数
void optimize_ir(Optimizer *opt);

// 各种优化算法
void constant_folding(Optimizer *opt);
void constant_propagation(Optimizer *opt);
void dead_code_elimination(Optimizer *opt);
void algebraic_simplification(Optimizer *opt);
void copy_propagation(Optimizer *opt);
void common_subexpression_elimination(Optimizer *opt);

// 常量表管理
ConstantTable* init_constant_table();
void free_constant_table(ConstantTable *table);
void add_constant(ConstantTable *table, int temp_id, ConstantValue value);
void add_var_constant(ConstantTable *table, const char *var_name, ConstantValue value);
ConstantValue* lookup_temp_constant(ConstantTable *table, int temp_id);
ConstantValue* lookup_var_constant(ConstantTable *table, const char *var_name);
void remove_temp_constant(ConstantTable *table, int temp_id);
void remove_var_constant(ConstantTable *table, const char *var_name);

// 常量值操作
ConstantValue create_int_constant(int value);
ConstantValue create_float_constant(float value);
ConstantValue create_unknown_constant();
bool is_constant_operand(Operand *operand, ConstantTable *table);
ConstantValue get_operand_constant(Operand *operand, ConstantTable *table);

// 常量运算
ConstantValue evaluate_binop(BinOpType op, ConstantValue left, ConstantValue right);
bool can_evaluate_binop(BinOpType op, ConstantValue left, ConstantValue right);

// 代数简化检查
bool is_zero_const_value(ConstantValue value);
bool is_one_const_value(ConstantValue value);
bool is_identity_operation(BinOpType op, ConstantValue operand);
bool is_absorbing_operation(BinOpType op, ConstantValue operand);

// 指令分析
bool is_dead_instruction(IRInstruction *instr, IRGenerator *gen);
bool has_side_effects(IRInstruction *instr);
bool is_temp_used(int temp_id, IRInstruction *start_instr);
bool is_var_used(const char *var_name, IRInstruction *start_instr);

// 指令操作
void remove_instruction(IRGenerator *gen, IRInstruction *instr);
IRInstruction* get_previous_instruction(IRGenerator *gen, IRInstruction *instr);
void replace_operand_in_instruction(IRInstruction *instr, Operand *old_operand, Operand *new_operand);

// 优化统计和报告
void print_optimization_stats(Optimizer *opt);
void set_optimization_level(Optimizer *opt, int level);
void enable_optimization(Optimizer *opt, OptimizationType type);
void disable_optimization(Optimizer *opt, OptimizationType type);

// 辅助函数
bool operands_equal(Operand *op1, Operand *op2);
Operand* copy_operand(Operand *operand);
bool is_comparison_op(BinOpType op);
bool is_commutative_op(BinOpType op);

#endif
