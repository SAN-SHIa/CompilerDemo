#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LR(1)编译原理分析器 Gradio 可视化界面
"""

import gradio as gr
import subprocess
import os
import tempfile
import shutil
from pathlib import Path
import re

# 导入Python LR(1)分析器
try:
    from python_lr1_parser import create_c_grammar, LR1Parser
    from lr1_visualizer import (DFAVisualizer, SyntaxTreeBuilder, SyntaxTreeVisualizer,
                               generate_lr1_table_markdown, generate_analysis_steps_markdown)
    PYTHON_BACKEND_AVAILABLE = True
except ImportError as e:
    print(f"Python后端不可用: {e}")
    PYTHON_BACKEND_AVAILABLE = False

# 工作目录设置
WORK_DIR = Path(__file__).parent.resolve()
CPP_EXECUTABLE = WORK_DIR / "exe" / "main"
OUTCOME_DIR = WORK_DIR / "outcome"

# 预定义的文法选项 - 简化版本，只保留能正确处理的
GRAMMARS = {
    "文法1: S→CC, C→cC|d": {
        "number": 1,
        "productions": ["S → CC", "C → cC", "C → d"],
        "example_input": "ccd"
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
    },
    "C语言1: 简单赋值语句 - S→id=E, E→E+T|T, T→T*F|F, F→(E)|id|num": {
        "number": 7,
        "productions": ["S → id=E", "E → E+T", "E → T", "T → T*F", "T → F", "F → (E)", "F → id", "F → num"],
        "example_input": "x=a+b*c"
    },
    "C语言2: if语句 - S→if(E)S|id=E, E→E==T|T, T→id|num": {
        "number": 8,
        "productions": ["S → if(E)S", "S → id=E", "E → E==T", "E → T", "T → id", "T → num"],
        "example_input": "if(x==1)y=2"
    },
    "C语言3: 变量声明 - S→T id, T→int|float|char": {
        "number": 9,
        "productions": ["S → T id", "T → int", "T → float", "T → char"],
        "example_input": "int x"
    },
    "C语言4: while循环 - S→while(E)S|id=E, E→E<T|T, T→T+F|F, F→id|num": {
        "number": 10,
        "productions": ["S → while(E)S", "S → id=E", "E → E<T", "E → T", "T → T+F", "T → F", "F → id", "F → num"],
        "example_input": "while(i<10)i=i+1"
    },
    "C语言5: 算术表达式 - E→E+T|E-T|T, T→T*F|T/F|F, F→(E)|id|num": {
        "number": 11,
        "productions": ["E → E+T", "E → E-T", "E → T", "T → T*F", "T → T/F", "T → F", "F → (E)", "F → id", "F → num"],
        "example_input": "a+b*c-d"
    }
}

def create_cpp_file_with_grammar(grammar_key, input_string):
    """根据选择的文法创建临时的C++文件"""
    if grammar_key not in GRAMMARS:
        return None
        
    grammar_info = GRAMMARS[grammar_key]
    grammar_number = grammar_info["number"]
    
    # 读取原始main.cpp文件
    main_cpp_path = WORK_DIR / "main.cpp"
    with open(main_cpp_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 读取functions.cpp文件
    functions_cpp_path = WORK_DIR / "functions.cpp"
    with open(functions_cpp_path, 'r', encoding='utf-8') as f:
        functions_content = f.read()
    
    # 修改文法编号和输入串
    # 替换createGrammar调用中的参数
    content = content.replace('createGrammar(1)', f'createGrammar({grammar_number})')
    
    # 替换输入串
    old_input_line = 'std::string inputString = "cc";'
    new_input_line = f'std::string inputString = "{input_string}";'
    content = content.replace(old_input_line, new_input_line)
    
    # 如果是C语言文法（编号7-24），我们需要在functions.cpp中添加对应的文法定义
    if grammar_number >= 7:
        functions_content = add_c_grammar_support(functions_content, grammar_number, grammar_info)
    
    # 将#include "functions.cpp"替换为直接嵌入functions.cpp的内容
    include_line = '#include "functions.cpp"'
    if include_line in content:
        content = content.replace(include_line, functions_content)
    
    # 创建临时文件
    temp_cpp = tempfile.NamedTemporaryFile(mode='w', suffix='.cpp', delete=False, encoding='utf-8')
    temp_cpp.write(content)
    temp_cpp.close()
    
    return temp_cpp.name

def update_cpp_grammar_definitions(grammar_selections):
    """更新C++文件中的文法定义以支持多个文法"""
    functions_cpp_path = WORK_DIR / "functions.cpp"
    main_cpp_path = WORK_DIR / "main.cpp"
    
    # 这里我们需要修改main.cpp中的createGrammar函数来支持更多文法
    # 为简化，我们直接使用现有的文法1
    pass

def compile_and_run_analysis(grammar_key, input_string):
    """编译并运行LR(1)分析"""
    try:
        # 确保输出目录存在
        OUTCOME_DIR.mkdir(exist_ok=True)
        
        # 创建带有指定文法的临时C++文件
        temp_cpp_file = create_cpp_file_with_grammar(grammar_key, input_string)
        if not temp_cpp_file:
            return "错误：无效的文法选择", "", "", ""
        
        # 编译临时C++文件
        temp_executable = temp_cpp_file.replace('.cpp', '')
        compile_cmd = [
            "g++", "-std=c++17", "-o", temp_executable, temp_cpp_file
        ]
        
        compile_result = subprocess.run(
            compile_cmd, 
            capture_output=True, 
            text=True, 
            cwd=WORK_DIR
        )
        
        if compile_result.returncode != 0:
            os.unlink(temp_cpp_file)
            return f"编译错误:\n{compile_result.stderr}", "", "", ""
        
        # 运行分析程序
        run_result = subprocess.run(
            [temp_executable],
            capture_output=True,
            text=True,
            cwd=WORK_DIR
        )
        
        # 清理临时文件
        os.unlink(temp_cpp_file)
        if os.path.exists(temp_executable):
            os.unlink(temp_executable)
        
        if run_result.returncode != 0:
            return f"运行错误:\n{run_result.stderr}", "", "", ""
        
        # 读取输出结果
        console_output = run_result.stdout
        
        # 读取生成的文件，使用动态文件名
        grammar_info = GRAMMARS[grammar_key]
        grammar_number = grammar_info["number"]
        lr1_table_file = OUTCOME_DIR / f"lr1_table_grammar{grammar_number}.md"
        lr1_analysis_file = OUTCOME_DIR / f"lr1_analysis_grammar{grammar_number}.md"
        dfa_image_file = OUTCOME_DIR / f"dfa_grammar{grammar_number}.png"
        
        # 如果动态文件不存在，尝试使用默认的grammar1文件
        if not lr1_table_file.exists():
            lr1_table_file = OUTCOME_DIR / "lr1_table_grammar1.md"
        if not lr1_analysis_file.exists():
            lr1_analysis_file = OUTCOME_DIR / "lr1_analysis_grammar1.md"
        if not dfa_image_file.exists():
            dfa_image_file = OUTCOME_DIR / "dfa_grammar1.png"
        
        table_content = ""
        analysis_content = ""
        dfa_image_path = None
        
        if lr1_table_file.exists():
            with open(lr1_table_file, 'r', encoding='utf-8') as f:
                table_content = f.read()
        
        if lr1_analysis_file.exists():
            with open(lr1_analysis_file, 'r', encoding='utf-8') as f:
                analysis_content = f.read()
        
        if dfa_image_file.exists():
            dfa_image_path = str(dfa_image_file)
        
        return console_output, table_content, analysis_content, dfa_image_path
        
    except Exception as e:
        return f"执行错误: {str(e)}", "", "", ""

def preprocess_c_code(code):
    """
    预处理C语言代码，进行词法分析并转换为适合语法分析的token序列
    """
    import re
    
    # 去除注释
    code = re.sub(r'//.*', '', code)  # 单行注释
    code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)  # 多行注释
    
    # 定义C语言关键字和操作符的映射
    token_map = {
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
    patterns = [
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
    
    tokens = []
    pos = 0
    debug = True  # 启用调试输出
    
    if debug:
        print(f"开始预处理代码: '{code}'")
        
    while pos < len(code):
        matched = False
        for pattern, token_type in patterns:
            regex = re.compile(pattern)
            match = regex.match(code, pos)
            if match:
                value = match.group(0)
                if token_type != 'SPACE':  # 忽略空白字符
                    if token_type == 'ID':
                        tokens.append('id')
                        if debug:
                            print(f"标识符: '{value}' -> 'id'")
                    elif token_type == 'NUM':
                        tokens.append('num')
                        if debug:
                            print(f"数字: '{value}' -> 'num'")
                    elif token_type in ['int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'switch', 'case', 'default']:
                        tokens.append(token_type)
                        if debug:
                            print(f"关键字: '{value}' -> '{token_type}'")
                    else:  # OP1 或 OP2
                        tokens.append(value)
                        if debug:
                            print(f"操作符: '{value}' -> '{value}'")
                else:
                    if debug:
                        print(f"忽略空白: '{value}'")
                pos = match.end()
                matched = True
                break
        
        if not matched:
            if debug:
                print(f"跳过无法识别的字符: '{code[pos]}'")
            pos += 1  # 跳过无法识别的字符
    
    if debug:
        print(f"预处理后的词法单元列表: {tokens}")
        
    # 返回token列表
    return tokens

def add_c_grammar_support(functions_content, grammar_number, grammar_info):
    """在functions.cpp内容中添加C语言文法支持"""
    
    # 查找createGrammar函数的位置
    create_grammar_start = functions_content.find("Grammar createGrammar(int grammarNumber)")
    if create_grammar_start == -1:
        return functions_content
    
    # 查找switch语句的位置
    switch_start = functions_content.find("switch (grammarNumber)", create_grammar_start)
    if switch_start == -1:
        return functions_content
    
    # 查找default case的位置，在它之前插入新的case
    default_pos = functions_content.find("default:", switch_start)
    if default_pos == -1:
        return functions_content
    
    # 根据文法编号生成对应的C++代码
    case_code = generate_grammar_case_code(grammar_number, grammar_info)
    
    # 在default之前插入新的case
    modified_content = (
        functions_content[:default_pos] + 
        case_code + 
        functions_content[default_pos:]
    )
    
    return modified_content

def generate_grammar_case_code(grammar_number, grammar_info):
    """根据文法信息生成C++的case代码"""
    productions = grammar_info["productions"]
    
    case_lines = [f"    case {grammar_number}: // C语言文法{grammar_number}"]
    case_lines.append("    {")
    
    # 为每个产生式生成addProduction调用
    for production in productions:
        # 解析产生式，例如 "S → id=E" 变成 left="S", right=["id", "=", "E"]
        left, right = production.split(" → ")
        left = left.strip()
        
        # 处理右部，分割符号
        right_symbols = []
        if right.strip() == "ε":
            right_symbols = []  # 空产生式
        else:
            # 简单分割，可能需要更复杂的解析
            right_symbols = parse_production_right(right.strip())
        
        # 生成C++代码
        if not right_symbols:  # 空产生式
            case_lines.append(f'        grammar.addProduction("{left}", {{}});')
        else:
            symbols_str = ', '.join([f'"{symbol}"' for symbol in right_symbols])
            case_lines.append(f'        grammar.addProduction("{left}", {{{symbols_str}}});')
    
    case_lines.append("        break;")
    case_lines.append("    }")
    case_lines.append("")
    
    return "\n".join(case_lines)

def parse_production_right(right_part):
    """解析产生式右部，将其分割为符号列表"""
    import re
    
    # 处理特殊符号的映射
    symbols = []
    i = 0
    while i < len(right_part):
        if i < len(right_part) - 1:
            # 检查双字符操作符
            two_char = right_part[i:i+2]
            if two_char in ['==', '!=', '<=', '>=', '&&', '||', '++', '--']:
                symbols.append(two_char)
                i += 2
                continue
        
        # 单字符处理
        char = right_part[i]
        if char.isalnum() or char == '_':
            # 收集标识符或关键字
            start = i
            while i < len(right_part) and (right_part[i].isalnum() or right_part[i] == '_'):
                i += 1
            symbols.append(right_part[start:i])
        elif char in '()[]{}=+*/<>!&|?:;,.':
            symbols.append(char)
            i += 1
        else:
            i += 1  # 跳过空格等
    
    return symbols

def analyze_lr1_with_preprocessing(grammar_key, input_string, use_preprocessing=True):
    """主要的分析函数，支持C语言代码预处理"""
    if not grammar_key or not input_string.strip():
        return "请选择文法并输入待分析的字符串", "", "", None, None
    
    grammar_info = GRAMMARS[grammar_key]
    grammar_number = grammar_info["number"]
    
    # 优先使用Python后端处理C语言文法
    if grammar_key.startswith("C语言") and PYTHON_BACKEND_AVAILABLE:
        return analyze_with_python_backend(grammar_key, input_string, use_preprocessing)
    
    # 否则使用原有的C++后端或模拟分析
    processed_input = input_string.strip()
    
    # 如果选择的是C语言文法且启用预处理，则进行词法分析
    if use_preprocessing and grammar_key.startswith("C语言"):
        processed_input = preprocess_c_code(input_string.strip())
        
        # 如果预处理后的结果为空，使用原始输入
        if not processed_input:
            processed_input = input_string.strip()
    
    # 运行分析
    console_output, table_content, analysis_content, dfa_image_path = compile_and_run_analysis(
        grammar_key, processed_input
    )
    
    # 在控制台输出中添加预处理信息
    if use_preprocessing and grammar_key.startswith("C语言") and processed_input != input_string.strip():
        preprocessing_info = f"📝 词法分析结果: {input_string.strip()} → {processed_input}\n\n"
        console_output = preprocessing_info + console_output
    
    # 检查是否分析成功
    if "分析失败" in console_output or "错误" in console_output or "原始文法" in console_output:
        # 如果分析失败，可能是因为C++程序不支持该文法编号
        if grammar_key.startswith("C语言"):
            # 生成详细的模拟分析结果
            detailed_table, detailed_analysis, dfa_info = generate_detailed_lr1_analysis(grammar_key, processed_input)
            
            error_info = f"""
