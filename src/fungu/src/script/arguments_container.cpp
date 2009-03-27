/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/arguments_container.hpp"

namespace fungu{
namespace script{

arguments_container::arguments_container()
{
    
}

void arguments_container::push_back(const arguments_container::value_type & arg)
{
    int index = m_front - m_arguments.begin();
    m_arguments.push_back(arg);
    m_front = m_arguments.begin() + index;
}

void arguments_container::push_back_placeholder()
{
    int index = m_next_placeholder - m_placeholders.begin();
    push_back(any());
    m_placeholders.push_back(m_arguments.size()-1);
    m_next_placeholder = m_placeholders.begin() + index;
}

void arguments_container::assign_next_placeholder(const arguments_container::value_type & arg)
{
    assert(m_next_placeholder < m_placeholders.end());
    m_arguments[*m_next_placeholder] = arg;
    m_next_placeholder++;
}

arguments_container::value_type & arguments_container::front()
{
    return *m_front;
}

arguments_container::value_type & arguments_container::back()
{
    return m_arguments.back();
}

any & arguments_container::front_reference()
{
    return *m_front;
}

arguments_container::value_type & arguments_container::safe_front()
{
    if(empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    return *m_front;
}

void arguments_container::pop_front()
{
    assert(m_front < m_arguments.end());
    m_front++;
}

std::size_t arguments_container::size()const
{
    return m_arguments.size() - (m_front - m_arguments.begin());
}

bool arguments_container::empty()const
{
    return !size();
}

void arguments_container::reset()
{
    m_front = m_arguments.begin();
    m_next_placeholder = m_placeholders.begin();
    
    for(std::vector<int>::const_iterator it = m_placeholders.begin(); 
        it != m_placeholders.end(); it++)
    {
        m_arguments[*it] = any();
    }
}

void arguments_container::clear()
{
    m_arguments.clear();
    m_placeholders.clear();
    reset();
}

} //namespace script
} //namespace fungu
