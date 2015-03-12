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
 
 Functions for lexical analysis of C# language.
 
********************************************************************************************/

#ifndef C_SHARP_LEXICAL_ANALYSER_H
#define C_SHARP_LEXICAL_ANALYSER_H

#include <string>
#include <vector>

#include "Lexem.h"

using std::string;
using std::vector;

//! Get lexem from C# source code. It can throw LexicalAnalyzerException in the case of error.
/*!
\param[in,out] contentOfSourceFile string that contains source of the program in C#. After parsing it is modified so first found lexem is dropped off from this content.
\return First found lexem or empty lexem if non correct lexem was found.
\sa LexicalAnalyzerException
*/
Lexem GetNextLexem(string& contentOfSourceFile);

//! Do lexical analysis on C# source code.
/*!
\param[in] originalSourceCode string that contains source of the program in C#.
\param[out] lexems list of lexems.
\return True if parsing was successful or false if not.
\sa GetNextLexem()
*/
bool GetLexemsList(const string& originalSourceCode, vector<Lexem>& lexems);

//! Select only significant lexems (just tokens) from the list of lexems.
/*!
\param[in] lexems list of lexems.
\param[out] tokens list of tokens.
\sa GetLexemsList()
*/
void ExtractTokensFromLexemsList(const vector<Lexem>& lexems, vector<Lexem>& tokens);

//! Create new .XLS file and write lexems to it.
/*!
\param lexems list of lexems.
\param filepath path to the .XLS file.
\sa GetLexemsList(), ExtractTokensFromLexemsList()
*/
void WriteLexemsToXLS(const vector<Lexem>& lexems, const char* filepath);

#endif // C_SHARP_LEXICAL_ANALYSER_H