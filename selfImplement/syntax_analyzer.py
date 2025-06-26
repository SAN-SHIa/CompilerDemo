#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LR(1)编译原理分析器 - 语法分析模块
"""

import os
import tempfile
import subprocess
from pathlib import Path
from grammar_definitions import GRAMMARS

# 尝试导入Python LR(1)分析器
try:
    from python_lr1_parser import create_c_grammar, LR1Parser
    from lr1_visualizer import (DFAVisualizer, SyntaxTreeBuilder, SyntaxTreeVisualizer,
                              generate_lr1_table_markdown, generate_analysis_steps_markdown)
    PYTHON_BACKEND_AVAILABLE = True
except ImportError as e:
    print(f"Python后端不可用: {e}")
    PYTHON_BACKEND_AVAILABLE = False

class SyntaxAnalyzer:
    """LR(1)语法分析器"""
    
    def __init__(self, work_dir=None):
        self.work_dir = Path(work_dir) if work_dir else Path(__file__).parent.resolve()
        self.outcome_dir = self.work_dir / "outcome"
        self.outcome_dir.mkdir(exist_ok=True)
        
        # C++可执行文件路径
        self.cpp_executable = self.work_dir / "exe" / "main"
    
    def create_cpp_file_with_grammar(self, grammar_key, input_string):
        """根据选择的文法创建临时的C++文件"""
        if grammar_key not in GRAMMARS:
            return None
            
        grammar_info = GRAMMARS[grammar_key]
        grammar_number = grammar_info["number"]
        
        # 读取原始main.cpp文件
        main_cpp_path = self.work_dir / "main.cpp"
        with open(main_cpp_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 读取functions.cpp文件
        functions_cpp_path = self.work_dir / "functions.cpp"
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
            from grammar_definitions import add_c_grammar_support
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
    
    def compile_and_run_analysis(self, grammar_key, input_string):
        """编译并运行LR(1)分析"""
        try:
            # 确保输出目录存在
            self.outcome_dir.mkdir(exist_ok=True)
            
            # 创建带有指定文法的临时C++文件
            temp_cpp_file = self.create_cpp_file_with_grammar(grammar_key, input_string)
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
                cwd=self.work_dir
            )
            
            if compile_result.returncode != 0:
                os.unlink(temp_cpp_file)
                return f"编译错误:\n{compile_result.stderr}", "", "", "", None
            
            # 运行分析程序
            run_result = subprocess.run(
                [temp_executable],
                capture_output=True,
                text=True,
                cwd=self.work_dir
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
            lr1_table_file = self.outcome_dir / f"lr1_table_grammar{grammar_number}.md"
            lr1_analysis_file = self.outcome_dir / f"lr1_analysis_grammar{grammar_number}.md"
            dfa_image_file = self.outcome_dir / f"dfa_grammar{grammar_number}.png"
            
            # 如果动态文件不存在，尝试使用默认的grammar1文件
            if not lr1_table_file.exists():
                lr1_table_file = self.outcome_dir / "lr1_table_grammar1.md"
            if not lr1_analysis_file.exists():
                lr1_analysis_file = self.outcome_dir / "lr1_analysis_grammar1.md"
            if not dfa_image_file.exists():
                dfa_image_file = self.outcome_dir / "dfa_grammar1.png"
            
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
            syntax_tree_file = self.outcome_dir / f"syntax_tree_grammar{grammar_number}.png"
            if syntax_tree_file.exists():
                syntax_tree_path = str(syntax_tree_file)
            
            return console_output, table_content, analysis_content, dfa_image_path, syntax_tree_path
            
        except Exception as e:
            return f"执行错误: {str(e)}", "", "", "", None
    
    def analyze_with_python_backend(self, grammar_key, input_string):
        """使用Python后端进行分析"""
        if not PYTHON_BACKEND_AVAILABLE:
            return "Python后端不可用", "", "", None, None
        
        grammar_info = GRAMMARS[grammar_key]
        grammar_number = grammar_info["number"]
        
        # 直接使用输入字符串，不需要额外的预处理分割
        processed_input = input_string if isinstance(input_string, list) else input_string.strip()
        
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
            dfa_path = self.outcome_dir / f"python_dfa_grammar{grammar_number}.png"
            dfa_result = dfa_visualizer.generate_dfa_graph(dfa_path)
            
            # 生成语法树
            syntax_tree_path = None
            if success and steps:
                tree_builder = SyntaxTreeBuilder(grammar)
                tree_root = tree_builder.build_tree_from_steps(steps)
                
                if tree_root:
                    tree_visualizer = SyntaxTreeVisualizer()
                    syntax_tree_path = self.outcome_dir / f"python_syntax_tree_grammar{grammar_number}.png"
                    tree_result = tree_visualizer.generate_tree_graph(tree_root, syntax_tree_path)
                    if tree_result:
                        syntax_tree_path = str(syntax_tree_path)
            
            # 生成控制台输出
            console_output = f"""
Python LR(1)分析器结果

词法分析: {input_string if isinstance(input_string, str) else '(预处理的词法单元)'} → {processed_input}

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
            
            return console_output, table_content, analysis_content, dfa_result, syntax_tree_path
            
        except Exception as e:
            import traceback
            error_msg = f"""Python后端分析失败: {str(e)}

详细错误信息:
{traceback.format_exc()}"""
            return error_msg, "", "", None, None
    
    def analyze(self, grammar_key, input_string, use_python_backend=True):
        """进行语法分析"""
        if use_python_backend and grammar_key.startswith("C语言") and PYTHON_BACKEND_AVAILABLE:
            return self.analyze_with_python_backend(grammar_key, input_string)
        else:
            return self.compile_and_run_analysis(grammar_key, input_string)
