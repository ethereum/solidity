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
 * Component that can generate various useful Yul functions.
 */

#include <libsolidity/codegen/YulUtilFunctions.h>

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libdevcore/Whiskers.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

string YulUtilFunctions::combineExternalFunctionIdFunction()
{
	string functionName = "combine_external_function_id";
	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(addr, selector) -> combined {
				combined := <shl64>(or(<shl32>(addr), and(selector, 0xffffffff)))
			}
		)")
		("functionName", functionName)
		("shl32", shiftLeftFunction(32))
		("shl64", shiftLeftFunction(64))
		.render();
	});
}

string YulUtilFunctions::splitExternalFunctionIdFunction()
{
	string functionName = "split_external_function_id";
	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(combined) -> addr, selector {
				combined := <shr64>(combined)
				selector := and(combined, 0xffffffff)
				addr := <shr32>(combined)
			}
		)")
		("functionName", functionName)
		("shr32", shiftRightFunction(32))
		("shr64", shiftRightFunction(64))
		.render();
	});
}

string YulUtilFunctions::copyToMemoryFunction(bool _fromCalldata)
{
	string functionName = "copy_" + string(_fromCalldata ? "calldata" : "memory") + "_to_memory";
	return m_functionCollector->createFunction(functionName, [&]() {
		if (_fromCalldata)
		{
			return Whiskers(R"(
				function <functionName>(src, dst, length) {
					calldatacopy(dst, src, length)
					// clear end
					mstore(add(dst, length), 0)
				}
			)")
			("functionName", functionName)
			.render();
		}
		else
		{
			return Whiskers(R"(
				function <functionName>(src, dst, length) {
					let i := 0
					for { } lt(i, length) { i := add(i, 32) }
					{
						mstore(add(dst, i), mload(add(src, i)))
					}
					if gt(i, length)
					{
						// clear end
						mstore(add(dst, length), 0)
					}
				}
			)")
			("functionName", functionName)
			.render();
		}
	});
}

string YulUtilFunctions::leftAlignFunction(Type const& _type)
{
	string functionName = string("leftAlign_") + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(value) -> aligned {
				<body>
			}
		)");
		templ("functionName", functionName);
		switch (_type.category())
		{
		case Type::Category::Address:
			templ("body", "aligned := " + leftAlignFunction(IntegerType(160)) + "(value)");
			break;
		case Type::Category::Integer:
		{
			IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
			if (type.numBits() == 256)
				templ("body", "aligned := value");
			else
				templ("body", "aligned := " + shiftLeftFunction(256 - type.numBits()) + "(value)");
			break;
		}
		case Type::Category::RationalNumber:
			solAssert(false, "Left align requested for rational number.");
			break;
		case Type::Category::Bool:
			templ("body", "aligned := " + leftAlignFunction(IntegerType(8)) + "(value)");
			break;
		case Type::Category::FixedPoint:
			solUnimplemented("Fixed point types not implemented.");
			break;
		case Type::Category::Array:
		case Type::Category::Struct:
			solAssert(false, "Left align requested for non-value type.");
			break;
		case Type::Category::FixedBytes:
			templ("body", "aligned := value");
			break;
		case Type::Category::Contract:
			templ("body", "aligned := " + leftAlignFunction(AddressType::address()) + "(value)");
			break;
		case Type::Category::Enum:
		{
			unsigned storageBytes = dynamic_cast<EnumType const&>(_type).storageBytes();
			templ("body", "aligned := " + leftAlignFunction(IntegerType(8 * storageBytes)) + "(value)");
			break;
		}
		case Type::Category::InaccessibleDynamic:
			solAssert(false, "Left align requested for inaccessible dynamic type.");
			break;
		default:
			solAssert(false, "Left align of type " + _type.identifier() + " requested.");
		}

		return templ.render();
	});
}

string YulUtilFunctions::shiftLeftFunction(size_t _numBits)
{
	solAssert(_numBits < 256, "");

	string functionName = "shift_left_" + to_string(_numBits);
	if (m_evmVersion.hasBitwiseShifting())
	{
		return m_functionCollector->createFunction(functionName, [&]() {
			return
				Whiskers(R"(
				function <functionName>(value) -> newValue {
					newValue := shl(<numBits>, value)
				}
				)")
				("functionName", functionName)
				("numBits", to_string(_numBits))
				.render();
		});
	}
	else
	{
		return m_functionCollector->createFunction(functionName, [&]() {
			return
				Whiskers(R"(
				function <functionName>(value) -> newValue {
					newValue := mul(value, <multiplier>)
				}
				)")
				("functionName", functionName)
				("multiplier", toCompactHexWithPrefix(u256(1) << _numBits))
				.render();
		});
	}
}

string YulUtilFunctions::shiftRightFunction(size_t _numBits)
{
	solAssert(_numBits < 256, "");

	// Note that if this is extended with signed shifts,
	// the opcodes SAR and SDIV behave differently with regards to rounding!

	string functionName = "shift_right_" + to_string(_numBits) + "_unsigned";
	if (m_evmVersion.hasBitwiseShifting())
	{
		return m_functionCollector->createFunction(functionName, [&]() {
			return
				Whiskers(R"(
				function <functionName>(value) -> newValue {
					newValue := shr(<numBits>, value)
				}
				)")
				("functionName", functionName)
				("numBits", to_string(_numBits))
				.render();
		});
	}
	else
	{
		return m_functionCollector->createFunction(functionName, [&]() {
			return
				Whiskers(R"(
				function <functionName>(value) -> newValue {
					newValue := div(value, <multiplier>)
				}
				)")
				("functionName", functionName)
				("multiplier", toCompactHexWithPrefix(u256(1) << _numBits))
				.render();
		});
	}
}

