#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "LexicalAnalyzer.h"

using namespace std;

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