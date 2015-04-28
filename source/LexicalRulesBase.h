#ifndef CSHARP_LEXICAL_ANALYZER_RULES_BASE_H
#define CSHARP_LEXICAL_ANALYZER_RULES_BASE_H

#include <vector>

#include "LexicalRule.h"

using std::vector;
using std::pair;

typedef bool(*stoppingCriteriaFunction)(const string& prefix);

Lexem  MatchContentOverRules(
    const string& contentOfSourceFile,
    Lexem::TypeOfLexem lexemType,
    vector<LexicalRule>::iterator iterFirstRule,
    vector<LexicalRule>::iterator iterLastRule,
    stoppingCriteriaFunction stoppingCriteria = nullptr
);

Lexem ReadTokenLiteralIntegerNumber(const string& contentOfSourceFile);
Lexem ReadTokenLiteralRealNumber(const string& contentOfSourceFile);
Lexem ReadTokenLiteralString(const string& contentOfSourceFile);
Lexem ReadTokenOperator(const string& contentOfSourceFile);
Lexem ReadTokenIdentifier(const string& contentOfSourceFile);

Lexem ReadWhitespace(const string& contentOfSourceFile);
Lexem ReadLineEnding(const string& contentOfSourceFile);
Lexem ReadComment(const string& contentOfSourceFile);
Lexem ReadDirective(const string& contentOfSourceFile);

#endif //CSHARP_LEXICAL_ANALYZER_RULES_BASE_H