/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_GENERIC_SCRIPT_FUNCTION_HPP
#define FUNGU_GENERIC_SCRIPT_FUNCTION_HPP

#include <boost/type_traits.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>

namespace fungu{

#ifndef HAS_TARGET_TAG_CLASS
#define HAS_TARGET_TAG_CLASS
template<typename T> struct target_tag{};
#endif

namespace detail{

template<typename Container,typename Serializer,typename ErrorException>
class base_script_function
{
public:
    virtual ~base_script_function(){}
protected:
    virtual void init(Container **, Serializer **) =0;
    virtual typename Container::value_type call(Container *) =0;
    virtual typename Container::value_type error_handler(int,ErrorException) =0;
};

#define FUNGU_INIT_SCRIPT_FUNCTION_CALL(a,s) \
    Container * a = NULL; Serializer * s = NULL; \
    init(& a , & s ); \
    int arg;

template<typename FunctionTraits,typename Container,typename Serializer,typename ErrorException>
class script_function0:public base_script_function<Container,Serializer,ErrorException>
{
public:
    typedef typename FunctionTraits::result_type result_type;
    
    result_type operator()()
    {
        FUNGU_INIT_SCRIPT_FUNCTION_CALL(args,s);
        try
        {
            arg = 0;
            return s->deserialize_return_value(call(args),target_tag<result_type>());
        }
        catch(ErrorException e){
            return s->deserialize_return_value(error_handler(arg,e),target_tag<result_type>());
        }
    }
};

template<typename FunctionTraits,typename Container,typename Serializer,typename ErrorException>
class script_function1:public base_script_function<Container,Serializer,ErrorException>
{
public:
    typedef typename FunctionTraits::result_type result_type;
    typedef typename FunctionTraits::arg1_type arg1_type;
    
    result_type operator()(arg1_type a1)
    {
        FUNGU_INIT_SCRIPT_FUNCTION_CALL(args,s);
        try
        {
            arg = 1; args->push_back(s->serialize(a1));
            arg = 0;
            return s->deserialize_return_value(call(args),target_tag<result_type>());
        }
        catch(ErrorException e){
            return s->deserialize_return_value(error_handler(arg,e),target_tag<result_type>());
        }
    }
};

template<typename FunctionTraits,typename Container,typename Serializer,typename ErrorException>
class script_function2:public base_script_function<Container,Serializer,ErrorException>
{
public:
    typedef typename FunctionTraits::result_type result_type;
    typedef typename FunctionTraits::arg1_type arg1_type;
    typedef typename FunctionTraits::arg2_type arg2_type;
    
    result_type operator()(arg1_type a1,arg2_type a2)
    {
        FUNGU_INIT_SCRIPT_FUNCTION_CALL(args,s);
        try
        {
            arg = 1; args->push_back(s->serialize(a1));
            arg = 2; args->push_back(s->serialize(a2));
            arg = 0;
            return s->deserialize_return_value(call(args),target_tag<result_type>());
        }
        catch(ErrorException e){
            return s->deserialize_return_value(error_handler(arg,e),target_tag<result_type>());
        }
    }
};

template<typename FunctionTraits,typename Container,typename Serializer,typename ErrorException>
class script_function3:public base_script_function<Container,Serializer,ErrorException>
{
public:
    typedef typename FunctionTraits::result_type result_type;
    typedef typename FunctionTraits::arg1_type arg1_type;
    typedef typename FunctionTraits::arg2_type arg2_type;
    typedef typename FunctionTraits::arg3_type arg3_type;

    result_type operator()(arg1_type a1,arg2_type a2,arg3_type a3)
    {
        FUNGU_INIT_SCRIPT_FUNCTION_CALL(args,s);
        try
        {
            arg = 1; args->push_back(s->serialize(a1));
            arg = 2; args->push_back(s->serialize(a2));
            arg = 3; args->push_back(s->serialize(a3));
            arg = 0;
            return s->deserialize_return_value(call(args),target_tag<result_type>());
        }
        catch(ErrorException e){
            return s->deserialize_return_value(error_handler(arg,e),target_tag<result_type>());
        }
    }
};

template<typename FunctionTraits,typename Container,typename Serializer,typename ErrorException>
class script_function4:public base_script_function<Container,Serializer,ErrorException>
{
public:
    typedef typename FunctionTraits::result_type result_type;
    typedef typename FunctionTraits::arg1_type arg1_type;
    typedef typename FunctionTraits::arg2_type arg2_type;
    typedef typename FunctionTraits::arg3_type arg3_type;
    typedef typename FunctionTraits::arg4_type arg4_type;

