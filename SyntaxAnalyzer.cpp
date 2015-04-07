#include <fstream>
#include <map>
#include <utility>
#include <sstream>

#include "SyntaxAnalyzer.h"
#include "Lexem.h"

using std::ifstream;
using std::ios_base;
using std::map;
using std::pair;
using std::make_pair;

#include <ExcelFormat.h>

using namespace ExcelFormat;

bool LoadSyntaxRules(const char *path, vector<SyntaxRule>& rules)
{
    rules.clear();

    ifstream in(path, ios_base::in);

    if (!in.good()) {
        return false;
    }

    vector<SyntaxRule> result;

    string name;
    string separator;
    string firstPart;
    string secondPart;

    do {
        in >> name >> separator >> firstPart;

        if (separator != "::=") {
            return false;
        }

        if (firstPart.empty()) {
            return false;
        }

        if (firstPart[0] == '"') {
            SyntaxRule rule(name, firstPart.substr(1, firstPart.length() - 2));
            rules.push_back(rule);
        } else if (firstPart[0] == '<') {
            in >> secondPart;

            SyntaxRule rule(name, firstPart, secondPart);
            rules.push_back(rule);
        } else if (firstPart == "identifier") {
            SyntaxRule rule(name, SyntaxRule::IDENTIFIER_NONKEYWORD_TERMINAL);
            rules.push_back(rule);
        } else if (firstPart == "literal") {
            SyntaxRule rule(name, SyntaxRule::LITERAL_TERMINAL);
            rules.push_back(rule);
        }
    } while (in.good());

    return true;
}

void TraceSyntaxAnalyzerPath(map<NonTerminalPositionType, vector<NonTerminalPositionType> >& P, ParseTree& tree, ParseTree::Node* node)
{
    if (P[node->value].empty()) {
        return;
    }

    if (P[node->value].size() == 1 && P[node->value][0].name == "-terminal-rule-") {
        // Ignore because -terminal-rule- is just a mark that the rule spawns a terminal

        return;
    }

    if (P[node->value].size() == 2) {
        ParseTree::Node* leftNode = tree.InsertChildNode(node, P[node->value][0]);
        ParseTree::Node* rightNode = tree.InsertChildNode(node, P[node->value][1]);

        TraceSyntaxAnalyzerPath(P, tree, leftNode);
        TraceSyntaxAnalyzerPath(P, tree, rightNode);

        return;
    }

    throw SyntaxError("Error: Not a binary tree");
}

ParseTree DoSyntaxAnalysis(const vector<Lexem>& tokens, const vector<SyntaxRule>& rules, const string& startingSymbol)
{
    size_t textSize = tokens.size();
    size_t rulesSize = rules.size();

    // Current implementation uses CYK algorithm
    //
    // For more details see http://web.cs.ucdavis.edu/~rogaway/classes/120/winter12/CYK.pdf

    // Initialize array

    map<NonTerminalPositionType, vector<NonTerminalPositionType> > P; // First argument is position of non-terminal and second is a list of positions of its children rules

    for (int i = 0; i < textSize; ++i) {
        for (int j = 0; j < textSize; ++j) {
            for (int k = 0; k < rulesSize; ++k) {
                P[NonTerminalPositionType(i, j, rules[k].GetName())] = vector<NonTerminalPositionType>();
            }
        }
    }

    // Do first pass (terminal rules)

    for (int i = 0; i < textSize; ++i) {
        for (int k = 0; k < rulesSize; ++k) {
            if (rules[k].IsTerminalRule()) {
                bool ruleFound = false;

                if ((tokens[i].type == Lexem::LEXEM_TOKEN_LITERAL_INTEGER_NUMBER ||
                     tokens[i].type == Lexem::LEXEM_TOKEN_LITERAL_REAL_NUMBER ||
                     tokens[i].type == Lexem::LEXEM_TOKEN_LITERAL_STRING)
                     && rules[k].GetFirstPartOfRule() == SyntaxRule::LITERAL_TERMINAL
                   ) {
                    ruleFound = true;
                } else if (tokens[i].type == Lexem::LEXEM_TOKEN_IDENTIFIER
                    && rules[k].GetFirstPartOfRule() == SyntaxRule::IDENTIFIER_NONKEYWORD_TERMINAL) {
                    ruleFound = true;
                } else if ((tokens[i].type == Lexem::LEXEM_TOKEN_KEYWORD || tokens[i].type == Lexem::LEXEM_TOKEN_OPERATOR)
                    && rules[k].GetFirstPartOfRule() == tokens[i].value) {
                    ruleFound = true;
                }

                if (ruleFound) {
                    P[NonTerminalPositionType(0, i, rules[k].GetName())].push_back(NonTerminalPositionType(0, i, "-terminal-rule-"));
                }
            }
        }
    }

    // Do second pass (non-terminal rules)

    for (int i = 1; i < textSize; ++i) {
        for (int j = 0; j < textSize - i; ++j) {
            for (int k = 0; k < i; ++k) {
                for (int l = 0; l < rulesSize; ++l) {
                    if (!rules[l].IsTerminalRule()) {
                        if (!P[NonTerminalPositionType(k, j, rules[l].GetFirstPartOfRule())].empty() &&
                            !P[NonTerminalPositionType(i - k - 1, j + k + 1, rules[l].GetSecondPartOfRule())].empty()) {

                            if (!P[NonTerminalPositionType(i, j, rules[l].GetName())].empty()) {
                                if (P[NonTerminalPositionType(i, j, rules[l].GetName())][0] == NonTerminalPositionType(k, j, rules[l].GetFirstPartOfRule())) {
                                    // Just skip and don't add same children non-terminals again
                                } else {
                                    throw SyntaxError("Error: Ambiguous grammar");
                                }
                            } else {
                                P[NonTerminalPositionType(i, j, rules[l].GetName())].push_back(NonTerminalPositionType(k, j, rules[l].GetFirstPartOfRule()));
                                P[NonTerminalPositionType(i, j, rules[l].GetName())].push_back(NonTerminalPositionType(i - k - 1, j + k + 1, rules[l].GetSecondPartOfRule()));
                            }
                        }
                    }
                }
            }
        }
    }

    // Trace syntax analyzer path

    if (P[NonTerminalPositionType(textSize - 1, 0, startingSymbol)].empty()) {
        return ParseTree();
    }

    ParseTree result;
    ParseTree::Node* startingNode = result.InsertChildNode(nullptr, NonTerminalPositionType(textSize - 1, 0, startingSymbol));

    TraceSyntaxAnalyzerPath(P, result, startingNode);

    return result;
}
