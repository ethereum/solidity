// SPDX-License-Identifier: GPL-3.0
/** @file IsolTestOptions.cpp
* @date 2019
*/

#include <test/tools/IsolTestOptions.h>

#include <libsolutil/Assertions.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <regex>
#include <string>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace solidity::test
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
		("test,t", po::value<std::string>(&testFilter)->default_value("*/*"), "Filters which test units to include.");
}

bool IsolTestOptions::parse(int _argc, char const* const* _argv)
{
	bool const res = CommonOptions::parse(_argc, _argv);

	if (showHelp || !res)
	{
		std::cout << options << std::endl;
		return false;
	}
	enforceViaYul = true;

	return res;
}

void IsolTestOptions::validate() const
{
	static std::string filterString{"[a-zA-Z1-9_/*]*"};
	static std::regex filterExpression{filterString};
	assertThrow(
		regex_match(testFilter, filterExpression),
		ConfigException,
		"Invalid test unit filter - can only contain '" + filterString + ": " + testFilter
	);
}

}
