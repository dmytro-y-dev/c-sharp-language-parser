#include <vector>

#include "IntermediateCodeTriads.h"

using std::vector;
using std::to_string;

static string BuildQualifiedName(const IntermediateCodeParseTreeNode& qualifiedIdRoot)
{
    string qualifiedName;

    for each (auto node in qualifiedIdRoot.children) {
        if (!qualifiedName.empty()) {
            qualifiedName += ".";
        }

        qualifiedName += node.token.value;
    }

    return qualifiedName;
}

static int GetNextTriadId(const map<int, Triad>& triads)
{
    if (triads.empty()) {
        return 1;
    } else {
        return triads.rbegin()->first + 1; /* Get last triad id and add 1 to it. Then return. */
    }
}

SymbolVariable* FindVariable(const string& name, int methodId, int currentPosition, SymbolsTable& symbolsTable, map<int, Triad>& triads)
{
    string methodName = triads[methodId].second;
    string className = triads[stoi(triads[methodId].first.substr(1))].first;

    for (auto definedClass = symbolsTable.begin(), endClass = symbolsTable.end(); definedClass != endClass; ++definedClass) {
        if (definedClass->second.name == className) {
            CodeBlock* block = FindBestCorrespondingBlock(&(definedClass->second.functions[methodName].blocks[0]), currentPosition);
            SymbolVariable* variable = FindVariable(name, block, currentPosition);

            return variable;
        }
    }

    return nullptr;
}

