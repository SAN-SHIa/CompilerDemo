#!/usr/bin/env python3
"""
Python LR(1)分析器 - 仿照C++后端逻辑
用于处理C语言文法的完整分析过程
"""

class Production:
    """产生式类"""
    def __init__(self, left, right):
        self.left = left
        self.right = right if right else ["ε"]
    
    def __str__(self):
        return f"{self.left} → {' '.join(self.right)}"
    
    def __repr__(self):
        return self.__str__()

class LRItem:
    """LR项目类"""
    def __init__(self, production, dot_pos, lookahead):
        self.production = production
        self.dot_pos = dot_pos
        self.lookahead = lookahead
    
    def __str__(self):
        right = self.production.right[:]
        right.insert(self.dot_pos, "•")
        return f"{self.production.left} → {' '.join(right)}, {self.lookahead}"
    
    def __eq__(self, other):
        return (self.production.left == other.production.left and
                self.production.right == other.production.right and
                self.dot_pos == other.dot_pos and
                self.lookahead == other.lookahead)
    
    def __hash__(self):
        return hash((self.production.left, tuple(self.production.right), 
                    self.dot_pos, self.lookahead))
    
    def next_symbol(self):
        """获取点后面的符号"""
        if self.dot_pos < len(self.production.right):
            return self.production.right[self.dot_pos]
        return None
    
    def advance(self):
        """点向前移动一位"""
        return LRItem(self.production, self.dot_pos + 1, self.lookahead)

class Grammar:
    """文法类"""
    def __init__(self):
        self.productions = []
        self.terminals = set()
        self.nonterminals = set()
        self.start_symbol = None
    
    def add_production(self, left, right):
        """添加产生式"""
        if isinstance(right, str):
            right = [right] if right != "ε" else []
        
        production = Production(left, right)
        self.productions.append(production)
        self.nonterminals.add(left)
        
        if self.start_symbol is None:
            self.start_symbol = left
        
        for symbol in right:
            if symbol != "ε":
                if symbol.islower() or symbol in ['(', ')', '=', '+', '-', '*', '/', '<', '>', '==', '!=', 
                                                  'if', 'while', 'int', 'float', 'char', 'id', 'num']:
                    self.terminals.add(symbol)
                else:
                    self.nonterminals.add(symbol)
    
    def get_first_set(self, symbol):
        """计算FIRST集"""
        if symbol in self.terminals or symbol == "ε":
            return {symbol}
        
        first = set()
        # 防止无限递归
        if hasattr(self, "_current_first_calc") and symbol in self._current_first_calc:
            return set()  # 正在计算中，返回空集
        
        # 标记当前符号正在计算
        if not hasattr(self, "_current_first_calc"):
            self._current_first_calc = set()
        self._current_first_calc.add(symbol)
        
        for prod in self.productions:
            if prod.left == symbol:
                if not prod.right or prod.right[0] == "ε":
                    first.add("ε")
                else:
                    all_have_epsilon = True
                    for s in prod.right:
                        first_s = self.get_first_set(s)
                        first.update(first_s - {"ε"})
                        if "ε" not in first_s:
                            all_have_epsilon = False
                            break
                    if all_have_epsilon:
                        first.add("ε")
        
        # 计算完毕，移除标记
        self._current_first_calc.remove(symbol)
        if len(self._current_first_calc) == 0:
            del self._current_first_calc
        
        return first
    
    def get_follow_set(self, symbol):
        """计算FOLLOW集"""
        follow = set()
        if symbol == self.start_symbol:
            follow.add("#")
        
        for prod in self.productions:
            for i, s in enumerate(prod.right):
                if s == symbol:
                    # 查看后面的符号
                    if i + 1 < len(prod.right):
                        next_symbol = prod.right[i + 1]
                        first_next = self.get_first_set(next_symbol)
                        follow.update(first_next - {"ε"})
                        if "ε" in first_next:
                            follow.update(self.get_follow_set(prod.left))
                    else:
                        follow.update(self.get_follow_set(prod.left))
        return follow

