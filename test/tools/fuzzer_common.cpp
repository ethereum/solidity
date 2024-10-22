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

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::util;

static std::vector<EVMVersion> s_evmVersions = EVMVersion::allVersions();

void FuzzerUtil::testCompilerJsonInterface(std::string const& _input, bool _optimize, bool _quiet)
{
	if (!_quiet)
		std::cout << "Testing compiler " << (_optimize ? "with" : "without") << " optimizer." << std::endl;

	Json config;
	config["language"] = "Solidity";
	config["sources"] = Json::object();
	config["sources"][""] = Json::object();
	config["sources"][""]["content"] = _input;
	config["settings"] = Json::object();
	config["settings"]["optimizer"] = Json::object();
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
	static auto constexpr smtPragma = "pragma experimental SMTChecker;";
	for (auto &sourceUnit: _input)
		if (sourceUnit.second.find(smtPragma) == std::string::npos)
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
			/*bmcLoopIterations*/1,
			frontend::ModelCheckerContracts::Default(),
			/*divModWithSlacks*/true,
			frontend::ModelCheckerEngine::All(),
			frontend::ModelCheckerExtCalls{},
			frontend::ModelCheckerInvariants::All(),
			/*printQuery=*/false,
			/*showProvedSafe=*/false,
			/*showUnproved=*/false,
			/*showUnsupported=*/false,
			smtutil::SMTSolverChoice::All(),
			frontend::ModelCheckerTargets::Default(),
			/*timeout=*/1
		});
	}
	compiler.setSources(_input);
	compiler.setEVMVersion(evmVersion);
	compiler.setOptimiserSettings(optimiserSettings);
	compiler.setViaIR(_compileViaYul);
	try
	{
		compiler.compile();
	}
	catch (UnimplementedFeatureError const&)
	{
	}
	catch (StackTooDeepError const&)
	{
		if (_optimize && _compileViaYul)
			throw;
	}
}

void FuzzerUtil::runCompiler(std::string const& _input, bool _quiet)
{
	if (!_quiet)
		std::cout << "Input JSON: " << _input << std::endl;
	std::string outputString(solidity_compile(_input.c_str(), nullptr, nullptr));
	if (!_quiet)
		std::cout << "Output JSON: " << outputString << std::endl;

	// This should be safe given the above copies the output.
	solidity_reset();

	Json output;
	if (!jsonParseStrict(outputString, output))
	{
		std::string msg{"Compiler produced invalid JSON output."};
		std::cout << msg << std::endl;
		BOOST_THROW_EXCEPTION(std::runtime_error(std::move(msg)));
	}
	if (output.contains("errors"))
		for (auto const& error: output["errors"])
		{
			std::string invalid = findAnyOf(error["type"].get<std::string>(), std::vector<std::string>{
					"Exception",
					"InternalCompilerError"
			});
			if (!invalid.empty())
			{
				std::string msg = "Invalid error: \"" + error["type"].get<std::string>() + "\"";
				std::cout << msg << std::endl;
				BOOST_THROW_EXCEPTION(std::runtime_error(std::move(msg)));
			}
		}
}

void FuzzerUtil::testConstantOptimizer(std::string const& _input, bool _quiet)
{
	if (!_quiet)
		std::cout << "Testing constant optimizer" << std::endl;
	std::vector<u256> numbers;
	std::stringstream sin(_input);

	while (!sin.eof())
	{
		h256 data;
		sin.read(reinterpret_cast<char *>(data.data()), 32);
		numbers.push_back(u256(data));
	}
	if (!_quiet)
		std::cout << "Got " << numbers.size() << " inputs:" << std::endl;

	for (bool isCreation: {false, true})
	{
		Assembly assembly{langutil::EVMVersion{}, isCreation, std::nullopt, {}};
		for (u256 const& n: numbers)
		{
			if (!_quiet)
				std::cout << n << std::endl;
			assembly.append(n);
		}
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
}

void FuzzerUtil::testStandardCompiler(std::string const& _input, bool _quiet)
{
	if (!_quiet)
		std::cout << "Testing compiler via JSON interface." << std::endl;

	runCompiler(_input, _quiet);
}
