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
 
 Class which describes syntax rules
 
********************************************************************************************/

#ifndef C_SHARP_SYNTAX_RULE_H
#define C_SHARP_SYNTAX_RULE_H

#include <string>

using std::string;

class SyntaxRule
{
public:
    enum RuleType {
        UNKNOWN = 0,
        TWO_NONTERMINALS = 1,
        ONE_TERMINAL = 2
    };

    static const string IDENTIFIER_NONKEYWORD_TERMINAL;
    static const string LITERAL_TERMINAL;

private:
    string m_name;
    RuleType m_type;

    string m_firstItem;
    string m_secondItem;

public:
    SyntaxRule(const string& name, const string& terminal);
    SyntaxRule(const string& name, const string& firstNonTerminal, const string& secondNonTerminal);

    bool IsTerminalRule() const;

    string GetFirstPartOfRule() const;
    string GetSecondPartOfRule() const;
    string GetName() const;
};

#endif // C_SHARP_SYNTAX_RULE_H
