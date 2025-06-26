#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LR(1)编译原理分析器 - 文法定义
"""

# 预定义的文法选项
GRAMMARS = {
    "C语言1: 简单赋值语句 - S→id=E, E→E+T|T, T→T*F|F, F→(E)|id|num": {
        "number": 7,
        "productions": ["S → id=E;", "E → E+T", "E → T", "T → T*F", "T → F", "F → (E)", "F → id", "F → num"],
        "example_input": "result = value + factor * 5;"
    },
    "C语言2: if语句 - S→if(E)S|id=E, E→E==T|T, T→id|num": {
        "number": 8,
        "productions": ["S → if(E)S", "S → id=E;", "S → {SL}", "SL → S", "SL → S SL", "E → E==T", "E → T", "T → id", "T → num"],
        "example_input": "if (count == 10) {\n    result = success;\n}"
    },
    "C语言3: 变量声明 - S→T id, T→int|float|char": {
        "number": 9,
        "productions": ["S → T id;", "S → T id=E;", "T → int", "T → float", "T → char", "E → id", "E → num"],
        "example_input": "int counter = 0;"
    },
    "C语言4: while循环 - S→while(E)S|id=E, E→E<T|T, T→T+F|F, F→id|num": {
        "number": 10,
        "productions": ["S → while(E)S", "S → id=E;", "S → {SL}", "SL → S", "SL → S SL", "E → E<T", "E → T", "T → T+F", "T → F", "F → id", "F → num"],
        "example_input": "while (i < 10) {\n    i = i + 1;\n}"
    },
    "C语言5: 算术表达式 - E→E+T|E-T|T, T→T*F|T/F|F, F→(E)|id|num": {
        "number": 11,
        "productions": ["E → E+T", "E → E-T", "E → T", "T → T*F", "T → T/F", "T → F", "F → (E)", "F → id", "F → num"],
        "example_input": "(a + b) * c / (d - 2)"
    },

    "文法1: S→CC, C→cC|d": {
        "number": 1,
        "productions": ["S → CC", "C → cC", "C → d"],
        "example_input": "ccdccd"
    },
    "文法2: S→L=S|R, L→aLR|b, R→a": {
        "number": 2, 
        "productions": ["S → L=S", "S → R", "L → aLR", "L → b", "R → a"],
        "example_input": "aba=b=a"
    },
    "文法3: S→aLb|a, L→aR, R→LR|b": {
        "number": 3,
        "productions": ["S → aLb", "S → a", "L → aR", "R → LR", "R → b"],
        "example_input": "aaabbb"
    },
    "文法4: S→L=LR|R, L→aR|b, R→L": {
        "number": 4,
        "productions": ["S → L=LR", "S → R", "L → aR", "L → b", "R → L"],
        "example_input": "b=abab"
    },
    "文法5: S→(L)|a, L→L,S|S": {
        "number": 5,
        "productions": ["S → (L)", "S → a", "L → L,S", "L → S"],
        "example_input": "((a),a)"
    },
    "文法6: S→(S)S|ε": {
        "number": 6,
        "productions": ["S → (S)S", "S → ε"],
        "example_input": "()()"
    }
}

def get_example_input(grammar_key):
    """根据选择的文法返回示例输入"""
    if grammar_key in GRAMMARS:
        return GRAMMARS[grammar_key]["example_input"]
    return ""

def add_c_grammar_support(cpp_content, grammar_number, grammar_info):
    """添加C语言文法支持到C++代码中"""
    # 如果需要处理C++代码的函数，可以在此实现
    # 这是一个占位函数，在当前的Python实现中可能不需要
    return cpp_content
