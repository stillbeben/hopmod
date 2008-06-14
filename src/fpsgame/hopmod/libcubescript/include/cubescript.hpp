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
#ifndef CUBESCRIPT_HPP
#define CUBESCRIPT_HPP

#include <iostream>
#include <exception>
#include <istream>
#include <sstream>
#include <string>
#include <list>
#include <stack>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

struct void_{}; //deprecated
struct _void{}; //deprecated

namespace cubescript{

/*!
    @deprecated
    @brief Pseudo void type.
    
    The native void type cannot be used as a return type on functions that are
    accessible from CubeScript, instead use void_t.
*/
struct void_t{};

class error_key:public std::exception
{
public:
    error_key(const char * key);
    ~error_key()throw(){}
    const char * what()const throw();
private:
    const char * m_key;
};

class symbol_error:public error_key
{
public:
    symbol_error(const error_key &,const std::string &);
    symbol_error(const char *,const std::string &);
    ~symbol_error()throw(){}
    const std::string & get_id()const throw();
private:
    std::string m_id;
};

template<typename T>
class expr_error:public T
{
public:
    expr_error(const T & key,const std::string & expr):T(key),m_expr(expr){}
    ~expr_error()throw(){}
    const std::string & get_expr()const throw(){return m_expr;}
private:
    std::string m_expr;
};

template<typename T>
class script_error:public expr_error<T>
{
public:
    script_error(const expr_error<T> & key,const std::string & filename,int linenum)
     :expr_error<T>(key),m_filename(filename),m_linenum(linenum){}
    ~script_error()throw(){}
    const std::string & get_filename()const throw(){return m_filename;}
    int get_linenumber()const throw(){return m_linenum;}
private:
    std::string m_filename;
    int m_linenum;
};

class domain;

/*!
    @brief The base class for symbol types.

    A derived symbol object acts as an adapter between the native and
    CubeScript runtime, performing the necessary string-type conversions and
    mapping operations. Symbol types are unintrusive to the native mapping's
    interface, meaning there are no signs of registered functions or variables
    in C++ being accessible from a scripting language.
*/
class symbol
{
public:
    virtual ~symbol();
    
    typedef std::list<std::string> arglist_type;
    
    /*!
        @brief Apply the list of arguments to the symbol.
    */
    virtual std::string apply(std::list<std::string> & args,domain * d)=0;
    
    /*!
        @brief Return a string representation of the symbol value.
    
        This method is called by the expression evaluator when the symbol name
        is prefixed by $.
    */
    virtual std::string value()const=0;
    
    class persistable;
};

class symbol::persistable:public symbol
{
public:
    virtual std::string save(const std::string &)const=0;
};

/*!
    @brief The symbol class for hosting aliases.
    
    An alias in the CubeScript programming language is a named string which can
    be executed or its stored value returned; an alias can serve as a variable
    or function for the script writer. Aliases are typically created and exist
    at runtime only; the alias assignment function registers newly created
    aliases to the domain passed from the expression object.
*/
class alias:public symbol::persistable
{
public:
    alias();
    
    /*!
        @brief Push value onto alias stack.
        
        The alias object stores a stack of values, when the apply or value
        method is called, the last pushed value is always the value used
        by these two methods.
        
        @note push and pop are accessible runtime functions.
    */
    void push(const std::string &);
    /*!
        @brief Remove latest value from alias stack.
    */
    bool pop();

    /*!
        @brief Call the alias with the given argument list.
        
        Evaluates the stored code block, with the given arguments. The passed
        arguments are accessible, by the alias code block, under symbol names
        using the convention $arg[1 to n], for example $arg1 and $arg2.
    */
    std::string apply(std::list<std::string> & args,domain *);
    /*!
        @brief Get the alias's stored value.
    */
    std::string value()const;
    
    std::string save(const std::string &)const;
    
private:
    void_t create_sub_alias(domain *,const std::string &,const std::string &);
    void_t create_reference(domain *,domain *,const std::string &,const std::string &);
    void_t create_parameter_refs(std::list<std::string> &,domain *);

