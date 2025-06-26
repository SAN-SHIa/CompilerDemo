#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// ��������
typedef enum {
    SYM_VARIABLE,  // ����
    SYM_FUNCTION   // ����
} SymbolKind;

// ��������
typedef enum {
    TYPE_UNKNOWN,  // δ֪���ͣ����������
    TYPE_INT,      // ����
    TYPE_FLOAT     // ������
} DataType;

// ���ű���
typedef struct SymbolEntry {
    char *name;             // ������
    SymbolKind kind;        // �������ࣨ����������
    DataType type;          // ��������
    int scope_level;        // �����򼶱�
    struct SymbolEntry *next;  // ������һ��
} SymbolEntry;

// ���ű�
typedef struct {
    SymbolEntry *head;      // ���ű�ͷָ��
    int current_scope;      // ��ǰ�����򼶱�
} SymbolTable;

// ���ű�ӿں���
SymbolTable* init_symbol_table();
void free_symbol_table(SymbolTable *table);
void enter_scope(SymbolTable *table);
void leave_scope(SymbolTable *table);
bool add_symbol(SymbolTable *table, const char *name, SymbolKind kind, DataType type);
SymbolEntry* lookup_symbol(SymbolTable *table, const char *name);
SymbolEntry* lookup_symbol_current_scope(SymbolTable *table, const char *name);
void print_symbol_table(SymbolTable *table);

// ���ͼ�鸨������
DataType get_result_type(DataType left_type, DataType right_type);
const char* data_type_to_str(DataType type);

#endif
