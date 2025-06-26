# C语言编译器实现

基于Flex&Bison工具链的完整C语言编译器实现，支持从源代码到目标代码的完整编译流程。

## 🛠️ 环境要求

- **MSYS2/MinGW-w64** - Windows下的类Unix环境
- **Flex** - 词法分析器生成工具
- **Bison** - 语法分析器生成工具
- **GraphViz** - 图形可视化工具
- **GCC** - GNU编译器集合

确保所有工具都已添加到系统PATH环境变量中。

## 🚀 编译与运行

### 编译编译器
```bash
# 使用Makefile编译
mingw32-make -f Makefile.win
```

### 运行编译器
```bash
# 编译C源文件
.\compiler.exe test.c

# 编译器将生成以下文件：
# - ast.dot        抽象语法树DOT文件
# - ast.png        抽象语法树图像
# - output.s       伪汇编代码
# - output.c       生成的C代码
# - output_x64.s   x86-64汇编代码
# - output.exe     可执行文件
```

## 📁 文件结构

```
compiler/
├── README.md              # 本说明文档
├── 说明.md               # 详细技术说明
├── Makefile.win          # Windows构建脚本
├── test.c                # 测试用例 
│
├── 词法分析 (Lexical Analysis)
│   ├── lexer.l           # Flex词法分析器定义
│   └── lex.yy.c          # 生成的词法分析器代码
│
├── 语法分析 (Syntax Analysis)  
│   ├── parser.y          # Bison语法分析器定义
│   ├── parser.tab.c      # 生成的语法分析器代码
│   └── parser.tab.h      # 语法分析器头文件
│
├── 抽象语法树 (Abstract Syntax Tree)
│   ├── ast.h             # AST结构定义
│   ├── ast.c             # AST实现
│   ├── ast.dot           # 生成的DOT文件
│   └── ast.png           # 生成的语法树图像
│
├── 语义分析 (Semantic Analysis)
│   ├── semantic.h        # 语义分析接口定义
│   ├── semantic.c        # 语义分析实现
│   ├── symbol_table.h    # 符号表结构定义
│   └── symbol_table.c    # 符号表实现
│
├── 中间代码生成 (Intermediate Code)
│   ├── ir.h              # 中间代码表示定义
│   └── ir.c              # 中间代码生成实现
│
├── 代码优化 (Code Optimization)
│   ├── optimize.h        # 代码优化器接口
│   └── optimize.c        # 代码优化器实现
│
├── 目标代码生成 (Code Generation)
│   ├── codegen.h         # 目标代码生成接口
│   └── codegen.c         # 目标代码生成实现
│
├── 解释器 (Interpreter)
│   ├── interpreter.h     # 解释器接口定义
│   └── interpreter.c     # 解释器实现
│
├── 输出文件 (Generated Files)
    ├── output.c          # 生成的C代码
    ├── output.s          # 伪汇编代码
    ├── output_fixed.c    # 修复后的C代码
    └── compiler.exe      # 编译器可执行文件

```

## 🔧 编译器架构与模块分析

### 1. 词法分析模块 (lexer.l + lex.yy.c)

**技术方法：** 基于Flex的正则表达式驱动词法分析
**实现特点：**
- **有限状态自动机**：Flex生成的DFA实现高效Token识别
- **位置跟踪**：自动跟踪行号(`yylineno`)和列号(`yycolumn`)
- **内存管理**：使用`_strdup()`确保字符串独立性

**支持的语言元素：**
```c
// 关键字
"int", "float", "return", "if", "else", "while", "printf"

// 运算符与操作符
"+", "-", "*", "/", "=", "==", "!=", "<", ">", "<=", ">="

// 语言构造
标识符: {letter}({letter}|{digit}|_)*
整数: {digit}+
字符串: \"[^\"]*\"
```

**技术特色：**
- 错误恢复：非法字符自动跳过并报告
- 语义值传递：通过`yylval`联合体传递Token值
- 编码兼容：支持GB2312编码的中文注释