static void PopTriadsStack(vector<string>& stack, map<int, Triad>& result)
{
    bool popOnceAgain = false;

    Triad newTriad;
    newTriad.id = GetNextTriadId(result);

    if (stack.back() == ")") {
        // Then ... (a + b) construction

        stack.pop_back();
        newTriad.first = stack.back(); // b
        stack.pop_back(); 
        newTriad.operation = stack.back(); // +
        stack.pop_back();
        newTriad.second = stack.back(); // a
        stack.pop_back();

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

static void GenerateTriadsExpression(const IntermediateCodeParseTreeNode& root, int methodId, int currentPosition, string &exprResultId, SymbolsTable& symbolsTable, map<int, Triad>& result)
{
    vector<string> stack;

    for each (auto node in root.children) {
        if (node.intermediateRuleName == "ic-expr-qualified-name") {
            string varName = BuildQualifiedName(node);
            stack.push_back(varName);

            continue;
        }

        if (node.intermediateRuleName == "ic-expr-token") {
            if (node.token.type == Lexem::LEXEM_TOKEN_IDENTIFIER) {
                string varName = node.token.value;

                auto variable = FindVariable(varName, methodId, currentPosition, symbolsTable, result);

                if (!variable) {
                    throw "Undefined variable " + varName;
                }

                stack.push_back("@" + to_string(variable->triadId));
            } else {
                stack.push_back(node.token.value);
            }

            continue;
        }

        if (node.intermediateRuleName == "ic-expr-operator") {
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

        if (node.intermediateRuleName == "ic-expr-priority-operator") {
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

static void GenerateTriadsMethodBody(const IntermediateCodeParseTreeNode& root, int methodId, SymbolsTable& symbolsTable, map<int, Triad>& result)
{
    for each (auto node in root.children) {
        if (node.intermediateRuleName == "ic-variable-declaration") {
            Triad newVariable;
            newVariable.id = GetNextTriadId(result);
            newVariable.operation = "new-variable";
            newVariable.first = node.token.value;
            newVariable.second = node.children[0].token.value;

            if (node.children.size() > 1){
                Triad valueAssignment;
                valueAssignment.id = GetNextTriadId(result);
                valueAssignment.operation = "=";
                valueAssignment.first = "@" + std::to_string(newVariable.id);

                GenerateTriadsExpression(node.children[1], methodId, node.positionInCode, valueAssignment.second, symbolsTable, result);

                result[valueAssignment.id] = valueAssignment;
            }

            result[newVariable.id] = newVariable;

            auto variable = FindVariable(newVariable.first, methodId, node.positionInCode, symbolsTable, result);

            if (!variable) {
                throw "Unexpected exception in GenerateTriadsMethod";
            }

            variable->triadId = newVariable.id;

            continue;
        }

        if (node.intermediateRuleName == "ic-statement-expr") {
            if (node.children[0].intermediateRuleName == "ic-statement-expr-qualified-name") {
                string lhsName = BuildQualifiedName(node.children[0]);

                Triad newStatement;
                newStatement.id = GetNextTriadId(result);

                if (lhsName.find('.') == string::npos) {
                    auto variable = FindVariable(lhsName, methodId, node.children[0].positionInCode, symbolsTable, result);

                    if (!variable) {
                        throw "Undefined variable " + lhsName;
                    }

                    newStatement.first = "@" + to_string(variable->triadId);
                } else {
                    newStatement.first = lhsName;
                }

                if (node.children[1].intermediateRuleName == "ic-statement-expr-parameters") {
                    newStatement.operation = node.children[1].children[0].token.value;

                    GenerateTriadsExpression(node.children[1].children[1], methodId, node.children[1].children[1].positionInCode, newStatement.second, symbolsTable, result);
                } else {
                    newStatement.operation = node.children[1].token.value;
                }

                result[newStatement.id] = newStatement;
            } else if (node.children[0].intermediateRuleName == "ic-statement-expr-increment-decrement") {
                string lhsName = BuildQualifiedName(node.children[1]);

                Triad newStatement;
                newStatement.id = GetNextTriadId(result);

                if (lhsName.find('.') == string::npos) {
                    auto variable = FindVariable(lhsName, methodId, node.children[1].positionInCode, symbolsTable, result);

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

        if (node.intermediateRuleName == "ic-while") {
            string exprResultId;

            GenerateTriadsExpression(node.children[0], methodId, node.children[0].positionInCode, exprResultId, symbolsTable, result);

            Triad newWhile;
            newWhile.id = GetNextTriadId(result);
            newWhile.operation = "while";
            newWhile.first = exprResultId;

            GenerateTriadsMethodBody(node.children[1], methodId, symbolsTable, result);
            newWhile.second = std::to_string(result.size() + 1 - newWhile.id);

            result[newWhile.id] = newWhile;

            continue;
        }
    }
}

static void GenerateTriadsMethod(const IntermediateCodeParseTreeNode& root, int classId, SymbolsTable& symbolsTable, map<int, Triad>& result)
{
    Triad triadMethod;
    triadMethod.id = GetNextTriadId(result);
    triadMethod.operation = "new-method";
    triadMethod.first = "@" + std::to_string(classId);
    triadMethod.second = root.token.value;

    result[triadMethod.id] = triadMethod;

    for (int i = 2, iend = root.children.size() - 1; i < iend; i += 2) {
        Triad newVariable;
        newVariable.id = GetNextTriadId(result);
        newVariable.operation = "reference-variable";
        newVariable.first = root.children[i + 1].token.value;
        newVariable.second = root.children[i].token.value;

        result[newVariable.id] = newVariable;

        auto variable = FindVariable(newVariable.first, triadMethod.id, root.children[i + 1].positionInCode, symbolsTable, result);

        if (!variable) {
            throw "Unexpected exception in GenerateTriadsMethod";
        }

        variable->triadId = newVariable.id;
    }

    GenerateTriadsMethodBody(root.children[root.children.size() - 1], triadMethod.id, symbolsTable, result);
}

static void GenerateTriadsClass(const IntermediateCodeParseTreeNode& root, SymbolsTable& symbolsTable, map<int, Triad>& result)
{
    Triad triadClass;
    triadClass.id = GetNextTriadId(result);
    triadClass.operation = "new-class";
    triadClass.first = root.token.value;

    result[triadClass.id] = triadClass;

    for (auto node = root.children.begin(), nodeEnd = root.children.end(); node != nodeEnd; ++node) {
        GenerateTriadsMethod(*node, triadClass.id, symbolsTable, result);
    }
}

void BuildTriadsTable(const IntermediateCodeParseTreeNode& intermediateTreeRoot, SymbolsTable& symbolsTable, map<int, Triad>& triadsTable)
{
    for each (auto node in intermediateTreeRoot.children) {
        GenerateTriadsClass(node, symbolsTable, triadsTable);
    }
}