#include <fstream>
#include <iostream>

#include "FileSystemOperations.h"

using std::ifstream;
using std::ios_base;
using std::string;

string LoadFileContentFromDiskToMemory(const char* filepath)
{
    ifstream in(filepath, ios_base::in | ios_base::binary | ios_base::ate);

    if (!in.good()) {
        return string();
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
