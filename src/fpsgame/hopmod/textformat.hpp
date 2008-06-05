
#ifndef HOPMOD_TEXTFORMAT_HPP
#define HOPMOD_TEXTFORMAT_HPP

#include <ostream>

enum ConsoleColourCode
{
    ConColour_Green = '0',
    ConColour_Blue,
    ConColour_Yellow,
    ConColour_Red,
    ConColour_Grey,
    ConColour_Magenta,
    ConColour_Orange,
    
    ConColour_Save = 's',
    ConColour_Restore = 'r',
    
    ConColour_Player = ConColour_Green,
    ConColour_Info = ConColour_Blue,
    ConColour_Gameplay = ConColour_Yellow,
    ConColour_Error = ConColour_Red
};

std::ostream & operator<<(std::ostream &,ConsoleColourCode);

#endif
