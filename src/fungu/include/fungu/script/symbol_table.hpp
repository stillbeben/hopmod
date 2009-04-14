/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_SYMBOL_TABLE_HPP
#define FUNGU_SCRIPT_SYMBOL_TABLE_HPP

#include "../string.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/function.hpp>
#include <algorithm>

namespace fungu{
namespace script{

template<typename SymbolType>
class symbol_table
{
    // wasn't able to use tuple type with the multi_index container because of const issue
    struct element
    {
        element(const_string aId,SymbolType * aObject)
         :id(aId),object(aObject){}
        const_string id;
        SymbolType * object;
    };
    
    typedef boost::multi_index::multi_index_container<
        element,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                BOOST_MULTI_INDEX_MEMBER(element,const_string,id)
            >
        >
    > symbol_container;
    
    typedef typename boost::multi_index::nth_index<symbol_container,0>::type hash_index;
    
    symbol_container m_symbols;
public:
    symbol_table(SymbolType default_value)
     :m_default_value(default_value)
    {
        
    }
    
    ~symbol_table()
    {
        // IGNORE MEMORY LEAK WARNING
        
        //Do not delete allocated objects because this class is used a static data member in the env class and
        //and the undefined destruction order can cause dangling pointers.
    }
    
    void clear()
    {
        for(typename hash_index::iterator it = m_symbols.begin(); it != m_symbols.end(); ++it)
            delete it->object;
    }
    
    SymbolType * lookup_symbol(const_string id)
    {
        typename hash_index::const_iterator it = m_symbols.find(id);
        if(it == m_symbols.end()) return NULL;
        return it->object;
    }
    
    SymbolType * register_symbol(const_string id)
    {
        std::pair<typename symbol_container::iterator,bool> insert = m_symbols.insert(element(id,NULL));
        if(insert.second) m_symbols.replace(insert.first,element(id,new SymbolType(m_default_value)));
        return insert.first->object;
    }
private:
    SymbolType m_default_value;
};

} //namespace script
} //namespace fungu

#endif
