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

namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(CommonIOTest)

BOOST_AUTO_TEST_CASE(readFileAsString_regular_file)
{
	TemporaryDirectory tempDir("common-io-test-");
	createFileWithContent(tempDir.path() / "test.txt", "ABC\ndef\n");

	BOOST_TEST(readFileAsString((tempDir.path() / "test.txt").string()) == "ABC\ndef\n");
}

BOOST_AUTO_TEST_CASE(readFileAsString_directory)
{
	TemporaryDirectory tempDir("common-io-test-");
	BOOST_CHECK_THROW(readFileAsString(tempDir.path().string()), NotAFile);
}

BOOST_AUTO_TEST_CASE(readFileAsString_symlink)
{
	TemporaryDirectory tempDir("common-io-test-");
	createFileWithContent(tempDir.path() / "test.txt", "ABC\ndef\n");

	if (!createSymlinkIfSupportedByFilesystem("test.txt", tempDir.path() / "symlink.txt"))
		return;

	BOOST_TEST(readFileAsString((tempDir.path() / "symlink.txt").string()) == "ABC\ndef\n");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::util::test
