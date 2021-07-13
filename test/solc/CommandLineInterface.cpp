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

/// Unit tests for solc/CommandLineInterface.h

#include <solc/CommandLineInterface.h>

#include <test/Common.h>
#include <test/FilesystemUtils.h>
#include <test/TemporaryDirectory.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <boost/test/unit_test.hpp>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/map.hpp>

#include <fstream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::test;

using PathSet = set<boost::filesystem::path>;

namespace
{

struct OptionsReaderAndMessages
{
	bool success;
	CommandLineOptions options;
	FileReader reader;
	optional<string> standardJsonInput;
	string stdoutContent;
	string stderrContent;
};

OptionsReaderAndMessages parseCommandLineAndReadInputFiles(vector<string> const& commandLine)
{
	size_t argc = commandLine.size();
	vector<char const*> argv(commandLine.size() + 1);

	// C++ standard mandates argv[argc] to be NULL
	argv[argc] = nullptr;

	for (size_t i = 0; i < argc; ++i)
		argv[i] = commandLine[i].c_str();

	stringstream sin, sout, serr;
	CommandLineInterface cli(sin, sout, serr);
	bool success = cli.parseArguments(static_cast<int>(argc), argv.data());
	success = success && cli.readInputFiles();

	return {success, cli.options(), cli.fileReader(), cli.standardJsonInput(), sout.str(), serr.str()};
}

CommandLineOptions defaultCommandLineOptions()
{
	CommandLineOptions options;

	options.optimizer.expectedExecutionsPerDeployment = 200;
	options.modelChecker.initialize = true;
	options.modelChecker.settings = {
		ModelCheckerContracts::Default(),
		ModelCheckerEngine::None(),
		ModelCheckerTargets::Default(),
		nullopt,
	};

	return options;
}

} // namespace

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(CommandLineInterfaceTest)

BOOST_AUTO_TEST_CASE(cli_input)
{
	TemporaryDirectory tempDir1("file-reader-test-");
	TemporaryDirectory tempDir2("file-reader-test-");
	createEmptyFilesWithParentDirs({tempDir1.path() / "input1.sol"});
	createEmptyFilesWithParentDirs({tempDir2.path() / "input2.sol"});

	boost::filesystem::path expectedDir1 = "/" / tempDir1.path().relative_path();
	boost::filesystem::path expectedDir2 = "/" / tempDir2.path().relative_path();
	soltestAssert(expectedDir1.is_absolute() || expectedDir1.root_path() == "/", "");
	soltestAssert(expectedDir2.is_absolute() || expectedDir2.root_path() == "/", "");

	vector<ImportRemapper::Remapping> expectedRemappings = {
		{"", "a", "b/c/d"},
		{"a", "b", "c/d/e/"},
	};
	map<string, string> expectedSources = {
		{"<stdin>", "\n"},
		{(expectedDir1 / "input1.sol").generic_string(), ""},
		{(expectedDir2 / "input2.sol").generic_string(), ""},
	};
	PathSet expectedAllowedPaths = {
		boost::filesystem::canonical(tempDir1.path()),
		boost::filesystem::canonical(tempDir2.path()),
		"b/c",
		"c/d/e"
	};

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({
		"solc",
		"a=b/c/d",
		(tempDir1.path() / "input1.sol").string(),
		(tempDir2.path() / "input2.sol").string(),
		"a:b=c/d/e/",
		"-",
	});

	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST((result.options.input.mode == InputMode::Compiler));
	BOOST_TEST(result.options.input.addStdin);
	BOOST_TEST(result.options.input.remappings == expectedRemappings);
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedPaths);
}

