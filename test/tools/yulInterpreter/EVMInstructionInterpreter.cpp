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
#include <libyul/Utilities.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/SemanticInformation.h>

#include <liblangutil/Exceptions.h>
#include <libsolutil/Keccak256.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/picosha2.h>

#include <limits>

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

}

namespace solidity::yul::test
{

void copyZeroExtended(
	std::map<u256, uint8_t>& _target,
	bytes const& _source,
	size_t _targetOffset,
	size_t _sourceOffset,
	size_t _size
)
{
	for (size_t i = 0; i < _size; ++i)
		_target[_targetOffset + i] = (_sourceOffset + i < _source.size() ? _source[_sourceOffset + i] : 0);
}

void copyZeroExtendedWithOverlap(
	std::map<u256, uint8_t>& _target,
	std::map<u256, uint8_t> const& _source,
	size_t _targetOffset,
	size_t _sourceOffset,
	size_t _size
)
{
	if (_targetOffset >= _sourceOffset)
		for (size_t i = _size; i > 0; --i)
			_target[_targetOffset + i - 1] = (_source.count(_sourceOffset + i - 1) != 0 ? _source.at(_sourceOffset + i - 1) : 0);
	else
		for (size_t i = 0; i < _size; ++i)
			_target[_targetOffset + i] = (_source.count(_sourceOffset + i) != 0 ? _source.at(_sourceOffset + i) : 0);
}

}

