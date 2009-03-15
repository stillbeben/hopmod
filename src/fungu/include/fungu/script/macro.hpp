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
class macro:public construct
{
public:
    class symbol:public word<word_exit_terminals>
    {
    public:
        result_type eval(env::frame * frame)
        {
            return frame->lookup_required_object(
                any_cast<const_string>(
                    word<word_exit_terminals>::eval(frame)
                )
            )->value();
        }
    };
public:
    macro()
     :m_escape(1),m_con(NULL)
    {
        
    }
    
    ~macro()
    {
        if(m_con) delete m_con;
    }
    
    parse_state parse(source_iterator * first,source_iterator last,env::frame * frame)
    {
        if(m_con) return m_con->parse(first,last,frame);
        
        switch(**first)
        {
            case '@': m_escape++;                ++(*first); break;
            case '(': m_con = new subexpression; ++(*first); break;
            default: m_con = new symbol;
        }
        
        return parse(first,last,frame);
    }
    
    result_type eval(env::frame * frame)
    {
        assert(m_con);
        return m_con->eval(frame);
    }
    
    int get_evaluation_level()const
    {
        return m_escape;
    }
    
    bool is_string_constant()const
    {
        return m_con->is_string_constant();
    }
    
    std::string form_source()const
    {
        return std::string(m_escape,'@') + m_con->form_source();
    }
private:
    int m_escape;
    construct * m_con;
};

#endif
