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
 * Yul interpreter module that evaluates Ewasm builtins.
 */

#include <test/tools/yulInterpreter/EwasmBuiltinInterpreter.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmData.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/Keccak256.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;

using solidity::util::h256;

namespace
{

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

/// Count leading zeros for uint64
uint64_t clz(uint64_t _v)
{
	if (_v == 0)
		return 64;

	uint64_t r = 0;
	while (!(_v & 0x8000000000000000))
	{
		r += 1;
		_v = _v << 1;
	}
	return r;
}

}

using u512 = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<512, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

u256 EwasmBuiltinInterpreter::evalBuiltin(YulString _fun, vector<u256> const& _arguments)
{
	vector<uint64_t> arg;
	for (u256 const& a: _arguments)
		arg.emplace_back(uint64_t(a & uint64_t(-1)));

	if (_fun == "datasize"_yulstring)
		return u256(keccak256(h256(_arguments.at(0)))) & 0xfff;
	else if (_fun == "dataoffset"_yulstring)
		return u256(keccak256(h256(_arguments.at(0) + 2))) & 0xfff;
	else if (_fun == "datacopy"_yulstring)
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
		return 0;
	}
	else if (_fun == "drop"_yulstring)
		return {};
	else if (_fun == "unreachable"_yulstring)
	{
		logTrace(evmasm::Instruction::INVALID, {});
		throw ExplicitlyTerminated();
	}
	else if (_fun == "i64.add"_yulstring)
		return arg[0] + arg[1];
	else if (_fun == "i64.sub"_yulstring)
		return arg[0] - arg[1];
	else if (_fun == "i64.mul"_yulstring)
		return arg[0] * arg[1];
	else if (_fun == "i64.div_u"_yulstring)
	{
		if (arg[1] == 0)
			throw ExplicitlyTerminated();
		else
			return arg[0] / arg[1];
	}
	else if (_fun == "i64.rem_u"_yulstring)
	{
		if (arg[1] == 0)
			throw ExplicitlyTerminated();
		else
			return arg[0] % arg[1];
	}
	else if (_fun == "i64.and"_yulstring)
		return arg[0] & arg[1];
	else if (_fun == "i64.or"_yulstring)
		return arg[0] | arg[1];
	else if (_fun == "i64.xor"_yulstring)
		return arg[0] ^ arg[1];
	else if (_fun == "i64.shl"_yulstring)
		return arg[0] << arg[1];
	else if (_fun == "i64.shr_u"_yulstring)
		return arg[0] >> arg[1];
	else if (_fun == "i64.eq"_yulstring)
		return arg[0] == arg[1] ? 1 : 0;
	else if (_fun == "i64.ne"_yulstring)
		return arg[0] != arg[1] ? 1 : 0;
	else if (_fun == "i64.eqz"_yulstring)
		return arg[0] == 0 ? 1 : 0;
	else if (_fun == "i64.clz"_yulstring)
		return clz(arg[0]);
	else if (_fun == "i64.lt_u"_yulstring)
		return arg[0] < arg[1] ? 1 : 0;
	else if (_fun == "i64.gt_u"_yulstring)
		return arg[0] > arg[1] ? 1 : 0;
	else if (_fun == "i64.le_u"_yulstring)
		return arg[0] <= arg[1] ? 1 : 0;
	else if (_fun == "i64.ge_u"_yulstring)
		return arg[0] >= arg[1] ? 1 : 0;
	else if (_fun == "i64.store"_yulstring)
	{
		accessMemory(arg[0], 8);
		writeMemoryWord(arg[0], arg[1]);
		return 0;
	}
	else if (_fun == "i64.store8"_yulstring)
	{
		accessMemory(arg[0], 1);
		writeMemoryByte(arg[0], static_cast<uint8_t>(arg[1] & 0xff));
		return 0;
	}
	else if (_fun == "i64.load"_yulstring)
	{
		accessMemory(arg[0], 8);
		return readMemoryWord(arg[0]);
	}
	else if (_fun == "eth.getAddress"_yulstring)
	{
		writeAddress(arg[0], m_state.address);
		return 0;
	}
	else if (_fun == "eth.getExternalBalance"_yulstring)
	{
		// TODO this does not read the address, but is consistent with
		// EVM interpreter implementation.
		// If we take the address into account, this needs to use readAddress.
		writeU128(arg[1], m_state.balance);
		return 0;
	}
	else if (_fun == "eth.getBlockHash"_yulstring)
	{
		if (arg[0] >= m_state.blockNumber || arg[0] + 256 < m_state.blockNumber)
			return 1;
		else
		{
			writeU256(arg[1], 0xaaaaaaaa + u256(arg[0] - m_state.blockNumber - 256));
			return 0;
		}
	}
	else if (_fun == "eth.call"_yulstring)
	{
		// TODO read args from memory
		// TODO use readAddress to read address.
		logTrace(evmasm::Instruction::CALL, {});
		return arg[0] & 1;
	}
	else if (_fun == "eth.callDataCopy"_yulstring)
	{
		if (arg[1] + arg[2] < arg[1] || arg[1] + arg[2] > m_state.calldata.size())
			throw ExplicitlyTerminated();
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.calldata,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return {};
	}
	else if (_fun == "eth.getCallDataSize"_yulstring)
		return m_state.calldata.size();
	else if (_fun == "eth.callCode"_yulstring)
	{
		// TODO read args from memory
		// TODO use readAddress to read address.
		logTrace(evmasm::Instruction::CALLCODE, {});
		return arg[0] & 1;
	}
	else if (_fun == "eth.callDelegate"_yulstring)
	{
		// TODO read args from memory
		// TODO use readAddress to read address.
		logTrace(evmasm::Instruction::DELEGATECALL, {});
		return arg[0] & 1;
	}
	else if (_fun == "eth.callStatic"_yulstring)
	{
		// TODO read args from memory
		// TODO use readAddress to read address.
		logTrace(evmasm::Instruction::STATICCALL, {});
		return arg[0] & 1;
	}
	else if (_fun == "eth.storageStore"_yulstring)
	{
		m_state.storage[h256(readU256(arg[0]))] = readU256((arg[1]));
		return 0;
	}
	else if (_fun == "eth.storageLoad"_yulstring)
	{
		writeU256(arg[1], m_state.storage[h256(readU256(arg[0]))]);
		return 0;
	}
	else if (_fun == "eth.getCaller"_yulstring)
	{
		// TODO should this only write 20 bytes?
		writeAddress(arg[0], m_state.caller);
		return 0;
	}
	else if (_fun == "eth.getCallValue"_yulstring)
	{
		writeU128(arg[0], m_state.callvalue);
		return 0;
	}
	else if (_fun == "eth.codeCopy"_yulstring)
	{
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return 0;
	}
	else if (_fun == "eth.getCodeSize"_yulstring)
		return m_state.code.size();
	else if (_fun == "eth.getBlockCoinbase"_yulstring)
	{
		writeAddress(arg[0], m_state.coinbase);
		return 0;
	}
	else if (_fun == "eth.create"_yulstring)
	{
		// TODO access memory
		// TODO use writeAddress to store resulting address
		logTrace(evmasm::Instruction::CREATE, {});
		return 0xcccccc + arg[1];
	}
	else if (_fun == "eth.getBlockDifficulty"_yulstring)
	{
		writeU256(arg[0], m_state.difficulty);
		return 0;
	}
	else if (_fun == "eth.externalCodeCopy"_yulstring)
	{
		// TODO use readAddress to read address.
		if (accessMemory(arg[1], arg[3]))
			// TODO this way extcodecopy and codecopy do the same thing.
			copyZeroExtended(
				m_state.memory, m_state.code,
				size_t(arg[1]), size_t(arg[2]), size_t(arg[3])
			);
		return 0;
	}
	else if (_fun == "eth.getExternalCodeSize"_yulstring)
		// Generate "random" code length. Make sure it fits the page size.
		return u256(keccak256(h256(readAddress(arg[0])))) & 0xfff;
	else if (_fun == "eth.getGasLeft"_yulstring)
		return 0x99;
	else if (_fun == "eth.getBlockGasLimit"_yulstring)
		return uint64_t(m_state.gaslimit);
	else if (_fun == "eth.getTxGasPrice"_yulstring)
	{
		writeU128(arg[0], m_state.gasprice);
		return 0;
	}
	else if (_fun == "eth.log"_yulstring)
	{
		uint64_t numberOfTopics = arg[2];
		if (numberOfTopics > 4)
			throw ExplicitlyTerminated();
		logTrace(evmasm::logInstruction(numberOfTopics), {});
		return 0;
	}
	else if (_fun == "eth.getBlockNumber"_yulstring)
		return m_state.blockNumber;
	else if (_fun == "eth.getTxOrigin"_yulstring)
	{
		writeAddress(arg[0], m_state.origin);
		return 0;
	}
	else if (_fun == "eth.finish"_yulstring)
	{
		bytes data;
		if (accessMemory(arg[0], arg[1]))
			data = readMemory(arg[0], arg[1]);
		logTrace(evmasm::Instruction::RETURN, {}, data);
		throw ExplicitlyTerminated();
	}
	else if (_fun == "eth.revert"_yulstring)
	{
		bytes data;
		if (accessMemory(arg[0], arg[1]))
			data = readMemory(arg[0], arg[1]);
		logTrace(evmasm::Instruction::REVERT, {}, data);
		throw ExplicitlyTerminated();
	}
	else if (_fun == "eth.getReturnDataSize"_yulstring)
		return m_state.returndata.size();
	else if (_fun == "eth.returnDataCopy"_yulstring)
	{
		if (arg[1] + arg[2] < arg[1] || arg[1] + arg[2] > m_state.returndata.size())
			throw ExplicitlyTerminated();
		if (accessMemory(arg[0], arg[2]))
			copyZeroExtended(
				m_state.memory, m_state.calldata,
				size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
			);
		return {};
	}
	else if (_fun == "eth.selfDestruct"_yulstring)
	{
		// TODO use readAddress to read address.
		logTrace(evmasm::Instruction::SELFDESTRUCT, {});
		throw ExplicitlyTerminated();
	}
	else if (_fun == "eth.getBlockTimestamp"_yulstring)
		return m_state.timestamp;

	yulAssert(false, "Unknown builtin: " + _fun.str() + " (or implementation did not return)");

	return 0;
}

