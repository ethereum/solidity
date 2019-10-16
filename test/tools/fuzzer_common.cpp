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

#include <test/tools/fuzzer_common.h>

#include <libdevcore/JSON.h>
#include <libevmasm/Assembly.h>
#include <libevmasm/ConstantOptimiser.h>
#include <libsolc/libsolc.h>

#include <sstream>

using namespace std;
using namespace dev;
using namespace dev::eth;

static vector<string> s_evmVersions = {
	"homestead",
	"tangerineWhistle",
	"spuriousDragon",
	"byzantium",
	"constantinople",
	"petersburg",
	"istanbul"
};

void FuzzerUtil::runCompiler(string const& _input, bool _quiet)
{
	if (!_quiet)
		cout << "Input JSON: " << _input << endl;
	string outputString(solidity_compile(_input.c_str(), nullptr));
	if (!_quiet)
		cout << "Output JSON: " << outputString << endl;

	// This should be safe given the above copies the output.
	solidity_free();

	Json::Value output;
	if (!jsonParseStrict(outputString, output))
	{
		string msg{"Compiler produced invalid JSON output."};
		cout << msg << endl;
		throw std::runtime_error(std::move(msg));
	}
	if (output.isMember("errors"))
		for (auto const& error: output["errors"])
		{
			string invalid = findAnyOf(error["type"].asString(), vector<string>{
					"Exception",
					"InternalCompilerError"
			});
			if (!invalid.empty())
			{
				string msg = "Invalid error: \"" + error["type"].asString() + "\"";
				cout << msg << endl;
				throw std::runtime_error(std::move(msg));
			}
		}
}

void FuzzerUtil::testCompiler(string const& _input, bool _optimize, bool _quiet)
{
	if (!_quiet)
		cout << "Testing compiler " << (_optimize ? "with" : "without") << " optimizer." << endl;

	Json::Value config = Json::objectValue;
	config["language"] = "Solidity";
	config["sources"] = Json::objectValue;
	config["sources"][""] = Json::objectValue;
	config["sources"][""]["content"] = _input;
	config["settings"] = Json::objectValue;
	config["settings"]["optimizer"] = Json::objectValue;
	config["settings"]["optimizer"]["enabled"] = _optimize;
	config["settings"]["optimizer"]["runs"] = 200;
	config["settings"]["evmVersion"] = s_evmVersions[_input.size() % s_evmVersions.size()];

	// Enable all SourceUnit-level outputs.
	config["settings"]["outputSelection"]["*"][""][0] = "*";
	// Enable all Contract-level outputs.
	config["settings"]["outputSelection"]["*"]["*"][0] = "*";

	runCompiler(jsonCompactPrint(config), _quiet);
}

void FuzzerUtil::testConstantOptimizer(string const& _input, bool _quiet)
{
	if (!_quiet)
		cout << "Testing constant optimizer" << endl;
	vector<u256> numbers;
	stringstream sin(_input);

	while (!sin.eof())
	{
		h256 data;
		sin.read(reinterpret_cast<char *>(data.data()), 32);
		numbers.push_back(u256(data));
	}
	if (!_quiet)
		cout << "Got " << numbers.size() << " inputs:" << endl;

	Assembly assembly;
	for (u256 const& n: numbers)
	{
		if (!_quiet)
			cout << n << endl;
		assembly.append(n);
	}
	for (bool isCreation: {false, true})
		for (unsigned runs: {1, 2, 3, 20, 40, 100, 200, 400, 1000})
		{
			// Make a copy here so that each time we start with the original state.
			Assembly tmp = assembly;
			ConstantOptimisationMethod::optimiseConstants(
					isCreation,
					runs,
					langutil::EVMVersion{},
					tmp
			);
		}
}

void FuzzerUtil::testStandardCompiler(string const& _input, bool _quiet)
{
	if (!_quiet)
		cout << "Testing compiler via JSON interface." << endl;

	runCompiler(_input, _quiet);
}
