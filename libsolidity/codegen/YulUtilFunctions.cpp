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
#include <libdevcore/StringUtils.h>

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

string YulUtilFunctions::requireOrAssertFunction(bool _assert, Type const* _messageType)
{
	string functionName =
		string(_assert ? "assert_helper" : "require_helper") +
		(_messageType ? ("_" + _messageType->identifier()) : "");

	solAssert(!_assert || !_messageType, "Asserts can't have messages!");

	return m_functionCollector->createFunction(functionName, [&]() {
		if (!_messageType)
			return Whiskers(R"(
				function <functionName>(condition) {
					if iszero(condition) { <invalidOrRevert> }
				}
			)")
			("invalidOrRevert", _assert ? "invalid()" : "revert(0, 0)")
			("functionName", functionName)
			.render();

		int const hashHeaderSize = 4;
		int const byteSize = 8;
		u256 const errorHash =
			u256(FixedHash<hashHeaderSize>::Arith(
				FixedHash<hashHeaderSize>(dev::keccak256("Error(string)"))
			)) << (256 - hashHeaderSize * byteSize);

		string const encodeFunc = ABIFunctions(m_evmVersion, m_functionCollector)
			.tupleEncoder(
				{_messageType},
				{TypeProvider::stringMemory()}
			);

		return Whiskers(R"(
			function <functionName>(condition <messageVars>) {
				if iszero(condition) {
					let fmp := mload(<freeMemPointer>)
					mstore(fmp, <errorHash>)
					let end := <abiEncodeFunc>(add(fmp, <hashHeaderSize>) <messageVars>)
					revert(fmp, sub(end, fmp))
				}
			}
		)")
		("functionName", functionName)
		("freeMemPointer", to_string(CompilerUtils::freeMemoryPointer))
		("errorHash", formatNumber(errorHash))
		("abiEncodeFunc", encodeFunc)
		("hashHeaderSize", to_string(hashHeaderSize))
		("messageVars",
			(_messageType->sizeOnStack() > 0 ? ", " : "") +
			suffixedVariableNameList("message_", 1, 1 + _messageType->sizeOnStack())
		)
		.render();
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
			templ("body", "aligned := " + leftAlignFunction(*TypeProvider::address()) + "(value)");
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
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(value) -> newValue {
				newValue :=
				<?hasShifts>
					shl(<numBits>, value)
				<!hasShifts>
					mul(value, <multiplier>)
				</hasShifts>
			}
			)")
			("functionName", functionName)
			("numBits", to_string(_numBits))
			("hasShifts", m_evmVersion.hasBitwiseShifting())
			("multiplier", toCompactHexWithPrefix(u256(1) << _numBits))
			.render();
	});
}

string YulUtilFunctions::shiftLeftFunctionDynamic()
{
	string functionName = "shift_left_dynamic";
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(bits, value) -> newValue {
				newValue :=
				<?hasShifts>
					shl(bits, value)
				<!hasShifts>
					mul(value, exp(2, bits))
				</hasShifts>
			}
			)")
			("functionName", functionName)
			("hasShifts", m_evmVersion.hasBitwiseShifting())
			.render();
	});
}

string YulUtilFunctions::shiftRightFunction(size_t _numBits)
{
	solAssert(_numBits < 256, "");

	// Note that if this is extended with signed shifts,
	// the opcodes SAR and SDIV behave differently with regards to rounding!

	string functionName = "shift_right_" + to_string(_numBits) + "_unsigned";
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(value) -> newValue {
				newValue :=
				<?hasShifts>
					shr(<numBits>, value)
				<!hasShifts>
					div(value, <multiplier>)
				</hasShifts>
			}
			)")
			("functionName", functionName)
			("hasShifts", m_evmVersion.hasBitwiseShifting())
			("numBits", to_string(_numBits))
			("multiplier", toCompactHexWithPrefix(u256(1) << _numBits))
			.render();
	});
}

string YulUtilFunctions::shiftRightFunctionDynamic()
{
	// Note that if this is extended with signed shifts,
	// the opcodes SAR and SDIV behave differently with regards to rounding!

	string const functionName = "shift_right_unsigned_dynamic";
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(bits, value) -> newValue {
				newValue :=
				<?hasShifts>
					shr(bits, value)
				<!hasShifts>
					div(value, exp(2, bits))
				</hasShifts>
			}
			)")
			("functionName", functionName)
			("hasShifts", m_evmVersion.hasBitwiseShifting())
			.render();
	});
}

string YulUtilFunctions::updateByteSliceFunction(size_t _numBytes, size_t _shiftBytes)
{
	solAssert(_numBytes <= 32, "");
	solAssert(_shiftBytes <= 32, "");
	size_t numBits = _numBytes * 8;
	size_t shiftBits = _shiftBytes * 8;
	string functionName = "update_byte_slice_" + to_string(_numBytes) + "_shift_" + to_string(_shiftBytes);
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(value, toInsert) -> result {
				let mask := <mask>
				toInsert := <shl>(toInsert)
				value := and(value, not(mask))
				result := or(value, and(toInsert, mask))
			}
			)")
			("functionName", functionName)
			("mask", formatNumber(((bigint(1) << numBits) - 1) << shiftBits))
			("shl", shiftLeftFunction(shiftBits))
			.render();
	});
}

string YulUtilFunctions::updateByteSliceFunctionDynamic(size_t _numBytes)
{
	solAssert(_numBytes <= 32, "");
	size_t numBits = _numBytes * 8;
	string functionName = "update_byte_slice_dynamic" + to_string(_numBytes);
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(value, shiftBytes, toInsert) -> result {
				let shiftBits := mul(shiftBytes, 8)
				let mask := <shl>(shiftBits, <mask>)
				toInsert := <shl>(shiftBits, toInsert)
				value := and(value, not(mask))
				result := or(value, and(toInsert, mask))
			}
			)")
			("functionName", functionName)
			("mask", formatNumber((bigint(1) << numBits) - 1))
			("shl", shiftLeftFunctionDynamic())
			.render();
	});
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