bool EwasmBuiltinInterpreter::accessMemory(u256 const& _offset, u256 const& _size)
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

bytes EwasmBuiltinInterpreter::readMemory(uint64_t _offset, uint64_t _size)
{
	yulAssert(_size <= 0xffff, "Too large read.");
	bytes data(size_t(_size), uint8_t(0));
	for (size_t i = 0; i < data.size(); ++i)
		data[i] = m_state.memory[_offset + i];
	return data;
}

uint64_t EwasmBuiltinInterpreter::readMemoryWord(uint64_t _offset)
{
	uint64_t r = 0;
	for (size_t i = 0; i < 8; i++)
		r |= uint64_t(m_state.memory[_offset + i]) << (i * 8);
	return r;
}

void EwasmBuiltinInterpreter::writeMemoryWord(uint64_t _offset, uint64_t _value)
{
	for (size_t i = 0; i < 8; i++)
		m_state.memory[_offset + i] = uint8_t((_value >> (i * 8)) & 0xff);
}

void EwasmBuiltinInterpreter::writeMemoryByte(uint64_t _offset, uint8_t _value)
{
	m_state.memory[_offset] = _value;
}

void EwasmBuiltinInterpreter::writeU256(uint64_t _offset, u256 _value, size_t _croppedTo)
{
	accessMemory(_offset, _croppedTo);
	for (size_t i = 0; i < _croppedTo; i++)
	{
		m_state.memory[_offset + _croppedTo - 1 - i] = uint8_t(_value & 0xff);
		_value >>= 8;
	}
}

