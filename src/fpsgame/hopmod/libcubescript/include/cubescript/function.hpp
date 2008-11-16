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
#ifndef LIBCUBESCRIPT_FUNCTION_HPP
#define LIBCUBESCRIPT_FUNCTION_HPP

#include <boost/function.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/bind.hpp>

#include "predefs.hpp"
#include "error.hpp"
#include "symbol.hpp"
#include "pointer.hpp"
#include "alias.hpp"

namespace cubescript{

template<typename T>
struct remove_const_reference{typedef typename boost::remove_const<typename boost::remove_reference<T>::type>::type type;};

template<typename T>
struct mutable_value{typedef typename remove_const_reference<T>::type type;};

#define DEFINE_TYPE_TRAIT(name,type) \
    template<typename T> struct name:public boost::false_type{}; \
    template<> struct name<type>:public boost::true_type{};

DEFINE_TYPE_TRAIT(is_alias_function,alias_function);
DEFINE_TYPE_TRAIT(is_const_cstr,const char *);

template<typename T> struct is_shared_ptr:public boost::false_type{};
template<typename T> struct is_shared_ptr< boost::shared_ptr<T> >:public boost::true_type{};

class functionN:public symbol
{
public:
    std::string value()const
    {
        throw error_key("syntax.function.no_value");
        return "";
    }
    
    template<typename T> inline static
    T pop_arg(std::list<std::string> & args)
    {
        if(args.empty()) throw error_key("syntax.missing_arguments");
        T tmp=parse_type<T>(args.front());
        args.pop_front();
        return tmp;
    }
    
    template<typename T>
    class value_argument
    {
    public:
        value_argument(std::list<std::string> & arglist,domain * aDomain)
         :m_value(functionN::pop_arg<T>(arglist)){}
        operator T &(){return m_value;}
    private:
        T m_value;
    };
    
    class alias_function_argument
    {
    public:
        alias_function_argument(std::list<std::string> & arglist,domain * aDomain)
         :m_func(functionN::pop_arg<std::string>(arglist),aDomain){}
        operator alias_function &(){return m_func;}
    private:
        alias_function m_func;
    };
    
    class const_cstr_argument
    {
    public:
        const_cstr_argument(std::list<std::string> & arglist,domain *)
         :m_value(functionN::pop_arg<std::string>(arglist)){}
        operator const char *(){return m_value.c_str();}
    private:
        std::string m_value;
    };
    
    template<typename T>
    class pointer_argument
    {
    public:
        typedef typename T::element_type element_type;
        pointer_argument(std::list<std::string> & arglist,domain * aDomain)
        {
            m_ptr=aDomain->get_symbol_of_kind< pointer<element_type> >(functionN::pop_arg<std::string>(arglist))->get();
        }
        operator T(){return m_ptr;}
    private:
        T m_ptr;
    };
    
    template<typename T>
    struct argument_handler
    {
        typedef typename mutable_value<T>::type value_type;
        typedef typename 
        boost::mpl::if_<
            boost::mpl::bool_<is_alias_function<value_type>::value>,
            alias_function_argument,
            typename boost::mpl::if_<
                boost::mpl::bool_<is_shared_ptr<value_type>::value>,
                pointer_argument<value_type>,
                typename boost::mpl::if_<
                    boost::mpl::bool_<is_const_cstr<T>::value>,
                    const_cstr_argument,
                    value_argument<value_type>
                >::type
            >::type
        >::type type;
    };
    
    struct void_call
    {
        template<typename FuncT> inline
            std::string invoke(FuncT func){func(); return "";}
    };
    
    template<typename Ret>
    struct nonvoid_call
    {
        template<typename FuncT> inline
            std::string invoke(FuncT func){return print_type<Ret>(func());}
    };
    
