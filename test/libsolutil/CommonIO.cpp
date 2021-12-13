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

#include <test/Common.h>
#include <test/FilesystemUtils.h>
#include <test/TemporaryDirectory.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <fstream>
#include <string>

using namespace std;
using namespace solidity::test;

#define TEST_CASE_NAME (boost::unit_test::framework::current_test_case().p_name)

namespace solidity::util::test
{

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
	istringstream inputStream("ABC\ndef");
	BOOST_TEST(readUntilEnd(inputStream) == "ABC\ndef");
}

BOOST_AUTO_TEST_CASE(readUntilEnd_with_ending_newline)
{
	istringstream inputStream("ABC\ndef\n");
	BOOST_TEST(readUntilEnd(inputStream) == "ABC\ndef\n");
}

BOOST_AUTO_TEST_CASE(readUntilEnd_cr_lf_newline)
{
	istringstream inputStream("ABC\r\ndef");
	BOOST_TEST(readUntilEnd(inputStream) == "ABC\r\ndef");
}

BOOST_AUTO_TEST_CASE(readUntilEnd_empty)
{
	istringstream inputStream("");
	BOOST_TEST(readUntilEnd(inputStream) == "");
}

BOOST_AUTO_TEST_CASE(readBytes_past_end)
{
	istringstream inputStream("abc");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 0), "");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 1), "a");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 20), "bc");
	BOOST_CHECK_EQUAL(readBytes(inputStream, 20), "");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::util::test
