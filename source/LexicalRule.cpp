#include "LexicalRule.h"

using namespace std;

LexicalRule::LexicalRule(const string& name, const string& regex) :
    m_name(name), m_template(regex)
{
}

bool LexicalRule::Match(const string& value) const
{
    return regex_match(value, m_template);
}

string LexicalRule::GetName() const
{
    return m_name;
}