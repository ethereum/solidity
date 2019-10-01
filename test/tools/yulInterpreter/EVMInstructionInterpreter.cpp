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
/**
 * Yul interpreter module that evaluates EVM instructions.
 */

#include <test/tools/yulInterpreter/EVMInstructionInterpreter.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmData.h>

#include <libevmasm/Instruction.h>

#include <libdevcore/Keccak256.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace yul::test;

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
		return *reinterpret_cast<h256 const*>(_data.data() + size_t(_offset));
	else
	{
		size_t off = size_t(_offset);
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
	dev::eth::Instruction _instruction,
	vector<u256> const& _arguments
)
{
	using namespace dev::eth;
	using dev::eth::Instruction;

	auto info = instructionInfo(_instruction);
	yulAssert(size_t(info.args) == _arguments.size(), "");

	auto const& arg = _arguments;
	switch (_instruction)
	{
	case Instruction::STOP:
		throw ExplicitlyTerminated();
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
		return u256(keccak256(readMemory(offset, size)));
	}
	case Instruction::ADDRESS:
		return m_state.address;
	case Instruction::BALANCE:
		return m_state.balance;
	case Instruction::SELFBALANCE:
		return m_state.selfbalance;
	case Instruction::ORIGIN:
		return m_state.origin;
	case Instruction::CALLER:
		return m_state.caller;
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
		return 0;
	case Instruction::CODESIZE:
		return m_state.code.size();
	case Instruction::CODECOPY:
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return 0;
	case Instruction::GASPRICE:
		return m_state.gasprice;
	case Instruction::CHAINID:
		return m_state.chainid;
	case Instruction::EXTCODESIZE:
		return u256(keccak256(h256(arg[0]))) & 0xffffff;
	case Instruction::EXTCODEHASH:
		return u256(keccak256(h256(arg[0] + 1)));
	case Instruction::EXTCODECOPY:
		logTrace(_instruction, arg);
		if (accessMemory(arg[1], arg[3]))
			// TODO this way extcodecopy and codecopy do the same thing.
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[1]), size_t(arg[2]), size_t(arg[3])
			);
		return 0;
	case Instruction::RETURNDATASIZE:
		return m_state.returndata.size();
	case Instruction::RETURNDATACOPY:
		logTrace(_instruction, arg);
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.returndata,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return 0;
	case Instruction::BLOCKHASH:
		if (arg[0] >= m_state.blockNumber || arg[0] + 256 < m_state.blockNumber)
			return 0;
		else
			return 0xaaaaaaaa + (arg[0] - m_state.blockNumber - 256);
	case Instruction::COINBASE:
		return m_state.coinbase;
	case Instruction::TIMESTAMP:
		return m_state.timestamp;
	case Instruction::NUMBER:
		return m_state.blockNumber;
	case Instruction::DIFFICULTY:
		return m_state.difficulty;
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
	// --------------- calls ---------------
	case Instruction::CREATE:
		accessMemory(arg[1], arg[2]);
		logTrace(_instruction, arg);
		return 0xcccccc + arg[1];
	case Instruction::CREATE2:
		accessMemory(arg[2], arg[3]);
		logTrace(_instruction, arg);
		return 0xdddddd + arg[1];
	case Instruction::CALL:
	case Instruction::CALLCODE:
		// TODO assign returndata
		accessMemory(arg[3], arg[4]);
		accessMemory(arg[5], arg[6]);
		logTrace(_instruction, arg);
		return arg[0] & 1;
	case Instruction::DELEGATECALL:
	case Instruction::STATICCALL:
		accessMemory(arg[2], arg[3]);
		accessMemory(arg[4], arg[5]);
		logTrace(_instruction, arg);
		return 0;
	case Instruction::RETURN:
	{
		bytes data;
		if (accessMemory(arg[0], arg[1]))
			data = readMemory(arg[0], arg[1]);
		logTrace(_instruction, arg, data);
		throw ExplicitlyTerminated();
	}
	case Instruction::REVERT:
		accessMemory(arg[0], arg[1]);
		logTrace(_instruction, arg);
		throw ExplicitlyTerminated();
	case Instruction::INVALID:
		logTrace(_instruction);
		throw ExplicitlyTerminated();
	case Instruction::SELFDESTRUCT:
		logTrace(_instruction, arg);
		throw ExplicitlyTerminated();
	case Instruction::POP:
		break;
	// --------------- invalid in strict assembly ---------------
	case Instruction::JUMP:
	case Instruction::JUMPI:
	case Instruction::JUMPDEST:
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
	// --------------- EVM 2.0 ---------------
	case Instruction::JUMPTO:
	case Instruction::JUMPIF:
	case Instruction::JUMPV:
	case Instruction::JUMPSUB:
	case Instruction::JUMPSUBV:
	case Instruction::BEGINSUB:
	case Instruction::BEGINDATA:
	case Instruction::RETURNSUB:
	case Instruction::PUTLOCAL:
	case Instruction::GETLOCAL:
	{
		yulAssert(false, "");
		return 0;
	}
	}

	return 0;
}

u256 EVMInstructionInterpreter::evalBuiltin(BuiltinFunctionForEVM const& _fun, const std::vector<u256>& _arguments)
{
	if (_fun.instruction)
		return eval(*_fun.instruction, _arguments);
	else if (_fun.name == "datasize"_yulstring)
		return u256(keccak256(h256(_arguments.at(0)))) & 0xfff;
	else if (_fun.name == "dataoffset"_yulstring)
		return u256(keccak256(h256(_arguments.at(0) + 2))) & 0xfff;
	else if (_fun.name == "datacopy"_yulstring)
	{
		// This is identical to codecopy.
		if (accessMemory(_arguments.at(0), _arguments.at(2)))
			copyZeroExtended(
				m_state.memory,
				m_state.code,
				size_t(_arguments.at(0)),
				size_t(_arguments.at(1) & size_t(-1)),
				size_t(_arguments.at(2))
			);
	}
	else
		yulAssert(false, "Unknown builtin: " + _fun.name.str());
	return 0;
}


bool EVMInstructionInterpreter::accessMemory(u256 const& _offset, u256 const& _size)
{
	if (((_offset + _size) >= _offset) && ((_offset + _size + 0x1f) >= (_offset + _size)))
	{
		u256 newSize = (_offset + _size + 0x1f) & ~u256(0x1f);
		m_state.msize = max(m_state.msize, newSize);
		return _size <= 0xffff;
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


void EVMInstructionInterpreter::logTrace(dev::eth::Instruction _instruction, std::vector<u256> const& _arguments, bytes const& _data)
{
	logTrace(dev::eth::instructionInfo(_instruction).name, _arguments, _data);
}

void EVMInstructionInterpreter::logTrace(std::string const& _pseudoInstruction, std::vector<u256> const& _arguments, bytes const& _data)
{
	string message = _pseudoInstruction + "(";
	for (size_t i = 0; i < _arguments.size(); ++i)
		message += (i > 0 ? ", " : "") + formatNumber(_arguments[i]);
	message += ")";
	if (!_data.empty())
		message += " [" + toHex(_data) + "]";
	m_state.trace.emplace_back(std::move(message));
	if (m_state.maxTraceSize > 0 && m_state.trace.size() >= m_state.maxTraceSize)
	{
		m_state.trace.emplace_back("Trace size limit reached.");
		throw TraceLimitReached();
	}
}
