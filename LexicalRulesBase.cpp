#include <list>
#include <ctype.h>

#include "LexicalAnalyzerException.h"
#include "LexicalRulesBase.h"
#include "LexicalDirectiveIf.h"

using namespace std;

Lexem MatchContentOverRules(const string& contentOfSourceFile, Lexem::TypeOfLexem lexemType, vector<LexicalRule>::iterator iterFirstRule, vector<LexicalRule>::iterator iterLastRule, stoppingCriteriaFunction stoppingCriteria)
{
    list<LexicalRule> correspondingRules;
    size_t bufferLength = 1;

    string correspondingRuleName;
    size_t correspondingRuleBufferLength = 0;

    do {
        string contentBuffer = contentOfSourceFile.substr(0, bufferLength);
        correspondingRules.clear();

        for (vector<LexicalRule>::iterator rule = iterFirstRule, ruleEnd = iterLastRule; rule != ruleEnd; ++rule) {
            if (rule->Match(contentBuffer)) {
                correspondingRules.push_back(*rule);
            }
        }

        if (correspondingRules.size() == 1) {
            // Save information about current longest prefix available which can be spawned by exactly 1 rule

            correspondingRuleBufferLength = bufferLength;
            correspondingRuleName = correspondingRules.begin()->GetName();
        } else if (correspondingRules.size() == 0 && correspondingRuleBufferLength > 0) {
            // If no rule satisfy string and some prefix was already found then break the cycle and return found prefix (if any)

            break;
        }

        if (stoppingCriteria != nullptr && stoppingCriteria(contentBuffer)) {
            break;
        }

        ++bufferLength;
    } while (bufferLength != (contentOfSourceFile.length() + 1));

    if (correspondingRuleBufferLength > 0) {
        Lexem resultLexem;
        resultLexem.type = lexemType;
        resultLexem.value = contentOfSourceFile.substr(0, correspondingRuleBufferLength);
        resultLexem.spawningRuleName = correspondingRuleName;

        return resultLexem;
    }

    if (correspondingRules.size() > 1) {
        throw LexicalAnalyzerException("Too many matching rules for `" + contentOfSourceFile + "`");
    }

    return Lexem();
}

Lexem ReadTokenLiteralIntegerNumber(const string& contentOfSourceFile)
{
    static vector<LexicalRule> rules;

    if (rules.empty()) {
        rules.push_back(LexicalRule("LITERAL_INT_DECIMAL", "(\\d+)(U|u|L|l|UL|Ul|uL|ul|LU|Lu|lU|lu)?"));
        rules.push_back(LexicalRule("LITERAL_INT_HEX", "0(x|X)(\\d|a|b|c|d|e|f|A|B|C|D|E|F)+(U|u|L|l|UL|Ul|uL|ul|LU|Lu|lU|lu)?"));
    }

    if (!isdigit(contentOfSourceFile[0])) {
        return Lexem();
    }

    return MatchContentOverRules(contentOfSourceFile, Lexem::LEXEM_TOKEN_LITERAL_INTEGER_NUMBER, rules.begin(), rules.end());
}

bool ReadRealNumberTokenStoppingCriteria(const string& prefix)
{
    size_t lastSymbolPos = prefix.length() - 1;

    if (prefix[lastSymbolPos] == 'F' && prefix[lastSymbolPos] == 'f' &&
        prefix[lastSymbolPos] == 'D' && prefix[lastSymbolPos] == 'd' &&
        prefix[lastSymbolPos] == 'M' && prefix[lastSymbolPos] == 'm') {
        return true;
    }

    bool expSymbolSignFound = false;
    bool expSymbolFound = false;
    bool pointFound = false;

    bool shouldWeStop = false;

    for (size_t i = 0, endi = prefix.length(); i != endi; ++i) {
        if (!isdigit(prefix[i])) {
            shouldWeStop = true;
        }

        if ((prefix[i] == 'E' || prefix[i] == 'e') && !expSymbolFound) {
            shouldWeStop = false;
            expSymbolFound = true;
        }

        if ((prefix[i] == '-' || prefix[i] == '+') && !expSymbolSignFound) {
            shouldWeStop = false;
            expSymbolSignFound = true;
        }

        if (prefix[i] == '.' && !pointFound) {
            shouldWeStop = false;
            pointFound = true;
        }
    }

    return shouldWeStop;
}

