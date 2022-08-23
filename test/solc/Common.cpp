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

#include <test/solc/Common.h>

#include <solc/CommandLineInterface.h>

#include <sstream>

using namespace std;
using namespace solidity::frontend;

vector<char const*> test::makeArgv(vector<string> const& _commandLine)
{
	size_t argc = _commandLine.size();
	vector<char const*> argv(_commandLine.size() + 1);

	// C++ standard mandates argv[argc] to be NULL
	argv[argc] = nullptr;

	for (size_t i = 0; i < argc; ++i)
		argv[i] = _commandLine[i].c_str();

	return argv;
}

test::OptionsReaderAndMessages test::parseCommandLineAndReadInputFiles(
	vector<string> const& _commandLine,
	string const& _standardInputContent
)
{
	vector<char const*> argv = makeArgv(_commandLine);
	stringstream sin(_standardInputContent), sout, serr;
	CommandLineInterface cli(sin, sout, serr);
	bool success = cli.parseArguments(static_cast<int>(_commandLine.size()), argv.data());
	cli.readInputFiles();

	return {
		success,
		cli.options(),
		cli.fileReader(),
		cli.standardJsonInput(),
		sout.str(),
		stripPreReleaseWarning(serr.str()),
	};
}

test::OptionsReaderAndMessages test::runCLI(
	vector<string> const& _commandLine,
	string const& _standardInputContent
)
{
	vector<char const*> argv = makeArgv(_commandLine);
	stringstream sin(_standardInputContent), sout, serr;
	CommandLineInterface cli(sin, sout, serr);
	bool success = cli.run(static_cast<int>(_commandLine.size()), argv.data());

	return {
		success,
		cli.options(),
		cli.fileReader(),
		cli.standardJsonInput(),
		sout.str(),
		stripPreReleaseWarning(serr.str()),
	};
}

string test::stripPreReleaseWarning(string const& _stderrContent)
{
	static regex const preReleaseWarningRegex{
		R"(Warning( \(3805\))?: This is a pre-release compiler version, please do not use it in production\.\n)"
		R"((\n)?)"
	};
	static regex const noOutputRegex{
		R"(Compiler run successful, no output requested\.\n)"
	};

	string output = regex_replace(_stderrContent, preReleaseWarningRegex, "");
	return regex_replace(std::move(output), noOutputRegex, "");
}
