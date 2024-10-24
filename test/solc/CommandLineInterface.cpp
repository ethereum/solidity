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
#include <solc/Exceptions.h>

#include <test/solc/Common.h>

#include <test/Common.h>
#include <test/libsolidity/util/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>
#include <liblangutil/SemVerHandler.h>
#include <test/FilesystemUtils.h>

#include <libsolutil/JSON.h>
#include <libsolutil/TemporaryDirectory.h>

#include <boost/algorithm/string.hpp>

#include <range/v3/view/transform.hpp>

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

using namespace solidity::frontend;
using namespace solidity::test;
using namespace solidity::util;
using namespace solidity::langutil;

using PathSet = std::set<boost::filesystem::path>;

#define TEST_CASE_NAME (boost::unit_test::framework::current_test_case().p_name)

namespace
{

std::ostream& operator<<(std::ostream& _out, std::vector<ImportRemapper::Remapping> const& _remappings)
{
	static auto remappingToString = [](auto const& _remapping)
	{
		return _remapping.context + ":" + _remapping.prefix + "=" + _remapping.target;
	};

	_out << "[" << joinHumanReadable(_remappings | ranges::views::transform(remappingToString)) << "]";
	return _out;
}

std::ostream& operator<<(std::ostream& _out, std::map<std::string, std::string> const& _map)
{
	_out << "{" << std::endl;
	for (auto const& [key, value]: _map)
		_out << "" << key << ": " << value << "," << std::endl;
	_out << "}";

	return _out;
}

std::ostream& operator<<(std::ostream& _out, PathSet const& _paths)
{
	static auto pathString = [](auto const& _path) { return _path.string(); };

	_out << "{" << joinHumanReadable(_paths | ranges::views::transform(pathString)) << "}";
	return _out;
}

} // namespace

namespace boost::test_tools::tt_detail
{

// Boost won't find the << operator unless we put it in the std namespace which is illegal.
// The recommended solution is to overload print_log_value<> struct and make it use our operator.

template<>
struct print_log_value<std::vector<ImportRemapper::Remapping>>
{
	void operator()(std::ostream& _out, std::vector<ImportRemapper::Remapping> const& _value) { ::operator<<(_out, _value); }
};

template<>
struct print_log_value<std::map<std::string, std::string>>
{
	void operator()(std::ostream& _out, std::map<std::string, std::string> const& _value) { ::operator<<(_out, _value); }
};

template<>
struct print_log_value<PathSet>
{
	void operator()(std::ostream& _out, PathSet const& _value) { ::operator<<(_out, _value); }
};

} // namespace boost::test_tools::tt_detail

namespace solidity::frontend::test
{

BOOST_AUTO_TEST_SUITE(CommandLineInterfaceTest)

BOOST_AUTO_TEST_CASE(help)
{
	OptionsReaderAndMessages result = runCLI({"solc", "--help"}, "");

	BOOST_TEST(result.success);
	BOOST_TEST(boost::starts_with(result.stdoutContent, "solc, the Solidity commandline compiler."));
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.options.input.mode == InputMode::Help);
}

BOOST_AUTO_TEST_CASE(license)
{
	OptionsReaderAndMessages result = runCLI({"solc", "--license"}, "");

	BOOST_TEST(result.success);
	BOOST_TEST(boost::starts_with(result.stdoutContent, "Most of the code is licensed under GPLv3"));
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.options.input.mode == InputMode::License);
}

BOOST_AUTO_TEST_CASE(version)
{
	OptionsReaderAndMessages result = runCLI({"solc", "--version"}, "");

	BOOST_TEST(result.success);
	BOOST_TEST(boost::ends_with(result.stdoutContent, "Version: " + solidity::frontend::VersionString + "\n"));
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.options.input.mode == InputMode::Version);
}

BOOST_AUTO_TEST_CASE(multiple_input_modes)
{
	std::array inputModeOptions {
		"--help",
		"--license",
		"--version",
		"--standard-json",
		"--link",
		"--assemble",
		"--strict-assembly",
		"--import-ast",
		"--import-asm-json",
	};
	std::string expectedMessage =
		"The following options are mutually exclusive: "
		"--help, --license, --version, --standard-json, --link, --assemble, --strict-assembly, --import-ast, --lsp, --import-asm-json. "
		"Select at most one.";

	for (auto const& mode1: inputModeOptions)
		for (auto const& mode2: inputModeOptions)
			if (mode1 != mode2)
				BOOST_CHECK_EXCEPTION(
					parseCommandLineAndReadInputFiles({"solc", mode1, mode2}),
					CommandLineValidationError,
					[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
				);
}

BOOST_AUTO_TEST_CASE(no_import_callback_allowed_paths)
{
	std::array<std::string, 2> options = {
		"--no-import-callback",
		"--allow-paths"
	};

	std::string expectedMessage =
		"The following options are mutually exclusive: "
		"--no-import-callback, --allow-paths. "
		"Select at most one.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({"solc", options[0], options[1], "."}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(cli_input)
{
	TemporaryDirectory tempDir1(TEST_CASE_NAME);
	TemporaryDirectory tempDir2(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir1.path() / "input1.sol"});
	createFilesWithParentDirs({tempDir2.path() / "input2.sol"});

	boost::filesystem::path expectedRootPath = FileReader::normalizeCLIRootPathForVFS(tempDir1);
	boost::filesystem::path expectedDir1 = expectedRootPath / tempDir1.path().relative_path();
	boost::filesystem::path expectedDir2 = expectedRootPath / tempDir2.path().relative_path();
	soltestAssert(expectedDir1.is_absolute() || expectedDir1.root_path() == "/", "");
	soltestAssert(expectedDir2.is_absolute() || expectedDir2.root_path() == "/", "");

	std::vector<ImportRemapper::Remapping> expectedRemappings = {
		{"", "a", "b/c/d"},
		{"a", "b", "c/d/e/"},
	};
	std::map<std::string, std::string> expectedSources = {
		{"<stdin>", ""},
		{(expectedDir1 / "input1.sol").generic_string(), ""},
		{(expectedDir2 / "input2.sol").generic_string(), ""},
	};
	PathSet expectedAllowedPaths = {
		boost::filesystem::canonical(tempDir1),
		boost::filesystem::canonical(tempDir2),
		"b/c",
		"c/d/e",
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
	BOOST_TEST(result.options.input.mode == InputMode::Compiler);
	BOOST_TEST(result.options.input.addStdin);
	BOOST_CHECK_EQUAL(result.options.input.remappings, expectedRemappings);
	BOOST_CHECK_EQUAL(result.reader.sourceUnits(), expectedSources);
	BOOST_CHECK_EQUAL(result.reader.allowedDirectories(), expectedAllowedPaths);
}

BOOST_AUTO_TEST_CASE(cli_optimizer_disabled_yul_optimization_input_whitespaces_or_empty)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir.path() / "input.sol"});
	createFilesWithParentDirs({tempDir.path() / "input.yul"});

	std::string const expectedMessage =
		"--yul-optimizations is invalid with a non-empty sequence if Yul optimizer is disabled."
		" Note that the empty optimizer sequence is properly denoted by \":\".";
	std::vector<std::vector<std::string>> const commandVariations = {
		{"solc", "--strict-assembly", "--yul-optimizations", "", (tempDir.path() / "input.yul").string()},
		{"solc", "--strict-assembly", "--yul-optimizations", "   ", (tempDir.path() / "input.yul").string()},
		{"solc", "--yul-optimizations", "", (tempDir.path() / "input.sol").string()},
		{"solc", "--yul-optimizations", "   ", (tempDir.path() / "input.sol").string()},
	};
	for (auto const& command: commandVariations)
		BOOST_CHECK_EXCEPTION(
			parseCommandLineAndReadInputFiles(command),
			CommandLineValidationError,
			[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
		);
}

