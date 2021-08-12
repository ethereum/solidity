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

#include <test/tools/fuzzer_common.h>

#include <libsolidity/interface/OptimiserSettings.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/formal/ModelCheckerSettings.h>

#include <libsolutil/JSON.h>

#include <libevmasm/Assembly.h>
#include <libevmasm/ConstantOptimiser.h>

#include <libsolc/libsolc.h>

#include <liblangutil/Exceptions.h>

#include <sstream>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::util;

static vector<EVMVersion> s_evmVersions = {
	EVMVersion::homestead(),
	EVMVersion::tangerineWhistle(),
	EVMVersion::spuriousDragon(),
	EVMVersion::byzantium(),
	EVMVersion::constantinople(),
	EVMVersion::petersburg(),
	EVMVersion::istanbul(),
	EVMVersion::berlin()
};

void FuzzerUtil::testCompilerJsonInterface(string const& _input, bool _optimize, bool _quiet)
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
	config["settings"]["optimizer"]["runs"] = static_cast<int>(OptimiserSettings{}.expectedExecutionsPerDeployment);
	config["settings"]["evmVersion"] = "berlin";

	// Enable all SourceUnit-level outputs.
	config["settings"]["outputSelection"]["*"][""][0] = "*";
	// Enable all Contract-level outputs.
	config["settings"]["outputSelection"]["*"]["*"][0] = "*";

	runCompiler(jsonCompactPrint(config), _quiet);
}

void FuzzerUtil::forceSMT(StringMap& _input)
{
	// Add SMT checker pragma if not already present in source
	static const char* smtPragma = "pragma experimental SMTChecker;";
	for (auto &sourceUnit: _input)
		if (sourceUnit.second.find(smtPragma) == string::npos)
			sourceUnit.second += smtPragma;
}

void FuzzerUtil::testCompiler(
	StringMap& _input,
	bool _optimize,
	unsigned _rand,
	bool _forceSMT,
	bool _compileViaYul
)
{
	frontend::CompilerStack compiler;
	EVMVersion evmVersion = s_evmVersions[_rand % s_evmVersions.size()];
	frontend::OptimiserSettings optimiserSettings;
	if (_optimize)
		optimiserSettings = frontend::OptimiserSettings::standard();
	else
		optimiserSettings = frontend::OptimiserSettings::minimal();
	if (_forceSMT)
	{
		forceSMT(_input);
		compiler.setModelCheckerSettings({
			frontend::ModelCheckerContracts::Default(),
			/*divModWithSlacks*/true,
			frontend::ModelCheckerEngine::All(),
			/*showUnproved=*/false,
			smtutil::SMTSolverChoice::All(),
			frontend::ModelCheckerTargets::Default(),
			/*timeout=*/1
		});
	}
	compiler.setSources(_input);
	compiler.setEVMVersion(evmVersion);
	compiler.setOptimiserSettings(optimiserSettings);
	compiler.enableIRGeneration(_compileViaYul);
	try
	{
		compiler.compile();
	}
	catch (Error const&)
	{
	}
	catch (FatalError const&)
	{
	}
	catch (UnimplementedFeatureError const&)
	{
	}
	catch (StackTooDeepError const&)
	{
	}
}

void FuzzerUtil::runCompiler(string const& _input, bool _quiet)
{
	if (!_quiet)
		cout << "Input JSON: " << _input << endl;
	string outputString(solidity_compile(_input.c_str(), nullptr, nullptr));
	if (!_quiet)
		cout << "Output JSON: " << outputString << endl;

	// This should be safe given the above copies the output.
	solidity_reset();

	Json::Value output;
	if (!jsonParseStrict(outputString, output))
	{
		string msg{"Compiler produced invalid JSON output."};
		cout << msg << endl;
		BOOST_THROW_EXCEPTION(std::runtime_error(std::move(msg)));
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
				BOOST_THROW_EXCEPTION(std::runtime_error(std::move(msg)));
			}
		}
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
		for (unsigned runs: {1u, 2u, 3u, 20u, 40u, 100u, 200u, 400u, 1000u})
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
