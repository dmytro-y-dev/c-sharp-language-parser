/****************************************************************************************//**

 Author of this file is
     __________ _______   __   _____ __ __________ _____ __ __ _____ ______   __ ____   
    / _   _   // ___  /__/ /_ / ___ / // _   _   // ___ / // //____// ____ \ / //___ \  
   / / / / / // /__/_//_  __// /  /  // / / / / // /  /  //   /    / /___/_//   /__/_/  
  / / / / / // /_____  / /_ / /__/  // / / / / // /__/  // /\ \__ / /_____ / /\ \__     
 /_/ /_/ /_/ \______/  \__/ \____/_//_/ /_/ /_/ \____/_//_/  \___/\______//_/  \___/    
                                                                                         
 webpage: http://www.goblinov.net
 email: guru@goblinov.net   

 This file is provided under certain circumstances. For more details see LICENSE file in
 the project's root folder.
  
 \author metamaker
 \brief 
 
 Symbols table type definition and functions to work with information about symbols.
 
********************************************************************************************/

#ifndef C_SHARP_PARSER_SYMBOLS_TABLE
#define C_SHARP_PARSER_SYMBOLS_TABLE

#include <vector>
#include <string>

#include "Symbol.h"
#include "ParseTree.h"
#include "Lexem.h"

using std::vector;
using std::string;

typedef map<string, SymbolClass> SymbolsTable;

void BuildSymbolsTable(const SyntaxParseTree& syntaxTree, const vector<Lexem>& tokens, SymbolsTable& symbolsTable);

SymbolVariable* FindVariable(const string& variableName, CodeBlock* bestCorrespondingBlock, int currentPositionInCode);
CodeBlock* FindBestCorrespondingBlock(CodeBlock* block, int currentPosition);

void WriteSymbolsTableToXLS(const SymbolsTable& symbolsTable, const char* filepath);

typedef std::string SymbolsTableGenerationError;

#endif //C_SHARP_PARSER_SYMBOLS_TABLE