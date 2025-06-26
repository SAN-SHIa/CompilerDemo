#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LR(1)编译原理分析器 - 词法分析模块
"""

import re

class LexicalAnalyzer:
    """C语言词法分析器"""
    
    def __init__(self):
        # 定义C语言关键字和操作符的映射
        self.token_map = {
            # 关键字
            'int': 'int', 'float': 'float', 'char': 'char', 'void': 'void',
            'if': 'if', 'else': 'else', 'while': 'while', 'for': 'for',
            'switch': 'switch', 'case': 'case', 'default': 'default',
            
            # 操作符
            '++': '++', '--': '--', 
            '==': '==', '!=': '!=', '<=': '<=', '>=': '>=',
            '&&': '&&', '||': '||',
            '->': '->', '<<': '<<', '>>': '>>',
            
            # 单字符操作符
            '+': '+', '-': '-', '*': '*', '/': '/', '%': '%',
            '=': '=', '<': '<', '>': '>', '!': '!', '&': '&', '|': '|',
            '?': '?', ':': ':', ';': ';', ',': ',', '.': '.',
            '(': '(', ')': ')', '[': '[', ']': ']', '{': '{', '}': '}'
        }
        
        # 词法分析正则表达式 - 确保关键字在标识符之前匹配
        self.patterns = [
            # 双字符操作符必须在单字符操作符之前检查
            (r'\+\+|--|==|!=|<=|>=|&&|\|\||->|<<|>>', 'OP2'),
            # 关键字必须在标识符之前检查，并且使用\b确保完整匹配词边界
            (r'\bint\b', 'int'),
            (r'\bfloat\b', 'float'),
            (r'\bchar\b', 'char'),
            (r'\bvoid\b', 'void'),
            (r'\bif\b', 'if'),
            (r'\belse\b', 'else'),
            (r'\bwhile\b', 'while'),
            (r'\bfor\b', 'for'),
            (r'\bswitch\b', 'switch'),
            (r'\bcase\b', 'case'),
            (r'\bdefault\b', 'default'),
            # 标识符在关键字之后匹配
            (r'\b[a-zA-Z_][a-zA-Z0-9_]*\b', 'ID'),
            (r'\b\d+(?:\.\d+)?\b', 'NUM'),
            (r'[+\-*/=%<>!&|?:;,.\[\](){}]', 'OP1'),
            (r'\s+', 'SPACE')
        ]
    
    def preprocess_code(self, code):
        """
        预处理C语言代码，进行词法分析并转换为适合语法分析的token序列
        """
        # 记录详细的词法分析过程
        lexical_analysis_result = []
        
        # 去除注释
        code_without_comments = re.sub(r'//.*', '', code)  # 单行注释
        code_without_comments = re.sub(r'/\*.*?\*/', '', code_without_comments, flags=re.DOTALL)  # 多行注释
        
        if code_without_comments != code:
            lexical_analysis_result.append(("注释处理", f"原始代码: {len(code)}字符 → 去除注释: {len(code_without_comments)}字符"))
        
        # 处理多行代码，将换行符替换为空格
        code_single_line = re.sub(r'\n', ' ', code_without_comments)
        # 处理多个空格
        code_normalized = re.sub(r'\s+', ' ', code_single_line).strip()
        
        if code_normalized != code_without_comments:
            lexical_analysis_result.append(("空白处理", f"规范化空白字符: {len(code_without_comments)}字符 → {len(code_normalized)}字符"))
        
        tokens = []
        detailed_tokens = []  # 带有详细信息的token列表
        pos = 0
        debug = False  # 设置为True可以启用调试输出
        
        if debug:
            print(f"开始预处理代码: '{code_normalized}'")
            
        while pos < len(code_normalized):
            matched = False
            for pattern, token_type in self.patterns:
                regex = re.compile(pattern)
                match = regex.match(code_normalized, pos)
                if match:
                    value = match.group(0)
                    if token_type != 'SPACE':  # 忽略空白字符
                        if token_type == 'ID':
                            tokens.append('id')
                            detailed_tokens.append(('id', value, pos))
                            if debug:
                                print(f"标识符: '{value}' -> 'id'")
                            lexical_analysis_result.append(("标识符", f"'{value}' → 'id'"))
                        elif token_type == 'NUM':
                            tokens.append('num')
                            detailed_tokens.append(('num', value, pos))
                            if debug:
                                print(f"数字: '{value}' -> 'num'")
                            lexical_analysis_result.append(("数字", f"'{value}' → 'num'"))
                        elif token_type in ['int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'switch', 'case', 'default']:
                            tokens.append(token_type)
                            detailed_tokens.append((token_type, value, pos))
                            if debug:
                                print(f"关键字: '{value}' -> '{token_type}'")
                            lexical_analysis_result.append(("关键字", f"'{value}' → '{token_type}'"))
                        else:  # OP1 或 OP2
                            tokens.append(value)
                            detailed_tokens.append((value, value, pos))
                            if debug:
                                print(f"操作符: '{value}' -> '{value}'")
                            lexical_analysis_result.append(("操作符", f"'{value}'"))
                    else:
                        lexical_analysis_result.append(("忽略", f"空白字符: '{value}'"))
                        if debug:
                            print(f"忽略空白: '{value}'")
                    pos = match.end()
                    matched = True
                    break
            
            if not matched:
                if debug:
                    print(f"跳过无法识别的字符: '{code_normalized[pos]}'")
                lexical_analysis_result.append(("错误", f"无法识别的字符: '{code_normalized[pos]}'"))
                pos += 1  # 跳过无法识别的字符
        
        if debug:
            print(f"预处理后的词法单元列表: {tokens}")
        
        # 返回token列表和详细的词法分析结果    
        return tokens, detailed_tokens, lexical_analysis_result
    
    def generate_markdown(self, code, detailed_tokens, lexical_analysis_result):
        """生成词法分析过程的Markdown格式输出"""
        # 添加原始代码
        md_lines = [
            "# C语言词法分析报告",
            "",
            "## 原始代码",
            "```c",
            code,
            "```",
            "",
            "## 词法分析过程",
            ""
        ]
        
        # 添加详细的词法分析过程
        for step_type, description in lexical_analysis_result:
            md_lines.append(f"- **{step_type}:** {description}")
        
        # 详细的词法单元表
        md_lines.extend([
            "",
            "## 词法单元详细表",
            "",
            "| 序号 | 类型 | 值 | 位置 | 分类 |",
            "|------|------|-----|------|------|"
        ])
        
        # 改进的词法单元分类函数，将运算符和分隔符明确区分
        def get_token_category(token_type):
            if token_type in ['int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'switch', 'case', 'default']:
                return "关键字"
            elif token_type == 'id':
                return "标识符"
            elif token_type == 'num':
                return "数值常量"
            elif token_type in ['+', '-', '*', '/', '%', '=', '<', '>', '!', '&', '|', '?', ':', 
                               '++', '--', '==', '!=', '<=', '>=', '&&', '||', '->', '<<', '>>']:
                return "运算符"
            elif token_type in [';', ',', '.', '(', ')', '[', ']', '{', '}']:
                return "分隔符"
            else:
                return "其他"
        
        # 添加词法单元详细信息，包括分类
        for i, (token_type, value, position) in enumerate(detailed_tokens):
            category = get_token_category(token_type)
            md_lines.append(f"| {i+1} | {token_type} | {value} | {position} | {category} |")
        
        # 符号表分析
        symbols = {}
        for token_type, value, position in detailed_tokens:
            if token_type == 'id':
                if value not in symbols:
                    symbols[value] = []
                symbols[value].append(position)
        
        md_lines.extend([
            "",
            "## 符号表分析",
            "",
            "| 标识符 | 出现次数 | 出现位置 |",
            "|--------|----------|----------|"
        ])
        
        for symbol, positions in sorted(symbols.items()):
            md_lines.append(f"| {symbol} | {len(positions)} | {', '.join(map(str, positions))} |")
        
        # 将词法元素频率分析替换为按类别的汇总分析
        md_lines.extend([
            "",
            "## 词法单元类别汇总",
            "",
            "### 关键字",
            ""
        ])
        
        # 按类别列出所有关键字及其出现次数
        keywords_list = [t for t in detailed_tokens if t[0] in [
            'int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'switch', 'case', 'default'
        ]]
        
        if keywords_list:
            keyword_count = {}
            for token_type, _, _ in keywords_list:
                if token_type not in keyword_count:
                    keyword_count[token_type] = 0
                keyword_count[token_type] += 1
            
            md_lines.append("| 关键字 | 出现次数 |")
            md_lines.append("|--------|----------|")
            
            for keyword, count in sorted(keyword_count.items()):
                md_lines.append(f"| {keyword} | {count} |")
            md_lines.append(f"\n**关键字总数:** {len(keywords_list)}")
        else:
            md_lines.append("*未发现关键字*")
        
        # 标识符分析
        md_lines.extend([
            "",
            "### 标识符",
            ""
        ])
        
        identifiers = [t for t in detailed_tokens if t[0] == 'id']
        if identifiers:
            md_lines.append(f"**标识符总数:** {len(identifiers)}")
            md_lines.append(f"**唯一标识符数:** {len(symbols)}")
            
            # 列出使用频率最高的前10个标识符
            if symbols:
                top_symbols = sorted(symbols.items(), key=lambda x: len(x[1]), reverse=True)[:10]
                md_lines.append("\n**使用频率最高的标识符:**")
                md_lines.append("\n| 标识符 | 出现次数 |")
                md_lines.append("|--------|----------|")
                for symbol, positions in top_symbols:
                    md_lines.append(f"| {symbol} | {len(positions)} |")
        else:
            md_lines.append("*未发现标识符*")
        
        # 常量分析
        md_lines.extend([
            "",
            "### 常量",
            ""
        ])
        
        constants = [t for t in detailed_tokens if t[0] == 'num']
        if constants:
            constant_values = {}
            for _, value, _ in constants:
                if value not in constant_values:
                    constant_values[value] = 0
                constant_values[value] += 1
            
            md_lines.append(f"**数字常量总数:** {len(constants)}")
            md_lines.append(f"**唯一数字常量数:** {len(constant_values)}")
            
            # 列出所有出现的常量值
            md_lines.append("\n**出现的数字常量:**")
            md_lines.append("\n| 常量值 | 出现次数 |")
            md_lines.append("|--------|----------|")
            for value, count in sorted(constant_values.items()):
                md_lines.append(f"| {value} | {count} |")
        else:
            md_lines.append("*未发现数字常量*")
        
        # 运算符分析
        md_lines.extend([
            "",
            "### 运算符",
            ""
        ])
        
        # 定义运算符分类
        operator_categories = {
            "算术运算符": ['+', '-', '*', '/', '%'],
            "赋值运算符": ['=', '+=', '-=', '*=', '/=', '%=', '<<=', '>>=', '&=', '^=', '|='],
            "关系运算符": ['==', '!=', '<', '>', '<=', '>='],
            "逻辑运算符": ['&&', '||', '!'],
            "位运算符": ['&', '|', '^', '~', '<<', '>>'],
            "自增/自减运算符": ['++', '--']
        }
        
        operators = [t for t in detailed_tokens if t[0] in [
            '+', '-', '*', '/', '%', '=', '<', '>', '!', '&', '|', '?', ':', '++', '--', 
            '==', '!=', '<=', '>=', '&&', '||', '->', '<<', '>>'
        ]]
        
        if operators:
            md_lines.append(f"**运算符总数:** {len(operators)}")
            
            # 按类别统计运算符
            categorized_operators = {category: [] for category in operator_categories}
            other_operators = []
            
            for token_type, value, _ in operators:
                categorized = False
                for category, ops in operator_categories.items():
                    if token_type in ops:
                        categorized_operators[category].append(token_type)
                        categorized = True
                        break
                if not categorized:
                    other_operators.append(token_type)
            
            # 展示各类运算符统计
            for category, ops in operator_categories.items():
                ops_in_category = [op for op in categorized_operators[category]]
                if ops_in_category:
                    op_count = {}
                    for op in ops_in_category:
                        if op not in op_count:
                            op_count[op] = 0
                        op_count[op] += 1
                    
                    md_lines.append(f"\n**{category}:** {len(ops_in_category)}")
                    md_lines.append("\n| 运算符 | 出现次数 |")
                    md_lines.append("|--------|----------|")
                    for op, count in sorted(op_count.items()):
                        md_lines.append(f"| {op} | {count} |")
        else:
            md_lines.append("*未发现运算符*")
        
        # 分隔符分析
        md_lines.extend([
            "",
            "### 分隔符",
            ""
        ])
        
        delimiters = [t for t in detailed_tokens if t[0] in [';', ',', '.', '(', ')', '[', ']', '{', '}']]
        if delimiters:
            delimiter_count = {}
            for token_type, _, _ in delimiters:
                if token_type not in delimiter_count:
                    delimiter_count[token_type] = 0
                delimiter_count[token_type] += 1
            
            md_lines.append(f"**分隔符总数:** {len(delimiters)}")
            md_lines.append("\n| 分隔符 | 出现次数 |")
            md_lines.append("|--------|----------|")
            for delimiter, count in sorted(delimiter_count.items()):
                md_lines.append(f"| {delimiter} | {count} |")
        else:
            md_lines.append("*未发现分隔符*")
        
        # 总体统计结果 - 简化版
        md_lines.extend([
            "",
            "## 词法分析结果汇总",
            "",
            f"- **总词法单元数:** {len(detailed_tokens)}",
            f"- **关键字数量:** {len(keywords_list)}",
            f"- **标识符数量:** {len(identifiers)}",
            f"- **唯一标识符数量:** {len(symbols)}",
            f"- **数字常量数量:** {len(constants)}",
            f"- **运算符数量:** {len(operators)}",
            f"- **分隔符数量:** {len(delimiters)}"
        ])
        
        # 添加简洁的分析总结
        md_lines.extend([
            "",
            "## 分析总结",
            "",
            "此代码的词法特点：",
            ""
        ])
        
        # 基于统计结果生成分析结论
        if len(detailed_tokens) > 0:
            # 计算标识符与关键字比例
            id_count = sum(1 for t in detailed_tokens if t[0] == 'id')
            kw_count = sum(1 for t in detailed_tokens if t[0] in ['int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'switch', 'case', 'default'])
            
            if id_count > kw_count * 3:
                md_lines.append("- 代码中标识符使用频繁，可能包含较多的变量定义和引用")
            elif id_count < kw_count:
                md_lines.append("- 代码中关键字使用频繁，控制结构比例较高")
            
            # 分析运算符使用
            if len(operators) > len(detailed_tokens) * 0.3:
                md_lines.append("- 代码中运算符比例较高，可能包含复杂的表达式计算")
            
            # 分析结构
            if sum(1 for t in detailed_tokens if t[0] in ['{', '}']) > 0:
                md_lines.append("- 代码包含块结构，具有一定的嵌套复杂度")
        else:
            md_lines.append("- 未检测到有效的词法单元")
        
        return "\n".join(md_lines)
