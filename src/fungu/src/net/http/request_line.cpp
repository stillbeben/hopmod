/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/net/http/request_line.hpp"
#include <strings.h>

static inline bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

namespace fungu{
namespace http{

/**
    @param cursor
    @return status code
    
    Pre-conditions:
        * String terminated by \r\n\0

    Post-conditions:
        * String buffer within the request line range remain allocated and immutable.
        
*/
request_line::parse_status request_line::parse(cursor_type cursor)
{
    m_method = parse_method(cursor);
    if(m_method == UNKNOWN) return PARSE_UNKNOWN_METHOD;
    
    char * input = *cursor;

    if(*input++ != ' ') return PARSE_MISSING_URI;
    
    m_uri = input;
    for(; *input != ' ' && *input; input++);
    if(!(input - m_uri)) return PARSE_MISSING_URI;
    
    char * end_of_uri = *cursor + (input - *cursor);
    
    bool valid_version = input[0] == ' ' && 
        input[1] == 'H' && input[2] == 'T' && input[3] == 'T' && input[4] == 'P' && input[5] == '/' &&
        is_digit(input[6]) && input[7] == '.' && is_digit(input[8]);
    
    if(!valid_version || input[9] != '\r' || input[10] != '\n') return PARSE_MALFORMED_VERSION;
    
    m_version = (input[6] - '0') * 10 + (input[8] - '0');
    
    *cursor = input + 11;
    *end_of_uri = '\0';
    
    return PARSE_OK;
}

method_code request_line::method()const
{
    return m_method;
}

const char *  request_line::uri()const
{
    return m_uri;
}

int request_line::version_major()const
{
    return m_version / 10;
}

int request_line::version_minor()const
{
    return m_version % 10;
}

bool request_line::is_version_1_0()const
{
    return version_major() == 1 && version_minor() == 0;
}

bool request_line::is_version_1_1()const
{
    return m_version % 10 == 1 && m_version / 10 == 1;
}

method_code request_line::parse_method(cursor_type cursor)
{
    char * input = *cursor;
    
    const char * wanted_method = NULL;
    method_code wanted_method_code;
    int wanted_method_len = 0;
    
    switch(toupper(*input))
    {
        case 'O':
            wanted_method = "OPTIONS";
            wanted_method_len = 7;
            wanted_method_code = OPTIONS;
            break;
        
        case 'G':
            wanted_method = "GET";
            wanted_method_len = 3;
            wanted_method_code = GET;
            break;
        
        case 'H':
            wanted_method = "HEAD";
            wanted_method_len = 4;
            wanted_method_code = HEAD;
            break;
        
        case 'P':
            
            switch(toupper(*(input+1)))
            {
                case 'O':
                    wanted_method = "POST";
                    wanted_method_len = 4;
                    wanted_method_code = POST;
                    break;
                
                case 'U':
                    wanted_method = "PUT";
                    wanted_method_len = 3;
                    wanted_method_code = PUT;
                    break;
                
                default: break;
            }
            
            break;
        
        case 'D':
            wanted_method = "DELETE";
            wanted_method_len = 6;
            wanted_method_code = DELETE;
            break;
        
        case 'T':
            wanted_method = "TRACE";
            wanted_method_len = 5;
            wanted_method_code = TRACE;
            break;

        case 'C':
            wanted_method = "CONNECT";
            wanted_method_len = 7;
            wanted_method_code = CONNECT;
            break;
        
        default: break;
    }
    
    if(wanted_method && strncasecmp(input, wanted_method, wanted_method_len) == 0)
    {
        *cursor = input + wanted_method_len;
        return wanted_method_code;
    }
    else return UNKNOWN;
}

} //namespace http
} //namespace fungu
