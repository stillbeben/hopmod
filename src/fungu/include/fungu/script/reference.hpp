/*   
 *   The Fungu Scripting Engine
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
    result_type eval(env::frame * frame);
    std::string form_source()const;
};

#endif