string YulUtilFunctions::roundUpFunction()
{
	string functionName = "round_up_to_mul_of_32";
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(value) -> result {
				result := and(add(value, 31), not(31))
			}
			)")
			("functionName", functionName)
			.render();
	});
}

string YulUtilFunctions::arrayLengthFunction(ArrayType const& _type)
{
	string functionName = "array_length_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers w(R"(
			function <functionName>(value) -> length {
				<body>
			}
		)");
		w("functionName", functionName);
		string body;
		if (!_type.isDynamicallySized())
			body = "length := " + toCompactHexWithPrefix(_type.length());
		else
		{
			switch (_type.location())
			{
			case DataLocation::CallData:
				solAssert(false, "called regular array length function on calldata array");
				break;
			case DataLocation::Memory:
				body = "length := mload(value)";
				break;
			case DataLocation::Storage:
				if (_type.isByteArray())
				{
					// Retrieve length both for in-place strings and off-place strings:
					// Computes (x & (0x100 * (ISZERO (x & 1)) - 1)) / 2
					// i.e. for short strings (x & 1 == 0) it does (x & 0xff) / 2 and for long strings it
					// computes (x & (-1)) / 2, which is equivalent to just x / 2.
					body = R"(
						length := sload(value)
						let mask := sub(mul(0x100, iszero(and(length, 1))), 1)
						length := div(and(length, mask), 2)
					)";
				}
				else
					body = "length := sload(value)";
				break;
			}
		}
		solAssert(!body.empty(), "");
		w("body", body);
		return w.render();
	});
}

string YulUtilFunctions::arrayAllocationSizeFunction(ArrayType const& _type)
{
	solAssert(_type.dataStoredIn(DataLocation::Memory), "");
	string functionName = "array_allocation_size_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers w(R"(
			function <functionName>(length) -> size {
				// Make sure we can allocate memory without overflow
				if gt(length, 0xffffffffffffffff) { revert(0, 0) }
				size := <allocationSize>
				<addLengthSlot>
			}
		)");
		w("functionName", functionName);
		if (_type.isByteArray())
			// Round up
			w("allocationSize", "and(add(length, 0x1f), not(0x1f))");
		else
			w("allocationSize", "mul(length, 0x20)");
		if (_type.isDynamicallySized())
			w("addLengthSlot", "size := add(size, 0x20)");
		else
			w("addLengthSlot", "");
		return w.render();
	});
}

string YulUtilFunctions::arrayDataAreaFunction(ArrayType const& _type)
{
	string functionName = "array_dataslot_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		if (_type.dataStoredIn(DataLocation::Memory))
		{
			if (_type.isDynamicallySized())
				return Whiskers(R"(
					function <functionName>(memPtr) -> dataPtr {
						dataPtr := add(memPtr, 0x20)
					}
				)")
				("functionName", functionName)
				.render();
			else
				return Whiskers(R"(
					function <functionName>(memPtr) -> dataPtr {
						dataPtr := memPtr
					}
				)")
				("functionName", functionName)
				.render();
		}
		else if (_type.dataStoredIn(DataLocation::Storage))
		{
			if (_type.isDynamicallySized())
			{
				Whiskers w(R"(
					function <functionName>(slot) -> dataSlot {
						mstore(0, slot)
						dataSlot := keccak256(0, 0x20)
					}
				)");
				w("functionName", functionName);
				return w.render();
			}
			else
			{
				Whiskers w(R"(
					function <functionName>(slot) -> dataSlot {
						dataSlot := slot
					}
				)");
				w("functionName", functionName);
				return w.render();
			}
		}
		else
		{
			// Not used for calldata
			solAssert(false, "");
		}
	});
}

string YulUtilFunctions::nextArrayElementFunction(ArrayType const& _type)
{
	solAssert(!_type.isByteArray(), "");
	solAssert(
		_type.location() == DataLocation::Memory ||
		_type.location() == DataLocation::Storage,
		""
	);
	solAssert(
		_type.location() == DataLocation::Memory ||
		_type.baseType()->storageBytes() > 16,
		""
	);
	string functionName = "array_nextElement_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		if (_type.location() == DataLocation::Memory)
			return Whiskers(R"(
				function <functionName>(memPtr) -> nextPtr {
					nextPtr := add(memPtr, 0x20)
				}
			)")
			("functionName", functionName)
			.render();
		else if (_type.location() == DataLocation::Storage)
			return Whiskers(R"(
				function <functionName>(slot) -> nextSlot {
					nextSlot := add(slot, 1)
				}
			)")
			("functionName", functionName)
			.render();
		else
			solAssert(false, "");
	});
}

string YulUtilFunctions::allocationFunction()
{
	string functionName = "allocateMemory";
	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(size) -> memPtr {
				memPtr := mload(<freeMemoryPointer>)
				let newFreePtr := add(memPtr, size)
				// protect against overflow
				if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { revert(0, 0) }
				mstore(<freeMemoryPointer>, newFreePtr)
			}
		)")
		("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer))
		("functionName", functionName)
		.render();
	});
}

