#include "SyntaxRule.h"

const string SyntaxRule::IDENTIFIER_NONKEYWORD_TERMINAL = "-any-id-";
const string SyntaxRule::LITERAL_TERMINAL = "-some-literal-";

SyntaxRule::SyntaxRule(const string& name, const string& firstNonTerminal, const string& secondNonTerminal) :
m_name(name), m_firstItem(firstNonTerminal), m_secondItem(secondNonTerminal), m_type(TWO_NONTERMINALS)
{
}

SyntaxRule::SyntaxRule(const string& name, const string& terminal) :
m_name(name), m_firstItem(terminal), m_type(ONE_TERMINAL)
{
}

bool SyntaxRule::IsTerminalRule() const
{
    return m_type == ONE_TERMINAL;
}

string SyntaxRule::GetFirstPartOfRule() const
{
    return m_firstItem;
}

string SyntaxRule::GetSecondPartOfRule() const
{
    return m_secondItem;
}

string SyntaxRule::GetName() const
{
    return m_name;
}

