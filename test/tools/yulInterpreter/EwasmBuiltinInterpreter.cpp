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
 * Yul interpreter module that evaluates Ewasm builtins.
 */

#include <test/tools/yulInterpreter/EwasmBuiltinInterpreter.h>

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AST.h>

#include <libevmasm/Instruction.h>

#include <libsolutil/Keccak256.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::test;

using solidity::util::h160;
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

/// Count leading zeros for uint64. Following WebAssembly rules, it returns 64 for @a _v being zero.
/// NOTE: the clz builtin of the compiler may or may not do this
uint64_t clz64(uint64_t _v)
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

/// Count trailing zeros for uint32. Following WebAssembly rules, it returns 32 for @a _v being zero.
/// NOTE: the ctz builtin of the compiler may or may not do this
uint32_t ctz32(uint32_t _v)
{
	if (_v == 0)
		return 32;

	uint32_t r = 0;
	while (!(_v & 1))
	{
		r++;
		_v >>= 1;
	}
	return r;
}

/// Count trailing zeros for uint64. Following WebAssembly rules, it returns 64 for @a _v being zero.
/// NOTE: the ctz builtin of the compiler may or may not do this
uint64_t ctz64(uint64_t _v)
{
	if (_v == 0)
		return 64;

	uint64_t r = 0;
	while (!(_v & 1))
	{
		r++;
		_v >>= 1;
	}
	return r;
}

/// Count number of bits set for uint64
uint64_t popcnt(uint64_t _v)
{
	uint64_t r = 0;
	while (_v)
	{
		r += (_v & 1);
		_v >>= 1;
	}
	return r;
}

}

u256 EwasmBuiltinInterpreter::evalBuiltin(
	YulString _functionName,
	vector<Expression> const& _arguments,
	vector<u256> const& _evaluatedArguments
)
{
	vector<uint64_t> arg;
	for (u256 const& a: _evaluatedArguments)
		arg.emplace_back(uint64_t(a & uint64_t(-1)));

	string const fun = _functionName.str();
	if (fun == "datasize" || fun == "dataoffset")
	{
		string arg = std::get<Literal>(_arguments.at(0)).value.str();
		if (arg.length() < 32)
			arg.resize(32, 0);
		if (fun == "datasize")
			return u256(util::keccak256(arg)) & 0xfff;
		else if (fun == "dataoffset")
		{
			// Force different value than for datasize
			arg[31]++;
			arg[31]++;
			return u256(util::keccak256(arg)) & 0xfff;
		}
	}
	else if (fun == "datacopy")
	{
		// This is identical to codecopy.
		accessMemory(_evaluatedArguments.at(0), _evaluatedArguments.at(2));
		copyZeroExtended(
			m_state.memory,
			m_state.code,
			static_cast<size_t>(_evaluatedArguments.at(0)),
			static_cast<size_t>(_evaluatedArguments.at(1) & numeric_limits<size_t>::max()),
			static_cast<size_t>(_evaluatedArguments.at(2))
		);
		return 0;
	}
	else if (fun == "i32.drop" || fun == "i64.drop" || fun == "nop")
		return {};
	else if (fun == "i32.select")
	{
		if ((arg.at(2) & 0xffffffff) == 0)
			return arg.at(1);
		else
			return arg.at(0);
	}
	else if (fun == "i64.select")
	{
		if ((arg.at(2) & 0xffffffffffffffff) == 0)
			return arg.at(1);
		else
			return arg.at(0);
	}
	else if (fun == "i32.wrap_i64")
		return arg.at(0) & uint32_t(-1);
	else if (fun == "i64.extend_i32_u")
		// Return the same as above because everything is u256 anyway.
		return arg.at(0) & uint32_t(-1);
	else if (fun == "unreachable")
	{
		logTrace(evmasm::Instruction::INVALID, {});
		throw ExplicitlyTerminated();
	}
	else if (fun == "i64.store")
	{
		accessMemory(arg[0], 8);
		writeMemoryWord(arg[0], arg[1]);
		return 0;
	}
	else if (fun == "i64.store8" || fun == "i32.store8")
	{
		accessMemory(arg[0], 1);
		writeMemoryByte(arg[0], static_cast<uint8_t>(arg[1] & 0xff));
		return 0;
	}
	else if (fun == "i64.load")
	{
		accessMemory(arg[0], 8);
		return readMemoryWord(arg[0]);
	}
	else if (fun == "i32.store")
	{
		accessMemory(arg[0], 4);
		writeMemoryHalfWord(arg[0], static_cast<uint32_t>(arg[1]));
		return 0;
	}
	else if (fun == "i32.load")
	{
		accessMemory(arg[0], 4);
		return readMemoryHalfWord(arg[0]);
	}
	else if (fun == "i32.clz")
		// NOTE: the clz implementation assumes 64-bit inputs, hence the adjustment
		return clz64(arg[0] & uint32_t(-1)) - 32;
	else if (fun == "i64.clz")
		return clz64(arg[0]);
	else if (fun == "i32.ctz")
		return ctz32(uint32_t(arg[0] & uint32_t(-1)));
	else if (fun == "i64.ctz")
		return ctz64(arg[0]);

	string prefix = fun;
	string suffix;
	auto dot = prefix.find(".");
	if (dot != string::npos)
	{
		suffix = prefix.substr(dot + 1);
		prefix.resize(dot);
	}

	if (prefix == "i32")
	{
		vector<uint32_t> halfWordArgs;
		for (uint64_t a: arg)
			halfWordArgs.push_back(uint32_t(a & uint32_t(-1)));
		return evalWasmBuiltin(suffix, halfWordArgs);
	}
	else if (prefix == "i64")
		return evalWasmBuiltin(suffix, arg);
	else if (prefix == "eth")
		return evalEthBuiltin(suffix, arg);

	yulAssert(false, "Unknown builtin: " + fun + " (or implementation did not return)");

	return 0;
}

