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

/// Unit tests for the CommonIO routines.

#include <libsolutil/CommonIO.h>
#include <libsolutil/TemporaryDirectory.h>

#include <test/Common.h>
#include <test/FilesystemUtils.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>
#include <string>
#include <tuple>

using namespace std::string_literals;
using namespace solidity::test;

#define TEST_CASE_NAME (boost::unit_test::framework::current_test_case().p_name)

namespace solidity::util::test
{

namespace
{

/// Joins two paths in a way that matches the way used by util::absolutePath() to
/// combine the import with the reference. Not generic - only does enough to handle example
/// paths used in this test suite.
std::string absolutePathJoin(std::string const& _parent, std::string const& _suffix)
{
	if (!_parent.empty() && _parent != "/" && !_suffix.empty())
		return _parent + "/" + _suffix;

	return _parent + _suffix;
}

}

BOOST_AUTO_TEST_SUITE(CommonIOTest)

BOOST_AUTO_TEST_CASE(readFileAsString_regular_file)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFileWithContent(tempDir.path() / "test.txt", "ABC\ndef\n");

	BOOST_TEST(readFileAsString(tempDir.path() / "test.txt") == "ABC\ndef\n");
}

BOOST_AUTO_TEST_CASE(readFileAsString_directory)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	BOOST_CHECK_THROW(readFileAsString(tempDir), NotAFile);
}

BOOST_AUTO_TEST_CASE(readFileAsString_symlink)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFileWithContent(tempDir.path() / "test.txt", "ABC\ndef\n");

	if (!createSymlinkIfSupportedByFilesystem("test.txt", tempDir.path() / "symlink.txt", false))
		return;

	BOOST_TEST(readFileAsString(tempDir.path() / "symlink.txt") == "ABC\ndef\n");
}

BOOST_AUTO_TEST_CASE(readUntilEnd_no_ending_newline)
{
	std::istringstream inputStream("ABC\ndef");
	BOOST_TEST(readUntilEnd(inputStream) == "ABC\ndef");
}

BOOST_AUTO_TEST_CASE(readUntilEnd_with_ending_newline)
{
	std::istringstream inputStream("ABC\ndef\n");
	BOOST_TEST(readUntilEnd(inputStream) == "ABC\ndef\n");
}

BOOST_AUTO_TEST_CASE(readUntilEnd_cr_lf_newline)
{
	std::istringstream inputStream("ABC\r\ndef");
	BOOST_TEST(readUntilEnd(inputStream) == "ABC\r\ndef");
}

BOOST_AUTO_TEST_CASE(readUntilEnd_empty)
{
	std::istringstream inputStream("");
	BOOST_TEST(readUntilEnd(inputStream) == "");
}

BOOST_AUTO_TEST_CASE(readBytes_past_end)
{
	std::istringstream inputStream("abc");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 0), "");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 1), "a");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 20), "bc");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 20), "");
}

