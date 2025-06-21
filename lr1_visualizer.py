#!/usr/bin/env python3
"""
DFA和语法树可视化模块
使用Graphviz生成图形
"""

import graphviz
from pathlib import Path

class SyntaxTreeNode:
    """语法树节点"""
    def __init__(self, symbol, children=None):
        self.symbol = symbol
        self.children = children or []
        self.node_id = None
    
    def add_child(self, child):
        self.children.append(child)
    
    def __str__(self):
        return f"Node({self.symbol})"

class DFAVisualizer:
    """DFA可视化器"""
    def __init__(self, parser):
        self.parser = parser
    
    def generate_dfa_graph(self, output_path):
        """生成DFA状态转换图"""
        try:
            dot = graphviz.Digraph(comment='LR(1) DFA')
            dot.attr(rankdir='LR', size='16,12')
            dot.attr('node', shape='circle', style='filled')
            
            # 添加状态节点
            for i, state in enumerate(self.parser.states):
                # 检查是否为接受状态
                is_accept = any(
                    item.production.left.endswith("'") and 
                    item.dot_pos == len(item.production.right)
                    for item in state
                )
                
                if i == 0:
                    dot.node(f'S{i}', f'S{i}', fillcolor='lightgreen')
                elif is_accept:
                    dot.node(f'S{i}', f'S{i}', fillcolor='lightcoral', shape='doublecircle')
                else:
                    dot.node(f'S{i}', f'S{i}', fillcolor='lightblue')
            
            # 添加转换边
            transitions = {}
            
            # 从ACTION表添加转换
            for (state, symbol), (action_type, target) in self.parser.action_table.items():
                if action_type == 'shift':
                    if (state, target) not in transitions:
                        transitions[(state, target)] = []
                    transitions[(state, target)].append(symbol)
            
            # 从GOTO表添加转换
            for (state, symbol), target in self.parser.goto_table.items():
                if (state, target) not in transitions:
                    transitions[(state, target)] = []
                transitions[(state, target)].append(symbol)
            
            # 绘制转换边
            for (from_state, to_state), symbols in transitions.items():
                label = ', '.join(symbols)
                dot.edge(f'S{from_state}', f'S{to_state}', label=label)
            
            # 渲染图像
            output_file = Path(output_path).with_suffix('')
            dot.render(str(output_file), format='png', cleanup=True)
            return str(output_path)
            
        except Exception as e:
            print(f"生成DFA图失败: {e}")
            return None

class SyntaxTreeBuilder:
    """语法树构建器"""
    def __init__(self, grammar):
        self.grammar = grammar
        self.node_counter = 0
    
    def build_tree_from_steps(self, parse_steps):
        """从分析步骤构建语法树"""
        # 模拟栈操作构建语法树
        tree_stack = []
        
        for step in parse_steps:
            action = step['action']
            description = step['description']
            
            if action.startswith('S'):  # 移进
                # 从描述中提取移进的符号
                if '移进' in description:
                    symbol = description.replace('移进', '')
                    node = SyntaxTreeNode(symbol)
                    node.node_id = self.node_counter
                    self.node_counter += 1
                    tree_stack.append(node)
            
            elif action.startswith('R'):  # 归约
                # 从描述中解析产生式
                if '归约' in description:
                    prod_str = description.replace('归约', '')
                    if '→' in prod_str:
                        left, right = prod_str.split('→', 1)
                        left = left.strip()
                        right_symbols = right.strip().split()
                        
                        # 创建新的内部节点
                        parent = SyntaxTreeNode(left)
                        parent.node_id = self.node_counter
                        self.node_counter += 1
                        
                        # 弹出对应数量的子节点
                        children = []
                        for _ in range(len(right_symbols)):
                            if tree_stack:
                                child = tree_stack.pop()
                                children.insert(0, child)
                        
                        # 如果是空产生式
                        if right_symbols == ['ε'] or not right_symbols:
                            epsilon_node = SyntaxTreeNode("ε")
                            epsilon_node.node_id = self.node_counter
                            self.node_counter += 1
                            children = [epsilon_node]
                        
                        # 添加子节点到父节点
                        for child in children:
                            parent.add_child(child)
                        
                        tree_stack.append(parent)
            
            elif action == 'ACC':  # 接受
                # 分析完成，返回根节点
                break
        
        return tree_stack[-1] if tree_stack else None

