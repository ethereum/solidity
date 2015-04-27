//
// Created by Marek Kotewicz on 27/04/15.
//

#include <boost/test/unit_test.hpp>
#include "../../libethconsole/JSV8ScopeBase.h"

using namespace std;
using namespace dev;
using namespace dev::eth;

BOOST_AUTO_TEST_SUITE(jsscope)

BOOST_AUTO_TEST_CASE(common)
{
	JSV8ScopeBase scope;
	string result = scope.evaluate("1 + 1");
	BOOST_CHECK_EQUAL(result, "2");
}

BOOST_AUTO_TEST_SUITE_END()