    void_t result(std::string);
    
    std::string eval_block(domain *);
    std::stack<std::string> m_blocks;
    std::string m_result;
    bool m_set_result;
    domain * m_local;
};

class alias_function
{
public:
    alias_function(const std::string &,domain *);
    std::string run(std::list<std::string> &);
private:
    alias m_alias;
    domain * m_domain;
};

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
    
    enum { SEARCH_PARENTS=1,        // search ancestors
           SEARCH_LOCAL_ONLY=2,     // search only this domain
           REGISTER_ROOT=8,         // register symbol to root parent
           REGISTER_FIRSTNTMP=16,   // register symbol to first non-temporary
           ADOPT_SYMBOL=32,         // assume ownership of symbol object
           DEFAULT_FLAGS=SEARCH_PARENTS | REGISTER_ROOT
         };
    
    /*!
        @brief Search for a symbol.

        @param id name of symbol.
        @param search_parents search ancestor domains.
        @return pointer to a symbol object, if symbol not found the return alue is NULL.
    */
    symbol * lookup_symbol(const std::string & id,int search=SEARCH_PARENTS)const;
    
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
            domain * register_domain=this;
            if(flags & REGISTER_ROOT) while(register_domain->m_parent) register_domain=register_domain->m_parent;
            else if(flags & REGISTER_FIRSTNTMP) register_domain=register_domain->get_first_nontmp_domain();
            register_domain->register_symbol(name,symbolPtr,ADOPT_SYMBOL);
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
    typedef std::pair<symbol *,boost::shared_ptr<symbol> > symbol_pointers;
    typedef std::map<std::string,symbol_pointers> symbol_map;
    symbol_map m_symbols;
    domain * m_parent;
    bool m_temporary;
    bool m_link;
};

/*!
    @brief The base class for construct types.

    A derived construct class represents a syntatical construct of the
    CubeScript programming language - consequently, the construct class is a
    critical design concept of the scripting engine. The primary functions of a
    construct are parsing and evaluation. The parsing function reads and
    interprets CubeScript syntax into an internal representation for later
    evaluation. The evaluation function uses the internal representation to
    perform an operation.
*/
class construct:public boost::noncopyable
{
public:
    construct();
    virtual ~construct();

    /*!
        @brief Parse CubeScript code.
        @return The return value is True when parsing is complete, otherwise
                False is the return value.

        The parse method is stream-oriented; when the current function call
        cannot complete the parsing, because not enough input has been given
        yet, the function will return to the caller, who will be responsible to
        re-call the function when more input data is received - the parsing
        state is preserved among multiple function calls until parsing is 
        complete.

        When a parsing error is detected an exception of syntax_error type is 
        thrown.
    */
    virtual bool parse(std::istream & input)=0;
    
    /*!
        @brief Evaluate the parsed construct.
        @brief The result of the evaluation.
    */
    virtual std::string eval()=0;
    
    /*!
        @brief Returns a reformed representation of the parsed construct.
    */
    virtual std::string formed()const=0;

    /*!
        @brief Set the execution/symbol domain.
        @param d pointer to the domain object.
        
        This method affects symbol resolution local to the construct object. It
        is convention for derived construct types to pass their domain
        pointer to their sub construct objects - a child construct inherits its
        parent's symbol domain.
        
        You should call this method before calling the parse method.
        
        @see domain
    */
    void set_domain(domain * d);
    
    domain * get_domain()const;
private:
    domain * m_ctx;
};

