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

#include <test/tools/yulInterpreter/EVMInstructionInterpreter.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AST.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/SemanticInformation.h>

#include <libsolutil/Keccak256.h>
#include <libsolutil/Numeric.h>

#include <limits>

using namespace std;
using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::yul;
using namespace solidity::yul::test;

using solidity::util::h160;
using solidity::util::h256;
using solidity::util::keccak256;

namespace
{

/// Reads 32 bytes from @a _data at position @a _offset bytes while
/// interpreting @a _data to be padded with an infinite number of zero
/// bytes beyond its end.
u256 readZeroExtended(bytes const& _data, u256 const& _offset)
{
	if (_offset >= _data.size())
		return 0;
	else if (_offset + 32 <= _data.size())
		return *reinterpret_cast<h256 const*>(_data.data() + static_cast<size_t>(_offset));
	else
	{
		size_t off = static_cast<size_t>(_offset);
		u256 val;
		for (size_t i = 0; i < 32; ++i)
		{
			val <<= 8;
			if (off + i < _data.size())
				val += _data[off + i];
		}
		return val;
	}
}

/// Copy @a _size bytes of @a _source at offset @a _sourceOffset to
/// @a _target at offset @a _targetOffset. Behaves as if @a _source would
/// continue with an infinite sequence of zero bytes beyond its end.
void copyZeroExtended(
	map<u256, uint8_t>& _target, bytes const& _source,
	size_t _targetOffset, size_t _sourceOffset, size_t _size
)
{
	for (size_t i = 0; i < _size; ++i)
		_target[_targetOffset + i] = _sourceOffset + i < _source.size() ? _source[_sourceOffset + i] : 0;
}

}

using u512 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<512, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

