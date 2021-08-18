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

#include <test/tools/ossfuzz/yulProto.pb.h>
#include <test/tools/ossfuzz/protoToYul.h>
#include <test/tools/ossfuzz/yulFuzzerCommon.h>

#include <test/EVMHost.h>

#include <test/tools/ossfuzz/YulEvmoneInterface.h>

#include <test/libyul/YulOptimizerTestCommon.h>

#include <libyul/Exceptions.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/CompilabilityChecker.h>

#include <libyul/AsmPrinter.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/EVMVersion.h>

#include <evmone/evmone.h>

#include <src/libfuzzer/libfuzzer_macro.h>

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

namespace
{
#if 0
/// @returns true if there are no recursive functions, false otherwise.
bool recursiveFunctionHasUnreachableVariables(Dialect const& _dialect, yul::Object& _object)
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
#endif

string optimiseYulSubObject(
	shared_ptr<yul::Object> _subObject,
	Dialect const& _dialect,
	bool _forceOldPipeline
)
{
	YulOptimizerTestCommon optimizerTest(
		_subObject,
		_dialect,
		_forceOldPipeline
	);
	// Run circular references pruner and then stack limit evader.
	string step = "fullSuite";
	optimizerTest.setStep(step);
	shared_ptr<solidity::yul::Block> astBlock = optimizerTest.run();
	string optimisedSubObject = AsmPrinter{}(*astBlock);
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
	})")
	("fuzzerInput", optimisedSubObject)
	.render();
	return optimisedProgram;
}

void runOnce(
	EVMHost& _host,
	bytes& _deployCode,
	ostringstream& _state,
	string _yulSubObject
)
{
	// Reset host before running optimised code.
	_host.reset();
	evmc::result deployResultOpt = YulEvmoneUtility{}.deployCode(_deployCode, _host);
	solAssert(
		deployResultOpt.status_code == EVMC_SUCCESS,
		"Evmone: Contract creation failed"
	);
	auto callMessageOpt = YulEvmoneUtility{}.callMessage(deployResultOpt.create_address);
	evmc::result callResultOpt = _host.call(callMessageOpt);
	// Bail out if we ran out of gas.
	if (callResultOpt.status_code == EVMC_OUT_OF_GAS)
		return;
	bool noRevertInSource = _yulSubObject.find("revert") == string::npos;
	bool noInvalidInSource = _yulSubObject.find("invalid") == string::npos;
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
		"Call failed."
		);
	_state << EVMHostPrinter{_host, deployResultOpt.create_address}.state();
}
}

static evmc::VM evmone = evmc::VM{evmc_create_evmone()};

DEFINE_PROTO_FUZZER(Program const& _input)
{
	// Solidity creates an invalid instruction for subobjects, so we simply
	// ignore them in this fuzzer.
	if (_input.has_obj())
		return;
	ProtoConverter converter;
	string fuzzerInput = converter.programToString(_input);
	langutil::EVMVersion version;
	EVMHost hostContext(version, evmone);

	YulStringRepository::reset();


	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
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
		})");
		string yul_source = yulObjectFormat("fuzzerInput", fuzzerInput).render();
		ofstream of(dump_path);
		of.write(yul_source.data(), static_cast<streamsize>(yul_source.size()));
	}

	solidity::frontend::OptimiserSettings settings = solidity::frontend::OptimiserSettings::none();
	AssemblyStack yulStack(version, AssemblyStack::Language::StrictAssembly, settings);
	solAssert(
		yulStack.parseAndAnalyze("source", fuzzerInput),
		"Parsing fuzzer generated input failed."
	);
	std::shared_ptr<yul::Object> fuzzedYulObject = yulStack.parserResult();
	Dialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(version);
	// Run Yul optimizer with old pipeline
	string controlOptYul = optimiseYulSubObject(fuzzedYulObject, dialect, true);
	bytes controlByteCode;
	settings.optimizeStackAllocation = true;
	settings.forceOldPipeline = true;
	bool controlThrowsStackTooDeep = false;
	try
	{
		controlByteCode = YulAssembler{version, settings, controlOptYul}.assemble();
	}
	catch (yul::StackTooDeepError const&)
	{
		controlThrowsStackTooDeep = true;
	}

	ostringstream controlState;
	runOnce(hostContext, controlByteCode, controlState, fuzzerInput);
	// Run Yul optimizer with new pipeline
	string experimentOptYul = optimiseYulSubObject(fuzzedYulObject, dialect, false);
	settings.forceOldPipeline = false;
	bytes experimentByteCode;
#if 0
	bool recursiveFunction = recursiveFunctionHasUnreachableVariables(dialect, *fuzzedYulObject);
	if (recursiveFunction)
		cout << "Fuzzer print: Recursive function" << endl;
#endif
	experimentByteCode = YulAssembler{version, settings, experimentOptYul}.assemble();
#if 0
	catch (yul::StackTooDeepError const&)
	{
		// If there are recursive functions, stack too deep errors are expected
		// even post stack evasion optimisation and hence ignored. Otherwise,
		// they are rethrown for further investigation.
		if (recursiveFunction)
			return;
		throw;
	}
#endif

	ostringstream experimentState;
	runOnce(hostContext, experimentByteCode, experimentState, fuzzerInput);

	if (
		!controlThrowsStackTooDeep &&
		(controlState.str() != experimentState.str())
	)
	{
		cout << controlState.str() << endl;
		cout << experimentState.str() << endl;
		solAssert(false, "State of unoptimised and optimised code do not match.");
	}
}
