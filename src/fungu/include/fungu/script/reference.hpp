/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_EXPRESSION_NESTED_CLASS

/**
    
*/
template<typename ExitTerminals>
class reference:public symbol<ExitTerminals>
{
public:
    result_type eval(env::frame * frame)
    {
        return symbol<ExitTerminals>::resolve_symbol(frame)->get_shared_ptr();
    }
    
    std::string form_source()const
    {
        symbol<ExitTerminals>::assert_parsed();
        return (std::string(1,'@') + symbol<ExitTerminals>::get_full_id().copy());
    }
};

#endif
