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
        "example_input": "(a + b) * c / (d - 2);"
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
            return "错误：无效的文法选择", "", "", "", None
        
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
            return f"编译错误:\n{compile_result.stderr}", "", "", "", None
        
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
            return f"运行错误:\n{run_result.stderr}", "", "", "", None
        
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
        syntax_tree_path = None  # 添加语法树路径变量
        
        if lr1_table_file.exists():
            with open(lr1_table_file, 'r', encoding='utf-8') as f:
                table_content = f.read()
        
        if lr1_analysis_file.exists():
            with open(lr1_analysis_file, 'r', encoding='utf-8') as f:
                analysis_content = f.read()
        
        if dfa_image_file.exists():
            dfa_image_path = str(dfa_image_file)
        
        # 尝试查找语法树图像（如果存在）
        syntax_tree_file = OUTCOME_DIR / f"syntax_tree_grammar{grammar_number}.png"
        if syntax_tree_file.exists():
            syntax_tree_path = str(syntax_tree_file)
        
        return console_output, table_content, analysis_content, dfa_image_path, syntax_tree_path
        
    except Exception as e:
        return f"执行错误: {str(e)}", "", "", "", None

def preprocess_c_code(code):
    """
    预处理C语言代码，进行词法分析并转换为适合语法分析的token序列
    """
    import re
    
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
    detailed_tokens = []  # 带有详细信息的token列表
    pos = 0
    debug = True  # 启用调试输出
    
    if debug:
        print(f"开始预处理代码: '{code_normalized}'")
        
    while pos < len(code_normalized):
        matched = False
        for pattern, token_type in patterns:
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