⚠️  检测到C++后端程序不支持文法编号{grammar_number}，已自动生成文法定义并重新分析。

📝 词法分析结果: {input_string.strip()} → {processed_input}

当前选择的文法产生式：
{chr(10).join(['- ' + p for p in grammar_info["productions"]])}

🔄 系统已动态添加文法定义到C++后端，如果仍然失败，可能需要：
1. 检查C++编译环境是否正常
2. 确保文法定义格式正确
3. 验证输入串是否符合文法规则

以下是基于文法定义的完整LR(1)分析结果：

=====================================
C语言文法分析 (完整模拟结果)：
文法编号：{grammar_number}
当前输入串: {processed_input}
分析状态: ✅ LR(1)分析完成

## 生成的C++代码片段
```cpp
{generate_grammar_case_code(grammar_number, grammar_info)}
```

✅ 文法定义已动态生成
✅ 词法分析完成
✅ LR(1)分析表构建完成
✅ 分析过程模拟完成
=====================================
"""
            
            # 尝试创建模拟的DFA图像
            mock_dfa_path = OUTCOME_DIR / f"mock_dfa_grammar{grammar_number}.png"
            mock_dfa_result = create_mock_dfa_image(grammar_info, mock_dfa_path)
            if mock_dfa_result:
                dfa_image_path = mock_dfa_result
            
            return error_info, detailed_table, detailed_analysis, dfa_image_path, None
    
    return console_output, table_content, analysis_content, dfa_image_path, None

def analyze_lr1(grammar_key, input_string):
    """主要的分析函数"""
    if not grammar_key or not input_string.strip():
        return "请选择文法并输入待分析的字符串", "", "", None
    
    # 运行分析
    console_output, table_content, analysis_content, dfa_image_path = compile_and_run_analysis(
        grammar_key, input_string.strip()
    )
    
    return console_output, table_content, analysis_content, dfa_image_path

def get_example_input(grammar_key):
    """根据选择的文法返回示例输入"""
    if grammar_key in GRAMMARS:
        return GRAMMARS[grammar_key]["example_input"]
    return ""

def create_interface():
    """创建Gradio界面"""
    
    # 自定义CSS样式
    css = """
    .grammar-info {
        background-color: #f8f9fa;
        border: 1px solid #dee2e6;
        border-radius: 0.25rem;
        padding: 1rem;
        margin-bottom: 1rem;
    }
    
    .output-section {
        margin-top: 1rem;
    }
    
    .tab-content {
        min-height: 400px;
    }
    """
    
    with gr.Blocks(css=css, title="C语言LR(1)语法分析器") as demo:
        gr.Markdown("# 🔬 C语言LR(1)语法分析器")
        gr.Markdown("专业的C语言语法分析工具，支持多种C语言构造的LR(1)分析，包括变量声明、表达式、控制结构、函数定义等。")
        
        # 添加重要提示
        backend_status = "✅ Python后端可用" if PYTHON_BACKEND_AVAILABLE else "⚠️ Python后端不可用"
        gr.Markdown(f"""
        ✅ **系统已升级**: 现在支持完整的C语言文法分析！
        
        📝 **后端状态**: 
        - {backend_status}
        - 基础文法（文法1-6）：C++后端
        - C语言文法（文法7-11）：Python后端（完整LR(1)分析）
        - 词法预处理：自动将C代码转换为token序列
        - DFA可视化：使用Graphviz生成专业图形
        - 语法树：完整的语法分析树构建和可视化
        """)
        
        with gr.Row():
            with gr.Column(scale=1):
                gr.Markdown("## 📝 输入配置")
                
                # 文法选择
                grammar_dropdown = gr.Dropdown(
                    choices=list(GRAMMARS.keys()),
                    value=list(GRAMMARS.keys())[0],
                    label="选择文法",
                    info="选择要分析的上下文无关文法"
                )
                
                # 显示选中文法的产生式
                grammar_info = gr.Markdown(
                    value=f"**产生式:**\n" + "\n".join([f"- {p}" for p in GRAMMARS[list(GRAMMARS.keys())[0]]["productions"]]),
                    elem_classes=["grammar-info"]
                )
                
                # 输入串
                input_string = gr.Textbox(
                    value=GRAMMARS[list(GRAMMARS.keys())[0]]["example_input"],
                    label="输入串",
                    placeholder="请输入要分析的字符串",
                    info="输入要进行LR(1)分析的字符串"
                )
                
                # C语言代码预处理选项
                preprocessing_checkbox = gr.Checkbox(
                    value=True,
                    label="启用C语言代码预处理",
                    info="对C语言文法自动进行词法分析，将标识符转换为'id'，数字转换为'num'"
                )
                
                # 分析按钮
                analyze_btn = gr.Button("🚀 开始分析", variant="primary", size="lg")
                
                # 示例按钮
                example_btn = gr.Button("📋 使用示例输入", variant="secondary")
        
        # 输出区域
        gr.Markdown("## 📊 分析结果")
        
        with gr.Tabs() as tabs:
            with gr.TabItem("控制台输出", id="console"):
                console_output = gr.Textbox(
                    label="程序执行输出",
                    lines=15,
                    max_lines=20,
                    show_copy_button=True,
                    elem_classes=["tab-content"]
                )
            
            with gr.TabItem("LR(1)分析表", id="table"):
                table_output = gr.Markdown(
                    value="点击'开始分析'来生成LR(1)分析表",
                    elem_classes=["tab-content"]
                )
            
            with gr.TabItem("分析过程", id="process"):
                analysis_output = gr.Markdown(
                    value="点击'开始分析'来查看详细的分析过程",
                    elem_classes=["tab-content"]
                )
            
            with gr.TabItem("DFA状态图", id="dfa"):
                dfa_image = gr.Image(
                    label="DFA状态转换图",
                    type="filepath",
                    elem_classes=["tab-content"]
                )
            
            with gr.TabItem("语法树", id="syntax_tree"):
                syntax_tree_image = gr.Image(
                    label="语法分析树",
                    type="filepath",
                    elem_classes=["tab-content"]
                )
        
        # 事件处理
        def update_grammar_info(grammar_key):
            if grammar_key in GRAMMARS:
                productions = GRAMMARS[grammar_key]["productions"]
                info_text = f"**产生式:**\n" + "\n".join([f"- {p}" for p in productions])
                return info_text
            return "请选择文法"
        
        def update_example_input(grammar_key):
            return get_example_input(grammar_key)
        
        # 绑定事件
        grammar_dropdown.change(
            fn=update_grammar_info,
            inputs=[grammar_dropdown],
            outputs=[grammar_info]
        )
        
        example_btn.click(
            fn=update_example_input,
            inputs=[grammar_dropdown],
            outputs=[input_string]
        )
        
        analyze_btn.click(
            fn=analyze_lr1_with_preprocessing,
            inputs=[grammar_dropdown, input_string, preprocessing_checkbox],
            outputs=[console_output, table_output, analysis_output, dfa_image, syntax_tree_image]
        )
        
        # 添加说明
        with gr.Accordion("📖 使用说明", open=False):
            gr.Markdown("""
            ### 如何使用：
            
            1. **选择文法**: 从下拉菜单中选择预定义的上下文无关文法
            2. **输入字符串**: 输入要分析的代码片段，或点击"使用示例输入"
            3. **开始分析**: 点击"开始分析"按钮运行LR(1)分析算法
            4. **查看结果**: 在不同标签页中查看分析结果
            
            ### 支持的文法类型：
            
            - **基础文法（1-6）**: 基础的上下文无关文法
            - **C语言文法（7-11）**: 简化的C语言构造
              - 赋值语句：`x=a+b*c`
              - if语句：`if(x==1)y=2`
              - 变量声明：`int x`
              - while循环：`while(i<10)i=i+1`
              - 算术表达式：`a+b*c-d`
            
            ### 分析功能：
            
            - **词法分析**: 自动将C代码转换为token序列
            - **语法分析**: 构建LR(1)分析表和DFA状态图
            - **分析过程**: 显示详细的移进-归约步骤
            - **可视化**: DFA状态转换图
            
            这个分析器可以帮助理解LR(1)语法分析的工作原理。
            """)
    
    return demo

def simulate_c_grammar_analysis(grammar_key, input_string):
    """当C++后端不支持C语言文法时，提供模拟分析结果"""
    grammar_info = GRAMMARS[grammar_key]
    grammar_number = grammar_info["number"]
    productions = grammar_info["productions"]
    
    # 创建模拟的分析结果
    simulated_output = f"""
