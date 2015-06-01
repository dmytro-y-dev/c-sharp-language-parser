#include <iostream>
#include <fstream>
#include <vector>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "FileSystemOperations.h"
#include "SymbolsTable.h"
#include "IntermediateCodeParseTree.h"

using namespace std;

SymbolsTable symbolsTable;

struct Triad
{
    int id;
    string operation;
    string first;
    string second;
};

VariablesBlock* GetBestCorrespondingBlock(VariablesBlock* block, int currentPosition)
{
    if (block->closingBracketPosition < currentPosition) {
        return nullptr;
    }

    if (block->openingBracketPosition > currentPosition) {
        return nullptr;
    }

    VariablesBlock *goodChild = block;

    for (int i = 0, iend = block->childrenBlocks.size(); i != iend; ++i) {
        VariablesBlock* child = GetBestCorrespondingBlock(&block->childrenBlocks[i], currentPosition);

        if (child) {
            goodChild = child;

            break;
        }
    }

    return goodChild;
}

SymbolVariable* GetVariable(const string& name, int methodId, int currentPosition, map<int, Triad>& triads)
{
    string methodName = triads[methodId].second;
    string className = triads[stoi(triads[methodId].first.substr(1))].first;

    for (auto definedClass = symbolsTable.begin(), endClass = symbolsTable.end(); definedClass != endClass; ++definedClass) {
        if (definedClass->second.name == className) {
            VariablesBlock* block = GetBestCorrespondingBlock(&definedClass->second.functions[methodName].blocks[0], currentPosition);
            SymbolVariable* variable = GetVariableByName(name, block, currentPosition);

            return variable;
        }
    }

    return nullptr;
}

void DisplayIntermediateTreeNodeAsText(ostream& out, const IntermediateCodeParseTreeNode& node, const string& prefix)
{
    if (node.token.type != Lexem::LEXEM_UNKNOWN) {
        out << prefix << node.token.value << " <" << node.intermediateRule << ">" << std::endl;
    } else {
        out << prefix << "<" << node.intermediateRule << ">" << std::endl;
    }

    for each (auto nextNode in node.children) {
        DisplayIntermediateTreeNodeAsText(out, nextNode, prefix + "    ");
    }
}

void DisplayIntermediateTreeAsText(ostream& out, const IntermediateCodeParseTreeNode& root)
{
    DisplayIntermediateTreeNodeAsText(out, root, "");
}

int GetNextTriadId()
{
    static int id = 0;
    ++id;

    return id;
}

string BuildQualifiedName(const IntermediateCodeParseTreeNode& root)
{
    string qualifiedName;

    for each (auto node in root.children) {
        if (!qualifiedName.empty()) {
            qualifiedName += ".";
        }

        qualifiedName += node.token.value;
    }

    return qualifiedName;
}

void PopTriadsStack(vector<string>& stack, map<int, Triad>& result)
{
    bool popOnceAgain = false;

    Triad newTriad;
    newTriad.id = GetNextTriadId();

    if (stack.back() == ")") {
        // Then ... (a + b) construction

        stack.pop_back(); // )
        newTriad.first = stack.back();
        stack.pop_back();
        newTriad.operation = stack.back();
        stack.pop_back();
        newTriad.second = stack.back();
        stack.pop_back();

        /*if (stack[stack.size() - 2] == "(") {
                    string singleValue = stack.back();
                    stack.pop_back();
                    stack.pop_back();
                    stack.push_back(singleValue);
                    } else*/
        if (stack.back() == "(") {
            stack.pop_back(); // (
            stack.push_back("@" + to_string(newTriad.id));
        } else {
            stack.push_back("@" + to_string(newTriad.id));
            stack.push_back(")");

            popOnceAgain = true;
        }
    } else {
        // Else ... a + b construction

        newTriad.first = stack.back();
        stack.pop_back();
        newTriad.operation = stack.back();
        stack.pop_back();
        newTriad.second = stack.back();
        stack.pop_back();

        stack.push_back("@" + to_string(newTriad.id));
    }

    result[newTriad.id] = newTriad;

    if (popOnceAgain) {
        PopTriadsStack(stack, result);
    }
}

