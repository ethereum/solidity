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
/**
 * Yul interpreter module that evaluates EVM instructions.
 */

#include <libyul/interpreter/PureEVMInstructionInterpreter.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libsolutil/FixedHash.h>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::yul;
using namespace solidity::yul::interpreter;

namespace solidity::yul::interpreter::PureEVMInstructionInterpreter {

using solidity::util::h160;
using solidity::util::h256;

EvaluationResult eval(
	langutil::EVMVersion _evmVersion,
	BuiltinFunctionForEVM const& _builtinFunction,
	std::vector<u256> const& _arguments
)
{
	if (!_builtinFunction.instruction)
		return ImpureBuiltinEncountered();
	evmasm::Instruction instruction = *_builtinFunction.instruction;

	auto info = instructionInfo(instruction, _evmVersion);
	yulAssert(static_cast<size_t>(info.args) == _arguments.size());

	auto const& arg = _arguments;
	switch (instruction)
	{
	case Instruction::STOP:
		return ExplicitlyTerminated();
	// --------------- arithmetic ---------------
	case Instruction::ADD:
		return EvaluationOk(arg[0] + arg[1]);
	case Instruction::MUL:
		return EvaluationOk(arg[0] * arg[1]);
	case Instruction::SUB:
		return EvaluationOk(arg[0] - arg[1]);
	case Instruction::DIV:
		return EvaluationOk(arg[1] == 0 ? 0 : arg[0] / arg[1]);
	case Instruction::SDIV:
		return EvaluationOk(arg[1] == 0 ? 0 : s2u(u2s(arg[0]) / u2s(arg[1])));
	case Instruction::MOD:
		return EvaluationOk(arg[1] == 0 ? 0 : arg[0] % arg[1]);
	case Instruction::SMOD:
		return EvaluationOk(arg[1] == 0 ? 0 : s2u(u2s(arg[0]) % u2s(arg[1])));
	case Instruction::EXP:
		return EvaluationOk(exp256(arg[0], arg[1]));
	case Instruction::NOT:
		return EvaluationOk(~arg[0]);
	case Instruction::LT:
		return EvaluationOk(arg[0] < arg[1] ? u256(1) : u256(0));
	case Instruction::GT:
		return EvaluationOk(arg[0] > arg[1] ? u256(1) : u256(0));
	case Instruction::SLT:
		return EvaluationOk(u2s(arg[0]) < u2s(arg[1]) ? u256(1) : u256(0));
	case Instruction::SGT:
		return EvaluationOk(u2s(arg[0]) > u2s(arg[1]) ? u256(1) : u256(0));
	case Instruction::EQ:
		return EvaluationOk(arg[0] == arg[1] ? u256(1) : u256(0));
	case Instruction::ISZERO:
		return EvaluationOk(arg[0] == 0 ? u256(1) : u256(0));
	case Instruction::AND:
		return EvaluationOk(arg[0] & arg[1]);
	case Instruction::OR:
		return EvaluationOk(arg[0] | arg[1]);
	case Instruction::XOR:
		return EvaluationOk(arg[0] ^ arg[1]);
	case Instruction::BYTE:
		return EvaluationOk(arg[0] >= 32 ? 0 : (arg[1] >> unsigned(8 * (31 - arg[0]))) & 0xff);
	case Instruction::SHL:
		return EvaluationOk(arg[0] > 255 ? 0 : (arg[1] << unsigned(arg[0])));
	case Instruction::SHR:
		return EvaluationOk(arg[0] > 255 ? 0 : (arg[1] >> unsigned(arg[0])));
	case Instruction::SAR:
	{
		static u256 const hibit = u256(1) << 255;
		if (arg[0] >= 256)
			return EvaluationOk(arg[1] & hibit ? u256(-1) : 0);
		else
		{
			unsigned amount = unsigned(arg[0]);
			u256 v = arg[1] >> amount;
			if (arg[1] & hibit)
				v |= u256(-1) << (256 - amount);
			return EvaluationOk(v);
		}
	}
	case Instruction::ADDMOD:
		return EvaluationOk(arg[2] == 0 ? 0 : u256((u512(arg[0]) + u512(arg[1])) % arg[2]));
	case Instruction::MULMOD:
		return EvaluationOk(arg[2] == 0 ? 0 : u256((u512(arg[0]) * u512(arg[1])) % arg[2]));
	case Instruction::SIGNEXTEND:
		if (arg[0] >= 31)
			return EvaluationOk(arg[1]);
		else
		{
			unsigned testBit = unsigned(arg[0]) * 8 + 7;
			u256 ret = arg[1];
			u256 mask = ((u256(1) << testBit) - 1);
			if (boost::multiprecision::bit_test(ret, testBit))
				ret |= ~mask;
			else
				ret &= mask;
			return EvaluationOk(ret);
		}
	// --------------- blockchain stuff ---------------
	case Instruction::KECCAK256:
	case Instruction::ADDRESS:
	case Instruction::BALANCE:
	case Instruction::SELFBALANCE:
	case Instruction::ORIGIN:
	case Instruction::CALLER:
	case Instruction::CALLVALUE:
	case Instruction::CALLDATALOAD:
	case Instruction::CALLDATASIZE:
	case Instruction::CALLDATACOPY:
	case Instruction::CODESIZE:
	case Instruction::CODECOPY:
	case Instruction::GASPRICE:
	case Instruction::CHAINID:
	case Instruction::BASEFEE:
	case Instruction::BLOBHASH:
	case Instruction::BLOBBASEFEE:
	case Instruction::EXTCODESIZE:
	case Instruction::EXTCODEHASH:
	case Instruction::EXTCODECOPY:
	case Instruction::RETURNDATASIZE:
	case Instruction::RETURNDATACOPY:
	case Instruction::MCOPY:
	case Instruction::BLOCKHASH:
	case Instruction::COINBASE:
	case Instruction::TIMESTAMP:
	case Instruction::NUMBER:
	case Instruction::PREVRANDAO:
	case Instruction::GASLIMIT:
		return ImpureBuiltinEncountered();
	// --------------- memory / storage / logs ---------------
	case Instruction::MLOAD:
	case Instruction::MSTORE:
	case Instruction::MSTORE8:
	case Instruction::SLOAD:
	case Instruction::SSTORE:
	case Instruction::PC:
	case Instruction::MSIZE:
	case Instruction::GAS:
	case Instruction::LOG0:
	case Instruction::LOG1:
	case Instruction::LOG2:
	case Instruction::LOG3:
	case Instruction::LOG4:
	case Instruction::TLOAD:
	case Instruction::TSTORE:
		return ImpureBuiltinEncountered();
	// --------------- calls ---------------
	case Instruction::DATALOADN:
	case Instruction::CREATE:
	case Instruction::CREATE2:
	case Instruction::CALL:
	case Instruction::CALLCODE:
	case Instruction::DELEGATECALL:
	case Instruction::STATICCALL:
	case Instruction::RETURN:
	case Instruction::REVERT:
	case Instruction::INVALID:
	case Instruction::SELFDESTRUCT:
		return ImpureBuiltinEncountered();

	case Instruction::POP:
		return EvaluationOk();
	// --------------- invalid in strict assembly ---------------
	case Instruction::JUMP:
	case Instruction::JUMPI:
	case Instruction::JUMPDEST:
	case Instruction::PUSH0:
	case Instruction::PUSH1:
	case Instruction::PUSH2:
	case Instruction::PUSH3:
	case Instruction::PUSH4:
	case Instruction::PUSH5:
	case Instruction::PUSH6:
	case Instruction::PUSH7:
	case Instruction::PUSH8:
	case Instruction::PUSH9:
	case Instruction::PUSH10:
	case Instruction::PUSH11:
	case Instruction::PUSH12:
	case Instruction::PUSH13:
	case Instruction::PUSH14:
	case Instruction::PUSH15:
	case Instruction::PUSH16:
	case Instruction::PUSH17:
	case Instruction::PUSH18:
	case Instruction::PUSH19:
	case Instruction::PUSH20:
	case Instruction::PUSH21:
	case Instruction::PUSH22:
	case Instruction::PUSH23:
	case Instruction::PUSH24:
	case Instruction::PUSH25:
	case Instruction::PUSH26:
	case Instruction::PUSH27:
	case Instruction::PUSH28:
	case Instruction::PUSH29:
	case Instruction::PUSH30:
	case Instruction::PUSH31:
	case Instruction::PUSH32:
	case Instruction::DUP1:
	case Instruction::DUP2:
	case Instruction::DUP3:
	case Instruction::DUP4:
	case Instruction::DUP5:
	case Instruction::DUP6:
	case Instruction::DUP7:
	case Instruction::DUP8:
	case Instruction::DUP9:
	case Instruction::DUP10:
	case Instruction::DUP11:
	case Instruction::DUP12:
	case Instruction::DUP13:
	case Instruction::DUP14:
	case Instruction::DUP15:
	case Instruction::DUP16:
	case Instruction::SWAP1:
	case Instruction::SWAP2:
	case Instruction::SWAP3:
	case Instruction::SWAP4:
	case Instruction::SWAP5:
	case Instruction::SWAP6:
	case Instruction::SWAP7:
	case Instruction::SWAP8:
	case Instruction::SWAP9:
	case Instruction::SWAP10:
	case Instruction::SWAP11:
	case Instruction::SWAP12:
	case Instruction::SWAP13:
	case Instruction::SWAP14:
	case Instruction::SWAP15:
	case Instruction::SWAP16:
	{
		yulAssert(false, "Instruction not allowed in strict assembly.");
	}
	}

	util::unreachable();
}

}
