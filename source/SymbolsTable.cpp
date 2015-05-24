#include "SymbolsTable.h"

#include <algorithm>
#include <ExcelFormat/ExcelFormat.h>
#include <memory>
#include <utility>

using std::auto_ptr;
using namespace ExcelFormat;

struct PositionInSymbolsTable
{
    int classIndex;
    string functionName;

    VariablesBlock* currentBlock;
};

SymbolVariable* GetVariableByName(const string& name, VariablesBlock* startingBlock, int currentPositionInCode)
{
    if (!startingBlock) {
        return nullptr;
    }

    map<string, SymbolVariable>::iterator ptrToVariable = startingBlock->variables.find(name);

    if (ptrToVariable != startingBlock->variables.end()
            && ptrToVariable->second.variableVisibilityStart <= currentPositionInCode
            && ptrToVariable->second.variableVisibilityEnd >= currentPositionInCode) {
        return &(ptrToVariable->second);
    }

    return GetVariableByName(name, startingBlock->parentBlock, currentPositionInCode);
}

bool TryClassNonterminals(const ParseTree::Node* syntaxTreeNode, const vector<Lexem>& tokens, PositionInSymbolsTable& newSymbolPosition, SymbolsTable& symbolsTable)
{
    if (syntaxTreeNode->value.name == "<class-part1>") {
        newSymbolPosition.classIndex = symbolsTable.size();

        SymbolClass newSymbol;
        newSymbol.name = tokens[syntaxTreeNode->left->value.j].value;

        for (auto classSymbol : symbolsTable)  {
            if (classSymbol.name == newSymbol.name) {
                throw SymbolsTableGenerationError("Error: Class name `" + newSymbol.name + "` is used twice in current unit");
            }
        }

        symbolsTable.push_back(newSymbol);

        return true;
    }

    if (syntaxTreeNode->value.name == "<one-more-class>") {
        newSymbolPosition.classIndex = -1;
    }

    return false;
}

bool TryBlockNonterminals(const ParseTree::Node* syntaxTreeNode, const vector<Lexem>& tokens, PositionInSymbolsTable& newSymbolPosition, SymbolsTable& symbolsTable)
{
    if (syntaxTreeNode && syntaxTreeNode->value.name == "<closing-bracket>") {
        if (newSymbolPosition.currentBlock && syntaxTreeNode->value.j == newSymbolPosition.currentBlock->closingBracketPosition) {
            newSymbolPosition.currentBlock = newSymbolPosition.currentBlock->parentBlock;

            return true;
        }

        return false;
    }

    if (!syntaxTreeNode || syntaxTreeNode->value.name != "<maybe-empty-block-part1>") {
        return false;
    }

    if (newSymbolPosition.classIndex == -1 || newSymbolPosition.functionName.empty()) {
        throw SymbolsTableGenerationError("Unexpected situation: Block outside of class");
    }

    int closingBracketPosition = syntaxTreeNode->value.i + syntaxTreeNode->value.j;
    SymbolFunction& currentFunction = symbolsTable[newSymbolPosition.classIndex].functions[newSymbolPosition.functionName];

    if (newSymbolPosition.currentBlock == nullptr) {
        VariablesBlock newBlock;
        newBlock.parentBlock = nullptr;

        currentFunction.blocks.push_back(newBlock);
        newSymbolPosition.currentBlock = &currentFunction.blocks[0];
    } else {
        if (newSymbolPosition.currentBlock->closingBracketPosition == closingBracketPosition) {
            return false;
        }

        VariablesBlock newBlock;
        newBlock.parentBlock = newSymbolPosition.currentBlock;

        newSymbolPosition.currentBlock->childrenBlocks.push_back(newBlock);
        newSymbolPosition.currentBlock = &newSymbolPosition.currentBlock->childrenBlocks[newSymbolPosition.currentBlock->childrenBlocks.size() - 1];
    }

    newSymbolPosition.currentBlock->closingBracketPosition = closingBracketPosition;

    return true;
}