string YulUtilFunctions::overflowCheckedIntAddFunction(IntegerType const& _type)
{
	string functionName = "checked_add_" + _type.identifier();
	// TODO: Consider to add a special case for unsigned 256-bit integers
	//       and use the following instead:
	//       sum := add(x, y) if lt(sum, x) { revert(0, 0) }
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> sum {
				<?signed>
					// overflow, if x >= 0 and y > (maxValue - x)
					if and(iszero(slt(x, 0)), sgt(y, sub(<maxValue>, x))) { revert(0, 0) }
					// underflow, if x < 0 and y < (minValue - x)
					if and(slt(x, 0), slt(y, sub(<minValue>, x))) { revert(0, 0) }
				<!signed>
					// overflow, if x > (maxValue - y)
					if gt(x, sub(<maxValue>, y)) { revert(0, 0) }
				</signed>
				sum := add(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("maxValue", toCompactHexWithPrefix(u256(_type.maxValue())))
			("minValue", toCompactHexWithPrefix(u256(_type.minValue())))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntMulFunction(IntegerType const& _type)
{
	string functionName = "checked_mul_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			// Multiplication by zero could be treated separately and directly return zero.
			Whiskers(R"(
			function <functionName>(x, y) -> product {
				<?signed>
					// overflow, if x > 0, y > 0 and x > (maxValue / y)
					if and(and(sgt(x, 0), sgt(y, 0)), gt(x, div(<maxValue>, y))) { revert(0, 0) }
					// underflow, if x > 0, y < 0 and y < (minValue / x)
					if and(and(sgt(x, 0), slt(y, 0)), slt(y, sdiv(<minValue>, x))) { revert(0, 0) }
					// underflow, if x < 0, y > 0 and x < (minValue / y)
					if and(and(slt(x, 0), sgt(y, 0)), slt(x, sdiv(<minValue>, y))) { revert(0, 0) }
					// overflow, if x < 0, y < 0 and x < (maxValue / y)
					if and(and(slt(x, 0), slt(y, 0)), slt(x, sdiv(<maxValue>, y))) { revert(0, 0) }
				<!signed>
					// overflow, if x != 0 and y > (maxValue / x)
					if and(iszero(iszero(x)), gt(y, div(<maxValue>, x))) { revert(0, 0) }
				</signed>
				product := mul(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("maxValue", toCompactHexWithPrefix(u256(_type.maxValue())))
			("minValue", toCompactHexWithPrefix(u256(_type.minValue())))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntDivFunction(IntegerType const& _type)
{
	string functionName = "checked_div_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> r {
				if iszero(y) { revert(0, 0) }
				<?signed>
				// overflow for minVal / -1
				if and(
					eq(x, <minVal>),
					eq(y, sub(0, 1))
				) { revert(0, 0) }
				</signed>
				r := <?signed>s</signed>div(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("minVal", toCompactHexWithPrefix(u256(_type.minValue())))
			.render();
	});
}

string YulUtilFunctions::checkedIntModFunction(IntegerType const& _type)
{
	string functionName = "checked_mod_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> r {
				if iszero(y) { revert(0, 0) }
				r := <?signed>s</signed>mod(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntSubFunction(IntegerType const& _type)
{
	string functionName = "checked_sub_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> diff {
				<?signed>
					// underflow, if y >= 0 and x < (minValue + y)
					if and(iszero(slt(y, 0)), slt(x, add(<minValue>, y))) { revert(0, 0) }
					// overflow, if y < 0 and x > (maxValue + y)
					if and(slt(y, 0), sgt(x, add(<maxValue>, y))) { revert(0, 0) }
				<!signed>
					if lt(x, y) { revert(0, 0) }
				</signed>
				diff := sub(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("maxValue", toCompactHexWithPrefix(u256(_type.maxValue())))
			("minValue", toCompactHexWithPrefix(u256(_type.minValue())))
			.render();
	});
}

string YulUtilFunctions::arrayLengthFunction(ArrayType const& _type)
{
	string functionName = "array_length_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers w(R"(
			function <functionName>(value) -> length {
				<?dynamic>
					<?memory>
						length := mload(value)
					</memory>
					<?storage>
						length := sload(value)
						<?byteArray>
							// Retrieve length both for in-place strings and off-place strings:
							// Computes (x & (0x100 * (ISZERO (x & 1)) - 1)) / 2
							// i.e. for short strings (x & 1 == 0) it does (x & 0xff) / 2 and for long strings it
							// computes (x & (-1)) / 2, which is equivalent to just x / 2.
							let mask := sub(mul(0x100, iszero(and(length, 1))), 1)
							length := div(and(length, mask), 2)
						</byteArray>
					</storage>
				<!dynamic>
					length := <length>
				</dynamic>
			}
		)");
		w("functionName", functionName);
		w("dynamic", _type.isDynamicallySized());
		if (!_type.isDynamicallySized())
			w("length", toCompactHexWithPrefix(_type.length()));
		w("memory", _type.location() == DataLocation::Memory);
		w("storage", _type.location() == DataLocation::Storage);
		w("byteArray", _type.isByteArray());
		if (_type.isDynamicallySized())
			solAssert(
				_type.location() != DataLocation::CallData,
				"called regular array length function on calldata array"
			);
		return w.render();
	});
}

std::string YulUtilFunctions::resizeDynamicArrayFunction(ArrayType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	solUnimplementedAssert(!_type.isByteArray(), "Byte Arrays not yet implemented!");
	solUnimplementedAssert(_type.baseType()->storageBytes() <= 32, "...");
	solUnimplementedAssert(_type.baseType()->storageSize() == 1, "");

	string functionName = "resize_array_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array, newLen) {
				if gt(newLen, <maxArrayLength>) {
					invalid()
				}

				let oldLen := <fetchLength>(array)

				// Store new length
				sstore(array, newLen)

				// Size was reduced, clear end of array
				if lt(newLen, oldLen) {
					let oldSlotCount := <convertToSize>(oldLen)
					let newSlotCount := <convertToSize>(newLen)
					let arrayDataStart := <dataPosition>(array)
					let deleteStart := add(arrayDataStart, newSlotCount)
					let deleteEnd := add(arrayDataStart, oldSlotCount)
					<clearStorageRange>(deleteStart, deleteEnd)
				}
			})")
			("functionName", functionName)
			("fetchLength", arrayLengthFunction(_type))
			("convertToSize", arrayConvertLengthToSize(_type))
			("dataPosition", arrayDataAreaFunction(_type))
			("clearStorageRange", clearStorageRangeFunction(*_type.baseType()))
			("maxArrayLength", (u256(1) << 64).str())
			.render();
	});
}

string YulUtilFunctions::clearStorageRangeFunction(Type const& _type)
{
	string functionName = "clear_storage_range_" + _type.identifier();

	solAssert(_type.storageBytes() >= 32, "Expected smaller value for storage bytes");

	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(start, end) {
				for {} lt(start, end) { start := add(start, <increment>) }
				{
					<setToZero>(start, 0)
				}
			}
		)")
		("functionName", functionName)
		("setToZero", storageSetToZeroFunction(_type))
		("increment", _type.storageSize().str())
		.render();
	});
}

