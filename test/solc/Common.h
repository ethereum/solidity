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

/// Utilities shared by multiple tests for code in solc/.

#include <solc/CommandLineParser.h>

#include <boost/test/unit_test.hpp>

#include <optional>
#include <string>
#include <vector>

BOOST_TEST_DONT_PRINT_LOG_VALUE(solidity::frontend::CommandLineOptions)
BOOST_TEST_DONT_PRINT_LOG_VALUE(solidity::frontend::InputMode)

namespace solidity::frontend::test
{

struct OptionsReaderAndMessages
{
	bool success;
	CommandLineOptions options;
	FileReader reader;
	std::optional<std::string> standardJsonInput;
	std::string stdoutContent;
	std::string stderrContent;
};

std::vector<char const*> makeArgv(std::vector<std::string> const& _commandLine);

/// Runs only command-line parsing, without compilation, assembling or any other input processing.
/// Lets through any @a CommandLineErrors throw by the CLI.
/// Note: This uses the @a CommandLineInterface class and does not actually spawn a new process.
/// @param _commandLine Arguments in the form of strings that would be specified on the command-line.
///                     You must specify the program name as the first item.
/// @param _standardInputContent Content that the CLI will be able to read from its standard input.
OptionsReaderAndMessages parseCommandLineAndReadInputFiles(
	std::vector<std::string> const& _commandLine,
	std::string const& _standardInputContent = ""
);

/// Runs all stages of command-line interface processing, including error handling.
/// Never throws @a CommandLineError - validation errors are included in the returned stderr content.
/// Note: This uses the @a CommandLineInterface class and does not actually spawn a new process.
/// @param _commandLine Arguments in the form of strings that would be specified on the command-line.
///                     You must specify the program name as the first item.
/// @param _standardInputContent Content that the CLI will be able to read from its standard input.
OptionsReaderAndMessages runCLI(
	std::vector<std::string> const& _commandLine,
	std::string const& _standardInputContent = ""
);

std::string stripPreReleaseWarning(std::string const& _stderrContent);

} // namespace solidity::frontend::test
