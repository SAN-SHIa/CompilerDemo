#include<iostream>
#include<vector>
#include<map>
#include<set>
#include<string>
#include<algorithm>
#include<iomanip>
#include<fstream>

// 文法产生式结构
struct Production {
    char leftSide;  // 产生式左侧的非终结符
    std::string rightSide;  // 产生式右侧的符号串
};

// LR(1)分析所需的结构体定义
struct lookaheadProduction {
    char leftSide;  // 产生式左侧的非终结符
    std::string rightSide;  // 产生式右侧的符号串
    char lookahead;  // 前瞻符号（用于LR(1)分析）
    int dotPosition; // 点号位置
    
    // 用于在set中比较
    bool operator<(const lookaheadProduction& other) const {
        if (leftSide != other.leftSide)
            return leftSide < other.leftSide;
        if (rightSide != other.rightSide)
            return rightSide < other.rightSide;
        if (lookahead != other.lookahead)
            return lookahead < other.lookahead;
        return dotPosition < other.dotPosition;
    }
};

struct DFAState {
    int id;  // 状态编号
    std::set<lookaheadProduction> productions;  // 项集
    bool isAccepting;  // 是否为接受状态
    // 替换单一转移为转移映射
    std::map<char, int> transitions;  // 字符到状态ID的转移映射
};

// 判断字符是否为非终结符（大写字母）
bool isNonTerminal(char c) {
    return c >= 'A' && c <= 'Z';
}

// 判断字符是否为终结符（小写字母或其他特殊符号）
bool isTerminal(char c) {
    return !isNonTerminal(c) && c != '@'; // 使用'@'代替'ε'
}

// 计算First集
std::map<char, std::set<char>> computeFirst(const std::vector<Production>& grammar) {
    std::map<char, std::set<char>> first;
    bool changed = true;

    // 初始化：对于终结符a，First(a) = {a}
    for (const auto& prod : grammar) {
        char leftSide = prod.leftSide;
        if (!first.count(leftSide)) {
            first[leftSide] = std::set<char>();
        }

        for (char c : prod.rightSide) {
            if (isTerminal(c)) {
                if (!first.count(c)) {
                    first[c] = std::set<char>{ c };
                }
            }
        }
    }

    // 重复直到没有First集发生变化
    while (changed) {
        changed = false;

        for (const auto& prod : grammar) {
            char leftSide = prod.leftSide;
            std::string rightSide = prod.rightSide;

            // 如果产生式是 X → ε
            if (rightSide == "@") { // 使用"@"代替"ε"
                if (first[leftSide].insert('@').second) { // 使用'@'代替'ε'
                    changed = true;
                }
                continue;
            }

            // 对于产生式 X → Y1Y2...Yk
            bool allHaveEpsilon = true;
            for (size_t i = 0; i < rightSide.size(); ++i) {
                char currentSymbol = rightSide[i];

                // 如果当前符号是终结符
                if (isTerminal(currentSymbol)) {
                    if (first[leftSide].insert(currentSymbol).second) {
                        changed = true;
                    }
                    allHaveEpsilon = false;
                    break; // 添加完终结符后直接结束
                }

                // 如果当前符号是非终结符
                else if (isNonTerminal(currentSymbol)) {
                    bool hasEpsilon = false;

                    // 将First(currentSymbol)中的非ε符号添加到First(leftSide)
                    for (char symbol : first[currentSymbol]) {
                        if (symbol != '@') { // 使用'@'代替'ε'
                            if (first[leftSide].insert(symbol).second) {
                                changed = true;
                            }
                        }
                        else {
                            hasEpsilon = true;
                        }
                    }

                    // 如果First(currentSymbol)不包含ε，就停止
                    if (!hasEpsilon) {
                        allHaveEpsilon = false;
                        break;
                    }

                    // 如果这是最后一个符号，且First(currentSymbol)包含ε
                    if (i == rightSide.size() - 1 && hasEpsilon) {
                        if (first[leftSide].insert('@').second) { // 使用'@'代替'ε'
                            changed = true;
                        }
                    }
                }
            }

            // 如果所有符号的First集都包含ε
            if (allHaveEpsilon) {
                if (first[leftSide].insert('@').second) { // 使用'@'代替'ε'
                    changed = true;
                }
            }
        }
    }

    return first;
}