void GenerateTriadsExpression(const IntermediateCodeParseTreeNode& root, int methodId, int currentPosition, string &exprResultId, map<int, Triad>& result)
{
    vector<string> stack;

    for each (auto node in root.children) {
        if (node.intermediateRule == "ic-expr-qualified-name") {
            string varName = BuildQualifiedName(node);
            stack.push_back(varName);

            continue;
        }

        if (node.intermediateRule == "ic-expr-token") {
            if (node.token.type == Lexem::LEXEM_TOKEN_IDENTIFIER) {
                string varName = node.token.value;

                auto variable = GetVariable(varName, methodId, currentPosition, result);

                if (!variable) {
                    throw "Undefined variable " + varName;
                }

                stack.push_back("@" + to_string(variable->triadId));
            } else {
                stack.push_back(node.token.value);
            }

            continue;
        }

        if (node.intermediateRule == "ic-expr-operator") {
            for (auto stack_item = stack.rbegin(), stack_end = stack.rend(); stack_item != stack_end; ++stack_item) {
                if (*stack_item == "+" || *stack_item == "-") {
                    if (node.token.value != "*" && node.token.value != "/") {
                        PopTriadsStack(stack, result);
                    }

                    break;
                } else if (*stack_item == "*" || *stack_item == "/") {
                    PopTriadsStack(stack, result);

                    break;
                } else if (*stack_item == "(") {

                    break;
                }
            }
            
            stack.push_back(node.token.value);

            continue;
        }

        if (node.intermediateRule == "ic-expr-priority-operator") {
            if (node.token.value == "(") {
                stack.push_back("(");
            }

            if (node.token.value == ")") {
                stack.push_back(node.token.value);

                PopTriadsStack(stack, result);
            }

            continue;
        }
    }

    while (stack.size() > 1) {
        PopTriadsStack(stack, result);
    }

    if (!stack.empty()) {
        exprResultId = stack.back();
    }
}

void GenerateTriadsMethodBody(const IntermediateCodeParseTreeNode& root, int methodId, map<int, Triad>& result)
{
    for each (auto node in root.children) {
        if (node.intermediateRule == "ic-variable-declaration") {
            Triad newVariable;
            newVariable.id = GetNextTriadId();
            newVariable.operation = "new-variable";
            newVariable.first = node.token.value;
            newVariable.second = node.children[0].token.value;

            if (node.children.size() > 1){ 
                Triad valueAssignment;
                valueAssignment.id = GetNextTriadId();
                valueAssignment.operation = "=";
                valueAssignment.first = "@" + std::to_string(newVariable.id);

                GenerateTriadsExpression(node.children[1], methodId, node.positionInCode, valueAssignment.second, result);

                result[valueAssignment.id] = valueAssignment;
            }

            result[newVariable.id] = newVariable;

            auto variable = GetVariable(newVariable.first, methodId, node.positionInCode, result);

            if (!variable) {
                throw "Unexpected exception in GenerateTriadsMethod";
            }

            variable->triadId = newVariable.id;

            continue;
        }

        if (node.intermediateRule == "ic-statement-expr") {
            if (node.children[0].intermediateRule == "ic-statement-expr-qualified-name") {
                string lhsName = BuildQualifiedName(node.children[0]);

                Triad newStatement;
                newStatement.id = GetNextTriadId();

                if (lhsName.find('.') == string::npos) {
                    auto variable = GetVariable(lhsName, methodId, node.children[0].positionInCode, result);

                    if (!variable) {
                        throw "Undefined variable " + lhsName;
                    }

                    newStatement.first = "@" + to_string(variable->triadId);
                } else {
                    newStatement.first = lhsName;
                }

                if (node.children[1].intermediateRule == "ic-statement-expr-parameters") {
                    newStatement.operation = node.children[1].children[0].token.value;

                    GenerateTriadsExpression(node.children[1].children[1], methodId, node.children[1].children[1].positionInCode, newStatement.second, result);
                } else {
                    newStatement.operation = node.children[1].token.value;
                }

                result[newStatement.id] = newStatement;
            } else if (node.children[0].intermediateRule == "ic-statement-expr-increment-decrement") {
                string lhsName = BuildQualifiedName(node.children[1]);

                Triad newStatement;
                newStatement.id = GetNextTriadId();

                if (lhsName.find('.') == string::npos) {
                    auto variable = GetVariable(lhsName, methodId, node.children[1].positionInCode, result);

                    if (!variable) {
                        throw "Undefined variable " + lhsName;
                    }

                    newStatement.first = "@" + to_string(variable->triadId);
                } else {
                    newStatement.first = lhsName;
                }

                newStatement.operation = node.children[0].token.value;

                result[newStatement.id] = newStatement;
            }

            continue;
        }

        if (node.intermediateRule == "ic-while") {
            string exprResultId;

            GenerateTriadsExpression(node.children[0], methodId, node.children[0].positionInCode, exprResultId, result);

            Triad newWhile;
            newWhile.id = GetNextTriadId();
            newWhile.operation = "while";
            newWhile.first = exprResultId;

            GenerateTriadsMethodBody(node.children[1], methodId, result);
            newWhile.second = std::to_string(result.size() + 1 - newWhile.id);

            result[newWhile.id] = newWhile;

            continue;
        }
    }
}

