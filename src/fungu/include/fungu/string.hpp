/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_STRING_HPP
#define FUNGU_STRING_HPP

#include <assert.h>
#include <math.h>
#include <string>
#include <string.h>
#include <typeinfo>
#include <limits>

#define FUNGU_LITERAL_STRING(str) str,str+sizeof(str)-2
#define FUNGU_EMPTY_STRING "\0"+1,"\0"

namespace fungu{

template<typename T> class basic_const_string;

namespace string_detail{

template<typename T>
struct const_string_traits
{
    typedef typename T::value_type char_type;
    typedef typename T::const_iterator const_iterator;
    static const_iterator begin(const T & str){return str.begin();}
    static const_iterator end(const T & str){return str.end();}
};

template<> 
struct const_string_traits<const char *>
{
    typedef char char_type;
    typedef const char * const_iterator;
    static const_iterator begin(const char * str){return str;}
    static const_iterator end(const char * str){return str+strlen(str);}
};

template<typename ST1,typename ST2>
int comparison(ST1 str1,ST2 str2)
{
    std::size_t str1_len=0;
    std::size_t str2_len=0;
    
    typename const_string_traits<ST1>::const_iterator str1_it =
        const_string_traits<ST1>::begin(str1);
    typename const_string_traits<ST1>::const_iterator str1_end =
        const_string_traits<ST1>::end(str1);
    
    typename const_string_traits<ST2>::const_iterator str2_it =
        const_string_traits<ST2>::begin(str2);
    typename const_string_traits<ST2>::const_iterator str2_end =
        const_string_traits<ST2>::end(str2);
    
    for( ; 
        str1_it!=str1_end && str2_it!=str2_end; 
        ++str1_it, ++str2_it,++str1_len, ++str2_len )
    {
        if( *str1_it != *str2_it )
        {
            if( *str1_it < *str2_it ) return -1;
            else return 1;
        }
    }
    
    if(str1_it!=str1_end) str1_len++;
    else if(str2_it!=str2_end) str2_len++;
    
    if(str1_len==str2_len) return 0;
    else
    {
        if(str1_len < str2_len) return -1;
        else return 1;
    }
}

template<typename ST1,typename ST2>
inline bool equals(ST1 str1,ST2 str2)
{
    return comparison(str1,str2)==0;
}

template<typename ST1,typename ST2>
inline bool less_than(ST1 str1,ST2 str2)
{
    return comparison(str1,str2)==-1;
}

template<typename ST1,typename ST2>
inline bool more_than(ST1 str1,ST2 str2)
{
    return comparison(str1,str2)==1;
}

template<typename T,bool (& O)(T,T)>
struct comparator
{
    bool operator()(T s1,T s2)const
    {
        return O(s1,s2);
    }
};

} //namespace string

template<typename T>
class basic_const_string
{
public:
    typedef const T value_type;
    typedef value_type * const_iterator;
    typedef string_detail::comparator<basic_const_string<T>,string_detail::less_than> less_than_comparator;
    typedef std::size_t index_type;
    
    basic_const_string()
     :m_firstc("\0"+1),m_lastc("\0")
    {
        
    }
    
    basic_const_string(const_iterator firstc,const_iterator lastc)
     :m_firstc(firstc),m_lastc(lastc)
    {
        assert( firstc <= lastc || firstc-1==lastc /* empty string condition */);
    }
    
    basic_const_string(const std::basic_string<T> & src)
     :m_copy(src),
      m_firstc(&((const std::string &)m_copy)[0]),
      m_lastc(&((const std::string &)m_copy)[m_copy.length()-1])
    {
        
    }
    
    basic_const_string(const T * raw_string)
     :m_copy(raw_string),
      m_firstc(&((const std::string &)m_copy)[0]),
      m_lastc(&((const std::string &)m_copy)[m_copy.length()-1])
    {
        
    }
    
    basic_const_string(const basic_const_string<T> & src)
     :m_copy(src.m_copy),
      m_firstc(src.m_firstc),
      m_lastc(src.m_lastc)
    {
        m_firstc = addr(m_copy,0) + (src.m_firstc - addr(src.m_copy,0));
        m_lastc = addr(m_copy,0) + (src.m_lastc - addr(src.m_copy,0));
    }
    
    static basic_const_string<T> literal(const T * literalString)
    {
        return basic_const_string<T>(literalString, literalString + std::char_traits<T>::length(literalString) - 1);
    }
    
    const_iterator begin()const{return m_firstc;}
    const_iterator end()const{return m_lastc+1;}
    
    static const_iterator null_const_iterator()
    {
        return NULL;
    }
    
    std::size_t length()const
    {
        return m_lastc - m_firstc +1;
    }
    
    basic_const_string<T> substring(const_iterator first,const_iterator last)const
    {
        assert( first <= last &&
                first >= m_firstc && 
                first <= m_lastc &&
                last <= m_lastc );
        
        basic_const_string<T> tmp(m_copy);
        tmp.m_firstc = first;
        tmp.m_lastc = last;
        return tmp;
    }
    
    std::string copy()const
    {
        return std::string(begin(),end());
    }
    
    bool operator<(const basic_const_string<T> & operand)const
    {
        return string_detail::less_than(*this,operand);
    }
    
    bool operator==(const basic_const_string<T> & operand)const
    {
        return string_detail::equals(*this,operand);
    }
    
    template<typename IntType>
    IntType to_int()const
    {
        //TODO replace with a pow metafunction, if that's possible
        static IntType max_placeval = static_cast<IntType>(pow(10,std::numeric_limits<IntType>::digits10));
        static IntType highest_digit = std::numeric_limits<IntType>::max() / max_placeval;
        
        IntType result = 0;
        IntType placeval = 1;
        
        bool signed_ = std::numeric_limits<IntType>::is_signed;
        bool minus = signed_ && *m_firstc == '-';
        const_iterator firstc = m_firstc + minus;
        
        for(const_iterator i = m_lastc; i >= firstc; --i)
        {
            char c = i[0];
            bool last = i == firstc;
            
            if(c < '0' || c > '9') throw std::bad_cast();
            
            IntType digit = c - '0';
            if(placeval == max_placeval && digit > highest_digit) throw std::bad_cast();
            IntType tmp = placeval * digit;
            
            if( tmp > std::numeric_limits<IntType>::max() - result ) 
                throw std::bad_cast();
            result += tmp;
            
            if(placeval == max_placeval && !last) throw std::bad_cast();
            placeval *= 10;
        }
        
        if(minus) result = -result;
        
        return result;
    }
    
    template<typename IntType>
    static basic_const_string<T> from_int(IntType i)
    {
        int index = std::numeric_limits<IntType>::digits10 + 1;
        bool negative = std::numeric_limits<IntType>::is_signed && i < 0;
        if(negative)
        {
            if(i == std::numeric_limits<IntType>::min()) throw std::bad_cast();
            i = -i;
        }
        
        basic_const_string<T> result;
        result.m_copy.resize(index + 1);
        
        do
        {
            result.m_copy[index--] = '0' + (i % 10);
            i /= 10;
        }while(i);
        
        if(negative) result.m_copy[index] = '-';
        else ++index;
        
        result.m_firstc = &result.m_copy[index];
        result.m_lastc = &result.m_copy[result.m_copy.size()-1];
        
        return result;
    }
private:
    static inline const_iterator addr(const std::basic_string<T> & str,std::size_t index)
    {
        return &str[index];
    }
    
    std::basic_string<T> m_copy;
    const_iterator m_firstc;
    const_iterator m_lastc;
};

typedef basic_const_string<char> const_string;

} //namespace fungu

#endif
