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


namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(FileReaderTest)

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_absolute_path)
{
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/.") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./.") == "/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/.") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./a/") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/./a/.") == "/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b") == "/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/") == "/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/./b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/../a/b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/..") == "/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/../") == "/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/../../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b/c/../../../") == "/");
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_relative_path)
{
	TemporaryDirectory tempDir("file-reader-test-");
	boost::filesystem::create_directories(tempDir.path() / "x/y/z");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "x/y/z");

	// NOTE: If path to work dir contains symlinks (of then the case on macOS), boost might resolve
	// them, making the path different from tempDirPath.
	boost::filesystem::path expectedWorkDir = boost::filesystem::current_path().parent_path().parent_path().parent_path();
	// On Windows tempDir.path() normally contains the drive letter while the normalized path should not.
	expectedWorkDir = "/" / expectedWorkDir.relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS(".") == expectedWorkDir / "x/y/z/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./") == expectedWorkDir / "x/y/z/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../") == expectedWorkDir / "x/y/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a") == expectedWorkDir / "x/y/z/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/") == expectedWorkDir / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/.") == expectedWorkDir / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a") == expectedWorkDir / "x/y/z/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a/") == expectedWorkDir / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a/.") == expectedWorkDir / "x/y/z/a/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b") == expectedWorkDir / "x/y/z/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b/") == expectedWorkDir / "x/y/z/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a/b") == expectedWorkDir / "x/y/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/b") == expectedWorkDir / "x/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("./a/b") == expectedWorkDir / "x/y/z/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("././a/b") == expectedWorkDir / "x/y/z/a/b");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/./b/") == expectedWorkDir / "x/y/z/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/../a/b/") == expectedWorkDir / "x/y/z/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b/c/..") == expectedWorkDir / "x/y/z/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/b/c/../") == expectedWorkDir / "x/y/z/a/b/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/.././../p/../q/../a/b") == expectedWorkDir / "a/b");

}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_redundant_slashes)
{
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("///") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("////") == "/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("////a/b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a//b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a////b/") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b//") == "/a/b/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/b////") == "/a/b/");
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_unc_path)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());

	// On Windows tempDir.path() normally contains the drive letter while the normalized path should not.
	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	// UNC paths start with // or \\ followed by a name. They are used for network shares on Windows.
	// On UNIX systems they are not supported but still treated in a special way.
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host/") == "//host/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host/a/b") == "//host/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host/a/b/") == "//host/a/b/");

#if defined(_WIN32)
	// On Windows an UNC path can also start with \\ instead of //
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/") == "//host/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b") == "//host/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b/") == "//host/a/b/");
#else
	// On UNIX systems it's just a fancy relative path instead
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/") == expectedWorkDir / "\\\\host/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b") == expectedWorkDir / "\\\\host/a/b");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("\\\\host/a/b/") == expectedWorkDir / "\\\\host/a/b/");
#endif
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_root_name_only)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	// A root **path** consists of a directory name (typically / or \) and the root name (drive
	// letter (C:), UNC host name (//host), etc.). Either can be empty. Root path as a whole may be
	// an absolute path but root name on its own is considered relative. For example on Windows
	// C:\ represents the root directory of drive C: but C: on its own refers to the current working
	// directory.

	// UNC paths
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//") == "//" / expectedWorkDir);
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("//host") == "//host" / expectedWorkDir);

	// On UNIX systems root name is empty.
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("") == expectedWorkDir);

#if defined(_WIN32)
	boost::filesystem::path driveLetter = boost::filesystem::current_path().root_name();
	solAssert(!driveLetter.empty(), "");
	solAssert(driveLetter.is_relative(), "");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS(driveLetter) == expectedWorkDir);
