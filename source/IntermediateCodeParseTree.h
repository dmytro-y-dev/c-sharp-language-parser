#ifndef CSHARP_INTERMEDIATE_CODE_PARSE_TREE_H
#define CSHARP_INTERMEDIATE_CODE_PARSE_TREE_H

#include <string>
#include <vector>

#include "Lexem.h"
#include "ParseTree.h"

using std::string;
using std::vector;

struct IntermediateCodeParseTreeNode
{
    vector<IntermediateCodeParseTreeNode> children;
    Lexem token;
    string intermediateRule;
    int positionInCode;
};

void BuildIntermediateCodeParseTreeFromSyntaxParseTree(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& intermediateRoot);

#endif // CSHARP_INTERMEDIATE_CODE_PARSE_TREE_H