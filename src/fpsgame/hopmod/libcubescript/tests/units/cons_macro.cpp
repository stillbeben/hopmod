
#include "cubescript.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

class MacroConstructTest:public CppUnit::TestFixture
{
public:
    void setUp();
	void tearDown();
    
    void testWordMacro();
    void testFormed();
    
    CPPUNIT_TEST_SUITE(MacroConstructTest);
        CPPUNIT_TEST(testWordMacro);
        CPPUNIT_TEST(testFormed);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MacroConstructTest);

void MacroConstructTest::setUp()
{
    
}

void MacroConstructTest::tearDown()
{
    
}

void MacroConstructTest::testWordMacro()
{
    //test incremental parsing
    {
        std::stringstream s;
        cubescript::cons::macro c;
        s<<"@"; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<"1"; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<" "; CPPUNIT_ASSERT( c.parse(s) == true );
        
        CPPUNIT_ASSERT( c.get_escape_level()== 2 );
    }
}

void MacroConstructTest::testFormed()
{
    {
        std::stringstream s;
        s<<"blah ";
        cubescript::cons::macro c;
        CPPUNIT_ASSERT(c.parse(s));
        CPPUNIT_ASSERT(c.formed()=="@blah");
    }
    
    {
        std::stringstream s;
        s<<"@@@blah)";
        cubescript::cons::macro c;
        CPPUNIT_ASSERT(c.parse(s));
        CPPUNIT_ASSERT(c.formed()=="@@@@blah");
    }
}