namespace cons{ //construct classes go inside here...

/*!
    The Word construct is the simplest construct defined in the scripting
    engine as no interpretation is performed and evaluation returns the same
    character sequence passed to the parse function. There are no starting
    characters to denote word construct parsing and is terminated by
    characters belonging to foreign constructs. The word construct is useful
    for self-evaluating forms, i.e. symbol identifiers and numbers.
*/
class word:public construct
{
public:
    word();
    ~word();
    /*!
        Parsing is terminated by the following characters: '"', '(', ')', '[',
        ']', ';', whitespaces and newline.

        @note Useful characters that can be parsed include: '-', '?'
    */
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    inline bool in_set(char)const;
    std::string m_word;
};

/*!
    The String construct is a character sequence enclosed in double quotes, and
    is similar to the word construct in that its evaluation returns the parse
    input, excluding the double quotes. The string construct is useful for
    supporting string literals, which may contain characters that would cause
    syntax interpretation. Parsing is terminated by a double-quote or newline
    character.
*/
class string:public construct
{
public:
    string();
    /*!
        An extension to the CubeScript language, not found in Sauerbraten's
        implementation of CubeScript, is support for the escape character. The
        escape character ("\") allows a double quote character to be expressed
        literally, any character proceeding a blackslash is treated literally.
        
        @note Escape sequences are not supported, this is using metacharacters
              to generate characters (e.g. "\n").
    */
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    std::string m_str;
};

/*!
    The macro construct allows for expression evaluation and symbol value
    substituation - at parse time, from within a block construct. The beginning
    of a macro construct is denoted by the @ character, the number of
    consecutive @ characters indicates at which block scope the macro should be
    evaluated. This construct has two known convenient uses: passing arguments
    to anonymous aliases and formatting strings.
    
    The following code example shows an anonymous alias being passed to the
    sleep function where it will be executed 2 seconds after the deadline is
    initiated.
    @code
    > sleep 2000 [echo [sleep was called at @@(local_time)]]
    @endcode
    The macro construct in this example, "@@(local_time))", has an escape level
    of 2 (indicated by two @ characters) which means the macro is parsed and
    evaluated when the first block is parsed, by the time the second block
    comes to being interpreted, the macro result has already been substituted.
    The "(local_time)" part of the macro is an expression to be evaluated.
    Symbol macros have the form, "@name_of_symbol", where characters proceeding
    the @ character(s) is a word construct.
*/
class macro:public construct
{
public:
    macro();
    ~macro();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
    int get_escape_level()const;
private:
    bool parse_subt(std::istream &);
    int m_escape;
    construct * m_subt;
};

/*!
    The block construct is useful for expressing multiline string literals. Its
    syntax is: "[" <arbitrary-string> "]". The arbitrary string is any
    characters from the ascii character set, and can include closed sub-block
    constructs (e.g. "[some string [a sub block]]"), also "@" characters are
    interpreted as part of a macro construct.

    The block and string construct are semantically the same thing, an alias
    is assigned with a value or code (it depends how you use the alias) of 
    string type. The block construct is better for expressing code.
*/
class block:public construct
{
public:
    block();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    bool parse_macro(std::istream &);
    std::string m_code;
    int m_nested;
    macro * m_macro;
};

/*!
    The expression construct is the core execution construct of the scripting
    language, an expression represents an apply operation to be performed on a
    symbol object, the meaning of apply is dependant on the symbol object's
    type. A CubeScript programmer writes a script from a sequence of
    expressions; no distinction is made between statements and expressions,
    the engine treats all expressions as one type of thing: an apply operation.
    Apply operations are abstract to the expression evaluator, only the script
    writer knows what apply operation is performed, by the symbol name given.

    An expression consists of elements separated by whitespace
    characters, with the first element being the symbol name and the remaining
    elements being the arguments. An expression is terminated by a newline or
    semicolon character. An element can be a word, string, block or expression.
    When the element constructs have been evaluated, from left to right, the
    apply method will be called on the symbol object. The result value of the
    apply operation is the result of the expression.

    There is an exception to the rule of expected syntax: the expression
    construct can also detect and accept an alias assignment using infix
    notation (i.e. "name = [value]"), for convenient use.
*/
class expression:public construct
{
public:
    expression();
    ~expression();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
    bool is_empty()const;
private:
    bool parse_sub_construct(std::istream &);
    inline bool in_termset(char)const;
    bool assign_alias_macro(std::list<std::string> &);
    inline bool eval_type(const construct *)const;
    
