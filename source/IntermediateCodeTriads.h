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
 
 This unit contains functions to generate intermediate code in triads from intermediate parse tree.
 
********************************************************************************************/

#ifndef CSHARP_INTERMEDIATE_CODE_TRIADS_H
#define CSHARP_INTERMEDIATE_CODE_TRIADS_H

#include <string>
#include <map>

#include "SymbolsTable.h"
#include "IntermediateCodeParseTree.h"

using std::string;
using std::map;

struct Triad
{
    int id;
    string operation;
    string first;
    string second;
};

SymbolVariable* FindVariable(const string& variableName, int methodId, int currentPosition, SymbolsTable& symbolsTable, map<int, Triad>& triads);
void BuildTriadsTable(const IntermediateCodeParseTreeNode& intermediateTreeRoot, SymbolsTable& symbolsTable, map<int, Triad>& triadsTable);

#endif //CSHARP_INTERMEDIATE_CODE_TRIADS_H