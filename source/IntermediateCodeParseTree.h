#ifndef CSHARP_INTERMEDIATE_CODE_PARSE_TREE_H
#define CSHARP_INTERMEDIATE_CODE_PARSE_TREE_H

#include <string>
#include <vector>
#include <ostream>

#include "Lexem.h"
#include "ParseTree.h"

using std::string;
using std::vector;
using std::ostream;

struct IntermediateCodeParseTreeNode
{
    vector<IntermediateCodeParseTreeNode> children;
    Lexem token;
    string intermediateRuleName;
    int positionInCode;
};

void BuildIntermediateCodeParseTree(const SyntaxParseTree::Node* syntaxTreeRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& intermediateTreeRoot);
void DisplayIntermediateTreeAsText(ostream& out, const IntermediateCodeParseTreeNode& root);

#endif // CSHARP_INTERMEDIATE_CODE_PARSE_TREE_H