class LR1Parser:
    """LR(1)分析器"""
    def __init__(self, grammar):
        self.grammar = grammar
        self.states = []
        self.action_table = {}
        self.goto_table = {}
        self.build_tables()
    
    def closure(self, items):
        """计算项目集的闭包"""
        closure_set = set(items)
        changed = True
        
        print(f"计算闭包，初始项目集大小: {len(items)}")
        for item in items:
            print(f"  初始项目: {item}")
        
        # 特殊处理：如果有S'→•S的项目，添加S→•id=E项目
        for item in items:
            if (item.production.left.endswith("'") and 
                item.dot_pos == 0 and 
                len(item.production.right) == 1 and
                item.production.right[0] == 'S'):
                # 查找所有以S为左部的产生式
                for prod in self.grammar.productions:
                    if prod.left == 'S':
                        new_item = LRItem(prod, 0, item.lookahead)
                        if new_item not in closure_set:
                            print(f"  添加S的起始项目: {new_item}")
                            closure_set.add(new_item)
                            changed = True
        
        iteration = 0
        while changed and iteration < 100:  # 限制迭代次数，防止无限循环
            iteration += 1
            changed = False
            new_items = set()
            
            for item in closure_set:
                next_sym = item.next_symbol()
                if next_sym and next_sym in self.grammar.nonterminals:
                    # 计算FIRST(βa)
                    beta = item.production.right[item.dot_pos + 1:] + [item.lookahead]
                    print(f"  处理项目: {item}, beta={beta}")
                    first_beta = self.compute_first_string(beta)
                    print(f"  FIRST({beta}) = {first_beta}")
                    
                    for prod in self.grammar.productions:
                        if prod.left == next_sym:
                            for la in first_beta:
                                new_item = LRItem(prod, 0, la)
                                if new_item not in closure_set:
                                    print(f"  添加新项目: {new_item}")
                                    new_items.add(new_item)
                                    changed = True
            
            closure_set.update(new_items)
            print(f"  迭代 {iteration} 后闭包大小: {len(closure_set)}")
        
        if iteration >= 100:
            print("警告：闭包计算达到最大迭代次数限制")
        
        return closure_set
    
    def compute_first_string(self, symbols):
        """计算符号串的FIRST集"""
        if not symbols:
            return {"ε"}
        
        first = set()
        
        # 处理特殊情况：前瞻符号
        if len(symbols) == 1 and symbols[0] == "#":
            return {"#"}
            
        # 如果第一个元素是终结符或ε，则直接返回
        if symbols[0] in self.grammar.terminals or symbols[0] == "ε":
            return {symbols[0]}
        
        # 如果第一个元素是前瞻符号#，直接返回
        if symbols[0] == "#":
            return {"#"}

        # 如果符号串的第一个元素是运算符，直接返回它
        if symbols[0] in ['+', '-', '*', '/']:
            return {symbols[0]}
            
        epsilon_in_all = True
        
        # 从左到右计算每个符号的FIRST集
        for symbol in symbols:
            # 如果是前瞻符号，直接添加并继续
            if symbol == "#":
                first.add("#")
                continue

            # 如果是运算符，直接添加并继续
            if symbol in ['+', '-', '*', '/']:
                first.add(symbol)
                epsilon_in_all = False
                break
                
            first_sym = self.grammar.get_first_set(symbol)
            # 添加除ε以外的所有符号
            first.update(first_sym - {"ε"})
            # 如果当前符号的FIRST集不包含ε，则停止计算
            if "ε" not in first_sym:
                epsilon_in_all = False
                break
        
        # 如果所有符号的FIRST集都包含ε，则在结果中添加ε
        if epsilon_in_all:
            first.add("ε")
            
        return first
    
    def goto(self, items, symbol):
        """计算GOTO函数"""
        goto_items = set()
        for item in items:
            if item.next_symbol() == symbol:
                goto_items.add(item.advance())
        
        return self.closure(goto_items) if goto_items else set()
    
    def build_tables(self):
        """构建LR(1)分析表"""
        # 清空旧表
        self.states = []
        self.action_table = {}
        self.goto_table = {}
        
        # 拓广文法
        augmented_start = self.grammar.start_symbol + "'"
        start_prod = Production(augmented_start, [self.grammar.start_symbol])
        
        # 确保拓广产生式是第一个
        if self.grammar.productions and self.grammar.productions[0].left != augmented_start:
            self.grammar.productions.insert(0, start_prod)
        
        start_item = LRItem(start_prod, 0, "#")
        
        # 初始状态
        initial_state = self.closure({start_item})
        self.states = [initial_state]
        
        # 用于快速查找状态
        state_map = {frozenset(initial_state): 0}
        
        # 构建所有状态
        i = 0
        print(f"开始构建LR(1)分析表...")
        
        # 确保特殊运算符被识别为终结符
        if '-' not in self.grammar.terminals:
            self.grammar.terminals.add('-')
        if '/' not in self.grammar.terminals:
            self.grammar.terminals.add('/')
        
        while i < len(self.states):
            state = self.states[i]
            symbols = set()
            
            print(f"处理状态 {i}，包含 {len(state)} 个项目")
            for item in state:
                print(f"  {item}")
            
            # 收集所有可能的转移符号
            for item in state:
                next_sym = item.next_symbol()
                if next_sym:
                    symbols.add(next_sym)
            
            for symbol in symbols:
                new_state = self.goto(state, symbol)
                if new_state:
                    # 使用frozenset作为状态的唯一标识
                    state_key = frozenset(new_state)
                    
                    if state_key in state_map:
                        state_index = state_map[state_key]
                    else:
                        state_index = len(self.states)
                        self.states.append(new_state)
                        state_map[state_key] = state_index
                    
                    # 构建转移表
                    if symbol in self.grammar.terminals:
                        self.action_table[(i, symbol)] = ('shift', state_index)
                    else:
                        self.goto_table[(i, symbol)] = state_index
            
            # 处理归约项目
            for item in state:
                if item.dot_pos == len(item.production.right):
                    if item.production.left == augmented_start:
                        self.action_table[(i, "#")] = ('accept', None)
                    else:
                        # 查找产生式编号
                        for j, prod in enumerate(self.grammar.productions):
                            if (prod.left == item.production.left and 
                                prod.right == item.production.right):
                                self.action_table[(i, item.lookahead)] = ('reduce', j)
                                break
            
            i += 1
    
    def parse(self, input_string):
        """解析输入串"""
        # 改进的token化处理
        if isinstance(input_string, str):
            tokens = self.tokenize(input_string)
        else:
            # 已经是token列表，直接使用
            tokens = input_string
            
            # 调试输出处理的tokens
            print(f"直接传入的词法单元: {tokens}")
        
        tokens = tokens + ["#"]
        stack = [0]  # 状态栈
        symbol_stack = ["#"]  # 符号栈
        steps = []
        
        i = 0
        while i < len(tokens):
            state = stack[-1]
            token = tokens[i]
            
            action = self.action_table.get((state, token))
            if not action:
                # 添加调试信息
                print(f"错误：在状态{state}遇到符号{token}，但没有对应动作")
                print(f"当前状态:{state}, 可用的动作:")
                for (s, sym), act in self.action_table.items():
                    if s == state:
                        print(f"  符号:{sym} -> 动作:{act}")
                return False, f"语法错误：在状态{state}遇到符号{token}", steps
            
            action_type, value = action
            
            if action_type == 'shift':
                stack.append(value)
                symbol_stack.append(token)
                steps.append({
                    'step': len(steps) + 1,
                    'stack': list(stack),
                    'symbols': list(symbol_stack),
                    'input': tokens[i:],
                    'action': f"S{value}",
                    'description': f"移进{token}"
                })
                i += 1
            
            elif action_type == 'reduce':
                prod = self.grammar.productions[value]
                # 弹出栈
                pop_count = len(prod.right) if prod.right != ["ε"] else 0
                for _ in range(pop_count):
                    stack.pop()
                    symbol_stack.pop()
                
                # 查找GOTO
                new_state = self.goto_table.get((stack[-1], prod.left))
                if new_state is None:
                    return False, f"GOTO表错误", steps
                
                stack.append(new_state)
                symbol_stack.append(prod.left)
                
                steps.append({
                    'step': len(steps) + 1,
                    'stack': list(stack),
                    'symbols': list(symbol_stack),
                    'input': tokens[i:],
                    'action': f"R{value}",
                    'description': f"归约{prod}"
                })
            
            elif action_type == 'accept':
                steps.append({
                    'step': len(steps) + 1,
                    'stack': list(stack),
                    'symbols': list(symbol_stack),
                    'input': tokens[i:],
                    'action': "ACC",
                    'description': "接受"
                })
                return True, "分析成功", steps
        
        return False, "意外结束", steps

    def tokenize(self, input_string):
        """将输入字符串分解为token列表"""
        import re
        
        # 定义token模式 - 关键字必须放在标识符前面检测，否则会被识别为标识符
        patterns = [
            # 二元运算符
            (r'==', '=='),
            (r'!=', '!='),
            (r'<=', '<='),
            (r'>=', '>='),
            (r'&&', '&&'),
            (r'\|\|', '||'),
            (r'\+\+', '++'),
            (r'--', '--'),
            
            # 关键字需要在标识符之前检测
            (r'\bif\b', 'if'),
            (r'\bwhile\b', 'while'),
            (r'\bfor\b', 'for'),
            (r'\bint\b', 'int'),
            (r'\bfloat\b', 'float'),
            (r'\bchar\b', 'char'),
            (r'\bvoid\b', 'void'),
            
            # 标识符放在关键字后面
            (r'[a-zA-Z_][a-zA-Z0-9_]*', 'id'),
            (r'\d+', 'num'),
            
            # 基本运算符 - 注意这里明确包含了减号和除号
            # 明确处理四种基本算术运算符，确保每个都被正确识别
            (r'[+]', '+'),
            (r'[-]', '-'),  # 特别注意减号的处理
            (r'[*]', '*'),
            (r'[/]', '/'),  # 特别注意除号的处理
            
            # 其他运算符和符号
            (r'[=<>!&|(){}[\];,.]', lambda m: m.group(0)),
            (r'\s+', None)  # 忽略空白字符
        ]
        
        tokens = []
        pos = 0
        debug = False  # 设置为True可以启用调试输出
        
        if debug:
            print(f"开始分词，输入: '{input_string}'")
        
        # 添加额外的调试输出
        if '-' in input_string or '/' in input_string:
            debug_special = True
            print(f"检测到特殊字符: 输入='{input_string}'")
        else:
            debug_special = False
            
        while pos < len(input_string):
            matched = False
            
            for pattern, token_type in patterns:
                regex = re.compile(pattern)
                match = regex.match(input_string, pos)
                
                if match:
                    value = match.group(0)
                    if token_type is not None:  # 不忽略
                        if callable(token_type):
                            token = token_type(match)
                            tokens.append(token)
                            if debug or (debug_special and value in ['-', '/']):
                                print(f"匹配: '{value}' -> '{token}' (调用函数)")
                        else:
                            tokens.append(token_type)
                            if debug or (debug_special and value in ['-', '/']):
                                print(f"匹配: '{value}' -> '{token_type}'")
                    else:
                        if debug:
                            print(f"忽略: '{value}'")
                    pos = match.end()
                    matched = True
                    break
            
            if not matched:
                # 跳过无法识别的字符
                if debug or (debug_special and input_string[pos] in ['-', '/']):
                    print(f"跳过无法识别的字符: '{input_string[pos]}'")
                pos += 1
        
        # 对算术表达式中的token进行额外检查
        has_arithmetic_ops = any(t in ['+', '-', '*', '/'] for t in tokens)
        if has_arithmetic_ops and (debug or debug_special):
            print(f"算术表达式分词结果: {tokens}")
        
        return tokens