template <typename Word>
u256 EwasmBuiltinInterpreter::evalWasmBuiltin(string const& _fun, vector<Word> const& _arguments)
{
	vector<Word> const& arg = _arguments;

	if (_fun == "add")
		return arg[0] + arg[1];
	else if (_fun == "sub")
		return arg[0] - arg[1];
	else if (_fun == "mul")
		return arg[0] * arg[1];
	else if (_fun == "div_u")
	{
		if (arg[1] == 0)
			throw ExplicitlyTerminated();
		else
			return arg[0] / arg[1];
	}
	else if (_fun == "rem_u")
	{
		if (arg[1] == 0)
			throw ExplicitlyTerminated();
		else
			return arg[0] % arg[1];
	}
	else if (_fun == "and")
		return arg[0] & arg[1];
	else if (_fun == "or")
		return arg[0] | arg[1];
	else if (_fun == "xor")
		return arg[0] ^ arg[1];
	else if (_fun == "shl")
		return arg[0] << arg[1];
	else if (_fun == "shr_u")
		return arg[0] >> arg[1];
	else if (_fun == "eq")
		return arg[0] == arg[1] ? 1 : 0;
	else if (_fun == "ne")
		return arg[0] != arg[1] ? 1 : 0;
	else if (_fun == "eqz")
		return arg[0] == 0 ? 1 : 0;
	else if (_fun == "popcnt")
		return popcnt(arg[0]);
	else if (_fun == "lt_u")
		return arg[0] < arg[1] ? 1 : 0;
	else if (_fun == "gt_u")
		return arg[0] > arg[1] ? 1 : 0;
	else if (_fun == "le_u")
		return arg[0] <= arg[1] ? 1 : 0;
	else if (_fun == "ge_u")
		return arg[0] >= arg[1] ? 1 : 0;

	yulAssert(false, "Unknown builtin: " + _fun + " (or implementation did not return)");

	return 0;
}

