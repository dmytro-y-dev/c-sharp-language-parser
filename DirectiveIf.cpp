#include <vector>

#include "RulesBase.h"
#include "Rule.h"
#include "DirectiveIf.h"

using namespace std;

size_t ReadIfDirectiveExpression(const string& contentOfSourceFile);
Lexem  MatchContentOverRules(
    const string& contentOfSourceFile,
    Lexem::TypeOfLexem lexemType,
    vector<Rule>::iterator iterFirstRule,
    vector<Rule>::iterator iterLastRule
);

size_t ReadIfDirectiveSingleLineComment(const string& contentOfSourceFile)
{
    if (contentOfSourceFile.substr(0, 2) != "//") {
        return 0;
    }

    size_t posInBuffer = 2;
    while (contentOfSourceFile[posInBuffer] != '\n') {
        ++posInBuffer;
    }

    return posInBuffer-1;
}

size_t ReadIfDirectiveWhitespace(const string& contentOfSourceFile)
{
    Lexem whitespace = ReadWhitespace(contentOfSourceFile);

    return whitespace.value.length();
}

size_t ReadIfDirectivePrimaryExpression(const string& contentOfSourceFile)
{
    size_t resultSize = 0;

    // Alternative 1

    Lexem identifier = ReadTokenIdentifier(contentOfSourceFile);
    if (!identifier.value.empty()) {
        return identifier.value.length();
    }

    // Alternative 2

    if (contentOfSourceFile.substr(resultSize, 1) != "(") {
        return 0;
    }
    resultSize += 1;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveExpression(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 1) != ")") {
        return 0;
    }
    resultSize += 1;

    return resultSize;
}

size_t ReadIfDirectiveUnaryExpression(const string& contentOfSourceFile)
{
    size_t resultSize = 0;

    // Alternative 1

    resultSize = ReadIfDirectivePrimaryExpression(contentOfSourceFile);

    if (resultSize == 0) {
        return 0;
    }

    // Alternative 2

    if (contentOfSourceFile.substr(resultSize, 1) != "!") {
        return resultSize;
    }
    resultSize += 1;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveUnaryExpression(contentOfSourceFile.substr(resultSize));

    return resultSize;
}

size_t ReadIfDirectiveEqualityExpression(const string& contentOfSourceFile)
{
    size_t resultSize = 0;

    // Alternative 1

    resultSize = ReadIfDirectiveUnaryExpression(contentOfSourceFile);

    if (resultSize == 0) {
        return 0;
    }

    // Alternatives 2 and 3 rule start

    resultSize += ReadIfDirectiveEqualityExpression(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 2) == "==") {
        // Alternative 2
        
        resultSize += 2;

        resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));
        resultSize += ReadIfDirectiveUnaryExpression(contentOfSourceFile.substr(resultSize));
    } else if (contentOfSourceFile.substr(resultSize, 2) == "!=") {
        // Alternative 3

        resultSize += 2;

        resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));
        resultSize += ReadIfDirectiveUnaryExpression(contentOfSourceFile.substr(resultSize));
    }
    
    return resultSize;
}

size_t ReadIfDirectiveAndExpression(const string& contentOfSourceFile)
{
    size_t resultSize = 0;

    // Alternative 1

    resultSize = ReadIfDirectiveEqualityExpression(contentOfSourceFile);

    if (resultSize == 0) {
        return 0;
    }

    // Alternative 2

    resultSize += ReadIfDirectiveAndExpression(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 2) != "&&") {
        return resultSize;
    }
    resultSize += 2;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveEqualityExpression(contentOfSourceFile.substr(resultSize));

    return resultSize;
}

size_t ReadIfDirectiveOrExpression(const string& contentOfSourceFile)
{
    size_t resultSize = 0;

    // Alternative 1

    resultSize = ReadIfDirectiveAndExpression(contentOfSourceFile);

    if (resultSize == 0) {
        return 0;
    }

    // Alternative 2

    resultSize += ReadIfDirectiveOrExpression(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 2) != "||") {
        return resultSize;
    }
    resultSize += 2;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveAndExpression(contentOfSourceFile.substr(resultSize));

    return resultSize;
}

size_t ReadIfDirectiveExpression(const string& contentOfSourceFile)
{
    size_t resultSize = 0;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile);
    resultSize += ReadIfDirectiveOrExpression(contentOfSourceFile.substr(resultSize));
    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    return resultSize;
}