u256 EVMInstructionInterpreter::eval(
	evmasm::InternalInstruction _instruction,
	vector<u256> const& _arguments
)
{
	using namespace solidity::evmasm;
	using evmasm::InternalInstruction;

	auto info = instructionInfo(_instruction);
	yulAssert(static_cast<size_t>(info.args) == _arguments.size(), "");

	auto const& arg = _arguments;
	switch (_instruction)
	{
	case InternalInstruction::STOP:
		logTrace(_instruction);
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	// --------------- arithmetic ---------------
	case InternalInstruction::ADD:
		return arg[0] + arg[1];
	case InternalInstruction::MUL:
		return arg[0] * arg[1];
	case InternalInstruction::SUB:
		return arg[0] - arg[1];
	case InternalInstruction::DIV:
		return arg[1] == 0 ? 0 : arg[0] / arg[1];
	case InternalInstruction::SDIV:
		return arg[1] == 0 ? 0 : s2u(u2s(arg[0]) / u2s(arg[1]));
	case InternalInstruction::MOD:
		return arg[1] == 0 ? 0 : arg[0] % arg[1];
	case InternalInstruction::SMOD:
		return arg[1] == 0 ? 0 : s2u(u2s(arg[0]) % u2s(arg[1]));
	case InternalInstruction::EXP:
		return exp256(arg[0], arg[1]);
	case InternalInstruction::NOT:
		return ~arg[0];
	case InternalInstruction::LT:
		return arg[0] < arg[1] ? 1 : 0;
	case InternalInstruction::GT:
		return arg[0] > arg[1] ? 1 : 0;
	case InternalInstruction::SLT:
		return u2s(arg[0]) < u2s(arg[1]) ? 1 : 0;
	case InternalInstruction::SGT:
		return u2s(arg[0]) > u2s(arg[1]) ? 1 : 0;
	case InternalInstruction::EQ:
		return arg[0] == arg[1] ? 1 : 0;
	case InternalInstruction::ISZERO:
		return arg[0] == 0 ? 1 : 0;
	case InternalInstruction::AND:
		return arg[0] & arg[1];
	case InternalInstruction::OR:
		return arg[0] | arg[1];
	case InternalInstruction::XOR:
		return arg[0] ^ arg[1];
	case InternalInstruction::BYTE:
		return arg[0] >= 32 ? 0 : (arg[1] >> unsigned(8 * (31 - arg[0]))) & 0xff;
	case InternalInstruction::SHL:
		return arg[0] > 255 ? 0 : (arg[1] << unsigned(arg[0]));
	case InternalInstruction::SHR:
		return arg[0] > 255 ? 0 : (arg[1] >> unsigned(arg[0]));
	case InternalInstruction::SAR:
	{
		static u256 const hibit = u256(1) << 255;
		if (arg[0] >= 256)
			return arg[1] & hibit ? u256(-1) : 0;
		else
		{
			unsigned amount = unsigned(arg[0]);
			u256 v = arg[1] >> amount;
			if (arg[1] & hibit)
				v |= u256(-1) << (256 - amount);
			return v;
		}
	}
	case InternalInstruction::ADDMOD:
		return arg[2] == 0 ? 0 : u256((u512(arg[0]) + u512(arg[1])) % arg[2]);
	case InternalInstruction::MULMOD:
		return arg[2] == 0 ? 0 : u256((u512(arg[0]) * u512(arg[1])) % arg[2]);
	case InternalInstruction::SIGNEXTEND:
		if (arg[0] >= 31)
			return arg[1];
		else
		{
			unsigned testBit = unsigned(arg[0]) * 8 + 7;
			u256 ret = arg[1];
			u256 mask = ((u256(1) << testBit) - 1);
			if (boost::multiprecision::bit_test(ret, testBit))
				ret |= ~mask;
			else
				ret &= mask;
			return ret;
		}
	// --------------- blockchain stuff ---------------
	case InternalInstruction::KECCAK256:
	{
		if (!accessMemory(arg[0], arg[1]))
			return u256("0x1234cafe1234cafe1234cafe") + arg[0];
		uint64_t offset = uint64_t(arg[0] & uint64_t(-1));
		uint64_t size = uint64_t(arg[1] & uint64_t(-1));
		return u256(keccak256(readMemory(offset, size)));
	}
	case InternalInstruction::ADDRESS:
		return h256(m_state.address, h256::AlignRight);
	case InternalInstruction::BALANCE:
		if (arg[0] == h256(m_state.address, h256::AlignRight))
			return m_state.selfbalance;
		else
			return m_state.balance;
	case InternalInstruction::SELFBALANCE:
		return m_state.selfbalance;
	case InternalInstruction::ORIGIN:
		return h256(m_state.origin, h256::AlignRight);
	case InternalInstruction::CALLER:
		return h256(m_state.caller, h256::AlignRight);
	case InternalInstruction::CALLVALUE:
		return m_state.callvalue;
	case InternalInstruction::CALLDATALOAD:
		return readZeroExtended(m_state.calldata, arg[0]);
	case InternalInstruction::CALLDATASIZE:
		return m_state.calldata.size();
	case InternalInstruction::CALLDATACOPY:
		logTrace(_instruction, arg);
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.calldata,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return 0;
	case InternalInstruction::CODESIZE:
		return m_state.code.size();
	case InternalInstruction::CODECOPY:
		logTrace(_instruction, arg);
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return 0;
	case InternalInstruction::GASPRICE:
		return m_state.gasprice;
	case InternalInstruction::CHAINID:
		return m_state.chainid;
	case InternalInstruction::BASEFEE:
		return m_state.basefee;
	case InternalInstruction::EXTCODESIZE:
		return u256(keccak256(h256(arg[0]))) & 0xffffff;
	case InternalInstruction::EXTCODEHASH:
		return u256(keccak256(h256(arg[0] + 1)));
	case InternalInstruction::EXTCODECOPY:
		logTrace(_instruction, arg);
		if (accessMemory(arg[1], arg[3]))
			// TODO this way extcodecopy and codecopy do the same thing.
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[1]), size_t(arg[2]), size_t(arg[3])
			);
		return 0;
	case InternalInstruction::RETURNDATASIZE:
		return m_state.returndata.size();
	case InternalInstruction::RETURNDATACOPY:
		logTrace(_instruction, arg);
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.returndata,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return 0;
	case InternalInstruction::BLOCKHASH:
		if (arg[0] >= m_state.blockNumber || arg[0] + 256 < m_state.blockNumber)
			return 0;
		else
			return 0xaaaaaaaa + (arg[0] - m_state.blockNumber - 256);
	case InternalInstruction::COINBASE:
		return h256(m_state.coinbase, h256::AlignRight);
	case InternalInstruction::TIMESTAMP:
		return m_state.timestamp;
	case InternalInstruction::NUMBER:
		return m_state.blockNumber;
	case InternalInstruction::DIFFICULTY:
	// TODO should be properly implemented
	case InternalInstruction::PREVRANDAO:
		return m_state.difficulty;
	case InternalInstruction::GASLIMIT:
		return m_state.gaslimit;
	// --------------- memory / storage / logs ---------------
	case InternalInstruction::MLOAD:
		accessMemory(arg[0], 0x20);
		return readMemoryWord(arg[0]);
	case InternalInstruction::MSTORE:
		accessMemory(arg[0], 0x20);
		writeMemoryWord(arg[0], arg[1]);
		return 0;
	case InternalInstruction::MSTORE8:
		accessMemory(arg[0], 1);
		m_state.memory[arg[0]] = uint8_t(arg[1] & 0xff);
		return 0;
	case InternalInstruction::SLOAD:
		return m_state.storage[h256(arg[0])];
	case InternalInstruction::SSTORE:
		m_state.storage[h256(arg[0])] = h256(arg[1]);
		return 0;
	case InternalInstruction::PC:
		return 0x77;
	case InternalInstruction::MSIZE:
		return m_state.msize;
	case InternalInstruction::GAS:
		return 0x99;
	case InternalInstruction::LOG0:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case InternalInstruction::LOG1:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case InternalInstruction::LOG2:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case InternalInstruction::LOG3:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case InternalInstruction::LOG4:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	// --------------- calls ---------------
	case InternalInstruction::CREATE:
		accessMemory(arg[1], arg[2]);
		logTrace(_instruction, arg);
		return (0xcccccc + arg[1]) & u256("0xffffffffffffffffffffffffffffffffffffffff");
	case InternalInstruction::CREATE2:
		accessMemory(arg[1], arg[2]);
		logTrace(_instruction, arg);
		return (0xdddddd + arg[1]) & u256("0xffffffffffffffffffffffffffffffffffffffff");
	case InternalInstruction::CALL:
	case InternalInstruction::CALLCODE:
		// TODO assign returndata
		accessMemory(arg[3], arg[4]);
		accessMemory(arg[5], arg[6]);
		logTrace(_instruction, arg);
		return arg[0] & 1;
	case InternalInstruction::DELEGATECALL:
	case InternalInstruction::STATICCALL:
		accessMemory(arg[2], arg[3]);
		accessMemory(arg[4], arg[5]);
		logTrace(_instruction, arg);
		return 0;
	case InternalInstruction::RETURN:
	{
		bytes data;
		if (accessMemory(arg[0], arg[1]))
			data = readMemory(arg[0], arg[1]);
		logTrace(_instruction, arg, data);
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	}
	case InternalInstruction::REVERT:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		m_state.storage.clear();
		m_state.trace.clear();
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	case InternalInstruction::INVALID:
		logTrace(_instruction);
		m_state.storage.clear();
		m_state.trace.clear();
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	case InternalInstruction::SELFDESTRUCT:
		logTrace(_instruction, arg);
		m_state.storage.clear();
		m_state.trace.clear();
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	case InternalInstruction::POP:
		break;
	// --------------- invalid in strict assembly ---------------
	case InternalInstruction::JUMP:
	case InternalInstruction::JUMPI:
	case InternalInstruction::JUMPDEST:
	case InternalInstruction::PUSH1:
	case InternalInstruction::PUSH2:
	case InternalInstruction::PUSH3:
	case InternalInstruction::PUSH4:
	case InternalInstruction::PUSH5:
	case InternalInstruction::PUSH6:
	case InternalInstruction::PUSH7:
	case InternalInstruction::PUSH8:
	case InternalInstruction::PUSH9:
	case InternalInstruction::PUSH10:
	case InternalInstruction::PUSH11:
	case InternalInstruction::PUSH12:
	case InternalInstruction::PUSH13:
	case InternalInstruction::PUSH14:
	case InternalInstruction::PUSH15:
	case InternalInstruction::PUSH16:
	case InternalInstruction::PUSH17:
	case InternalInstruction::PUSH18:
	case InternalInstruction::PUSH19:
	case InternalInstruction::PUSH20:
	case InternalInstruction::PUSH21:
	case InternalInstruction::PUSH22:
	case InternalInstruction::PUSH23:
	case InternalInstruction::PUSH24:
	case InternalInstruction::PUSH25:
	case InternalInstruction::PUSH26:
	case InternalInstruction::PUSH27:
	case InternalInstruction::PUSH28:
	case InternalInstruction::PUSH29:
	case InternalInstruction::PUSH30:
	case InternalInstruction::PUSH31:
	case InternalInstruction::PUSH32:
	case InternalInstruction::DUP1:
	case InternalInstruction::DUP2:
	case InternalInstruction::DUP3:
	case InternalInstruction::DUP4:
	case InternalInstruction::DUP5:
	case InternalInstruction::DUP6:
	case InternalInstruction::DUP7:
	case InternalInstruction::DUP8:
	case InternalInstruction::DUP9:
	case InternalInstruction::DUP10:
	case InternalInstruction::DUP11:
	case InternalInstruction::DUP12:
	case InternalInstruction::DUP13:
	case InternalInstruction::DUP14:
	case InternalInstruction::DUP15:
	case InternalInstruction::DUP16:
	case InternalInstruction::SWAP1:
	case InternalInstruction::SWAP2:
	case InternalInstruction::SWAP3:
	case InternalInstruction::SWAP4:
	case InternalInstruction::SWAP5:
	case InternalInstruction::SWAP6:
	case InternalInstruction::SWAP7:
	case InternalInstruction::SWAP8:
	case InternalInstruction::SWAP9:
	case InternalInstruction::SWAP10:
	case InternalInstruction::SWAP11:
	case InternalInstruction::SWAP12:
	case InternalInstruction::SWAP13:
	case InternalInstruction::SWAP14:
	case InternalInstruction::SWAP15:
	case InternalInstruction::SWAP16:
	case InternalInstruction::MAX_INTERNAL_INSTRUCTION:
	{
		yulAssert(false, "");
		return 0;
	}
	}

	return 0;
}