    result_type operator()(arg1_type a1,arg2_type a2,arg3_type a3,arg4_type a4)
    {
        FUNGU_INIT_SCRIPT_FUNCTION_CALL(args,s);
        try
        {
            arg = 1; args->push_back(s->serialize(a1));
            arg = 2; args->push_back(s->serialize(a2));
            arg = 3; args->push_back(s->serialize(a3));
            arg = 4; args->push_back(s->serialize(a4));
            arg = 0;
            return s->deserialize_return_value(call(args),target_tag<result_type>());
        }
        catch(ErrorException e){
            return s->deserialize_return_value(error_handler(arg,e),target_tag<result_type>());
        }
    }
};

template<typename FunctionTraits,typename Container,typename Serializer,typename ErrorException>
class script_function5:public base_script_function<Container,Serializer,ErrorException>
{
public:
    typedef typename FunctionTraits::result_type result_type;
    typedef typename FunctionTraits::arg1_type arg1_type;
    typedef typename FunctionTraits::arg2_type arg2_type;
    typedef typename FunctionTraits::arg3_type arg3_type;
    typedef typename FunctionTraits::arg4_type arg4_type;
    typedef typename FunctionTraits::arg5_type arg5_type;
    
    result_type operator()(arg1_type a1,arg2_type a2,arg3_type a3,arg4_type a4,arg5_type a5)
    {
        FUNGU_INIT_SCRIPT_FUNCTION_CALL(args,s);
        try
        {
            arg = 1; args->push_back(s->serialize(a1));
            arg = 2; args->push_back(s->serialize(a2));
            arg = 3; args->push_back(s->serialize(a3));
            arg = 4; args->push_back(s->serialize(a4));
            arg = 5; args->push_back(s->serialize(a5));
            arg = 0;
            return s->deserialize_return_value(call(args),target_tag<result_type>());
        }
        catch(ErrorException e){
            return s->deserialize_return_value(error_handler(arg,e),target_tag<result_type>());
        }
    }
};

template<typename FunctionTraits,typename Container,typename Serializer,typename ErrorException>
class script_function6:public base_script_function<Container,Serializer,ErrorException>
{
public:
    typedef typename FunctionTraits::result_type result_type;
    typedef typename FunctionTraits::arg1_type arg1_type;
    typedef typename FunctionTraits::arg2_type arg2_type;
    typedef typename FunctionTraits::arg3_type arg3_type;
    typedef typename FunctionTraits::arg4_type arg4_type;
    typedef typename FunctionTraits::arg5_type arg5_type;
    typedef typename FunctionTraits::arg5_type arg6_type;
    
    result_type operator()(arg1_type a1,arg2_type a2,arg3_type a3,arg4_type a4,arg5_type a5,arg6_type a6)
    {
        FUNGU_INIT_SCRIPT_FUNCTION_CALL(args,s);
        try
        {
            arg = 1; args->push_back(s->serialize(a1));
            arg = 2; args->push_back(s->serialize(a2));
            arg = 3; args->push_back(s->serialize(a3));
            arg = 4; args->push_back(s->serialize(a4));
            arg = 5; args->push_back(s->serialize(a5));
            arg = 6; args->push_back(s->serialize(a6));
            arg = 0;
            return s->deserialize_return_value(call(args),target_tag<result_type>());
        }
        catch(ErrorException e){
            return s->deserialize_return_value(error_handler(arg,e),target_tag<result_type>());
        }
    }
};

#define FUNGU_SCRIPT_FUNCTION_CLASS_SELECTION \
boost::mpl::at<boost::mpl::vector< \
        script_function0<boost::function_traits<Signature>, Container, Serializer, ErrorException >, \
        script_function1<boost::function_traits<Signature>, Container, Serializer, ErrorException >, \
        script_function2<boost::function_traits<Signature>, Container, Serializer, ErrorException >, \
        script_function3<boost::function_traits<Signature>, Container, Serializer, ErrorException >, \
        script_function4<boost::function_traits<Signature>, Container, Serializer, ErrorException >, \
        script_function5<boost::function_traits<Signature>, Container, Serializer, ErrorException >, \
        script_function6<boost::function_traits<Signature>, Container, Serializer, ErrorException > >, \
    boost::mpl::int_<boost::function_traits<Signature>::arity> >::type \

template<typename Signature,typename Container,typename Serializer, typename ErrorException>
class get_script_function_class:public FUNGU_SCRIPT_FUNCTION_CLASS_SELECTION{};

} //namespace detail

template<typename Signature,typename Container,typename Serializer,typename ErrorException>
class generic_script_function:public detail::get_script_function_class<Signature,Container,Serializer,ErrorException>{};

} //namespace fungu

#endif
