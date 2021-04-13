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

static evmc::VM evmone = evmc::VM{evmc_create_evmone()};

DEFINE_PROTO_FUZZER(Program const& _input)
{
	// Solidity creates an invalid instruction for subobjects, so we simply
	// ignore them in this fuzzer.
	if (_input.has_obj())
		return;
	bool filterStatefulInstructions = true;
	bool filterUnboundedLoops = true;
	bool filterMemoryWrites = true;
	bool filterLogs = true;
	ProtoConverter converter(
		filterStatefulInstructions,
		filterUnboundedLoops,
		filterMemoryWrites,
		filterLogs
	);
	string yulSubObject = converter.programToString(_input);
	// Fuzzer also fuzzes the EVM version field.
	langutil::EVMVersion version = converter.version();
	EVMHost hostContext(version, evmone);
	hostContext.reset();

	// Do not proceed with tests that are too large. 1200 is an arbitrary
	// threshold.
	if (yulSubObject.size() > 1200)
		return;

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

	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
		ofstream of(dump_path);
		of.write(yul_source.data(), static_cast<streamsize>(yul_source.size()));
	}

	solidity::frontend::OptimiserSettings settings = solidity::frontend::OptimiserSettings::none();
	AssemblyStack stackUnoptimized(version, AssemblyStack::Language::StrictAssembly, settings);
	solAssert(
		stackUnoptimized.parseAndAnalyze("source", yulSubObject),
		"Parsing fuzzer generated input failed."
	);
	ostringstream unoptimizedState;
	yulFuzzerUtil::TerminationReason termReason = yulFuzzerUtil::interpret(
		unoptimizedState,
		stackUnoptimized.parserResult()->code,
		EVMDialect::strictAssemblyForEVMObjects(version),
		true
	);
	if (yulFuzzerUtil::resourceLimitsExceeded(termReason))
		return;

	AssemblyStack stackOptimized;
	solAssert(
		stackOptimized.parseAndAnalyze("source", yulSubObject),
		"Parsing fuzzer generated input failed."
	);
	YulOptimizerTestCommon optimizerTest(
		stackOptimized.parserResult(),
		EVMDialect::strictAssemblyForEVMObjects(version)
	);
	// HACK: Force this to fake stack limit evader for now
	string step = "stackLimitEvader";
	optimizerTest.setStep(step);
//	optimizerTest.setStep(optimizerTest.randomOptimiserStep(_input.step()));
	shared_ptr<solidity::yul::Block> astBlock = optimizerTest.run();
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
		("fuzzerInput", AsmPrinter{}(*astBlock))
		.render();
	bytes optimisedByteCode;
	optimisedByteCode = YulAssembler{version, settings, optimisedProgram}.assemble();

	// Reset host before running optimised code.
	hostContext.reset();
	evmc::result deployResultOpt = YulEvmoneUtility{}.deployCode(optimisedByteCode, hostContext);
	solAssert(
		deployResultOpt.status_code == EVMC_SUCCESS,
		"Evmone: Optimized contract creation failed"
	);
	auto callMessageOpt = YulEvmoneUtility{}.callMessage(deployResultOpt.create_address);
	evmc::result callResultOpt = hostContext.call(callMessageOpt);
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
}