u256 EVMInstructionInterpreter::eval(
	evmasm::Instruction _instruction,
	std::vector<u256> const& _arguments
)
{
	using namespace solidity::evmasm;
	using evmasm::Instruction;

	auto info = instructionInfo(_instruction, m_evmVersion);
	yulAssert(static_cast<size_t>(info.args) == _arguments.size(), "");

	auto const& arg = _arguments;
	switch (_instruction)
	{
	case Instruction::STOP:
		logTrace(_instruction);
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	// --------------- arithmetic ---------------
	case Instruction::ADD:
		return arg[0] + arg[1];
	case Instruction::MUL:
		return arg[0] * arg[1];
	case Instruction::SUB:
		return arg[0] - arg[1];
	case Instruction::DIV:
		return arg[1] == 0 ? 0 : arg[0] / arg[1];
	case Instruction::SDIV:
		return arg[1] == 0 ? 0 : s2u(u2s(arg[0]) / u2s(arg[1]));
	case Instruction::MOD:
		return arg[1] == 0 ? 0 : arg[0] % arg[1];
	case Instruction::SMOD:
		return arg[1] == 0 ? 0 : s2u(u2s(arg[0]) % u2s(arg[1]));
	case Instruction::EXP:
		return exp256(arg[0], arg[1]);
	case Instruction::NOT:
		return ~arg[0];
	case Instruction::LT:
		return arg[0] < arg[1] ? 1 : 0;
	case Instruction::GT:
		return arg[0] > arg[1] ? 1 : 0;
	case Instruction::SLT:
		return u2s(arg[0]) < u2s(arg[1]) ? 1 : 0;
	case Instruction::SGT:
		return u2s(arg[0]) > u2s(arg[1]) ? 1 : 0;
	case Instruction::EQ:
		return arg[0] == arg[1] ? 1 : 0;
	case Instruction::ISZERO:
		return arg[0] == 0 ? 1 : 0;
	case Instruction::AND:
		return arg[0] & arg[1];
	case Instruction::OR:
		return arg[0] | arg[1];
	case Instruction::XOR:
		return arg[0] ^ arg[1];
	case Instruction::BYTE:
		return arg[0] >= 32 ? 0 : (arg[1] >> unsigned(8 * (31 - arg[0]))) & 0xff;
	case Instruction::SHL:
		return arg[0] > 255 ? 0 : (arg[1] << unsigned(arg[0]));
	case Instruction::SHR:
		return arg[0] > 255 ? 0 : (arg[1] >> unsigned(arg[0]));
	case Instruction::SAR:
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
	case Instruction::ADDMOD:
		return arg[2] == 0 ? 0 : u256((u512(arg[0]) + u512(arg[1])) % arg[2]);
	case Instruction::MULMOD:
		return arg[2] == 0 ? 0 : u256((u512(arg[0]) * u512(arg[1])) % arg[2]);
	case Instruction::SIGNEXTEND:
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
	case Instruction::KECCAK256:
	{
		if (!accessMemory(arg[0], arg[1]))
			return u256("0x1234cafe1234cafe1234cafe") + arg[0];
		uint64_t offset = uint64_t(arg[0] & uint64_t(-1));
		uint64_t size = uint64_t(arg[1] & uint64_t(-1));
		return u256(keccak256(m_state.readMemory(offset, size)));
	}
	case Instruction::ADDRESS:
		return h256(m_state.address, h256::AlignRight);
	case Instruction::BALANCE:
		if (arg[0] == h256(m_state.address, h256::AlignRight))
			return m_state.selfbalance;
		else
			return m_state.balance;
	case Instruction::SELFBALANCE:
		return m_state.selfbalance;
	case Instruction::ORIGIN:
		return h256(m_state.origin, h256::AlignRight);
	case Instruction::CALLER:
		return h256(m_state.caller, h256::AlignRight);
	case Instruction::CALLVALUE:
		return m_state.callvalue;
	case Instruction::CALLDATALOAD:
		return readZeroExtended(m_state.calldata, arg[0]);
	case Instruction::CALLDATASIZE:
		return m_state.calldata.size();
	case Instruction::CALLDATACOPY:
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.calldata,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::CODESIZE:
		return m_state.code.size();
	case Instruction::CODECOPY:
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::GASPRICE:
		return m_state.gasprice;
	case Instruction::CHAINID:
		return m_state.chainid;
	case Instruction::BASEFEE:
		return m_state.basefee;
	case Instruction::BLOBHASH:
		return blobHash(arg[0]);
	case Instruction::BLOBBASEFEE:
		return m_state.blobbasefee;
	case Instruction::EXTCODESIZE:
		return u256(keccak256(h256(arg[0]))) & 0xffffff;
	case Instruction::EXTCODEHASH:
		return u256(keccak256(h256(arg[0] + 1)));
	case Instruction::EXTCODECOPY:
		if (accessMemory(arg[1], arg[3]))
			// TODO this way extcodecopy and codecopy do the same thing.
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[1]), size_t(arg[2]), size_t(arg[3])
			);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::RETURNDATASIZE:
		return m_state.returndata.size();
	case Instruction::RETURNDATACOPY:
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.returndata,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::MCOPY:
		if (accessMemory(arg[1], arg[2]) && accessMemory(arg[0], arg[2]))
			copyZeroExtendedWithOverlap(
				m_state.memory,
				m_state.memory,
				static_cast<size_t>(arg[0]),
				static_cast<size_t>(arg[1]),
				static_cast<size_t>(arg[2])
			);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::BLOCKHASH:
		if (arg[0] >= m_state.blockNumber || arg[0] + 256 < m_state.blockNumber)
			return 0;
		else
			return 0xaaaaaaaa + (arg[0] - m_state.blockNumber - 256);
	case Instruction::COINBASE:
		return h256(m_state.coinbase, h256::AlignRight);
	case Instruction::TIMESTAMP:
		return m_state.timestamp;
	case Instruction::NUMBER:
		return m_state.blockNumber;
	case Instruction::PREVRANDAO:
		return (m_evmVersion < langutil::EVMVersion::paris()) ? m_state.difficulty : m_state.prevrandao;
	case Instruction::GASLIMIT:
		return m_state.gaslimit;
	// --------------- memory / storage / logs ---------------
	case Instruction::MLOAD:
		accessMemory(arg[0], 0x20);
		return readMemoryWord(arg[0]);
	case Instruction::MSTORE:
		accessMemory(arg[0], 0x20);
		writeMemoryWord(arg[0], arg[1]);
		return 0;
	case Instruction::MSTORE8:
		accessMemory(arg[0], 1);
		m_state.memory[arg[0]] = uint8_t(arg[1] & 0xff);
		return 0;
	case Instruction::SLOAD:
		return m_state.storage[h256(arg[0])];
	case Instruction::SSTORE:
		m_state.storage[h256(arg[0])] = h256(arg[1]);
		return 0;
	case Instruction::PC:
		return 0x77;
	case Instruction::MSIZE:
		return m_state.msize;
	case Instruction::GAS:
		return 0x99;
	case Instruction::LOG0:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::LOG1:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::LOG2:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::LOG3:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::LOG4:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::TLOAD:
		return m_state.transientStorage[h256(arg[0])];
	case Instruction::TSTORE:
		m_state.transientStorage[h256(arg[0])] = h256(arg[1]);
		return 0;
	// --------------- calls ---------------
	case Instruction::CREATE:
		accessMemory(arg[1], arg[2]);
		logTrace(_instruction, arg);
		if (arg[2] != 0)
			return (0xcccccc + arg[1]) & u256("0xffffffffffffffffffffffffffffffffffffffff");
		else
			return 0xcccccc;
	case Instruction::CREATE2:
		accessMemory(arg[1], arg[2]);
		logTrace(_instruction, arg);
		if (arg[2] != 0)
			return (0xdddddd + arg[1]) & u256("0xffffffffffffffffffffffffffffffffffffffff");
		else
			return 0xdddddd;
	case Instruction::CALL:
	case Instruction::CALLCODE:
		accessMemory(arg[3], arg[4]);
		accessMemory(arg[5], arg[6]);
		logTrace(_instruction, arg);
		// Randomly fail based on the called address if it isn't a call to self.
		// Used for fuzzing.
		return (
			(arg[0] > 0) &&
			(arg[1] == util::h160::Arith(m_state.address) || (arg[1] & 1))
		) ? 1 : 0;
	case Instruction::DELEGATECALL:
	case Instruction::STATICCALL:
		accessMemory(arg[2], arg[3]);
		accessMemory(arg[4], arg[5]);
		logTrace(_instruction, arg);
		// Randomly fail based on the called address if it isn't a call to self.
		// Used for fuzzing.
		return (
			(arg[0] > 0) &&
			(arg[1] == util::h160::Arith(m_state.address) || (arg[1] & 1))
		) ? 1 : 0;
	case Instruction::RETURN:
	{
		m_state.returndata = {};
		if (accessMemory(arg[0], arg[1]))
			m_state.returndata = m_state.readMemory(arg[0], arg[1]);
		logTrace(_instruction, arg, m_state.returndata);
		BOOST_THROW_EXCEPTION(ExplicitlyTerminatedWithReturn());
	}
	case Instruction::REVERT:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		m_state.storage.clear();
		m_state.transientStorage.clear();
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	case Instruction::INVALID:
		logTrace(_instruction);
		m_state.storage.clear();
		m_state.transientStorage.clear();
		m_state.trace.clear();
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	case Instruction::SELFDESTRUCT:
		logTrace(_instruction, arg);
		m_state.storage.clear();
		m_state.transientStorage.clear();
		m_state.trace.clear();
		BOOST_THROW_EXCEPTION(ExplicitlyTerminated());
	case Instruction::POP:
		return 0;
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
		yulAssert(false, "Impossible in strict assembly.");
	case Instruction::DATALOADN:
	case Instruction::EOFCREATE:
	case Instruction::RETURNCONTRACT:
		solUnimplemented("EOF not yet supported by Yul interpreter.");
	}

	util::unreachable();
}