Lexem ReadTokenLiteralRealNumber(const string& contentOfSourceFile)
{
    static vector<LexicalRule> rules;

    if (rules.empty()) {
        rules.push_back(LexicalRule("LITERAL_REAL1", "(\\d*)\\.(\\d+)((e|E)(\\+|\\-)?(\\d+))?(F|f|D|d|M|m)?"));
        rules.push_back(LexicalRule("LITERAL_REAL2", "(\\d+)((e|E)(\\+|\\-)?(\\d+))(F|f|D|d|M|m)?"));
        rules.push_back(LexicalRule("LITERAL_REAL3", "(\\d+)(F|f|D|d|M|m)"));
    }

    if (!isdigit(contentOfSourceFile[0]) && contentOfSourceFile[0] != '.') {
        return Lexem();
    }

    return MatchContentOverRules(contentOfSourceFile, Lexem::LEXEM_TOKEN_LITERAL_REAL_NUMBER, rules.begin(), rules.end(), ReadRealNumberTokenStoppingCriteria);
}

Lexem ReadTokenLiteralString(const string& contentOfSourceFile)
{
    static vector<LexicalRule> rules;

    if (rules.empty()) {
        rules.push_back(LexicalRule("LITERAL_STRING1", "\"([^\"\n]|\\\\\"|\\\\'|\\\\\\\\|\\\\0|\\\\a|\\\\b|\\\\f|\\\\n|\\\\r|\\\\t|\\\\v|\\\\x(\\d|a|b|c|d|e|f|A|B|C|D|E|F){1,4}|\\\\u(\\d|a|b|c|d|e|f|A|B|C|D|E|F){4}|\\\\U(\\d|a|b|c|d|e|f|A|B|C|D|E|F){8})*\""));
        rules.push_back(LexicalRule("LITERAL_STRING2", "@\"(\"\"|[^\"])*\""));
    }
    
    if (contentOfSourceFile.substr(0, 1) != "\"" && contentOfSourceFile.substr(0, 2) != "@\"") {
        return Lexem();
    }

    return MatchContentOverRules(contentOfSourceFile, Lexem::LEXEM_TOKEN_LITERAL_STRING, rules.begin(), rules.end());
}

bool ReadTokenOperatorStoppingCriteria(const string& prefix)
{
    return prefix.length() >= 4;
}

Lexem ReadTokenOperator(const string& contentOfSourceFile)
{
    static vector<LexicalRule> rules;

    if (rules.empty()) {
        rules.push_back(LexicalRule("OPERATOR", "\\{|\\}|\\[|\\]|\\(|\\)|\\.|\\,|\\:|\\;|\\+|\\-|\\*|\\/|\\%|\\&|\\||\\^|\\!|\\~|\\=|\\<|\\>|\\?|\\+\\+|\\-\\-|\\&\\&|\\|\\||\\<\\<|\\>\\>|\\=\\=|\\!\\=|\\<\\=|\\>\\=|\\+\\=|\\-\\=|\\*\\=|\\/\\=|\\%\\=|\\&\\=|\\|\\=|\\^\\=|\\<\\<\\=|\\>\\>\\=|\\-\\>"));
    }

    return MatchContentOverRules(contentOfSourceFile, Lexem::LEXEM_TOKEN_OPERATOR, rules.begin(), rules.end(), ReadTokenOperatorStoppingCriteria);
}

Lexem ReadTokenIdentifier(const string& contentOfSourceFile)
{
    static vector<LexicalRule> rules;

    if (rules.empty()) {
        rules.push_back(LexicalRule("IDENTIFIER1", "(_|[a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|_)*"));
        rules.push_back(LexicalRule("IDENTIFIER2", "@(_|[a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|_)*"));
    }

    if (!isalpha(contentOfSourceFile[0]) && contentOfSourceFile[0] != '@' && contentOfSourceFile[0] != '_') {
        return Lexem();
    }

    return MatchContentOverRules(contentOfSourceFile, Lexem::LEXEM_TOKEN_IDENTIFIER, rules.begin(), rules.end());
}