class SyntaxTreeVisualizer:
    """语法树可视化器"""
    def __init__(self):
        self.node_counter = 0
    
    def generate_tree_graph(self, root, output_path):
        """生成语法树图"""
        try:
            dot = graphviz.Digraph(comment='Syntax Tree')
            dot.attr(size='12,8')
            dot.attr('node', shape='circle', style='filled')
            
            self._add_tree_nodes(dot, root)
            
            # 渲染图像
            from pathlib import Path
            output_file = Path(output_path).with_suffix('')
            dot.render(str(output_file), format='png', cleanup=True)
            return str(output_path)
            
        except Exception as e:
            print(f"生成语法树图失败: {e}")
            return None
    
    def _add_tree_nodes(self, dot, node):
        """递归添加树节点"""
        if node is None:
            return
        
        # 设置节点样式
        if node.symbol in ['id', 'num', 'int', 'float', 'char', 'if', 'while', '(', ')', '=', '+', '*', '<', '>', '==']:
            # 终结符 - 绿色
            dot.node(str(node.node_id), node.symbol, fillcolor='lightgreen')
        elif node.symbol == 'ε':
            # 空符号 - 灰色
            dot.node(str(node.node_id), 'ε', fillcolor='lightgray')
        else:
            # 非终结符 - 蓝色
            dot.node(str(node.node_id), node.symbol, fillcolor='lightblue')
        
        # 添加子节点和边
        for child in node.children:
            self._add_tree_nodes(dot, child)
            dot.edge(str(node.node_id), str(child.node_id))

def generate_lr1_table_markdown(parser, grammar):
    """生成LR(1)分析表的Markdown格式"""
    # 收集所有符号
    terminals = sorted(list(grammar.terminals) + ["#"])
    nonterminals = sorted(list(grammar.nonterminals))
    
    # 创建表头
    header = "| 状态 | " + " | ".join(terminals) + " | " + " | ".join(nonterminals) + " |"
    separator = "|------|" + "|".join(["-" * (len(t) + 2) for t in terminals]) + "|" + "|".join(["-" * (len(nt) + 2) for nt in nonterminals]) + "|"
    
    table_lines = [
        "# LR(1)分析表",
        "",
        "## 文法产生式",
        ""
    ]
    
    # 添加产生式
    for i, prod in enumerate(grammar.productions):
        table_lines.append(f"{i}. {prod}")
    
    table_lines.extend([
        "",
        "## ACTION表和GOTO表",
        "",
        header,
        separator
    ])
    
    # 生成表格内容
    for i in range(len(parser.states)):
        row = f"| {i:4} |"
        
        # ACTION部分
        for terminal in terminals:
            action = parser.action_table.get((i, terminal))
            if action:
                action_type, value = action
                if action_type == 'shift':
                    row += f" S{value} |"
                elif action_type == 'reduce':
                    row += f" R{value} |"
                elif action_type == 'accept':
                    row += " ACC |"
            else:
                row += "     |"
        
        # GOTO部分
        for nonterminal in nonterminals:
            goto = parser.goto_table.get((i, nonterminal))
            if goto is not None:
                row += f" {goto:3} |"
            else:
                row += "     |"
        
        table_lines.append(row)
    
    table_lines.extend([
        "",
        "### 符号说明",
        "- **S**: 移进到状态",
        "- **R**: 按产生式归约",
        "- **ACC**: 接受",
        "- **数字**: 转移到的状态"
    ])
    
    return "\n".join(table_lines)

def generate_analysis_steps_markdown(steps, success, message):
    """生成分析步骤的Markdown格式"""
    lines = [
        "# LR(1)分析过程",
        "",
        f"## 分析结果: {'✅ 成功' if success else '❌ 失败'}",
        f"**消息**: {message}",
        "",
        "## 逐步分析过程",
        "",
        "| 步骤 | 状态栈 | 符号栈 | 剩余输入 | 动作 | 说明 |",
        "|------|--------|--------|----------|------|------|"
    ]
    
    for step in steps:
        stack_str = ' '.join(map(str, step['stack']))
        symbols_str = ' '.join(step['symbols'])
        input_str = ' '.join(step['input'])
        
        lines.append(f"| {step['step']:4} | {stack_str:15} | {symbols_str:15} | {input_str:15} | {step['action']:8} | {step['description']} |")
    
    lines.extend([
        "",
        "## 分析说明",
        "上述过程展示了LR(1)分析器的完整工作流程：",
        "1. **移进操作(S)**: 将输入符号及其目标状态压入栈",
        "2. **归约操作(R)**: 根据产生式将栈顶符号归约为左部非终结符",
        "3. **接受操作(ACC)**: 分析成功完成",
        "4. **状态转移**: 根据ACTION表和GOTO表进行状态转换"
    ])
    
    return "\n".join(lines)