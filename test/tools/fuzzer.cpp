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

void testConstantOptimizer()
{
	if (!quiet)
		cout << "Testing constant optimizer" << endl;
	vector<u256> numbers;
	while (!cin.eof())
	{
		h256 data;
		cin.read(reinterpret_cast<char*>(data.data()), 32);
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

void testStandardCompiler()
{
	if (!quiet)
		cout << "Testing compiler via JSON interface." << endl;
	string input = readStandardInput();

	string outputString(compileStandard(input.c_str(), NULL));
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

void testCompiler(bool optimize)
{
	if (!quiet)
		cout << "Testing compiler " << (optimize ? "with" : "without") << " optimizer." << endl;
	string input = readStandardInput();

	string outputString(compileJSON(input.c_str(), optimize));
	Json::Value outputJson;
	if (!jsonParseStrict(outputString, outputJson))
	{
		cout << "Compiler produced invalid JSON output." << endl;
		abort();
	}
	if (outputJson.isMember("errors"))
	{
		if (!outputJson["errors"].isArray())
		{
			cout << "Output JSON has \"errors\" but it is not an array." << endl;
			abort();
		}
		for (Json::Value const& error: outputJson["errors"])
		{
			string invalid = contains(error.asString(), vector<string>{
				// StandardJSON error types
				"Exception",
				"InternalCompilerError",
				// Old-school error messages
				"Internal compiler error",
				"Exception during compilation",
				"Unknown exception during compilation",
				"Unknown exception while generating contract data output",
				"Unknown exception while generating source name output",
				"Unknown error while generating JSON"
			});
			if (!invalid.empty())
			{
				cout << "Invalid error: \"" << error.asString() << "\"" << endl;
				abort();
			}
		}
	}
	else if (!outputJson.isMember("contracts"))
	{
		cout << "Output JSON has neither \"errors\" nor \"contracts\"." << endl;
		abort();
	}
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
			"without-optimizer",
			"Run without optimizations. Cannot be used together with standard-json."
		);

	po::variables_map arguments;
	try
	{
		po::command_line_parser cmdLineParser(argc, argv);
		cmdLineParser.options(options);
		po::store(cmdLineParser.run(), arguments);
	}
	catch (po::error const& _exception)
	{
		cerr << _exception.what() << endl;
		return 1;
	}

	if (arguments.count("quiet"))
		quiet = true;

	if (arguments.count("help"))
		cout << options;
	else if (arguments.count("const-opt"))
		testConstantOptimizer();
	else if (arguments.count("standard-json"))
		testStandardCompiler();
	else
		testCompiler(!arguments.count("without-optimizer"));

	return 0;
}
