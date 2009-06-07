/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CLASS_OBJECT_FACTORY_HPP
#define FUNGU_SCRIPT_CLASS_OBJECT_FACTORY_HPP

#include "class_object.hpp"
#include <map>

namespace fungu{
namespace script{

class class_object_factory:public env::object
{
public:
    class constructor
    {
    public:
        virtual ~constructor(){}
        virtual env::object * create(call_arguments &)=0;
    };
    
    template<typename ClassType>
    class default_constructor:public constructor
    {
    public:
        env::object * create(call_arguments & args)
        {
            if(args.size()) throw error(TOO_MANY_ARGUMENTS);
            ClassType * newobj = new ClassType;
            return new class_object<ClassType>(newobj);
        }
    };
    
    void register_class(const_string className,constructor * classCtor)
    {
        m_classes[className] = classCtor;
    }
    
    result_type call(call_arguments & args,frame * aScope)
    {
        if(args.empty()) 
            throw error(NOT_ENOUGH_ARGUMENTS);
        const_string className = lexical_cast<const_string>(args.front());
        args.pop_front();
        
        if(args.empty()) 
            throw error(NOT_ENOUGH_ARGUMENTS);
        const_string instanceName =lexical_cast<const_string>(args.front());
        args.pop_front();
        
        class_map::iterator it = m_classes.find(className);
        if(it == m_classes.end()) 
            throw error(OPERATION_ERROR,boost::make_tuple("class not found"));
        
        aScope->bind_object(it->second->create(args),instanceName.copy())
            .adopt_object();
        
        return const_string();
    }
    
    result_type value()
    {
        throw error(NO_VALUE);
    }
private:
    typedef std::map<const_string,
        constructor *,
        const_string::less_than_comparator> class_map;
    class_map m_classes;
};

} //namespace script
} //namespace fungu

#endif