u256 EwasmBuiltinInterpreter::evalEthBuiltin(string const& _fun, vector<uint64_t> const& _arguments)
{
	vector<uint64_t> const& arg = _arguments;

	if (_fun == "getAddress")
	{
		writeAddress(arg[0], m_state.address);
		return 0;
	}
	else if (_fun == "getExternalBalance")
	{
		readAddress(arg[0]);
		writeU128(arg[1], m_state.balance);
		return 0;
	}
	else if (_fun == "getBlockHash")
	{
		if (arg[0] >= m_state.blockNumber || arg[0] + 256 < m_state.blockNumber)
			return 1;
		else
		{
			writeBytes32(arg[1], h256(0xaaaaaaaa + u256(arg[0] - m_state.blockNumber - 256)));
			return 0;
		}
	}
	else if (_fun == "call")
	{
		readAddress(arg[1]);
		readU128(arg[2]);
		accessMemory(arg[3], arg[4]);
		logTrace(evmasm::Instruction::CALL, {});
		return arg[0] & 1;
	}
	else if (_fun == "callDataCopy")
	{
		if (arg[1] + arg[2] < arg[1] || arg[1] + arg[2] > m_state.calldata.size())
			throw ExplicitlyTerminated();
		accessMemory(arg[0], arg[2]);
		copyZeroExtended(
			m_state.memory, m_state.calldata,
			size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
		);
		return {};
	}
	else if (_fun == "getCallDataSize")
		return m_state.calldata.size();
	else if (_fun == "callCode")
	{
		readAddress(arg[1]);
		readU128(arg[2]);
		accessMemory(arg[3], arg[4]);
		logTrace(evmasm::Instruction::CALLCODE, {});
		return arg[0] & 1;
	}
	else if (_fun == "callDelegate")
	{
		readAddress(arg[1]);
		accessMemory(arg[2], arg[3]);
		logTrace(evmasm::Instruction::DELEGATECALL, {});
		return arg[0] & 1;
	}
	else if (_fun == "callStatic")
	{
		readAddress(arg[1]);
		accessMemory(arg[2], arg[3]);
		logTrace(evmasm::Instruction::STATICCALL, {});
		return arg[0] & 1;
	}
	else if (_fun == "storageStore")
	{
		m_state.storage[readBytes32(arg[0])] = readBytes32(arg[1]);
		return 0;
	}
	else if (_fun == "storageLoad")
	{
		writeBytes32(arg[1], m_state.storage[readBytes32(arg[0])]);
		return 0;
	}
	else if (_fun == "getCaller")
	{
		writeAddress(arg[0], m_state.caller);
		return 0;
	}
	else if (_fun == "getCallValue")
	{
		writeU128(arg[0], m_state.callvalue);
		return 0;
	}
	else if (_fun == "codeCopy")
	{
		accessMemory(arg[0], arg[2]);
		copyZeroExtended(
			m_state.memory, m_state.code,
			size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
		);
		return 0;
	}
	else if (_fun == "getCodeSize")
		return m_state.code.size();
	else if (_fun == "getBlockCoinbase")
	{
		writeAddress(arg[0], m_state.coinbase);
		return 0;
	}
	else if (_fun == "create")
	{
		readU128(arg[0]);
		accessMemory(arg[1], arg[2]);
		logTrace(evmasm::Instruction::CREATE, {});
		writeAddress(arg[3], h160(h256(0xcccccc + arg[1])));
		return 1;
	}
	else if (_fun == "getBlockDifficulty")
	{
		writeU256(arg[0], m_state.difficulty);
		return 0;
	}
	else if (_fun == "externalCodeCopy")
	{
		readAddress(arg[0]);
		accessMemory(arg[1], arg[3]);
		// TODO this way extcodecopy and codecopy do the same thing.
		copyZeroExtended(
			m_state.memory, m_state.code,
			size_t(arg[1]), size_t(arg[2]), size_t(arg[3])
		);
		return 0;
	}
	else if (_fun == "getExternalCodeSize")
		// Generate "random" code length.
		return uint32_t(u256(keccak256(h256(readAddress(arg[0])))) & 0xfff);
	else if (_fun == "getGasLeft")
		return 0x99;
	else if (_fun == "getBlockGasLimit")
		return uint64_t(m_state.gaslimit);
	else if (_fun == "getTxGasPrice")
	{
		writeU128(arg[0], m_state.gasprice);
		return 0;
	}
	else if (_fun == "log")
	{
		accessMemory(arg[0], arg[1]);
		uint64_t numberOfTopics = arg[2];
		if (numberOfTopics > 4)
			throw ExplicitlyTerminated();
		if (numberOfTopics > 0)
			readBytes32(arg[3]);
		if (numberOfTopics > 1)
			readBytes32(arg[4]);
		if (numberOfTopics > 2)
			readBytes32(arg[5]);
		if (numberOfTopics > 3)
			readBytes32(arg[6]);
		logTrace(evmasm::logInstruction(static_cast<unsigned>(numberOfTopics)), {});
		return 0;
	}
	else if (_fun == "getBlockNumber")
		return m_state.blockNumber;
	else if (_fun == "getTxOrigin")
	{
		writeAddress(arg[0], m_state.origin);
		return 0;
	}
	else if (_fun == "finish")
	{
		bytes data;
		accessMemory(arg[0], arg[1]);
		data = readMemory(arg[0], arg[1]);
		logTrace(evmasm::Instruction::RETURN, {}, data);
		throw ExplicitlyTerminated();
	}
	else if (_fun == "revert")
	{
		bytes data;
		accessMemory(arg[0], arg[1]);
		data = readMemory(arg[0], arg[1]);
		logTrace(evmasm::Instruction::REVERT, {}, data);
		throw ExplicitlyTerminated();
	}
	else if (_fun == "getReturnDataSize")
		return m_state.returndata.size();
	else if (_fun == "returnDataCopy")
	{
		if (arg[1] + arg[2] < arg[1] || arg[1] + arg[2] > m_state.returndata.size())
			throw ExplicitlyTerminated();
		accessMemory(arg[0], arg[2]);
		copyZeroExtended(
			m_state.memory, m_state.calldata,
			size_t(arg[0]), size_t(arg[1]), size_t(arg[2])
		);
		return {};
	}
	else if (_fun == "selfDestruct")
	{
		readAddress(arg[0]);
		logTrace(evmasm::Instruction::SELFDESTRUCT, {});
		throw ExplicitlyTerminated();
	}
	else if (_fun == "getBlockTimestamp")
		return m_state.timestamp;

	yulAssert(false, "Unknown builtin: " + _fun + " (or implementation did not return)");

	return 0;
}

