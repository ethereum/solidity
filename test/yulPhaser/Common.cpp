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

#include <test/yulPhaser/TestHelpers.h>

#include <tools/yulPhaser/Common.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/TemporaryDirectory.h>

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include <fstream>
#include <sstream>
#include <string>

using namespace std;
using namespace boost::test_tools;
using namespace solidity::util;

namespace solidity::phaser::test
{

class ReadLinesFromFileFixture
{
protected:
	TemporaryDirectory m_tempDir;
};

namespace
{

enum class TestEnum
{
	A,
	B,
	AB,
	CD,
	EF,
	GH,
};

map<TestEnum, string> const TestEnumToStringMap =
{
	{TestEnum::A, "a"},
	{TestEnum::B, "b"},
	{TestEnum::AB, "a b"},
	{TestEnum::CD, "c-d"},
	{TestEnum::EF, "e f"},
};
map<string, TestEnum> const StringToTestEnumMap = invertMap(TestEnumToStringMap);

}

BOOST_AUTO_TEST_SUITE(Phaser, *boost::unit_test::label("nooptions"))
BOOST_AUTO_TEST_SUITE(CommonTest)

BOOST_FIXTURE_TEST_CASE(readLinesFromFile_should_return_all_lines_from_a_text_file_as_strings_without_newlines, ReadLinesFromFileFixture)
{
	{
		ofstream tmpFile((m_tempDir.path() / "test-file.txt").string());
		tmpFile << endl << "Line 1" << endl << endl << endl << "Line 2" << endl << "#" << endl << endl;
	}

	vector<string> lines = readLinesFromFile((m_tempDir.path() / "test-file.txt").string());
	BOOST_TEST((lines == vector<string>{"", "Line 1", "", "", "Line 2", "#", ""}));
}

BOOST_AUTO_TEST_CASE(deserializeChoice_should_convert_string_to_enum)
{
	istringstream aStream("a");
	TestEnum aResult;
	deserializeChoice(aStream, aResult, StringToTestEnumMap);
	BOOST_CHECK(aResult == TestEnum::A);
	BOOST_TEST(!aStream.fail());

	istringstream bStream("b");
	TestEnum bResult;
	deserializeChoice(bStream, bResult, StringToTestEnumMap);
	BOOST_CHECK(bResult == TestEnum::B);
	BOOST_TEST(!bStream.fail());

	istringstream cdStream("c-d");
	TestEnum cdResult;
	deserializeChoice(cdStream, cdResult, StringToTestEnumMap);
	BOOST_CHECK(cdResult == TestEnum::CD);
	BOOST_TEST(!cdStream.fail());
}

BOOST_AUTO_TEST_CASE(deserializeChoice_should_set_failbit_if_there_is_no_enum_corresponding_to_string)
{
	istringstream xyzStream("xyz");
	TestEnum xyzResult;
	deserializeChoice(xyzStream, xyzResult, StringToTestEnumMap);
	BOOST_TEST(xyzStream.fail());
}

BOOST_AUTO_TEST_CASE(deserializeChoice_does_not_have_to_support_strings_with_spaces)
{
	istringstream abStream("a b");
	TestEnum abResult;
	deserializeChoice(abStream, abResult, StringToTestEnumMap);
	BOOST_CHECK(abResult == TestEnum::A);
	BOOST_TEST(!abStream.fail());

	istringstream efStream("e f");
	TestEnum efResult;
	deserializeChoice(efStream, efResult, StringToTestEnumMap);
	BOOST_TEST(efStream.fail());
}

BOOST_AUTO_TEST_CASE(serializeChoice_should_convert_enum_to_string)
{
	output_test_stream output;

	serializeChoice(output, TestEnum::A, TestEnumToStringMap);
	BOOST_CHECK(output.is_equal("a"));
	BOOST_TEST(!output.fail());

	serializeChoice(output, TestEnum::AB, TestEnumToStringMap);
	BOOST_CHECK(output.is_equal("a b"));
	BOOST_TEST(!output.fail());
}

BOOST_AUTO_TEST_CASE(serializeChoice_should_set_failbit_if_there_is_no_string_corresponding_to_enum)
{
	output_test_stream output;
	serializeChoice(output, TestEnum::GH, TestEnumToStringMap);
	BOOST_TEST(output.fail());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

}