string YulUtilFunctions::clearStorageArrayFunction(ArrayType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");

	if (_type.baseType()->storageBytes() < 32)
	{
		solAssert(_type.baseType()->isValueType(), "Invalid storage size for non-value type.");
		solAssert(_type.baseType()->storageSize() <= 1, "Invalid storage size for type.");
	}

	if (_type.baseType()->isValueType())
		solAssert(_type.baseType()->storageSize() <= 1, "Invalid size for value type.");

	string functionName = "clear_storage_array_" + _type.identifier();

	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(slot) {
				<?dynamic>
					<resizeArray>(slot, 0)
				<!dynamic>
					<clearRange>(slot, add(slot, <lenToSize>(<len>)))
				</dynamic>
			}
		)")
		("functionName", functionName)
		("dynamic", _type.isDynamicallySized())
		("resizeArray", _type.isDynamicallySized() ? resizeDynamicArrayFunction(_type) : "")
		(
			"clearRange",
			clearStorageRangeFunction(
				(_type.baseType()->storageBytes() < 32) ?
				*TypeProvider::uint256() :
				*_type.baseType()
			)
		)
		("lenToSize", arrayConvertLengthToSize(_type))
		("len", _type.length().str())
		.render();
	});
}

string YulUtilFunctions::arrayConvertLengthToSize(ArrayType const& _type)
{
	string functionName = "array_convert_length_to_size_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Type const& baseType = *_type.baseType();

		switch (_type.location())
		{
			case DataLocation::Storage:
			{
				unsigned const baseStorageBytes = baseType.storageBytes();
				solAssert(baseStorageBytes > 0, "");
				solAssert(32 / baseStorageBytes > 0, "");

				return Whiskers(R"(
					function <functionName>(length) -> size {
						size := length
						<?multiSlot>
							size := <mul>(<storageSize>, length)
						<!multiSlot>
							// Number of slots rounded up
							size := div(add(length, sub(<itemsPerSlot>, 1)), <itemsPerSlot>)
						</multiSlot>
					})")
					("functionName", functionName)
					("multiSlot", baseType.storageSize() > 1)
					("itemsPerSlot", to_string(32 / baseStorageBytes))
					("storageSize", baseType.storageSize().str())
					("mul", overflowCheckedIntMulFunction(*TypeProvider::uint256()))
					.render();
			}
			case DataLocation::CallData: // fallthrough
			case DataLocation::Memory:
				return Whiskers(R"(
					function <functionName>(length) -> size {
						<?byteArray>
							size := length
						<!byteArray>
							size := <mul>(length, <stride>)
						</byteArray>
					})")
					("functionName", functionName)
					("stride", to_string(_type.location() == DataLocation::Memory ? _type.memoryStride() : _type.calldataStride()))
					("byteArray", _type.isByteArray())
					("mul", overflowCheckedIntMulFunction(*TypeProvider::uint256()))
					.render();
			default:
				solAssert(false, "");
		}

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
				<?byteArray>
					// round up
					size := and(add(length, 0x1f), not(0x1f))
				<!byteArray>
					size := mul(length, 0x20)
				</byteArray>
				<?dynamic>
					// add length slot
					size := add(size, 0x20)
				</dynamic>
			}
		)");
		w("functionName", functionName);
		w("byteArray", _type.isByteArray());
		w("dynamic", _type.isDynamicallySized());
		return w.render();
	});
}

string YulUtilFunctions::arrayDataAreaFunction(ArrayType const& _type)
{
	string functionName = "array_dataslot_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		// No special processing for calldata arrays, because they are stored as
		// offset of the data area and length on the stack, so the offset already
		// points to the data area.
		// This might change, if calldata arrays are stored in a single
		// stack slot at some point.
		return Whiskers(R"(
			function <functionName>(ptr) -> data {
				data := ptr
				<?dynamic>
					<?memory>
						data := add(ptr, 0x20)
					</memory>
					<?storage>
						mstore(0, ptr)
						data := keccak256(0, 0x20)
					</storage>
				</dynamic>
			}
		)")
		("functionName", functionName)
		("dynamic", _type.isDynamicallySized())
		("memory", _type.location() == DataLocation::Memory)
		("storage", _type.location() == DataLocation::Storage)
		.render();
	});
}

string YulUtilFunctions::storageArrayIndexAccessFunction(ArrayType const& _type)
{
	solUnimplementedAssert(_type.baseType()->storageBytes() > 16, "");

	string functionName = "storage_array_index_access_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array, index) -> slot, offset {
				if iszero(lt(index, <arrayLen>(array))) {
					invalid()
				}

				let data := <dataAreaFunc>(array)
				<?multipleItemsPerSlot>

				<!multipleItemsPerSlot>
					slot := add(data, mul(index, <storageSize>))
					offset := 0
				</multipleItemsPerSlot>
			}
		)")
		("functionName", functionName)
		("arrayLen", arrayLengthFunction(_type))
		("dataAreaFunc", arrayDataAreaFunction(_type))
		("multipleItemsPerSlot", _type.baseType()->storageBytes() <= 16)
		("storageSize", _type.baseType()->storageSize().str())
		.render();
	});
}