BOOST_AUTO_TEST_CASE(cli_ignore_missing_some_files_exist)
{
	TemporaryDirectory tempDir1(TEST_CASE_NAME);
	TemporaryDirectory tempDir2(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir1.path() / "input1.sol"});

	boost::filesystem::path expectedRootPath = FileReader::normalizeCLIRootPathForVFS(tempDir1);
	boost::filesystem::path expectedDir1 = expectedRootPath / tempDir1.path().relative_path();
	soltestAssert(expectedDir1.is_absolute() || expectedDir1.root_path() == "/", "");

	// NOTE: Allowed paths should not be added for skipped files.
	std::map<std::string, std::string> expectedSources = {{(expectedDir1 / "input1.sol").generic_string(), ""}};
	PathSet expectedAllowedPaths = {boost::filesystem::canonical(tempDir1)};

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({
		"solc",
		(tempDir1.path() / "input1.sol").string(),
		(tempDir2.path() / "input2.sol").string(),
		"--ignore-missing",
		"--no-color",
	});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "Info: \"" + (tempDir2.path() / "input2.sol").string() + "\" is not found. Skipping.\n");
	BOOST_TEST(result.options.input.mode == InputMode::Compiler);
	BOOST_TEST(!result.options.input.addStdin);
	BOOST_CHECK_EQUAL(result.reader.sourceUnits(), expectedSources);
	BOOST_CHECK_EQUAL(result.reader.allowedDirectories(), expectedAllowedPaths);
}

BOOST_AUTO_TEST_CASE(cli_ignore_missing_no_files_exist)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);

	std::string expectedMessage =
		"Info: \"" + (tempDir.path() / "input1.sol").string() + "\" is not found. Skipping.\n"
		"Info: \"" + (tempDir.path() / "input2.sol").string() + "\" is not found. Skipping.\n"
		"Error: All specified input files either do not exist or are not regular files.\n";

	OptionsReaderAndMessages result = runCLI({
		"solc",
		(tempDir.path() / "input1.sol").string(),
		(tempDir.path() / "input2.sol").string(),
		"--ignore-missing",
		"--no-color",
	});
	BOOST_TEST(!result.success);
	BOOST_TEST(result.stderrContent == expectedMessage);
}

BOOST_AUTO_TEST_CASE(cli_not_a_file)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);

	std::string expectedMessage = "\"" + tempDir.path().string() + "\" is not a valid file.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({"solc", tempDir.path().string()}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(standard_json_base_path)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir.path().root_path());

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({
		"solc",
		"--standard-json",
		"--base-path=" + tempDir.path().string(),
	});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.options.input.mode == InputMode::StandardJson);
	BOOST_TEST(result.options.input.addStdin);
	BOOST_TEST(result.options.input.paths.empty());
	BOOST_TEST(result.reader.sourceUnits().empty());
	BOOST_TEST(result.reader.allowedDirectories().empty());
	BOOST_TEST(result.reader.basePath() == "/" / tempDir.path().relative_path());
}

BOOST_AUTO_TEST_CASE(standard_json_no_input_file)
{
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({"solc", "--standard-json"});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.options.input.mode == InputMode::StandardJson);
	BOOST_TEST(result.options.input.addStdin);
	BOOST_TEST(result.options.input.paths.empty());
	BOOST_TEST(result.reader.sourceUnits().empty());
	BOOST_TEST(result.reader.allowedDirectories().empty());
}

BOOST_AUTO_TEST_CASE(standard_json_dash)
{
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles({"solc", "--standard-json", "-"});
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.options.input.mode == InputMode::StandardJson);
	BOOST_TEST(result.options.input.addStdin);
	BOOST_TEST(result.reader.sourceUnits().empty());
	BOOST_TEST(result.reader.allowedDirectories().empty());
}

BOOST_AUTO_TEST_CASE(standard_json_one_input_file)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir.path() / "input.json"});

	std::vector<std::string> commandLine = {"solc", "--standard-json", (tempDir.path() / "input.json").string()};
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
	BOOST_TEST(result.success);
	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.options.input.mode == InputMode::StandardJson);
	BOOST_TEST(!result.options.input.addStdin);
	BOOST_TEST(result.options.input.paths == PathSet{tempDir.path() / "input.json"});
	BOOST_TEST(result.reader.allowedDirectories().empty());
}

