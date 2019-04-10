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
/** @file IsolTestOptions.cpp
* @date 2019
*/

#include <test/tools/IsolTestOptions.h>

#include <libdevcore/Assertions.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <regex>
#include <string>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace dev
{
namespace test
{

auto const description = R"(isoltest, tool for interactively managing test contracts.
Usage: isoltest [Options]
Interactively validates test contracts.

Allowed options)";

std::string editorPath()
{
	if (getenv("EDITOR"))
		return getenv("EDITOR");
	else if (fs::exists("/usr/bin/editor"))
		return "/usr/bin/editor";

	return std::string{};
}

IsolTestOptions::IsolTestOptions(std::string* _editor):
	CommonOptions(description)
{
	options.add_options()
		("editor", po::value<std::string>(_editor)->default_value(editorPath()), "Path to editor for opening test files.")
		("help", po::bool_switch(&showHelp), "Show this help screen.")
		("no-color", po::bool_switch(&noColor), "Don't use colors.")
		("test,t", po::value<std::string>(&test)->default_value("*/*"), "Filters which test units to include.");
}

bool IsolTestOptions::parse(int _argc, char const* const* _argv)
{
	bool const res = CommonOptions::parse(_argc, _argv);

	if (showHelp || !res)
	{
		std::cout << options << std::endl;
		return false;
	}

	return res;
}

void IsolTestOptions::validate() const
{
	std::regex filterExpression{"(((\\*+|\\w+|\\w+\\*+)\\/)+(\\*|\\w+\\**))"};
	assertThrow(
		regex_match(test, filterExpression),
		ConfigException,
		"Invalid test unit filter: " + test
	);
}

}
}
