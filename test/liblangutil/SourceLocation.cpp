// SPDX-License-Identifier: GPL-3.0
/**
 * @author Yoichi Hirai <yoichi@ethereum.org>
 * @date 2016
 * Unit tests for the SourceLocation class.
 */

#include <liblangutil/SourceLocation.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

namespace solidity::langutil::test
{

BOOST_AUTO_TEST_SUITE(SourceLocationTest)

BOOST_AUTO_TEST_CASE(test_fail)
{
	auto const source = std::make_shared<CharStream>("lorem ipsum", "source");
	auto const sourceA = std::make_shared<CharStream>("lorem ipsum", "sourceA");
	auto const sourceB = std::make_shared<CharStream>("lorem ipsum", "sourceB");

	BOOST_CHECK(SourceLocation{} == SourceLocation{});
	BOOST_CHECK((SourceLocation{0, 3, sourceA} != SourceLocation{0, 3, sourceB}));
	BOOST_CHECK((SourceLocation{0, 3, source} == SourceLocation{0, 3, source}));
	BOOST_CHECK((SourceLocation{3, 7, source}.contains(SourceLocation{4, 6, source})));
	BOOST_CHECK((!SourceLocation{3, 7, sourceA}.contains(SourceLocation{4, 6, sourceB})));
	BOOST_CHECK((SourceLocation{3, 7, sourceA} < SourceLocation{4, 6, sourceB}));
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