    template<typename Ret>
    struct call_handler
    {
        typedef typename
        boost::mpl::if_<
            boost::mpl::bool_<boost::is_void<Ret>::value>,
            void_call,
            nonvoid_call<Ret>
        >::type type;
    };
};


template<typename RetT>
class function0:public functionN
{
public:
    template<typename F>
    function0(F func):m_func(func){}
    std::string apply(std::list<std::string> & args,domain *)
    {
        typename call_handler<RetT>::type invoker;
        return invoker.invoke(m_func);
    }
private:
    boost::function0<RetT> m_func;
};

template<typename RetT,typename Arg1_T>
class function1:public functionN
{
public:
    template<typename F>
    function1(F func):m_func(func){}
    std::string apply(std::list<std::string> & args,domain * aDomain)
    {
        typename argument_handler<Arg1_T>::type arg1(args,aDomain);
        typename call_handler<RetT>::type invoker;
        return invoker.invoke(boost::bind(m_func,arg1));
    }
private:
    boost::function1<RetT,Arg1_T> m_func;
};

template<typename RetT,typename Arg1_T,typename Arg2_T>
class function2:public functionN
{
public:
    template<typename F>
    function2(F func):m_func(func){}
    std::string apply(std::list<std::string> & args,domain * aDomain)
    {
        typename argument_handler<Arg1_T>::type arg1(args,aDomain);
        typename argument_handler<Arg2_T>::type arg2(args,aDomain);
        typename call_handler<RetT>::type invoker;
        return invoker.invoke(boost::bind(m_func,arg1,arg2));
    }
private:
    boost::function2<RetT,Arg1_T,Arg2_T> m_func;
};

template<typename RetT,typename Arg1_T,typename Arg2_T,typename Arg3_T>
class function3:public functionN
{
public:
    template<typename F>
    function3(F func):m_func(func){}
    std::string apply(std::list<std::string> & args,domain * aDomain)
    {
        typename argument_handler<Arg1_T>::type arg1(args,aDomain);
        typename argument_handler<Arg2_T>::type arg2(args,aDomain);
        typename argument_handler<Arg3_T>::type arg3(args,aDomain);
        typename call_handler<RetT>::type invoker;
        return invoker.invoke(boost::bind(m_func,arg1,arg2,arg3));
    }
private:
    boost::function3<RetT,Arg1_T,Arg2_T,Arg3_T> m_func;
};

template<typename RetT,typename Arg1_T,typename Arg2_T,typename Arg3_T,typename Arg4_T>
class function4:public functionN
{
public:
    template<typename F>
    function4(F func):m_func(func){}
    std::string apply(std::list<std::string> & args,domain * aDomain)
    {
        typename argument_handler<Arg1_T>::type arg1(args,aDomain);
        typename argument_handler<Arg2_T>::type arg2(args,aDomain);
        typename argument_handler<Arg3_T>::type arg3(args,aDomain);
        typename argument_handler<Arg4_T>::type arg4(args,aDomain);
        typename call_handler<RetT>::type invoker;
        return invoker.invoke(boost::bind(m_func,arg1,arg2,arg3,arg4));
    }
private:
    boost::function4<RetT,Arg1_T,Arg2_T,Arg3_T,Arg4_T> m_func;
};

template<typename RetT>
class functionV:public functionN
{
public:
    template<typename F>
    functionV(F func):m_func(func){}
    std::string apply(std::list<std::string> & args,domain * d)
    {
        typename call_handler<RetT>::type invoker;
        return invoker.invoke(boost::bind(m_func,boost::ref(args),d));
    }
private:
    boost::function2<RetT,std::list<std::string> &,domain *> m_func;
};

inline void nullary(){}

class nullary_setter:public function0<void>
{
public:
    nullary_setter():function0<void>(nullary),m_called(false){}
    bool is_set()const{return m_called;}
    std::string apply(std::list<std::string> & args,domain *)
    {
        if(!args.empty()) throw error_key("syntax.too_many_arguments");
        m_called=true;
        return "";
    }
private:
    bool m_called;
};

} //namespace cubescript

#endif