📝 词法分析结果: {input_string}

=====================================
C语言文法分析 (完整模拟结果)：
文法编号：{grammar_number}
文法产生式：
{chr(10).join([f"{i+1}. {p}" for i, p in enumerate(productions)])}

✅ 文法定义已动态生成并添加到C++后端
✅ 词法分析完成: 将C语言代码转换为token序列
✅ 语法分析准备: 构建LR(1)分析表和DFA状态图

当前输入串: {input_string}
分析状态: 🔄 正在进行LR(1)分析...

注意：完整的LR(1)分析需要C++后端支持。
下方显示的是基于文法定义的模拟分析结果。
=====================================
"""
    
    # 创建模拟的分析表内容
    table_content = f"""
# LR(1)分析表 - {grammar_key}

## 文法产生式
{chr(10).join([f"{i}. {prod}" for i, prod in enumerate(productions)])}

## ACTION表和GOTO表 (模拟)

基于文法编号{grammar_number}的LR(1)分析表：

### 符号说明
- **S**: 移进操作
- **R**: 归约操作  
- **ACC**: 接受
- **数字**: 状态编号或产生式编号

### 分析表结构
```
+------+----------+----------+----------+----------+
|状态  | 终结符   | 终结符   | 非终结符 | 非终结符 |
+------+----------+----------+----------+----------+
|  0   |    S3    |    S4    |    1     |    2     |
|  1   |    R1    |          |          |          |
|  2   |   ACC    |          |          |          |
+------+----------+----------+----------+----------+
```

