#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <process.h>

#include <ExcelFormat.h>

#include "LexicalAnalyzerException.h"
#include "Lexem.h"
#include "Rule.h"
#include "RulesBase.h"

using namespace std;
using namespace ExcelFormat;

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
    throw LexicalAnalyzerException("Error: No rule was found for the code\n\n" + codeHint);
}

string LoadFileContentFromDiskToMemory(const char* filepath)
{
    ifstream in(filepath, ios_base::in | ios_base::binary | ios_base::ate);

    if (!in.good()) {
        return nullptr;
    }

    unsigned long fileLength = static_cast<unsigned long>(in.tellg());
    in.seekg(0);

    char* content = new char[fileLength + 1];
    in.read(content, fileLength);
    content[fileLength] = '\0';

    string result = content;
    delete[] content;

    return result;
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

    while (notYetParsedSourceCode.size() != 0) {
        try {
            Lexem newLexem = GetNextLexem(notYetParsedSourceCode);

            // Calculate position of lexem

            string textBeforeLexem = originalSourceCode.substr(0, offsetToNextLexem);
            size_t lineEndingsCount = 0;
            for (size_t lastPosition = 0; (lastPosition = textBeforeLexem.find('\n', lastPosition)) != textBeforeLexem.npos; lastPosition += 1, ++lineEndingsCount);
            size_t lastLineEnding = textBeforeLexem.rfind('\n');

            newLexem.lineNumber = lineEndingsCount + 1;
            newLexem.offsetFromLineStart = offsetToNextLexem - lastLineEnding;

            offsetToNextLexem += newLexem.value.length();

            // Add lexem to the list of lexems

            lexems.push_back(newLexem);
        } catch (LexicalAnalyzerException& exception) {
            string textBeforeLexem = originalSourceCode.substr(0, offsetToNextLexem);
            size_t lineEndingsCount = 0;
            for (size_t lastPosition = 0; (lastPosition = textBeforeLexem.find('\n', lastPosition)) != textBeforeLexem.npos; lastPosition += 1, ++lineEndingsCount);
            size_t lastLineEnding = textBeforeLexem.rfind('\n');

            cout << "Unhandled exception at the position [" << lineEndingsCount + 1 << ":" << offsetToNextLexem - lastLineEnding << "]: " << endl << exception << endl;
            system("pause");

            return false;
        }
    }

    return true;
}

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

int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "Error: path to C# source file was not specified." << endl;

        return 1;
    }

    // Lets analyze C# source code file
    // We start with loading source code from the file which user specified in command line

    string originalContentOfCSharpFile = LoadFileContentFromDiskToMemory(argv[1]);

    // Do lexical analysis

    vector<Lexem> lexems;
    if (!GetLexemsList(originalContentOfCSharpFile, lexems)) {
        return 1;
    }

    vector<Lexem> tokens;
    ExtractTokensFromLexemsList(lexems, tokens);

    // Write lexical analysis results to .xls file

    WriteLexemsToXLS(lexems, "lexems.xls");

    // Do syntax analysis

    // Write syntax analysis results to .xls file

    return 0;
}