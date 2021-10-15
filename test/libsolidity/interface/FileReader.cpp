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

/// Unit tests for libsolidity/interface/FileReader.h

#include <libsolidity/interface/FileReader.h>

#include <test/Common.h>
#include <test/FilesystemUtils.h>
#include <test/TemporaryDirectory.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::test;

#define TEST_CASE_NAME (boost::unit_test::framework::current_test_case().p_name)

namespace solidity::frontend::test
{

using SymlinkResolution = FileReader::SymlinkResolution;

BOOST_AUTO_TEST_SUITE(FileReaderTest)

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_absolute_path)
{
	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/.", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/./", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/./.", resolveSymlinks), "/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a", resolveSymlinks), "/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/", resolveSymlinks), "/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/.", resolveSymlinks), "/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/./a", resolveSymlinks), "/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/./a/", resolveSymlinks), "/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/./a/.", resolveSymlinks), "/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b", resolveSymlinks), "/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b/", resolveSymlinks), "/a/b/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/./b/", resolveSymlinks), "/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/../a/b/", resolveSymlinks), "/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b/c/..", resolveSymlinks), "/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b/c/../", resolveSymlinks), "/a/b/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b/c/../../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b/c/../../../", resolveSymlinks), "/");
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_relative_path)
{
	TemporaryDirectory tempDir({"x/y/z"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "x/y/z");

	// NOTE: If path to work dir contains symlinks (often the case on macOS), boost might resolve
	// them, making the path different from tempDirPath.
	boost::filesystem::path expectedPrefix = boost::filesystem::current_path().parent_path().parent_path().parent_path();
	// On Windows tempDir.path() normally contains the drive letter while the normalized path should not.
	expectedPrefix = "/" / expectedPrefix.relative_path();
	soltestAssert(expectedPrefix.is_absolute() || expectedPrefix.root_path() == "/", "");

	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS(".", resolveSymlinks), expectedPrefix / "x/y/z/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./", resolveSymlinks), expectedPrefix / "x/y/z/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS(".//", resolveSymlinks), expectedPrefix / "x/y/z/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("..", resolveSymlinks), expectedPrefix / "x/y");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../", resolveSymlinks), expectedPrefix / "x/y/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("..//", resolveSymlinks), expectedPrefix / "x/y/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a", resolveSymlinks), expectedPrefix / "x/y/z/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/.", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a", resolveSymlinks), expectedPrefix / "x/y/z/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/.", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/./", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/.//", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/./.", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/././", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/././/", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b", resolveSymlinks), expectedPrefix / "x/y/z/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b/", resolveSymlinks), expectedPrefix / "x/y/z/a/b/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../a/b", resolveSymlinks), expectedPrefix / "x/y/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../../a/b", resolveSymlinks), expectedPrefix / "x/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("./a/b", resolveSymlinks), expectedPrefix / "x/y/z/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("././a/b", resolveSymlinks), expectedPrefix / "x/y/z/a/b");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/./b/", resolveSymlinks), expectedPrefix / "x/y/z/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/../a/b/", resolveSymlinks), expectedPrefix / "x/y/z/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b/c/..", resolveSymlinks), expectedPrefix / "x/y/z/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b/c/../", resolveSymlinks), expectedPrefix / "x/y/z/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b/c/..//", resolveSymlinks), expectedPrefix / "x/y/z/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b/c/../..", resolveSymlinks), expectedPrefix / "x/y/z/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b/c/../../", resolveSymlinks), expectedPrefix / "x/y/z/a/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/b/c/../..//", resolveSymlinks), expectedPrefix / "x/y/z/a/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../../a/.././../p/../q/../a/b", resolveSymlinks), expectedPrefix / "a/b");
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_redundant_slashes)
{
	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("///", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("////", resolveSymlinks), "/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("////a/b/", resolveSymlinks), "/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a//b/", resolveSymlinks), "/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a////b/", resolveSymlinks), "/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b//", resolveSymlinks), "/a/b/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/b////", resolveSymlinks), "/a/b/");
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_unc_path)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);

	// On Windows tempDir.path() normally contains the drive letter while the normalized path should not.
	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		// UNC paths start with // or \\ followed by a name. They are used for network shares on Windows.
		// On UNIX systems they are not supported but still treated in a special way.
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("//host/", resolveSymlinks), "//host/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("//host/a/b", resolveSymlinks), "//host/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("//host/a/b/", resolveSymlinks), "//host/a/b/");

