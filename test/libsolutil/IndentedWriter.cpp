// SPDX-License-Identifier: GPL-3.0
/**
 * Unit tests for IndentedWriter.
 */

#include <libsolutil/IndentedWriter.h>

#include <test/Common.h>

#include <boost/test/unit_test.hpp>

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
