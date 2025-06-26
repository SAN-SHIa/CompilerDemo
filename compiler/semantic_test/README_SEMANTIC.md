# 语义分析器使用说明

## 编译方法

### 方法1：使用现有的Makefile
```bash
# 使用Windows Makefile编译完整编译器
mingw32-make -f Makefile.win
```

### 方法2：手动编译语义分析测试
```bash
# 编译各个模块
gcc -c ast.c -o ast.o
gcc -c symbol_table.c -o symbol_table.o  
gcc -c semantic.c -o semantic.o

# 编译测试程序
gcc -o semantic_test.exe semantic_test.c ast.o symbol_table.o semantic.o

# 运行测试
./semantic_test.exe
or
./semantic_test_en.exe
```

## 语义分析器功能

### 主要检查项目：
1. **变量声明检查**
   - 检测重复声明
   - 检测未声明变量的使用

2. **类型检查**
   - 赋值语句的类型兼容性
   - 表达式的类型推导
   - 函数返回值类型检查

3. **运算检查**
   - 二元运算的类型兼容性
   - 除零错误检测
   - 比较运算的类型检查

4. **作用域管理**
   - 局部作用域变量
   - 函数作用域管理

### 支持的数据类型：
- `int`: 整型
- `float`: 浮点型
- `TYPE_UNKNOWN`: 错误类型

### 支持的运算符：
- 算术运算：`+`, `-`, `*`, `/`
- 比较运算：`==`, `!=`, `<`, `>`, `<=`, `>=`

## API 使用示例

```c
#include "semantic.h"
#include "ast.h"

int main() {
    // 1. 初始化语义分析上下文
    SemanticContext *context = init_semantic();
    
    // 2. 创建AST节点（示例：变量声明）
    ASTNode *var_decl = create_decl("int", "x");
    
    // 3. 进行语义分析
    bool success = check_stmt(var_decl, context);
    
    // 4. 检查结果
    if (success && context->error_count == 0) {
        printf("语义分析成功\n");
    } else {
        printf("发现 %d 个语义错误\n", context->error_count);
    }
    
    // 5. 清理资源
    free_ast(var_decl);
    free_semantic(context);
    
    return 0;
}
```

## 错误类型说明

| 错误类型 | 说明 |
|---------|------|
| `SEM_OK` | 无错误 |
| `SEM_UNDECLARED_VAR` | 未声明变量 |
| `SEM_REDECLARED_VAR` | 重复声明变量 |
| `SEM_TYPE_MISMATCH` | 类型不匹配 |
| `SEM_INVALID_RETURN_TYPE` | 无效的返回类型 |
| `SEM_DIVISION_BY_ZERO` | 除零错误 |
| `SEM_INVALID_OPERATION` | 无效操作 |
| `SEM_FUNCTION_NOT_DECLARED` | 函数未声明 |

## 测试结果解读

运行 `semantic_test.exe` 后，您会看到：

```
==================== 语义分析测试 ====================

测试1：变量声明和使用
✓ 变量声明分析成功
✓ 变量使用类型检查成功  
✓ 赋值语句分析成功

测试2：类型不匹配错误检测
✓ 类型兼容性检查通过（int->float隐式转换）

测试3：未声明变量错误检测
语义错误: 未声明的变量 - 'undefined_var'
✓ 未声明变量错误检测成功

测试4：二元运算类型检查
✓ 二元运算类型推导成功

测试5：除零错误检测  
语义错误: 除零错误 - 整数除零
✓ 除零错误检测成功

==================== 测试结果 ====================
总错误数量: 2
===== 符号表 =====
名称            种类       类型       作用域
--------------------------------------------
y               变量       float      0
x               变量       int        0
```

其中：
- `✓` 表示测试通过
- `✗` 表示测试失败  
- 语义错误信息会自动输出
- 最后会显示符号表内容

这个语义分析器已经能够有效检测常见的语义错误，为编译器的后续阶段提供可靠的语义信息。