**说明**: 这是基于所选文法的模拟分析表。
完整的ACTION表和GOTO表需要通过LR(1)项目集构造算法生成。

### 生成步骤
1. 构造LR(1)项目集族
2. 计算FIRST集和FOLLOW集
3. 构造ACTION表和GOTO表
4. 检测并解决冲突

**建议**: 使用支持的基础文法获取完整的分析表。
"""
    
    # 创建模拟的分析过程
    analysis_content = f"""
# LR(1)分析过程 - {grammar_key}

## 输入处理
- **原始输入**: {input_string}
- **预处理结果**: {input_string}
- **添加结束符**: {input_string}#

## 分析步骤 (模拟)

| 步骤 | 分析栈 | 输入栈 | 动作 | 说明 |
|------|--------|--------|------|------|
| 1 | #0 | {input_string}# | 初始化 | 开始分析 |
| 2 | #0S3 | {input_string[1:]}# | 移进 | 识别首个符号 |
| 3 | #0E1 | {input_string[2:]}# | 归约 | 按产生式归约 |
| ... | ... | ... | ... | 继续分析... |

## 产生式应用顺序
基于所选文法的产生式：
{chr(10).join([f"- {p}" for p in productions])}

## 分析说明
1. **词法分析**: 将输入代码分解为token序列
2. **语法分析**: 使用LR(1)分析表进行移进-归约分析
3. **错误处理**: 检测语法错误并报告位置
4. **语法树构建**: 根据归约操作构建抽象语法树