string YulUtilFunctions::memoryArrayIndexAccessFunction(ArrayType const& _type)
{
	string functionName = "memory_array_index_access_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(baseRef, index) -> addr {
				if iszero(lt(index, <arrayLen>(baseRef))) {
					invalid()
				}

				let offset := mul(index, <stride>)
				<?dynamicallySized>
					offset := add(offset, 32)
				</dynamicallySized>
				addr := add(baseRef, offset)
			}
		)")
		("functionName", functionName)
		("arrayLen", arrayLengthFunction(_type))
		("stride", to_string(_type.memoryStride()))
		("dynamicallySized", _type.isDynamicallySized())
		.render();
	});
}

string YulUtilFunctions::nextArrayElementFunction(ArrayType const& _type)
{
	solAssert(!_type.isByteArray(), "");
	if (_type.dataStoredIn(DataLocation::Storage))
		solAssert(_type.baseType()->storageBytes() > 16, "");
	string functionName = "array_nextElement_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(ptr) -> next {
				next := add(ptr, <advance>)
			}
		)");
		templ("functionName", functionName);
		switch (_type.location())
		{
		case DataLocation::Memory:
			templ("advance", "0x20");
			break;
		case DataLocation::Storage:
		{
			u256 size = _type.baseType()->storageSize();
			solAssert(size >= 1, "");
			templ("advance", toCompactHexWithPrefix(size));
			break;
		}
		case DataLocation::CallData:
		{
			u256 size = _type.calldataStride();
			solAssert(size >= 32 && size % 32 == 0, "");
			templ("advance", toCompactHexWithPrefix(size));
			break;
		}
		}
		return templ.render();
	});
}

string YulUtilFunctions::mappingIndexAccessFunction(MappingType const& _mappingType, Type const& _keyType)
{
	solAssert(_keyType.sizeOnStack() <= 1, "");

	string functionName = "mapping_index_access_" + _mappingType.identifier() + "_of_" + _keyType.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		if (_mappingType.keyType()->isDynamicallySized())
			return Whiskers(R"(
				function <functionName>(slot <comma> <key>) -> dataSlot {
					dataSlot := <hash>(slot <comma> <key>)
				}
			)")
			("functionName", functionName)
			("key", _keyType.sizeOnStack() > 0 ? "key" : "")
			("comma", _keyType.sizeOnStack() > 0 ? "," : "")
			("hash", packedHashFunction(
				{&_keyType, TypeProvider::uint256()},
				{_mappingType.keyType(), TypeProvider::uint256()}
			))
			.render();
		else
		{
			solAssert(CompilerUtils::freeMemoryPointer >= 0x40, "");
			solAssert(!_mappingType.keyType()->isDynamicallyEncoded(), "");
			solAssert(_mappingType.keyType()->calldataEncodedSize(false) <= 0x20, "");
			Whiskers templ(R"(
				function <functionName>(slot <key>) -> dataSlot {
					mstore(0, <convertedKey>)
					mstore(0x20, slot)
					dataSlot := keccak256(0, 0x40)
				}
			)");
			templ("functionName", functionName);
			templ("key", _keyType.sizeOnStack() == 1 ? ", key" : "");
			if (_keyType.sizeOnStack() == 0)
				templ("convertedKey", conversionFunction(_keyType, *_mappingType.keyType()) + "()");
			else
				templ("convertedKey", conversionFunction(_keyType, *_mappingType.keyType()) + "(key)");
			return templ.render();
		}
	});
}

string YulUtilFunctions::readFromStorage(Type const& _type, size_t _offset, bool _splitFunctionTypes)
{
	solUnimplementedAssert(!_splitFunctionTypes, "");
	string functionName =
		"read_from_storage_" +
		string(_splitFunctionTypes ? "split_" : "") +
		"offset_" +
		to_string(_offset) +
		"_" +
		_type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		solAssert(_type.sizeOnStack() == 1, "");
		return Whiskers(R"(
			function <functionName>(slot) -> value {
				value := <extract>(sload(slot))
			}
		)")
		("functionName", functionName)
		("extract", extractFromStorageValue(_type, _offset, false))
		.render();
	});
}

string YulUtilFunctions::readFromStorageDynamic(Type const& _type, bool _splitFunctionTypes)
{
	solUnimplementedAssert(!_splitFunctionTypes, "");
	string functionName =
		"read_from_storage_dynamic" +
		string(_splitFunctionTypes ? "split_" : "") +
		"_" +
		_type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		solAssert(_type.sizeOnStack() == 1, "");
		return Whiskers(R"(
			function <functionName>(slot, offset) -> value {
				value := <extract>(sload(slot), offset)
			}
		)")
		("functionName", functionName)
		("extract", extractFromStorageValueDynamic(_type, _splitFunctionTypes))
		.render();
	});
}

string YulUtilFunctions::readFromMemory(Type const& _type)
{
	return readFromMemoryOrCalldata(_type, false);
}

string YulUtilFunctions::readFromCalldata(Type const& _type)
{
	return readFromMemoryOrCalldata(_type, true);
}

string YulUtilFunctions::updateStorageValueFunction(Type const& _type, boost::optional<unsigned> const& _offset)
{
	string const functionName =
		"update_storage_value_" +
		(_offset.is_initialized() ? ("offset_" + to_string(*_offset)) : "") +
		_type.identifier();

	return m_functionCollector->createFunction(functionName, [&] {
		if (_type.isValueType())
		{
			solAssert(_type.storageBytes() <= 32, "Invalid storage bytes size.");
			solAssert(_type.storageBytes() > 0, "Invalid storage bytes size.");

			return Whiskers(R"(
				function <functionName>(slot, <offset>value) {
					sstore(slot, <update>(sload(slot), <offset><prepare>(value)))
				}

			)")
			("functionName", functionName)
			("update",
				_offset.is_initialized() ?
					updateByteSliceFunction(_type.storageBytes(), *_offset) :
					updateByteSliceFunctionDynamic(_type.storageBytes())
			)
			("offset", _offset.is_initialized() ? "" : "offset, ")
			("prepare", prepareStoreFunction(_type))
			.render();
		}
		else
		{
			if (_type.category() == Type::Category::Array)
				solUnimplementedAssert(false, "");
			else if (_type.category() == Type::Category::Struct)
				solUnimplementedAssert(false, "");
			else
				solAssert(false, "Invalid non-value type for assignment.");
		}
	});
}

