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
#include <test/TemporaryDirectory.h>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using boost::filesystem::create_directories;
using boost::filesystem::current_path;
using solidity::test::TemporaryDirectory;
using solidity::test::TemporaryWorkingDirectory;

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(FileReaderTest)

BOOST_AUTO_TEST_CASE(default_initialization)
{
	FileReader reader;

	BOOST_TEST(reader.basePath() == "");
	BOOST_TEST(reader.allowedDirectories() == FileReader::FileSystemPathSet{""});
	BOOST_TEST(reader.sourceCodes() == FileReader::StringMap{});
}

BOOST_AUTO_TEST_CASE(initialization)
{
	TemporaryDirectory tempDir("file-reader-test-");
	create_directories(tempDir.path() / "a/b/c");

	FileReader reader(tempDir.path() / "a/b/c", {"/a", "/b", "/c/d/e"});

	BOOST_TEST(reader.basePath() == tempDir.path() / "a/b/c");
	BOOST_TEST(reader.allowedDirectories() == FileReader::FileSystemPathSet(
		{tempDir.path() / "a/b/c", "/a", "/b", "/c/d/e"}
	));
	BOOST_TEST(reader.sourceCodes() == FileReader::StringMap{});
}

BOOST_AUTO_TEST_CASE(sources)
{
	FileReader::StringMap initialSources = {
		{"/a/contract.sol", "contract C {}"},
		{"token.sol", "contract Token {}"},
		{"<stdin>", "library L {}"},
		{"..", "// comment"},
	};

	FileReader reader;

	BOOST_TEST(reader.sourceCodes() == StringMap{});

	reader.setSources(initialSources);

	BOOST_TEST(reader.sourceCodes() == initialSources);
	BOOST_TEST(reader.sourceCode("/a/contract.sol") == initialSources.at("/a/contract.sol"));
	BOOST_TEST(reader.sourceCode("token.sol") == initialSources.at("token.sol"));
	BOOST_TEST(reader.sourceCode("<stdin>") == initialSources.at("<stdin>"));
	BOOST_TEST(reader.sourceCode("..") == initialSources.at(".."));

	reader.setSource("<stdin>", "x");
	reader.setSource("y", "y");

	BOOST_TEST(reader.sourceCodes() == (StringMap{
		{"/a/contract.sol", initialSources.at("/a/contract.sol")},
		{"token.sol", initialSources.at("token.sol")},
		{"<stdin>", "x"},
		{"..", initialSources.at("..")},
		{"y", "y"},
	}));
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_relative_paths_as_is)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("contract.sol") == "contract.sol");
	BOOST_TEST(reader.toSourceUnitName("c/d/contract.sol") == "c/d/contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_absolute_paths_as_is_when_base_path_is_empty)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	solAssert(tempDirCurrent.path().is_absolute(), "");
	solAssert(tempDirOther.path().is_absolute(), "");

	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName(tempDirCurrent.path() / "contract.sol") == tempDirCurrent.path().string() + "/contract.sol");
	BOOST_TEST(reader.toSourceUnitName(tempDirOther.path() / "contract.sol") == tempDirOther.path().string() + "/contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_absolute_paths_as_is_when_base_path_matches_working_directory)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());

	FileReader reader(tempDirCurrent.path());
	BOOST_TEST(reader.toSourceUnitName(tempDirCurrent.path() / "contract.sol") == tempDirCurrent.path().string() + "/contract.sol");
	BOOST_TEST(reader.toSourceUnitName(tempDirOther.path() / "contract.sol") == tempDirOther.path().string() + "/contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_absolute_paths_as_is_when_base_path_does_not_match_working_directory)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirBase("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());

	FileReader reader(tempDirBase.path());
	BOOST_TEST(reader.toSourceUnitName(tempDirCurrent.path() / "contract.sol") == tempDirCurrent.path().string() + "/contract.sol");
	BOOST_TEST(reader.toSourceUnitName(tempDirBase.path() / "contract.sol") == tempDirBase.path().string() + "/contract.sol");
	BOOST_TEST(reader.toSourceUnitName(tempDirOther.path() / "contract.sol") == tempDirOther.path().string() + "/contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_absolute_paths_as_is_when_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.toSourceUnitName(tempDirCurrent.path() / "contract.sol") == tempDirCurrent.path().string() + "/contract.sol");
	BOOST_TEST(reader.toSourceUnitName(tempDirCurrent.path() / "base/contract.sol") == tempDirCurrent.path().string() + "/base/contract.sol");
	BOOST_TEST(reader.toSourceUnitName(tempDirOther.path() / "contract.sol") == tempDirOther.path().string() + "/contract.sol");
	BOOST_TEST(reader.toSourceUnitName(tempDirOther.path() / "base/contract.sol") == tempDirOther.path().string() + "/base/contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_relative_paths_inside_base_path_as_is_when_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.toSourceUnitName("contract.sol") == "contract.sol");
	BOOST_TEST(reader.toSourceUnitName("base/contract.sol") == "base/contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_special_stdin_path_as_is)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("<stdin>") == "<stdin>");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_empty_path_as_is)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("") == "");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_path_with_root_as_is)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("//contract.sol") == "//contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_use_urls_as_is)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("file://c/d/contract.sol") == "file://c/d/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("file:///c/d/contract.sol") == "file:///c/d/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("https://example.com/contract.sol") == "https://example.com/contract.sol");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_not_remove_redundant_slashes)
{
	FileReader reader;

	BOOST_TEST(reader.toSourceUnitName("a/b//contract.sol") == "a/b//contract.sol");
	BOOST_TEST(reader.toSourceUnitName("a/b///contract.sol") == "a/b///contract.sol");
	BOOST_TEST(reader.toSourceUnitName("a/b////contract.sol") == "a/b////contract.sol");

	BOOST_TEST(reader.toSourceUnitName("a/b/contract/") == "a/b/contract/");
	BOOST_TEST(reader.toSourceUnitName("a/b/contract//") == "a/b/contract//");
	BOOST_TEST(reader.toSourceUnitName("a/b/contract////") == "a/b/contract////");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_not_remove_dot_segments)
{
	FileReader reader;

	BOOST_TEST(reader.toSourceUnitName("./a/b/contract.sol") == "./a/b/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("././a/b/contract.sol") == "././a/b/contract.sol");

	BOOST_TEST(reader.toSourceUnitName("a/./b/contract.sol") == "a/./b/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("a/././b/contract.sol") == "a/././b/contract.sol");

	BOOST_TEST(reader.toSourceUnitName("a/b/contract/.") == "a/b/contract/.");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_not_remove_dot_dot_segments)
{
	FileReader reader;

	BOOST_TEST(reader.toSourceUnitName("../a/b/contract.sol") == "../a/b/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("../../a/b/contract.sol") == "../../a/b/contract.sol");

	BOOST_TEST(reader.toSourceUnitName("a/../b/contract.sol") == "a/../b/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("a/b/../../contract.sol") == "a/b/../../contract.sol");
	BOOST_TEST(reader.toSourceUnitName("/a/../b/contract.sol") == "/a/../b/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("/a/b/../../contract.sol") == "/a/b/../../contract.sol");

	BOOST_TEST(reader.toSourceUnitName("a/b/contract/..") == "a/b/contract/..");
	BOOST_TEST(reader.toSourceUnitName("/a/b/contract/..") == "/a/b/contract/..");
}

BOOST_AUTO_TEST_CASE(toSourceUnitName_should_not_remove_dot_dot_segments_going_beyond_root)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("/../contract/") == "/../contract/");
	BOOST_TEST(reader.toSourceUnitName("/../../contract/") == "/../../contract/");
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(toSourceUnitName_should_convert_backslashes_on_windows)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("a\\b\\contract.sol") == "a/b/contract.sol");
	BOOST_TEST(reader.toSourceUnitName("C:\\a\\b\\contract.sol") == "C:/a/b/contract.sol");
}
#else
BOOST_AUTO_TEST_CASE(toSourceUnitName_should_not_convert_backslashes_when_not_on_windows)
{
	FileReader reader;
	BOOST_TEST(reader.toSourceUnitName("a\\b\\contract.sol") == "a\\b\\contract.sol");
	BOOST_TEST(reader.toSourceUnitName("C:\\a\\b\\contract.sol") == "C:\\a\\b\\contract.sol");
}
#endif

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_not_preprend_current_directory_to_relative_paths_when_base_path_is_empty)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("contract.sol") == "contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("c/d/contract.sol") == "c/d/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_not_preprend_current_directory_to_absolute_paths_when_base_path_is_empty)
{
	TemporaryDirectory tempDir("file-reader-test-");

	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName((tempDir.path() / "contract.sol").generic_string()) == tempDir.path() / "/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName((tempDir.path() / "c/d/contract.sol").generic_string()) == tempDir.path() / "/c/d/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_prepend_absolute_base_path_to_relative_paths_when_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.fromSourceUnitName("contract.sol") == tempDirCurrent.path() / "base/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_prepend_absolute_base_path_to_relative_paths_when_base_path_is_absolute)
{
	TemporaryDirectory tempDirBase("file-reader-test-");
	solAssert(tempDirBase.path().is_absolute(), "");

	FileReader reader(tempDirBase.path());
	BOOST_TEST(reader.fromSourceUnitName("contract.sol") == tempDirBase.path() / "contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_prepend_absolute_base_path_to_absolute_paths_when_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.fromSourceUnitName("/root/contract.sol") == tempDirCurrent.path() / "base/root/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_prepend_absolute_base_path_to_absolute_paths_when_base_path_is_absolute)
{
	TemporaryDirectory tempDirBase("file-reader-test-");
	solAssert(tempDirBase.path().is_absolute(), "");

	FileReader reader(tempDirBase.path());
	BOOST_TEST(reader.fromSourceUnitName("/root/contract.sol") == tempDirBase.path() / "root/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_prepend_absolute_base_path_to_absolute_paths_when_base_path_matches_working_directory)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());

	FileReader reader(tempDirCurrent.path());
	BOOST_TEST(
		reader.fromSourceUnitName((tempDirCurrent.path() / "contract.sol").generic_string()) ==
		tempDirCurrent.path() / tempDirCurrent.path() / "contract.sol"
	);
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_return_special_stdin_path_as_is_when_base_path_is_empty)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.fromSourceUnitName("<stdin>") == tempDirCurrent.path() / "base/<stdin>");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_prepend_absolute_base_path_to_relative_paths_within_relative_base_path)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.fromSourceUnitName("contract.sol") == tempDirCurrent.path() / "base/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("base/contract.sol") == tempDirCurrent.path() / "base/base/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_return_empty_path_when_path_is_empty_and_base_path_is_empty)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("") == "");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_return_absolute_base_path_when_path_is_empty_but_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.fromSourceUnitName("") == tempDirCurrent.path() / "base");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_return_path_with_root_as_is_when_base_path_is_empty)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("//contract.sol") == "//contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_prepend_absolute_base_path_to_path_with_root_when_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.fromSourceUnitName("//contract.sol") == tempDirCurrent.path() / "base//contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_strip_protocol_from_file_urls)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("file://c/d/contract.sol") == "c/d/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("file:///c/d/contract.sol") == "/c/d/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_use_non_file_urls_as_paths)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("https://example.com/contract.sol") == "https:/example.com/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_remove_redundant_slashes)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("a/b//contract.sol") == "a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("a/b///contract.sol") == "a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("a/b////contract.sol") == "a/b/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_replace_trailing_slashes_with_a_dot)
{
	// TODO: This behavior is unintuitive. The trailing dot should not be there.
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("a/b/contract/") == "a/b/contract/.");
	BOOST_TEST(reader.fromSourceUnitName("a/b/contract//") == "a/b/contract/.");
	BOOST_TEST(reader.fromSourceUnitName("a/b/contract////") == "a/b/contract/.");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_remove_leading_dot_segments_and_prepend_working_directory_when_base_path_is_empty)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());

	// TODO: Work dir path should not be included. This is inconsistent with how empty base path behaves in other cases.
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("./a/b/contract.sol") == tempDirCurrent.path().string() + "/a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("././a/b/contract.sol") == tempDirCurrent.path().string() + "/a/b/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_remove_leading_dot_segments_and_prepend_absolute_base_directory_when_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base");

	FileReader reader("base");
	BOOST_TEST(reader.fromSourceUnitName("./a/b/contract.sol") == tempDirCurrent.path().string() + "/base/a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("././a/b/contract.sol") == tempDirCurrent.path().string() + "/base/a/b/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_remove_internal_dot_segments)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("a/./b/contract.sol") == "a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("a/././b/contract.sol") == "a/b/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_not_remove_trailing_dot_segments)
{
	// TODO: This should behave the same way as for internal and leading dot segments.
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("a/b/contract/.") == "a/b/contract/.");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_normalize_leading_dot_dot_segments_and_prepend_working_directory_when_base_path_is_empty)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("x/y/z");
	current_path(tempDirCurrent.path() / "x/y/z");

	// TODO: Work dir path should not be included. This is inconsistent with how empty base path behaves in other cases.
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("../a/b/contract.sol") == tempDirCurrent.path().string() + "/x/y/a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("../../a/b/contract.sol") == tempDirCurrent.path().string() + "/x/a/b/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_normalize_leading_dot_dot_segments_and_prepend_absolute_base_directory_when_base_path_is_relative)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	create_directories("base/base");

	FileReader reader("base/base");
	BOOST_TEST(reader.fromSourceUnitName("../a/b/contract.sol") == tempDirCurrent.path().string() + "/base/a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("../../a/b/contract.sol") == tempDirCurrent.path().string() + "/a/b/contract.sol");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_normalize_internal_and_trailing_dot_dot_segments)
{
	FileReader reader;

	BOOST_TEST(reader.fromSourceUnitName("a/../b/contract.sol") == "b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("a/b/../../contract.sol") == "contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("/a/../b/contract.sol") == "/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("/a/b/../../contract.sol") == "/contract.sol");

	BOOST_TEST(reader.fromSourceUnitName("a/b/contract/..") == "a/b");
	BOOST_TEST(reader.fromSourceUnitName("/a/b/contract/..") == "/a/b");
}

BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_remove_dot_dot_segments_going_beyond_root)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("/../contract/") == "/contract/");
	BOOST_TEST(reader.fromSourceUnitName("/../../contract/") == "/contract/");
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_convert_backslashes_on_windows)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("a\\b\\contract.sol") == "a\\b\\contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("a\\b\\contract.sol") == "a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("C:\\a\\b\\contract.sol") == "C:\\a\\b\\contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("C:\\a\\b\\contract.sol") == "C:/a/b/contract.sol");
}
#else
BOOST_AUTO_TEST_CASE(fromSourceUnitName_should_not_convert_backslashes_when_not_on_windows)
{
	FileReader reader;
	BOOST_TEST(reader.fromSourceUnitName("a\\b\\contract.sol") == "a\\b\\contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("a\\b\\contract.sol") != "a/b/contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("C:\\a\\b\\contract.sol") == "C:\\a\\b\\contract.sol");
	BOOST_TEST(reader.fromSourceUnitName("C:\\a\\b\\contract.sol") != "C:/a/b/contract.sol");
}
#endif

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