bool TryVariablesNonterminals(const ParseTree::Node* syntaxTreeNode, const vector<Lexem>& tokens, PositionInSymbolsTable& newSymbolPosition, SymbolsTable& symbolsTable)
{
    if (!syntaxTreeNode || syntaxTreeNode->value.name != "<local-variable-declaration>") {
        return false;
    }

    if (newSymbolPosition.classIndex == -1 || newSymbolPosition.functionName.empty() || newSymbolPosition.currentBlock == nullptr) {
        throw SymbolsTableGenerationError("Unexpected situation: Variable defined outside of class");
    }

    SymbolVariable newVariable;
    newVariable.tokenValueStart = syntaxTreeNode->value.j + 2;
    newVariable.tokenValueEnd = syntaxTreeNode->value.j + syntaxTreeNode->value.i + 1;
    newVariable.type = tokens[syntaxTreeNode->left->value.j].value;
    newVariable.name = tokens[syntaxTreeNode->left->value.j + 1].value;
    newVariable.variableVisibilityStart = newVariable.tokenValueEnd + 1;
    newVariable.variableVisibilityEnd = newSymbolPosition.currentBlock->closingBracketPosition;

    if (newVariable.tokenValueStart < newVariable.tokenValueEnd) {
        newVariable.tokenValueStart += 1; // Exclude = from sequence
    }

    newVariable.value = "";
    for (int i = newVariable.tokenValueStart; i != newVariable.tokenValueEnd; ++i) {
        newVariable.value += tokens[i].value;
    }

    // Check if variable is not defined twice

    if (newSymbolPosition.currentBlock->variables.find(newVariable.name) != newSymbolPosition.currentBlock->variables.end()) {
        throw SymbolsTableGenerationError("Error: Variable `" + newVariable.name + "` is defined twice");
    }

    // Check type for expression int x = y;

    if (newVariable.tokenValueEnd - newVariable.tokenValueStart == 1) {
        switch (tokens[newVariable.tokenValueStart].type) {
        case Lexem::LEXEM_TOKEN_IDENTIFIER:
            {
                SymbolVariable* variable = GetVariableByName(tokens[newVariable.tokenValueStart].value, newSymbolPosition.currentBlock, syntaxTreeNode->value.j);

                if (!variable) {
                    throw SymbolsTableGenerationError("Error: Variable `" + tokens[newVariable.tokenValueStart].value + "` is not defined before");
                }

                if (variable->type != newVariable.type) {
                    throw SymbolsTableGenerationError("Error: Type of variable `" + variable->name + "` is not same the same as `" + newVariable.name + "`");
                }
            }
            break;
        case Lexem::LEXEM_TOKEN_LITERAL_INTEGER_NUMBER:
            if (newVariable.type != "int") {
                throw SymbolsTableGenerationError("Error: Type of variable `" + newVariable.name + "` must be `int`");
            }
            break;
        case Lexem::LEXEM_TOKEN_LITERAL_REAL_NUMBER:
            if (newVariable.type != "double") {
                throw SymbolsTableGenerationError("Error: Type of variable `" + newVariable.name + "` must be `double`");
            }
            break;
        case Lexem::LEXEM_TOKEN_LITERAL_STRING:
            if (newVariable.type != "string") {
                throw SymbolsTableGenerationError("Error: Type of variable `" + newVariable.name + "` must be `string`");
            }
            break;
        }
    }

    newSymbolPosition.currentBlock->variables.insert(make_pair(newVariable.name, newVariable));

    return true;
}