u256 EVMInstructionInterpreter::evalBuiltin(
	BuiltinFunctionForEVM const& _fun,
	std::vector<Expression> const& _arguments,
	std::vector<u256> const& _evaluatedArguments
)
{
	if (_fun.instruction)
		return eval(*_fun.instruction, _evaluatedArguments);

	std::string const& fun = _fun.name;
	// Evaluate datasize/offset/copy instructions
	if (fun == "datasize" || fun == "dataoffset")
	{
		std::string arg = formatLiteral(std::get<Literal>(_arguments.at(0)));
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
		if (
			_evaluatedArguments.at(2) != 0 &&
			accessMemory(_evaluatedArguments.at(0), _evaluatedArguments.at(2))
		)
			copyZeroExtended(
				m_state.memory,
				m_state.code,
				size_t(_evaluatedArguments.at(0)),
				size_t(_evaluatedArguments.at(1) & std::numeric_limits<size_t>::max()),
				size_t(_evaluatedArguments.at(2))
			);
		return 0;
	}
	else if (fun == "memoryguard")
		return _evaluatedArguments.at(0);
	else
		yulAssert(false, "Unknown builtin: " + fun);
	return 0;
}


bool EVMInstructionInterpreter::accessMemory(u256 const& _offset, u256 const& _size)
{
	if (_size == 0)
		return true;

	if (_offset <= (_offset + _size) && (_offset + _size) <= (_offset + _size + 0x1f))
	{
		u256 newMSize = (_offset + _size + 0x1f) & ~u256(0x1f);
		m_state.msize = std::max(m_state.msize, newMSize);
		// We only record accesses to contiguous memory chunks that are at most s_maxRangeSize bytes
		// in size and at an offset of at most numeric_limits<size_t>::max() - s_maxRangeSize
		return _size <= s_maxRangeSize && _offset <= u256(std::numeric_limits<size_t>::max() - s_maxRangeSize);
	}

	m_state.msize = u256(-1);
	return false;
}

