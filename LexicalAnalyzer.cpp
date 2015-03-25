#include <sstream>
#include <iostream>

#include <ExcelFormat.h>

#include "LexicalAnalyzer.h"
#include "LexicalAnalyzerException.h"
#include "Lexem.h"
#include "LexicalRulesBase.h"

using namespace std;
using namespace ExcelFormat;

void ExtractTokensFromLexemsList(const vector<Lexem>& lexems, vector<Lexem>& tokens)
{
    copy_if(lexems.begin(), lexems.end(), back_inserter(tokens), [](const Lexem& lexem) {
        switch (lexem.type) {
        case Lexem::LEXEM_TOKEN_IDENTIFIER:
        case Lexem::LEXEM_TOKEN_KEYWORD:
        case Lexem::LEXEM_TOKEN_LITERAL_INTEGER_NUMBER:
        case Lexem::LEXEM_TOKEN_LITERAL_REAL_NUMBER:
        case Lexem::LEXEM_TOKEN_LITERAL_STRING:
        case Lexem::LEXEM_TOKEN_OPERATOR:
            return true;
        }

        return false;
    });
}

Lexem GetNextLexem(string& contentOfSourceFile)
{
    Lexem lexemLineEnding = ReadLineEnding(contentOfSourceFile);

    if (!lexemLineEnding.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(lexemLineEnding.value.length());

        return lexemLineEnding;
    }

    Lexem lexemWhitespace = ReadWhitespace(contentOfSourceFile);

    if (!lexemWhitespace.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(lexemWhitespace.value.length());

        return lexemWhitespace;
    }

    Lexem tokenLiteralRealNumber = ReadTokenLiteralRealNumber(contentOfSourceFile);

    if (!tokenLiteralRealNumber.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(tokenLiteralRealNumber.value.length());

        return tokenLiteralRealNumber;
    }

    Lexem tokenLiteralIntegerNumber = ReadTokenLiteralIntegerNumber(contentOfSourceFile);

    if (!tokenLiteralIntegerNumber.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(tokenLiteralIntegerNumber.value.length());

        return tokenLiteralIntegerNumber;
    }

    Lexem tokenLiteralString = ReadTokenLiteralString(contentOfSourceFile);

    if (!tokenLiteralString.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(tokenLiteralString.value.length());

        return tokenLiteralString;
    }

    Lexem tokenIdentifier = ReadTokenIdentifier(contentOfSourceFile);

    if (!tokenIdentifier.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(tokenIdentifier.value.length());

        if (IsKeyword(tokenIdentifier.value)) {
            tokenIdentifier.type = Lexem::LEXEM_TOKEN_KEYWORD;
        }

        return tokenIdentifier;
    }

    Lexem lexemComment = ReadComment(contentOfSourceFile);

    if (!lexemComment.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(lexemComment.value.length());

        return lexemComment;
    }

    Lexem tokenOperator = ReadTokenOperator(contentOfSourceFile);

    if (!tokenOperator.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(tokenOperator.value.length());

        return tokenOperator;
    }

    Lexem lexemDirective = ReadDirective(contentOfSourceFile);

    if (!lexemDirective.value.empty()) {
        contentOfSourceFile = contentOfSourceFile.substr(lexemDirective.value.length());

        return lexemDirective;
    }

    string codeHint = contentOfSourceFile.length() > 200 ? contentOfSourceFile.substr(0, 200) + "..." : contentOfSourceFile;
    throw LexicalAnalyzerException("Error: No rule was found for the code\n\n" + codeHint + "\n");
}

void WriteLexemsToXLS(const vector<Lexem>& lexems, const char* filepath)
{
    BasicExcel xlsFile;
    BasicExcelWorksheet* worksheet = xlsFile.AddWorksheet("Lexems");

    worksheet->Cell(0, 0)->Set(L"Значение");
    worksheet->Cell(0, 1)->Set(L"Тип");
    worksheet->Cell(0, 2)->Set(L"Правило породившее лексему");
    worksheet->Cell(0, 3)->Set(L"Расположение");

    int row = 1;
    for (vector<Lexem>::const_iterator lexem = lexems.begin(), lexemLast = lexems.end(); lexem != lexemLast; ++lexem, ++row) {
        worksheet->Cell(row, 0)->Set(lexem->value.c_str());
        worksheet->Cell(row, 1)->Set(TypeOfLexemToString(lexem->type).c_str());
        worksheet->Cell(row, 2)->Set(lexem->spawningRuleName.c_str());

        stringstream positionString;
        positionString << lexem->lineNumber << ":" << lexem->offsetFromLineStart;

        worksheet->Cell(row, 3)->Set(positionString.str().c_str());
    }

    worksheet->SetColWidth(0, 15000);
    worksheet->SetColWidth(1, 15000);
    worksheet->SetColWidth(2, 15000);
    worksheet->SetColWidth(3, 10000);

    xlsFile.SaveAs(filepath);
}

bool GetLexemsList(const string& originalSourceCode, vector<Lexem>& lexems)
{
    string notYetParsedSourceCode = originalSourceCode;
    size_t offsetToNextLexem = 0;
    size_t lastLineEndingOffset = 0;
    size_t lineEndingsCount = 0;

    while (notYetParsedSourceCode.size() != 0) {
        try {
            // Read next lexem and add it to the list of lexems

            Lexem newLexem = GetNextLexem(notYetParsedSourceCode);
            newLexem.lineNumber = lineEndingsCount + 1;
            newLexem.offsetFromLineStart = offsetToNextLexem - lastLineEndingOffset;

            lexems.push_back(newLexem);

            // Calculate position of next lexem

            offsetToNextLexem += newLexem.value.length();

            while ((lastLineEndingOffset = originalSourceCode.find('\n', lastLineEndingOffset)) < offsetToNextLexem) {
                lastLineEndingOffset += 1;
                ++lineEndingsCount;
            }

            lastLineEndingOffset = lineEndingsCount < 1 ? 0 : originalSourceCode.rfind('\n', lastLineEndingOffset - 1) + 1;

        } catch (LexicalAnalyzerException& exception) {
            cout << "Unhandled exception at the position [" << lineEndingsCount + 1 << ":" << offsetToNextLexem - lastLineEndingOffset << "]: " << endl << exception << endl;

            return false;
        }
    }

    return true;
}