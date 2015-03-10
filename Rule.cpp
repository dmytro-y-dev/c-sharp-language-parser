#include "Rule.h"

using namespace std;

Rule::Rule(const string& name, const string& regex) :
    m_name(name), m_template(regex)
{
}

bool Rule::Match(const string& value) const
{
    return regex_match(value, m_template);
}

string Rule::GetName() const
{
    return m_name;
}