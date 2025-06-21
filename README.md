# C语言LR(1)语法分析器

基于LR(1)算法实现的C语言语法分析器，带有可视化界面。

## 功能特点

- LR(1)语法分析算法实现（C++和Python双引擎）
- DFA状态图可视化
- 语法树生成与可视化
- LR(1)分析表展示
- 分析步骤详细追踪
- 基于Gradio的友好Web界面

## 快速开始

### 环境要求

- Python 3.6+
- C++ 编译器（可选，用于C++后端）
- Graphviz（用于图形可视化）

### 安装依赖

```bash
pip install gradio graphviz
```

### 运行程序

```bash
python run_app.py
```

启动后，在浏览器中访问 http://localhost:7860 使用分析器。

## 项目结构

- `run_app.py`: 启动脚本
- `gradio_app.py`: Gradio界面实现
- `python_lr1_parser.py`: Python实现的LR(1)分析器
- `lr1_visualizer.py`: DFA和语法树可视化工具
- `main.cpp` & `functions.cpp`: C++实现的LR(1)分析器
- `outcome/`: 输出图像和结果目录
