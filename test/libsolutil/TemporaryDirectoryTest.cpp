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

#include <libsolutil/TemporaryDirectory.h>

#include <libsolidity/util/SoltestErrors.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <fstream>

using namespace std;
using namespace boost::test_tools;

namespace solidity::util::test
{

BOOST_AUTO_TEST_SUITE(TemporaryDirectoryTest)

BOOST_AUTO_TEST_CASE(TemporaryDirectory_should_create_and_delete_a_unique_and_empty_directory)
{
	boost::filesystem::path dirPath;
	{
		TemporaryDirectory tempDir("temporary-directory-test");
		dirPath = tempDir.path();

		BOOST_TEST(dirPath.stem().string().find("temporary-directory-test") == 0);
		BOOST_TEST(boost::filesystem::equivalent(dirPath.parent_path(), boost::filesystem::temp_directory_path()));
		BOOST_TEST(boost::filesystem::is_directory(dirPath));
		BOOST_TEST(boost::filesystem::is_empty(dirPath));
	}
	BOOST_TEST(!boost::filesystem::exists(dirPath));
}

BOOST_AUTO_TEST_CASE(TemporaryDirectory_should_delete_its_directory_even_if_not_empty)
{
	boost::filesystem::path dirPath;
	{
		TemporaryDirectory tempDir("temporary-directory-test");
		dirPath = tempDir.path();

		BOOST_TEST(boost::filesystem::is_directory(dirPath));

		{
			ofstream tmpFile((dirPath / "test-file.txt").string());
			tmpFile << "Delete me!" << endl;
		}
		soltestAssert(boost::filesystem::is_regular_file(dirPath / "test-file.txt"), "");
	}
	BOOST_TEST(!boost::filesystem::exists(dirPath / "test-file.txt"));
}

BOOST_AUTO_TEST_CASE(TemporaryDirectory_should_create_subdirectories)
{
	boost::filesystem::path dirPath;
	{
		TemporaryDirectory tempDir({"a", "a/", "a/b/c", "x.y/z"}, "temporary-directory-test");
		dirPath = tempDir.path();

		BOOST_TEST(boost::filesystem::is_directory(dirPath / "a"));
		BOOST_TEST(boost::filesystem::is_directory(dirPath / "a/b/c"));
		BOOST_TEST(boost::filesystem::is_directory(dirPath / "x.y/z"));
	}
}

BOOST_AUTO_TEST_CASE(TemporaryWorkingDirectory_should_change_and_restore_working_directory)
{
	boost::filesystem::path originalWorkingDirectory = boost::filesystem::current_path();

	try
	{
		{
			TemporaryDirectory tempDir("temporary-directory-test");
			soltestAssert(boost::filesystem::equivalent(boost::filesystem::current_path(), originalWorkingDirectory), "");
			soltestAssert(!boost::filesystem::equivalent(tempDir.path(), originalWorkingDirectory), "");

			TemporaryWorkingDirectory tempWorkDir(tempDir.path());

			BOOST_TEST(boost::filesystem::equivalent(boost::filesystem::current_path(), tempDir.path()));
		}
		BOOST_TEST(boost::filesystem::equivalent(boost::filesystem::current_path(), originalWorkingDirectory));

		boost::filesystem::current_path(originalWorkingDirectory);
	}
	catch (...)
	{
		boost::filesystem::current_path(originalWorkingDirectory);
	}
}

BOOST_AUTO_TEST_SUITE_END()

}