    construct * m_parsing;
    char m_sub_term;
    std::istream::streampos m_inputpos;
    
    std::list<construct *> m_ops;
};

/*!
    The expression_container construct is used for expressing sub-expressions.
    The syntax for a sub-expression is: "(" <expression> ")".
*/
class expression_container:public construct
{
public:
    expression_container();
    ~expression_container();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    expression * m_parsing;
    std::list<expression *> m_exps;
};

/*!
    The comment construct allows the CubeScript programmer to write non-code
    text into a script; comment constructs are ignored by the expression
    evaluator. Comments begins with "//" and are terminated by a newline
    character.
*/
class comment:public construct
{
public:
    bool parse(std::istream &);
    /*!
        @return Empty string.
    */
    std::string eval();
    std::string formed()const;
private:
    std::string m_line;
};

} //namespace cons

template<typename T> inline void parse_type(const std::string & str,std::vector<T> & v);
template<typename T> inline void parse_type(const std::string & str,std::list<T> & v);

template<typename T>
void parse_type(const std::string & str,T & v)
{
    std::stringstream reader(str);
    reader.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    try{reader>>v;}
    catch(std::ios_base::failure){throw error_key("runtime.type_mismatch");}
}

template<typename T> inline
T parse_type(const std::string & str)
{
    T tmp;
    parse_type(str,tmp);
    return tmp;
}

template<> inline
std::string parse_type<std::string>(const std::string & str)
{
    return str;
}

template<> inline
bool parse_type<bool>(const std::string & str)
{
    if(str.length()==0 || str[0]=='0') return false;
    else return true;
}

template<> inline
int parse_type<int>(const std::string & str)
{
    if(!str.length()) return 0;
    int val; parse_type(str,val);
    return val;
}

template<typename T>
T parse_pushback_container_list(const std::string & str)
{
    T container;
    std::stringstream input(str);
    
    while(input.peek()>0)
    {
        char c;
        input.get(c);
        
        if(c==' ' || c=='\t' || c=='\r' || c=='\n') continue;
        
        if(c=='\"')
        {
            cons::string element;
            element.parse(input);
            container.push_back(parse_type<typename T::value_type>(element.eval()));
            input.get(c);
        }
        else
        {
            input.putback(c);
            cons::word element;
            element.parse(input);
            container.push_back(parse_type<typename T::value_type>(element.eval()));
        }
    }
    
    return container;
}

template<typename T> inline
void parse_type(const std::string & str,std::vector<T> & v)
{
    v=parse_pushback_container_list< std::vector<T> >(str);
}

template<typename T> inline
void parse_type(const std::string & str,std::list<T> & v)
{
    v=parse_pushback_container_list< std::list<T> >(str);
}

template<typename T> inline std::string __print_type(std::vector<T>);
template<typename T> inline std::string __print_type(std::list<T> &);

template<typename T> 
std::string __print_type(T value)
{
    std::stringstream writer;
    writer<<value;
    return writer.str();
}

template<typename T> inline
std::string print_type(T value)
{
    return __print_type(value);
}

template<> inline std::string print_type<void_t>(void_t){return "";}
template<> inline std::string print_type<void_>(void_){return "";}
template<> inline std::string print_type<_void>(_void){return "";}

template<typename T>
std::string print_forward_container_list(const T & data)
{
    std::string list;
    list.reserve(64);
    
    for(typename T::const_iterator it=data.begin(); it!=data.end(); ++it)
    {
        list+=compose_string(print_type<typename T::value_type>(*it));
        list+=" ";
    }
    
    return list;
}

template<typename T> inline
std::string __print_type(std::vector<T> value)
{
    return print_forward_container_list(value);
}

template<typename T> inline
std::string __print_type(std::list<T> & value)
{
    return print_forward_container_list(value);
}