size_t ReadIfDirectiveConditionalBlock(string contentOfSourceFile)
{
    size_t resultSize = 0, resultSizePrev = 0;
    Lexem lookAheadLexem;

    do {
        resultSizePrev = resultSize;

        lookAheadLexem = ReadLineEnding(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        lookAheadLexem = ReadWhitespace(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        lookAheadLexem = ReadComment(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        lookAheadLexem = ReadTokenLiteralRealNumber(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        lookAheadLexem = ReadTokenLiteralIntegerNumber(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        lookAheadLexem = ReadTokenLiteralString(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        lookAheadLexem = ReadTokenIdentifier(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        lookAheadLexem = ReadTokenOperator(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
            continue;
        }

        if (resultSize == 0) {
            return 0;
        }

        lookAheadLexem = ReadDirective(contentOfSourceFile);
        if (!lookAheadLexem.value.empty()) {
            resultSize += lookAheadLexem.value.length();
            contentOfSourceFile = contentOfSourceFile.substr(lookAheadLexem.value.length());
        }
    } while (resultSize != resultSizePrev);

    return resultSize;
}

size_t ReadIfDirectiveLineEnding(const string& contentOfSourceFile)
{
    size_t resultSize = 0;

    resultSize = ReadIfDirectiveWhitespace(contentOfSourceFile);
    resultSize += ReadIfDirectiveSingleLineComment(contentOfSourceFile.substr(resultSize));

    Lexem lineEnding = ReadLineEnding(contentOfSourceFile.substr(resultSize));
    if (lineEnding.value.empty()) {
        return 0;
    }

    resultSize += lineEnding.value.length();

    return resultSize;
}

string ReadIfDirectiveIfSection(const string& contentOfSourceFile)
{
    size_t resultSize = 0, addToResultSize = 0;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile);

    if (contentOfSourceFile.substr(resultSize, 1) != "#") {
        return "";
    }
    resultSize += 1;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 2) != "if") {
        return "";
    }
    resultSize += 2;

    if ((addToResultSize = ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    if ((addToResultSize = ReadIfDirectiveExpression(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    if ((addToResultSize = ReadIfDirectiveLineEnding(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    resultSize += ReadIfDirectiveConditionalBlock(contentOfSourceFile.substr(resultSize));

    return contentOfSourceFile.substr(0, resultSize);
}

string ReadIfDirectiveElifSection(const string& contentOfSourceFile)
{
    size_t resultSize = 0, addToResultSize = 0;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile);

    if (contentOfSourceFile.substr(resultSize, 1) != "#") {
        return "";
    }
    resultSize += 1;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 4) != "elif") {
        return "";
    }
    resultSize += 4;

    if ((addToResultSize = ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    if ((addToResultSize = ReadIfDirectiveExpression(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    if ((addToResultSize = ReadIfDirectiveLineEnding(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    resultSize += ReadIfDirectiveConditionalBlock(contentOfSourceFile.substr(resultSize));

    return contentOfSourceFile.substr(0, resultSize);
}

string ReadIfDirectiveElseSection(const string& contentOfSourceFile)
{
    size_t resultSize = 0, addToResultSize = 0;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile);

    if (contentOfSourceFile.substr(resultSize, 1) != "#") {
        return "";
    }
    resultSize += 1;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 4) != "else") {
        return "";
    }
    resultSize += 4;

    if ((addToResultSize = ReadIfDirectiveLineEnding(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    resultSize += ReadIfDirectiveConditionalBlock(contentOfSourceFile.substr(resultSize));

    return contentOfSourceFile.substr(0, resultSize);
}

string ReadIfDirectiveEndifSection(const string& contentOfSourceFile)
{
    size_t resultSize = 0, addToResultSize = 0;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile);

    if (contentOfSourceFile.substr(resultSize, 1) != "#") {
        return "";
    }
    resultSize += 1;

    resultSize += ReadIfDirectiveWhitespace(contentOfSourceFile.substr(resultSize));

    if (contentOfSourceFile.substr(resultSize, 5) != "endif") {
        return "";
    }
    resultSize += 5;

    if ((addToResultSize = ReadIfDirectiveLineEnding(contentOfSourceFile.substr(resultSize))) == 0) {
        return "";
    }
    resultSize += addToResultSize;

    return contentOfSourceFile.substr(0, resultSize);
}

Lexem ReadIfDirective(string contentOfSourceFile)
{
    string resultValue = "";

    // Read if section

    resultValue += ReadIfDirectiveIfSection(contentOfSourceFile);
    contentOfSourceFile = contentOfSourceFile.substr(resultValue.length());

    if (resultValue.empty()) {
        return Lexem();
    }

    // Read elif sections

    for (;;) {
        string elIfSection = ReadIfDirectiveElifSection(contentOfSourceFile);
        if (elIfSection.empty()) {
            break;
        } else {
            resultValue += elIfSection;
            contentOfSourceFile = contentOfSourceFile.substr(elIfSection.length());
        }
    }

    // Read else section

    string elseSection = ReadIfDirectiveElseSection(contentOfSourceFile);
    if (!elseSection.empty()) {
        resultValue += elseSection;
        contentOfSourceFile = contentOfSourceFile.substr(elseSection.length());
    }

    // Read endif section

    string endifSection = ReadIfDirectiveEndifSection(contentOfSourceFile);
    if (endifSection.empty()) {
        return Lexem();
    }
    resultValue += endifSection;

    Lexem resultLexem;
    resultLexem.type = Lexem::LEXEM_DIRECTIVE;
    resultLexem.spawningRuleName = "DIRECTIVE_IF";
    resultLexem.value = resultValue;

    return resultLexem;
}