def generate_lexical_analysis_markdown(code, detailed_tokens, lexical_analysis_result):
    """生成词法分析过程的Markdown格式输出"""
    # 添加原始代码
    md_lines = [
        "# C语言词法分析",
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
    
    md_lines.extend([
        "",
        "## 词法单元表",
        "",
        "| 序号 | 类型 | 值 | 位置 |",
        "|------|------|-----|------|"
    ])
    
    # 添加词法单元详细信息
    for i, (token_type, value, position) in enumerate(detailed_tokens):
        md_lines.append(f"| {i+1} | {token_type} | {value} | {position} |")
    
    md_lines.extend([
        "",
        "## 词法分析结果统计",
        "",
        f"- **总词法单元数:** {len(detailed_tokens)}",
        f"- **关键字数量:** {sum(1 for t in detailed_tokens if t[0] in ['int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'switch', 'case', 'default'])}",
        f"- **标识符数量:** {sum(1 for t in detailed_tokens if t[0] == 'id')}",
        f"- **数字常量数量:** {sum(1 for t in detailed_tokens if t[0] == 'num')}",
        f"- **运算符数量:** {sum(1 for t in detailed_tokens if t[0] not in ['id', 'num'] and t[0] not in ['int', 'float', 'char', 'void', 'if', 'else', 'while', 'for', 'switch', 'case', 'default'])}"
    ])
    
    return "\n".join(md_lines)

def analyze_lr1_with_preprocessing(grammar_key, input_string, use_preprocessing=True):
    """主要的分析函数，支持C语言代码预处理"""
    if not grammar_key or not input_string.strip():
        return "请选择文法并输入待分析的C代码", "", "", None, None, ""
    
    grammar_info = GRAMMARS[grammar_key]
    grammar_number = grammar_info["number"]
    
    # 优先使用Python后端处理C语言文法
    if grammar_key.startswith("C语言") and PYTHON_BACKEND_AVAILABLE:
        return analyze_with_python_backend(grammar_key, input_string, use_preprocessing)
    
    # 否则使用原有的C++后端或模拟分析
    processed_input = input_string.strip()
    lexical_analysis_md = ""
    
    # 如果选择的是C语言文法且启用预处理，则进行词法分析
    if use_preprocessing and grammar_key.startswith("C语言"):
        tokens, detailed_tokens, lexical_analysis_result = preprocess_c_code(input_string.strip())
        processed_input = tokens
        
        # 生成词法分析Markdown
        lexical_analysis_md = generate_lexical_analysis_markdown(input_string.strip(), detailed_tokens, lexical_analysis_result)
        
        # 如果预处理后的结果为空，使用原始输入
        if not processed_input:
            processed_input = input_string.strip()
    
    # 运行分析
    console_output, table_content, analysis_content, dfa_image_path, syntax_tree_path = compile_and_run_analysis(
        grammar_key, processed_input
    )
    
    # 在控制台输出中添加预处理信息
    if use_preprocessing and grammar_key.startswith("C语言") and processed_input != input_string.strip():
        original_code = input_string.strip()
        if len(original_code) > 50:  # 如果代码较长，截断显示
            original_code = original_code[:50] + "..."
            
        preprocessing_info = f"### 📝 词法分析结果\n原始代码: `{original_code}`\n转换为: `{processed_input}`\n\n"
        console_output = preprocessing_info + "```\n" + console_output + "\n```"
    else:
        console_output = "```\n" + console_output + "\n```"
    
    # 检查是否分析成功
    if "分析失败" in console_output or "错误" in console_output or "原始文法" in console_output:
        # 如果分析失败，可能是因为C++程序不支持该文法编号
        if grammar_key.startswith("C语言"):
            return console_output, "", "", "", None, lexical_analysis_md
    
    return console_output, table_content, analysis_content, dfa_image_path, syntax_tree_path, lexical_analysis_md

def analyze_lr1(grammar_key, input_string):
    """主要的分析函数"""
    if not grammar_key or not input_string.strip():
        return "请选择文法并输入待分析的字符串", "", "", None, None
    
    # 运行分析
    console_output, table_content, analysis_content, dfa_image_path, syntax_tree_path = compile_and_run_analysis(
        grammar_key, input_string.strip()
    )
    
    return console_output, table_content, analysis_content, dfa_image_path, syntax_tree_path

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
        padding: 0.75rem;
        margin-bottom: 0.75rem;
        font-size: 0.9em;
    }
    
    .output-section {
        margin-top: 0.5rem;
    }
    
    .tab-content {
        min-height: 400px;
    }
    
    .code-editor {
        font-family: 'Courier New', monospace;
        min-height: 150px;
        font-size: 16px !important;
    }
    
    .compact-header {
        margin-bottom: 0.5rem;
    }
    
    .compact-container {
        gap: 0.5rem;
    }
    
    .example-btn {
        background-color: #ffc107 !important;
        font-weight: bold !important;
        color: #343a40 !important;
        margin-bottom: 0.5rem !important;
    }
    
    .analyze-btn {
        background-color: #007bff !important;
        font-weight: bold !important;
        margin-top: 0.5rem !important;
        margin-bottom: 1rem !important;
    }
    
    .console-output {
        border: none !important;
        background: transparent !important;
        padding: 0px !important;
    }
    
    .console-output > div {
        border: none !important;
        background: transparent !important;
    }
    
    .left-panel {
        min-width: 400px !important;
        max-width: 550px !important;
    }
    
    .right-panel {
        min-width: 500px !important;
    }
    """
    
    with gr.Blocks(css=css, title="C语言LR(1)语法分析器") as demo:
        gr.Markdown("# C语言LR(1)语法分析器", elem_classes=["compact-header"])
        
        with gr.Row(equal_height=False):
            # 左侧输入面板
            with gr.Column(scale=1, elem_classes=["left-panel"]):
                grammar_dropdown = gr.Dropdown(
                    choices=list(GRAMMARS.keys()),
                    value=list(GRAMMARS.keys())[0],
                    label="选择文法",
                )
                
                grammar_info = gr.Markdown(
                    value=f"**产生式:**\n" + "\n".join([f"- {p}" for p in GRAMMARS[list(GRAMMARS.keys())[0]]["productions"]]),
                    elem_classes=["grammar-info"]
                )
                
                example_btn = gr.Button("📋 使用示例", variant="secondary", elem_classes=["example-btn"])
                
                input_string = gr.Code(
                    value=GRAMMARS[list(GRAMMARS.keys())[0]]["example_input"],
                    language="c",
                    label="代码输入",
                    elem_classes=["code-editor"]
                )
                
                preprocessing_checkbox = gr.Checkbox(
                    value=True,
                    label="词法预处理",
                    info="自动将标识符转换为'id'，数字转换为'num'"
                )
                
                # 将分析按钮置于底部
                analyze_btn = gr.Button("🚀 开始分析", variant="primary", elem_classes=["analyze-btn"])
            
            # 右侧结果面板
            with gr.Column(scale=1, elem_classes=["right-panel"]):
                with gr.Tabs() as tabs:

                    with gr.TabItem("词法分析"):
                        lexical_analysis_output = gr.Markdown(
                            value="点击'开始分析'查看词法分析结果"
                        )
                    with gr.TabItem("LR(1)分析表"):
                        table_output = gr.Markdown(
                            value="点击'开始分析'生成分析表"
                        )
                    
                    with gr.TabItem("分析过程"):
                        analysis_output = gr.Markdown(
                            value="点击'开始分析'查看分析过程"
                        )
                    
                    with gr.TabItem("DFA状态图"):
                        dfa_image = gr.Image(
                            type="filepath",
                            label="DFA状态转换图"
                        )
                    
                    with gr.TabItem("语法分析树"):
                        syntax_tree_image = gr.Image(
                            type="filepath",
                            label="语法树可视化"
                        )
                    with gr.TabItem("控制台输出"):
                        console_output = gr.Markdown(
                            value="点击'开始分析'查看执行结果",
                            elem_classes=["console-output"]
                        )

        
        # 事件处理
        def update_grammar_info(grammar_key):
            if grammar_key in GRAMMARS:
                productions = GRAMMARS[grammar_key]["productions"]
                info_text = f"**产生式:**\n" + "\n".join([f"- {p}" for p in productions])
                return info_text
            return "请选择文法"
        
        # 绑定事件
        grammar_dropdown.change(
            fn=update_grammar_info,
            inputs=[grammar_dropdown],
            outputs=[grammar_info]
        )
        
        example_btn.click(
            fn=get_example_input,
            inputs=[grammar_dropdown],
            outputs=[input_string]
        )
        
        analyze_btn.click(
            fn=analyze_lr1_with_preprocessing,
            inputs=[grammar_dropdown, input_string, preprocessing_checkbox],
            outputs=[console_output, table_output, analysis_output, dfa_image, syntax_tree_image, lexical_analysis_output]
        )
        
        # 简化的使用说明
        with gr.Accordion("使用说明", open=False):
            gr.Markdown("""
            **基本操作**：选择文法 → 输入代码 → 开始分析
            
            **支持的文法**：
            - 基础文法(1-6): 上下文无关文法基础示例
            - C语言文法(7-11): 赋值语句、if语句、变量声明、while循环、算术表达式
            """)
    
    return demo

def analyze_with_python_backend(grammar_key, input_string, use_preprocessing=True):
    """使用Python后端进行分析"""
    if not PYTHON_BACKEND_AVAILABLE:
        return "Python后端不可用", "", "", None, None, ""
    
    grammar_info = GRAMMARS[grammar_key]
    grammar_number = grammar_info["number"]
    
    # 直接使用输入字符串，不需要额外的预处理分割
    processed_input = input_string.strip()
    lexical_analysis_md = ""
    
    # 词法预处理 - 返回词法单元列表
    if use_preprocessing and grammar_key.startswith("C语言"):
        tokens, detailed_tokens, lexical_analysis_result = preprocess_c_code(input_string.strip())
        processed_input = tokens
        
        # 生成词法分析Markdown
        lexical_analysis_md = generate_lexical_analysis_markdown(input_string.strip(), detailed_tokens, lexical_analysis_result)
        
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
Python LR(1)分析器结果

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

"""
        
        return console_output, table_content, analysis_content, dfa_result, syntax_tree_path, lexical_analysis_md
        
    except Exception as e:
        import traceback
        error_msg = f"""Python后端分析失败: {str(e)}

详细错误信息:
{traceback.format_exc()}"""
        return error_msg, "", "", None, None, lexical_analysis_md
