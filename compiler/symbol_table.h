#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// 符号类型
typedef enum {
    SYM_VARIABLE,  // 变量
    SYM_FUNCTION   // 函数
} SymbolKind;

// 数据类型
typedef enum {
    TYPE_UNKNOWN,  // 未知类型（错误情况）
    TYPE_INT,      // 整型
    TYPE_FLOAT     // 浮点型
} DataType;

// 符号表项
typedef struct SymbolEntry {
    char *name;             // 符号名
    SymbolKind kind;        // 符号种类（变量或函数）
    DataType type;          // 数据类型
    int scope_level;        // 作用域级别
    struct SymbolEntry *next;  // 链表下一项
} SymbolEntry;

// 符号表
typedef struct {
    SymbolEntry *head;      // 符号表头指针
    int current_scope;      // 当前作用域级别
} SymbolTable;

// 符号表接口函数
SymbolTable* init_symbol_table();
void free_symbol_table(SymbolTable *table);
void enter_scope(SymbolTable *table);
void leave_scope(SymbolTable *table);
bool add_symbol(SymbolTable *table, const char *name, SymbolKind kind, DataType type);
SymbolEntry* lookup_symbol(SymbolTable *table, const char *name);
SymbolEntry* lookup_symbol_current_scope(SymbolTable *table, const char *name);
void print_symbol_table(SymbolTable *table);

// 类型检查辅助函数
DataType get_result_type(DataType left_type, DataType right_type);
const char* data_type_to_str(DataType type);

#endif