BOOST_AUTO_TEST_CASE(standard_json_two_input_files)
{
	std::string expectedMessage =
		"Too many input files for --standard-json.\n"
		"Please either specify a single file name or provide its content on standard input.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({"solc", "--standard-json", "input1.json", "input2.json"}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(standard_json_one_input_file_and_stdin)
{
	std::string expectedMessage =
		"Too many input files for --standard-json.\n"
		"Please either specify a single file name or provide its content on standard input.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({"solc", "--standard-json", "input1.json", "-"}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(standard_json_ignore_missing)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);

	// This option is pretty much useless Standard JSON mode.
	std::string expectedMessage =
		"All specified input files either do not exist or are not regular files.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({
			"solc",
			"--standard-json",
			(tempDir.path() / "input.json").string(),
			"--ignore-missing",
		}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(standard_json_remapping)
{
	std::string expectedMessage =
		"Import remappings are not accepted on the command line in Standard JSON mode.\n"
		"Please put them under 'settings.remappings' in the JSON input.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({"solc", "--standard-json", "a=b"}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_no_base_path)
{
	TemporaryDirectory tempDirCurrent(TEST_CASE_NAME);
	TemporaryDirectory tempDirOther(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent);
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent);
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther);

	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");

	std::vector<std::string> commandLine = {
		"solc",
		"contract1.sol",                                   // Relative path
		"c/d/contract2.sol",                               // Relative path with subdirectories
		currentDirNoSymlinks.string() + "/contract3.sol",  // Absolute path inside working dir
		otherDirNoSymlinks.string() + "/contract4.sol",    // Absolute path outside of working dir
	};

	CommandLineOptions expectedOptions;
	expectedOptions.input.paths = {
		"contract1.sol",
		"c/d/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		otherDirNoSymlinks / "contract4.sol",
	};
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
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

	createFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == "");
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_base_path_same_as_work_dir)
{
	TemporaryDirectory tempDirCurrent(TEST_CASE_NAME);
	TemporaryDirectory tempDirOther(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent);
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent);
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther);

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");

	std::vector<std::string> commandLine = {
		"solc",
		"--base-path=" + currentDirNoSymlinks.string(),
		"contract1.sol",                                   // Relative path
		"c/d/contract2.sol",                               // Relative path with subdirectories
		currentDirNoSymlinks.string() + "/contract3.sol",  // Absolute path inside working dir
		otherDirNoSymlinks.string() + "/contract4.sol",    // Absolute path outside of working dir
	};

	CommandLineOptions expectedOptions;
	expectedOptions.input.paths = {
		"contract1.sol",
		"c/d/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		otherDirNoSymlinks / "contract4.sol",
	};
	expectedOptions.input.basePath = currentDirNoSymlinks;
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
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

	createFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_base_path_different_from_work_dir)
{
	TemporaryDirectory tempDirCurrent(TEST_CASE_NAME);
	TemporaryDirectory tempDirOther(TEST_CASE_NAME);
	TemporaryDirectory tempDirBase(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent);
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");
	soltestAssert(tempDirBase.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent);
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther);
	boost::filesystem::path baseDirNoSymlinks = boost::filesystem::canonical(tempDirBase);

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	boost::filesystem::path expectedCurrentDir = "/" / currentDirNoSymlinks.relative_path();
	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	boost::filesystem::path expectedBaseDir = "/" / baseDirNoSymlinks.relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");
	soltestAssert(expectedCurrentDir.is_absolute() || expectedCurrentDir.root_path() == "/", "");
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");
	soltestAssert(expectedBaseDir.is_absolute() || expectedBaseDir.root_path() == "/", "");

	std::vector<std::string> commandLine = {
		"solc",
		"--base-path=" + baseDirNoSymlinks.string(),
		"contract1.sol",                                   // Relative path
		"c/d/contract2.sol",                               // Relative path with subdirectories
		currentDirNoSymlinks.string() + "/contract3.sol",  // Absolute path inside working dir
		otherDirNoSymlinks.string() + "/contract4.sol",    // Absolute path outside of working dir
		baseDirNoSymlinks.string() + "/contract5.sol",     // Absolute path inside base path
	};

	CommandLineOptions expectedOptions;
	expectedOptions.input.paths = {
		"contract1.sol",
		"c/d/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		otherDirNoSymlinks / "contract4.sol",
		baseDirNoSymlinks / "contract5.sol",
	};
	expectedOptions.input.basePath = baseDirNoSymlinks;
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
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

	createFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedBaseDir);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_relative_base_path)
{
	TemporaryDirectory tempDirCurrent(TEST_CASE_NAME);
	TemporaryDirectory tempDirOther(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDirCurrent);
	soltestAssert(tempDirCurrent.path().is_absolute(), "");
	soltestAssert(tempDirOther.path().is_absolute(), "");

	// NOTE: On macOS the path usually contains symlinks which prevents base path from being stripped.
	// Use canonical() to resolve symnlinks and get consistent results on all platforms.
	boost::filesystem::path currentDirNoSymlinks = boost::filesystem::canonical(tempDirCurrent);
	boost::filesystem::path otherDirNoSymlinks = boost::filesystem::canonical(tempDirOther);

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	boost::filesystem::path expectedOtherDir = "/" / otherDirNoSymlinks.relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");
	soltestAssert(expectedOtherDir.is_absolute() || expectedOtherDir.root_path() == "/", "");

	std::vector<std::string> commandLine = {
		"solc",
		"--base-path=base",
		"contract1.sol",                                       // Relative path outside of base path
		"base/contract2.sol",                                  // Relative path inside base path
		currentDirNoSymlinks.string() + "/contract3.sol",      // Absolute path inside working dir
		currentDirNoSymlinks.string() + "/base/contract4.sol", // Absolute path inside base path
		otherDirNoSymlinks.string() + "/contract5.sol",        // Absolute path outside of working dir
		otherDirNoSymlinks.string() + "/base/contract6.sol",   // Absolute path outside of working dir
	};

	CommandLineOptions expectedOptions;
	expectedOptions.input.paths = {
		"contract1.sol",
		"base/contract2.sol",
		currentDirNoSymlinks / "contract3.sol",
		currentDirNoSymlinks / "base/contract4.sol",
		otherDirNoSymlinks / "contract5.sol",
		otherDirNoSymlinks / "base/contract6.sol",
	};
	expectedOptions.input.basePath = "base";
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
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

	createFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "base");
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_normalization_and_weird_names)
{
	TemporaryDirectory tempDir({"x/y/z"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "x/y/z");
	soltestAssert(tempDir.path().is_absolute(), "");

	std::string uncPath = "//" + tempDir.path().relative_path().generic_string();
	soltestAssert(FileReader::isUNCPath(uncPath), "");

	boost::filesystem::path tempDirNoSymlinks = boost::filesystem::canonical(tempDir);

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	std::vector<std::string> commandLine = {
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

	CommandLineOptions expectedOptions;
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
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
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

	createFilesWithParentDirs(expectedOptions.input.paths);

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedOptions.input.basePath);
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_symlinks)
{
	TemporaryDirectory tempDir({"r/"}, TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir.path() / "x/y/z/contract.sol"});
	TemporaryWorkingDirectory tempWorkDir(tempDir.path() / "r");

	if (
		!createSymlinkIfSupportedByFilesystem("../x/y", tempDir.path() / "r/sym", true) ||
		!createSymlinkIfSupportedByFilesystem("contract.sol", tempDir.path() / "x/y/z/contract_symlink.sol", false)
	)
		return;

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();
	soltestAssert(expectedWorkDir.is_absolute() || expectedWorkDir.root_path() == "/", "");

	std::vector<std::string> commandLine = {
		"solc",

		"--base-path=../r/sym/z/",
		"sym/z/contract.sol",            // File accessed directly + same dir symlink as base path
		"../x/y/z/contract.sol",         // File accessed directly + different dir symlink than base path
		"sym/z/contract_symlink.sol",    // File accessed via symlink + same dir symlink as base path
		"../x/y/z/contract_symlink.sol", // File accessed via symlink + different dir symlink than base path
	};

	CommandLineOptions expectedOptions;
	expectedOptions.input.paths = {
		"sym/z/contract.sol",
		"../x/y/z/contract.sol",
		"sym/z/contract_symlink.sol",
		"../x/y/z/contract_symlink.sol",
	};
	expectedOptions.input.basePath = "../r/sym/z/";
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
		{"contract.sol", ""},
		{(expectedWorkDir.parent_path() / "x/y/z/contract.sol").generic_string(), ""},
		{"contract_symlink.sol", ""},
		{(expectedWorkDir.parent_path() / "x/y/z/contract_symlink.sol").generic_string(), ""},
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
		boost::filesystem::canonical(tempDir) / "x/y/z",
	};

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "sym/z/");
}