bytes EVMInstructionInterpreter::readMemory(u256 const& _offset, u256 const& _size)
{
	yulAssert(_size <= s_maxRangeSize, "Too large read.");
	bytes data(size_t(_size), uint8_t(0));
	for (size_t i = 0; i < data.size(); ++i)
		data[i] = m_state.memory[_offset + i];
	return data;
}

u256 EVMInstructionInterpreter::readMemoryWord(u256 const& _offset)
{
	return u256(h256(m_state.readMemory(_offset, 32)));
}

void EVMInstructionInterpreter::writeMemoryWord(u256 const& _offset, u256 const& _value)
{
	for (size_t i = 0; i < 32; i++)
		m_state.memory[_offset + i] = uint8_t((_value >> (8 * (31 - i))) & 0xff);
}


void EVMInstructionInterpreter::logTrace(
	evmasm::Instruction _instruction,
	std::vector<u256> const& _arguments,
	bytes const& _data
)
{
	logTrace(
		evmasm::instructionInfo(_instruction, m_evmVersion).name,
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
		std::string message = _pseudoInstruction + "(";
		std::pair<bool, size_t> inputMemoryPtrModified = isInputMemoryPtrModified(_pseudoInstruction, _arguments);
		for (size_t i = 0; i < _arguments.size(); ++i)
		{
			bool printZero = inputMemoryPtrModified.first && inputMemoryPtrModified.second == i;
			u256 arg = printZero ? 0 : _arguments[i];
			message += (i > 0 ? ", " : "") + formatNumber(arg);
		}
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

std::pair<bool, size_t> EVMInstructionInterpreter::isInputMemoryPtrModified(
	std::string const& _pseudoInstruction,
	std::vector<u256> const& _arguments
)
{
	if (_pseudoInstruction == "RETURN" || _pseudoInstruction == "REVERT")
	{
		if (_arguments[1] == 0)
			return {true, 0};
		else
			return {false, 0};
	}
	else if (
		_pseudoInstruction == "RETURNDATACOPY" || _pseudoInstruction == "CALLDATACOPY"
		|| _pseudoInstruction == "CODECOPY")
	{
		if (_arguments[2] == 0)
			return {true, 0};
		else
			return {false, 0};
	}
	else if (_pseudoInstruction == "EXTCODECOPY")
	{
		if (_arguments[3] == 0)
			return {true, 1};
		else
			return {false, 0};
	}
	else if (
		_pseudoInstruction == "LOG0" || _pseudoInstruction == "LOG1" || _pseudoInstruction == "LOG2"
		|| _pseudoInstruction == "LOG3" || _pseudoInstruction == "LOG4")
	{
		if (_arguments[1] == 0)
			return {true, 0};
		else
			return {false, 0};
	}
	if (_pseudoInstruction == "CREATE" || _pseudoInstruction == "CREATE2")
	{
		if (_arguments[2] == 0)
			return {true, 1};
		else
			return {false, 0};
	}
	if (_pseudoInstruction == "CALL" || _pseudoInstruction == "CALLCODE")
	{
		if (_arguments[4] == 0)
			return {true, 3};
		else
			return {false, 0};
	}
	else if (_pseudoInstruction == "DELEGATECALL" || _pseudoInstruction == "STATICCALL")
	{
		if (_arguments[3] == 0)
			return {true, 2};
		else
			return {false, 0};
	}
	else
		return {false, 0};
}

h256 EVMInstructionInterpreter::blobHash(u256 const& _index)
{
	yulAssert(m_evmVersion.hasBlobHash());
	if (_index >= m_state.blobCommitments.size())
		return util::FixedHash<32>{};

	h256 hashedCommitment = h256(picosha2::hash256(toBigEndian(m_state.blobCommitments[static_cast<size_t>(_index)])));
	yulAssert(m_state.blobHashVersion.size == 1);
	hashedCommitment[0] = *m_state.blobHashVersion.data();
	yulAssert(hashedCommitment.size == 32);
	return hashedCommitment;
}