string YulUtilFunctions::writeToMemoryFunction(Type const& _type)
{
	string const functionName =
		string("write_to_memory_") +
		_type.identifier();

	return m_functionCollector->createFunction(functionName, [&] {
		solAssert(!dynamic_cast<StringLiteralType const*>(&_type), "");
		if (auto ref = dynamic_cast<ReferenceType const*>(&_type))
		{
			solAssert(
				ref->location() == DataLocation::Memory,
				"Can only update types with location memory."
			);

			return Whiskers(R"(
				function <functionName>(memPtr, value) {
					mstore(memPtr, value)
			}
			)")
			("functionName", functionName)
			.render();
		}
		else if (
			_type.category() == Type::Category::Function &&
			dynamic_cast<FunctionType const&>(_type).kind() == FunctionType::Kind::External
		)
		{
			return Whiskers(R"(
				function <functionName>(memPtr, addr, selector) {
					mstore(memPtr, <combine>(addr, selector))
				}
			)")
			("functionName", functionName)
			("combine", combineExternalFunctionIdFunction())
			.render();
		}
		else if (_type.isValueType())
		{
			return Whiskers(R"(
				function <functionName>(memPtr, value) {
					mstore(memPtr, <cleanup>(value))
			}
			)")
			("functionName", functionName)
			("cleanup", cleanupFunction(_type))
			.render();
		}
		else // Should never happen
		{
			solAssert(
				false,
				"Memory store of type " + _type.toString(true) + " not allowed."
			);
		}
	});
}

string YulUtilFunctions::extractFromStorageValueDynamic(Type const& _type, bool _splitFunctionTypes)
{
	solUnimplementedAssert(!_splitFunctionTypes, "");

	string functionName =
		"extract_from_storage_value_dynamic" +
		string(_splitFunctionTypes ? "split_" : "") +
		_type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		return Whiskers(R"(
			function <functionName>(slot_value, offset) -> value {
				value := <cleanupStorage>(<shr>(mul(offset, 8), slot_value))
			}
		)")
		("functionName", functionName)
		("shr", shiftRightFunctionDynamic())
		("cleanupStorage", cleanupFromStorageFunction(_type, _splitFunctionTypes))
		.render();
	});
}

string YulUtilFunctions::extractFromStorageValue(Type const& _type, size_t _offset, bool _splitFunctionTypes)
{
	solUnimplementedAssert(!_splitFunctionTypes, "");

	string functionName =
		"extract_from_storage_value_" +
		string(_splitFunctionTypes ? "split_" : "") +
		"offset_" +
		to_string(_offset) +
		_type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		return Whiskers(R"(
			function <functionName>(slot_value) -> value {
				value := <cleanupStorage>(<shr>(slot_value))
			}
		)")
		("functionName", functionName)
		("shr", shiftRightFunction(_offset * 8))
		("cleanupStorage", cleanupFromStorageFunction(_type, _splitFunctionTypes))
		.render();
	});
}

string YulUtilFunctions::cleanupFromStorageFunction(Type const& _type, bool _splitFunctionTypes)
{
	solAssert(_type.isValueType(), "");
	solUnimplementedAssert(!_splitFunctionTypes, "");

	string functionName = string("cleanup_from_storage_") + (_splitFunctionTypes ? "split_" : "") + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&] {
		Whiskers templ(R"(
			function <functionName>(value) -> cleaned {
				cleaned := <cleaned>
			}
		)");
		templ("functionName", functionName);

		unsigned storageBytes = _type.storageBytes();
		if (IntegerType const* type = dynamic_cast<IntegerType const*>(&_type))
			if (type->isSigned() && storageBytes != 32)
			{
				templ("cleaned", "signextend(" + to_string(storageBytes - 1) + ", value)");
				return templ.render();
			}

		if (storageBytes == 32)
			templ("cleaned", "value");
		else if (_type.leftAligned())
			templ("cleaned", shiftLeftFunction(256 - 8 * storageBytes) + "(value)");
		else
			templ("cleaned", "and(value, " + toCompactHexWithPrefix((u256(1) << (8 * storageBytes)) - 1) + ")");

		return templ.render();
	});
}

string YulUtilFunctions::prepareStoreFunction(Type const& _type)
{
	solUnimplementedAssert(_type.category() != Type::Category::Function, "");

	string functionName = "prepare_store_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(value) -> ret {
				ret := <actualPrepare>
			}
		)");
		templ("functionName", functionName);
		if (_type.category() == Type::Category::FixedBytes)
			templ("actualPrepare", shiftRightFunction(256 - 8 * _type.storageBytes()) + "(value)");
		else
			templ("actualPrepare", "value");
		return templ.render();
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

string YulUtilFunctions::allocateMemoryArrayFunction(ArrayType const& _type)
{
	solUnimplementedAssert(!_type.isByteArray(), "");

	string functionName = "allocate_memory_array_" + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(length) -> memPtr {
				memPtr := <alloc>(<allocSize>(length))
				<?dynamic>
				mstore(memPtr, length)
				</dynamic>
			}
		)")
		("functionName", functionName)
		("alloc", allocationFunction())
		("allocSize", arrayAllocationSizeFunction(_type))
		("dynamic", _type.isDynamicallySized())
		.render();
	});
}

