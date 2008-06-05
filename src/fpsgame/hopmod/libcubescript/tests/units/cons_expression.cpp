
#include "cubescript.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

static int counter_count=0;

static int counter()
{
    return ++counter_count;
}

class ExpressionConstructTest:public CppUnit::TestFixture
{
public:
    void setUp();
	void tearDown();
    
    void testParse();
    void testEval();
    void testFormed();
    
    CPPUNIT_TEST_SUITE(ExpressionConstructTest);
        CPPUNIT_TEST(testParse);
        CPPUNIT_TEST(testEval);
        CPPUNIT_TEST(testFormed);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ExpressionConstructTest);

void ExpressionConstructTest::setUp()
{
    
}

void ExpressionConstructTest::tearDown()
{
    
}

void ExpressionConstructTest::testParse()
{
    {
        std::stringstream s;
        cubescript::cons::expression c;
        s<<"command [arg1] (arg2) arg3; command\r\n"; 
        CPPUNIT_ASSERT( c.parse(s) );
        char i;s.get(i);
        CPPUNIT_ASSERT( !s.fail() && i==';');
    }
    
    //test incremental parsing
    {
        std::stringstream s;
        cubescript::cons::expression c;
        s<<"command"; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<" "; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<"[arg1]"; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<" (arg2) "; CPPUNIT_ASSERT( c.parse(s) == false );
        s.clear(); s<<"; command"; CPPUNIT_ASSERT( c.parse(s) == true );
        s.clear(); s<<"\r\n"; CPPUNIT_ASSERT( c.parse(s) == true );
    }
}

void ExpressionConstructTest::testEval()
{
    {
        std::stringstream s;
        cubescript::cons::expression c;
        s<<"+ 1 (+ 2 (+ 3 4))"; 
        c.parse(s);
        CPPUNIT_ASSERT( c.eval()=="10" );
    }
    
    {
        std::stringstream s;
        cubescript::cons::expression c;
        
        s<<"+ 1 (+ 2"; c.parse(s);
        s.clear(); s<<" (+ 3"; c.parse(s);
        s.clear(); s<<" 4))"; c.parse(s);
        
        CPPUNIT_ASSERT( c.eval()=="10" );
    }
    
    {
        cubescript::function0<int> func_counter(&counter);
        cubescript::domain tmp;
        tmp.register_symbol("counter",&func_counter);
        
        counter_count=0;
        
        cubescript::cons::expression exp1;
        exp1.set_domain(&tmp);
        
        cubescript::cons::expression exp2;
        exp2.set_domain(&tmp);
        
        cubescript::cons::expression exp3;
        exp3.set_domain(&tmp);
        
        std::stringstream exps;
        exps<<"counter; counter; counter\r\n";
        
        exp1.parse(exps); CPPUNIT_ASSERT(exps.get()==';');
        exp1.eval();
        
        exp2.parse(exps); CPPUNIT_ASSERT(exps.get()==';');
        exp2.eval();
        
        exp3.parse(exps); CPPUNIT_ASSERT(exps.get()=='\r');
        exp3.eval();
        
        CPPUNIT_ASSERT( counter_count == 3 );
    }
}

void ExpressionConstructTest::testFormed()
{
    {
        std::stringstream s;
        cubescript::cons::expression c;
        s<<"command [arg1] (arg2); command\r\n"; 
        c.parse(s);
        CPPUNIT_ASSERT( c.formed()=="command [arg1] (arg2)" ); 
    }
}
