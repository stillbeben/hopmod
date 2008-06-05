
#include "cubescript.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

class AliasSymbolTest:public CppUnit::TestFixture
{
public:
    void setUp();
	void tearDown();
    
    void testValue();
    void testApply();
    void testAssign();
    
    CPPUNIT_TEST_SUITE(AliasSymbolTest);
        CPPUNIT_TEST(testValue);
        CPPUNIT_TEST(testApply);
        CPPUNIT_TEST(testAssign);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AliasSymbolTest);

void AliasSymbolTest::setUp()
{
    
}

void AliasSymbolTest::tearDown()
{
    
}

void AliasSymbolTest::testValue()
{
    cubescript::alias a;
    a.push("+ 1 (+ 2 3)");
    CPPUNIT_ASSERT( a.value()=="+ 1 (+ 2 3)" );
    
    a.push("echo blah blah");
    CPPUNIT_ASSERT( a.value()=="echo blah blah" );
    
    a.pop();
    CPPUNIT_ASSERT( a.value()=="+ 1 (+ 2 3)" );
    
    a.pop();
    CPPUNIT_ASSERT( a.value().empty() );
}

void AliasSymbolTest::testApply()
{
    std::list<std::string> noargs;
    
    {
        cubescript::alias a;
        a.push("+ 1 (* 10 5)");
        CPPUNIT_ASSERT( a.apply(noargs,NULL) == "51" );
    }
    
    {
        cubescript::alias a;
        a.push("+ $arg1 (* $arg2 $arg3)");
        
        std::list<std::string> args;
        args.push_back(cubescript::print_type<int>(5));
        args.push_back(cubescript::print_type<int>(10));
        args.push_back(cubescript::print_type<int>(6));
        
        CPPUNIT_ASSERT( a.apply(args,NULL) == "65" );
    }
    
    // test default result
    {
        cubescript::alias a;
        a.push("+ 1 2; + 3 4");
        CPPUNIT_ASSERT( a.apply(noargs,NULL) == "7" );
    }
    
    // test explicit result
    {
        cubescript::alias a;
        a.push("result (+ 1 2); + 3 4");
        CPPUNIT_ASSERT( a.apply(noargs,NULL) == "3" );
    }
    
}

void AliasSymbolTest::testAssign()
{
    {
        cubescript::domain tmp;
        
        cubescript::cons::expression exp1;
        std::stringstream exp1_string;
        exp1_string<<"alias blah [+ 1 2]\r\n";
        exp1.set_domain(&tmp);
        exp1.parse(exp1_string);
        exp1.eval();
        
        CPPUNIT_ASSERT( tmp.lookup_symbol("blah") );
        
        cubescript::cons::expression exp2;
        std::stringstream exp2_string;
        exp2_string<<"blah\r\n";
        exp2.set_domain(&tmp);
        exp2.parse(exp2_string);
        
        CPPUNIT_ASSERT( exp2.eval()=="3" );
    }
    
    {
        cubescript::domain tmp;
        
        cubescript::cons::expression exp1;
        std::stringstream exp1_string;
        exp1_string<<"blah = [+ 1 2]\r\n";
        exp1.set_domain(&tmp);
        exp1.parse(exp1_string);
        exp1.eval();
        
        CPPUNIT_ASSERT( tmp.lookup_symbol("blah") );
        
        cubescript::cons::expression exp2;
        std::stringstream exp2_string;
        exp2_string<<"blah\r\n";
        exp2.set_domain(&tmp);
        exp2.parse(exp2_string);
        
        CPPUNIT_ASSERT( exp2.eval()=="3" );
    }
}