#endif
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_stripping_root_name)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());

	soltestAssert(boost::filesystem::current_path().is_absolute(), "");
	soltestAssert(!boost::filesystem::current_path().root_name().empty(), "");

	boost::filesystem::path normalizedPath = FileReader::normalizeCLIPathForVFS(boost::filesystem::current_path());
	BOOST_TEST(normalizedPath == "/" / boost::filesystem::current_path().relative_path());
	BOOST_TEST(normalizedPath.root_name().empty());
	BOOST_TEST(normalizedPath.root_directory() == "/");
}
#endif

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_path_beyond_root)
{
	TemporaryWorkingDirectory tempWorkDir("/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../.") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/../../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("/a/../../b/../..") == "/");

	BOOST_TEST(FileReader::normalizeCLIPathForVFS("..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../.") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a") == "/a");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("../../a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/../..") == "/");
	BOOST_TEST(FileReader::normalizeCLIPathForVFS("a/../../b/../..") == "/");
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_case_sensitivity)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path());
	boost::filesystem::create_directories(tempDir.path() / "abc");

	boost::filesystem::path expectedPrefix = "/" / tempDir.path().relative_path();
	soltestAssert(expectedPrefix.is_absolute() || expectedPrefix.root_path() == "/", "");

	bool caseSensitiveFilesystem = boost::filesystem::create_directories(tempDir.path() / "ABC");
	soltestAssert(boost::filesystem::equivalent(tempDir.path() / "abc", tempDir.path() / "ABC") != caseSensitiveFilesystem, "");

	BOOST_TEST((FileReader::normalizeCLIPathForVFS((tempDir.path() / "abc")) == (expectedPrefix / "abc")));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS((tempDir.path() / "abc")) != (expectedPrefix / "ABC")));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS((tempDir.path() / "ABC")) != (expectedPrefix / "abc")));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS((tempDir.path() / "ABC")) == (expectedPrefix / "ABC")));
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_path_separators)
{
	// Even on Windows we want / as a separator.
	BOOST_TEST((FileReader::normalizeCLIPathForVFS("/a/b/c").native() == boost::filesystem::path("/a/b/c").native()));
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_should_not_resolve_symlinks)
{
	TemporaryDirectory tempDir("file-reader-test-");
	soltestAssert(tempDir.path().is_absolute(), "");
	boost::filesystem::create_directories(tempDir.path() / "abc");

	if (!createSymlinkIfSupportedByFilesystem(tempDir.path() / "abc", tempDir.path() / "sym", true))
		return;

	boost::filesystem::path expectedPrefix = "/" / tempDir.path().relative_path();
	soltestAssert(expectedPrefix.is_absolute() || expectedPrefix.root_path() == "/", "");

	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol") == expectedPrefix / "sym/contract.sol"));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol") == expectedPrefix / "abc/contract.sol"));
}

BOOST_AUTO_TEST_CASE(normalizeCLIPathForVFS_should_resolve_symlinks_in_workdir_when_path_is_relative)
{
	TemporaryDirectory tempDir("file-reader-test-");
	soltestAssert(tempDir.path().is_absolute(), "");
	boost::filesystem::create_directories(tempDir.path() / "abc");

	if (!createSymlinkIfSupportedByFilesystem(tempDir.path() / "abc", tempDir.path() / "sym", true))
		return;

	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "sym");
	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::weakly_canonical(boost::filesystem::current_path()).relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	boost::filesystem::path expectedPrefix = "/" / tempDir.path().relative_path();
	soltestAssert(expectedPrefix.is_absolute() || expectedPrefix.root_path() == "/", "");

	BOOST_TEST((FileReader::normalizeCLIPathForVFS("contract.sol") == expectedWorkDir / "contract.sol"));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "sym/contract.sol") == expectedPrefix / "sym/contract.sol"));
	BOOST_TEST((FileReader::normalizeCLIPathForVFS(tempDir.path() / "abc/contract.sol") == expectedPrefix / "abc/contract.sol"));
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
	BOOST_TEST(FileReader::stripPrefixIfPresent("/", "/contract.sol") == "contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/contract.sol", "/contract.sol") == ".");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/contract.sol/", "/contract.sol") == ".");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/contract.sol/.", "/contract.sol") == ".");

	BOOST_TEST(FileReader::stripPrefixIfPresent("/", "/a/bc/def/contract.sol") == "a/bc/def/contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a", "/a/bc/def/contract.sol") == "bc/def/contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/", "/a/bc/def/contract.sol") == "bc/def/contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/bc", "/a/bc/def/contract.sol") == "def/contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/bc/def/", "/a/bc/def/contract.sol") == "contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/bc/def/contract.sol", "/a/bc/def/contract.sol") == ".");


	BOOST_TEST(FileReader::stripPrefixIfPresent("/contract.sol", "/token.sol") == "/token.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/contract", "/contract.sol") == "/contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/contract.sol", "/contract") == "/contract");

	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/b/c/contract.sol", "/a/b/contract.sol") == "/a/b/contract.sol");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/b/contract.sol", "/a/b/c/contract.sol") == "/a/b/c/contract.sol");
}

BOOST_AUTO_TEST_CASE(stripPrefixIfPresent_directory_prefix)
{
	BOOST_TEST(FileReader::stripPrefixIfPresent("/", "/") == ".");

	BOOST_TEST(FileReader::stripPrefixIfPresent("/", "/a/bc/def/") == "a/bc/def/");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a", "/a/bc/def/") == "bc/def/");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/", "/a/bc/def/") == "bc/def/");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/bc", "/a/bc/def/") == "def/");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/bc/def/", "/a/bc/def/") == ".");

	BOOST_TEST(FileReader::stripPrefixIfPresent("/a", "/b/") == "/b/");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/", "/b/") == "/b/");

	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/b/c/", "/a/b/") == "/a/b/");
	BOOST_TEST(FileReader::stripPrefixIfPresent("/a/b/c", "/a/b/") == "/a/b/");
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
