#include<iostream>
#include<vector>
#include<map>
#include<set>
#include<string>
#include<fstream>
#include<cstdlib>

// 直接引入functions.cpp
#include "functions.cpp"

std::vector<Production> extendGrammar(const std::vector<Production>& grammar) {
    std::vector<Production> extendedGrammar = grammar;
    char newStartSymbol = 'X'; // 假设原始文法的起始符号是'S'
    
    // 添加新的起始符号
    extendedGrammar.insert(extendedGrammar.begin(), { newStartSymbol, std::string(1, grammar[0].leftSide) });
    
    return extendedGrammar;
}

// 创建文法产生式
std::vector<Production> createGrammar(int grammarNumber) {
    std::vector<Production> grammar;

    switch (grammarNumber) {
    case 1:
        // 文法1: S→CC, C→cC|d (示例输入: cc)
        grammar = {
            {'S', "CC"},
            {'C', "cC"},
            {'C', "d"},
        };
        break;
    case 2:
        // 文法2: S→L=S|R, L→aLR|b, R→a (示例输入: aba=b=a)
        grammar = {
            {'S', "L=S"},
            {'S', "R"},
            {'L', "aLR"},
            {'L', "b"},
            {'R', "a"},
        };
        break;
    case 3:
        // 文法3: S→aLb|a, L→aR, R→LR|b (示例输入: aaabbb)
        grammar = {
            {'S', "aLb"},
            {'S', "a"},
            {'L', "aR"},
            {'R', "LR"},
            {'R', "b"},
        };
        break;
    case 4:
        // 文法4: S→L=LR|R, L→aR|b, R→L (示例输入: b=abab)
        grammar = {
            {'S', "L=LR"},
            {'S', "R"},
            {'L', "aR"},
            {'L', "b"},
            {'R', "L"},
        };
        break;
    case 5:
        // 文法5: S→(L)|a, L→L,S|S (示例输入: ((a),a))
        grammar = {
            {'S', "(L)"},
            {'S', "a"},
            {'L', "L,S"},
            {'L', "S"},
        };
        break;
    case 6:
        // 文法6: S→(S)S|ε (示例输入: ()())
        grammar = {
            {'S', "(S)S"},
            {'S', "@"},
        };
        break;
    default:
        // 默认使用文法1
        grammar = {
            {'S', "CC"},
            {'C', "cC"},
            {'C', "d"},
        };
        break;
    }
    return grammar;
}

// std::string inputString = "b=abab";  // 根据当前文法的适当输入串
// std::string inputString = "((a),a)";  // 根据当前文法的适当输入串
std::string inputString = "cc";  // 根据当前文法的适当输入串


int main() {
    std::cout << "=====================================" << std::endl;

    // 创建原始文法
    std::vector<Production> grammar = createGrammar(1);
    std::cout << "原始文法：" << std::endl;
    printGrammar(grammar);

    // 拓广文法
    std::vector<Production> extendedGrammar = extendGrammar(grammar);

    // 打印编号文法产生式
    printNumGrammar(extendedGrammar);

    // 计算First集
    std::map<char, std::set<char>> first = computeFirst(extendedGrammar);
    printSets(first, "First");

    // 绘制 DFA
    std::vector<DFAState> dfaStates = createDFA(extendedGrammar, first);
    
    // 导出DFA到DOT文件并生成图像 - 使用相对路径
    std::string dotFilePath = "outcome/dfa_grammar1.dot";
    exportDFAtoDOT(dfaStates, dotFilePath);
    
    // 使用dot命令生成PNG图像
    std::string pngFilePath = "outcome/dfa_grammar1.png";
    std::string command = "dot -Tpng " + dotFilePath + " -o " + pngFilePath;
    
    // 执行系统命令
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "成功生成DFA图像: " << pngFilePath << std::endl;
    } else {
        std::cerr << "生成图像失败，请确保已安装GraphViz并可用" << std::endl;
    }

    // 输出LR(1)分析表
    std::cout << "\n生成LR(1)分析表：" << std::endl;
    printLR1Table(dfaStates, extendedGrammar);
    
    // 将LR(1)分析表写入Markdown文件
    std::string tableMarkdownFile = "outcome/lr1_table_grammar1.md";
    writeLR1TableToMarkdown(dfaStates, extendedGrammar, tableMarkdownFile);

    // LR(1)分析栈
    std::cout << "\nLR(1)分析过程：" << std::endl;
    std::string lr1AnalysisFile = "outcome/lr1_analysis_grammar1.md";
    std::ofstream lr1MdFile(lr1AnalysisFile);
    if (lr1MdFile.is_open()) {
        bool success = analyzeLR1String(inputString, extendedGrammar, dfaStates, &lr1MdFile);
        lr1MdFile.close();
        std::cout << "LR(1)分析过程已保存到: " << lr1AnalysisFile << std::endl;
    }
    
    std::cout << "=====================================" << std::endl;

    return 0;
}