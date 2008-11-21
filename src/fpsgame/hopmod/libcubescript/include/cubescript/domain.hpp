/*   libcubescript - A CubeScript engine.  */
/*
 *  Copyright (c) 2008 Graham Daws. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *          
 *      2. Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *         
 *      3. The names of the contributors and copyright holders must not be used
 *         to endorse or promote products derived from this software without
 *         specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef LIBCUBESCRIPT_DOMAIN_HPP
#define LIBCUBESCRIPT_DOMAIN_HPP

#include <map>
#include <boost/shared_ptr.hpp>
#include "predefs.hpp"
#include "error.hpp"

namespace cubescript{

/*!
    @brief Holds a set of symbols.
    
    The domain class is useful for grouping related symbols and restricting
    use of these symbols to a specific environment.
*/
class domain
{
public:
    enum
    {
        TEMPORARY_DOMAIN=1
    };
    
    domain(int flags=0);
    domain(domain *,int flags=0);
    domain(const domain &);
    
    ~domain();
    
    domain * get_parent()const;
    
    void unite(const domain *);
    
    /*!
        @brief Register a symbol.

        @param id name of symbol
        @param s pointer to symbol object.
    
        The domain class does not hold responsibilty for the deletion of its
        registered symbol objects - the domain maintains a weak ownership of
        these symbols. Note: registered symbols MUST live as long as the 
        dependant domain.
        
        If the given id argument conflicts with a prior symbol
        registration, the existing symbol reference will be overwritten.
    */
    void register_symbol(const std::string & id,symbol * s,int flags=0);
    
    void register_symbol(const char * id,symbol * s,int flags=0);
    
    enum { SEARCH_PARENTS=1,        // search ancestors
           SEARCH_LOCAL_ONLY=2,     // search only this domain
           REGISTER_ROOT=8,         // register symbol to root parent
           REGISTER_FIRSTNTMP=16,   // register symbol to first non-temporary
           ADOPT_SYMBOL=32,         // assume ownership of symbol object
           HIDE_FROM_DESCENDANTS=64,// hide symbol from descendant domains
           DEFAULT_FLAGS=SEARCH_PARENTS | REGISTER_ROOT
         };
    
    /*!
        @brief Search for a symbol.

        @param id name of symbol.
        @param search_parents search ancestor domains.
        @return pointer to a symbol object, if symbol not found the return alue is NULL.
    */
    inline
     symbol * lookup_symbol(const std::string & id,int search=SEARCH_PARENTS)const
    {
        return lookup_symbol(id,search,0);
    }
    
    /*!
        @brief Same as lookup symbol method except if symbol not found
               unknown_symbol exception is thrown.
    */
    symbol * require_symbol(const std::string & id,int search=SEARCH_PARENTS)const;
    
    /*!
        @brief Save/Serialize the persistable symbols to the given output
               stream.
        
        A symbol that is inherited from symbol::persistable is able to write
        a representation of itself in the form of CubeScript syntax.
    */
    void save(std::ostream &)const;
    
    /*!
        @brief Load symbols from input stream into domain object.
        
        This is nothing more than a helper function, the exec_script function
        is used to load the represented symbols, which are stored in CubeScript
        syntax.
        
        @see save
    */
    void load(std::istream &);
    
    template<typename SymbolType>
    SymbolType * get_symbol_of_kind(const std::string & id,int search=DEFAULT_FLAGS)
    {
        SymbolType * s=dynamic_cast<SymbolType *>(require_symbol(id,search));
        if(!s) throw error_key("runtime.wrong_symbol_type");
        return s;
    }
    
    template<typename SymbolType>
    SymbolType * get_or_create_symbol(const std::string & name,const SymbolType & value,int flags=DEFAULT_FLAGS)
    {
        symbol * symbolPtr=lookup_symbol(name,flags);
        if(!symbolPtr)
        {
            symbolPtr=new SymbolType(value);
            register_symbol(name,symbolPtr,flags | ADOPT_SYMBOL);
        }
        SymbolType * castedSymbol=dynamic_cast<SymbolType *>(symbolPtr);
        if(!castedSymbol) throw error_key("runtime.symbol.override_forbidden");
        return castedSymbol;
    }
    
    bool is_temporary()const{return m_temporary;}
    domain * get_first_nontmp_domain()
    {
        domain * current=this;
        while(current->m_temporary && current->m_parent) current=current->m_parent;
        if(current && current->m_temporary) current=NULL;
        return current;
    }
    
    bool link(domain *);
    void unlink();
private:
    struct ltstr
    {
        bool operator()(const char* s1, const char* s2)const{return strcmp(s1, s2) < 0;}
    };
    struct symbol_node
    {
        symbol * m_symbol;
        boost::shared_ptr<symbol> m_symbol_owner;
        const std::string * m_id_alloc;
        bool m_hide_from_descendants; //symbol is invisible from descendant domain lookup
    };
    void register_symbol(const char *,symbol_node &,int flags);
    symbol * lookup_symbol(const std::string & id,int search,int steps)const;
    typedef std::map<const char *,symbol_node,ltstr> symbol_map;
    symbol_map m_symbols;
    std::list<std::string> m_id_allocs;
    domain * m_parent;
    bool m_temporary;
    bool m_link;
};

} //namespace cubescript

#endif