Lexem ReadWhitespace(const string& contentOfSourceFile)
{
    size_t prefixLength, contentLength = contentOfSourceFile.length();
    for (prefixLength = 0; prefixLength < contentLength; ++prefixLength) {
        char c = contentOfSourceFile[prefixLength];
        if (c != '\x20' && c != '\xA0' && c != '\x09' && c != '\x0B' && c != '\x0C') {
            break;
        }
    }

    Lexem resultLexem;
    resultLexem.type = Lexem::LEXEM_WHITESPACE;
    resultLexem.spawningRuleName = "WHITESPACE";
    resultLexem.value = contentOfSourceFile.substr(0, prefixLength);

    return resultLexem;
}

Lexem ReadLineEnding(const string& contentOfSourceFile)
{
    Lexem lineEndingLexem;
    lineEndingLexem.spawningRuleName = "LINE_ENDING";
    lineEndingLexem.type = Lexem::LEXEM_LINEENDING;

    if (contentOfSourceFile.substr(0, 2) == "\x0D\x0A") {
        lineEndingLexem.value = contentOfSourceFile.substr(0, 2);
    } else if (contentOfSourceFile.substr(0, 1) == "\x0D" ||
        contentOfSourceFile.substr(0, 1) == "\x0D") {
        lineEndingLexem.value = contentOfSourceFile.substr(0, 1);
    } else {
        return Lexem();
    }

    return lineEndingLexem;
}

Lexem ReadComment(const string& contentOfSourceFile)
{
    // Try to read multi line comment

    if (contentOfSourceFile.substr(0, 2) == "/*") {
        size_t lexemLength = 4, contentLength = contentOfSourceFile.length();
        bool commentEndingNotFound = false;

        while (true) {
            if (contentOfSourceFile.substr(lexemLength - 2, 2) == "*/") {
                break;
            }

            if (lexemLength == contentLength) {
                commentEndingNotFound = true;
                break;
            }

            lexemLength++;
        }

        if (commentEndingNotFound) {
            return Lexem();
        } else {
            Lexem resultLexem;
            resultLexem.spawningRuleName = "MULTILINE_COMMENT";
            resultLexem.type = Lexem::LEXEM_COMMENT;
            resultLexem.value = contentOfSourceFile.substr(0, lexemLength);

            return resultLexem;
        }
    }

    // Try to read single line comment

    static vector<LexicalRule> rules;

    if (rules.empty()) {
        rules.push_back(LexicalRule("SINGLELINE_COMMENT", "\\/\\/([^\\u000D\\u000A])*(\\u000D|\\u000A|\\u000D\\u000A)"));
    }

    if (contentOfSourceFile.substr(0, 2) != "//") {
        return Lexem();
    }

    return MatchContentOverRules(contentOfSourceFile, Lexem::LEXEM_COMMENT, rules.begin(), rules.end());
}

Lexem ReadDirective(const string& contentOfSourceFile)
{
    static vector<LexicalRule> rules;

    if (rules.empty()) {
        rules.push_back(LexicalRule("DIRECTIVE_DEFINE", "\\#(\\u0020|\\u00A0\\u0009|\\u000B|\\u000C)*(define)(\\u0020|\\u00A0\\u0009|\\u000B|\\u000C)+((\\@)?(_|[a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|_)*)((\\/\\/([^\\u000D\\u000A])*)?)(\\u000D|\\u000A|\\u000D\\u000A)"));
        rules.push_back(LexicalRule("DIRECTIVE_UNDEFINE", "\\#(\\u0020|\\u00A0\\u0009|\\u000B|\\u000C)*(undef)(\\u0020|\\u00A0\\u0009|\\u000B|\\u000C)+((\\@)?(_|[a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|_)*)((\\/\\/([^\\u000D\\u000A])*)?)(\\u000D|\\u000A|\\u000D\\u000A)"));
    }

    if (contentOfSourceFile[0] != '#') {
        return Lexem();
    }

    Lexem lexemIfDirective = ReadIfDirective(contentOfSourceFile);
    if (!lexemIfDirective.value.empty()) {
        return lexemIfDirective;
    }

    return MatchContentOverRules(contentOfSourceFile, Lexem::LEXEM_DIRECTIVE, rules.begin(), rules.end());
}
