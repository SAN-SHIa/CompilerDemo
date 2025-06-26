#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stddef.h>
#include "ir.h"
#include "optimize.h"

// 目标架构类型
typedef enum {
    TARGET_X86_64,    // x86-64汇编
    TARGET_X86_32,    // x86-32汇编
    TARGET_ARM64,     // ARM64汇编
    TARGET_MIPS,      // MIPS汇编
    TARGET_C_CODE,    // 生成C代码
    TARGET_PSEUDO     // 伪汇编（教学用）
} TargetArch;

// 寄存器类型
typedef enum {
    REG_EAX, REG_EBX, REG_ECX, REG_EDX,     // x86-32通用寄存器
    REG_RAX, REG_RBX, REG_RCX, REG_RDX,     // x86-64通用寄存器
    REG_RSI, REG_RDI, REG_R8, REG_R9,       // x86-64额外寄存器
    REG_ESP, REG_EBP, REG_RSP, REG_RBP,     // 栈指针和基址指针
    REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, // SSE浮点寄存器
    REG_NONE                                  // 无寄存器
} RegisterType;

// 寄存器描述
typedef struct {
    RegisterType type;
    char *name;
    bool is_available;
    int temp_id;        // 当前存储的临时变量ID (-1表示空闲)
    DataType data_type; // 寄存器中数据类型
} Register;

// 内存位置
typedef struct {
    enum {
        MEM_STACK,      // 栈上位置
        MEM_GLOBAL,     // 全局变量
        MEM_REGISTER    // 寄存器
    } type;
    
    union {
        struct {
            int offset;     // 相对于基址指针的偏移
        } stack;
        
        struct {
            char *label;    // 全局标签
        } global;
        
        struct {
            RegisterType reg;
        } reg;
    };
} MemoryLocation;

// 变量位置映射
typedef struct VarLocation {
    char *var_name;
    int temp_id;
    MemoryLocation location;
    struct VarLocation *next;
} VarLocation;

// 代码生成器上下文
typedef struct {
    TargetArch target_arch;         // 目标架构
    FILE *output_file;              // 输出文件
    Register *registers;            // 寄存器数组
    int register_count;             // 寄存器数量
    VarLocation *var_locations;     // 变量位置映射
    int stack_offset;               // 当前栈偏移
    int label_counter;              // 标签计数器
    bool optimization_enabled;      // 是否启用优化
    
    // 统计信息
    int instructions_generated;     // 生成的指令数
    int registers_used;             // 使用的寄存器数
    int stack_space_used;           // 使用的栈空间
} CodeGenerator;

// 指令模板
typedef struct {
    char *template;    // 指令模板
    char *comment;     // 注释
} InstructionTemplate;

// 函数声明

// 代码生成器初始化和清理
CodeGenerator* init_code_generator(TargetArch target_arch, const char *output_filename);
void free_code_generator(CodeGenerator *gen);

// 主代码生成函数
void generate_target_code(IRGenerator *ir_gen, CodeGenerator *code_gen);
void generate_assembly_code(IRGenerator *ir_gen, CodeGenerator *code_gen);
void generate_c_code(IRGenerator *ir_gen, CodeGenerator *code_gen);
void generate_pseudo_code(IRGenerator *ir_gen, CodeGenerator *code_gen);

// 寄存器分配
void init_registers(CodeGenerator *gen);
RegisterType allocate_register(CodeGenerator *gen, DataType data_type);
void free_register(CodeGenerator *gen, RegisterType reg);
RegisterType get_temp_register(CodeGenerator *gen, int temp_id);
void assign_temp_to_register(CodeGenerator *gen, int temp_id, RegisterType reg, DataType data_type);
bool is_register_free(CodeGenerator *gen, RegisterType reg);
void spill_register(CodeGenerator *gen, RegisterType reg);

// 变量位置管理
MemoryLocation get_var_location(CodeGenerator *gen, const char *var_name);
MemoryLocation get_temp_location(CodeGenerator *gen, int temp_id);
void set_var_location(CodeGenerator *gen, const char *var_name, MemoryLocation location);
void set_temp_location(CodeGenerator *gen, int temp_id, MemoryLocation location);
int allocate_stack_space(CodeGenerator *gen, DataType data_type);

// 指令生成
void emit_instruction(CodeGenerator *gen, const char *format, ...);
void emit_comment(CodeGenerator *gen, const char *comment);
void emit_label(CodeGenerator *gen, const char *label);
void emit_function_prologue(CodeGenerator *gen, const char *func_name);
void emit_function_epilogue(CodeGenerator *gen);

// 操作数处理
void generate_operand_code(CodeGenerator *gen, Operand *operand, char *buffer, size_t buffer_size);
RegisterType load_operand_to_register(CodeGenerator *gen, Operand *operand);
void store_register_to_operand(CodeGenerator *gen, RegisterType reg, Operand *operand);

// 不同架构的代码生成
void generate_x86_64_instruction(CodeGenerator *gen, IRInstruction *instr);
void generate_x86_32_instruction(CodeGenerator *gen, IRInstruction *instr);
void generate_pseudo_instruction(CodeGenerator *gen, IRInstruction *instr);

// 数据类型处理
const char* get_type_suffix(DataType type, TargetArch arch);
const char* get_register_name(RegisterType reg, DataType type, TargetArch arch);
int get_type_size(DataType type);

// 优化相关
void peephole_optimization(CodeGenerator *gen);
void register_allocation_optimization(CodeGenerator *gen);

// 辅助函数
const char* get_binop_instruction(BinOpType op, TargetArch arch, DataType type);
const char* get_condition_code(BinOpType op, TargetArch arch);
bool needs_float_register(DataType type);
bool is_commutative_operation(BinOpType op);

// 调试和统计
void print_register_allocation(CodeGenerator *gen);
void print_codegen_stats(CodeGenerator *gen);
void print_var_locations(CodeGenerator *gen);

// 文件头和尾
void emit_file_header(CodeGenerator *gen);
void emit_file_footer(CodeGenerator *gen);
void emit_data_section(CodeGenerator *gen);
void emit_text_section(CodeGenerator *gen);

#endif
