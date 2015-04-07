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
 
 Non-terminal position for CYK algorithm for syntax parsing.
 
********************************************************************************************/

#ifndef C_SHARP_NON_TERMINAL_POSITION_TYPE_H
#define C_SHARP_NON_TERMINAL_POSITION_TYPE_H

#include <string>

using std::string;

struct NonTerminalPositionType {
    int i;
    int j;
    string name;

    NonTerminalPositionType(int _i, int _j, const string& _name);

    bool operator<(const NonTerminalPositionType& rhs) const;
    bool operator==(const NonTerminalPositionType& rhs) const;
    bool operator!=(const NonTerminalPositionType& rhs) const;
    bool operator>(const NonTerminalPositionType& rhs) const;

    friend std::ostream& operator<<(std::ostream& stream, const NonTerminalPositionType& nonTerminalPosition);
};

extern NonTerminalPositionType NullPosition;

#endif // C_SHARP_NON_TERMINAL_POSITION_TYPE_H