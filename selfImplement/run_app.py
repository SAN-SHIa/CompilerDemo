#!/usr/bin/env python3
"""
简化的启动脚本
"""

import sys
import os
from pathlib import Path

# 添加当前目录到Python路径
sys.path.insert(0, str(Path(__file__).parent))

def main():
    print("🚀 正在启动C语言LR(1)语法分析器...")
    try:
        import gradio as gr
        version = gr.__version__
        print(f"✅ Gradio已安装 (版本: {version})")
    except ImportError:
        print("❌ Gradio未安装，正在尝试安装...")
        os.system("pip install gradio")
        import gradio as gr
        print(f"✅ Gradio已安装 (版本: {gr.__version__})")

    from gradio_app import create_interface, WORK_DIR, OUTCOME_DIR
    print(f"📁 工作目录: {WORK_DIR}")
    OUTCOME_DIR.mkdir(exist_ok=True)
    print(f"📁 输出目录: {OUTCOME_DIR}")
    demo = create_interface()
    print("✅ 界面创建成功!")
    print("🌐 启动Web服务器...")
    print("📱 访问地址: http://localhost:7860")
    print("💻 现在支持完整的C语言代码格式输入")
    demo.launch(
        server_name="0.0.0.0",
        server_port=7860,
        share=False,
        inbrowser=True
    )

if __name__ == "__main__":
    main()