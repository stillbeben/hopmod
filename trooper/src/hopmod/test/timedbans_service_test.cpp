#include "test.hpp"
#include "../timedbans_service.hpp"

bool test_use()
{
    banned_networks bans;
    timedbans_service timedbans(bans);
    
    timedbans.update(0);
    
    timedbans.set_ban(netmask::make("127.0.0.1"),3);
    timedbans.set_ban(netmask::make("192.168.0.1"),1);
    
    TEST_ASSERT(bans.is_banned(netmask::make("192.168.0.1")));
    TEST_ASSERT(bans.is_banned(netmask::make("127.0.0.1")));
    
    timedbans.update(1000);
    
    TEST_ASSERT(!bans.is_banned(netmask::make("192.168.0.1")));
    TEST_ASSERT(bans.is_banned(netmask::make("127.0.0.1")));
    
    timedbans.update(3000);
    
    TEST_ASSERT(!bans.is_banned(netmask::make("127.0.0.1")));
    
    return true;
}

int main()
{
    BOOST_TEST(test_use());
    return boost::report_errors();
}
