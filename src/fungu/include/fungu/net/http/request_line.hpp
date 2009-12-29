/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_REQUEST_LINE_HPP
#define FUNGU_NET_HTTP_REQUEST_LINE_HPP

#include <string>

namespace fungu{
namespace http{

enum method_code
{
    UNKNOWN = 0,
    OPTIONS,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT
};

/**
    @brief HTTP request-line parser
*/
class request_line
{
public:
    typedef char ** cursor_type;
    
    enum parse_status
    {
        PARSE_OK = 0,
        PARSE_UNKNOWN_METHOD,
        PARSE_MISSING_URI,
        PARSE_MALFORMED_VERSION
    };
    
    parse_status parse(cursor_type);
    
    method_code method()const;
    
    const char * uri()const;
    
    int version_major()const;
    int version_minor()const;
    bool is_version_1_0()const;
    bool is_version_1_1()const;
private:
    static method_code parse_method(cursor_type);
    method_code m_method;
    const char * m_uri;
    char m_version;
};

} //namespace http
} //namespace fungu

#endif