BOOST_DATA_TEST_CASE(
	absolutePath_direct_import,
	boost::unit_test::data::make(std::vector<char const*>{
		"",
		"/",
		"x",
		"/x",
		"x/",
		"/x/",
		"x/y",
		"/x/y",
		"x/y/",
		"/x/y/",
		".",
		"..",
		"./../.../../."
	}),
	reference
)
{
	// absolutePath() should have no effect on a direct import, regardless of what the source
	// unit name of the importing module is.

	BOOST_TEST(absolutePath("", reference) == "");
	BOOST_TEST(absolutePath("/", reference) == "/");
	BOOST_TEST(absolutePath("//", reference) == "//");
	BOOST_TEST(absolutePath("///", reference) == "///");

	BOOST_TEST(absolutePath("\\", reference) == "\\");
	BOOST_TEST(absolutePath("\\\\", reference) == "\\\\");
	BOOST_TEST(absolutePath("\\\\\\", reference) == "\\\\\\");

	BOOST_TEST(absolutePath(".\\", reference) == ".\\");
	BOOST_TEST(absolutePath(".\\\\", reference) == ".\\\\");
	BOOST_TEST(absolutePath(".\\\\\\", reference) == ".\\\\\\");

	BOOST_TEST(absolutePath("..\\", reference) == "..\\");
	BOOST_TEST(absolutePath("..\\\\", reference) == "..\\\\");
	BOOST_TEST(absolutePath("..\\\\\\", reference) == "..\\\\\\");

	BOOST_TEST(absolutePath("...", reference) == "...");
	BOOST_TEST(absolutePath("....", reference) == "....");
	BOOST_TEST(absolutePath("...a", reference) == "...a");

	BOOST_TEST(absolutePath("/./", reference) == "/./");
	BOOST_TEST(absolutePath("/././", reference) == "/././");
	BOOST_TEST(absolutePath("/../", reference) == "/../");
	BOOST_TEST(absolutePath("/../../", reference) == "/../../");
	BOOST_TEST(absolutePath("//..//..//", reference) == "//..//..//");
	BOOST_TEST(absolutePath("///..///..///", reference) == "///..///..///");

	BOOST_TEST(absolutePath("@", reference) == "@");
	BOOST_TEST(absolutePath(":", reference) == ":");
	BOOST_TEST(absolutePath("|", reference) == "|");
	BOOST_TEST(absolutePath("<>", reference) == "<>");
	BOOST_TEST(absolutePath("123", reference) == "123");
	BOOST_TEST(absolutePath("https://example.com", reference) == "https://example.com");

	BOOST_TEST(absolutePath("a", reference) == "a");
	BOOST_TEST(absolutePath("a/", reference) == "a/");
	BOOST_TEST(absolutePath("/a", reference) == "/a");
	BOOST_TEST(absolutePath("/a/", reference) == "/a/");

	BOOST_TEST(absolutePath("a/b", reference) == "a/b");
	BOOST_TEST(absolutePath("a/b/", reference) == "a/b/");
	BOOST_TEST(absolutePath("/a/b", reference) == "/a/b");
	BOOST_TEST(absolutePath("/a/b/", reference) == "/a/b/");

	BOOST_TEST(absolutePath("/.", reference) == "/.");
	BOOST_TEST(absolutePath("a.", reference) == "a.");
	BOOST_TEST(absolutePath("a/.", reference) == "a/.");
	BOOST_TEST(absolutePath("/a/.", reference) == "/a/.");
	BOOST_TEST(absolutePath("a/./", reference) == "a/./");
	BOOST_TEST(absolutePath("/a/./", reference) == "/a/./");

	BOOST_TEST(absolutePath("/..", reference) == "/..");
	BOOST_TEST(absolutePath("a..", reference) == "a..");
	BOOST_TEST(absolutePath("a/..", reference) == "a/..");
	BOOST_TEST(absolutePath("/a/..", reference) == "/a/..");
	BOOST_TEST(absolutePath("a/../", reference) == "a/../");
	BOOST_TEST(absolutePath("/a/../", reference) == "/a/../");

	BOOST_TEST(absolutePath("a//", reference) == "a//");
	BOOST_TEST(absolutePath("//a", reference) == "//a");
	BOOST_TEST(absolutePath("//a//", reference) == "//a//");

	BOOST_TEST(absolutePath("a//b", reference) == "a//b");
	BOOST_TEST(absolutePath("a//b//", reference) == "a//b//");
	BOOST_TEST(absolutePath("//a//b", reference) == "//a//b");
	BOOST_TEST(absolutePath("//a//b//", reference) == "//a//b//");

	BOOST_TEST(absolutePath("a\\b", reference) == "a\\b");
	BOOST_TEST(absolutePath("a\\b\\", reference) == "a\\b\\");
	BOOST_TEST(absolutePath("\\a\\b", reference) == "\\a\\b");
	BOOST_TEST(absolutePath("\\a\\b\\", reference) == "\\a\\b\\");
}

