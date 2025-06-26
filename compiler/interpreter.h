#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ir.h"
#include <stdbool.h>

// 变量值类型
typedef enum {
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING
} ValueType;

// 运行时值
typedef struct {
    ValueType type;
    union {
        int int_val;
        float float_val;
        char *str_val;
    } data;
} RuntimeValue;

// 变量表项
typedef struct VarEntry {
    char *name;
    RuntimeValue value;
    struct VarEntry *next;
} VarEntry;

// 解释器上下文
typedef struct {
    VarEntry *variables;     // 变量表
    int pc;                  // 程序计数器
    bool running;            // 是否继续执行
    RuntimeValue return_val; // 返回值
} Interpreter;

// 函数声明
Interpreter* init_interpreter(void);
void free_interpreter(Interpreter *interp);
void execute_ir(Interpreter *interp, IRGenerator *ir_gen);
void set_variable(Interpreter *interp, const char *name, RuntimeValue value);
RuntimeValue get_variable(Interpreter *interp, const char *name);
void print_runtime_value(RuntimeValue value);

// 运行时值创建辅助函数
RuntimeValue create_int_value(int value);
RuntimeValue create_float_value(float value);
RuntimeValue create_string_value(const char *str);

// 操作数执行函数
RuntimeValue execute_operand(Interpreter *interp, Operand *operand);
RuntimeValue execute_binop(RuntimeValue left, RuntimeValue right, BinOpType op);

#endif