u256 EwasmBuiltinInterpreter::readU256(uint64_t _offset, size_t _croppedTo)
{
	accessMemory(_offset, _croppedTo);
	u256 value;
	for (size_t i = 0; i < _croppedTo; i++)
		value = (value << 8) | m_state.memory[_offset + i];

	return value;
}

void EwasmBuiltinInterpreter::logTrace(evmasm::Instruction _instruction, std::vector<u256> const& _arguments, bytes const& _data)
{
	logTrace(evmasm::instructionInfo(_instruction).name, _arguments, _data);
}

void EwasmBuiltinInterpreter::logTrace(std::string const& _pseudoInstruction, std::vector<u256> const& _arguments, bytes const& _data)
{
	string message = _pseudoInstruction + "(";
	for (size_t i = 0; i < _arguments.size(); ++i)
		message += (i > 0 ? ", " : "") + util::formatNumber(_arguments[i]);
	message += ")";
	if (!_data.empty())
		message += " [" + util::toHex(_data) + "]";
	m_state.trace.emplace_back(std::move(message));
	if (m_state.maxTraceSize > 0 && m_state.trace.size() >= m_state.maxTraceSize)
	{
		m_state.trace.emplace_back("Trace size limit reached.");
		throw TraceLimitReached();
	}
}