string YulUtilFunctions::conversionFunction(Type const& _from, Type const& _to)
{
	if (_from.sizeOnStack() != 1 || _to.sizeOnStack() != 1)
		return conversionFunctionSpecial(_from, _to);

	string functionName =
		"convert_" +
		_from.identifier() +
		"_to_" +
		_to.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(value) -> converted {
				<body>
			}
		)");
		templ("functionName", functionName);
		string body;
		auto toCategory = _to.category();
		auto fromCategory = _from.category();
		switch (fromCategory)
		{
		case Type::Category::Address:
			body =
				Whiskers("converted := <convert>(value)")
					("convert", conversionFunction(IntegerType(160), _to))
					.render();
			break;
		case Type::Category::Integer:
		case Type::Category::RationalNumber:
		case Type::Category::Contract:
		{
			if (RationalNumberType const* rational = dynamic_cast<RationalNumberType const*>(&_from))
				solUnimplementedAssert(!rational->isFractional(), "Not yet implemented - FixedPointType.");
			if (toCategory == Type::Category::FixedBytes)
			{
				solAssert(
					fromCategory == Type::Category::Integer || fromCategory == Type::Category::RationalNumber,
					"Invalid conversion to FixedBytesType requested."
				);
				FixedBytesType const& toBytesType = dynamic_cast<FixedBytesType const&>(_to);
				body =
					Whiskers("converted := <shiftLeft>(<clean>(value))")
						("shiftLeft", shiftLeftFunction(256 - toBytesType.numBytes() * 8))
						("clean", cleanupFunction(_from))
						.render();
			}
			else if (toCategory == Type::Category::Enum)
			{
				solAssert(_from.mobileType(), "");
				body =
					Whiskers("converted := <cleanEnum>(<cleanInt>(value))")
					("cleanEnum", cleanupFunction(_to))
					// "mobileType()" returns integer type for rational
					("cleanInt", cleanupFunction(*_from.mobileType()))
					.render();
			}
			else if (toCategory == Type::Category::FixedPoint)
				solUnimplemented("Not yet implemented - FixedPointType.");
			else if (toCategory == Type::Category::Address)
				body =
					Whiskers("converted := <convert>(value)")
						("convert", conversionFunction(_from, IntegerType(160)))
						.render();
			else
			{
				solAssert(
					toCategory == Type::Category::Integer ||
					toCategory == Type::Category::Contract,
				"");
				IntegerType const addressType(160);
				IntegerType const& to =
					toCategory == Type::Category::Integer ?
					dynamic_cast<IntegerType const&>(_to) :
					addressType;

				// Clean according to the "to" type, except if this is
				// a widening conversion.
				IntegerType const* cleanupType = &to;
				if (fromCategory != Type::Category::RationalNumber)
				{
					IntegerType const& from =
						fromCategory == Type::Category::Integer ?
						dynamic_cast<IntegerType const&>(_from) :
						addressType;
					if (to.numBits() > from.numBits())
						cleanupType = &from;
				}
				body =
					Whiskers("converted := <cleanInt>(value)")
					("cleanInt", cleanupFunction(*cleanupType))
					.render();
			}
			break;
		}
		case Type::Category::Bool:
		{
			solAssert(_from == _to, "Invalid conversion for bool.");
			body =
				Whiskers("converted := <clean>(value)")
				("clean", cleanupFunction(_from))
				.render();
			break;
		}
		case Type::Category::FixedPoint:
			solUnimplemented("Fixed point types not implemented.");
			break;
		case Type::Category::Array:
		{
			bool equal = _from == _to;

			if (!equal)
			{
				ArrayType const& from = dynamic_cast<decltype(from)>(_from);
				ArrayType const& to = dynamic_cast<decltype(to)>(_to);

				if (*from.mobileType() == *to.mobileType())
					equal = true;
			}

			if (equal)
				body = "converted := value";
			else
				solUnimplementedAssert(false, "Array conversion not implemented.");

			break;
		}
		case Type::Category::Struct:
			solUnimplementedAssert(false, "Struct conversion not implemented.");
			break;
		case Type::Category::FixedBytes:
		{
			FixedBytesType const& from = dynamic_cast<FixedBytesType const&>(_from);
			if (toCategory == Type::Category::Integer)
				body =
					Whiskers("converted := <convert>(<shift>(value))")
					("shift", shiftRightFunction(256 - from.numBytes() * 8))
					("convert", conversionFunction(IntegerType(from.numBytes() * 8), _to))
					.render();
			else if (toCategory == Type::Category::Address)
				body =
					Whiskers("converted := <convert>(value)")
						("convert", conversionFunction(_from, IntegerType(160)))
						.render();
			else
			{
				// clear for conversion to longer bytes
				solAssert(toCategory == Type::Category::FixedBytes, "Invalid type conversion requested.");
				body =
					Whiskers("converted := <clean>(value)")
					("clean", cleanupFunction(from))
					.render();
			}
			break;
		}
		case Type::Category::Function:
		{
			solAssert(false, "Conversion should not be called for function types.");
			break;
		}
		case Type::Category::Enum:
		{
			solAssert(toCategory == Type::Category::Integer || _from == _to, "");
			EnumType const& enumType = dynamic_cast<decltype(enumType)>(_from);
			body =
				Whiskers("converted := <clean>(value)")
				("clean", cleanupFunction(enumType))
				.render();
			break;
		}
		case Type::Category::Tuple:
		{
			solUnimplementedAssert(false, "Tuple conversion not implemented.");
			break;
		}
		default:
			solAssert(false, "");
		}

		solAssert(!body.empty(), _from.canonicalName() + " to " + _to.canonicalName());
		templ("body", body);
		return templ.render();
	});
}

