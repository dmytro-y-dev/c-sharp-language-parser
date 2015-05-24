#include <iostream>
#include <fstream>
#include <vector>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "FileSystemOperations.h"
#include "SymbolsTable.h"

using namespace std;

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
    if (!DoLexicalAnalysis(originalContentOfCSharpFile, lexems)) {
        return 1;
    }

    vector<Lexem> tokens;
    ExtractTokensFromLexemsList(lexems, tokens);

    // Write lexical analysis results to .xls file

    WriteLexemsToXLS(lexems, "lexems.xls");

    // Do syntax analysis

    vector<SyntaxRule> syntaxRules;
    if (!LoadSyntaxRules("../grammars/syntax-grammar.txt", syntaxRules) || syntaxRules.empty()) {
        cout << "Error during loading syntax rules base" << endl;

        return 2;
    }

    ParseTree syntaxTree = DoSyntaxAnalysis(tokens, syntaxRules, syntaxRules[0].GetName());

    // Write syntax analysis results to text file

    ofstream syntaxResultsOut("syntax-analysis-results.txt", ios_base::out);

    if (syntaxTree.Empty()) {
        syntaxResultsOut << "Error: Unable to spawn program's text." << endl << endl;
    } else {
        syntaxResultsOut << "File has been parsed successfully." << endl << endl;
    }

    DisplayTreeAsText(syntaxResultsOut, syntaxTree);

    try {
        SymbolsTable symbolsTable;
        GenerateSymbolsTable(syntaxTree, tokens, symbolsTable);
        WriteSymbolsTableToXLS(symbolsTable, "symbols.xls");
    } catch (SymbolsTableGenerationError& error) {
        cout << error << endl;
    }

    return 0;
}