#if defined(_WIN32)
		// On Windows an UNC path can also start with \\ instead of //
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("\\\\host/", resolveSymlinks), "//host/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("\\\\host/a/b", resolveSymlinks), "//host/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("\\\\host/a/b/", resolveSymlinks), "//host/a/b/");
#else
		// On UNIX systems it's just a fancy relative path instead
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("\\\\host/", resolveSymlinks), expectedWorkDir / "\\\\host/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("\\\\host/a/b", resolveSymlinks), expectedWorkDir / "\\\\host/a/b");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("\\\\host/a/b/", resolveSymlinks), expectedWorkDir / "\\\\host/a/b/");
#endif
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_root_name_only)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	// A root **path** consists of a directory name (typically / or \) and the root name (drive
	// letter (C:), UNC host name (//host), etc.). Either can be empty. Root path as a whole may be
	// an absolute path but root name on its own is considered relative. For example on Windows
	// C:\ represents the root directory of drive C: but C: on its own refers to the current working
	// directory.

	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		// UNC paths
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("//", resolveSymlinks), "//" / expectedWorkDir);
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("//host", resolveSymlinks), "//host" / expectedWorkDir);

		// On UNIX systems root name is empty.
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("", resolveSymlinks), expectedWorkDir);

#if defined(_WIN32)
		boost::filesystem::path driveLetter = boost::filesystem::current_path().root_name();
		solAssert(!driveLetter.empty(), "");
		solAssert(driveLetter.is_relative(), "");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS(driveLetter, resolveSymlinks), expectedWorkDir);
#endif
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_stripping_root_name)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);

	soltestAssert(boost::filesystem::current_path().is_absolute(), "");
#if defined(_WIN32)
	soltestAssert(!boost::filesystem::current_path().root_name().empty(), "");
#endif

	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		boost::filesystem::path normalizedPath = FileReader::normalizeCLIPathForVFS(
			boost::filesystem::current_path(),
			resolveSymlinks
		);
		BOOST_CHECK_EQUAL(normalizedPath, "/" / boost::filesystem::current_path().relative_path());
		BOOST_TEST(normalizedPath.root_name().empty());
		BOOST_CHECK_EQUAL(normalizedPath.root_directory(), "/");
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_path_beyond_root)
{
	TemporaryWorkingDirectory tempWorkDir("/");

	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../.", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../a", resolveSymlinks), "/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../a/..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../a/../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../../a", resolveSymlinks), "/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../../a/..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/../../a/../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("/a/../../b/../..", resolveSymlinks), "/");

		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../.", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../a", resolveSymlinks), "/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../a/..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../a/../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../../a", resolveSymlinks), "/a");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../../a/..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("../../a/../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/../..", resolveSymlinks), "/");
		BOOST_CHECK_EQUAL(FileReader::normalizeCLIPathForVFS("a/../../b/../..", resolveSymlinks), "/");
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_case_sensitivity)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);

	boost::filesystem::path workDirNoSymlinks = boost::filesystem::weakly_canonical(tempDir);
	boost::filesystem::path expectedPrefix = "/" / workDirNoSymlinks.relative_path();

	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		BOOST_TEST(FileReader::normalizeCLIPathForVFS(workDirNoSymlinks / "abc", resolveSymlinks) == expectedPrefix / "abc");
		BOOST_TEST(FileReader::normalizeCLIPathForVFS(workDirNoSymlinks / "abc", resolveSymlinks) != expectedPrefix / "ABC");
		BOOST_TEST(FileReader::normalizeCLIPathForVFS(workDirNoSymlinks / "ABC", resolveSymlinks) != expectedPrefix / "abc");
		BOOST_TEST(FileReader::normalizeCLIPathForVFS(workDirNoSymlinks / "ABC", resolveSymlinks) == expectedPrefix / "ABC");
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_path_separators)
{
	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		// Even on Windows we want / as a separator.
		BOOST_TEST((
			FileReader::normalizeCLIPathForVFS("/a/b/c", resolveSymlinks).native() ==
			boost::filesystem::path("/a/b/c").native()
		));
	}
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_should_not_resolve_symlinks_unless_requested)
{
	TemporaryDirectory tempDir({"abc/"}, TEST_CASE_NAME);
	soltestAssert(tempDir.path().is_absolute(), "");

	if (!createSymlinkIfSupportedByFilesystem(tempDir.path() / "abc", tempDir.path() / "sym", true))
		return;

	boost::filesystem::path expectedPrefixWithSymlinks = "/" / tempDir.path().relative_path();
	boost::filesystem::path expectedPrefixWithoutSymlinks = "/" / boost::filesystem::weakly_canonical(tempDir).relative_path();

	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol", SymlinkResolution::Disabled),
		expectedPrefixWithSymlinks / "sym/contract.sol"
	);
	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol", SymlinkResolution::Disabled),
		expectedPrefixWithSymlinks / "abc/contract.sol"
	);

	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol", SymlinkResolution::Enabled),
		expectedPrefixWithoutSymlinks / "abc/contract.sol"
	);
	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol", SymlinkResolution::Enabled),
		expectedPrefixWithoutSymlinks / "abc/contract.sol"
	);
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_should_resolve_symlinks_in_workdir_when_path_is_relative)
{
	TemporaryDirectory tempDir({"abc/"}, TEST_CASE_NAME);
	soltestAssert(tempDir.path().is_absolute(), "");

	if (!createSymlinkIfSupportedByFilesystem(tempDir.path() / "abc", tempDir.path() / "sym", true))
		return;

	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "sym");
	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::weakly_canonical(boost::filesystem::current_path()).relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	boost::filesystem::path expectedPrefix = "/" / tempDir.path().relative_path();
	soltestAssert(expectedPrefix.is_absolute() || expectedPrefix.root_path() == "/", "");

	for (SymlinkResolution resolveSymlinks: {SymlinkResolution::Enabled, SymlinkResolution::Disabled})
	{
		BOOST_CHECK_EQUAL(
			FileReader::normalizeCLIPathForVFS("contract.sol", resolveSymlinks),
			expectedWorkDir / "contract.sol"
		);
	}

	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol", SymlinkResolution::Disabled),
		expectedPrefix / "sym/contract.sol"
	);
	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol", SymlinkResolution::Disabled),
		expectedPrefix / "abc/contract.sol"
	);

	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol", SymlinkResolution::Enabled),
		expectedWorkDir / "contract.sol"
	);
	BOOST_CHECK_EQUAL(
		FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol", SymlinkResolution::Enabled),
		expectedWorkDir / "contract.sol"
	);
}