BOOST_AUTO_TEST_CASE(cli_paths_to_source_unit_names_base_path_and_stdin)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);
	boost::filesystem::create_directories(tempDir.path() / "base");

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::current_path().relative_path();

	std::vector<std::string> commandLine = {"solc", "--base-path=base", "-"};

	CommandLineOptions expectedOptions;
	expectedOptions.input.addStdin = true;
	expectedOptions.input.basePath = "base";
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
		{"<stdin>", ""},
	};
	FileReader::FileSystemPathSet expectedAllowedDirectories = {};

	createFilesWithParentDirs(expectedOptions.input.paths);
	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);

	BOOST_TEST(result.stderrContent == "");
	BOOST_TEST(result.stdoutContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "base");
}

BOOST_AUTO_TEST_CASE(cli_include_paths)
{
	TemporaryDirectory tempDir({"base/", "include/", "lib/nested/"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);

	std::string const mainContractSource = withPreamble(
		"import \"contract.sol\";\n"
		"import \"contract_via_callback.sol\";\n"
		"import \"include.sol\";\n"
		"import \"include_via_callback.sol\";\n"
		"import \"nested.sol\";\n"
		"import \"nested_via_callback.sol\";\n"
		"import \"lib.sol\";\n"
		"import \"lib_via_callback.sol\";\n"
	);

	std::string const onlyPreamble = withPreamble("");
	createFilesWithParentDirs(
		{
			tempDir.path() / "base/contract.sol",
			tempDir.path() / "base/contract_via_callback.sol",
			tempDir.path() / "include/include.sol",
			tempDir.path() / "include/include_via_callback.sol",
			tempDir.path() / "lib/nested/nested.sol",
			tempDir.path() / "lib/nested/nested_via_callback.sol",
			tempDir.path() / "lib/lib.sol",
			tempDir.path() / "lib/lib_via_callback.sol",
		},
		onlyPreamble
	);
	createFilesWithParentDirs({tempDir.path() / "base/main.sol"}, mainContractSource);

	boost::filesystem::path canonicalWorkDir = boost::filesystem::canonical(tempDir);
	boost::filesystem::path expectedWorkDir = "/" / canonicalWorkDir.relative_path();

	std::vector<std::string> commandLine = {
		"solc",
		"--no-color",
		"--base-path=base/",
		"--include-path=include/",
		"--include-path=lib/nested",
		"--include-path=lib/",
		"base/main.sol",
		"base/contract.sol",
		"include/include.sol",
		"lib/nested/nested.sol",
		"lib/lib.sol",
	};

	CommandLineOptions expectedOptions;
	expectedOptions.input.paths = {
		"base/main.sol",
		"base/contract.sol",
		"include/include.sol",
		"lib/nested/nested.sol",
		"lib/lib.sol",
	};
	expectedOptions.input.basePath = "base/";
	expectedOptions.input.includePaths = {
		"include/",
		"lib/nested",
		"lib/",
	};
	expectedOptions.formatting.coloredOutput = false;
	expectedOptions.modelChecker.initialize = true;

	std::map<std::string, std::string> expectedSources = {
		{"main.sol", mainContractSource},
		{"contract.sol", onlyPreamble},
		{"contract_via_callback.sol", onlyPreamble},
		{"include.sol", onlyPreamble},
		{"include_via_callback.sol", onlyPreamble},
		{"nested.sol", onlyPreamble},
		{"nested_via_callback.sol", onlyPreamble},
		{"lib.sol", onlyPreamble},
		{"lib_via_callback.sol", onlyPreamble},
	};

	std::vector<boost::filesystem::path> expectedIncludePaths = {
		expectedWorkDir / "include/",
		expectedWorkDir / "lib/nested",
		expectedWorkDir / "lib/",
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {
		canonicalWorkDir / "base",
		canonicalWorkDir / "include",
		canonicalWorkDir / "lib/nested",
		canonicalWorkDir / "lib",
	};

	std::string const expectedStdoutContent = "Compiler run successful. No contracts to compile.\n";
	OptionsReaderAndMessages result = runCLI(commandLine, "");

	BOOST_TEST(result.stderrContent == "");
	if (SemVerVersion{std::string(VersionString)}.isPrerelease())
		BOOST_TEST(result.stdoutContent == "");
	else
		BOOST_TEST(result.stdoutContent == expectedStdoutContent);
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.includePaths() == expectedIncludePaths);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "base/");
}

