/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

callargs::callargs(std::vector<value_type> & args)
 :m_arguments(args),m_front(m_arguments.begin())
{
    
}

void callargs::push_back(const value_type & value)
{
    int index = m_front - m_arguments.begin();
    m_arguments.push_back(value);
    m_front = m_arguments.begin() + index;
}

callargs::value_type & callargs::front()
{
    return *m_front;
}

callargs::value_type & callargs::back()
{
    return m_arguments.back();
}

any & callargs::front_reference()
{
    return *m_front;
}

callargs::value_type & callargs::safe_front()
{
    if(empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    return *m_front;
}

void callargs::pop_front()
{
    assert(m_front < m_arguments.end());
    m_front++;
}

std::size_t callargs::size()const
{
    return m_arguments.size() - (m_front - m_arguments.begin());
}

bool callargs::empty()const
{
    return !size();
}

} //namespace script
} //namespace fungu
