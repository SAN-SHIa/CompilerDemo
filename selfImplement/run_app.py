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
    """主函数"""
    try:
        print("🚀 正在启动C语言LR(1)语法分析器...")
        
        # 检查Gradio
        try:
            import gradio as gr
            print("✅ Gradio已安装")
        except ImportError:
            print("❌ Gradio未安装，正在尝试安装...")
            os.system("pip install gradio")
            import gradio as gr
        
        # 导入主程序
        from gradio_app import create_interface, WORK_DIR, OUTCOME_DIR
        
        print(f"📁 工作目录: {WORK_DIR}")
        
        # 确保输出目录存在
        OUTCOME_DIR.mkdir(exist_ok=True)
        print(f"📁 输出目录: {OUTCOME_DIR}")
        
        # 创建界面
        demo = create_interface()
        print("✅ 界面创建成功!")
        
        # 启动
        print("🌐 启动Web服务器...")
        print("📱 访问地址: http://localhost:7860")
        
        demo.launch(
            server_name="0.0.0.0",
            server_port=7860,
            share=False,
            inbrowser=True
        )
        
    except Exception as e:
        print(f"❌ 启动失败: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()