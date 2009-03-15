/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CONSTRUCT_HPP
#define FUNGU_SCRIPT_CONSTRUCT_HPP

#include "../string.hpp"
#include "env.hpp"

namespace fungu{
namespace script{

enum parse_state
{
    PARSE_NOSTATE = 0,
    PARSE_PARSING,
    PARSE_COMPLETE
};

/**
    @brief Represents a language construct.
    @see expression
*/
class construct
{
public:
    typedef const_string::const_iterator source_iterator;
    
    construct()
     :m_next_sibling(NULL)
    {
        
    }
    
    virtual ~construct()
    {
        delete m_next_sibling;
    }
    
    /**
        @brief Link to a sibling construct.
        
        Assumes ownership of the sibling object and is deleted by the host
        object when the host object goes out of scope. Supporting a singly
        linked list is useful for a construct object to reference a chain 
        of subconstructs.
    */
    void set_next_sibling(construct * next_sibling)
    {
        m_next_sibling = next_sibling;
    }
    
    /**
        @brief Get pointer to the next sibling construct.
    */
    construct * get_next_sibling()
    {
        return m_next_sibling;
    }
    
    /**
        @brief Get pointer of tail sibling.
        
        Traverses the sibling list until the tail node is reached.
    */
    construct * get_tail_sibling()
    {
        construct * tail = this;
        while(tail->get_next_sibling())
            tail = tail->get_next_sibling();
        return tail;
    }
    
    /**
        @brief Parse source code.
        @param read_ptr A pointer to the first character to be read.
        @param last_c A pointer of the last character to be read.
        @param a_frame A pointer to an environment frame.
        @return parse state code
    */
    virtual parse_state parse(source_iterator * read_ptr,source_iterator last_c,env::frame * a_frame)=0;
    
    /**
        @brief Evaluate the construct to return a value.
        @param aFrame A pointer to an environment frame.
        @return anything
    */
    virtual result_type eval(env::frame * aFrame)=0;
    
    /**
        @brief Is eval method supported?
        
        Some derived construct objects have nothing to evaluate and cannot
        return anything meaningful, a comment construct is an example of a
        non-evaluating form. Calling eval method on a non-evaluating construct
        is undefined; the method will probably return an empty string (to
        represent void) or throw an exception.
    */
    virtual bool is_eval_supported() const
    {
        return true;
    }
    
    /**
        @brief Will the eval method return a constant string?
        
        This method is useful for various optimizations.
    */
    virtual bool is_string_constant() const =0;
    
    /**
        @brief Form a source code representation of the construct.
        
        The returned output is the same or equivalent to the source code that
        was input to the parse method.
    */
    virtual std::string form_source() const =0;
    
protected:
    static inline bool is_terminator(const_string::value_type c,const const_string & termset)
    {
        for(const_string::const_iterator i=termset.begin(); i!=termset.end(); ++i)
            if(*i==c) return true;
        return false;
    }
private:
    construct(const construct &){}
    construct * m_next_sibling;
};

} //namespace script
} //namespace fungu

#endif
