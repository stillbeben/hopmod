/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

template<typename Terms>
result_type expression::reference<Terms>::eval(env::frame * frame)
{
    return symbol<Terms>::resolve_symbol(frame)->get_shared_ptr();
}

template<typename Terms>
std::string expression::reference<Terms>::form_source()const
{
    symbol<Terms>::assert_parsed();
    return (std::string(1,'@') + symbol<Terms>::get_full_id().copy());
}    

template class expression::reference<expression::word_exit_terminals>;

} //namespace script
} //namespace fungu
