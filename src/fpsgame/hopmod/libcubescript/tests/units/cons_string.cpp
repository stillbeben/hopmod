
#include "cubescript.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

class StringConstructTest:public CppUnit::TestFixture
{
public:
    void setUp();
	void tearDown();
    
    void testParse();
    void testEval();
    void testFormed();
    
    CPPUNIT_TEST_SUITE(StringConstructTest);
        CPPUNIT_TEST(testParse);
        CPPUNIT_TEST(testEval);
        CPPUNIT_TEST(testFormed);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StringConstructTest);

void StringConstructTest::setUp()
{
    
}

void StringConstructTest::tearDown()
{
    
}

void StringConstructTest::testParse()
{
    //test term chars
    {
        std::stringstream s;
        cubescript::cons::string c;
        s<<"blah\""; CPPUNIT_ASSERT( c.parse(s) == true );
        char i;
        s.get(i);
        CPPUNIT_ASSERT( !s.fail() && i=='\"');
    }

    {
        std::stringstream s;
        cubescript::cons::string c;
        s<<"blah\r\n"; CPPUNIT_ASSERT( c.parse(s) == true );
        char i;
        s.get(i);
        CPPUNIT_ASSERT( !s.fail() && i=='\r');
    }
    
    //test incremental parsing
    {
        std::stringstream s;
        cubescript::cons::string c;
        s<<"1"; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<"6"; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<"\""; CPPUNIT_ASSERT( c.parse(s) == true );
    }
}

void StringConstructTest::testEval()
{
    {
        std::stringstream s;
        cubescript::cons::string c;
        s<<"blah\""; 
        c.parse(s);
        
        CPPUNIT_ASSERT( c.eval()=="blah" );
    }
    
    {
        std::stringstream s;
        cubescript::cons::string c;
        
        s<<"b";             c.parse(s);
        s.clear(); s<<"l";  c.parse(s);
        s.clear(); s<<"a";  c.parse(s);
        s.clear(); s<<"h";  c.parse(s);
        s.clear(); s<<"\"";  c.parse(s);
        
        CPPUNIT_ASSERT( c.eval()=="blah" );
    }
    
}

void StringConstructTest::testFormed()
{
    std::stringstream s;
    cubescript::cons::string c;
    s<<"blah\""; 
    c.parse(s);
       
    CPPUNIT_ASSERT( c.formed()=="\"blah\"" );
}