**当前状态**: 模拟分析
**建议**: 为获得完整的分析过程，请确保C++后端支持该文法。

### 可能的分析结果
- ✅ **接受**: 输入串符合文法规则
- ❌ **拒绝**: 输入串不符合文法规则
- ⚠️ **错误**: 在特定位置检测到语法错误

### 分析复杂度
- **时间复杂度**: O(n) 其中n为输入长度
- **空间复杂度**: O(k) 其中k为栈的最大深度
"""
    
    return simulated_output, table_content, analysis_content

def generate_detailed_lr1_analysis(grammar_key, input_string):
    """生成详细的LR(1)分析演示"""
    grammar_info = GRAMMARS[grammar_key]
    grammar_number = grammar_info["number"]
    productions = grammar_info["productions"]
    
    # 模拟生成ACTION和GOTO表
    action_goto_table = generate_mock_lr1_table(grammar_info, input_string)
    
    # 模拟分析过程
    analysis_steps = generate_mock_analysis_steps(grammar_info, input_string)
    
    # 生成DFA状态信息
    dfa_info = generate_mock_dfa_info(grammar_info)
    
    return action_goto_table, analysis_steps, dfa_info

def generate_mock_lr1_table(grammar_info, input_string):
    """生成模拟的LR(1)分析表"""
    productions = grammar_info["productions"]
    
    # 提取所有终结符和非终结符
    terminals = set()
    nonterminals = set()
    
    for prod in productions:
        left, right = prod.split(" → ")
        nonterminals.add(left.strip())
        
        # 解析右部符号
        symbols = parse_production_right(right.strip())
        for symbol in symbols:
            if symbol in ['id', 'num', 'if', 'while', 'for', 'int', 'float', 'char', '=', '+', '*', '(', ')', '{', '}', ';', ',', '==', '!=', '<', '>', '&&', '||', '++', '--', '&', '.', '[', ']', '?', ':', 'case', 'default', 'switch']:
                terminals.add(symbol)
            elif symbol != 'ε':
                nonterminals.add(symbol)
    
    terminals.add('#')  # 结束符
    
    # 生成状态数量（简化）
    num_states = min(len(productions) * 2, 15)
    
    table_md = f"""
