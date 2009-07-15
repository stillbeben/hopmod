/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace corelib{

namespace aliaslib{

class alias:public env::object
{
public:
    alias();
    void push_block(const_string code,env::frame * aScope);
    bool pop_block();
    result_type call(call_arguments & args,env::frame * frm);
    result_type value();
    const source_context * get_source_context()const;
private:
    std::list<boost::shared_ptr<code_block> > m_blocks;
    int m_recursion_depth;
};

static result_type params_scriptfun(env::object::call_arguments & args,env::frame * aFrame);

class module:public env_module<module>
{
public:
    module(env * e);
    ~module();
    int get_numargs()const;
    void set_numargs(int numargs);
    void push_arg(int i, const_string value, env::frame * frm);
    void pop_arg(int i, env::frame * frm);
    alias * get_arg_alias(int i,env * e);
private:
    std::vector<std::pair<env::symbol *,alias *> > m_arg;

    int m_numargs;
    variable<int> m_numargs_wrapper;
    
    function<raw_function_type> m_params_func;
};

inline result_type define_alias(env::object::call_arguments & args,env::frame * aScope)
{
    const_string name = args.safe_casted_front<const_string>();
    args.pop_front();
    
    const_string content = args.safe_casted_front<const_string>();
    args.pop_front();
    
    alias * aAlias = new alias;
    aAlias->set_adopted();
    
    aAlias->push_block(content,aScope);
    
    aScope->get_env()->bind_global_object(aAlias, name.copy());
    
    return aAlias->value();
}

inline result_type push_alias_block(env::object::call_arguments & args,env::frame * aFrame)
{
    const_string name = args.safe_casted_front<const_string>();
    args.pop_front();
    
    const_string content = args.safe_casted_front<const_string>();
    args.pop_front();
    
    alias * a = dynamic_cast<alias *>(aFrame->lookup_required_object(name));
    if(!a) throw error(BAD_CAST);
    
    a->push_block(content,aFrame);
    
    return any::null_value();
}

inline result_type pop_alias_block(env::object::call_arguments & args,env::frame * aFrame)
{   
    const_string name = args.safe_casted_front<const_string>();
    args.pop_front();
    
    alias * a = dynamic_cast<alias *>(aFrame->lookup_required_object(name));
    if(!a) throw error(BAD_CAST);
    
    return a->pop_block();
}

module::module(env * e)
 :env_module<module>(e),
  m_numargs(0),
  m_numargs_wrapper(m_numargs),
  m_params_func(&params_scriptfun)
{
    e->bind_global_object(&m_numargs_wrapper, FUNGU_OBJECT_ID("numargs"));
    e->bind_global_object(&m_params_func, FUNGU_OBJECT_ID("params"));
}

module::~module()
{
    for(std::vector<std::pair<env::symbol *,alias *> >::iterator it = m_arg.begin(); 
        it != m_arg.end(); ++it)
    {
        it->first->set_global_object(NULL);
        delete it->second;
    }
    
    m_arg.clear();
}

int module::get_numargs()const
{
    return m_numargs;
}

void module::set_numargs(int numargs)
{
    m_numargs = numargs;
}
    
void module::push_arg(int i, const_string value, env::frame * frm)
{
    get_arg_alias(i, frm->get_env())->push_block(value, frm);
}

void module::pop_arg(int i, env::frame * frm)
{
    get_arg_alias(i, frm->get_env())->pop_block();
}
    
alias * module::get_arg_alias(int i, env * e)
{
    i--;
    
    assert(i <= (int)m_arg.size());
    
    env::symbol * alias_sym;
    alias * a;
    
    if(i == (int)m_arg.size())
    {
        a = new alias;
        alias_sym = e->create_symbol(std::string("arg") + lexical_cast<std::string>(i+1));
        m_arg.push_back(std::pair<env::symbol *,alias *>(alias_sym, a));
    }
    else
    {
        alias_sym = m_arg[i].first;
        a = m_arg[i].second;
    }
    
    // ensure alias object is bound to argI symbol
    alias_sym->set_global_object(a);
    
    return a;
}

alias::alias()
 :m_recursion_depth(0)
{
    
}
    
void alias::push_block(const_string code,env::frame * aScope)
{
    m_blocks.push_front(
        boost::shared_ptr<code_block>(new code_block(code,aScope->get_env()->get_source_context()))
    );
}
    
bool alias::pop_block()
{
    if(m_blocks.empty())return false;
    m_blocks.pop_front();
    return true;
}
    
result_type alias::call(call_arguments & args,env::frame * frm)
{
    if(++m_recursion_depth > env::recursion_limit)
        throw error(HIT_RECURSION_LIMIT,boost::make_tuple(env::recursion_limit));
    
    if(m_blocks.empty()) return any::null_value();
    
    module * alias_globals = frm->get_env()->get_module_instance<module>();
    assert(alias_globals);
    
    #define ALIAS_APPLY_CLEANUP \
        m_recursion_depth--; \
        alias_globals->set_numargs(last_numargs); \
        for(int i = 1; i<= argc; i++) alias_globals->pop_arg(i, frm);
    
    int last_numargs = alias_globals->get_numargs();
    alias_globals->set_numargs(args.size());
    int argc = args.size();
    
    int argn = 1;
    while(!args.empty())
    {
        alias_globals->push_arg(argn++, lexical_cast<const_string>(args.front()), frm);
        args.pop_front();
    }
    
    result_type implicit_result;
    
    code_block & block = *m_blocks.front();
    block.compile(frm);
    
    try
    {
        for(code_block::iterator i = m_blocks.front()->begin();
            i != m_blocks.front()->end() && !frm->has_expired(); ++i)
        {
            implicit_result = i->eval(frm);
        }
    }
    catch(...)
    {
        ALIAS_APPLY_CLEANUP
        throw;
    }
    
    block.destroy_compilation();
    
    ALIAS_APPLY_CLEANUP
    
    if(!frm->get_result_value().empty()) return frm->get_result_value();
    else return implicit_result;
}

result_type alias::value()
{
    if(m_blocks.empty()) return any::null_value();
    return m_blocks.front()->value();
}
    
const source_context * alias::get_source_context()const
{
    return m_blocks.front()->get_source_context();
}

result_type params_scriptfun(env::object::call_arguments & args,env::frame * aFrame)
{
    module * alias_globals = aFrame->get_env()->get_module_instance<module>();
    
    class alias_ref:public env::object
    {
    public:
        alias_ref(object * obj)
         :m_object(obj)
        {
            set_adopted();
        }
        
        result_type call(call_arguments & args,frame * aFrame)
        {
            return m_object->call(args,aFrame);
        }
        
        result_type value()
        {
            return m_object->value();
        }
    private:
        object * m_object;
    };
    
    int arg_index = 1;
    while(!args.empty())
    {
        const_string argname = args.casted_front<const_string>();
        aFrame->bind_object(new alias_ref(alias_globals->get_arg_alias(arg_index++, aFrame->get_env())), argname.copy());
        args.pop_front();
    }
    
    return any::null_value();
}

} //namespace detail

void register_alias_functions(env & environment)
{
    static function<raw_function_type> define_alias_func(aliaslib::define_alias);
    environment.bind_global_object(&define_alias_func,FUNGU_OBJECT_ID("alias"));
    
    static function<raw_function_type> push_alias_block_func(aliaslib::push_alias_block);
    environment.bind_global_object(&push_alias_block_func,FUNGU_OBJECT_ID("push"));
    
    static function<raw_function_type> pop_alias_block_func(aliaslib::pop_alias_block);
    environment.bind_global_object(&pop_alias_block_func,FUNGU_OBJECT_ID("pop"));
    
    environment.set_module_instance(new aliaslib::module(&environment));
}

void unload_alias_functions(env & environment)
{
    aliaslib::module * alias_globals = environment.get_module_instance<aliaslib::module>();
    environment.unset_module_instance<aliaslib::module>();
    delete alias_globals;
}

} //namespace corelib
} //namespace script
} //namespace fungu
