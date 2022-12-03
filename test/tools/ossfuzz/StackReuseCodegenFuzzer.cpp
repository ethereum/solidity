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

#include <test/EVMHost.h>

#include <test/tools/ossfuzz/YulEvmoneInterface.h>

#include <libyul/Exceptions.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/EVMVersion.h>

#include <evmone/evmone.h>

#include <src/libfuzzer/libfuzzer_macro.h>

#include <fstream>

using namespace solidity;
using namespace solidity::test;
using namespace solidity::test::fuzzer;
using namespace solidity::yul;
using namespace solidity::yul::test::yul_fuzzer;
using namespace solidity::langutil;
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
	ProtoConverter converter(
		filterStatefulInstructions,
		filterUnboundedLoops
	);
	string yul_source = converter.programToString(_input);
	// Do not fuzz the EVM Version field.
	// See https://github.com/ethereum/solidity/issues/12590
	langutil::EVMVersion version;
	EVMHost hostContext(version, evmone);
	hostContext.reset();

	if (const char* dump_path = getenv("PROTO_FUZZER_DUMP_PATH"))
	{
		ofstream of(dump_path);
		of.write(yul_source.data(), static_cast<streamsize>(yul_source.size()));
	}

	YulStringRepository::reset();

	solidity::frontend::OptimiserSettings settings = solidity::frontend::OptimiserSettings::full();
	settings.runYulOptimiser = false;
	settings.optimizeStackAllocation = false;
	bytes unoptimisedByteCode;
	try
	{
		unoptimisedByteCode = YulAssembler{version, nullopt, settings, yul_source}.assemble();
	}
	catch (solidity::yul::StackTooDeepError const&)
	{
		return;
	}

	evmc::Result deployResult = YulEvmoneUtility{}.deployCode(unoptimisedByteCode, hostContext);
	if (deployResult.status_code != EVMC_SUCCESS)
		return;
	auto callMessage = YulEvmoneUtility{}.callMessage(deployResult.create_address);
	evmc::Result callResult = hostContext.call(callMessage);
	// If the fuzzer synthesized input does not contain the revert opcode which
	// we lazily check by string find, the EVM call should not revert.
	bool noRevertInSource = yul_source.find("revert") == string::npos;
	bool noInvalidInSource = yul_source.find("invalid") == string::npos;
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
		return;
	solAssert(
		(callResult.status_code == EVMC_SUCCESS ||
		(!noRevertInSource && callResult.status_code == EVMC_REVERT) ||
		(!noInvalidInSource && callResult.status_code == EVMC_INVALID_INSTRUCTION)),
		"Unoptimised call failed."
	);
	ostringstream unoptimizedState;
	unoptimizedState << EVMHostPrinter{hostContext, deployResult.create_address}.state();

	settings.runYulOptimiser = true;
	settings.optimizeStackAllocation = true;
	bytes optimisedByteCode;
	try
	{
		optimisedByteCode = YulAssembler{version, nullopt, settings, yul_source}.assemble();
	}
	catch (solidity::yul::StackTooDeepError const&)
	{
		return;
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
	optimizedState << EVMHostPrinter{hostContext, deployResultOpt.create_address}.state();

	solAssert(
		unoptimizedState.str() == optimizedState.str(),
		"State of unoptimised and optimised stack reused code do not match."
	);
}