### 2. 语法分析模块 (parser.y + parser.tab.c)

**技术方法：** 基于Bison的LALR(1)算法
**核心算法：**
- **LALR(1)分析表**：构建状态转移表进行移进-规约分析
- **优先级处理**：定义运算符优先级和结合性
- **冲突解决**：解决shift/reduce和reduce/reduce冲突

**语法规则设计：**
```yacc
// 运算符优先级定义
%left EQ NE                    // 比较运算符
%left '<' '>' LE GE           // 关系运算符  
%left '+' '-'                 // 加减运算符
%left '*' '/'                 // 乘除运算符

// 核心语法产生式
program → func_def
func_def → INT IDENTIFIER '(' ')' '{' stmt_list '}'
stmt_list → stmt_list stmt | stmt  // 左递归避免栈溢出
```

**特殊处理：**
- **Dangling-else问题**：使用`%nonassoc`解决else悬挂
- **语法制导翻译**：在产生式中直接构建AST
- **集成流水线**：自动调用后续编译阶段

### 3. 抽象语法树模块 (ast.h + ast.c)

**技术方法：** 基于标记联合(Tagged Union)的多态节点设计
**数据结构特点：**
```c
typedef struct ASTNode {
    NodeType type;              // 节点类型标签
    int line_number, column;    // 位置信息
    struct ASTNode *left, *right; // 子节点指针
    union {                     // 多态数据存储
        struct { char *name; char *var_type; } decl;
        struct { BinOpType op; } binop;
        struct { int value; } integer;
        // ... 其他节点类型
    };
} ASTNode;
```

**核心算法：**
- **递归构建**：自底向上构建语法树
- **内存管理**：递归释放避免内存泄漏
- **可视化**：生成GraphViz DOT格式进行图形化展示

**技术亮点：**
- 支持16种不同节点类型的统一表示
- 位置信息保留用于精确错误报告
- 深度优先遍历支持多种访问模式

### 4. 符号表管理模块 (symbol_table.h + symbol_table.c)

**技术方法：** 链表实现的分层作用域管理
**数据结构：**
```c
typedef struct SymbolEntry {
    char *name;                 // 符号名称
    SymbolKind kind;           // 符号类型(VAR/FUNC)
    DataType type;             // 数据类型(INT/FLOAT)
    int scope_level;           // 作用域层级
    struct SymbolEntry *next;  // 链表指针
} SymbolEntry;
```

**核心算法：**
- **作用域栈管理**：`enter_scope()`和`leave_scope()`
- **符号查找**：作用域链逆向查找算法
- **冲突检测**：同一作用域内符号重定义检查

**特性优势：**
- O(n)查找复杂度，适合小型程序
- 支持无限嵌套作用域
- 自动内存清理和作用域退出处理

### 5. 语义分析模块 (semantic.h + semantic.c)

**技术方法：** 基于AST的递归下降语义检查
**核心检查算法：**
```c
// 主要检查类型
SEM_UNDECLARED_VAR        // 未声明变量使用
SEM_REDECLARED_VAR        // 变量重复声明  
SEM_TYPE_MISMATCH         // 类型不匹配
SEM_DIVISION_BY_ZERO      // 除零检查
SEM_INVALID_RETURN_TYPE   // 返回类型检查
```

**类型系统特点：**
- **隐式类型转换**：支持int ↔ float自动转换
- **类型推导**：表达式类型自动推导
- **编译期检查**：常量表达式求值和除零检测

**错误处理机制：**
- 精确的错误定位(行号+列号)
- 分类的错误类型和详细消息
- 错误计数和批量报告

### 6. 中间代码生成模块 (ir.h + ir.c)

