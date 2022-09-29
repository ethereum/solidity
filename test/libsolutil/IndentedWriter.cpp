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
// SPDX-License-Identifier: GPL-3.0
/**
 * Unit tests for IndentedWriter.
 */

#include <libsolutil/IndentedWriter.h>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/preprocessor/control/expr_iif.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/detail/auto_rec.hpp>
#include <boost/preprocessor/logical/bool.hpp>
#include <boost/preprocessor/logical/compl.hpp>
#include <boost/preprocessor/repetition/detail/for.hpp>
#include <boost/preprocessor/repetition/for.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/utils/basic_cstring/basic_cstring.hpp>
#include <boost/test/utils/basic_cstring/basic_cstring_fwd.hpp>
#include <boost/test/utils/lazy_ostream.hpp>
#include <iosfwd>

using namespace std;

namespace solidity::util::test
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