string YulUtilFunctions::cleanupFunction(Type const& _type)
{
	string functionName = string("cleanup_") + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(value) -> cleaned {
				<body>
			}
		)");
		templ("functionName", functionName);
		switch (_type.category())
		{
		case Type::Category::Address:
			templ("body", "cleaned := " + cleanupFunction(IntegerType(160)) + "(value)");
			break;
		case Type::Category::Integer:
		{
			IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
			if (type.numBits() == 256)
				templ("body", "cleaned := value");
			else if (type.isSigned())
				templ("body", "cleaned := signextend(" + to_string(type.numBits() / 8 - 1) + ", value)");
			else
				templ("body", "cleaned := and(value, " + toCompactHexWithPrefix((u256(1) << type.numBits()) - 1) + ")");
			break;
		}
		case Type::Category::RationalNumber:
			templ("body", "cleaned := value");
			break;
		case Type::Category::Bool:
			templ("body", "cleaned := iszero(iszero(value))");
			break;
		case Type::Category::FixedPoint:
			solUnimplemented("Fixed point types not implemented.");
			break;
		case Type::Category::Function:
			solAssert(dynamic_cast<FunctionType const&>(_type).kind() == FunctionType::Kind::External, "");
			templ("body", "cleaned := " + cleanupFunction(FixedBytesType(24)) + "(value)");
			break;
		case Type::Category::Array:
		case Type::Category::Struct:
		case Type::Category::Mapping:
			solAssert(_type.dataStoredIn(DataLocation::Storage), "Cleanup requested for non-storage reference type.");
			templ("body", "cleaned := value");
			break;
		case Type::Category::FixedBytes:
		{
			FixedBytesType const& type = dynamic_cast<FixedBytesType const&>(_type);
			if (type.numBytes() == 32)
				templ("body", "cleaned := value");
			else if (type.numBytes() == 0)
				// This is disallowed in the type system.
				solAssert(false, "");
			else
			{
				size_t numBits = type.numBytes() * 8;
				u256 mask = ((u256(1) << numBits) - 1) << (256 - numBits);
				templ("body", "cleaned := and(value, " + toCompactHexWithPrefix(mask) + ")");
			}
			break;
		}
		case Type::Category::Contract:
		{
			AddressType addressType(dynamic_cast<ContractType const&>(_type).isPayable() ?
				StateMutability::Payable :
				StateMutability::NonPayable
			);
			templ("body", "cleaned := " + cleanupFunction(addressType) + "(value)");
			break;
		}
		case Type::Category::Enum:
		{
			// Out of range enums cannot be truncated unambigiously and therefore it should be an error.
			templ("body", "cleaned := value " + validatorFunction(_type) + "(value)");
			break;
		}
		case Type::Category::InaccessibleDynamic:
			templ("body", "cleaned := 0");
			break;
		default:
			solAssert(false, "Cleanup of type " + _type.identifier() + " requested.");
		}

		return templ.render();
	});
}

string YulUtilFunctions::validatorFunction(Type const& _type, bool _revertOnFailure)
{
	string functionName = string("validator_") + (_revertOnFailure ? "revert_" : "assert_") + _type.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(value) {
				if iszero(<condition>) { <failure> }
			}
		)");
		templ("functionName", functionName);
		if (_revertOnFailure)
			templ("failure", "revert(0, 0)");
		else
			templ("failure", "invalid()");

		switch (_type.category())
		{
		case Type::Category::Address:
		case Type::Category::Integer:
		case Type::Category::RationalNumber:
		case Type::Category::Bool:
		case Type::Category::FixedPoint:
		case Type::Category::Function:
		case Type::Category::Array:
		case Type::Category::Struct:
		case Type::Category::Mapping:
		case Type::Category::FixedBytes:
		case Type::Category::Contract:
		{
			templ("condition", "eq(value, " + cleanupFunction(_type) + "(value))");
			break;
		}
		case Type::Category::Enum:
		{
			size_t members = dynamic_cast<EnumType const&>(_type).numberOfMembers();
			solAssert(members > 0, "empty enum should have caused a parser error.");
			templ("condition", "lt(value, " + to_string(members) + ")");
			break;
		}
		case Type::Category::InaccessibleDynamic:
			templ("condition", "1");
			break;
		default:
			solAssert(false, "Validation of type " + _type.identifier() + " requested.");
		}

		return templ.render();
	});
}

string YulUtilFunctions::packedHashFunction(
	vector<Type const*> const& _givenTypes,
	vector<Type const*> const& _targetTypes
)
{
	string functionName = string("packed_hashed_");
	for (auto const& t: _givenTypes)
		functionName += t->identifier() + "_";
	functionName += "_to_";
	for (auto const& t: _targetTypes)
		functionName += t->identifier() + "_";
	size_t sizeOnStack = 0;
	for (Type const* t: _givenTypes)
		sizeOnStack += t->sizeOnStack();
	return m_functionCollector->createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(<variables>) -> hash {
				let pos := mload(<freeMemoryPointer>)
				let end := <packedEncode>(pos <comma> <variables>)
				hash := keccak256(pos, sub(end, pos))
			}
		)");
		templ("functionName", functionName);
		templ("variables", suffixedVariableNameList("var_", 1, 1 + sizeOnStack));
		templ("comma", sizeOnStack > 0 ? "," : "");
		templ("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer));
		templ("packedEncode", ABIFunctions(m_evmVersion, m_functionCollector).tupleEncoderPacked(_givenTypes, _targetTypes));
		return templ.render();
	});
}

string YulUtilFunctions::forwardingRevertFunction()
{
	bool forward = m_evmVersion.supportsReturndata();
	string functionName = "revert_forward_" + to_string(forward);
	return m_functionCollector->createFunction(functionName, [&]() {
		if (forward)
			return Whiskers(R"(
				function <functionName>() {
					returndatacopy(0, 0, returndatasize())
					revert(0, returndatasize())
				}
			)")
			("functionName", functionName)
			.render();
		else
			return Whiskers(R"(
				function <functionName>() {
					revert(0, 0)
				}
			)")
			("functionName", functionName)
			.render();
	});
}

std::string YulUtilFunctions::decrementCheckedFunction(Type const& _type)
{
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);

	string const functionName = "decrement_" + _type.identifier();

	return m_functionCollector->createFunction(functionName, [&]() {
		u256 minintval;

		// Smallest admissible value to decrement
		if (type.isSigned())
			minintval = 0 - (u256(1) << (type.numBits() - 1)) + 1;
		else
			minintval = 1;

		return Whiskers(R"(
			function <functionName>(value) -> ret {
				if <lt>(value, <minval>) { revert(0,0) }
				ret := sub(value, 1)
			}
		)")
			("functionName", functionName)
			("minval", toCompactHexWithPrefix(minintval))
			("lt", type.isSigned() ? "slt" : "lt")
			.render();
	});
}

std::string YulUtilFunctions::incrementCheckedFunction(Type const& _type)
{
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);

	string const functionName = "increment_" + _type.identifier();

	return m_functionCollector->createFunction(functionName, [&]() {
		u256 maxintval;

		// Biggest admissible value to increment
		if (type.isSigned())
			maxintval = (u256(1) << (type.numBits() - 1)) - 2;
		else
			maxintval = (u256(1) << type.numBits()) - 2;

		return Whiskers(R"(
			function <functionName>(value) -> ret {
				if <gt>(value, <maxval>) { revert(0,0) }
				ret := add(value, 1)
			}
		)")
			("functionName", functionName)
			("maxval", toCompactHexWithPrefix(maxintval))
			("gt", type.isSigned() ? "sgt" : "gt")
			.render();
	});
}