BOOST_AUTO_TEST_CASE(cli_no_contracts_to_compile)
{
	std::string const contractSource = R"(
		// SPDX-License-Identifier: GPL-3.0
		pragma solidity >=0.0;
		enum Status { test }
	)";

	std::string const expectedStdoutContent = "Compiler run successful. No contracts to compile.\n";
	OptionsReaderAndMessages result = runCLI({"solc", "-"}, contractSource);

	if (SemVerVersion{std::string(VersionString)}.isPrerelease())
		BOOST_TEST(result.stdoutContent == "");
	else
		BOOST_TEST(result.stdoutContent == expectedStdoutContent);
	BOOST_REQUIRE(result.success);
}

BOOST_AUTO_TEST_CASE(cli_no_output)
{
	std::string const contractSource = R"(
		// SPDX-License-Identifier: GPL-3.0
		pragma solidity >=0.0;
		abstract contract A {
			function B() public virtual returns(uint);
		})";

	std::string const expectedStdoutContent = "Compiler run successful. No output generated.\n";
	OptionsReaderAndMessages result = runCLI({"solc", "-"}, contractSource);

	if (SemVerVersion{std::string(VersionString)}.isPrerelease())
		BOOST_TEST(result.stdoutContent == "");
	else
		BOOST_TEST(result.stdoutContent == expectedStdoutContent);
	BOOST_REQUIRE(result.success);
}

BOOST_AUTO_TEST_CASE(standard_json_include_paths)
{
	TemporaryDirectory tempDir({"base/", "include/", "lib/nested/"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);

	std::string const mainContractSource = withPreamble(
		"import 'contract_via_callback.sol';\n"
		"import 'include_via_callback.sol';\n"
		"import 'nested_via_callback.sol';\n"
		"import 'lib_via_callback.sol';\n"
	);

	std::string const standardJsonInput = R"(
		{
			"language": "Solidity",
			"sources": {
				"main.sol": {"content": ")" + mainContractSource + R"("}
			}
		}
	)";

	std::string const onlyPreamble = withPreamble("");
	createFilesWithParentDirs(
		{
			tempDir.path() / "base/contract_via_callback.sol",
			tempDir.path() / "include/include_via_callback.sol",
			tempDir.path() / "lib/nested/nested_via_callback.sol",
			tempDir.path() / "lib/lib_via_callback.sol",
		},
		onlyPreamble
	);

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::canonical(tempDir).relative_path();

	std::vector<std::string> commandLine = {
		"solc",
		"--base-path=base/",
		"--include-path=include/",
		"--include-path=lib/nested",
		"--include-path=lib/",
		"--standard-json",
	};

	CommandLineOptions expectedOptions;
	expectedOptions.input.mode = InputMode::StandardJson;
	expectedOptions.input.paths = {};
	expectedOptions.input.addStdin = true;
	expectedOptions.input.basePath = "base/";
	expectedOptions.input.includePaths = {
		"include/",
		"lib/nested",
		"lib/",
	};
	expectedOptions.modelChecker.initialize = false;

	// NOTE: Source code from Standard JSON does not end up in FileReader. This is not a problem
	// because FileReader is only used once to initialize the compiler stack and after that
	// its sources are irrelevant (even though the callback still stores everything it loads).
	std::map<std::string, std::string> expectedSources = {
		{"contract_via_callback.sol", onlyPreamble},
		{"include_via_callback.sol", onlyPreamble},
		{"nested_via_callback.sol", onlyPreamble},
		{"lib_via_callback.sol", onlyPreamble},
	};

	std::vector<boost::filesystem::path> expectedIncludePaths = {
		expectedWorkDir / "include/",
		expectedWorkDir / "lib/nested",
		expectedWorkDir / "lib/",
	};

	FileReader::FileSystemPathSet expectedAllowedDirectories = {};

	OptionsReaderAndMessages result = runCLI(commandLine, standardJsonInput);

	Json parsedStdout;
	std::string jsonParsingErrors;
	BOOST_TEST(util::jsonParseStrict(result.stdoutContent, parsedStdout, &jsonParsingErrors));
	BOOST_TEST(jsonParsingErrors == "");
	for (Json const& errorDict: parsedStdout["errors"])
		// The error list might contain pre-release compiler warning
		BOOST_TEST(errorDict["severity"] != "error");
	// we might be able to use ranges again, but the nlohmann::json support is not yet fully there.
	// (parsedStdout["sources"].items() | ranges::views::keys | ranges::to<std::set>)
	std::set<std::string> sources;
	for (auto const& [key, _]: parsedStdout["sources"].items())
		sources.insert(key);
	BOOST_TEST(
		sources ==
		(expectedSources | ranges::views::keys | ranges::to<std::set>) + std::set<std::string>{"main.sol"}
	);

	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.options == expectedOptions);
	BOOST_TEST(result.reader.sourceUnits() == expectedSources);
	BOOST_TEST(result.reader.includePaths() == expectedIncludePaths);
	BOOST_TEST(result.reader.allowedDirectories() == expectedAllowedDirectories);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "base/");
}