u256 EVMInstructionInterpreter::evalBuiltin(
	BuiltinFunctionForEVM const& _fun,
	vector<Expression> const& _arguments,
	vector<u256> const& _evaluatedArguments
)
{
	if (_fun.instruction)
		return eval(*_fun.instruction, _evaluatedArguments);

	string fun = _fun.name.str();
	// Evaluate datasize/offset/copy instructions
	if (fun == "datasize" || fun == "dataoffset")
	{
		string arg = std::get<Literal>(_arguments.at(0)).value.str();
		if (arg.length() < 32)
			arg.resize(32, 0);
		if (fun == "datasize")
			return u256(keccak256(arg)) & 0xfff;
		else
		{
			// Force different value than for datasize
			arg[31]++;
			arg[31]++;
			return u256(keccak256(arg)) & 0xfff;
		}
	}
	else if (fun == "datacopy")
	{
		// This is identical to codecopy.
		if (accessMemory(_evaluatedArguments.at(0), _evaluatedArguments.at(2)))
			copyZeroExtended(
				m_state.memory,
				m_state.code,
				size_t(_evaluatedArguments.at(0)),
				size_t(_evaluatedArguments.at(1) & numeric_limits<size_t>::max()),
				size_t(_evaluatedArguments.at(2))
			);
		return 0;
	}
	else
		yulAssert(false, "Unknown builtin: " + fun);
	return 0;
}


