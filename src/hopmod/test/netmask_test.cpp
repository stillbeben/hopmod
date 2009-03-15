#include "test.hpp"
#include "../netmask.hpp"

bool test_constructors()
{
    netmask blank;
    netmask blank2(0);
    netmask localhost(inet_addr("127.0.0.1"), 8);
    return true;
}

bool test_comparisons()
{
    TEST_ASSERT(netmask(0) == netmask(0));
    TEST_ASSERT(netmask(inet_addr("127.0.0.1"),8) == netmask(inet_addr("127.255.255.254"),32));
    TEST_ASSERT(netmask(inet_addr("127.0.0.1"),8) == netmask(inet_addr("127.255.255.254"),32));
    TEST_ASSERT(netmask(inet_addr("192.168.0.1"),16) != netmask(inet_addr("127.0.0.1"),8));
    TEST_ASSERT(netmask(inet_addr("127.0.0.1"),32) < netmask(inet_addr("192.168.0.1"),8));
    return true;
}

bool test_to_string()
{
    TEST_ASSERT(netmask(inet_addr("127.0.0.1"), 8).to_string() == "127.0.0.0/8");
    TEST_ASSERT(netmask(inet_addr("127.0.0.1"), 32).to_string() == "127.0.0.1");
    TEST_ASSERT(netmask(inet_addr("192.168.10.12"),32).to_string() == "192.168.10.12");
    return true;
}

bool test_make()
{
    TEST_ASSERT(netmask::make("127.0.0.1").to_string() == "127.0.0.1");
    TEST_ASSERT(netmask::make("127/8").to_string() == "127.0.0.0/8");
    TEST_ASSERT(netmask::make("192.168.0.1/16").to_string() == "192.168.0.0/16");
    
    try
    {
        netmask::make("256.0.0.0/8");
        return false;
    }catch(std::bad_cast){}
    
    try
    {
        netmask::make("127.0.0.0/0");
        return false;
    }catch(std::bad_cast){}
    
    try
    {
        netmask::make("127.0.0.0/33");
        return false;
    }catch(std::bad_cast){}
        
    try
    {
        netmask::make("127.0.a.0/8");
        return false;
    }catch(std::bad_cast){}
    
    try
    {
        netmask::make("00000000000000000000127.0.0.0");
        return false;
    }catch(std::bad_cast){}
    
    return true;
}

int main()
{
    BOOST_TEST(test_constructors());
    BOOST_TEST(test_comparisons());
    BOOST_TEST(test_to_string());
    BOOST_TEST(test_make());
    return boost::report_errors();
}