BOOST_AUTO_TEST_CASE(cli_include_paths_empty_path)
{
	TemporaryDirectory tempDir({"base/", "include/"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);
	createFilesWithParentDirs({tempDir.path() / "base/main.sol"});

	std::string expectedMessage = "Empty values are not allowed in --include-path.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({
			"solc",
			"--base-path=base/",
			"--include-path", "include/",
			"--include-path", "",
			"base/main.sol",
		}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(cli_include_paths_without_base_path)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);
	createFilesWithParentDirs({tempDir.path() / "contract.sol"});

	std::string expectedMessage = "--include-path option requires a non-empty base path.";

	BOOST_CHECK_EXCEPTION(
		parseCommandLineAndReadInputFiles({"solc", "--include-path", "include/", "contract.sol"}),
		CommandLineValidationError,
		[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
	);
}

BOOST_AUTO_TEST_CASE(cli_include_paths_should_detect_source_unit_name_collisions)
{
	TemporaryDirectory tempDir({"dir1/", "dir2/", "dir3/"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);
	createFilesWithParentDirs({
		"dir1/contract1.sol",
		"dir1/contract2.sol",
		"dir2/contract1.sol",
		"dir2/contract2.sol",
	});

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::canonical(tempDir).relative_path();

	std::string expectedMessage =
		"Source unit name collision detected. "
		"The specified values of base path and/or include paths would result in multiple "
		"input files being assigned the same source unit name:\n"
		"contract1.sol matches: "
		"\"" + (expectedWorkDir / "dir1/contract1.sol").generic_string() + "\", "
		"\"" + (expectedWorkDir / "dir2/contract1.sol").generic_string() + "\"\n"
		"contract2.sol matches: "
		"\"" + (expectedWorkDir / "dir1/contract2.sol").generic_string() + "\", "
		"\"" + (expectedWorkDir / "dir2/contract2.sol").generic_string() + "\"\n";

	{
		// import "contract1.sol" and import "contract2.sol" would be ambiguous:
		BOOST_CHECK_EXCEPTION(
			parseCommandLineAndReadInputFiles({
				"solc",
				"--base-path=dir1/",
				"--include-path=dir2/",
				"dir1/contract1.sol",
				"dir2/contract1.sol",
				"dir1/contract2.sol",
				"dir2/contract2.sol",
			}),
			CommandLineValidationError,
			[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
		);
	}

	{
		// import "contract1.sol" and import "contract2.sol" would be ambiguous:
		BOOST_CHECK_EXCEPTION(
			parseCommandLineAndReadInputFiles({
				"solc",
				"--base-path=dir3/",
				"--include-path=dir1/",
				"--include-path=dir2/",
				"dir1/contract1.sol",
				"dir2/contract1.sol",
				"dir1/contract2.sol",
				"dir2/contract2.sol",
			}),
			CommandLineValidationError,
			[&](auto const& _exception) { BOOST_TEST(_exception.what() == expectedMessage); return true; }
		);
	}

	{
		// No conflict if files with the same name exist but only one is given to the compiler.
		std::vector<std::string> commandLine = {
			"solc",
			"--base-path=dir3/",
			"--include-path=dir1/",
			"--include-path=dir2/",
			"dir1/contract1.sol",
			"dir1/contract2.sol",
		};
		OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
		BOOST_TEST(result.stderrContent == "");
		BOOST_REQUIRE(result.success);
	}

	{
		// The same file specified multiple times is not a conflict.
		std::vector<std::string> commandLine = {
			"solc",
			"--base-path=dir3/",
			"--include-path=dir1/",
			"--include-path=dir2/",
			"dir1/contract1.sol",
			"dir1/contract1.sol",
			"./dir1/contract1.sol",
		};
		OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
		BOOST_TEST(result.stderrContent == "");
		BOOST_REQUIRE(result.success);
	}
}

BOOST_AUTO_TEST_CASE(cli_include_paths_should_allow_duplicate_paths)
{
	TemporaryDirectory tempDir({"dir1/", "dir2/"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);
	createFilesWithParentDirs({"dir1/contract.sol"});

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::canonical(tempDir).relative_path();
	boost::filesystem::path expectedTempDir = "/" / tempDir.path().relative_path();

	std::vector<std::string> commandLine = {
		"solc",
		"--base-path=dir1/",
		"--include-path", "dir1",
		"--include-path", "dir1",
		"--include-path", "dir1/",
		"--include-path", "dir1/",
		"--include-path", "./dir1/",
		"--include-path", "dir2/../dir1/",
		"--include-path", (tempDir.path() / "dir1/").string(),
		"--include-path", (expectedWorkDir / "dir1/").string(),
		"--include-path", "dir1/",
		"dir1/contract.sol",
	};

	// Duplicates do not affect the result but are not removed from the include path list.
	std::vector<boost::filesystem::path> expectedIncludePaths = {
		expectedWorkDir / "dir1",
		expectedWorkDir / "dir1",
		expectedWorkDir / "dir1/",
		expectedWorkDir / "dir1/",
		expectedWorkDir / "dir1/",
		expectedWorkDir / "dir1/",
		// NOTE: On macOS expectedTempDir usually contains a symlink and therefore for us it's
		// different from expectedWorkDir.
		expectedTempDir / "dir1/",
		expectedWorkDir / "dir1/",
		expectedWorkDir / "dir1/",
	};

	OptionsReaderAndMessages result = parseCommandLineAndReadInputFiles(commandLine);
	BOOST_TEST(result.stderrContent == "");
	BOOST_REQUIRE(result.success);
	BOOST_TEST(result.reader.includePaths() == expectedIncludePaths);
	BOOST_TEST(result.reader.basePath() == expectedWorkDir / "dir1/");
}

BOOST_AUTO_TEST_CASE(cli_include_paths_ambiguous_import)
{
	TemporaryDirectory tempDir({"base/", "include/"}, TEST_CASE_NAME);
	TemporaryWorkingDirectory tempWorkDir(tempDir);

	// Ambiguous: both base/contract.sol and include/contract.sol match the import.
	std::string const mainContractSource = withPreamble("import \"contract.sol\";");

	createFilesWithParentDirs({"base/contract.sol", "include/contract.sol"}, withPreamble(""));

	boost::filesystem::path expectedWorkDir = "/" / boost::filesystem::canonical(tempDir).relative_path();

	std::vector<std::string> commandLine = {
		"solc",
		"--no-color",
		"--base-path=base/",
		"--include-path=include/",
		"-",
	};

	std::string expectedMessage =
		"Error: Source \"contract.sol\" not found: Ambiguous import. "
		"Multiple matching files found inside base path and/or include paths: \"" +
		(expectedWorkDir / "base/contract.sol").generic_string() + "\", \"" +
		(expectedWorkDir / "include/contract.sol").generic_string() + "\".\n"
		" --> <stdin>:3:1:\n"
		"  |\n"
		"3 | import \"contract.sol\";\n"
		"  | ^^^^^^^^^^^^^^^^^^^^^^\n\n";

	OptionsReaderAndMessages result = runCLI(commandLine, mainContractSource);
	BOOST_TEST(result.stderrContent == expectedMessage);
	BOOST_REQUIRE(!result.success);
}

BOOST_AUTO_TEST_CASE(cli_ethdebug_no_ethdebug_in_help)
{
	OptionsReaderAndMessages result = runCLI({"solc", "--help"});
	BOOST_REQUIRE(result.stdoutContent.find("ethdebug") == std::string::npos);
	// just in case
	BOOST_REQUIRE(result.stderrContent.find("ethdebug") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(cli_ethdebug_incompatible_outputs)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir.path() / "input.sol"});
	static std::vector<std::pair<std::vector<std::string>, std::string>> tests{
		{
			{"solc", "--ethdebug",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug",  "--ir-ast-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug",  "--ir-optimized-ast-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--ethdebug", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: Option --ethdebug is not supported with --import-asm-json.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--ethdebug-runtime",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug-runtime",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug-runtime",  "--ir-ast-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug-runtime",  "--ir-optimized-ast-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--ethdebug-runtime", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: Option --ethdebug-runtime is not supported with --import-asm-json.\n"
		},
		{
			{"solc", "--via-ir", "--ethdebug-runtime",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --ethdebug / --ethdebug-runtime output can only be used with --ir, --ir-optimized.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --debug-info ethdebug can only be used with --ir, --ir-optimized and/or --ethdebug / --ethdebug-runtime.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug",  "--asm-json", tempDir.path().string() + "/input.sol"},
			"Error: --debug-info ethdebug can only be used with --ir, --ir-optimized and/or --ethdebug / --ethdebug-runtime.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug",  "--ir-ast-json", tempDir.path().string() + "/input.sol"},
			"Error: --debug-info ethdebug can only be used with --ir, --ir-optimized and/or --ethdebug / --ethdebug-runtime.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug",  "--ir-optimized-ast-json", tempDir.path().string() + "/input.sol"},
			"Error: --debug-info ethdebug can only be used with --ir, --ir-optimized and/or --ethdebug / --ethdebug-runtime.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: Option --debug-info is not supported with --import-asm-json.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: Option --debug-info is not supported with --import-asm-json.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug", "--asm-json", tempDir.path().string() + "/input.json"},
			"Error: --debug-info ethdebug can only be used with --ir, --ir-optimized and/or --ethdebug / --ethdebug-runtime.\n"
		}
	};
	for (auto const& test: tests)
	{
		OptionsReaderAndMessages result = runCLI(test.first, "");
		BOOST_REQUIRE(!result.success);
		BOOST_CHECK_EQUAL(result.stderrContent, test.second);
	}
}

BOOST_AUTO_TEST_CASE(cli_ethdebug_incompatible_input_modes)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir.path() / "input.json"});
	static std::vector<std::pair<std::vector<std::string>, std::string>> tests{
		{
			{"solc", "--ethdebug", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: Option --ethdebug is not supported with --import-asm-json.\n"
		},
		{
			{"solc", "--ethdebug", "--via-ir", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: The following options are not supported in the current input mode: --via-ir\n"
		},
		{
			{"solc", "--ethdebug", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: Option --ethdebug is not supported with --import-asm-json.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug", "--import-asm-json", tempDir.path().string() + "/input.json"},
			"Error: Option --debug-info is not supported with --import-asm-json.\n"
		},
		{
			{"solc", "--ethdebug", "--import-ast", tempDir.path().string() + "/input.json"},
			"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"
		},
		{
			{"solc", "--ethdebug", "--via-ir", "--import-ast", tempDir.path().string() + "/input.json"},
			"Error: Invalid input mode for --debug-info ethdebug / --ethdebug / --ethdebug-runtime.\n"
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ir", "--import-ast", tempDir.path().string() + "/input.json"},
			"Error: Invalid input mode for --debug-info ethdebug / --ethdebug / --ethdebug-runtime.\n"
		}
	};
	for (auto const& test: tests)
	{
		OptionsReaderAndMessages result = runCLI(test.first, "");
		BOOST_REQUIRE(!result.success);
		BOOST_CHECK_EQUAL(result.stderrContent, test.second);
	}
}

BOOST_AUTO_TEST_CASE(cli_ethdebug_debug_info_ethdebug)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir.path() / "input.sol"},  "pragma solidity >=0.0; contract C { function f() public pure {} }");
	createFilesWithParentDirs({tempDir.path() / "input.yul"}, "{}");
	static std::vector<std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::string>>> tests{
		{
			{"solc", "--debug-info", "ethdebug", tempDir.path().string() + "/input.sol"},
			{"Error: --debug-info ethdebug can only be used with --ir, --ir-optimized and/or --ethdebug / --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ir", tempDir.path().string() + "/input.sol"},
			{},
			{"/// ethdebug: enabled"},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ir-optimized", tempDir.path().string() + "/input.sol"},
			{},
			{"/// ethdebug: enabled"},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ethdebug", tempDir.path().string() + "/input.sol"},
			{"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ethdebug", "--via-ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program):"},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data of the runtime part (ethdebug/format/program):"},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--strict-assembly", tempDir.path().string() + "/input.yul"},
			{},
			{"/// ethdebug: enabled", "Pretty printed source", "Binary representation", "Text representation"},
		},
		{
			{"solc",  "--ethdebug", "--strict-assembly", tempDir.path().string() + "/input.yul"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program):"},
		},
		{
			{"solc",  "--ethdebug-runtime", "--strict-assembly", tempDir.path().string() + "/input.yul"},
			{"Error: The following outputs are not supported in assembler mode: --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc",  "--ethdebug", "--ethdebug-runtime", "--strict-assembly", tempDir.path().string() + "/input.yul"},
			{"Error: The following outputs are not supported in assembler mode: --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ethdebug", tempDir.path().string() + "/input.sol"},
			{"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ethdebug-runtime", tempDir.path().string() + "/input.sol"},
			{"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ethdebug", "--ethdebug-runtime", tempDir.path().string() + "/input.sol"},
			{"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "ethdebug", "--ethdebug", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program):", "Debug Data of the runtime part (ethdebug/format/program):"},
		},
		{
			{"solc", "--debug-info", "location", "--ethdebug", "--via-ir", tempDir.path().string() + "/input.sol"},
			{"Error: --debug-info must contain ethdebug, when compiling with --ethdebug / --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "location", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{"Error: --debug-info must contain ethdebug, when compiling with --ethdebug / --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "location", "--ethdebug", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{"Error: --debug-info must contain ethdebug, when compiling with --ethdebug / --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "all", "--ethdebug", "--via-ir", tempDir.path().string() + "/input.sol"},
			{"Error: --debug-info must contain ethdebug, when compiling with --ethdebug / --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "all", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{"Error: --debug-info must contain ethdebug, when compiling with --ethdebug / --ethdebug-runtime.\n"},
			{},
		},
		{
			{"solc", "--debug-info", "all", "--ethdebug", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{"Error: --debug-info must contain ethdebug, when compiling with --ethdebug / --ethdebug-runtime.\n"},
			{},
		},
	};
	for (auto const& test: tests)
	{
		OptionsReaderAndMessages result{runCLI(std::get<0>(test), "")};
		BOOST_REQUIRE(!std::get<1>(test).empty() ? !result.success : result.success);
		for (auto const& error : std::get<1>(test))
			BOOST_REQUIRE(result.stderrContent == error);
		for (auto const& output : std::get<2>(test))
			BOOST_REQUIRE(result.stdoutContent.find(output) != std::string::npos);
	}
}

BOOST_AUTO_TEST_CASE(cli_ethdebug_ethdebug_output)
{
	TemporaryDirectory tempDir(TEST_CASE_NAME);
	createFilesWithParentDirs({tempDir.path() / "input.sol"}, "pragma solidity >=0.0; contract C { function f() public pure {} }");
	static std::vector<std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<std::string>>> tests{
		{
			{"solc", "--ethdebug", tempDir.path().string() + "/input.sol"},
			{"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"},
			{},
		},
		{
			{"solc", "--ethdebug", "--ethdebug-runtime", tempDir.path().string() + "/input.sol"},
			{"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"},
			{},
		},
		{
			{"solc", "--ethdebug-runtime", tempDir.path().string() + "/input.sol"},
			{"Error: --ethdebug / --ethdebug-runtime output can only be selected, if --via-ir was specified.\n"},
			{},
		},
		{
			{"solc", "--ethdebug", "--via-ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)"},
		},
		{
			{"solc", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data of the runtime part (ethdebug/format/program)"},
		},
		{
			{"solc", "--ethdebug", "--ethdebug-runtime", "--via-ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)", "Debug Data of the runtime part (ethdebug/format/program)"},
		},
		{
			{"solc", "--ethdebug", "--via-ir", "--ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug-runtime", "--via-ir", "--ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data of the runtime part (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug", "--ethdebug-runtime", "--via-ir", "--ir", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)", "Debug Data of the runtime part (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug", "--via-ir", "--ir-optimized", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug-runtime", "--via-ir", "--ir-optimized", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data of the runtime part (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug", "--ethdebug-runtime", "--via-ir", "--ir-optimized", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)", "Debug Data of the runtime part (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug", "--via-ir", "--ir-optimized", "--optimize", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug-runtime", "--via-ir", "--ir-optimized", "--optimize", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data of the runtime part (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
		{
			{"solc", "--ethdebug", "--ethdebug-runtime", "--via-ir", "--ir-optimized", "--optimize", tempDir.path().string() + "/input.sol"},
			{},
			{"======= Debug Data (ethdebug/format/object) =======", "Debug Data (ethdebug/format/program)", "Debug Data of the runtime part (ethdebug/format/program)", "/// ethdebug: enabled"},
		},
	};
	for (auto const& test: tests)
	{
		OptionsReaderAndMessages result{runCLI(std::get<0>(test), "")};
		BOOST_REQUIRE(!std::get<1>(test).empty() ? !result.success : result.success);
		for (auto const& error : std::get<1>(test))
			BOOST_REQUIRE(result.stderrContent == error);
		for (auto const& output : std::get<2>(test))
			BOOST_REQUIRE(result.stdoutContent.find(output) != std::string::npos);
	}
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