BOOST_AUTO_TEST_CASE(cli_ignore_missing_some_files_exist)
{
	TemporaryDirectory tempDir1("file-reader-test-");
	TemporaryDirectory tempDir2("file-reader-test-");
	createEmptyFilesWithParentDirs({tempDir1.path() / "input1.sol"});

	boost::filesystem::path expectedDir1 = "/" / tempDir1.path().relative_path();
	soltestAssert(expectedDir1.is_absolute() || expectedDir1.root_path() == "/", "");

	// NOTE: Allowed paths should not be added for skipped files.
	map<string, string> expectedSources = {{(expectedDir1 / "input1.sol").generic_string(), ""}};
	PathSet expectedAllowedPaths = {boost::filesystem::canonical(tempDir1.path())};

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({
		"solc",
		(tempDir1.path() / "input1.sol").string(),
		(tempDir2.path() / "input2.sol").string(),
		"--ignore-missing",
	});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "\"" + (tempDir2.path() / "input2.sol").string() + "\" is not found. Skipping.\n");
	BOOST_TEST((result.options.input.mode == InputMode::Compiler));
	BOOST_TEST(!result.options.input.addStdin);
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedPaths);
}

BOOST_AUTO_TEST_CASE(cli_ignore_missing_no_files_exist)
{
	TemporaryDirectory tempDir("file-reader-test-");

	string expectedMessage =
		"\"" + (tempDir.path() / "input1.sol").string() + "\" is not found. Skipping.\n"
		"\"" + (tempDir.path() / "input2.sol").string() + "\" is not found. Skipping.\n"
		"All specified input files either do not exist or are not regular files.\n";

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({
		"solc",
		(tempDir.path() / "input1.sol").string(),
		(tempDir.path() / "input2.sol").string(),
		"--ignore-missing",
	});
	BOOST_TEST(!result.success);
	BOOST_TEST(result.stderrContent == expectedMessage);
}

BOOST_AUTO_TEST_CASE(cli_not_a_file)
{
	TemporaryDirectory tempDir("file-reader-test-");

	string expectedMessage = "\"" + tempDir.path().string() + "\" is not a valid file.\n";

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({"solc", tempDir.path().string()});
	BOOST_TEST(!result.success);
	BOOST_TEST(result.stderrContent == expectedMessage);
}