BOOST_AUTO_TEST_CASE(isPathPrefix_file_prefix)
{
	BOOST_TEST(FileReader::isPathPrefix("/", "/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/contract.sol", "/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/contract.sol/", "/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/contract.sol/.", "/contract.sol"));

	BOOST_TEST(FileReader::isPathPrefix("/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc/def/contract.sol", "/a/bc/def/contract.sol"));

	BOOST_TEST(FileReader::isPathPrefix("/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc", "/a/bc/def/contract.sol"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc/def/contract.sol", "/a/bc/def/contract.sol"));

	BOOST_TEST(!FileReader::isPathPrefix("/contract.sol", "/token.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract", "/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract.sol", "/contract"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract.so", "/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/contract.sol", "/contract.so"));

	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/contract.sol", "/a/b/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/contract.sol", "/a/b/c/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/contract.sol", "/a/b/c/d/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/d/contract.sol", "/a/b/c/contract.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/contract.sol", "/contract.sol"));
}

BOOST_AUTO_TEST_CASE(isPathPrefix_directory_prefix)
{
	BOOST_TEST(FileReader::isPathPrefix("/", "/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/", "/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c", "/"));

	BOOST_TEST(FileReader::isPathPrefix("/", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a/", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc", "/a/bc/"));
	BOOST_TEST(FileReader::isPathPrefix("/a/bc/", "/a/bc/"));

	BOOST_TEST(!FileReader::isPathPrefix("/a", "/b/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/", "/b/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/contract.sol", "/a/b/"));

	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/", "/a/b/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c", "/a/b/"));
}

BOOST_AUTO_TEST_CASE(isPathPrefix_unc_path)
{
	BOOST_TEST(FileReader::isPathPrefix("//host/a/b/", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/a/b", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/a/", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/a", "//host/a/b/"));
	BOOST_TEST(FileReader::isPathPrefix("//host/", "//host/a/b/"));

	// NOTE: //host and // cannot be passed to isPathPrefix() because they are considered relative.

	BOOST_TEST(!FileReader::isPathPrefix("//host1/", "//host2/"));
	BOOST_TEST(!FileReader::isPathPrefix("//host1/a/b/", "//host2/a/b/"));

	BOOST_TEST(!FileReader::isPathPrefix("/a/b/c/", "//a/b/c/"));
	BOOST_TEST(!FileReader::isPathPrefix("//a/b/c/", "/a/b/c/"));
}

BOOST_AUTO_TEST_CASE(isPathPrefix_case_sensitivity)
{
	BOOST_TEST(!FileReader::isPathPrefix("/a.sol", "/A.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/A.sol", "/a.sol"));
	BOOST_TEST(!FileReader::isPathPrefix("/A/", "/a/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/", "/A/"));
	BOOST_TEST(!FileReader::isPathPrefix("/a/BC/def/", "/a/bc/def/contract.sol"));
}

BOOST_AUTO_TEST_CASE(stripPrefixIfPresent_file_prefix)
{
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/", "/contract.sol"), "contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/contract.sol", "/contract.sol"), ".");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/contract.sol/", "/contract.sol"), ".");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/contract.sol/.", "/contract.sol"), ".");

	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/", "/a/bc/def/contract.sol"), "a/bc/def/contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a", "/a/bc/def/contract.sol"), "bc/def/contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/", "/a/bc/def/contract.sol"), "bc/def/contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/bc", "/a/bc/def/contract.sol"), "def/contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/bc/def/", "/a/bc/def/contract.sol"), "contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/bc/def/contract.sol", "/a/bc/def/contract.sol"), ".");

	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/contract.sol", "/token.sol"), "/token.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/contract", "/contract.sol"), "/contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/contract.sol", "/contract"), "/contract");

	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/b/c/contract.sol", "/a/b/contract.sol"), "/a/b/contract.sol");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/b/contract.sol", "/a/b/c/contract.sol"), "/a/b/c/contract.sol");
}

BOOST_AUTO_TEST_CASE(stripPrefixIfPresent_directory_prefix)
{
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/", "/"), ".");

	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/", "/a/bc/def/"), "a/bc/def/");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a", "/a/bc/def/"), "bc/def/");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/", "/a/bc/def/"), "bc/def/");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/bc", "/a/bc/def/"), "def/");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/bc/def/", "/a/bc/def/"), ".");

	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a", "/b/"), "/b/");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/", "/b/"), "/b/");

	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/b/c/", "/a/b/"), "/a/b/");
	BOOST_CHECK_EQUAL(FileReader::stripPrefixIfPresent("/a/b/c", "/a/b/"), "/a/b/");
}

