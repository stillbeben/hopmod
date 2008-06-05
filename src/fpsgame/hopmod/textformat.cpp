
#include "textformat.hpp"

std::ostream & operator<<(std::ostream & output,ConsoleColourCode code)
{
    output<<"\f"<<(char)code;
    return output;
}