bool TryMethodNonterminals(const ParseTree::Node* syntaxTreeNode, const vector<Lexem>& tokens, PositionInSymbolsTable& newSymbolPosition, SymbolsTable& symbolsTable)
{
    if (!syntaxTreeNode || syntaxTreeNode->value.name != "<method-header>") {
        return false;
    }

    ParseTree::Node* methodModifier = syntaxTreeNode->left;
     
    if (!methodModifier || methodModifier->value.name != "<method-modifier>") {
        return false;
    }

    ParseTree::Node* methodHeaderPart1 = syntaxTreeNode->right;

    if (!methodHeaderPart1 || methodHeaderPart1->value.name != "<method-header-part1>") {
        return false;
    }

    ParseTree::Node* methodReturnType = methodHeaderPart1->left;

    if (!methodReturnType || (methodReturnType->value.name != "<void>" && methodReturnType->value.name != "<type>")) {
        return false;
    }

    ParseTree::Node* methodHeaderPart2 = methodHeaderPart1->right;

    if (!methodHeaderPart2 || methodHeaderPart2->value.name != "<method-header-part2>") {
        return false;
    }

    ParseTree::Node* methodName = methodHeaderPart2->left;

    if (!methodName || methodName->value.name != "<identifier>") {
        return false;
    }

    ParseTree::Node* methodHeaderPart3 = methodHeaderPart2->right;

    if (!methodHeaderPart3 || methodHeaderPart3->value.name != "<method-header-part3>") {
        return false;
    }

    ParseTree::Node* openingParentheses = methodHeaderPart3->left;

    if (!openingParentheses || openingParentheses->value.name != "<opening-parentheses>") {
        return false;
    }

    ParseTree::Node* formalParametersStart = methodHeaderPart3->right;

    if (!formalParametersStart || (formalParametersStart->value.name != "<method-header-part4a>"
            && formalParametersStart->value.name != "<method-header-part4b>"
            && formalParametersStart->value.name != "<closing-parentheses>")) {
        return false;
    }

    SymbolFunction newFunction;
    newFunction.modifier = tokens[methodModifier->value.j].value;
    newFunction.name = tokens[methodName->value.j].value;
    newFunction.returnType = tokens[methodReturnType->value.j].value;

    if (newSymbolPosition.classIndex == -1) {
        throw SymbolsTableGenerationError("Unexpected situation: Method outside of class");
    }
    newSymbolPosition.functionName = newFunction.name;

    if (symbolsTable[newSymbolPosition.classIndex].functions.find(newFunction.name) != symbolsTable[newSymbolPosition.classIndex].functions.end()) {
        throw SymbolsTableGenerationError("Error: Method name `" + newFunction.name + "` is used twice in `" + symbolsTable[newSymbolPosition.classIndex].name + "` class declaration");
    }

    VariablesBlock newBlock;
    newBlock.parentBlock = nullptr;
    
    if (formalParametersStart->value.i > 1) {
        for (int curLexem = formalParametersStart->value.j, lastLexem = formalParametersStart->value.j + formalParametersStart->value.i; curLexem < lastLexem; curLexem += 3) {
            FixedParameter parameter;
            parameter.type = tokens[curLexem].value;
            parameter.name = tokens[curLexem + 1].value;

            if (tokens[curLexem + 2].value != "," && tokens[curLexem + 2].value != ")") {
                return false;
            }

            if (newFunction.parameters.find(parameter.name) != newFunction.parameters.end()) {
                throw SymbolsTableGenerationError("Error: Formal parameter name `" + parameter.name + "` is used twice in `" + newFunction.name + "` method declaration");
            }

            newFunction.parameters.insert(make_pair(parameter.name, parameter));

            SymbolVariable newVariable;
            newVariable.tokenValueStart = curLexem + 2;
            newVariable.tokenValueEnd = curLexem + 2;
            newVariable.type = parameter.type;
            newVariable.name = parameter.name;
            newVariable.variableVisibilityStart = 0;
            newVariable.variableVisibilityEnd = tokens.size();

            newBlock.variables.insert(make_pair(newVariable.name, newVariable));
        }
    }

    newFunction.blocks.push_back(newBlock);

    symbolsTable[newSymbolPosition.classIndex].functions.insert(make_pair(newFunction.name, newFunction));
    newSymbolPosition.currentBlock = &symbolsTable[newSymbolPosition.classIndex].functions[newFunction.name].blocks[0];

    return true;
}

void TraverseThroughSyntaxTreeAndAddSymbols(const ParseTree::Node* syntaxTreeNode, const vector<Lexem>& tokens, PositionInSymbolsTable& newSymbolPosition, SymbolsTable& symbolsTable)
{
    if (!syntaxTreeNode) {
        return;
    }

    if (TryClassNonterminals(syntaxTreeNode, tokens, newSymbolPosition, symbolsTable)) {
        TraverseThroughSyntaxTreeAndAddSymbols(syntaxTreeNode->right, tokens, newSymbolPosition, symbolsTable);
        
        return;
    }

    if (TryMethodNonterminals(syntaxTreeNode, tokens, newSymbolPosition, symbolsTable)) {
        return;
    }

    if (TryVariablesNonterminals(syntaxTreeNode, tokens, newSymbolPosition, symbolsTable)) {
        return;
    }

    TryBlockNonterminals(syntaxTreeNode, tokens, newSymbolPosition, symbolsTable);

    TraverseThroughSyntaxTreeAndAddSymbols(syntaxTreeNode->left, tokens, newSymbolPosition, symbolsTable);
    TraverseThroughSyntaxTreeAndAddSymbols(syntaxTreeNode->right, tokens, newSymbolPosition, symbolsTable);
}