**技术方法：** 三地址码(Three-Address Code)线性中间表示
**指令系统设计：**
```c
typedef enum {
    IR_ASSIGN,         // t1 = t2        赋值
    IR_BINOP,          // t1 = t2 op t3  二元运算
    IR_LOAD,           // t1 = var       变量加载
    IR_STORE,          // var = t1       变量存储
    IR_LOAD_CONST,     // t1 = const     常量加载
    IR_LABEL,          // L1:            标签
    IR_GOTO,           // goto L1        无条件跳转
    IR_IF_GOTO,        // if t1 goto L1  条件跳转
    IR_RETURN,         // return t1      函数返回
} IROpcode;
```

**生成算法：**
- **递归代码生成**：AST后序遍历生成IR序列
- **临时变量管理**：自动分配和编号临时变量
- **类型转换插入**：自动插入类型转换指令

**技术特色：**
- 线性化表示便于优化算法处理
- 支持复杂表达式的分解
- 控制流图(CFG)友好的跳转指令

### 7. 代码优化模块 (optimize.h + optimize.c)

**技术方法：** 多遍数据流分析优化算法
**优化等级系统：**
```c
// O0: 无优化
// O1: 基本优化 - 常量折叠、传播、代数简化
// O2: 高级优化 - 复制传播、死代码消除  
// O3: 最高优化 - 公共子表达式消除
```

**核心优化算法：**

**常量折叠(Constant Folding)：**
- 编译期计算常量表达式: `3 + 5` → `8`
- 支持整数和浮点数运算
- 处理溢出和特殊值

**常量传播(Constant Propagation)：**
- 传播变量的常量值: `x = 5; y = x + 1` → `y = 6`
- 基于数据流方程的不动点算法
- 迭代收敛保证正确性

**死代码消除(Dead Code Elimination)：**
- 删除不影响程序输出的代码
- 基于活跃变量分析
- 递归删除无用定义

**代数简化(Algebraic Simplification)：**
```c
x + 0 → x          // 加法单位元
x * 1 → x          // 乘法单位元  
x * 0 → 0          // 乘法零元
x - x → 0          // 自减为零
```

**技术特点：**
- 多遍迭代直到收敛
- 优化统计信息输出
- 可配置的优化启用/禁用

### 8. 目标代码生成模块 (codegen.h + codegen.c)

**技术方法：** 基于模板的多目标代码生成
**支持的目标架构：**
- **TARGET_PSEUDO**: 教学用伪汇编
- **TARGET_C_CODE**: 标准C代码生成
- **TARGET_X86_64**: x86-64汇编代码

**寄存器分配算法：**
```c
// 寄存器描述
typedef struct {
    RegisterType type;     // 寄存器类型(通用/浮点)
    char *name;           // 寄存器名称
    bool is_available;    // 可用状态
    int temp_id;          // 当前存储的临时变量
    DataType data_type;   // 数据类型
} Register;
```

**分配策略：**
- **线性扫描算法**：简单高效的寄存器分配
- **生命周期分析**：基于活跃变量的生命周期
- **溢出处理**：LRU策略的栈溢出

**代码生成模板：**
- 指令选择：IR指令到目标指令的映射
- 地址计算：变量地址和栈偏移计算
- 函数调用：标准调用约定实现

### 9. 解释器模块 (interpreter.h + interpreter.c)

**技术方法：** 基于虚拟机的IR解释执行
**虚拟机架构：**
```c
typedef struct {
    VarEntry *variables;       // 变量环境
    RuntimeValue *param_stack; // 参数栈
    int pc;                   // 程序计数器
    bool running;             // 运行状态
    RuntimeValue return_val;  // 返回值
} Interpreter;
```

**执行引擎：**
- **指令解释循环**：fetch-decode-execute循环
- **动态类型系统**：运行时类型检查和转换
- **内存管理**：自动垃圾回收机制

**内置函数支持：**
- `printf`: 格式化输出支持
- 类型转换函数
- 数学运算函数

**技术特色：**
- 与编译器共享相同的IR表示
- 便于调试和教学演示
- 运行时错误检测和报告

