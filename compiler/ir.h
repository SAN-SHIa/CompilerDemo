#ifndef IR_H
#define IR_H

#include "ast.h"
#include "symbol_table.h"

// 变量类型映射节点
typedef struct VarTypeNode {
    char *var_name;
    DataType type;
    struct VarTypeNode *next;
} VarTypeNode;

// 中间代码指令类型
typedef enum {
    IR_ASSIGN,      // 赋值：t1 = t2
    IR_BINOP,       // 二元运算：t1 = t2 op t3
    IR_LOAD,        // 加载：t1 = var
    IR_STORE,       // 存储：var = t1
    IR_LOAD_CONST,  // 加载常量：t1 = const
    IR_LABEL,       // 标签：L1:
    IR_GOTO,        // 无条件跳转：goto L1
    IR_IF_GOTO,     // 条件跳转：if t1 goto L1
    IR_IF_FALSE_GOTO, // 条件跳转：if !t1 goto L1
    IR_PARAM,       // 参数：param t1
    IR_CALL,        // 函数调用：t1 = call func, n
    IR_RETURN,      // 返回：return t1
    IR_FUNC_BEGIN,  // 函数开始：func_begin name
    IR_FUNC_END,    // 函数结束：func_end
    IR_CONVERT      // 类型转换：t1 = (type) t2
} IROpcode;

// 操作数类型
typedef enum {
    OPERAND_TEMP,     // 临时变量
    OPERAND_VAR,      // 变量
    OPERAND_CONST,    // 常量
    OPERAND_LABEL,    // 标签
    OPERAND_FUNC      // 函数名
} OperandType;

// 操作数
typedef struct {
    OperandType type;
    DataType data_type;  // 数据类型
    union {
        int temp_id;      // 临时变量ID
        char *var_name;   // 变量名
        struct {
            union {
                int int_val;
                float float_val;
            };
        } const_val;      // 常量值
        char *label_name; // 标签名
        char *func_name;  // 函数名
    };
} Operand;

// 中间代码指令
typedef struct IRInstruction {
    IROpcode opcode;
    Operand *result;    // 结果操作数
    Operand *operand1;  // 第一个操作数
    Operand *operand2;  // 第二个操作数
    BinOpType binop;    // 二元运算符（用于IR_BINOP）
    struct IRInstruction *next;
} IRInstruction;

// 中间代码生成器上下文
typedef struct {
    IRInstruction *instructions;  // 指令链表头
    IRInstruction *last_instr;    // 指令链表尾
    int temp_counter;             // 临时变量计数器
    int label_counter;            // 标签计数器
    SymbolTable *symbol_table;    // 符号表
    VarTypeNode *var_type_table;  // 变量类型映射表
} IRGenerator;

// 函数声明
IRGenerator* init_ir_generator(SymbolTable *symbol_table);
void free_ir_generator(IRGenerator *gen);

// 变量类型管理函数
void add_var_type(IRGenerator *gen, const char *var_name, DataType type);
DataType get_var_type(IRGenerator *gen, const char *var_name);

// 操作数创建函数
Operand* create_temp_operand(int temp_id, DataType type);
Operand* create_var_operand(const char *var_name, DataType type);
Operand* create_int_const_operand(int value);
Operand* create_float_const_operand(float value);
Operand* create_label_operand(const char *label_name);
Operand* create_func_operand(const char *func_name);
void free_operand(Operand *operand);

// 指令创建函数
IRInstruction* create_ir_instruction(IROpcode opcode);
void append_instruction(IRGenerator *gen, IRInstruction *instr);

// 中间代码生成主函数
void generate_ir(ASTNode *node, IRGenerator *gen);
Operand* generate_expr_ir(ASTNode *node, IRGenerator *gen);
void generate_stmt_ir(ASTNode *node, IRGenerator *gen);
void generate_call_ir(ASTNode *node, IRGenerator *gen);  // 函数调用代码生成
Operand* generate_call_expr_ir(ASTNode *node, IRGenerator *gen);  // 函数调用表达式代码生成

// 辅助函数
int get_next_temp(IRGenerator *gen);
char* get_next_label(IRGenerator *gen);
DataType get_expr_type(ASTNode *node, SymbolTable *symbol_table);

// 打印函数
void print_ir(IRGenerator *gen);
void print_operand(Operand *operand);
void print_instruction(IRInstruction *instr);

// 类型转换相关
bool need_type_conversion(DataType from, DataType to);
Operand* generate_type_conversion(IRGenerator *gen, Operand *operand, DataType target_type);

#endif