template<typename T>
class pointer:public symbol
{
public:
    pointer(const boost::shared_ptr<T> & ptr):m_ptr(ptr){}
    std::string apply(std::list<std::string> & arglist,domain * aDomain)
    {
        std::string src_name=parse_type<std::string>(arglist.front()); arglist.pop_front();
        pointer<T> * ptrwrapper=aDomain->get_symbol_of_kind< pointer<T> >(src_name);
        m_ptr=ptrwrapper->m_ptr;
        return "";
    }
    std::string value()const
    {
        throw error_key("runtime.pointer.no_value");
        //TODO use if print_type specialization exists: return print_type<element_type>(*m_ptr);
    }
    boost::shared_ptr<T> & get(){return m_ptr;}
private:
    boost::shared_ptr<T> m_ptr;
};

/*!
    @brief Make string safe for use in CubeScript.
    
    The function quotes (prepends a "\" character) to any characters that
    would cause a string construst parser to end prematurely.
*/
std::string compose_string(const std::string &);

template<typename T>
struct remove_const_reference
{
    typedef typename boost::remove_const<typename boost::remove_reference<T>::type>::type type;
};

#define DEFINE_FUNCTION_ARGUMENT(unsafe_type,id,arglist) \
    typename remove_const_reference<unsafe_type>::type id=pop_arg<typename remove_const_reference<unsafe_type>::type>(arglist);

template<typename T>
struct mutable_value
{
    typedef typename remove_const_reference<T>::type type;
};

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
    std::string apply(std::list<std::string> & args,domain *){return print_type<RetT>(m_func());}
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

/*!
    @brief The base class for variable symbol types.
    
    A variable symbol object refers to a native variable in the host
    programming language, operations in the scripting language are translated
    into native operations in the host environment.
*/
template<typename T>
class variable_base:public symbol::persistable
{
public:
    variable_base():m_ro(false){}
    
    /*!
        @brief Assign value to variable.

        Expects one argument only, providing any more arguments will result in
        an exception being thrown. The value passed is expected to have a
        string representation that can be lexically casted to the variable's
        type, incompatible notation will result in an exception being thrown.
    */
    std::string apply(std::list<std::string> & args,domain *)
    {
        if(m_ro) throw error_key("runtime.variable.read_only.");
        if(args.empty()) throw error_key("syntax.missing_arguments");
        get_ref()=parse_type<T>(args.front());
        args.pop_front();
        return "";
    }
    
    operator const T &()const{return get_ref();}
    /*!
        @brief Get the stored value.
    */
    std::string value()const{return print_type<T>(get_ref());}
    
    std::string save(const std::string & id)const
    {
        std::ostringstream form;
        form<<id<<" "<<value();
        return form.str();
    }
    
    void readonly(bool ro)
    {
        m_ro=ro;
    }
protected:
    virtual T & get_ref()const=0;
private:
    bool m_ro;
};

/*!
    @brief Variable Symbol.
    
    Is the actual variable and registerable symbol type.
*/
template<typename T>
class variable:public variable_base<T>
{
public:
    typedef T var_type;
    variable():m_ref(m_var){}
    variable<T> & operator=(const T & src)
    {
        get_ref()=src;
        return *this;
    }
protected:
    T & get_ref()const{return m_ref;}
private:
    T m_var;
    T & m_ref;
};

/*!
    @brief Variable Symbol.

    References an existing variable.
*/
template<typename T>
class variable_ref:public variable_base<T>
{
public:
    variable_ref(T & ref):m_var(ref){}
protected:
    T & get_ref()const{return m_var;}
private:
    T & m_var;
};

/*!
    @brief Bounded variable symbol.
    
    An exception is thrown when an out-of-bounds value is attempted to be
    assigned, in such a case, the value stored previously is restored.
*/
template<typename T,T min,T max>
class bounded_variable:public variable_ref<T>
{
public:
    bounded_variable(T & ref):variable_ref<T>(ref){}
    
