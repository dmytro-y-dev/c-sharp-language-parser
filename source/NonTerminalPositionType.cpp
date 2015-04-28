#include <ostream>

#include "NonTerminalPositionType.h"

using std::ostream;

NonTerminalPositionType NullPosition(-1, -1, "");

NonTerminalPositionType::NonTerminalPositionType(int _i, int _j, const string& _name) :
i(_i), j(_j), name(_name)
{
}

bool NonTerminalPositionType::operator<(const NonTerminalPositionType& rhs) const
{
    return this->i < rhs.i || (!(rhs.i < this->i) && this->j < rhs.j) || (!(rhs.i < this->i) && !(rhs.j < this->j) && this->name < rhs.name);
}

bool NonTerminalPositionType::operator==(const NonTerminalPositionType& rhs) const
{
    return this->i == rhs.i && this->j == rhs.j && this->name == rhs.name;
}

bool NonTerminalPositionType::operator!=(const NonTerminalPositionType& rhs) const
{
    return !this->operator==(rhs);
}

bool NonTerminalPositionType::operator>(const NonTerminalPositionType& rhs) const
{
    return !(this->operator<(rhs) || this->operator==(rhs));
}

std::ostream& operator<<(std::ostream& stream, const NonTerminalPositionType& nonTerminalPosition)
{
    stream << nonTerminalPosition.i << ":" << nonTerminalPosition.j << "=" << nonTerminalPosition.name;

    return stream;
}