void GenerateSymbolsTable(const ParseTree& syntaxTree, const vector<Lexem>& tokens, SymbolsTable& symbolsTable)
{
    PositionInSymbolsTable firstSymbolPosition;
    firstSymbolPosition.classIndex    = -1;
    firstSymbolPosition.functionName  = "";
    firstSymbolPosition.currentBlock  = nullptr;

    TraverseThroughSyntaxTreeAndAddSymbols(syntaxTree.GetRoot(), tokens, firstSymbolPosition, symbolsTable);
}

void WriteBlocksToXLS(BasicExcelWorksheet* worksheet, int& row, int col, VariablesBlock& block)
{
    worksheet->Cell(row, col)->Set("Block");

    int varsCol = col + 1;

    for (auto curVariable = block.variables.begin(), lastVariable = block.variables.end(); curVariable != lastVariable; ++curVariable) {
        string content = curVariable->second.name + ": " + curVariable->second.type;

        if (!curVariable->second.value.empty()) {
            content += " = " + curVariable->second.value;
        }

        worksheet->Cell(row, varsCol)->Set(content.c_str());

        ++varsCol;
    }

    for (auto curBlock = block.childrenBlocks.begin(), lastBlock = block.childrenBlocks.end(); curBlock != lastBlock; ++curBlock) {
        ++row;
        
        WriteBlocksToXLS(worksheet, row, col + 1, *curBlock);
    }
}

void WriteSymbolsTableToXLS(const SymbolsTable& symbolsTable, const char* filepath)
{
    BasicExcel xlsFile;
    BasicExcelWorksheet* worksheet = xlsFile.AddWorksheet("Symbols");

    worksheet->Cell(0, 0)->Set(L"Classes ->");

    int row = 1;

    for (vector<SymbolClass>::const_iterator curClass = symbolsTable.begin(), lastClass = symbolsTable.end(); curClass != lastClass; ++curClass) {
        worksheet->Cell(row, 0)->Set(curClass->name.c_str());

        if (curClass->functions.empty()) {
            row += 2;

            continue;
        }

        worksheet->Cell(row + 2, 0)->Set(L"Methods ->");
        worksheet->Cell(row + 2, 1)->Set(L"Modifier");
        worksheet->Cell(row + 2, 2)->Set(L"Return type");
        worksheet->Cell(row + 2, 3)->Set(L"Name");
        worksheet->Cell(row + 2, 4)->Set(L"Parameters");

        row += 3;

        for (auto curFunction : curClass->functions) {
            worksheet->Cell(row, 1)->Set(curFunction.second.modifier.c_str());
            worksheet->Cell(row, 2)->Set(curFunction.second.returnType.c_str());
            worksheet->Cell(row, 3)->Set(curFunction.second.name.c_str());

            int parameterIndex = 0;

            for (auto curParameter = curFunction.second.parameters.begin(), lastParameter = curFunction.second.parameters.end(); curParameter != lastParameter; ++curParameter) {
                worksheet->Cell(row, 4 + parameterIndex)->Set((curParameter->second.name + ": " + curParameter->second.type).c_str());

                ++parameterIndex;
            }

            worksheet->Cell(row + 1, 0)->Set(L"Blocks ->");

            ++row;

            for (auto curBlock = curFunction.second.blocks.begin(), lastBlock = curFunction.second.blocks.end(); curBlock != lastBlock; ++curBlock) {
                WriteBlocksToXLS(worksheet, row, 1, *curBlock);
                
                ++row;
            }

            ++row;
        }

        ++row;
    }

    worksheet->SetColWidth(0, 15000);
    worksheet->SetColWidth(1, 15000);

    xlsFile.SaveAs(filepath);
}

