#include <set>

#include "Lexem.h"

using namespace std;

string TypeOfLexemToString(Lexem::TypeOfLexem type)
{
    switch (type) {
    case Lexem::LEXEM_UNKNOWN:
        return "LEXEM_UNKNOWN";
    case Lexem::LEXEM_TOKEN_LITERAL_INTEGER_NUMBER:
        return "LEXEM_TOKEN_LITERAL_INTEGER_NUMBER";
    case Lexem::LEXEM_TOKEN_LITERAL_REAL_NUMBER:
        return "LEXEM_TOKEN_LITERAL_REAL_NUMBER";
    case Lexem::LEXEM_TOKEN_LITERAL_STRING:
        return "LEXEM_TOKEN_LITERAL_STRING";
    case Lexem::LEXEM_TOKEN_IDENTIFIER:
        return "LEXEM_TOKEN_IDENTIFIER";
    case Lexem::LEXEM_TOKEN_KEYWORD:
        return "LEXEM_TOKEN_KEYWORD";
    case Lexem::LEXEM_TOKEN_OPERATOR:
        return "LEXEM_TOKEN_OPERATOR";
    case Lexem::LEXEM_WHITESPACE:
        return "LEXEM_WHITESPACE";
    case Lexem::LEXEM_LINEENDING:
        return "LEXEM_LINEENDING";
    case Lexem::LEXEM_COMMENT:
        return "LEXEM_COMMENT";
    case Lexem::LEXEM_DIRECTIVE:
        return "LEXEM_DIRECTIVE";
    };

    return "";
}


bool IsKeyword(const string& identifier)
{
    static set<string> keywords;

    if (keywords.empty()) {
        keywords.insert("abstract");
        keywords.insert("as");
        keywords.insert("base");
        keywords.insert("bool");
        keywords.insert("break");
        keywords.insert("byte");
        keywords.insert("case");
        keywords.insert("catch");
        keywords.insert("char");
        keywords.insert("checked");
        keywords.insert("class");
        keywords.insert("const");
        keywords.insert("continue");
        keywords.insert("decimal");
        keywords.insert("default");
        keywords.insert("delegate");
        keywords.insert("do");
        keywords.insert("double");
        keywords.insert("else");
        keywords.insert("enum");
        keywords.insert("event");
        keywords.insert("explicit");
        keywords.insert("extern");
        keywords.insert("false");
        keywords.insert("finally");
        keywords.insert("fixed");
        keywords.insert("float");
        keywords.insert("for");
        keywords.insert("foreach");
        keywords.insert("goto");
        keywords.insert("if");
        keywords.insert("implicit");
        keywords.insert("in");
        keywords.insert("int");
        keywords.insert("interface");
        keywords.insert("internal");
        keywords.insert("is");
        keywords.insert("lock");
        keywords.insert("long");
        keywords.insert("namespace");
        keywords.insert("new");
        keywords.insert("null");
        keywords.insert("object");
        keywords.insert("operator");
        keywords.insert("out");
        keywords.insert("override");
        keywords.insert("params");
        keywords.insert("private");
        keywords.insert("protected");
        keywords.insert("public");
        keywords.insert("readonly");
        keywords.insert("ref");
        keywords.insert("return");
        keywords.insert("sbyte");
        keywords.insert("sealed");
        keywords.insert("short");
        keywords.insert("sizeof");
        keywords.insert("stackalloc");
        keywords.insert("static");
        keywords.insert("string");
        keywords.insert("struct");
        keywords.insert("switch");
        keywords.insert("this");
        keywords.insert("throw");
        keywords.insert("true");
        keywords.insert("try");
        keywords.insert("typeof");
        keywords.insert("uint");
        keywords.insert("ulong");
        keywords.insert("unchecked");
        keywords.insert("unsafe");
        keywords.insert("ushort");
        keywords.insert("using");
        keywords.insert("virtual");
        keywords.insert("void");
        keywords.insert("volatile");
        keywords.insert("while");
    }

    return keywords.find(identifier) != keywords.end();
}