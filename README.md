# C语言编译器 Demo

基于LR(1)算法实现的C语言编译器Demo，带有可视化界面。本项目主要演示编译原理中的语法分析部分实现。

## 功能特点

- LR(1)语法分析算法实现（Python和C++双引擎）
- DFA状态图可视化
- 语法树生成与可视化
- LR(1)分析表展示
- 分析步骤详细追踪
- Web界面便于使用和演示

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
cd selfImplement
python run_app.py
```

启动后，在浏览器中访问 http://localhost:7860 使用分析器。

## 项目结构

- `/selfImplement`: 编译器实现代码
  - `run_app.py`: 启动脚本
  - `gradio_app.py`: Gradio界面实现
  - `python_lr1_parser.py`: Python实现的LR(1)分析器
  - `lr1_visualizer.py`: DFA和语法树可视化工具
  - `main.cpp` & `functions.cpp`: C++实现的LR(1)分析器
- `/outcome`: 输出图像和结果目录

## 项目进度

- [ ] Flex&Bison框架实现
- [ ] 自主实现
  - [x] 词法分析
  - [x] 语法分析(LR1)
  - [ ] 语义分析
  - [ ] 中间代码生成
  - [ ] 目标代码生成

## 使用说明

1. 在Web界面中输入C语言代码片段
2. 选择分析引擎（Python或C++）
3. 点击"分析"按钮生成语法分析结果
4. 查看DFA图、语法树及分析步骤

## 开发者

- 编译原理课程项目