BOOST_AUTO_TEST_CASE(standard_json_base_path)
{
	TemporaryDirectory tempDir("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path().root_path());

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({
		"solc",
		"--standard-json",
		"--base-path=" + tempDir.path().string(),
	});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST((result.options.input.mode == InputMode::StandardJson));
	BOOST_TEST(result.options.input.addStdin);
	BOOST_TEST(result.options.input.paths.empty());
	BOOST_TEST(result.reader.sourceCodes().empty());
	BOOST_TEST(result.reader.allowedDirectories().empty());
	BOOST_TEST(result.reader.basePath() == "/" / tempDir.path().relative_path());
}

BOOST_AUTO_TEST_CASE(standard_json_no_input_file)
{
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({"solc", "--standard-json"});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST((result.options.input.mode == InputMode::StandardJson));
	BOOST_TEST(result.options.input.addStdin);
	BOOST_TEST(result.options.input.paths.empty());
	BOOST_TEST(result.reader.sourceCodes().empty());
	BOOST_TEST(result.reader.allowedDirectories().empty());
}

BOOST_AUTO_TEST_CASE(standard_json_dash)
{
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({"solc", "--standard-json", "-"});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST((result.options.input.mode == InputMode::StandardJson));
	BOOST_TEST(result.options.input.addStdin);
	BOOST_TEST(result.reader.sourceCodes().empty());
	BOOST_TEST(result.reader.allowedDirectories().empty());
}

BOOST_AUTO_TEST_CASE(standard_json_one_input_file)
{
	TemporaryDirectory tempDir("file-reader-test-");
	createEmptyFilesWithParentDirs({tempDir.path() / "input.json"});

	vector<string> commandLine = {"solc", "--standard-json", (tempDir.path() / "input.json").string()};
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST((result.options.input.mode == InputMode::StandardJson));
	BOOST_TEST(!result.options.input.addStdin);
	BOOST_TEST(result.options.input.paths == PathSet{tempDir.path() / "input.json"});
	BOOST_TEST(result.reader.allowedDirectories().empty());
}

BOOST_AUTO_TEST_CASE(standard_json_two_input_files)
{
	string expectedMessage =
		"Too many input files for --standard-json.\n"
		"Please either specify a single file name or provide its content on standard input.\n";

	vector<string> commandLine = {"solc", "--standard-json", "input1.json", "input2.json"};
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
	BOOST_TEST(!result.success);
	BOOST_TEST(result.stderrContent == expectedMessage);
}

BOOST_AUTO_TEST_CASE(standard_json_one_input_file_and_stdin)
{
	string expectedMessage =
		"Too many input files for --standard-json.\n"
		"Please either specify a single file name or provide its content on standard input.\n";

	vector<string> commandLine = {"solc", "--standard-json", "input1.json", "-"};
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
	BOOST_TEST(!result.success);
	BOOST_TEST(result.stderrContent == expectedMessage);
}

BOOST_AUTO_TEST_CASE(standard_json_ignore_missing)
{
	TemporaryDirectory tempDir("file-reader-test-");

	// This option is pretty much useless Standard JSON mode.
	string expectedMessage =
		"\"" + (tempDir.path() / "input.json").string() + "\" is not found. Skipping.\n"
		"All specified input files either do not exist or are not regular files.\n";

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({
		"solc",
		"--standard-json",
		(tempDir.path() / "input.json").string(),
		"--ignore-missing",
	});
	BOOST_TEST(!result.success);
	BOOST_TEST(result.stderrContent == expectedMessage);
}

BOOST_AUTO_TEST_CASE(standard_json_remapping)
{
	string expectedMessage =
		"Import remappings are not accepted on the command line in Standard JSON mode.\n"
		"Please put them under 'settings.remappings' in the JSON input.\n";

	vector<string> commandLine = {"solc", "--standard-json", "a=b"};
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
	BOOST_TEST(!result.success);
	BOOST_TEST(result.stderrContent == expectedMessage);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_no_base_path)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent.path());
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther.path());

	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");

	vector<string> commandLine = {
		"solc",
		"contract1.sol",                                   // Relative path
		"c/d/contract2.sol",                               // Relative path with subdirectories
		currentDirNoSymlinks.string() + "/contract3.sol",  // Absolute path inside working dir
		otherDirNoSymlinks.string() + "/contract4.sol",    // Absolute path outside of working dir
	};

	CommandLineOptions expectedOptions = defaultCommandLineOptions();
	expectedOptions.input.paths = {
		"contract1.sol",
		"c/d/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		otherDirNoSymlinks / "contract4.sol",
	};

	map<string, string> expectedSources = {
		{"contract1.sol", ""},
		{"c/d/contract2.sol", ""},
		{"contract3.sol", ""},
		{expectedOtherDir.generic_string() + "/contract4.sol", ""},
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
		currentDirNoSymlinks / "c/d",
		currentDirNoSymlinks,
		otherDirNoSymlinks,
	};

	createEmptyFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST((result.options == expectedOptions));
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == "");
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_base_path_same_as_work_dir)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent.path());
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther.path());

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");

	vector<string> commandLine = {
		"solc",
		"--base-path=" + currentDirNoSymlinks.string(),
		"contract1.sol",                                   // Relative path
		"c/d/contract2.sol",                               // Relative path with subdirectories
		currentDirNoSymlinks.string() + "/contract3.sol",  // Absolute path inside working dir
		otherDirNoSymlinks.string() + "/contract4.sol",    // Absolute path outside of working dir
	};

	CommandLineOptions expectedOptions = defaultCommandLineOptions();
	expectedOptions.input.paths = {
		"contract1.sol",
		"c/d/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		otherDirNoSymlinks / "contract4.sol",
	};
	expectedOptions.input.basePath = currentDirNoSymlinks;

	map<string, string> expectedSources = {
		{"contract1.sol", ""},
		{"c/d/contract2.sol", ""},
		{"contract3.sol", ""},
		{expectedOtherDir.generic_string() + "/contract4.sol", ""},
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
		currentDirNoSymlinks / "c/d",
		currentDirNoSymlinks,
		otherDirNoSymlinks,
	};

	createEmptyFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST((result.options == expectedOptions));
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_base_path_different_from_work_dir)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryDirectory tempDirBase("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");
	soltestAssert(tempDirBase.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent.path());
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther.path());
	boost::filesystem::path baseDirNoSymlinks = boost::filesystem::canonical(tempDirBase.path());

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	boost::filesystem::path expectedCurrentDir = "/" / currentDirNoSymlinks.relative_path();
	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	boost::filesystem::path expectedBaseDir = "/" / baseDirNoSymlinks.relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");
	soltestAssert(expectedCurrentDir.is_absolute() || expectedCurrentDir.root_path() == "/", "");
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");
	soltestAssert(expectedBaseDir.is_absolute() || expectedBaseDir.root_path() == "/", "");

	vector<string> commandLine = {
		"solc",
		"--base-path=" + baseDirNoSymlinks.string(),
		"contract1.sol",                                   // Relative path
		"c/d/contract2.sol",                               // Relative path with subdirectories
		currentDirNoSymlinks.string() + "/contract3.sol",  // Absolute path inside working dir
		otherDirNoSymlinks.string() + "/contract4.sol",    // Absolute path outside of working dir
		baseDirNoSymlinks.string() + "/contract5.sol",     // Absolute path inside base path
	};

	CommandLineOptions expectedOptions = defaultCommandLineOptions();
	expectedOptions.input.paths = {
		"contract1.sol",
		"c/d/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		otherDirNoSymlinks / "contract4.sol",
		baseDirNoSymlinks / "contract5.sol",
	};
	expectedOptions.input.basePath = baseDirNoSymlinks;

	map<string, string> expectedSources = {
		{expectedWorkDir.generic_string() + "/contract1.sol", ""},
		{expectedWorkDir.generic_string() + "/c/d/contract2.sol", ""},
		{expectedCurrentDir.generic_string() + "/contract3.sol", ""},
		{expectedOtherDir.generic_string() + "/contract4.sol", ""},
		{"contract5.sol", ""},
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
		currentDirNoSymlinks / "c/d",
		currentDirNoSymlinks,
		otherDirNoSymlinks,
		baseDirNoSymlinks,
	};

	createEmptyFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST((result.options == expectedOptions));
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedBaseDir);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_relative_base_path)
{
	TemporaryDirectory tempDirCurrent("file-reader-test-");
	TemporaryDirectory tempDirOther("file-reader-test-");
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent.path());
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent.path());
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther.path());

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");

	vector<string> commandLine = {
		"solc",
		"--base-path=base",
		"contract1.sol",                                       // Relative path outside of base path
		"base/contract2.sol",                                  // Relative path inside base path
		currentDirNoSymlinks.string() + "/contract3.sol",      // Absolute path inside working dir
		currentDirNoSymlinks.string() + "/base/contract4.sol", // Absolute path inside base path
		otherDirNoSymlinks.string() + "/contract5.sol",        // Absolute path outside of working dir
		otherDirNoSymlinks.string() + "/base/contract6.sol",   // Absolute path outside of working dir
	};

	CommandLineOptions expectedOptions = defaultCommandLineOptions();
	expectedOptions.input.paths = {
		"contract1.sol",
		"base/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		currentDirNoSymlinks / "base/contract4.sol",
		otherDirNoSymlinks / "contract5.sol",
		otherDirNoSymlinks / "base/contract6.sol",
	};
	expectedOptions.input.basePath = "base";

	map<string, string> expectedSources = {
		{expectedWorkDir.generic_string() + "/contract1.sol", ""},
		{"contract2.sol", ""},
		{expectedWorkDir.generic_string() + "/contract3.sol", ""},
		{"contract4.sol", ""},
		{expectedOtherDir.generic_string() + "/contract5.sol", ""},
		{expectedOtherDir.generic_string() + "/base/contract6.sol", ""},
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
		currentDirNoSymlinks / "base",
		currentDirNoSymlinks,
		otherDirNoSymlinks,
		otherDirNoSymlinks / "base",
	};

	createEmptyFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST((result.options == expectedOptions));
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "base");
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_normalization_and_weird_names)
{
	TemporaryDirectory tempDir("file-reader-test-");
	boost::filesystem::create_directories(tempDir.path() / "x/y/z");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "x/y/z");
	soltestAssert(tempDir.path().is_absolute(), "");

	string uncPath = "//" + tempDir.path().relative_path().generic_string();
	soltestAssert(uncPath[0] == '/' && uncPath[1] == '/', "");
	soltestAssert(uncPath[2] != '/', "");

	boost::filesystem::path tempDirNoSymlinks = boost::filesystem::canonical(tempDir.path());

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	vector<string> commandLine = {
		"solc",

#if !defined(_WIN32)
		// URLs. We interpret them as local paths.
		// Note that : is not allowed in file names on Windows.
		"file://c/d/contract1.sol",
		"file:///c/d/contract2.sol",
		"https://example.com/contract3.sol",
#endif

		// Redundant slashes
		"a/b//contract4.sol",
		"a/b///contract5.sol",
		"a/b////contract6.sol",

		// Dot segments
		"./a/b/contract7.sol",
		"././a/b/contract8.sol",
		"a/./b/contract9.sol",
		"a/././b/contract10.sol",

		// Dot dot segments
		"../a/b/contract11.sol",
		"../../a/b/contract12.sol",
		"a/../b/contract13.sol",
		"a/b/../../contract14.sol",
		tempDirNoSymlinks.string() + "/x/y/z/a/../b/contract15.sol",
		tempDirNoSymlinks.string() + "/x/y/z/a/b/../../contract16.sol",

		// Dot dot segments going beyond filesystem root
		"/../" + tempDir.path().relative_path().generic_string() + "/contract17.sol",
		"/../../" + tempDir.path().relative_path().generic_string() + "/contract18.sol",

#if !defined(_WIN32)
		// Name conflict with source unit name of stdin.
		// Note that < and > are not allowed in file names on Windows.
		"<stdin>",

		// UNC paths on UNIX just resolve into normal paths. On Windows this would be an network
		// share (and an error unless the share actually exists so I can't test it here).
		uncPath + "/contract19.sol",

		// Windows paths on non-Windows systems.
		// Note that on Windows we tested them already just by using absolute paths.
		"a\\b\\contract20.sol",
		"C:\\a\\b\\contract21.sol",
#endif
	};

	CommandLineOptions expectedOptions = defaultCommandLineOptions();
	expectedOptions.input.paths = {
#if !defined(_WIN32)
		"file://c/d/contract1.sol",
		"file:///c/d/contract2.sol",
		"https://example.com/contract3.sol",
#endif

		"a/b//contract4.sol",
		"a/b///contract5.sol",
		"a/b////contract6.sol",

		"./a/b/contract7.sol",
		"././a/b/contract8.sol",
		"a/./b/contract9.sol",
		"a/././b/contract10.sol",

		"../a/b/contract11.sol",
		"../../a/b/contract12.sol",
		"a/../b/contract13.sol",
		"a/b/../../contract14.sol",
		tempDirNoSymlinks.string() + "/x/y/z/a/../b/contract15.sol",
		tempDirNoSymlinks.string() + "/x/y/z/a/b/../../contract16.sol",

		"/../" + tempDir.path().relative_path().string() + "/contract17.sol",
		"/../../" + tempDir.path().relative_path().string() + "/contract18.sol",

#if !defined(_WIN32)
		"<stdin>",

		uncPath + "/contract19.sol",

		"a\\b\\contract20.sol",
		"C:\\a\\b\\contract21.sol",
#endif
	};

	map<string, string> expectedSources = {
#if !defined(_WIN32)
		{"file:/c/d/contract1.sol", ""},
		{"file:/c/d/contract2.sol", ""},
		{"https:/example.com/contract3.sol", ""},
#endif

		{"a/b/contract4.sol", ""},
		{"a/b/contract5.sol", ""},
		{"a/b/contract6.sol", ""},

		{"a/b/contract7.sol", ""},
		{"a/b/contract8.sol", ""},
		{"a/b/contract9.sol", ""},
		{"a/b/contract10.sol", ""},

		{expectedWorkDir.parent_path().generic_string() + "/a/b/contract11.sol", ""},
		{expectedWorkDir.parent_path().parent_path().generic_string() + "/a/b/contract12.sol", ""},
		{"b/contract13.sol", ""},
		{"contract14.sol", ""},
		{"b/contract15.sol", ""},
		{"contract16.sol", ""},

		{"/" + tempDir.path().relative_path().generic_string() + "/contract17.sol", ""},
		{"/" + tempDir.path().relative_path().generic_string() + "/contract18.sol", ""},

#if !defined(_WIN32)
		{"<stdin>", ""},

		{uncPath + "/contract19.sol", ""},

		{"a\\b\\contract20.sol", ""},
		{"C:\\a\\b\\contract21.sol", ""},
#endif
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
#if !defined(_WIN32)
		tempDirNoSymlinks / "x/y/z/file:/c/d",
		tempDirNoSymlinks / "x/y/z/https:/example.com",
#endif
		tempDirNoSymlinks / "x/y/z/a/b",
		tempDirNoSymlinks / "x/y/z",
		tempDirNoSymlinks / "x/y/z/b",
		tempDirNoSymlinks / "x/y/a/b",
		tempDirNoSymlinks / "x/a/b",
		tempDirNoSymlinks,
#if !defined(_WIN32)
		boost::filesystem::canonical(uncPath),
#endif
	};

	createEmptyFilesWithParentDirs(expectedOptions.input.paths);

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST((result.options == expectedOptions));
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedOptions.input.basePath);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_symlinks)
{
	TemporaryDirectory tempDir("file-reader-test-");
	createEmptyFilesWithParentDirs({tempDir.path() / "x/y/z/contract.sol"});
	boost::filesystem::create_directories(tempDir.path() / "r");
	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "r");

	if (
#if !defined(_WIN32)
		!createSymlinkIfSupportedByFilesystem("../x/y", tempDir.path() / "r/sym", true) ||
#else
		// NOTE: On Windows / works as a separator in a symlink target only if the target is absolute
		!createSymlinkIfSupportedByFilesystem("..\\x\\y", tempDir.path() / "r/sym", true) ||
#endif
		!createSymlinkIfSupportedByFilesystem("contract.sol", tempDir.path() / "x/y/z/contract_symlink.sol", false)
	)
		return;

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	vector<string> commandLine = {
		"solc",

		"--base-path=../r/sym/z/",
		"sym/z/contract.sol",            // File accessed directly + same dir symlink as base path
		"../x/y/z/contract.sol",         // File accessed directly + different dir symlink than base path
		"sym/z/contract_symlink.sol",    // File accessed via symlink + same dir symlink as base path
		"../x/y/z/contract_symlink.sol", // File accessed via symlink + different dir symlink than base path
	};

	CommandLineOptions expectedOptions = defaultCommandLineOptions();
	expectedOptions.input.paths = {
		"sym/z/contract.sol",
		"../x/y/z/contract.sol",
		"sym/z/contract_symlink.sol",
		"../x/y/z/contract_symlink.sol",
	};
	expectedOptions.input.basePath = "../r/sym/z/";

	map<string, string> expectedSources = {
		{"contract.sol", ""},
		{(expectedWorkDir.parent_path() / "x/y/z/contract.sol").generic_string(), ""},
		{"contract_symlink.sol", ""},
		{(expectedWorkDir.parent_path() / "x/y/z/contract_symlink.sol").generic_string(), ""},
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
		boost::filesystem::canonical(tempDir.path()) / "x/y/z",
	};

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST((result.options == expectedOptions));
	BOOST_TEST(result.reader.sourceCodes() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "sym/z/");
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
