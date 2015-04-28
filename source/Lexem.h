#ifndef CSHARP_LEXICAL_ANALYZER_LEXEM_H
#define CSHARP_LEXICAL_ANALYZER_LEXEM_H

#include <string>

using std::string;

struct Lexem
{
    enum TypeOfLexem {
        LEXEM_UNKNOWN = 0,

        LEXEM_TOKEN_LITERAL_INTEGER_NUMBER,
        LEXEM_TOKEN_LITERAL_REAL_NUMBER,
        LEXEM_TOKEN_LITERAL_STRING,
        LEXEM_TOKEN_IDENTIFIER,
        LEXEM_TOKEN_KEYWORD,
        LEXEM_TOKEN_OPERATOR,

        LEXEM_WHITESPACE,
        LEXEM_LINEENDING,
        LEXEM_COMMENT,
        LEXEM_DIRECTIVE
    };

    TypeOfLexem type;
    string value;
    string spawningRuleName;

    size_t lineNumber;
    size_t offsetFromLineStart;
};

string TypeOfLexemToString(Lexem::TypeOfLexem type);
bool IsKeyword(const string& identifier);

#endif //CSHARP_LEXICAL_ANALYZER_LEXEM_H