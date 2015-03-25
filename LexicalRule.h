#ifndef CSHARP_LEXICAL_ANALYZER_RULE_H
#define CSHARP_LEXICAL_ANALYZER_RULE_H

#include <string>
#include <regex>

#include "Lexem.h"

using std::string;
using std::regex;

class LexicalRule
{
private:
    regex m_template; //! Template that is written as regular expression. It describes the rule.
    string m_name;  //! Name of the rule

public:
    LexicalRule(const string& name, const string& regex);

    //! Check if value matches the rule.
    /*!
    \param value some string to match over rule's template.
    \return If value matches the rule than true; otherwise - false.
    */
    bool Match(const string& value) const;

    //! Get the name of this rule.
    /*!
    \return Name of this rule.
    */
    string GetName() const;
};

#endif //CSHARP_LEXICAL_ANALYZER_RULE_H