def create_c_grammar(grammar_number):
    """创建C语言文法"""
    grammar = Grammar()
    
    if grammar_number == 7:  # 简单赋值语句
        grammar.add_production("S", ["id", "=", "E"])
        grammar.add_production("E", ["E", "+", "T"])
        grammar.add_production("E", ["T"])
        grammar.add_production("T", ["T", "*", "F"])
        grammar.add_production("T", ["F"])
        grammar.add_production("F", ["(", "E", ")"])
        grammar.add_production("F", ["id"])
        grammar.add_production("F", ["num"])
    
    elif grammar_number == 8:  # if语句
        grammar.add_production("S", ["if", "(", "E", ")", "S"])
        grammar.add_production("S", ["id", "=", "E"])
        grammar.add_production("E", ["E", "==", "T"])
        grammar.add_production("E", ["T"])
        grammar.add_production("T", ["id"])
        grammar.add_production("T", ["num"])
    
    elif grammar_number == 9:  # 变量声明
        grammar.add_production("S", ["T", "id"])
        grammar.add_production("T", ["int"])
        grammar.add_production("T", ["float"])
        grammar.add_production("T", ["char"])
    
    elif grammar_number == 10:  # while循环
        grammar.add_production("S", ["while", "(", "E", ")", "S"])
        grammar.add_production("S", ["id", "=", "E"])
        grammar.add_production("E", ["E", "<", "T"])
        grammar.add_production("E", ["T"])
        grammar.add_production("T", ["T", "+", "F"])  # 添加加法支持
        grammar.add_production("T", ["F"])
        grammar.add_production("F", ["id"])
        grammar.add_production("F", ["num"])
    
    elif grammar_number == 11:  # 算术表达式
        # 修正：按标准算术表达式优先级处理，确保词法分析器对减号和除号正确识别
        grammar.add_production("E", ["E", "+", "T"])  # 加法
        grammar.add_production("E", ["E", "-", "T"])  # 减法
        grammar.add_production("E", ["T"])
        grammar.add_production("T", ["T", "*", "F"])  # 乘法
        grammar.add_production("T", ["T", "/", "F"])  # 除法
        grammar.add_production("T", ["F"])
        grammar.add_production("F", ["(", "E", ")"])  # 括号
        grammar.add_production("F", ["id"])           # 标识符
        grammar.add_production("F", ["num"])          # 数字
    
    return grammar