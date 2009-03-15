/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"
#include "fungu/script/function.hpp"
#include "fungu/script/execute.hpp"
#include <math.h>
#include <limits>

namespace fungu{
namespace script{
namespace corelib{

namespace detail{

#define FUNGU_VARIADIC_BINARY_INT_FUNCTION(name,binfun) \
    inline result_type name(env::object::apply_arguments & args,env::frame *) \
    { \
        int operand[2]; \
    \
        operand[0] = args.safe_casted_front<int>(); \
        args.pop_front(); \
    \
        operand[1] = args.safe_casted_front<int>(); \
        args.pop_front(); \
    \
        for(;;) \
        { \
            operand[0] = binfun(operand[0],operand[1]); \
            if(args.size()) \
            { \
                operand[1] = args.safe_casted_front<int>(); \
                args.pop_front(); \
            } \
            else break; \
        } \
         \
        return operand[0]; \
    }
    
template<typename BigType,typename IntType>
inline void check_overflow(BigType ans)
{
    if(ans > std::numeric_limits<IntType>::max() )
        throw error(INTEGER_OVERFLOW);
    else if(ans < std::numeric_limits<IntType>::min()+1)
        throw error(INTEGER_UNDERFLOW);
}

inline int add(int a,int b)
{
    register long long ans = static_cast<long long>(a) + static_cast<long long>(b);
    check_overflow<long long,int>(ans);
    return ans;
}

inline int sub(int a,int b)
{
    register long long ans = static_cast<long long>(a) - static_cast<long long>(b);
    check_overflow<long long,int>(ans);
    return ans;
}

inline int mul(int a,int b)
{
    register long long ans = static_cast<long long>(a) * static_cast<long long>(b);
    check_overflow<long long,int>(ans);
    return ans;
}

inline int div(int a,int b)
{
    if(!b) throw error(DIVIDE_BY_ZERO);
    else return a / b;
}

inline int mod(int a,int b){return a % b;}
inline int min(int a,int b){return a < b ? a : b;}
inline int max(int a,int b){return a < b ? b : a;}

FUNGU_VARIADIC_BINARY_INT_FUNCTION(addv,add);
FUNGU_VARIADIC_BINARY_INT_FUNCTION(subv,sub);
FUNGU_VARIADIC_BINARY_INT_FUNCTION(mulv,mul);
FUNGU_VARIADIC_BINARY_INT_FUNCTION(divv,div);
FUNGU_VARIADIC_BINARY_INT_FUNCTION(minv,min);
FUNGU_VARIADIC_BINARY_INT_FUNCTION(maxv,max);

inline float fadd(float a,float b){return a + b;}
inline float fsub(float a,float b){return a - b;}
inline float fmul(float a,float b){return a * b;}
inline float fdiv(float a,float b){return a / b;}
inline float fmin(float a,float b){return a < b ? a : b;}
inline float fmax(float a,float b){return a < b ? b : a;}

inline float round(float a){return roundf(a);}
inline float ceil(float a){return ceilf(a);}
inline float floor(float a){return floorf(a);}

inline bool equal_to(int a,int b){return a == b;}
FUNGU_VARIADIC_BINARY_INT_FUNCTION(equal_to_v,equal_to);
inline bool not_equal_to(int a,int b){return a != b;}
inline bool less_than(int a,int b){return a < b;}
inline bool less_than_or_equal_to(int a,int b){return a <= b;}
inline bool greater_than(int a,int b){return a > b;}
inline bool greater_than_or_equal_to(int a,int b){return a >= b;}

inline bool logical_not(int a){return !a;}

inline int bitwise_lshift(int a,int shift){return a << shift;}
inline int bitwise_rshift(int a,int shift){return a >> shift;}
inline int bitwise_not(int a){return ~a;}
inline int bitwise_and(int a,int b){return a & b;}
inline int bitwise_or(int a,int b){return a | b;}
inline int bitwise_xor(int a,int b){return a ^ b;}

} //namespace detail

void register_math_functions(env & environment)
{
    static function<raw_function_type> add_func(detail::addv);
    environment.bind_global_object(&add_func,FUNGU_OBJECT_ID("+"));
    
    static function<raw_function_type> sub_func(detail::subv);
    environment.bind_global_object(&sub_func,FUNGU_OBJECT_ID("-"));
    
    static function<raw_function_type> mul_func(detail::mulv);
    environment.bind_global_object(&mul_func,FUNGU_OBJECT_ID("*"));
    
    static function<raw_function_type> div_func(detail::divv);
    environment.bind_global_object(&div_func,FUNGU_OBJECT_ID("div"));
    
    static function<int (int,int)> mod_func(detail::mod);
    environment.bind_global_object(&mod_func,FUNGU_OBJECT_ID("mod"));
    
    static function<raw_function_type> min_func(detail::minv);
    environment.bind_global_object(&min_func,FUNGU_OBJECT_ID("min"));
    
    static function<raw_function_type> max_func(detail::maxv);
    environment.bind_global_object(&max_func,FUNGU_OBJECT_ID("max"));
    
    static function<float (float,float)> fadd_func(detail::fadd);
    environment.bind_global_object(&fadd_func,FUNGU_OBJECT_ID("fadd"));
    
    static function<float (float,float)> fsub_func(detail::fsub);
    environment.bind_global_object(&fsub_func,FUNGU_OBJECT_ID("fsub"));
    
    static function<float (float,float)> fmul_func(detail::fmul);
    environment.bind_global_object(&fmul_func,FUNGU_OBJECT_ID("fmul"));
    
    static function<float (float,float)> fdiv_func(detail::fdiv);
    environment.bind_global_object(&fdiv_func,FUNGU_OBJECT_ID("fdiv"));
    
    static function<float (float,float)> fmin_func(detail::fmin);
    environment.bind_global_object(&fmin_func,FUNGU_OBJECT_ID("fmin"));
    
    static function<float (float,float)> fmax_func(detail::fmax);
    environment.bind_global_object(&fmax_func,FUNGU_OBJECT_ID("fmax"));
    
    static function<float (float)> round_func(detail::round);
    environment.bind_global_object(&round_func,FUNGU_OBJECT_ID("round"));
    
    static function<float (float)> ceil_func(detail::ceil);
    environment.bind_global_object(&ceil_func,FUNGU_OBJECT_ID("ceil"));
    
    static function<float (float)> floor_func(detail::floor);
    environment.bind_global_object(&floor_func,FUNGU_OBJECT_ID("floor"));
    
    static function<raw_function_type> equal_to_func(detail::equal_to_v);
    environment.bind_global_object(&equal_to_func,FUNGU_OBJECT_ID("="));
    
    static function<bool (int,int)> not_equal_to_func(detail::not_equal_to);
    environment.bind_global_object(&not_equal_to_func,FUNGU_OBJECT_ID("!="));
    
    static function<bool (int,int)> less_than_func(detail::less_than);
    environment.bind_global_object(&less_than_func,FUNGU_OBJECT_ID("<"));
    
    static function<bool (int,int)> less_than_or_equal_to_func(detail::less_than_or_equal_to);
    environment.bind_global_object(&less_than_or_equal_to_func,FUNGU_OBJECT_ID("<="));
    
    static function<bool (int,int)> greater_than_func(detail::greater_than);
    environment.bind_global_object(&greater_than_func,FUNGU_OBJECT_ID(">"));
    
    static function<bool (int,int)> greater_than_or_equal_to_func(detail::greater_than_or_equal_to);
    environment.bind_global_object(&greater_than_or_equal_to_func,FUNGU_OBJECT_ID(">="));
    
    static function<bool (int)> logical_not_func(detail::logical_not);
    environment.bind_global_object(&logical_not_func,FUNGU_OBJECT_ID("!"));
    
    static function<int (int,int)> blshift_func(detail::bitwise_lshift);
    environment.bind_global_object(&blshift_func,FUNGU_OBJECT_ID("<<"));
    
    static function<int (int,int)> brshift_func(detail::bitwise_rshift);
    environment.bind_global_object(&brshift_func,FUNGU_OBJECT_ID(">>"));
    
    static function<int (int)> bnot_func(detail::bitwise_not);
    environment.bind_global_object(&bnot_func,FUNGU_OBJECT_ID("~"));
    
    static function<int (int,int)> band_func(detail::bitwise_and);
    environment.bind_global_object(&band_func,FUNGU_OBJECT_ID("&"));
    
    static function<int (int,int)> bor_func(detail::bitwise_or);
    environment.bind_global_object(&bor_func,FUNGU_OBJECT_ID("|"));
    
    static function<int (int,int)> bxor_func(detail::bitwise_xor);
    environment.bind_global_object(&bxor_func,FUNGU_OBJECT_ID("^"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
