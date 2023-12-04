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

#include <test/EVMHost.h>

#include <test/tools/ossfuzz/YulEvmoneInterface.h>
#include <test/tools/ossfuzz/yulFuzzerCommon.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/YulStack.h>

#include <libyul/backends/evm/EVMCodeTransform.h>

#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/CompilabilityChecker.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/EVMVersion.h>
#include <liblangutil/Exceptions.h>

#include <evmone/evmone.h>

#include <fstream>

using namespace solidity;
using namespace solidity::test;
using namespace solidity::test::fuzzer;
using namespace solidity::yul;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::yul::test::yul_fuzzer;
using namespace std;

static evmc::VM evmone = evmc::VM{evmc_create_evmone()};

namespace
{
/// @returns true if there are recursive functions, false otherwise.
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

// Prototype as we can't use the FuzzerInterface.h header.
extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size);

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	string optimizerSequence;
	string testProgram;
	// Fuzz sequence
	if (const char* crashFileEnv = std::getenv("CRASH_FILE"))
	{
		std::ifstream ifs(crashFileEnv);
		testProgram = std::string(std::istreambuf_iterator<char>{ifs}, {});
		auto fuzzedSequence = string(reinterpret_cast<char const*>(_data), _size);
		auto cleanedSequence = OptimiserSettings::removeInvalidCharacters(fuzzedSequence);
		optimizerSequence = OptimiserSettings::createValidSequence(cleanedSequence);
	}
	// Fuzz program
	else if (const char* optSeqEnv = std::getenv("OPT_SEQ"))
	{
		optimizerSequence = optSeqEnv;
		testProgram = string(reinterpret_cast<char const*>(_data), _size);
	}
	// Fuzz program and sequence
	else
	{
		testProgram = string(reinterpret_cast<char const*>(_data), _size);
		optimizerSequence = OptimiserSettings::randomYulOptimiserSequence(_size);
	}

	if (std::any_of(testProgram.begin(), testProgram.end(), [](char c) {
		return ((static_cast<unsigned char>(c) > 127) || !(isprint(c) || (c == '\n') || (c == '\t')));
	}))
		return 0;

	YulStringRepository::reset();

	langutil::EVMVersion version;
	OptimiserSettings settings = OptimiserSettings::fuzz(optimizerSequence);

	YulStack yulStack(
		version,
		nullopt,
		YulStack::Language::StrictAssembly,
		settings,
		DebugInfoSelection::All()
	);
	if (
		!yulStack.parseAndAnalyze("source", testProgram) ||
		!yulStack.parserResult()->code ||
		!yulStack.parserResult()->analysisInfo
	)
		return 0;

	ostringstream testrun;
	try
	{
		auto termReason = yulFuzzerUtil::interpret(
			testrun,
			yulStack.parserResult()->code,
			EVMDialect::strictAssemblyForEVMObjects(version),
			true
		);
		if (yulFuzzerUtil::resourceLimitsExceeded(termReason))
			return 0;
	}
	catch (solidity::yul::YulAssertion const&)
	{
		cout << "Ignoring failed assert while interpreting" << endl;
	}

	cout << "Optimizer sequence: " << optimizerSequence << endl;

	EVMHost hostContext(version, evmone);
	hostContext.reset();

	settings.runYulOptimiser = false;
	settings.optimizeStackAllocation = false;
	bytes unoptimisedByteCode;
	bool recursiveFunction = false;
	bool unoptimizedStackTooDeep = false;
	try
	{
		YulAssembler assembler{version, nullopt, settings, testProgram};
		unoptimisedByteCode = assembler.assemble();
		auto yulObject = assembler.object();
		try
		{
			recursiveFunction = recursiveFunctionExists(
				EVMDialect::strictAssemblyForEVMObjects(version),
				*yulObject
			);
		}
		catch (solidity::yul::YulAssertion const&)
		{
			cout << "Ignoring failed assert during check for recursive function" << endl;
		}
	}
	catch (solidity::yul::StackTooDeepError const&)
	{
		unoptimizedStackTooDeep = true;
		cout << "Unoptimized stack too deep" << endl;
	}

	ostringstream unoptimizedState;
	bool noRevertInSource = true;
	bool noInvalidInSource = true;
	if (!unoptimizedStackTooDeep)
	{
		evmc::Result deployResult = YulEvmoneUtility{}.deployCode(unoptimisedByteCode, hostContext);
		if (deployResult.status_code != EVMC_SUCCESS)
			return 0;
		auto callMessage = YulEvmoneUtility{}.callMessage(deployResult.create_address);
		evmc::Result callResult = hostContext.call(callMessage);
		// If the fuzzer synthesized input does not contain the revert opcode which
		// we lazily check by string find, the EVM call should not revert.
		noRevertInSource = testProgram.find("revert") == string::npos;
		noInvalidInSource = testProgram.find("invalid") == string::npos;
		if (noInvalidInSource)
			solAssert(
				callResult.status_code != EVMC_INVALID_INSTRUCTION,
				"Invalid instruction."
			);
		if (noRevertInSource)
			solAssert(
				callResult.status_code != EVMC_REVERT,
				"SolidityEvmoneInterface: EVM One reverted"
			);
		// Bail out on serious errors encountered during a call.
		if (YulEvmoneUtility{}.seriousCallError(callResult.status_code))
			return 0;
		cout << "Unoptimized call status" << endl;
		cout << callResult.status_code << endl;
		solAssert(
			(callResult.status_code == EVMC_SUCCESS ||
			(!noRevertInSource && callResult.status_code == EVMC_REVERT) ||
			(!noInvalidInSource && callResult.status_code == EVMC_INVALID_INSTRUCTION)),
			"Unoptimised call failed."
		);
		unoptimizedState << EVMHostPrinter{hostContext, deployResult.create_address}.state();
		cout << unoptimizedState.str() << endl;
	}

	settings.runYulOptimiser = true;
	settings.optimizeStackAllocation = true;
	bytes optimisedByteCode;
	try
	{
		optimisedByteCode = YulAssembler{version, nullopt, settings, testProgram}.assemble();
	}
	catch (solidity::yul::StackTooDeepError const&)
	{
		if (!recursiveFunction)
			throw;
		else
			return 0;
	}

	if (unoptimizedStackTooDeep)
		return 0;
	// Reset host before running optimised code.
	hostContext.reset();
	evmc::Result deployResultOpt = YulEvmoneUtility{}.deployCode(optimisedByteCode, hostContext);
	solAssert(
		deployResultOpt.status_code == EVMC_SUCCESS,
		"Evmone: Optimized contract creation failed"
	);
	auto callMessageOpt = YulEvmoneUtility{}.callMessage(deployResultOpt.create_address);
	evmc::Result callResultOpt = hostContext.call(callMessageOpt);
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
	cout << "Optimized call status" << endl;
	cout << callResultOpt.status_code << endl;
	solAssert(
		(callResultOpt.status_code == EVMC_SUCCESS ||
		 (!noRevertInSource && callResultOpt.status_code == EVMC_REVERT) ||
		 (!noInvalidInSource && callResultOpt.status_code == EVMC_INVALID_INSTRUCTION)),
		"Optimised call failed."
	);
	ostringstream optimizedState;
	optimizedState << EVMHostPrinter{hostContext, deployResultOpt.create_address}.state();

	if (unoptimizedState.str() != optimizedState.str())
	{
		cout << unoptimizedState.str() << endl;
		cout << optimizedState.str() << endl;
		solAssert(
			false,
			"State of unoptimised and optimised stack reused code do not match."
		);
	}
	return 0;
}
