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
* Yul util functions that are known by name and always included.
 */

#include <libsolidity/codegen/ir/WellKnownUtilFunctions.h>

#include <libsolidity/codegen/YulUtilFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>

#include <liblangutil/Exceptions.h>
#include <libsolutil/Whiskers.h>

#include <boost/algorithm/string/split.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;

WellKnownUtilFunctions::WellKnownUtilFunctions(
	langutil::EVMVersion _evmVersion,
	RevertStrings _revertStrings,
	MultiUseYulFunctionCollector& _functionCollector
)
{
	YulUtilFunctions utils(_evmVersion, _revertStrings, _functionCollector);

	// Turn a memory object plus offset into an absolute memory address.
	// Not to be used outside the memory helpers.
	_functionCollector.createFunction("m_absolute", [&]() {
		return R"(
			function m_absolute(mobj, offset) -> mptr {
				mptr := add(mobj, offset)
			}
		)";
	});

	_functionCollector.createFunction("m_allocate", [&]() {
		return Whiskers(R"(
			function m_allocate(size) -> mobj {
				mobj := m_allocateUnbounded()
				m_finalize(mobj, size)
			}
		)")
		("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer))
		("panic", utils.panicFunction())
		.render();
	});

	_functionCollector.createFunction("m_allocateUnbounded", [&]() {
		return Whiskers(R"(
			function m_allocateUnbounded() -> mobj {
				mobj := mload(<freeMemoryPointer>)
			}
		)")
		("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer))
		.render();
	});

	_functionCollector.createFunction("m_finalize", [&]() {
		return Whiskers(R"(
			function m_finalize(mobj, size) {
				size := <roundUp>(size)
				let newFreePtr := add(mobj, size)
				// protect against overflow
				if or(gt(size, 0xffffffffffffffff), lt(newFreePtr, mobj)) { <panic>() }
				mstore(<freeMemoryPointer>, newFreePtr)
			}
		)")
		("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer))
		("panic", utils.panicFunction())
		("roundUp", utils.roundUpFunction())
		.render();
	});

	_functionCollector.createFunction("m_discard", [&]() {
		return "\nfunction m_discard(mobj) { }\n";
	});

	_functionCollector.createFunction("m_load", [&]() {
		return R"(
			function m_load(mobj, offset) -> value {
				value := mload(m_absolute(mobj, offset))
			}
		)";
	});

	_functionCollector.createFunction("m_store", [&]() {
		return R"(
			function m_store(mobj, offset, value) {
				mstore(m_absolute(mobj, offset), value)
			}
		)";
	});

	_functionCollector.createFunction("m_store8", [&]() {
		return R"(
			function m_store8(mobj, offset, value) {
				mstore8(m_absolute(mobj, offset), value)
			}
		)";
	});

	_functionCollector.createFunction("m_slice", [&]() {
		return R"(
			function m_slice(mobj, offset) -> ret {
				ret := add(mobj, offset)
			}
		)";
	});

	_functionCollector.createFunction("m_calldatacopy", [&]() {
		return R"(
			function m_calldatacopy(mobj, from_offset, size) {
				calldatacopy(m_absolute(mobj, 0), from_offset, size)
			}
		)";
	});

	_functionCollector.createFunction("m_codecopy", [&]() {
		return R"(
			function m_codecopy(mobj, from_offset, size) {
				codecopy(m_absolute(mobj, 0), from_offset, size)
			}
		)";
	});

	_functionCollector.createFunction("m_extcodecopy", [&]() {
		return R"(
			function m_extcodecopy(addr, mobj, from_offset, size) {
				extcodecopy(addr, m_absolute(mobj, 0), from_offset, size)
			}
		)";
	});

	_functionCollector.createFunction("m_returndatacopy", [&]() {
		return R"(
			function m_returndatacopy(mobj, from_offset, size) {
				returndatacopy(m_absolute(mobj, 0), from_offset, size)
			}
		)";
	});

	_functionCollector.createFunction("m_create", [&]() {
		return R"(
			function m_create(value, mobj, size) {
				create(value, m_absolute(mobj, 0), size)
			}
		)";
	});

	_functionCollector.createFunction("m_create2", [&]() {
		return R"(
			function m_create2(value, mobj, size, salt) {
				create2(value, m_absolute(mobj, 0), size, salt)
			}
		)";
	});
	_functionCollector.createFunction("m_call", [&]() {
		return R"(
			function m_call(gasAllowance, addr, value, mobjInput, inputSize, mobjOutput, outputSize) -> success {
				success := call(gasAllowance, addr, value, m_absolute(mobjInput, 0), inputSize, m_absolute(mobjOutput, 0), outputSize)
			}
		)";
	});
	_functionCollector.createFunction("m_callcode", [&]() {
		return R"(
			function m_callcode(gasAllowance, addr, value, mobjInput, inputSize, mobjOutput, outputSize) -> success {
				success := callcode(gasAllowance, addr, value, m_absolute(mobjInput, 0), inputSize, m_absolute(mobjOutput, 0), outputSize)
			}
		)";
	});
	_functionCollector.createFunction("m_delegatecall", [&]() {
		return R"(
			function m_delegatecall(gasAllowance, addr, mobjInput, inputSize, mobjOutput, outputSize) -> success {
				success := delegatecall(gasAllowance, addr, m_absolute(mobjInput, 0), inputSize, m_absolute(mobjOutput, 0), outputSize)
			}
		)";
	});
	_functionCollector.createFunction("m_staticcall", [&]() {
		return R"(
			function m_staticcall(gasAllowance, addr, mobjInput, inputSize, mobjOutput, outputSize) -> success {
				success := staticcall(gasAllowance, addr, m_absolute(mobjInput, 0), inputSize, m_absolute(mobjOutput, 0), outputSize)
			}
		)";
	});
	_functionCollector.createFunction("m_return", [&]() {
		return R"(
			function m_return(mobj, size) {
				return(m_absolute(mobj, 0), size)
			}
		)";
	});
	_functionCollector.createFunction("m_revert", [&]() {
		return R"(
			function m_revert(mobj, size) {
				revert(m_absolute(mobj, 0), size)
			}
		)";
	});
	_functionCollector.createFunction("m_log0", [&]() {
		return R"(
			function m_log0(mobj, size) {
				m_log0(m_absolute(mobj, 0), size)
			}
		)";
	});
	_functionCollector.createFunction("m_log0", [&]() {
		return R"(
			function m_log0(mobj, size) {
				m_log0(m_absolute(mobj, 0), size)
			}
		)";
	});
	_functionCollector.createFunction("m_log1", [&]() {
		return R"(
			function m_log1(mobj, size, topic1) {
				m_log1(m_absolute(mobj, 0), size, topic1)
			}
		)";
	});
	_functionCollector.createFunction("m_log2", [&]() {
		return R"(
			function m_log2(mobj, size, topic1, topic2) {
				m_log2(m_absolute(mobj, 0), size, topic1, topic2)
			}
		)";
	});
	_functionCollector.createFunction("m_log3", [&]() {
		return R"(
			function m_log3(mobj, size, topic1, topic2, topic3) {
				m_log3(m_absolute(mobj, 0), size, topic1, topic2, topic3)
			}
		)";
	});
	_functionCollector.createFunction("m_log4", [&]() {
		return R"(
			function m_log4(mobj, size, topic1, topic2, topic3, topic4) {
				m_log4(m_absolute(mobj, 0), size, topic1, topic2, topic3, topic4)
			}
		)";
	});
}