void EwasmBuiltinInterpreter::accessMemory(u256 const& _offset, u256 const& _size)
{
	// Single WebAssembly page.
	// TODO: Support expansion in this interpreter.
	m_state.msize = 65536;

	if (((_offset + _size) < _offset) || ((_offset + _size) > m_state.msize))
		// Ewasm throws out of bounds exception as opposed to the EVM.
		throw ExplicitlyTerminated();
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

uint32_t EwasmBuiltinInterpreter::readMemoryHalfWord(uint64_t _offset)
{
	uint32_t r = 0;
	for (size_t i = 0; i < 4; i++)
		r |= uint32_t(m_state.memory[_offset + i]) << (i * 8);
	return r;
}

void EwasmBuiltinInterpreter::writeMemory(uint64_t _offset, bytes const& _value)
{
	for (size_t i = 0; i < _value.size(); i++)
		m_state.memory[_offset + i] = _value[i];
}

void EwasmBuiltinInterpreter::writeMemoryWord(uint64_t _offset, uint64_t _value)
{
	for (size_t i = 0; i < 8; i++)
		m_state.memory[_offset + i] = uint8_t((_value >> (i * 8)) & 0xff);
}

void EwasmBuiltinInterpreter::writeMemoryHalfWord(uint64_t _offset, uint32_t _value)
{
	for (size_t i = 0; i < 4; i++)
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
		m_state.memory[_offset + i] = uint8_t(_value & 0xff);
		_value >>= 8;
	}
}

u256 EwasmBuiltinInterpreter::readU256(uint64_t _offset, size_t _croppedTo)
{
	accessMemory(_offset, _croppedTo);
	u256 value{0};
	for (size_t i = 0; i < _croppedTo; i++)
		value = (value << 8) | m_state.memory[_offset + _croppedTo - 1 - i];

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
