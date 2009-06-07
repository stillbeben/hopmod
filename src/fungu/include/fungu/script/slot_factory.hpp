/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_SLOT_FACTORY_HPP
#define FUNGU_SCRIPT_SLOT_FACTORY_HPP

#include "../make_function_signature.hpp"
#include "script_function.hpp"
#include <boost/function.hpp>
#include <boost/signal.hpp>
#include <map>
#include <vector>
#include <list>

namespace fungu{
namespace script{
    
class slot_factory
{
public:
    ~slot_factory()
    {
        clear();
    }
    
    void clear()
    {
        for(unsigned int i = 0; i < m_slots.size(); ++i) destroy_slot(i);
        m_slots.clear();
    }
    
    template<typename SignalType,typename ErrorHandlerFunction>
    void register_signal(SignalType & sig,const_string name,ErrorHandlerFunction error_handler,boost::signals::connect_position cp = boost::signals::at_back)
    {
        boost::function<std::vector<result_type>::value_type (error_trace *)> error_handler_wrapper = error_handler;
        m_signal_connectors[name] = boost::bind(&slot_factory::connect_slot<SignalType>,this,boost::ref(sig),error_handler,cp,_1,_2);
    }
    
    int create_slot(const_string name, env::object::shared_ptr obj, env * environment)
    {
        std::map<const_string,slot_connect_function>::iterator it = m_signal_connectors.find(name);
        if(it == m_signal_connectors.end()) return -1;
        return it->second(obj, environment);
    }
    
    void destroy_slot(int handle)
    {
        if((unsigned int)handle >= m_slots.size() || handle < 0) return;
        
        m_destroyed.push_back(m_slots[handle].first);
        
        m_slots[handle].first = NULL;
        m_slots[handle].second.disconnect();
    }
    
    void deallocate_destroyed_slots()
    {
        while(!m_destroyed.empty())
        {
            delete m_destroyed.front();
            m_destroyed.pop_front();
        }
    }
private:
    template<typename SignalType>
    int connect_slot(SignalType & sig,
        boost::function<std::vector<result_type>::value_type (error_trace *)> error_handler,
        boost::signals::connect_position cp,
        env::object::shared_ptr obj,
        env * environment)
    {
        typedef typename make_function_signature<typename SignalType::slot_function_type>::type SignalSignature;
        
        std::pair<base_script_function *,boost::signals::connection> newSlot;
        script_function<SignalSignature> * newSlotFunction = new script_function<SignalSignature>(obj, environment, error_handler);
        
        newSlot.first = newSlotFunction;
        newSlot.second = sig.connect(boost::ref(*newSlotFunction),cp);
        
        int handle = -1;
        for(slot_vector::iterator it = m_slots.begin(); it != m_slots.end(); ++it) 
            if(!it->first){
                handle = it - m_slots.begin(); 
                break;
            }
        
        if(handle != -1) 
            m_slots[handle] = newSlot;
        else
        {
            m_slots.push_back(newSlot); 
            handle = m_slots.size() - 1;
        }
        
        return handle;
    }
    
    typedef boost::function<int (env::object::shared_ptr,env *)> slot_connect_function;
    std::map<const_string,slot_connect_function> m_signal_connectors;
    
    typedef detail::base_script_function<std::vector<result_type>, call_serializer, error> base_script_function;
    typedef std::vector<std::pair<base_script_function *,boost::signals::connection> > slot_vector;
    slot_vector m_slots;
    
    std::list<base_script_function *> m_destroyed;
};

} //namespace script
} //namespace fungu

#endif