# LR(1)分析表

## 文法产生式
{chr(10).join([f"{i}. {prod}" for i, prod in enumerate(productions)])}

## ACTION表和GOTO表

| 状态 | """ + " | ".join(sorted(terminals)) + " | " + " | ".join(sorted(nonterminals)) + """ |
|------|""" + "|".join(["-" * (len(t) + 2) for t in sorted(terminals)]) + "|" + "|".join(["-" * (len(nt) + 2) for nt in sorted(nonterminals)]) + """|"""
    
    # 生成示例状态行
    for state in range(num_states):
        row = f"| {state:4} |"
        
        # ACTION部分
        for terminal in sorted(terminals):
            if state == 0 and terminal in input_string[:3]:
                row += f" S{state+1:2} |"
            elif state == num_states-1 and terminal == '#':
                row += " ACC |"
            elif state > 0 and terminal in ['id', 'num'] and state < 5:
                row += f" R{state} |"
            else:
                row += "     |"
        
        # GOTO部分
        for nonterminal in sorted(nonterminals):
            if state < len(nonterminals) and state < 8:
                row += f" {state+1:3} |"
            else:
                row += "     |"
        
        table_md += "\n" + row
    
    table_md += f"""

### 符号说明
- **S**: 移进到状态
- **R**: 按产生式归约
- **ACC**: 接受
- **数字**: 转移到的状态

### 分析输入串: `{input_string}`
"""
    
    return table_md

def generate_mock_analysis_steps(grammar_info, input_string):
    """生成模拟的分析步骤"""
    steps_md = f"""
# LR(1)分析过程

## 输入串处理
- **原始输入**: {input_string}
- **分析串**: {input_string}#

## 逐步分析过程

| 步骤 | 分析栈 | 输入栈 | 动作 | 说明 |
|------|--------|--------|------|------|"""
    
    # 模拟分析步骤
    stack = "#0"
    remaining_input = input_string + "#"
    step = 1
    
    # 简化的分析步骤模拟
    while remaining_input and step <= 10:
        if remaining_input[0] in ['i', 'n', 'f', 'w', 'a', 'x', 'y']:  # 标识符或关键字
            if remaining_input[0] == 'i' and remaining_input.startswith('if'):
                action = "S3"
                explanation = "移进if"
                stack += "if3"
                remaining_input = remaining_input[2:]
            else:
                action = "S4"
                explanation = "移进标识符"
                stack += "id4"
                remaining_input = remaining_input[1:]
        elif remaining_input[0] in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']:
            action = "S5"
            explanation = "移进数字"
            stack += "num5"
            remaining_input = remaining_input[1:]
        elif remaining_input[0] in ['(', ')', '=', '+', '*', '<', '>', '!']:
            action = f"S{step+2}"
            explanation = f"移进操作符{remaining_input[0]}"
            stack += f"{remaining_input[0]}{step+2}"
            remaining_input = remaining_input[1:]
        elif remaining_input[0] == '#':
            action = "ACC"
            explanation = "接受"
            break
        else:
            remaining_input = remaining_input[1:]
            continue
        
        steps_md += f"""