// // 计算Follow集
// std::map<char, std::set<char>> computeFollow(const std::vector<Production>& grammar, const std::map<char, std::set<char>>& first) {
//     std::map<char, std::set<char>> follow;
//     bool changed = true;

//     // 初始化：对于开始符号S，Follow(S)包含$
//     char startSymbol = 'S';
//     follow[startSymbol].insert('$');

//     // 为所有非终结符初始化Follow集
//     for (const auto& prod : grammar) {
//         if (!follow.count(prod.leftSide)) {
//             follow[prod.leftSide] = std::set<char>();
//         }

//         for (char c : prod.rightSide) {
//             if (isNonTerminal(c) && !follow.count(c)) {
//                 follow[c] = std::set<char>();
//             }
//         }
//     }

//     // 重复直到没有Follow集发生变化
//     while (changed) {
//         changed = false;

//         for (const auto& prod : grammar) {
//             char leftSide = prod.leftSide;
//             std::string rightSide = prod.rightSide;

//             if (rightSide == "@") continue; // 使用"@"代替"ε"

//             for (size_t i = 0; i < rightSide.size(); ++i) {
//                 // 只关心非终结符
//                 if (!isNonTerminal(rightSide[i])) continue;

//                 char currentNonTerminal = rightSide[i];
//                 bool shouldAddFollowOfLeftSide = false;

//                 // 如果不是最后一个字符
//                 if (i < rightSide.size() - 1) {
//                     // 处理紧跟着的符号
//                     if (isTerminal(rightSide[i + 1])) {
//                         if (follow[currentNonTerminal].insert(rightSide[i + 1]).second) {
//                             changed = true;
//                         }
//                     }

//                     else if (isNonTerminal(rightSide[i + 1])) {
//                         // 添加First(下一个符号)到Follow(当前符号)
//                         for (char symbol : first.at(rightSide[i + 1])) {
//                             if (symbol != '@') { // 使用'@'代替'ε'
//                                 if (follow[currentNonTerminal].insert(symbol).second) {
//                                     changed = true;
//                                 }
//                             }
//                             else {
//                                 shouldAddFollowOfLeftSide = true;
//                             }
//                         }
//                     }

//                     // 检查剩余所有符号是否都可推导出ε
//                     bool allCanDeriveEpsilon = true;
//                     for (size_t j = i + 1; j < rightSide.size(); ++j) {
//                         if (isTerminal(rightSide[j]) ||
//                             (isNonTerminal(rightSide[j]) && !first.at(rightSide[j]).count('@'))) { // 使用'@'代替'ε'
//                             allCanDeriveEpsilon = false;
//                             break;
//                         }
//                     }

//                     if (allCanDeriveEpsilon) {
//                         shouldAddFollowOfLeftSide = true;
//                     }
//                 }

//                 else {
//                     // 是最后一个符号
//                     shouldAddFollowOfLeftSide = true;
//                 }

//                 // 将Follow(左侧)添加到Follow(当前非终结符)
//                 if (shouldAddFollowOfLeftSide) {
//                     for (char symbol : follow[leftSide]) {
//                         if (follow[currentNonTerminal].insert(symbol).second) {
//                             changed = true;
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     return follow;
// }

// 打印First集或Follow集
void printSets(const std::map<char, std::set<char>>& sets, const std::string& setName) {
    std::cout << setName << " 集合：" << std::endl;
    for (const auto& pair : sets) {
        if (isNonTerminal(pair.first)) { // 只打印非终结符的集合
            std::cout << setName << "(" << pair.first << ") = { ";
            bool first = true;
            for (char c : pair.second) {
                if (!first) std::cout << ", ";
                // 输出时将'@'显示为'ε'以保持原有语义
                if (c == '@') {
                    std::cout << "ε";
                }
                else {
                    std::cout << c;
                }
                first = false;
            }
            std::cout << " }" << std::endl;
        }
    }
    std::cout << std::endl;
}