BOOST_DATA_TEST_CASE(
	absolutePath_relative_import_no_backtracking_into_reference,
	boost::unit_test::data::make(std::vector<std::tuple<char const*, std::string>>{
		{"", ""},
		{"/", "/"},
		{"x", ""},
		{"/x", "/"},
		{"x/", "x"},
		{"/x/", "/x"},
		{"x/y", "x"},
		{"/x/y", "/x"},
		{"x/y/", "x/y"},
		{"/x/y/", "/x/y"},
	}),
	reference,
	parent
)
{
	BOOST_TEST(absolutePath(".", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath(".//", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath(".///", reference) == absolutePathJoin(parent, ""));

	BOOST_TEST(absolutePath("./.", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("././", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath(".//./", reference) == absolutePathJoin(parent, ""));

	BOOST_TEST(absolutePath("./a", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("./a/", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("./a/b", reference) == absolutePathJoin(parent, "a/b"));
	BOOST_TEST(absolutePath("./a/b/", reference) == absolutePathJoin(parent, "a/b"));

	BOOST_TEST(absolutePath("./@", reference) == absolutePathJoin(parent, "@"));
	BOOST_TEST(absolutePath("./:", reference) == absolutePathJoin(parent, ":"));
	BOOST_TEST(absolutePath("./|", reference) == absolutePathJoin(parent, "|"));
	BOOST_TEST(absolutePath("./<>", reference) == absolutePathJoin(parent, "<>"));
	BOOST_TEST(absolutePath("./123", reference) == absolutePathJoin(parent, "123"));
	BOOST_TEST(absolutePath("./https://example.com", reference) == absolutePathJoin(parent, "https:/example.com"));

	BOOST_TEST(absolutePath(".//a", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath(".//a//", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath(".//a//b", reference) == absolutePathJoin(parent, "a/b"));
	BOOST_TEST(absolutePath(".//a//b//", reference) == absolutePathJoin(parent, "a/b"));

	BOOST_TEST(absolutePath("./a\\", reference) == absolutePathJoin(parent, "a\\"));
	BOOST_TEST(absolutePath("./a\\b", reference) == absolutePathJoin(parent, "a\\b"));
	BOOST_TEST(absolutePath("./a\\b\\", reference) == absolutePathJoin(parent, "a\\b\\"));

	BOOST_TEST(absolutePath("./a/.", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("./a/b/.", reference) == absolutePathJoin(parent, "a/b"));
	BOOST_TEST(absolutePath("./a/./b/.", reference) == absolutePathJoin(parent, "a/b"));

	BOOST_TEST(absolutePath("./a/..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./a/../", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./a/b/..", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("./a/b/../", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("./a/../b", reference) == absolutePathJoin(parent, "b"));
	BOOST_TEST(absolutePath("./a/../b/", reference) == absolutePathJoin(parent, "b"));
	BOOST_TEST(absolutePath("./a/../b/..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./a/../b/../", reference) == absolutePathJoin(parent, ""));

	BOOST_TEST(absolutePath("./a\\..", reference) == absolutePathJoin(parent, "a\\.."));
	BOOST_TEST(absolutePath("./a\\..\\", reference) == absolutePathJoin(parent, "a\\..\\"));
	BOOST_TEST(absolutePath("./a\\b\\..", reference) == absolutePathJoin(parent, "a\\b\\.."));
	BOOST_TEST(absolutePath("./a\\b\\..\\", reference) == absolutePathJoin(parent, "a\\b\\..\\"));
	BOOST_TEST(absolutePath("./a\\..\\b", reference) == absolutePathJoin(parent, "a\\..\\b"));
	BOOST_TEST(absolutePath("./a\\..\\b\\", reference) == absolutePathJoin(parent, "a\\..\\b\\"));
	BOOST_TEST(absolutePath("./a\\..\\b\\..\\", reference) == absolutePathJoin(parent, "a\\..\\b\\..\\"));

	BOOST_TEST(absolutePath("./a\\../..", reference) == absolutePathJoin(parent, ""));
}

BOOST_DATA_TEST_CASE(
	absolutePath_relative_import_with_backtracking_1_level_into_reference,
	boost::unit_test::data::make(std::vector<std::tuple<char const*, char const*>>{
		{"", ""},
		{"/", ""},
		{"x", ""},
		{"/x", ""},
		{"x/", ""},
		{"/x/", "/"},
		{"x/y", ""},
		{"/x/y", "/"},
		{"x/y/", "x"},
		{"/x/y/", "/x"},
	}),
	reference,
	parent
)
{
	BOOST_TEST(absolutePath("..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("../", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("..//", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("..///", reference) == absolutePathJoin(parent, ""));

	BOOST_TEST(absolutePath("../a", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("../a/", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("../a/b", reference) == absolutePathJoin(parent, "a/b"));
	BOOST_TEST(absolutePath("../a/b/", reference) == absolutePathJoin(parent, "a/b"));

	BOOST_TEST(absolutePath("../@", reference) == absolutePathJoin(parent, "@"));
	BOOST_TEST(absolutePath("../:", reference) == absolutePathJoin(parent, ":"));
	BOOST_TEST(absolutePath("../|", reference) == absolutePathJoin(parent, "|"));
	BOOST_TEST(absolutePath("../<>", reference) == absolutePathJoin(parent, "<>"));
	BOOST_TEST(absolutePath("../123", reference) == absolutePathJoin(parent, "123"));
	BOOST_TEST(absolutePath("../https://example.com", reference) == absolutePathJoin(parent, "https:/example.com"));

	BOOST_TEST(absolutePath("../a/..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("../a/b/..", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("../a/b/../..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("../a/../b/..", reference) == absolutePathJoin(parent, ""));

	BOOST_TEST(absolutePath("./../a/..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./../a/b/..", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("./../a/b/../..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./../a/../b/..", reference) == absolutePathJoin(parent, ""));

	BOOST_TEST(absolutePath("./a/../..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./a/b/../../..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./a/../b/../..", reference) == absolutePathJoin(parent, ""));
	BOOST_TEST(absolutePath("./a/../../b/..", reference) == absolutePathJoin(parent, ""));

	BOOST_TEST(absolutePath("./a/../../c", reference) == absolutePathJoin(parent, "c"));
	BOOST_TEST(absolutePath("./a/b/../../../c", reference) == absolutePathJoin(parent, "c"));
	BOOST_TEST(absolutePath("./a/../b/../../c", reference) == absolutePathJoin(parent, "c"));
	BOOST_TEST(absolutePath("./a/../../b/../c/d", reference) == absolutePathJoin(parent, "c/d"));

	BOOST_TEST(absolutePath("..//a", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath("..//a//", reference) == absolutePathJoin(parent, "a"));
	BOOST_TEST(absolutePath(".//a//..//..//b", reference) == absolutePathJoin(parent, "b"));
}

BOOST_AUTO_TEST_CASE(absolutePath_relative_import_with_backtracking_multiple_levels_level_into_reference)
{
	BOOST_TEST(absolutePath("..", "x/y/z/") == "x/y");
	BOOST_TEST(absolutePath("../..", "x/y/z/") == "x");
	BOOST_TEST(absolutePath("../../..", "x/y/z/") == "");
	BOOST_TEST(absolutePath("../../../..", "x/y/z/") == "");

	BOOST_TEST(absolutePath("..", "/x/y/z/") == "/x/y");
	BOOST_TEST(absolutePath("../..", "/x/y/z/") == "/x");
	BOOST_TEST(absolutePath("../../..", "/x/y/z/") == "/");
	BOOST_TEST(absolutePath("../../../..", "/x/y/z/") == "");

	BOOST_TEST(absolutePath("../../../../a", "x/y/z/") == "a");
	BOOST_TEST(absolutePath("../../../../a/..", "x/y/z/") == "");
	BOOST_TEST(absolutePath("../../../../a/../a", "x/y/z/") == "a");

	BOOST_TEST(absolutePath("../.", "x/y/z/") == "x/y");
	BOOST_TEST(absolutePath(".././..", "x/y/z/") == "x");
	BOOST_TEST(absolutePath(".././.././..", "x/y/z/") == "");
	BOOST_TEST(absolutePath("./../././.././.././..", "x/y/z/") == "");
}

BOOST_AUTO_TEST_CASE(absolutePath_relative_import_and_relative_segments_in_reference)
{
	BOOST_TEST(absolutePath(".", "x/../y/./z/") == "x/../y/./z");
	BOOST_TEST(absolutePath("..", "x/../y/./z/") == "x/../y/.");
	BOOST_TEST(absolutePath("../..", "x/../y/./z/") == "x/../y");
	BOOST_TEST(absolutePath("../../..", "x/../y/./z/") == "x/..");
	BOOST_TEST(absolutePath("../../../..", "x/../y/./z/") == "x");
	BOOST_TEST(absolutePath("../../../../..", "x/../y/./z/") == "");
	BOOST_TEST(absolutePath("../../../../../..", "x/../y/./z/") == "");
}

BOOST_AUTO_TEST_CASE(absolutePath_relative_import_and_consecutive_slashes_in_reference)
{
	BOOST_TEST(absolutePath(".", "/x/") == "/x");
	BOOST_TEST(absolutePath("..", "/x/") == "/");
	BOOST_TEST(absolutePath("../..", "/x/") == "");

	BOOST_TEST(absolutePath(".", "//x//") == "//x//");
	BOOST_TEST(absolutePath("..", "//x//") == "//x");
	BOOST_TEST(absolutePath("../..", "//x//") == "");

	BOOST_TEST(absolutePath(".", "///x///") == "///x");
	BOOST_TEST(absolutePath("..", "///x///") == "/");
	BOOST_TEST(absolutePath("../..", "///x///") == "");
}

BOOST_AUTO_TEST_CASE(absolutePath_relative_import_and_unnormalized_slashes_in_reference)
{
	BOOST_TEST(absolutePath(".", "//x//y\\z/\\//") == "//x//y\\z/\\");
	BOOST_TEST(absolutePath("..", "//x//y\\z/\\//") == "//x//y\\z");
	BOOST_TEST(absolutePath("../..", "//x//y\\z/\\//") == "//x/");
	BOOST_TEST(absolutePath("../../..", "//x//y\\z/\\//") == "//x");
	BOOST_TEST(absolutePath("../../../..", "//x//y\\z/\\//") == "");
}

BOOST_AUTO_TEST_CASE(sanitizePath_direct_import)
{
	BOOST_TEST(sanitizePath("") == "");
	BOOST_TEST(sanitizePath("/") == "/");
	BOOST_TEST(sanitizePath("//") == "//");
	BOOST_TEST(sanitizePath("///") == "///");

	BOOST_TEST(sanitizePath("\\") == "\\");
	BOOST_TEST(sanitizePath("\\\\") == "\\\\");
	BOOST_TEST(sanitizePath("\\\\\\") == "\\\\\\");

	BOOST_TEST(sanitizePath(".\\") == ".\\");
	BOOST_TEST(sanitizePath(".\\\\") == ".\\\\");
	BOOST_TEST(sanitizePath(".\\\\\\") == ".\\\\\\");

	BOOST_TEST(sanitizePath("..\\") == "..\\");
	BOOST_TEST(sanitizePath("..\\\\") == "..\\\\");
	BOOST_TEST(sanitizePath("..\\\\\\") == "..\\\\\\");

	BOOST_TEST(sanitizePath("...") == "...");
	BOOST_TEST(sanitizePath("....") == "....");
	BOOST_TEST(sanitizePath("...a") == "...a");

	BOOST_TEST(sanitizePath("/./") == "/./");
	BOOST_TEST(sanitizePath("/././") == "/././");
	BOOST_TEST(sanitizePath("/../") == "/../");
	BOOST_TEST(sanitizePath("/../../") == "/../../");
	BOOST_TEST(sanitizePath("//..//..//") == "//..//..//");
	BOOST_TEST(sanitizePath("///..///..///") == "///..///..///");

	BOOST_TEST(sanitizePath("@") == "@");
	BOOST_TEST(sanitizePath(":") == ":");
	BOOST_TEST(sanitizePath("|") == "|");
	BOOST_TEST(sanitizePath("<>") == "<>");
	BOOST_TEST(sanitizePath("123") == "123");
	BOOST_TEST(sanitizePath("https://example.com") == "https://example.com");

	BOOST_TEST(sanitizePath("a") == "a");
	BOOST_TEST(sanitizePath("a/") == "a/");
	BOOST_TEST(sanitizePath("/a") == "/a");
	BOOST_TEST(sanitizePath("/a/") == "/a/");

	BOOST_TEST(sanitizePath("a/b") == "a/b");
	BOOST_TEST(sanitizePath("a/b/") == "a/b/");
	BOOST_TEST(sanitizePath("/a/b") == "/a/b");
	BOOST_TEST(sanitizePath("/a/b/") == "/a/b/");

	BOOST_TEST(sanitizePath("/.") == "/.");
	BOOST_TEST(sanitizePath("a.") == "a.");
	BOOST_TEST(sanitizePath("a/.") == "a/.");
	BOOST_TEST(sanitizePath("/a/.") == "/a/.");
	BOOST_TEST(sanitizePath("a/./") == "a/./");
	BOOST_TEST(sanitizePath("/a/./") == "/a/./");

	BOOST_TEST(sanitizePath("/..") == "/..");
	BOOST_TEST(sanitizePath("a..") == "a..");
	BOOST_TEST(sanitizePath("a/..") == "a/..");
	BOOST_TEST(sanitizePath("/a/..") == "/a/..");
	BOOST_TEST(sanitizePath("a/../") == "a/../");
	BOOST_TEST(sanitizePath("/a/../") == "/a/../");

	BOOST_TEST(sanitizePath("a//") == "a//");
	BOOST_TEST(sanitizePath("//a") == "//a");
	BOOST_TEST(sanitizePath("//a//") == "//a//");

	BOOST_TEST(sanitizePath("a//b") == "a//b");
	BOOST_TEST(sanitizePath("a//b//") == "a//b//");
	BOOST_TEST(sanitizePath("//a//b") == "//a//b");
	BOOST_TEST(sanitizePath("//a//b//") == "//a//b//");

	BOOST_TEST(sanitizePath("a\\b") == "a\\b");
	BOOST_TEST(sanitizePath("a\\b\\") == "a\\b\\");
	BOOST_TEST(sanitizePath("\\a\\b") == "\\a\\b");
	BOOST_TEST(sanitizePath("\\a\\b\\") == "\\a\\b\\");
}

BOOST_AUTO_TEST_CASE(sanitizePath_relative_import)
{
	BOOST_TEST(sanitizePath(".") == ".");
	BOOST_TEST(sanitizePath("./") == "./");
	BOOST_TEST(sanitizePath(".//") == ".//");
	BOOST_TEST(sanitizePath(".///") == ".///");

	BOOST_TEST(sanitizePath("./.") == "./.");
	BOOST_TEST(sanitizePath("././") == "././");
	BOOST_TEST(sanitizePath(".//./") == ".//./");

	BOOST_TEST(sanitizePath("./a") == "./a");
	BOOST_TEST(sanitizePath("./a/") == "./a/");
	BOOST_TEST(sanitizePath("./a/b") == "./a/b");
	BOOST_TEST(sanitizePath("./a/b/") == "./a/b/");

	BOOST_TEST(sanitizePath("./@") == "./@");
	BOOST_TEST(sanitizePath("./:") == "./:");
	BOOST_TEST(sanitizePath("./|") == "./|");
	BOOST_TEST(sanitizePath("./<>") == "./<>");
	BOOST_TEST(sanitizePath("./123") == "./123");
	BOOST_TEST(sanitizePath("./https://example.com") == "./https://example.com");

	BOOST_TEST(sanitizePath(".//a") == ".//a");
	BOOST_TEST(sanitizePath(".//a//") == ".//a//");
	BOOST_TEST(sanitizePath(".//a//b") == ".//a//b");
	BOOST_TEST(sanitizePath(".//a//b//") == ".//a//b//");

	BOOST_TEST(sanitizePath("./a\\") == "./a\\");
	BOOST_TEST(sanitizePath("./a\\b") == "./a\\b");
	BOOST_TEST(sanitizePath("./a\\b\\") == "./a\\b\\");

	BOOST_TEST(sanitizePath("./a/.") == "./a/.");
	BOOST_TEST(sanitizePath("./a/b/.") == "./a/b/.");
	BOOST_TEST(sanitizePath("./a/./b/.") == "./a/./b/.");

	BOOST_TEST(sanitizePath("./a/..") == "./a/..");
	BOOST_TEST(sanitizePath("./a/b/..") == "./a/b/..");
	BOOST_TEST(sanitizePath("./a/../b/../") == "./a/../b/../");

	BOOST_TEST(sanitizePath("./a\\..\\b\\..\\") == "./a\\..\\b\\..\\");
	BOOST_TEST(sanitizePath("./a\\../..") == "./a\\../..");

	BOOST_TEST(sanitizePath("..") == "..");
	BOOST_TEST(sanitizePath("../") == "../");
	BOOST_TEST(sanitizePath("..//") == "..//");
	BOOST_TEST(sanitizePath("..///") == "..///");

	BOOST_TEST(sanitizePath("../..") == "../..");
	BOOST_TEST(sanitizePath("../../") == "../../");
	BOOST_TEST(sanitizePath("../../../") == "../../../");
	BOOST_TEST(sanitizePath("../../../../") == "../../../../");

	BOOST_TEST(sanitizePath("../a") == "../a");
	BOOST_TEST(sanitizePath("../a/..") == "../a/..");
	BOOST_TEST(sanitizePath("./a/../../b/../c/d") == "./a/../../b/../c/d");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::util::test