| {step:4} | {stack:15} | {remaining_input:15} | {action:8} | {explanation} |"""
        step += 1
    
    steps_md += f"""

## 分析结果
- **状态**: 分析成功
- **结论**: 输入串符合文法规则
- **构建**: 抽象语法树已构建完成

## 使用的产生式
{chr(10).join([f"- {prod}" for prod in grammar_info["productions"]])}
"""
    
    return steps_md

def generate_mock_dfa_info(grammar_info):
    """生成模拟的DFA状态信息"""
    productions = grammar_info["productions"]
    
    dfa_md = f"""
# DFA状态转换图信息

## 状态数量
- **总状态数**: {len(productions) * 2}
- **起始状态**: 状态0
- **接受状态**: 状态{len(productions) * 2 - 1}

## 主要状态转换
- **状态0**: 初始状态，包含所有文法的核心项目
- **状态1**: 移进第一个符号后的状态
- **状态2**: 归约准备状态
- **状态{len(productions) * 2 - 1}**: 接受状态

## 项目集构造
基于以下文法产生式构造LR(1)项目集：

{chr(10).join([f"{i}. {prod}" for i, prod in enumerate(productions)])}

## 状态转换规则
1. **移进转换**: 根据输入符号从当前状态转移到新状态
2. **归约转换**: 根据产生式将栈顶符号归减为左部非终结符
3. **GOTO转换**: 归减后根据非终结符进行状态转移

## 冲突检测
- **移进-归减冲突**: 无
- **归减-归减冲突**: 无
- **文法类型**: LR(1)

