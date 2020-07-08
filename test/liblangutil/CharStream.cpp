// SPDX-License-Identifier: GPL-3.0
/**
 * @author Rocky Bernstein <rocky.bernstein@consensys.net>
 * @date 2019
 * Unit tests for the CharStream class.
 */

#include <liblangutil/CharStream.h>
#include <liblangutil/Exceptions.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

namespace solidity::langutil::test
{

BOOST_AUTO_TEST_SUITE(CharStreamtest)

BOOST_AUTO_TEST_CASE(test_fail)
{
	auto const source = std::make_shared<CharStream>("now is the time for testing", "source");

	BOOST_CHECK('n' == source->get());
	BOOST_CHECK('n' == source->get());
	BOOST_CHECK('o' == source->advanceAndGet());
	BOOST_CHECK('n' == source->rollback(1));
	BOOST_CHECK('w' == source->setPosition(2));
	BOOST_REQUIRE_THROW(
		source->setPosition(200),
		::solidity::langutil::InternalCompilerError
	);
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
