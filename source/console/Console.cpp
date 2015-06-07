#include <iostream>
#include <fstream>
#include <vector>

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "FileSystemOperations.h"
#include "IntermediateCodeTriads.h"
#include "SymbolsTable.h"

using namespace std;

SymbolsTable symbolsTable;

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

    // Write lexical analysis results to .XLS file

    WriteLexemsToXLS(lexems, "lexems.xls");

    // Do syntax analysis

    vector<SyntaxRule> syntaxRules;
    if (!LoadSyntaxRules("../grammars/syntax-grammar.txt", syntaxRules) || syntaxRules.empty()) {
        cout << "Error during loading syntax rules base" << endl;

        return 2;
    }

    SyntaxParseTree syntaxTree = DoSyntaxAnalysis(tokens, syntaxRules, syntaxRules[0].GetName());

    // Write syntax analysis results to text file

    ofstream textualResults("parse-results.txt", ios_base::out);

    if (syntaxTree.Empty()) {
        textualResults << "Syntax error: Unable to spawn program's text." << endl << endl;
    } else {
        textualResults << "File has been parsed successfully." << endl << endl;
    }

    DisplaySyntaxParseTreeAsText(textualResults, syntaxTree);

    try {
        BuildSymbolsTable(syntaxTree, tokens, symbolsTable);
        WriteSymbolsTableToXLS(symbolsTable, "symbols.xls");
    } catch (SymbolsTableGenerationError& error) {
        cout << error << endl;

        return 3;
    }

    textualResults << endl;

    IntermediateCodeParseTreeNode intermediateCodeParseTreeRoot;

    try {
        BuildIntermediateCodeParseTree(syntaxTree.GetRoot(), tokens, intermediateCodeParseTreeRoot);

        DisplayIntermediateTreeAsText(textualResults, intermediateCodeParseTreeRoot);
        textualResults << endl;

        map<int, Triad> triads;
        BuildTriadsTable(intermediateCodeParseTreeRoot, symbolsTable, triads);

        // Display triads table
        for each (auto triad_pair in triads) {
            Triad triad = triad_pair.second;

            textualResults << "(" << triad.id << "; " << triad.operation << "; " << triad.first << "; " << triad.second << ")" << endl;
        }
    } catch (string& error) {
        cout << error << endl;

        return 4;
    }

    return 0;
}