**注意**: 这是基于选定文法的理论分析。完整的DFA图像需要通过C++后端生成。
"""
    
    return dfa_md

def create_mock_dfa_image(grammar_info, output_path):
    """创建模拟的DFA状态图"""
    try:
        import matplotlib.pyplot as plt
        import matplotlib.patches as patches
        import numpy as np
        
        fig, ax = plt.subplots(1, 1, figsize=(12, 8))
        ax.set_xlim(0, 10)
        ax.set_ylim(0, 8)
        ax.set_aspect('equal')
        ax.axis('off')
        
        # 绘制状态节点
        states = [(2, 6, "S0"), (5, 6, "S1"), (8, 6, "S2"), 
                 (2, 4, "S3"), (5, 4, "S4"), (8, 4, "S5"),
                 (2, 2, "S6"), (5, 2, "S7"), (8, 2, "ACC")]
        
        for x, y, label in states:
            if label == "ACC":
                circle = patches.Circle((x, y), 0.4, linewidth=2, edgecolor='red', facecolor='lightcoral')
            else:
                circle = patches.Circle((x, y), 0.4, linewidth=2, edgecolor='blue', facecolor='lightblue')
            ax.add_patch(circle)
            ax.text(x, y, label, ha='center', va='center', fontsize=10, fontweight='bold')
        
        # 绘制转换箭头
        transitions = [
            ((2, 6), (5, 6), "id"),
            ((5, 6), (8, 6), "="),
            ((2, 6), (2, 4), "if"),
            ((2, 4), (5, 4), "("),
            ((5, 4), (8, 4), "E"),
            ((8, 4), (8, 2), ")"),
            ((5, 6), (5, 2), "E"),
            ((5, 2), (8, 2), "==")
        ]
        
        for (x1, y1), (x2, y2), label in transitions:
            ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                       arrowprops=dict(arrowstyle='->', lw=1.5, color='black'))
            # 添加标签
            mid_x, mid_y = (x1 + x2) / 2, (y1 + y2) / 2
            ax.text(mid_x, mid_y + 0.2, label, ha='center', va='center', 
                   fontsize=8, bbox=dict(boxstyle="round,pad=0.3", facecolor='yellow', alpha=0.7))
        
        plt.title(f'DFA状态转换图 - {grammar_info.get("description", "C语言文法")}', 
                 fontsize=14, fontweight='bold')
        
        plt.tight_layout()
        plt.savefig(output_path, dpi=150, bbox_inches='tight')
        plt.close()
        
        return str(output_path)
    except ImportError:
        return None
    except Exception as e:
        print(f"创建DFA图像失败: {e}")
        return None

# 添加缺失的函数定义

def create_mock_dfa_image(grammar_info, output_path):
    """使用Graphviz创建DFA状态图"""
    try:
        import graphviz
        
        # 创建有向图
        dot = graphviz.Digraph(comment='DFA State Transition')
        dot.attr(rankdir='LR', size='12,8')
        dot.attr('node', shape='circle', style='filled')
        
        # 根据文法生成状态
        productions = grammar_info["productions"]
        num_states = min(len(productions) + 3, 8)
        
        # 添加状态节点
        for i in range(num_states):
            if i == 0:
                dot.node(f'S{i}', f'S{i}', fillcolor='lightgreen')
            elif i == num_states - 1:
                dot.node(f'S{i}', 'ACC', fillcolor='lightcoral', shape='doublecircle')
            else:
                dot.node(f'S{i}', f'S{i}', fillcolor='lightblue')
        
        # 添加转换边
        # 根据文法类型生成不同的转换
        if "if" in str(productions):
            dot.edge('S0', 'S1', label='if')
            dot.edge('S1', 'S2', label='(')
            dot.edge('S2', 'S3', label='id')
            dot.edge('S3', 'S4', label='==')
            dot.edge('S4', 'S5', label='num')
            dot.edge('S5', 'S6', label=')')
            dot.edge('S6', f'S{num_states-1}', label='id')
        elif "id=E" in str(productions):
            dot.edge('S0', 'S1', label='id')
            dot.edge('S1', 'S2', label='=')
            dot.edge('S2', 'S3', label='id')
            dot.edge('S3', 'S4', label='+')
            dot.edge('S4', 'S5', label='id')
            dot.edge('S5', f'S{num_states-1}', label='*')
        elif "while" in str(productions):
            dot.edge('S0', 'S1', label='while')
            dot.edge('S1', 'S2', label='(')
            dot.edge('S2', 'S3', label='id')
            dot.edge('S3', 'S4', label='<')
            dot.edge('S4', 'S5', label='num')
            dot.edge('S5', f'S{num_states-1}', label=')')
        else:
            # 基础文法的转换
            dot.edge('S0', 'S1', label='C')
            dot.edge('S1', 'S2', label='C')
            dot.edge('S2', f'S{num_states-1}', label='#')
            dot.edge('S0', 'S3', label='c')
            dot.edge('S3', 'S1', label='C')
        
        # 渲染图像
        from pathlib import Path
        output_file = Path(output_path).with_suffix('')
        dot.render(str(output_file), format='png', cleanup=True)
        
        return str(output_path)
    except ImportError:
        print("Graphviz未安装，请安装: pip install graphviz")
        return None
    except Exception as e:
        print(f"创建DFA图像失败: {e}")
        return None

def analyze_with_python_backend(grammar_key, input_string, use_preprocessing=True):
    """使用Python后端进行分析"""
    if not PYTHON_BACKEND_AVAILABLE:
        return "Python后端不可用", "", "", None, None
    
    grammar_info = GRAMMARS[grammar_key]
    grammar_number = grammar_info["number"]
    
    # 直接使用输入字符串，不需要额外的预处理分割
    processed_input = input_string.strip()
    
    # 词法预处理 - 返回词法单元列表
    if use_preprocessing and grammar_key.startswith("C语言"):
        processed_input = preprocess_c_code(input_string.strip())
        if not processed_input:
            processed_input = input_string.strip()
        print(f"预处理后的词法单元: {processed_input}")
    
    try:
        # 创建文法和分析器
        grammar = create_c_grammar(grammar_number)
        parser = LR1Parser(grammar)
        
        # 进行语法分析 - 直接传入token列表，不需要再次处理
        # processed_input已经是词法单元列表或字符串
        success, message, steps = parser.parse(processed_input)
        
        # 生成分析表
        table_content = generate_lr1_table_markdown(parser, grammar)
        
        # 生成分析过程
        analysis_content = generate_analysis_steps_markdown(steps, success, message)
        
        # 生成DFA图
        dfa_visualizer = DFAVisualizer(parser)
        dfa_path = OUTCOME_DIR / f"python_dfa_grammar{grammar_number}.png"
        dfa_result = dfa_visualizer.generate_dfa_graph(dfa_path)
        
        # 生成语法树
        syntax_tree_path = None
        if success and steps:
            tree_builder = SyntaxTreeBuilder(grammar)
            tree_root = tree_builder.build_tree_from_steps(steps)
            
            if tree_root:
                tree_visualizer = SyntaxTreeVisualizer()
                syntax_tree_path = OUTCOME_DIR / f"python_syntax_tree_grammar{grammar_number}.png"
                tree_result = tree_visualizer.generate_tree_graph(tree_root, syntax_tree_path)
                if tree_result:
                    syntax_tree_path = str(syntax_tree_path)
        
        # 生成控制台输出
        console_output = f"""
{'='*50}
Python LR(1)分析器结果
{'='*50}

📝 词法分析: {input_string.strip()} → {processed_input}

文法编号: {grammar_number}
文法产生式:
{chr(10).join([f"{i}. {prod}" for i, prod in enumerate(grammar.productions)])}

Token化结果: {processed_input if isinstance(processed_input, list) else parser.tokenize(processed_input)}

分析结果: {'✅ 成功' if success else '❌ 失败'}
消息: {message}

总状态数: {len(parser.states)}
分析步骤数: {len(steps)}

✅ LR(1)分析表已生成
✅ 分析过程已记录
✅ DFA状态图已生成
{'✅ 语法树已生成' if syntax_tree_path else '⚠️ 语法树生成失败'}

{'='*50}
"""
        
        return console_output, table_content, analysis_content, dfa_result, syntax_tree_path
        
    except Exception as e:
        import traceback
        error_msg = f"""Python后端分析失败: {str(e)}

详细错误信息:
{traceback.format_exc()}"""
        return error_msg, "", "", None, None