## 📊 算法复杂度分析

| 模块 | 时间复杂度 | 空间复杂度 | 特点 |
|------|------------|------------|------|
| 词法分析 | O(n) | O(1) | 线性扫描 |
| 语法分析 | O(n) | O(n) | LALR(1)表驱动 |
| 语义分析 | O(n) | O(s) | 单遍扫描 |
| IR生成 | O(n) | O(n) | 递归遍历 |
| 代码优化 | O(n²) | O(n) | 多遍迭代 |
| 寄存器分配 | O(n²) | O(r) | 线性扫描 |
| 代码生成 | O(n) | O(n) | 模板匹配 |

(n=程序大小, s=符号数量, r=寄存器数量)

## 📋 支持的C语言特性

### 数据类型
- `int` - 32位整数
- `float` - 32位浮点数

### 变量操作
- 变量声明：`int x;`、`float y;`
- 变量初始化：`int x = 10;`
- 变量赋值：`x = y + 5;`

### 表达式
- **算术运算**：`+`、`-`、`*`、`/`
- **比较运算**：`==`、`!=`、`<`、`>`、`<=`、`>=`
- **优先级**：正确的运算符优先级和结合性

### 控制结构
- **条件语句**：
  ```c
  if (condition) {
      // statements
  } else {
      // statements
  }
  ```
- **循环语句**：
  ```c
  while (condition) {
      // statements
  }
  ```

### 函数
- **函数定义**：基本函数结构支持
- **返回语句**：`return expression;`

## 🧪 测试用例

### test.c - 综合测试
演示编译器的主要功能，包括：
- 变量声明和初始化
- 算术运算和类型转换
- 条件分支和循环
- 函数定义和返回

### 语义分析测试
位于`semantic_test/`目录，专门测试：
- 类型检查功能
- 作用域管理
- 错误检测机制

## 📊 输出示例

### 中间代码输出
```
func_begin main
x = 10
y = 5.50
t1 = x
t2 = y
t3 = t1 * t2
result = t3
t4 = result
t5 = (float) t4
t6 = t5 > 40.00
if !t6 goto L1
t7 = result
return t7
goto L2
L1:
    // 优化后的代码
L2:
func_end
```

### 优化统计
编译器会输出详细的优化统计信息，包括：
- 常量折叠次数
- 死代码消除数量
- 优化前后指令对比

## 🐛 错误处理

编译器提供详细的错误报告：
- **词法错误**：非法字符、未识别token
- **语法错误**：语法结构不正确
- **语义错误**：类型不匹配、未声明变量等
- **警告信息**：潜在问题提示

## 📚 技术文档

- `说明.md` - 详细的技术实现说明
- `semantic_test/README_SEMANTIC.md` - 语义分析专项说明
- `C_CODEGEN_FIX_SUMMARY.md` - 代码生成修复总结
- `INTERPRETER_README.md` - 解释器使用说明

## 🔄 编译流程

1. **词法分析** → Token流
2. **语法分析** → 抽象语法树
3. **语义分析** → 类型检查和错误检测
4. **中间代码生成** → 三地址码
5. **代码优化** → 优化后的中间代码
6. **目标代码生成** → 汇编/C代码
7. **可视化输出** → AST图像和分析报告

## 📝 使用注意事项

1. **编码格式**：源文件使用GB2312编码
2. **路径设置**：确保所有工具在PATH中
3. **依赖检查**：编译前检查所有依赖工具
4. **输出清理**：每次编译会覆盖之前的输出文件

## 🎓 学习价值

本编译器实现适合编译原理课程学习，涵盖：
- **理论到实践**：完整的编译器构造过程
- **工具使用**：主流编译器构造工具的应用
- **算法实现**：经典编译算法的具体实现
- **优化技术**：实用的代码优化方法
- **调试技巧**：编译器调试和测试方法

通过这个项目，可以深入理解编译器的工作原理和实现细节。
