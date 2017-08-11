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
 * Unit tests for IndentedWriter.
 */

#include <libdevcore/IndentedWriter.h>

#include "../TestHelper.h"

using namespace std;

namespace dev
{
namespace test
{

BOOST_AUTO_TEST_SUITE(IndentedWriterTest)

BOOST_AUTO_TEST_CASE(empty)
{
	IndentedWriter iw;
	BOOST_CHECK_EQUAL(iw.format(), "\n");
}

BOOST_AUTO_TEST_CASE(new_lines)
{
	IndentedWriter iw;
	iw.newLine();
	BOOST_CHECK_EQUAL(iw.format(), "\n");
}

BOOST_AUTO_TEST_CASE(text_without_newline)
{
	IndentedWriter iw;
	iw.add("Hello World");
	BOOST_CHECK_EQUAL(iw.format(), "Hello World\n");
}

BOOST_AUTO_TEST_CASE(text_with_newline)
{
	IndentedWriter iw;
	iw.addLine("Hello World");
	BOOST_CHECK_EQUAL(iw.format(), "Hello World\n\n");
}

BOOST_AUTO_TEST_CASE(indent)
{
	IndentedWriter iw;
	iw.addLine("Hello");
	iw.indent();
	iw.addLine("World");
	iw.unindent();
	iw.addLine("and everyone else");
	BOOST_CHECK_EQUAL(iw.format(), "Hello\n    World\nand everyone else\n\n");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
