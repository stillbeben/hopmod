
#include "cubescript.hpp"

#include <limits>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

class LexCastingTest:public CppUnit::TestFixture
{
public:
    void setUp();
	void tearDown();
    
    void testParsePods();
    void testPrintPods();
    
    void testParseVector();
    void testParseList();
    
    void testPrintVector();
    void testPrintList();
    
    CPPUNIT_TEST_SUITE(LexCastingTest);
        CPPUNIT_TEST(testParsePods);
        CPPUNIT_TEST(testPrintPods);
        CPPUNIT_TEST(testParseVector);
        CPPUNIT_TEST(testParseList);
        CPPUNIT_TEST(testPrintVector);
        CPPUNIT_TEST(testPrintList);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LexCastingTest);

void LexCastingTest::setUp()
{
    
}

void LexCastingTest::tearDown()
{
    
}

void LexCastingTest::testParsePods()
{
    CPPUNIT_ASSERT( cubescript::parse_type<int>("22") == 22 );
    CPPUNIT_ASSERT( cubescript::parse_type<int>("-10") == -10 );
    CPPUNIT_ASSERT( cubescript::parse_type<unsigned long>("4294967295") == 0xFFFFFFFF );
    CPPUNIT_ASSERT( cubescript::parse_type<unsigned short>("65535") == 0xFFFF );
    
    CPPUNIT_ASSERT( cubescript::parse_type<std::string>("hello") == "hello" );
    
    CPPUNIT_ASSERT( cubescript::parse_type<bool>("1") == true );
    CPPUNIT_ASSERT( cubescript::parse_type<bool>("0") == false);
    CPPUNIT_ASSERT( cubescript::parse_type<bool>("any letters") == true);
}

void LexCastingTest::testPrintPods()
{
    CPPUNIT_ASSERT( cubescript::print_type<int>(22) == "22" );
    CPPUNIT_ASSERT( cubescript::print_type<int>(-10) == "-10");
    CPPUNIT_ASSERT( cubescript::print_type<std::string>("test test") == "test test" );
    CPPUNIT_ASSERT( cubescript::print_type<bool>(true) == "1" );
    CPPUNIT_ASSERT( cubescript::print_type<bool>(false) == "0" );
}

void LexCastingTest::testParseVector()
{
    std::vector<int> n=cubescript::parse_type< std::vector<int> >("0 \"1\" \"2\" 3      4");
    
    CPPUNIT_ASSERT( n[0] == 0 );
    CPPUNIT_ASSERT( n[1] == 1 );
    CPPUNIT_ASSERT( n[2] == 2 );
    CPPUNIT_ASSERT( n[3] == 3 );
    CPPUNIT_ASSERT( n[4] == 4 );
    
    std::vector<std::string> str=cubescript::parse_type< std::vector<std::string> >("one\ttwo three \"four five\"");
    
    CPPUNIT_ASSERT( str[0] == "one" );
    CPPUNIT_ASSERT( str[1] == "two" );
    CPPUNIT_ASSERT( str[2] == "three" );
    CPPUNIT_ASSERT( str[3] == "four five" );
}

void LexCastingTest::testParseList()
{
    std::list<int> n=cubescript::parse_type< std::list<int> >("0 \"1\" \"2\" 3      4");
    
    CPPUNIT_ASSERT( n.front() == 0 ); n.pop_front();
    CPPUNIT_ASSERT( n.front() == 1 ); n.pop_front();
    CPPUNIT_ASSERT( n.front() == 2 ); n.pop_front();
    CPPUNIT_ASSERT( n.front() == 3 ); n.pop_front();
    CPPUNIT_ASSERT( n.front() == 4 );
}

void LexCastingTest::testPrintVector()
{
    std::vector<int> n;
    n.push_back(1);
    n.push_back(2);
    n.push_back(3);
 
    CPPUNIT_ASSERT( cubescript::print_type< std::vector<int> >(n) == "\"1\" \"2\" \"3\" " );
}

void LexCastingTest::testPrintList()
{
    std::list<int> n;
    n.push_back(1);
    n.push_back(2);
    n.push_back(3);
    
    CPPUNIT_ASSERT( cubescript::print_type< std::list<int> >(n) == "\"1\" \"2\" \"3\" " );
}
