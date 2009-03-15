#include "test.hpp"
#include "../banned_networks.hpp"

bool test_permban()
{
    banned_networks bans;
    
    bans.set_permanent_ban(netmask::make("169.10.0.1"));
    
    bans.set_permanent_ban(netmask::make("192.168.0.1"));
    
    TEST_ASSERT(bans.is_banned(netmask::make("192.168.0.1")));
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.2")));
    
    bans.set_permanent_ban(netmask::make("192/8"));
    
    TEST_ASSERT(bans.is_banned(netmask::make("192.168.0.3")));
    TEST_ASSERT(!bans.is_banned(netmask::make("191.168.0.1")));
    
    TEST_ASSERT(bans.is_banned(netmask::make("169.10.0.1")));
    
    return true;
}

bool test_tempban()
{
    banned_networks bans;
    
    bans.set_temporary_ban(netmask::make("192.168.0.1"));
    
    TEST_ASSERT(bans.is_banned(netmask::make("192.168.0.1")));
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.2")));
    
    bans.set_temporary_ban(netmask::make("192/8"));
    
    TEST_ASSERT(bans.is_banned(netmask::make("192.168.0.3")));
    TEST_ASSERT(!bans.is_banned(netmask::make("191.168.0.1")));
    
    return true;
}

bool test_unsetban()
{
    banned_networks bans;
    
    bans.set_permanent_ban(netmask::make("192.168.0.1"));
    bans.unset_ban(netmask::make("192/8"));
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.1")));
    
    bans.set_permanent_ban(netmask::make("192.168/16"));
    bans.unset_ban(netmask::make("192.168.0.1"));
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.1")));
    
    bans.set_temporary_ban(netmask::make("192.168.0.2"));
    bans.unset_ban(netmask::make("192.168.0.2"));
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.2")));
    
    return true;
}

bool test_unset_tempbans()
{
    banned_networks bans;
    
    bans.set_permanent_ban(netmask::make("192.168.0.1"));
    bans.set_permanent_ban(netmask::make("127.0.0.1"));
    bans.set_temporary_ban(netmask::make("192.168.0.2"));
    bans.set_temporary_ban(netmask::make("192.168.0.3"));
    
    bans.unset_temporary_bans();
    
    TEST_ASSERT(bans.is_banned(netmask::make("192.168.0.1")));
    TEST_ASSERT(bans.is_banned(netmask::make("127.0.0.1")));
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.2")));
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.3")));
    
    return true;
}

bool test_getters()
{
    banned_networks bans;
    bans.set_permanent_ban(netmask::make("192.168.0.1"));
    bans.set_permanent_ban(netmask::make("127.0.0.1"));
    bans.set_permanent_ban(netmask::make("169/8"));
    bans.set_temporary_ban(netmask::make("192.168.0.2"));
    bans.set_temporary_ban(netmask::make("192.168.0.3"));
    
    std::vector<netmask> permbans = bans.get_permanent_bans();
    TEST_ASSERT(permbans.size()==3);
    TEST_ASSERT(permbans[0].to_string()=="127.0.0.1");
    TEST_ASSERT(permbans[1].to_string()=="169.0.0.0/8");
    TEST_ASSERT(permbans[2].to_string()=="192.168.0.1");
    
    std::vector<netmask> tmpbans = bans.get_temporary_bans();
    TEST_ASSERT(tmpbans.size()==2);
    TEST_ASSERT(tmpbans[0].to_string()=="192.168.0.2");
    TEST_ASSERT(tmpbans[1].to_string()=="192.168.0.3");
    
    return true;
}

int main()
{
    BOOST_TEST(test_permban());
    BOOST_TEST(test_tempban());
    BOOST_TEST(test_unsetban());
    BOOST_TEST(test_unset_tempbans());
    BOOST_TEST(test_getters());
    return boost::report_errors();
}