bool EVMInstructionInterpreter::accessMemory(u256 const& _offset, u256 const& _size)
{
	if (((_offset + _size) >= _offset) && ((_offset + _size + 0x1f) >= (_offset + _size)))
	{
		u256 newSize = (_offset + _size + 0x1f) & ~u256(0x1f);
		m_state.msize = max(m_state.msize, newSize);
		// We only record accesses to contiguous memory chunks that are at most 0xffff bytes
		// in size and at an offset of at most numeric_limits<size_t>::max() - 0xffff
		return _size <= 0xffff && _offset <= u256(numeric_limits<size_t>::max() - 0xffff);
	}
	else
		m_state.msize = u256(-1);

	return false;
}

bytes EVMInstructionInterpreter::readMemory(u256 const& _offset, u256 const& _size)
{
	yulAssert(_size <= 0xffff, "Too large read.");
	bytes data(size_t(_size), uint8_t(0));
	for (size_t i = 0; i < data.size(); ++i)
		data[i] = m_state.memory[_offset + i];
	return data;
}

u256 EVMInstructionInterpreter::readMemoryWord(u256 const& _offset)
{
	return u256(h256(readMemory(_offset, 32)));
}

void EVMInstructionInterpreter::writeMemoryWord(u256 const& _offset, u256 const& _value)
{
	for (size_t i = 0; i < 32; i++)
		m_state.memory[_offset + i] = uint8_t((_value >> (8 * (31 - i))) & 0xff);
}


void EVMInstructionInterpreter::logTrace(
	evmasm::InternalInstruction _instruction,
	std::vector<u256> const& _arguments,
	bytes const& _data
)
{
	logTrace(
		evmasm::instructionInfo(_instruction).name,
		SemanticInformation::memory(_instruction) == SemanticInformation::Effect::Write,
		_arguments,
		_data
	);
}

void EVMInstructionInterpreter::logTrace(
	std::string const& _pseudoInstruction,
	bool _writesToMemory,
	std::vector<u256> const& _arguments,
	bytes const& _data
)
{
	if (!(_writesToMemory && memWriteTracingDisabled()))
	{
		string message = _pseudoInstruction + "(";
		for (size_t i = 0; i < _arguments.size(); ++i)
			message += (i > 0 ? ", " : "") + formatNumber(_arguments[i]);
		message += ")";
		if (!_data.empty())
			message += " [" + util::toHex(_data) + "]";
		m_state.trace.emplace_back(std::move(message));
		if (m_state.maxTraceSize > 0 && m_state.trace.size() >= m_state.maxTraceSize)
		{
			m_state.trace.emplace_back("Trace size limit reached.");
			BOOST_THROW_EXCEPTION(TraceLimitReached());
		}
	}
}
