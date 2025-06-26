#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 初始化符号表
SymbolTable* init_symbol_table() {
    SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (table) {
        table->head = NULL;
        table->current_scope = 0;  // 全局作用域从0开始
    }
    return table;
}

// 释放符号表
void free_symbol_table(SymbolTable *table) {
    if (!table) return;

    SymbolEntry *current = table->head;
    while (current) {
        SymbolEntry *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    free(table);
}

// 进入新作用域
void enter_scope(SymbolTable *table) {
    if (table) {
        table->current_scope++;
        printf("进入作用域: %d\n", table->current_scope);
    }
}

// 离开当前作用域
void leave_scope(SymbolTable *table) {
    if (!table || table->current_scope <= 0) return;
    
    // 删除当前作用域的所有符号
    SymbolEntry *current = table->head;
    SymbolEntry *prev = NULL;
    
    while (current) {
        if (current->scope_level == table->current_scope) {
            // 删除此符号
            SymbolEntry *to_delete = current;
            
            if (prev) {
                prev->next = current->next;
                current = current->next;
            } else {
                table->head = current->next;
                current = table->head;
            }
            
            printf("删除符号: %s (作用域: %d)\n", to_delete->name, to_delete->scope_level);
            free(to_delete->name);
            free(to_delete);
        } else {
            prev = current;
            current = current->next;
        }
    }
    
    table->current_scope--;
    printf("离开作用域，当前作用域: %d\n", table->current_scope);
}

// 添加符号到符号表
bool add_symbol(SymbolTable *table, const char *name, SymbolKind kind, DataType type) {
    if (!table || !name) return false;
    
    // 检查当前作用域中是否已存在同名符号
    if (lookup_symbol_current_scope(table, name)) {
        printf("错误: 符号 '%s' 在当前作用域中重复定义\n", name);
        return false;
    }
    
    // 创建新符号表项
    SymbolEntry *entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (!entry) return false;
    
    entry->name = _strdup(name);
    entry->kind = kind;
    entry->type = type;
    entry->scope_level = table->current_scope;
    
    // 插入到符号表头部
    entry->next = table->head;
    table->head = entry;
    
    printf("添加符号: %s, 类型: %s, 作用域: %d\n", 
           name, data_type_to_str(type), table->current_scope);
    
    return true;
}

// 在符号表中查找符号（考虑作用域）
SymbolEntry* lookup_symbol(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;
    
    SymbolEntry *current = table->head;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current; // 找到符号
        }
        current = current->next;
    }
    
    return NULL; // 未找到
}

// 仅在当前作用域中查找符号
SymbolEntry* lookup_symbol_current_scope(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;
    
    SymbolEntry *current = table->head;
    
    while (current) {
        if (current->scope_level == table->current_scope && 
            strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// 打印符号表内容
void print_symbol_table(SymbolTable *table) {
    if (!table) return;
    
    printf("\n===== 符号表 =====\n");
    printf("%-15s %-10s %-10s %s\n", "名称", "种类", "类型", "作用域");
    printf("--------------------------------------------\n");
    
    SymbolEntry *current = table->head;
    
    while (current) {
        printf("%-15s %-10s %-10s %d\n", 
               current->name,
               current->kind == SYM_VARIABLE ? "变量" : "函数",
               data_type_to_str(current->type),
               current->scope_level);
        
        current = current->next;
    }
    
    printf("==================\n\n");
}

// 获取类型转换后的结果类型
DataType get_result_type(DataType left_type, DataType right_type) {
    // 如果有任何一个类型未知，结果未知
    if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
        return TYPE_UNKNOWN;
    }
    
    // 如果有任何一个是float，结果是float（隐式转换）
    if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }
    
    // 否则都是int，结果是int
    return TYPE_INT;
}

// 数据类型转字符串
const char* data_type_to_str(DataType type) {
    switch (type) {
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "float";
        case TYPE_UNKNOWN:
        default:          return "unknown";
    }
}
