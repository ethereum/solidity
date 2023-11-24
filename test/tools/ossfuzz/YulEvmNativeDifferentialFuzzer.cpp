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

#include <test/tools/ossfuzz/yulFuzzerCommon.h>

#include <test/EVMHost.h>

#include <test/tools/ossfuzz/YulEvmoneInterface.h>

#include <libyul/Exceptions.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/CompilabilityChecker.h>

#include <libyul/AsmPrinter.h>
#include <libyul/YulStack.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/EVMVersion.h>

#include <libsolutil/Whiskers.h>

#include <evmone/evmone.h>

#include <fstream>

using namespace solidity;
using namespace solidity::test;
using namespace solidity::test::fuzzer;
using namespace solidity::yul;
using namespace solidity::yul::test;
using namespace solidity::yul::test::yul_fuzzer;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace std;
using namespace solidity::frontend;

namespace
{
/// @returns true if there are no recursive functions, false otherwise.
bool recursiveFunctionExists(Dialect const& _dialect, yul::Object& _object)
{
	auto recursiveFunctions = CallGraphGenerator::callGraph(*_object.code).recursiveFunctions();
	for(auto&& [function, variables]: CompilabilityChecker{
			_dialect,
			_object,
			true
		}.unreachableVariables
	)
		if(recursiveFunctions.count(function))
			return true;
	return false;
}
}

static evmc::VM evmone = evmc::VM{evmc_create_evmone()};

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	string optimizerSequence;
	string yulSubObject;
	// Fuzz sequence
	if (const char* crashFileEnv = std::getenv("CRASH_FILE"))
	{
		std::ifstream ifs(crashFileEnv);
		yulSubObject = std::string(std::istreambuf_iterator<char>{ifs}, {});
		auto fuzzedSequence = string(reinterpret_cast<char const*>(_data), _size);
		auto cleanedSequence = OptimiserSettings::removeInvalidCharacters(fuzzedSequence);
		optimizerSequence = OptimiserSettings::createValidSequence(cleanedSequence);
	}
	// Fuzz program
	else if (const char* optSeqEnv = std::getenv("OPT_SEQ"))
	{
		optimizerSequence = optSeqEnv;
		yulSubObject = string(reinterpret_cast<char const*>(_data), _size);
	}
	// Fuzz program and sequence
	else
	{
		yulSubObject = string(reinterpret_cast<char const*>(_data), _size);
		optimizerSequence = OptimiserSettings::randomYulOptimiserSequence(_size);
	}

	// Fuzzer also fuzzes the EVM version field.
	langutil::EVMVersion version;
	EVMHost hostContext(version, evmone);
	hostContext.reset();

	YulStringRepository::reset();

	// Package test case into a sub-object
	Whiskers yulObjectFormat(R"(
		object "main" {
			code {
				codecopy(0, dataoffset("deployed"), datasize("deployed"))
				return(0, datasize("deployed"))
			}
			object "deployed" {
				code {
					<fuzzerInput>
				}
			}
		}
			)");
	string yul_source = yulObjectFormat("fuzzerInput", yulSubObject).render();

	cout << "Optimizer sequence: " << optimizerSequence << endl;
	auto settings = solidity::frontend::OptimiserSettings::fuzz(optimizerSequence);
	YulStack yulStack(
		version,
		nullopt,
		YulStack::Language::StrictAssembly,
		settings,
		DebugInfoSelection::All()
	);
	if (!yulStack.parseAndAnalyze("source", yulSubObject))
		return 0;
	ostringstream unoptimizedState;
	std::shared_ptr<yul::Object> subObject = yulStack.parserResult();
	Dialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(version);
	yulFuzzerUtil::TerminationReason termReason = yulFuzzerUtil::interpret(
		unoptimizedState,
		subObject->code,
		dialect,
		false,
		true,
		10000,
		10000,
		100
	);
	if (yulFuzzerUtil::resourceLimitsExceeded(termReason))
		return 0;

	bool recursiveFunction = recursiveFunctionExists(dialect, *yulStack.parserResult());

	yulStack.optimize();

	string optimisedSubObject = AsmPrinter{}(*yulStack.parserResult()->code);
	string optimisedProgram = Whiskers(R"(
		object "main" {
			code {
				codecopy(0, dataoffset("deployed"), datasize("deployed"))
				return(0, datasize("deployed"))
			}
			object "deployed" {
				code {
					<fuzzerInput>
				}
			}
		}
			)")
		("fuzzerInput", optimisedSubObject)
		.render();
	cout << optimisedSubObject << endl;
	bytes optimisedByteCode;
	settings.optimizeStackAllocation = true;
	try
	{
		optimisedByteCode = YulAssembler{version, nullopt, settings, optimisedProgram}.assemble();
	}
	catch (yul::StackTooDeepError const&)
	{
		// If there are recursive functions, stack too deep errors are expected
		// even post stack evasion optimisation and hence ignored. Otherwise,
		// they are rethrown for further investigation.
		if (recursiveFunction)
			return 0;
		throw;
	}
	// InvalidDeposit
	catch (Exception const&)
	{
		if (recursiveFunction)
			return 0;
		throw;
	}

	// Reset host before running optimised code.
	hostContext.reset();
	evmc::Result deployResultOpt = YulEvmoneUtility{}.deployCode(optimisedByteCode, hostContext);
	solAssert(
		deployResultOpt.status_code == EVMC_SUCCESS,
		"Evmone: Optimized contract creation failed"
	);
	auto callMessageOpt = YulEvmoneUtility{}.callMessage(deployResultOpt.create_address);
	evmc::Result callResultOpt = hostContext.call(callMessageOpt);
	// Bail out if we ran out of gas.
	if (callResultOpt.status_code == EVMC_OUT_OF_GAS)
		return 0;
	bool noRevertInSource = yulSubObject.find("revert") == string::npos;
	bool noInvalidInSource = yulSubObject.find("invalid") == string::npos;
	if (noRevertInSource)
		solAssert(
			callResultOpt.status_code != EVMC_REVERT,
			"SolidityEvmoneInterface: EVM One reverted"
		);
	if (noInvalidInSource)
		solAssert(
			callResultOpt.status_code != EVMC_INVALID_INSTRUCTION,
			"Invalid instruction."
		);
	solAssert(
		(callResultOpt.status_code == EVMC_SUCCESS ||
		 (!noRevertInSource && callResultOpt.status_code == EVMC_REVERT) ||
		 (!noInvalidInSource && callResultOpt.status_code == EVMC_INVALID_INSTRUCTION)),
		"Optimised call failed."
	);
	ostringstream optimizedState;
	optimizedState << EVMHostPrinter{hostContext, deployResultOpt.create_address}.storageOnly();

	if (unoptimizedState.str() != optimizedState.str())
	{
		cout << unoptimizedState.str() << endl;
		cout << optimizedState.str() << endl;
	}
	solAssert(
		unoptimizedState.str() == optimizedState.str(),
		"State of unoptimised and optimised stack saver code do not match."
	);
	return 0;
}
