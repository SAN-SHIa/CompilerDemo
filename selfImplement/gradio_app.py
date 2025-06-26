#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LR(1)编译原理分析器 Gradio 可视化界面
"""

import gradio as gr
from pathlib import Path

# 导入自定义模块
from grammar_definitions import GRAMMARS, get_example_input
from lexical_analyzer import LexicalAnalyzer
from syntax_analyzer import SyntaxAnalyzer, PYTHON_BACKEND_AVAILABLE

# 工作目录设置
WORK_DIR = Path(__file__).parent.resolve()
OUTCOME_DIR = WORK_DIR / "outcome"
OUTCOME_DIR.mkdir(exist_ok=True)

def analyze_lr1_with_preprocessing(grammar_key, input_string, use_preprocessing=True):
    """主要的分析函数，支持C语言代码预处理"""
    if not grammar_key or not input_string.strip():
        return "请选择文法并输入待分析的C代码", "", "", None, None, ""
    
    grammar_info = GRAMMARS[grammar_key]
    
    # 初始化分析器
    lexer = LexicalAnalyzer()
    syntax_analyzer = SyntaxAnalyzer(work_dir=WORK_DIR)
    
    processed_input = input_string.strip()
    lexical_analysis_md = ""
    
    # 如果选择的是C语言文法且启用预处理，则进行词法分析
    if use_preprocessing and grammar_key.startswith("C语言"):
        tokens, detailed_tokens, lexical_analysis_result = lexer.preprocess_code(input_string.strip())
        processed_input = tokens
        
        # 生成词法分析Markdown
        lexical_analysis_md = lexer.generate_markdown(input_string.strip(), detailed_tokens, lexical_analysis_result)
        
        # 如果预处理后的结果为空，使用原始输入
        if not processed_input:
            processed_input = input_string.strip()
    
    # 运行分析
    console_output, table_content, analysis_content, dfa_image_path, syntax_tree_path = syntax_analyzer.analyze(
        grammar_key, processed_input, use_python_backend=True
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

# 启动应用
if __name__ == "__main__":
    demo = create_interface()
    demo.launch()