    std::string apply(std::list<std::string> & args,domain * d)
    {
        T tmp=variable_ref<T>::get_ref();
        variable_ref<T>::apply(args,d);
        if( variable_ref<T>::get_ref() < min || variable_ref<T>::get_ref() > max )
        {
            variable_ref<T>::get_ref()=tmp;
            throw error_key("runtime.out_of_bounds");
        }
        return "";
    }
};

/*!
    @brief A variable symbol class for C strings.
*/
class cstr_variable:public symbol::persistable
{
public:
    cstr_variable(char * str,size_t maxlen):m_str(str),m_maxlen(maxlen),m_ro(false){}
    std::string apply(std::list<std::string> & args,domain *)
    {
        if(m_ro) throw error_key("runtime.variable.read_only");
        std::string newstr=parse_type<std::string>(args.front()); args.pop_front();
        if(newstr.length() > m_maxlen-1) throw error_key("runtime.string.too_long");
        strcpy(m_str,newstr.c_str());
        return "";
    }
    std::string value()const{return m_str;}
    std::string save(const std::string & id)const
    {
        std::ostringstream form;
        form<<id<<" "<<m_str;
        return form.str();
    }
    void readonly(bool ro){m_ro=ro;}
private:
    char * m_str;
    size_t m_maxlen;
    bool m_ro;
};

template<> inline
alias parse_type<alias>(const std::string & str)
{
    alias tmp;
    tmp.push(str);
    return tmp;
}

std::list<std::string> & args0();

/*!
    @brief Reference symbol

    Allows the scripter to create reference symbols, symbols that refer to
    other symbols, existing within the scope of an alias block.
    
    @code
    print_name = [
        reference title arg1
        reference firstname arg2
        reference lastname arg3
        
        result (format "%1. %2 %3" $title $firstname $lastname)
    ]
    @endcode
*/
class reference:public symbol
{
public:
    reference(symbol &);
    std::string apply(std::list<std::string> & args,domain *);
    std::string value()const;
private:
    symbol & m_symbol;
};

/*!
    @brief Constant value symbol.
*/
template<typename T>
class constant:public symbol
{
public:
    typedef typename mutable_value<T>::type value_type;
    constant(const value_type & val):m_value(val){}
    std::string apply(std::list<std::string> &,domain *){throw error_key("syntax.has_no_apply.constant");}
    std::string value()const{return print_type<const value_type &>(m_value);}
private:
    value_type m_value;
};

class proto_object:public symbol
{
public:
    proto_object();
    std::string apply(std::list<std::string> &,domain *);
    std::string value()const;
    virtual proto_object * clone()const=0;
protected:
    explicit proto_object(proto_object *); //proto_object inheritance
    proto_object(const proto_object &); //normal copy constructor
    proto_object(domain *);
    void add_member(const char *,symbol *);
    domain * get_parent_domain()const;
private:
    void register_members();
    
    function1<std::string,const std::string &> m_func_value;
    functionV<std::string> m_func_super;
    function2<void_,const std::string &,const std::string &> m_func_function;

    std::string get_member_value(const std::string &)const;
    std::string call_super_operation(std::list<std::string> &,domain *);
    void_ create_function(const std::string &,const std::string &);
    
    std::string m_operation;
    domain m_members;
};

class object_adaptor:public proto_object
{
public:
    object_adaptor(domain *);
    object_adaptor(const object_adaptor &);
    proto_object * clone()const;
};

class null:public symbol
{
public:
    std::string apply(std::list<std::string> &,domain *);
    std::string value()const;
};