string YulUtilFunctions::negateNumberCheckedFunction(Type const& _type)
{
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	solAssert(type.isSigned(), "Expected signed type!");

	string const functionName = "negate_" + _type.identifier();

	u256 const minintval = 0 - (u256(1) << (type.numBits() - 1)) + 1;

	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(_value) -> ret {
				if slt(_value, <minval>) { revert(0,0) }
				ret := sub(0, _value)
			}
		)")
			("functionName", functionName)
			("minval", toCompactHexWithPrefix(minintval))
			.render();
		});
}

string YulUtilFunctions::zeroValueFunction(Type const& _type)
{
	solUnimplementedAssert(_type.sizeOnStack() == 1, "Stacksize not yet implemented!");
	solUnimplementedAssert(_type.isValueType(), "Zero value for non-value types not yet implemented");

	string const functionName = "zero_value_for_" + _type.identifier();

	return m_functionCollector->createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>() -> ret {
				<body>
			}
		)")
			("functionName", functionName)
			("body", "ret := 0x0")
			.render();
		});
}

string YulUtilFunctions::storageSetToZeroFunction(Type const& _type)
{
	string const functionName = "storage_set_to_zero_" + _type.identifier();

	return m_functionCollector->createFunction(functionName, [&]() {
		if (_type.isValueType())
			return Whiskers(R"(
				function <functionName>(slot, offset) {
					<store>(slot, offset, <zeroValue>())
				}
			)")
			("functionName", functionName)
			("store", updateStorageValueFunction(_type))
			("zeroValue", zeroValueFunction(_type))
			.render();
		else if (_type.category() == Type::Category::Array)
			return Whiskers(R"(
				function <functionName>(slot, offset) {
					<clearArray>(slot)
				}
			)")
			("functionName", functionName)
			("clearArray", clearStorageArrayFunction(dynamic_cast<ArrayType const&>(_type)))
			.render();
		else
			solUnimplemented("setToZero for type " + _type.identifier() + " not yet implemented!");
	});
}

string YulUtilFunctions::conversionFunctionSpecial(Type const& _from, Type const& _to)
{
	string functionName =
		"convert_" +
		_from.identifier() +
		"_to_" +
		_to.identifier();
	return m_functionCollector->createFunction(functionName, [&]() {
		solUnimplementedAssert(
			_from.category() == Type::Category::StringLiteral,
			"Type conversion " + _from.toString() + " -> " + _to.toString() + " not yet implemented."
		);
		string const& data = dynamic_cast<StringLiteralType const&>(_from).value();
		if (_to.category() == Type::Category::FixedBytes)
		{
			unsigned const numBytes = dynamic_cast<FixedBytesType const&>(_to).numBytes();
			solAssert(data.size() <= 32, "");
			Whiskers templ(R"(
				function <functionName>() -> converted {
					converted := <data>
				}
			)");
			templ("functionName", functionName);
			templ("data", formatNumber(
				h256::Arith(h256(data, h256::AlignLeft)) &
				(~(u256(-1) >> (8 * numBytes)))
			));
			return templ.render();
		}
		else if (_to.category() == Type::Category::Array)
		{
			auto const& arrayType = dynamic_cast<ArrayType const&>(_to);
			solAssert(arrayType.isByteArray(), "");
			size_t words = (data.size() + 31) / 32;
			size_t storageSize = 32 + words * 32;

			Whiskers templ(R"(
				function <functionName>() -> converted {
					converted := <allocate>(<storageSize>)
					mstore(converted, <size>)
					<#word>
						mstore(add(converted, <offset>), <wordValue>)
					</word>
				}
			)");
			templ("functionName", functionName);
			templ("allocate", allocationFunction());
			templ("storageSize", to_string(storageSize));
			templ("size", to_string(data.size()));
			vector<map<string, string>> wordParams(words);
			for (size_t i = 0; i < words; ++i)
			{
				wordParams[i]["offset"] = to_string(32 + i * 32);
				wordParams[i]["wordValue"] = "0x" + h256(data.substr(32 * i, 32), h256::AlignLeft).hex();
			}
			templ("word", wordParams);
			return templ.render();
		}
		else
			solAssert(
				false,
				"Invalid conversion from string literal to " + _to.toString() + " requested."
			);
	});
}

string YulUtilFunctions::readFromMemoryOrCalldata(Type const& _type, bool _fromCalldata)
{
	string functionName =
		string("read_from_") +
		(_fromCalldata ? "calldata" : "memory") +
		_type.identifier();

	// TODO use ABI functions for handling calldata
	if (_fromCalldata)
		solAssert(!_type.isDynamicallyEncoded(), "");

	return m_functionCollector->createFunction(functionName, [&] {
		if (auto refType = dynamic_cast<ReferenceType const*>(&_type))
		{
			solAssert(refType->sizeOnStack() == 1, "");
			solAssert(!_fromCalldata, "");

			return Whiskers(R"(
				function <functionName>(memPtr) -> value {
					value := mload(memPtr)
				}
			)")
			("functionName", functionName)
			.render();
		}

		solAssert(_type.isValueType(), "");

		if (auto const* funType = dynamic_cast<FunctionType const*>(&_type))
			if (funType->kind() == FunctionType::Kind::External)
				return Whiskers(R"(
					function <functionName>(memPtr) -> addr, selector {
						let combined := <load>(memPtr)
						addr, selector := <splitFunction>(combined)
					}
				)")
				("functionName", functionName)
				("load", _fromCalldata ? "calldataload" : "mload")
				("splitFunction", splitExternalFunctionIdFunction())
				.render();

		return Whiskers(R"(
			function <functionName>(memPtr) -> value {
				value := <load>(memPtr)
				<?needsValidation>
					value := <validate>(value)
				</needsValidation>
			}
		)")
		("functionName", functionName)
		("load", _fromCalldata ? "calldataload" : "mload")
		("needsValidation", _fromCalldata)
		("validate", _fromCalldata ? validatorFunction(_type) : "")
		.render();
	});
}
