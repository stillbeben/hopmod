#include <cubescript.hpp>
#include <sqlite3.h>

class sqlite3_database:public cubescript::proto_object
{
public:
    
private:
    sqlite3 * m_handle;
};

class sqlite3_result:public cubescript::proto_object
{
public:
    
};
