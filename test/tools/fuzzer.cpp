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
/**
 * Executable for use with AFL <http://lcamtuf.coredump.cx/afl>.
 */

#include <libdevcore/CommonIO.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/ConstantOptimiser.h>
#include <libsolc/libsolc.h>

#include <libdevcore/JSON.h>

#include <boost/program_options.hpp>

#include <string>
#include <sstream>
#include <iostream>

using namespace std;
using namespace dev;
using namespace dev::eth;
namespace po = boost::program_options;

namespace
{

bool quiet = false;

string contains(string const& _haystack, vector<string> const& _needles)
{
	for (string const& needle: _needles)
		if (_haystack.find(needle) != string::npos)
			return needle;
	return "";
}

void testConstantOptimizer(string const& input)
{
	if (!quiet)
		cout << "Testing constant optimizer" << endl;
	vector<u256> numbers;
	stringstream sin(input);

	while (!sin.eof())
	{
		h256 data;
		sin.read(reinterpret_cast<char*>(data.data()), 32);
		numbers.push_back(u256(data));
	}
	if (!quiet)
		cout << "Got " << numbers.size() << " inputs:" << endl;

	Assembly assembly;
	for (u256 const& n: numbers)
	{
		if (!quiet)
			cout << n << endl;
		assembly.append(n);
	}
	for (bool isCreation: {false, true})
	{
		for (unsigned runs: {1, 2, 3, 20, 40, 100, 200, 400, 1000})
		{
			ConstantOptimisationMethod::optimiseConstants(
				isCreation,
				runs,
				EVMVersion{},
				assembly,
				const_cast<AssemblyItems&>(assembly.items())
			);
		}
	}
}

void runCompiler(string input)
{
	string outputString(compileStandard(input.c_str(), nullptr));
	Json::Value output;
	if (!jsonParseStrict(outputString, output))
	{
		cout << "Compiler produced invalid JSON output." << endl;
		abort();
	}
	if (output.isMember("errors"))
		for (auto const& error: output["errors"])
		{
			string invalid = contains(error["type"].asString(), vector<string>{
				"Exception",
				"InternalCompilerError"
			});
			if (!invalid.empty())
			{
				cout << "Invalid error: \"" << error["type"].asString() << "\"" << endl;
				abort();
			}
		}
}

void testStandardCompiler(string const& input)
{
	if (!quiet)
		cout << "Testing compiler via JSON interface." << endl;

	runCompiler(input);
}

void testCompiler(string const& input, bool optimize)
{
	if (!quiet)
		cout << "Testing compiler " << (optimize ? "with" : "without") << " optimizer." << endl;

	Json::Value config = Json::objectValue;
	config["language"] = "Solidity";
	config["sources"] = Json::objectValue;
	config["sources"][""] = Json::objectValue;
	config["sources"][""]["content"] = input;
	config["settings"] = Json::objectValue;
	config["settings"]["optimizer"] = Json::objectValue;
	config["settings"]["optimizer"]["enabled"] = optimize;
	config["settings"]["optimizer"]["runs"] = 200;

	// Enable all SourceUnit-level outputs.
	config["settings"]["outputSelection"]["*"][""][0] = "*";
	// Enable all Contract-level outputs.
	config["settings"]["outputSelection"]["*"]["*"][0] = "*";

	runCompiler(jsonCompactPrint(config));
}

}

int main(int argc, char** argv)
{
	po::options_description options(
		R"(solfuzzer, fuzz-testing binary for use with AFL.
Usage: solfuzzer [Options] < input
Reads a single source from stdin, compiles it and signals a failure for internal errors.

Allowed options)",
		po::options_description::m_default_line_length,
		po::options_description::m_default_line_length - 23);
	options.add_options()
		("help", "Show this help screen.")
		("quiet", "Only output errors.")
		(
			"standard-json",
			"Test via the standard-json interface, i.e. "
			"input is expected to be JSON-encoded instead of "
			"plain source file."
		)
		(
			"const-opt",
			"Run the constant optimizer instead of compiling. "
			"Expects a binary string of up to 32 bytes on stdin."
		)
		(
			"input-file",
			po::value<string>(),
			"input file"
		)
		(
			"without-optimizer",
			"Run without optimizations. Cannot be used together with standard-json."
		);

	// All positional options should be interpreted as input files
	po::positional_options_description filesPositions;
	filesPositions.add("input-file", 1);

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(argc, argv);
		cmdLineParser.options(options).positional(filesPositions);
		po::store(cmdLineParser.run(), arguments);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
		return 1;
	}

	string input;
	if (arguments.count("input-file"))
		input = readFileAsString(arguments["input-file"].as<string>());
	else
		input = readStandardInput();

	if (arguments.count("quiet"))
		quiet = true;

	if (arguments.count("help"))
		cout << options;
	else if (arguments.count("const-opt"))
		testConstantOptimizer(input);
	else if (arguments.count("standard-json"))
		testStandardCompiler(input);
	else
		testCompiler(input, !arguments.count("without-optimizer"));

	return 0;
}
