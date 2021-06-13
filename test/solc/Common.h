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

OptionsReaderAndMessages parseCommandLineAndReadInputFiles(
	std::vector<std::string> const& _commandLine,
	std::string const& _standardInputContent = "",
	bool _processInput = false
);

} // namespace solidity::frontend::test