BOOST_AUTO_TEST_CASE(isUNCPath)
{
	BOOST_TEST(FileReader::isUNCPath("//"));
	BOOST_TEST(FileReader::isUNCPath("//root"));
	BOOST_TEST(FileReader::isUNCPath("//root/"));

#if defined(_WIN32)
	// On Windows boost sees these as ///, which is equivalent to /
	BOOST_TEST(!FileReader::isUNCPath("//\\"));
	BOOST_TEST(!FileReader::isUNCPath("\\\\/"));
	BOOST_TEST(!FileReader::isUNCPath("\\/\\"));

	BOOST_TEST(FileReader::isUNCPath("\\\\"));
	BOOST_TEST(FileReader::isUNCPath("\\\\root"));
	BOOST_TEST(FileReader::isUNCPath("\\\\root/"));
#else
	// On UNIX it's actually an UNC path
	BOOST_TEST(FileReader::isUNCPath("//\\"));

	// On UNIX these are just weird relative directory names consisting only of backslashes.
	BOOST_TEST(!FileReader::isUNCPath("\\\\/"));
	BOOST_TEST(!FileReader::isUNCPath("\\/\\"));

	BOOST_TEST(!FileReader::isUNCPath("\\\\"));
	BOOST_TEST(!FileReader::isUNCPath("\\\\root"));
	BOOST_TEST(!FileReader::isUNCPath("\\\\root/"));
#endif

	BOOST_TEST(!FileReader::isUNCPath("\\/"));
	BOOST_TEST(!FileReader::isUNCPath("/\\"));

	BOOST_TEST(!FileReader::isUNCPath(""));
	BOOST_TEST(!FileReader::isUNCPath("."));
	BOOST_TEST(!FileReader::isUNCPath(".."));
	BOOST_TEST(!FileReader::isUNCPath("/"));
	BOOST_TEST(!FileReader::isUNCPath("a"));
	BOOST_TEST(!FileReader::isUNCPath("a/b/c"));
	BOOST_TEST(!FileReader::isUNCPath("contract.sol"));
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
