
#include "cubescript.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

class BlockConstructTest:public CppUnit::TestFixture
{
public:
    void setUp();
	void tearDown();
    
    void testParse();
    void testEval();
    void testFormed();
    
    CPPUNIT_TEST_SUITE(BlockConstructTest);
        CPPUNIT_TEST(testParse);
        CPPUNIT_TEST(testEval);
        CPPUNIT_TEST(testFormed);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BlockConstructTest);

void BlockConstructTest::setUp()
{
    
}

void BlockConstructTest::tearDown()
{
    
}

void BlockConstructTest::testParse()
{
    {
        std::stringstream s;
        cubescript::cons::block c;
        s<<"line 1\nline2 ;\"\t@()]";
        CPPUNIT_ASSERT( c.parse(s) );
        
        char i;
        s.get(i);
        CPPUNIT_ASSERT( i==']' );
    }
}

void BlockConstructTest::testEval()
{
    std::stringstream s;
    cubescript::cons::block c;
    s<<"line 1\nline2 ;\"\t[]@()]";
    c.parse(s);
    
    CPPUNIT_ASSERT( c.eval()=="line 1\nline2 ;\"\t[]" );
}

void BlockConstructTest::testFormed()
{
    std::stringstream s;
    cubescript::cons::block c;
    s<<"line 1\nline2 ;\"\t[][@()]]";
    c.parse(s);
    
    CPPUNIT_ASSERT( c.formed()=="[line 1\nline2 ;\"\t[][@()]]" );
}
