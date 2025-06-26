#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ��ʼ�����ű�
SymbolTable* init_symbol_table() {
    SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (table) {
        table->head = NULL;
        table->current_scope = 0;  // ȫ���������0��ʼ
    }
    return table;
}

// �ͷŷ��ű�
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

// ������������
void enter_scope(SymbolTable *table) {
    if (table) {
        table->current_scope++;
        printf("����������: %d\n", table->current_scope);
    }
}

// �뿪��ǰ������
void leave_scope(SymbolTable *table) {
    if (!table || table->current_scope <= 0) return;
    
    // ɾ����ǰ����������з���
    SymbolEntry *current = table->head;
    SymbolEntry *prev = NULL;
    
    while (current) {
        if (current->scope_level == table->current_scope) {
            // ɾ���˷���
            SymbolEntry *to_delete = current;
            
            if (prev) {
                prev->next = current->next;
                current = current->next;
            } else {
                table->head = current->next;
                current = table->head;
            }
            
            printf("ɾ������: %s (������: %d)\n", to_delete->name, to_delete->scope_level);
            free(to_delete->name);
            free(to_delete);
        } else {
            prev = current;
            current = current->next;
        }
    }
    
    table->current_scope--;
    printf("�뿪�����򣬵�ǰ������: %d\n", table->current_scope);
}

// ��ӷ��ŵ����ű�
bool add_symbol(SymbolTable *table, const char *name, SymbolKind kind, DataType type) {
    if (!table || !name) return false;
    
    // ��鵱ǰ���������Ƿ��Ѵ���ͬ������
    if (lookup_symbol_current_scope(table, name)) {
        printf("����: ���� '%s' �ڵ�ǰ���������ظ�����\n", name);
        return false;
    }
    
    // �����·��ű���
    SymbolEntry *entry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (!entry) return false;
    
    entry->name = _strdup(name);
    entry->kind = kind;
    entry->type = type;
    entry->scope_level = table->current_scope;
    
    // ���뵽���ű�ͷ��
    entry->next = table->head;
    table->head = entry;
    
    printf("��ӷ���: %s, ����: %s, ������: %d\n", 
           name, data_type_to_str(type), table->current_scope);
    
    return true;
}

// �ڷ��ű��в��ҷ��ţ�����������
SymbolEntry* lookup_symbol(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;
    
    SymbolEntry *current = table->head;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current; // �ҵ�����
        }
        current = current->next;
    }
    
    return NULL; // δ�ҵ�
}

// ���ڵ�ǰ�������в��ҷ���
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

// ��ӡ���ű�����
void print_symbol_table(SymbolTable *table) {
    if (!table) return;
    
    printf("\n===== ���ű� =====\n");
    printf("%-15s %-10s %-10s %s\n", "����", "����", "����", "������");
    printf("--------------------------------------------\n");
    
    SymbolEntry *current = table->head;
    
    while (current) {
        printf("%-15s %-10s %-10s %d\n", 
               current->name,
               current->kind == SYM_VARIABLE ? "����" : "����",
               data_type_to_str(current->type),
               current->scope_level);
        
        current = current->next;
    }
    
    printf("==================\n\n");
}

// ��ȡ����ת����Ľ������
DataType get_result_type(DataType left_type, DataType right_type) {
    // ������κ�һ������δ֪�����δ֪
    if (left_type == TYPE_UNKNOWN || right_type == TYPE_UNKNOWN) {
        return TYPE_UNKNOWN;
    }
    
    // ������κ�һ����float�������float����ʽת����
    if (left_type == TYPE_FLOAT || right_type == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }
    
    // ������int�������int
    return TYPE_INT;
}

// ��������ת�ַ���
const char* data_type_to_str(DataType type) {
    switch (type) {
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "float";
        case TYPE_UNKNOWN:
        default:          return "unknown";
    }
}