void GenerateTriadsMethod(const IntermediateCodeParseTreeNode& root, int classId, map<int, Triad>& result)
{
    Triad triadMethod;
    triadMethod.id = GetNextTriadId();
    triadMethod.operation = "new-method";
    triadMethod.first = "@" + std::to_string(classId);
    triadMethod.second = root.token.value;

    result[triadMethod.id] = triadMethod;

    for (int i = 2, iend = root.children.size() - 1; i < iend; i += 2) {
        Triad newVariable;
        newVariable.id = GetNextTriadId();
        newVariable.operation = "reference-variable";
        newVariable.first = root.children[i + 1].token.value;
        newVariable.second = root.children[i].token.value;

        result[newVariable.id] = newVariable;

        auto variable = GetVariable(newVariable.first, triadMethod.id, root.children[i + 1].positionInCode, result);

        if (!variable) {
            throw "Unexpected exception in GenerateTriadsMethod";
        }

        variable->triadId = newVariable.id;
    }

    GenerateTriadsMethodBody(root.children[root.children.size() - 1], triadMethod.id, result);
}

void GenerateTriadsClass(const IntermediateCodeParseTreeNode& root, map<int, Triad>& result)
{
    Triad triadClass;
    triadClass.id = GetNextTriadId();
    triadClass.operation = "new-class";
    triadClass.first = root.token.value;

    result[triadClass.id] = triadClass;

    for (auto node = root.children.begin(), nodeEnd = root.children.end(); node != nodeEnd; ++node) {
        GenerateTriadsMethod(*node, triadClass.id, result);
    }
}

void GenerateTriads(const IntermediateCodeParseTreeNode& root, map<int, Triad>& result)
{
    for each (auto node in root.children) {
        GenerateTriadsClass(node, result);
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "Error: path to C# source file was not specified." << endl;

        return 1;
    }

    // Lets analyze C# source code file
    // We start with loading source code from the file which user specified in command line

    string originalContentOfCSharpFile = LoadFileContentFromDiskToMemory(argv[1]);

    // Do lexical analysis

    vector<Lexem> lexems;
    if (!DoLexicalAnalysis(originalContentOfCSharpFile, lexems)) {
        return 1;
    }

    vector<Lexem> tokens;
    ExtractTokensFromLexemsList(lexems, tokens);

    // Write lexical analysis results to .xls file

    WriteLexemsToXLS(lexems, "lexems.xls");

    // Do syntax analysis

    vector<SyntaxRule> syntaxRules;
    if (!LoadSyntaxRules("../grammars/syntax-grammar.txt", syntaxRules) || syntaxRules.empty()) {
        cout << "Error during loading syntax rules base" << endl;

        return 2;
    }

    ParseTree syntaxTree = DoSyntaxAnalysis(tokens, syntaxRules, syntaxRules[0].GetName());

    // Write syntax analysis results to text file

    ofstream syntaxResultsOut("syntax-analysis-results.txt", ios_base::out);

    if (syntaxTree.Empty()) {
        syntaxResultsOut << "Error: Unable to spawn program's text." << endl << endl;
    } else {
        syntaxResultsOut << "File has been parsed successfully." << endl << endl;
    }

    DisplaySyntaxParseTreeAsText(syntaxResultsOut, syntaxTree);

    try {
        GenerateSymbolsTable(syntaxTree, tokens, symbolsTable);
        WriteSymbolsTableToXLS(symbolsTable, "symbols.xls");
    } catch (SymbolsTableGenerationError& error) {
        cout << error << endl;

        return 3;
    }

    syntaxResultsOut << endl;

    IntermediateCodeParseTreeNode intermediateCodeParseTreeRoot;

    try {
        BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxTree.GetRoot(), tokens, intermediateCodeParseTreeRoot);

        DisplayIntermediateTreeAsText(syntaxResultsOut, intermediateCodeParseTreeRoot);
        syntaxResultsOut << endl;

        map<int, Triad> triads;
        GenerateTriads(intermediateCodeParseTreeRoot, triads);

        for each (auto triad_pair in triads) {
            Triad triad = triad_pair.second;

            syntaxResultsOut << "(" << triad.id << "; " << triad.operation << "; " << triad.first << "; " << triad.second << ")" << endl;
        }
    } catch (string& error) {
        cout << error << endl;

        return 4;
    }

    return 0;
}