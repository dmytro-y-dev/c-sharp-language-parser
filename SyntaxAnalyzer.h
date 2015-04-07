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
 
 Functions to load syntax rules base and analyze stack of tokens. 
 
********************************************************************************************/

#ifndef C_SHARP_SYNTAX_ANALYZER_H
#define C_SHARP_SYNTAX_ANALYZER_H

#include "ParseTree.h"
#include "SyntaxRule.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

struct Lexem;

bool LoadSyntaxRules(const char *path, vector<SyntaxRule>& rules);
ParseTree DoSyntaxAnalysis(const vector<Lexem>& tokens, const vector<SyntaxRule>& rules, const string& startingSymbol);

typedef std::string SyntaxError;

#endif // C_SHARP_SYNTAX_ANALYZER_H