template<typename T> inline
void bind(const T & c,const char * name,domain * aDomain)
{
    constant<T> * wrapper=new constant<T>(c);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

inline
void bind(const char * c,const char * name,domain * aDomain)
{
    constant<const char *> * wrapper=new constant<const char *>(c);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename T> inline
void bind(T & var,const char * name,domain * aDomain)
{
    variable_ref<T> * wrapper=new variable_ref<T>(var);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R> inline
void bind(R(&func)(),const char * name,domain * aDomain)
{
    function0<R> * wrapper=new function0<R>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1> inline
void bind(R(& func)(A1),const char * name,domain * aDomain)
{
    function1<R,A1> * wrapper=new function1<R,A1>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1,typename A2> inline
void bind(R(&func)(A1,A2),const char * name,domain * aDomain)
{
    function2<R,A1,A2> * wrapper=new function2<R,A1,A2>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1,typename A2,typename A3> inline
void bind(R(&func)(A1,A2,A3),const char * name,domain * aDomain)
{
    function3<R,A1,A2,A3> * wrapper=new function3<R,A1,A2,A3>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1,typename A2,typename A3,typename A4> inline
void bind(R(&func)(A1,A2,A3,A4),const char * name,domain * aDomain)
{
    function4<R,A1,A2,A3,A4> * wrapper=new function4<R,A1,A2,A3,A4>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R> inline
void bind(R(&func)(std::list<std::string> &,domain *),const char * name,domain * aDomain)
{
    functionV<R> * wrapper=new functionV<R>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename SymbolType> inline
void bind_symbol(SymbolType & var,const char * name,domain * aDomain)
{
    SymbolType * wrapper=new SymbolType(var);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

namespace runtime{

void register_base_functions(domain * aDomain);

/*!
    @brief Register essential runtime functions.
    @param domain register core functions to a specific domain.
    
    This function defines all the core runtime functions needed to make the
    language useful; arthmetic, logic, list and control flow operations are 
    registered, by default, to the global domain, where they can be accessed 
    from anywhere.
    
    You should call this function first thing, before *any* code evaluation is
    performed, that is before the construct::parse method is called.
*/
void register_core_functions(domain *);

/*!
    @brief Register system functions.
*/
void register_system_functions(domain *);

} //namespace runtime

/*!
    @brief Execute a block of CubeScript code.
    @param input a block of code (excluding the outer square brackets).
    @return eval result the string result of the last expression evaluation or
            the string argument passed to the last calling of the result
            function.
*/
std::string exec_block(const std::string & input,domain * aDomain);

/*!
    @brief Execute a script of CubeScript code.
    
    @return eval result the result string of the last expression evaluation.
    
    Expressions are parsed and evaluated until the input stream reaches EOF.
    
    The difference between this function and exec_block is on error an
    exec_failure exception is thrown, containing the line number referencing
    the bad code and a pointer to the original raised exception.
*/
std::string exec_script(std::istream & input,const std::string &,domain * aDomain);

/*!
    @brief Execute a CubeScript file.
    @return eval result the result string of the last expression evaluation.
    
    This helper function opens and passes a file input stream to exec_script.
*/
std::string exec_file(const std::string & filename,domain * aDomain);

/*!
    @brief A helper function for consuming last character(s) left on parse
           input stream.
    
    This function is useful if you are directly using cons::expression objects
    to handle (parse and execute) cubescript expressions. The expression
    construct writes back the terminating character(s) intended for the parent
    constrct, the root expression object has to read the term chars off to keep
    the parse stream valid for the next sequence of root expressions. It's a
    trivial but potentially repetitive task for the library user, this helper
    function does the work for you.
    
    Example of use:
    @code
    if(expr->parse(input))
    {
        cubescript::consume_terminator(input);
        std::string result=expr->eval();
    }
    @endcode
*/
void consume_terminator(std::istream &);

/*!
    @brief A helper class for building argument lists.
    
    The first parameter of the symbol apply method is an argument list for the
    apply operation. This argument list is of type std::list<std::string>, this
    helper class offers a more convenient method of forming the argument list.
    
    Example of use:
    @code
    cubescript::arguments args;
    aSymbolPtr->apply(args & 1234 & std::string("some text"),aDomainPtr);
    @endcode
*/
class arguments
{
public:
    template<typename T>
    arguments & operator&(const T & val)
    {
        m_args.push_back(print_type<T>(val));
        return *this;
    }
    operator std::list<std::string>&(){return m_args;}
private:
    std::list<std::string> m_args;
};

} //namespace cubescript

#endif