// 打印文法
void printGrammar(const std::vector<Production>& grammar) {
    std::cout << "文法产生式：" << std::endl;

    // 按照非终结符分组，使用set去重
    std::map<char, std::set<std::string>> groupedProductions;
    for (const auto& prod : grammar) {
        groupedProductions[prod.leftSide].insert(prod.rightSide);
    }

    // 打印每个非终结符的产生式
    for (const auto& group : groupedProductions) {
        std::cout << group.first << " → ";
        size_t i = 0;
        for (const auto& right : group.second) {
            if (i > 0) std::cout << " | ";
            // 输出时将"@"显示为"ε"以保持原有语义
            if (right == "@") {
                std::cout << "ε";
            }
            else {
                std::cout << right;
            }
            i++;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void printNumGrammar(const std::vector<Production>& grammar) {
    std::cout << "编号文法产生式：" << std::endl;

    for (int i = 0; i < grammar.size(); i++) {
        const auto& prod = grammar[i];
        std::cout << i + 1 << ". " << prod.leftSide << " → ";
        // 输出时将"@"显示为"ε"以保持原有语义
        if (prod.rightSide == "@") {
            std::cout << "ε";
        }
        else {
            std::cout << prod.rightSide;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// 带前瞻符号的项目结构
struct LRItem {
    int productionIndex;  // 产生式在文法中的索引
    int dotPosition;      // 点号位置
    char lookahead;       // 前瞻符号
    
    // 用于在set中比较两个LRItem是否相等
    bool operator==(const LRItem& other) const {
        return productionIndex == other.productionIndex && 
               dotPosition == other.dotPosition && 
               lookahead == other.lookahead;
    }
    
    // 用于在set中排序
    bool operator<(const LRItem& other) const {
        if (productionIndex != other.productionIndex)
            return productionIndex < other.productionIndex;
        if (dotPosition != other.dotPosition)
            return dotPosition < other.dotPosition;
        return lookahead < other.lookahead;
    }
};

// 计算项目的闭包
std::set<LRItem> computeClosure(const std::set<LRItem>& items, 
                               const std::vector<Production>& grammar,
                               const std::map<char, std::set<char>>& first) {
    std::set<LRItem> closure = items;
    bool changed = true;
    
    while (changed) {
        changed = false;
        std::set<LRItem> newItems;
        
        for (const auto& item : closure) {
            int prodIndex = item.productionIndex;
            int dotPos = item.dotPosition;
            char lookahead = item.lookahead;
            
            // 如果点号在产生式右部的末尾，则无法扩展
            if (dotPos >= grammar[prodIndex].rightSide.length())
                continue;
            
            // 获取点号后的符号
            char symbolAfterDot = grammar[prodIndex].rightSide[dotPos];
            
            // 如果点号后的符号不是非终结符，则不需要扩展
            if (!isNonTerminal(symbolAfterDot))
                continue;
            
            // 计算First集
            std::set<char> firstBeta;
            bool hasEpsilon = true;
            
            // 计算First(β a)，β是点号后面的字符串，a是lookahead
            std::string beta = "";
            if (dotPos + 1 < grammar[prodIndex].rightSide.length()) {
                beta = grammar[prodIndex].rightSide.substr(dotPos + 1);
            }
            
            // 如果β为空，则直接使用lookahead
            if (beta.empty()) {
                firstBeta.insert(lookahead);//A → α.B中β, a中β为空
            } else {
                // 否则计算First(β)
                hasEpsilon = true;
                for (char c : beta) {
                    if (hasEpsilon) {
                        if (isTerminal(c)) {
                            firstBeta.insert(c);
                            hasEpsilon = false;
                            break;
                        } else if (isNonTerminal(c)) {
                            // 添加First(c)中除ε之外的所有符号
                            for (char fc : first.at(c)) {
                                if (fc != '@') { // @代表ε
                                    firstBeta.insert(fc);
                                }
                            }
                            // 检查c是否能推导出ε
                            if (first.at(c).find('@') == first.at(c).end()) {
                                hasEpsilon = false;
                                break;
                            }
                        }
                    }
                }
                
                // 如果β可以推导出ε，则添加lookahead
                if (hasEpsilon) {
                    firstBeta.insert(lookahead);
                }
            }
            
            // 对于产生式B->γ，点号后的符号是B
            for (int i = 0; i < grammar.size(); i++) {
                if (grammar[i].leftSide == symbolAfterDot) {//将行出现的产生式转为带搜索符的
                    // 为每个可能的前瞻符号b∈First(βa)添加新项
                    for (char b : firstBeta) {
                        LRItem newItem = {i, 0, b}; //编号，点的位置，搜索符
                        if (closure.find(newItem) == closure.end()) {
                            newItems.insert(newItem);
                            changed = true;
                        }
                    }
                }
            }
        }
        
        // 添加新找到的项目到闭包
        for (const auto& newItem : newItems) {
            closure.insert(newItem);
        }
    }
    
    return closure;
}

// 计算状态的转换
std::set<LRItem> computeGoto(const std::set<LRItem>& state, 
                            char symbol,
                            const std::vector<Production>& grammar,
                            const std::map<char, std::set<char>>& first) {
    std::set<LRItem> result;
    
    for (const auto& item : state) {
        int prodIndex = item.productionIndex;
        int dotPos = item.dotPosition;
        
        // 如果点号在产生式右部的末尾，或点号后的符号不是要查找的符号，则跳过
        if (dotPos >= grammar[prodIndex].rightSide.length() ||
            grammar[prodIndex].rightSide[dotPos] != symbol)
            continue;
        
        // 移动点号
        LRItem newItem = {prodIndex, dotPos + 1, item.lookahead};
        result.insert(newItem);
    }
    
    // 计算闭包
    return computeClosure(result, grammar, first);
}

// 创建LR(1) DFA
std::vector<DFAState> createDFA(const std::vector<Production>& grammar,
                               const std::map<char, std::set<char>>& first) {
    std::vector<DFAState> dfaStates;
    
    // 初始项目是起始产生式 X -> .S, #
    std::set<LRItem> initialItems = {{0, 0, '#'}};
    
    // 计算初始状态的闭包
    std::set<LRItem> initialClosure = computeClosure(initialItems, grammar, first);
    
    // 创建初始状态
    DFAState initialState = {0, {}, false, {}};  // 初始化为空转移表
    dfaStates.push_back(initialState);
    
    // 状态集合映射到状态ID的映射
    std::map<std::string, int> stateMap;
    std::vector<std::set<LRItem>> stateItems = {initialClosure};
    
    // 收集所有可能的转换符号（终结符和非终结符）
    std::set<char> symbols;
    for (const auto& prod : grammar) {
        for (char c : prod.rightSide) {
            if (c != '@') { // 排除ε
                symbols.insert(c);
            }
        }
    }
    
    // 处理所有状态，直到没有新的状态产生
    for (int i = 0; i < stateItems.size(); i++) {
        // 对于每个符号，计算转换
        for (char symbol : symbols) {
            std::set<LRItem> nextState = computeGoto(stateItems[i], symbol, grammar, first);
            
            // 如果转换结果不为空
            if (!nextState.empty()) {
                // 检查是否已存在相同的状态
                bool stateExists = false;
                int stateId = 0;
                
                for (int j = 0; j < stateItems.size(); j++) {
                    if (stateItems[j] == nextState) {
                        stateExists = true;
                        stateId = j;
                        break;
                    }
                }
                
                // 如果是新状态，则添加
                if (!stateExists) {
                    stateId = stateItems.size();
                    stateItems.push_back(nextState);
                    
                    DFAState newState = {stateId, {}, false, {}};  // 空转移表
                    dfaStates.push_back(newState);
                }
                
                // 记录当前符号对应的转移
                dfaStates[i].transitions[symbol] = stateId;
                
                std::cout << "状态" << i << " 通过符号 " << symbol << " 转移到状态" << stateId << std::endl;
            }
        }
    }
    
    // 标记接受状态（点号在最后，且前瞻符号是#的项目）
    for (int i = 0; i < stateItems.size(); i++) {
        for (const auto& item : stateItems[i]) {
            if (item.dotPosition == grammar[item.productionIndex].rightSide.length() &&
                item.lookahead == '#' &&
                item.productionIndex == 0) {
                dfaStates[i].isAccepting = true;
                break;
            }
        }
    }
    
    // 记录每个状态的项目集（用于展示）
    for (int i = 0; i < stateItems.size(); i++) {
        for (const auto& item : stateItems[i]) {
            lookaheadProduction lp = {
                grammar[item.productionIndex].leftSide,
                grammar[item.productionIndex].rightSide,
                item.lookahead,
                item.dotPosition
            };
            dfaStates[i].productions.insert(lp);
        }
    }
    
    std::cout << "生成了" << dfaStates.size() << "个DFA状态" << std::endl;
    return dfaStates;
}

// 将DFA导出为DOT格式
void exportDFAtoDOT(const std::vector<DFAState>& dfaStates, const std::string& filename) {
    std::ofstream dotFile(filename);
    
    if (!dotFile.is_open()) {
        std::cerr << "无法创建DOT文件: " << filename << std::endl;
        return;
    }
    
    // 写入DOT文件头
    dotFile << "digraph LR1_DFA {\n";
    dotFile << "  rankdir=LR;\n";
    dotFile << "  node [shape=box, style=rounded];\n";
    
    // 写入状态节点
    for (const auto& state : dfaStates) {
        dotFile << "  state" << state.id << " [label=\"状态 " << state.id << "\\n";
        
        // 添加项目集内容
        for (const auto& prod : state.productions) {
            std::string displayRightSide;
            
            // 根据dotPosition插入点号
            if (prod.rightSide == "@") {
                // 空产生式特殊处理
                displayRightSide = ".";
            } else {
                displayRightSide = prod.rightSide;
                // 在正确位置插入点号
                displayRightSide.insert(prod.dotPosition, ".");
            }
            
            dotFile << prod.leftSide << " → " << displayRightSide << ", " << prod.lookahead << "\\n";
        }
        
        if (state.isAccepting) {
            dotFile << "接受状态";
        }
        
        dotFile << "\"";
        
        // 接受状态使用双线框
        if (state.isAccepting) {
            dotFile << ", peripheries=2";
        }
        
        dotFile << "];\n";
    }
    
    // 写入转移边，每条边带有标签显示转移符号
    for (const auto& state : dfaStates) {
        for (const auto& transition : state.transitions) {
            char symbol = transition.first;
            int targetStateId = transition.second;
            
            // 输出形如：state0 -> state1 [label="L"];
            dotFile << "  state" << state.id << " -> state" << targetStateId 
                   << " [label=\"" << symbol << "\"];\n";
        }
    }
    
    dotFile << "}\n";
    dotFile.close();
    
    std::cout << "DFA已导出到DOT文件: " << filename << std::endl;
}

// 生成LR(1)分析表
std::pair<std::map<int, std::map<char, std::string>>, std::map<int, std::map<char, int>>> 
generateLR1Table(const std::vector<DFAState>& dfaStates, const std::vector<Production>& grammar) {
    // ACTION表: 状态 -> (终结符 -> 动作)
    std::map<int, std::map<char, std::string>> actionTable;
    // GOTO表: 状态 -> (非终结符 -> 目标状态)
    std::map<int, std::map<char, int>> gotoTable;
    
    // 遍历所有状态
    for (const auto& state : dfaStates) {
        // 处理移进操作 (通过transitions)
        for (const auto& transition : state.transitions) {
            char symbol = transition.first;
            int nextState = transition.second;
            
            if (isTerminal(symbol)) {
                // 终结符 - ACTION表中的移进操作
                actionTable[state.id][symbol] = "S" + std::to_string(nextState);
            } else {
                // 非终结符 - GOTO表
                gotoTable[state.id][symbol] = nextState;
            }
        }
        
        // 处理规约操作 (检查点号在末尾的项目)
        for (const auto& prod : state.productions) {
            if (prod.dotPosition == prod.rightSide.length()) {
                // 点号在末尾，规约操作
                if (prod.leftSide == 'X' && prod.lookahead == '#') {
                    // 接受状态
                    actionTable[state.id]['#'] = "ACC";
                } else {
                    // 规约操作，找出对应的产生式编号
                    int prodIndex = -1;
                    for (int i = 0; i < grammar.size(); i++) {
                        if (grammar[i].leftSide == prod.leftSide && 
                            grammar[i].rightSide == prod.rightSide) {
                            prodIndex = i;
                            break;
                        }
                    }
                    
                    if (prodIndex != -1) {
                        // 规约使用产生式 prodIndex
                        actionTable[state.id][prod.lookahead] = "R" + std::to_string(prodIndex);
                    }
                }
            }
        }
    }
    
    return {actionTable, gotoTable};
}

// 打印LR(1)分析表到控制台
void printLR1Table(const std::vector<DFAState>& dfaStates, 
                  const std::vector<Production>& grammar) {
    // 生成分析表
    auto [actionTable, gotoTable] = generateLR1Table(dfaStates, grammar);
    
    // 收集所有终结符和非终结符
    std::set<char> terminals;
    std::set<char> nonTerminals;
    
    for (const auto& prod : grammar) {
        nonTerminals.insert(prod.leftSide);
        for (char c : prod.rightSide) {
            if (isTerminal(c)) {
                terminals.insert(c);
            }
        }
    }
    terminals.insert('#'); // 添加结束符
    
    // 打印表头
    std::cout << "LR(1) 分析表：" << std::endl;
    std::cout << "+------+";
    // ACTION表的列
    for (char term : terminals) {
        std::cout << "----------+";
    }
    // GOTO表的列
    for (char nonTerm : nonTerminals) {
        if (nonTerm != 'X') { // 不显示增广文法的起始符号
            std::cout << "----------+";
        }
    }
    std::cout << std::endl;
    
    // 打印列标题
    std::cout << "|      |";
    // ACTION部分的列标题
    for (char term : terminals) {
        std::cout << "   " << term << "      |";
    }
    // GOTO部分的列标题
    for (char nonTerm : nonTerminals) {
        if (nonTerm != 'X') {
            std::cout << "   " << nonTerm << "      |";
        }
    }
    std::cout << std::endl;
    
    // 打印分隔线
    std::cout << "+------+";
    for (char term : terminals) {
        std::cout << "----------+";
    }
    for (char nonTerm : nonTerminals) {
        if (nonTerm != 'X') {
            std::cout << "----------+";
        }
    }
    std::cout << std::endl;
    
    // 打印每个状态的行
    for (const auto& state : dfaStates) {
        std::cout << "| " << std::setw(4) << state.id << " |";
        
        // 打印ACTION部分
        for (char term : terminals) {
            if (actionTable[state.id].find(term) != actionTable[state.id].end()) {
                std::string action = actionTable[state.id][term];
                std::cout << " " << std::setw(8) << action << " |";
            } else {
                std::cout << "          |";
            }
        }
        
        // 打印GOTO部分
        for (char nonTerm : nonTerminals) {
            if (nonTerm != 'X' && gotoTable[state.id].find(nonTerm) != gotoTable[state.id].end()) {
                int gotoState = gotoTable[state.id][nonTerm];
                std::cout << " " << std::setw(8) << gotoState << " |";
            } else if (nonTerm != 'X') {
                std::cout << "          |";
            }
        }
        std::cout << std::endl;
    }
    
    std::cout << "+------+";
    for (char term : terminals) {
        std::cout << "----------+";
    }
    for (char nonTerm : nonTerminals) {
        if (nonTerm != 'X') {
            std::cout << "----------+";
        }
    }
    std::cout << std::endl;
    
    // 打印产生式编号对照表
    std::cout << "\n产生式编号对照表：" << std::endl;
    for (int i = 0; i < grammar.size(); i++) {
        std::cout << i << ": " << grammar[i].leftSide << " → ";
        if (grammar[i].rightSide == "@") {
            std::cout << "ε";
        } else {
            std::cout << grammar[i].rightSide;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// 将LR(1)分析表写入Markdown文件
void writeLR1TableToMarkdown(const std::vector<DFAState>& dfaStates, 
                           const std::vector<Production>& grammar,
                           const std::string& filename) {
    // 生成分析表
    auto [actionTable, gotoTable] = generateLR1Table(dfaStates, grammar);
    
    // 收集所有终结符和非终结符
    std::set<char> terminals;
    std::set<char> nonTerminals;

    for (const auto& prod : grammar) {
        nonTerminals.insert(prod.leftSide);
        for (char c : prod.rightSide) {
            if (isTerminal(c)) {
                terminals.insert(c);
            }
        }
    }
    terminals.insert('#'); // 添加结束符
    
    // 创建或打开markdown文件
    std::ofstream mdFile(filename);
    if (!mdFile.is_open()) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }
    
    // 写入标题
    mdFile << "# LR(1)分析表\n\n";
    
    // 表格头部
    mdFile << "| 状态 |";
    // ACTION列
    for (char term : terminals) {
        mdFile << " " << term << " |";
    }
    // GOTO列
    for (char nonTerm : nonTerminals) {
        if (nonTerm != 'X') { // 不显示增广文法的起始符号
            mdFile << " " << nonTerm << " |";
        }
    }
    mdFile << "\n";
    
    // 表格分隔行
    mdFile << "| ---- |";
    for (size_t i = 0; i < terminals.size(); i++) {
        mdFile << " ---- |";
    }
    for (char nonTerm : nonTerminals) {
        if (nonTerm != 'X') {
            mdFile << " ---- |";
        }
    }
    mdFile << "\n";
    
    // 填充表格内容
    for (const auto& state : dfaStates) {
        mdFile << "| " << state.id << " |";
        
        // ACTION部分
        for (char term : terminals) {
            if (actionTable[state.id].find(term) != actionTable[state.id].end()) {
                std::string action = actionTable[state.id][term];
                mdFile << " " << action << " |";
            } else {
                mdFile << "  |";
            }
        }
        
        // GOTO部分
        for (char nonTerm : nonTerminals) {
            if (nonTerm != 'X' && gotoTable[state.id].find(nonTerm) != gotoTable[state.id].end()) {
                int gotoState = gotoTable[state.id][nonTerm];
                mdFile << " " << gotoState << " |";
            } else if (nonTerm != 'X') {
                mdFile << "  |";
            }
        }
        mdFile << "\n";
    }
    
    // 写入产生式编号对照表
    mdFile << "\n## 产生式编号对照表\n\n";
    for (int i = 0; i < grammar.size(); i++) {
        mdFile << "- " << i << ": " << grammar[i].leftSide << " → ";
        if (grammar[i].rightSide == "@") {
            mdFile << "ε";
        } else {
            mdFile << grammar[i].rightSide;
        }
        mdFile << "\n";
    }
    
    mdFile.close();
    std::cout << "LR(1)分析表已保存到: " << filename << std::endl;
}

// LR(1)分析函数，模拟LR(1)分析过程并打印分析表
bool analyzeLR1String(
    const std::string& input,
    const std::vector<Production>& grammar,
    const std::vector<DFAState>& dfaStates,
    std::ofstream* mdOutputFile = nullptr)
{
    // 生成LR(1)分析表
    auto [actionTable, gotoTable] = generateLR1Table(dfaStates, grammar);
    
    // 准备输入串
    std::string inputString = input + '#'; // 添加结束符
    std::cout << "分析串：" << inputString << std::endl << std::endl;
    
    // 初始化分析栈和符号栈
    std::vector<int> stateStack = {0}; // 初始状态为0
    std::vector<char> symbolStack = {'#'}; // 初始符号为#
    
    // 打印表头
    std::cout << "| 序号 | 分析栈 | 输入栈 | 动作 |\n";
    std::cout << "| ---- | ------- | ------- | ---- |\n";
    
    if (mdOutputFile && mdOutputFile->is_open()) {
        *mdOutputFile << "# LR(1)分析过程\n\n";
        *mdOutputFile << "| 序号 | 分析栈 | 输入栈 | 动作 |\n";
        *mdOutputFile << "| ---- | ------- | ------- | ---- |\n";
    }
    
    int step = 1;
    while (true) {
        // 获取当前状态和输入符号
        int currentState = stateStack.back();
        char currentInput = inputString[0];
        
        // 构建分析栈字符串 - 修改此部分以显示初始状态
        std::string stackStr = "#0"; // 始终显示初始符号#和状态0
        for (size_t i = 1; i < stateStack.size(); i++) {
            stackStr += symbolStack[i] + std::to_string(stateStack[i]);
        }
        
        // 查找动作
        std::string action;
        if (actionTable[currentState].find(currentInput) != actionTable[currentState].end()) {
            action = actionTable[currentState][currentInput];
        } else {
            // 错误：没有对应的动作
            std::cout << "| " << step << " | " << stackStr << " | " << inputString << " | 错误 |\n";
            if (mdOutputFile && mdOutputFile->is_open()) {
                *mdOutputFile << "| " << step << " | " << stackStr << " | " << inputString << " | 错误 |\n";
            }
            std::cout << "\n分析失败！没有找到对应的动作。\n";
            return false;
        }
        
        // 打印当前步骤
        std::cout << "| " << step << " | " << stackStr << " | " << inputString << " | " << action << " |\n";
        if (mdOutputFile && mdOutputFile->is_open()) {
            *mdOutputFile << "| " << step << " | " << stackStr << " | " << inputString << " | " << action << " |\n";
        }
        
        // 执行动作
        if (action == "ACC") {
            // 接受，分析成功
            std::cout << "\n分析成功！输入串符合文法。\n";
            if (mdOutputFile && mdOutputFile->is_open()) {
                *mdOutputFile << "\n**分析结果：成功**\n";
            }
            return true;
        } else if (action[0] == 'S') {
            // 移进操作
            int nextState = std::stoi(action.substr(1));
            stateStack.push_back(nextState);
            symbolStack.push_back(currentInput);
            inputString.erase(0, 1); // 移除已处理的输入符号
        } else if (action[0] == 'R') {
            // 规约操作
            int prodIndex = std::stoi(action.substr(1));
            const Production& prod = grammar[prodIndex];
            
            // 弹出产生式右部长度的符号和状态
            size_t rightSize = (prod.rightSide == "@") ? 0 : prod.rightSide.size();
            for (size_t i = 0; i < rightSize; i++) {
                stateStack.pop_back();
                symbolStack.pop_back();
            }
            
            // 压入产生式左部
            symbolStack.push_back(prod.leftSide);
            
            // 根据GOTO表确定下一个状态
            int nextState = gotoTable[stateStack.back()][prod.leftSide];
            stateStack.push_back(nextState);
        } else {
            // 未知动作
            std::cout << "| " << step << " | " << stackStr << " | " << inputString << " | 未知动作 |\n";
            if (mdOutputFile && mdOutputFile->is_open()) {
                *mdOutputFile << "| " << step << " | " << stackStr << " | " << inputString << " | 未知动作 |\n";
            }
            std::cout << "\n分析失败！遇到未知动作。\n";
            return false;
        }
        
        step++;
        
        // 如果分析栈过大或者进入无限循环，中止分析
        if (step > 1000) {
            std::cout << "| - | - | - | 分析步骤过多，可能存在循环 |\n";
            if (mdOutputFile && mdOutputFile->is_open()) {
                *mdOutputFile << "| - | - | - | 分析步骤过多，可能存在循环 |\n";
            }
            return false;
        }
    }
}