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

	vector<ImportRemapper::Remapping> expectedRemappings = {
		{"", "a", "b/c/d"},
		{"a", "b", "c/d/e/"},
	};
	map<string, string> expectedSources = {
		{"<stdin>", "\n"},
		{(tempDir1.path() / "input1.sol").generic_string(), ""},
		{(tempDir2.path() / "input2.sol").generic_string(), ""},
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

	// NOTE: Allowed paths should not be added for skipped files.
	map<string, string> expectedSources = {{(tempDir1.path() / "input1.sol").generic_string(), ""}};
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
	BOOST_TEST(result.reader.basePath() == tempDir.path());
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

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
