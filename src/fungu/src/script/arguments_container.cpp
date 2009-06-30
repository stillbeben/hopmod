/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

arguments_container::arguments_container(std::vector<value_type> & args)
 :m_arguments(args),m_front(m_arguments.begin())
{
    
}

void arguments_container::push_back(const value_type & value)
{
    int index = m_front - m_arguments.begin();
    m_arguments.push_back(value);
    m_front = m_arguments.begin() + index;
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

} //namespace script
} //namespace fungu
