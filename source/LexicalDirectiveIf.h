#ifndef CSHARP_LEXICAL_ANALYZER_DIRECTIVEIF_H
#define CSHARP_LEXICAL_ANALYZER_DIRECTIVEIF_H

#include <string>

#include "Lexem.h"

using std::string;

Lexem ReadIfDirective(string contentOfSourceFile);

#endif //CSHARP_LEXICAL_ANALYZER_DIRECTIVEIF_H