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

#include <test/TemporaryDirectory.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <fstream>

using namespace std;
using namespace boost::test_tools;

namespace fs = boost::filesystem;

namespace solidity::test
{

BOOST_AUTO_TEST_SUITE(TemporaryDirectoryTest)

BOOST_AUTO_TEST_CASE(TemporaryDirectory_should_create_and_delete_a_unique_and_empty_directory)
{
	fs::path dirPath;
	{
		TemporaryDirectory tempDir("temporary-directory-test-");
		dirPath = tempDir.path();

		BOOST_TEST(dirPath.stem().string().find("temporary-directory-test-") == 0);
		BOOST_TEST(fs::equivalent(dirPath.parent_path(), fs::temp_directory_path()));
		BOOST_TEST(fs::is_directory(dirPath));
		BOOST_TEST(fs::is_empty(dirPath));
	}
	BOOST_TEST(!fs::exists(dirPath));
}

BOOST_AUTO_TEST_CASE(TemporaryDirectory_should_delete_its_directory_even_if_not_empty)
{
	fs::path dirPath;
	{
		TemporaryDirectory tempDir("temporary-directory-test-");
		dirPath = tempDir.path();

		BOOST_TEST(fs::is_directory(dirPath));

		{
			ofstream tmpFile((dirPath / "test-file.txt").string());
			tmpFile << "Delete me!" << endl;
		}
		assert(fs::is_regular_file(dirPath / "test-file.txt"));
	}
	BOOST_TEST(!fs::exists(dirPath / "test-file.txt"));
}

BOOST_AUTO_TEST_CASE(TemporaryWorkingDirectory_should_change_and_restore_working_directory)
{
	fs::path originalWorkingDirectory = fs::current_path();

	try
	{
		{
			TemporaryDirectory tempDir("temporary-directory-test-");
			assert(fs::equivalent(fs::current_path(), originalWorkingDirectory));
			assert(!fs::equivalent(tempDir.path(), originalWorkingDirectory));

			TemporaryWorkingDirectory tempWorkDir(tempDir.path());

			BOOST_TEST(fs::equivalent(fs::current_path(), tempDir.path()));
		}
		BOOST_TEST(fs::equivalent(fs::current_path(), originalWorkingDirectory));

		fs::current_path(originalWorkingDirectory);
	}
	catch (...)
	{
		fs::current_path(originalWorkingDirectory);
	}
}

BOOST_AUTO_TEST_SUITE_END()

}
