/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Rocky Bernstein <rocky.bernstein@consensys.net>
 * @date 2019
 * Unit tests for the CharStream class.
 */

#include <liblangutil/CharStream.h>
#include <liblangutil/Exceptions.h>

#include <test/Options.h>

namespace langutil
{
namespace test
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
		::langutil::InternalCompilerError
	);
}

BOOST_AUTO_TEST_SUITE_END()

}
} // end namespaces
