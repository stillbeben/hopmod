
#ifndef HOPMOD_TOOLS_HPP
#define HOPMOD_TOOLS_HPP

#include <list>
#include <string>

template<typename T>
class assign
{
public:
    assign(T value):m_value(value){}
    void operator()(T & ref){ref=m_value;}
private:
    T m_value;
};

#endif
