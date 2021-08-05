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
 * Component that can generate various useful Yul functions.
 */

#include <libsolidity/codegen/YulUtilFunctions.h>

#include <libsolidity/codegen/MultiUseYulFunctionCollector.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/CompilerUtils.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/Whiskers.h>
#include <libsolutil/StringUtils.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;

string YulUtilFunctions::combineExternalFunctionIdFunction()
{
	string functionName = "combine_external_function_id";
	return m_functionCollector.createFunction(functionName, [&]() {
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
	return m_functionCollector.createFunction(functionName, [&]() {
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
	return m_functionCollector.createFunction(functionName, [&]() {
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

string YulUtilFunctions::copyLiteralToMemoryFunction(string const& _literal)
{
	string functionName = "copy_literal_to_memory_" + util::toHex(util::keccak256(_literal).asBytes());

	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>() -> memPtr {
				memPtr := <arrayAllocationFunction>(<size>)
				<storeLiteralInMem>(add(memPtr, 32))
			}
			)")
			("functionName", functionName)
			("arrayAllocationFunction", allocateMemoryArrayFunction(*TypeProvider::array(DataLocation::Memory, true)))
			("size", to_string(_literal.size()))
			("storeLiteralInMem", storeLiteralInMemoryFunction(_literal))
			.render();
	});
}

string YulUtilFunctions::storeLiteralInMemoryFunction(string const& _literal)
{
	string functionName = "store_literal_in_memory_" + util::toHex(util::keccak256(_literal).asBytes());

	return m_functionCollector.createFunction(functionName, [&]() {
		size_t words = (_literal.length() + 31) / 32;
		vector<map<string, string>> wordParams(words);
		for (size_t i = 0; i < words; ++i)
		{
			wordParams[i]["offset"] = to_string(i * 32);
			wordParams[i]["wordValue"] = formatAsStringOrNumber(_literal.substr(32 * i, 32));
		}

		return Whiskers(R"(
			function <functionName>(memPtr) {
				<#word>
					mstore(add(memPtr, <offset>), <wordValue>)
				</word>
			}
			)")
			("functionName", functionName)
			("word", wordParams)
			.render();
	});
}

string YulUtilFunctions::copyLiteralToStorageFunction(string const& _literal)
{
	string functionName = "copy_literal_to_storage_" + util::toHex(util::keccak256(_literal).asBytes());

	return m_functionCollector.createFunction(functionName, [&](vector<string>& _args, vector<string>&) {
		_args = {"slot"};

		if (_literal.size() >= 32)
		{
			size_t words = (_literal.length() + 31) / 32;
			vector<map<string, string>> wordParams(words);
			for (size_t i = 0; i < words; ++i)
			{
				wordParams[i]["offset"] = to_string(i);
				wordParams[i]["wordValue"] = formatAsStringOrNumber(_literal.substr(32 * i, 32));
			}
			return Whiskers(R"(
				let oldLen := <byteArrayLength>(sload(slot))
				<cleanUpArrayEnd>(slot, oldLen, <length>)
				sstore(slot, <encodedLen>)
				let dstPtr := <dataArea>(slot)
				<#word>
					sstore(add(dstPtr, <offset>), <wordValue>)
				</word>
			)")
			("byteArrayLength", extractByteArrayLengthFunction())
			("cleanUpArrayEnd", cleanUpDynamicByteArrayEndSlotsFunction(*TypeProvider::bytesStorage()))
			("dataArea", arrayDataAreaFunction(*TypeProvider::bytesStorage()))
			("word", wordParams)
			("length", to_string(_literal.size()))
			("encodedLen", to_string(2 * _literal.size() + 1))
			.render();
		}
		else
			return Whiskers(R"(
				let oldLen := <byteArrayLength>(sload(slot))
				<cleanUpArrayEnd>(slot, oldLen, <length>)
				sstore(slot, add(<wordValue>, <encodedLen>))
			)")
			("byteArrayLength", extractByteArrayLengthFunction())
			("cleanUpArrayEnd", cleanUpDynamicByteArrayEndSlotsFunction(*TypeProvider::bytesStorage()))
			("wordValue", formatAsStringOrNumber(_literal))
			("length", to_string(_literal.size()))
			("encodedLen", to_string(2 * _literal.size()))
			.render();
	});
}

string YulUtilFunctions::requireOrAssertFunction(bool _assert, Type const* _messageType)
{
	string functionName =
		string(_assert ? "assert_helper" : "require_helper") +
		(_messageType ? ("_" + _messageType->identifier()) : "");

	solAssert(!_assert || !_messageType, "Asserts can't have messages!");

	return m_functionCollector.createFunction(functionName, [&]() {
		if (!_messageType)
			return Whiskers(R"(
				function <functionName>(condition) {
					if iszero(condition) { <error> }
				}
			)")
			("error", _assert ? panicFunction(PanicCode::Assert) + "()" : "revert(0, 0)")
			("functionName", functionName)
			.render();

		int const hashHeaderSize = 4;
		u256 const errorHash = util::selectorFromSignature("Error(string)");

		string const encodeFunc = ABIFunctions(m_evmVersion, m_revertStrings, m_functionCollector)
			.tupleEncoder(
				{_messageType},
				{TypeProvider::stringMemory()}
			);

		return Whiskers(R"(
			function <functionName>(condition <messageVars>) {
				if iszero(condition) {
					let memPtr := <allocateUnbounded>()
					mstore(memPtr, <errorHash>)
					let end := <abiEncodeFunc>(add(memPtr, <hashHeaderSize>) <messageVars>)
					revert(memPtr, sub(end, memPtr))
				}
			}
		)")
		("functionName", functionName)
		("allocateUnbounded", allocateUnboundedFunction())
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
	return m_functionCollector.createFunction(functionName, [&]() {
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
			solAssert(dynamic_cast<EnumType const&>(_type).storageBytes() == 1, "");
			templ("body", "aligned := " + leftAlignFunction(IntegerType(8)) + "(value)");
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
	return m_functionCollector.createFunction(functionName, [&]() {
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
	return m_functionCollector.createFunction(functionName, [&]() {
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
	return m_functionCollector.createFunction(functionName, [&]() {
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
	string const functionName = "shift_right_unsigned_dynamic";
	return m_functionCollector.createFunction(functionName, [&]() {
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

string YulUtilFunctions::shiftRightSignedFunctionDynamic()
{
	string const functionName = "shift_right_signed_dynamic";
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(bits, value) -> result {
				<?hasShifts>
					result := sar(bits, value)
				<!hasShifts>
					let divisor := exp(2, bits)
					let xor_mask := sub(0, slt(value, 0))
					result := xor(div(xor(value, xor_mask), divisor), xor_mask)
					// combined version of
					//   switch slt(value, 0)
					//   case 0 { result := div(value, divisor) }
					//   default { result := not(div(not(value), divisor)) }
				</hasShifts>
			}
			)")
			("functionName", functionName)
			("hasShifts", m_evmVersion.hasBitwiseShifting())
			.render();
	});
}


string YulUtilFunctions::typedShiftLeftFunction(Type const& _type, Type const& _amountType)
{
	solUnimplementedAssert(_type.category() != Type::Category::FixedPoint, "Not yet implemented - FixedPointType.");
	solAssert(_type.category() == Type::Category::FixedBytes || _type.category() == Type::Category::Integer, "");
	solAssert(_amountType.category() == Type::Category::Integer, "");
	solAssert(!dynamic_cast<IntegerType const&>(_amountType).isSigned(), "");
	string const functionName = "shift_left_" + _type.identifier() + "_" + _amountType.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(value, bits) -> result {
				bits := <cleanAmount>(bits)
				result := <cleanup>(<shift>(bits, <cleanup>(value)))
			}
			)")
			("functionName", functionName)
			("cleanAmount", cleanupFunction(_amountType))
			("shift", shiftLeftFunctionDynamic())
			("cleanup", cleanupFunction(_type))
			.render();
	});
}

string YulUtilFunctions::typedShiftRightFunction(Type const& _type, Type const& _amountType)
{
	solUnimplementedAssert(_type.category() != Type::Category::FixedPoint, "Not yet implemented - FixedPointType.");
	solAssert(_type.category() == Type::Category::FixedBytes || _type.category() == Type::Category::Integer, "");
	solAssert(_amountType.category() == Type::Category::Integer, "");
	solAssert(!dynamic_cast<IntegerType const&>(_amountType).isSigned(), "");
	IntegerType const* integerType = dynamic_cast<IntegerType const*>(&_type);
	bool valueSigned = integerType && integerType->isSigned();

	string const functionName = "shift_right_" + _type.identifier() + "_" + _amountType.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(value, bits) -> result {
				bits := <cleanAmount>(bits)
				result := <cleanup>(<shift>(bits, <cleanup>(value)))
			}
			)")
			("functionName", functionName)
			("cleanAmount", cleanupFunction(_amountType))
			("shift", valueSigned ? shiftRightSignedFunctionDynamic() : shiftRightFunctionDynamic())
			("cleanup", cleanupFunction(_type))
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
	return m_functionCollector.createFunction(functionName, [&]() {
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
	return m_functionCollector.createFunction(functionName, [&]() {
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

string YulUtilFunctions::maskBytesFunctionDynamic()
{
	string functionName = "mask_bytes_dynamic";
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(data, bytes) -> result {
				let mask := not(<shr>(mul(8, bytes), not(0)))
				result := and(data, mask)
			})")
			("functionName", functionName)
			("shr", shiftRightFunctionDynamic())
			.render();
	});
}

string YulUtilFunctions::maskLowerOrderBytesFunction(size_t _bytes)
{
	string functionName = "mask_lower_order_bytes_" + to_string(_bytes);
	solAssert(_bytes <= 32, "");
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(data) -> result {
				result := and(data, <mask>)
			})")
			("functionName", functionName)
			("mask", formatNumber((~u256(0)) >> (256 - 8 * _bytes)))
			.render();
	});
}

string YulUtilFunctions::maskLowerOrderBytesFunctionDynamic()
{
	string functionName = "mask_lower_order_bytes_dynamic";
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(data, bytes) -> result {
				let mask := not(<shl>(mul(8, bytes), not(0)))
				result := and(data, mask)
			})")
			("functionName", functionName)
			("shl", shiftLeftFunctionDynamic())
			.render();
	});
}

string YulUtilFunctions::roundUpFunction()
{
	string functionName = "round_up_to_mul_of_32";
	return m_functionCollector.createFunction(functionName, [&]() {
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

string YulUtilFunctions::divide32CeilFunction()
{
	return m_functionCollector.createFunction(
		"divide_by_32_ceil",
		[&](vector<string>& _args, vector<string>& _ret) {
			_args = {"value"};
			_ret = {"result"};
			return "result := div(add(value, 31), 32)";
		}
	);
}

string YulUtilFunctions::overflowCheckedIntAddFunction(IntegerType const& _type)
{
	string functionName = "checked_add_" + _type.identifier();
	// TODO: Consider to add a special case for unsigned 256-bit integers
	//       and use the following instead:
	//       sum := add(x, y) if lt(sum, x) { <panic>() }
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> sum {
				x := <cleanupFunction>(x)
				y := <cleanupFunction>(y)
				<?signed>
					// overflow, if x >= 0 and y > (maxValue - x)
					if and(iszero(slt(x, 0)), sgt(y, sub(<maxValue>, x))) { <panic>() }
					// underflow, if x < 0 and y < (minValue - x)
					if and(slt(x, 0), slt(y, sub(<minValue>, x))) { <panic>() }
				<!signed>
					// overflow, if x > (maxValue - y)
					if gt(x, sub(<maxValue>, y)) { <panic>() }
				</signed>
				sum := add(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("maxValue", toCompactHexWithPrefix(u256(_type.maxValue())))
			("minValue", toCompactHexWithPrefix(u256(_type.minValue())))
			("cleanupFunction", cleanupFunction(_type))
			("panic", panicFunction(PanicCode::UnderOverflow))
			.render();
	});
}

string YulUtilFunctions::wrappingIntAddFunction(IntegerType const& _type)
{
	string functionName = "wrapping_add_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> sum {
				sum := <cleanupFunction>(add(x, y))
			}
			)")
			("functionName", functionName)
			("cleanupFunction", cleanupFunction(_type))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntMulFunction(IntegerType const& _type)
{
	string functionName = "checked_mul_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			// Multiplication by zero could be treated separately and directly return zero.
			Whiskers(R"(
			function <functionName>(x, y) -> product {
				x := <cleanupFunction>(x)
				y := <cleanupFunction>(y)
				<?signed>
					// overflow, if x > 0, y > 0 and x > (maxValue / y)
					if and(and(sgt(x, 0), sgt(y, 0)), gt(x, div(<maxValue>, y))) { <panic>() }
					// underflow, if x > 0, y < 0 and y < (minValue / x)
					if and(and(sgt(x, 0), slt(y, 0)), slt(y, sdiv(<minValue>, x))) { <panic>() }
					// underflow, if x < 0, y > 0 and x < (minValue / y)
					if and(and(slt(x, 0), sgt(y, 0)), slt(x, sdiv(<minValue>, y))) { <panic>() }
					// overflow, if x < 0, y < 0 and x < (maxValue / y)
					if and(and(slt(x, 0), slt(y, 0)), slt(x, sdiv(<maxValue>, y))) { <panic>() }
				<!signed>
					// overflow, if x != 0 and y > (maxValue / x)
					if and(iszero(iszero(x)), gt(y, div(<maxValue>, x))) { <panic>() }
				</signed>
				product := mul(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("maxValue", toCompactHexWithPrefix(u256(_type.maxValue())))
			("minValue", toCompactHexWithPrefix(u256(_type.minValue())))
			("cleanupFunction", cleanupFunction(_type))
			("panic", panicFunction(PanicCode::UnderOverflow))
			.render();
	});
}

string YulUtilFunctions::wrappingIntMulFunction(IntegerType const& _type)
{
	string functionName = "wrapping_mul_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> product {
				product := <cleanupFunction>(mul(x, y))
			}
			)")
			("functionName", functionName)
			("cleanupFunction", cleanupFunction(_type))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntDivFunction(IntegerType const& _type)
{
	string functionName = "checked_div_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> r {
				x := <cleanupFunction>(x)
				y := <cleanupFunction>(y)
				if iszero(y) { <panicDivZero>() }
				<?signed>
				// overflow for minVal / -1
				if and(
					eq(x, <minVal>),
					eq(y, sub(0, 1))
				) { <panicOverflow>() }
				</signed>
				r := <?signed>s</signed>div(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("minVal", toCompactHexWithPrefix(u256(_type.minValue())))
			("cleanupFunction", cleanupFunction(_type))
			("panicDivZero", panicFunction(PanicCode::DivisionByZero))
			("panicOverflow", panicFunction(PanicCode::UnderOverflow))
			.render();
	});
}

string YulUtilFunctions::wrappingIntDivFunction(IntegerType const& _type)
{
	string functionName = "wrapping_div_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> r {
				x := <cleanupFunction>(x)
				y := <cleanupFunction>(y)
				if iszero(y) { <error>() }
				r := <?signed>s</signed>div(x, y)
			}
			)")
			("functionName", functionName)
			("cleanupFunction", cleanupFunction(_type))
			("signed", _type.isSigned())
			("error", panicFunction(PanicCode::DivisionByZero))
			.render();
	});
}

string YulUtilFunctions::intModFunction(IntegerType const& _type)
{
	string functionName = "mod_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> r {
				x := <cleanupFunction>(x)
				y := <cleanupFunction>(y)
				if iszero(y) { <panic>() }
				r := <?signed>s</signed>mod(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("cleanupFunction", cleanupFunction(_type))
			("panic", panicFunction(PanicCode::DivisionByZero))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntSubFunction(IntegerType const& _type)
{
	string functionName = "checked_sub_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&] {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> diff {
				x := <cleanupFunction>(x)
				y := <cleanupFunction>(y)
				<?signed>
					// underflow, if y >= 0 and x < (minValue + y)
					if and(iszero(slt(y, 0)), slt(x, add(<minValue>, y))) { <panic>() }
					// overflow, if y < 0 and x > (maxValue + y)
					if and(slt(y, 0), sgt(x, add(<maxValue>, y))) { <panic>() }
				<!signed>
					if lt(x, y) { <panic>() }
				</signed>
				diff := sub(x, y)
			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("maxValue", toCompactHexWithPrefix(u256(_type.maxValue())))
			("minValue", toCompactHexWithPrefix(u256(_type.minValue())))
			("cleanupFunction", cleanupFunction(_type))
			("panic", panicFunction(PanicCode::UnderOverflow))
			.render();
	});
}

string YulUtilFunctions::wrappingIntSubFunction(IntegerType const& _type)
{
	string functionName = "wrapping_sub_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&] {
		return
			Whiskers(R"(
			function <functionName>(x, y) -> diff {
				diff := <cleanupFunction>(sub(x, y))
			}
			)")
			("functionName", functionName)
			("cleanupFunction", cleanupFunction(_type))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntExpFunction(
	IntegerType const& _type,
	IntegerType const& _exponentType
)
{
	solAssert(!_exponentType.isSigned(), "");

	string functionName = "checked_exp_" + _type.identifier() + "_" + _exponentType.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(base, exponent) -> power {
				base := <baseCleanupFunction>(base)
				exponent := <exponentCleanupFunction>(exponent)
				<?signed>
					power := <exp>(base, exponent, <minValue>, <maxValue>)
				<!signed>
					power := <exp>(base, exponent, <maxValue>)
				</signed>

			}
			)")
			("functionName", functionName)
			("signed", _type.isSigned())
			("exp", _type.isSigned() ? overflowCheckedSignedExpFunction() : overflowCheckedUnsignedExpFunction())
			("maxValue", toCompactHexWithPrefix(_type.max()))
			("minValue", toCompactHexWithPrefix(_type.min()))
			("baseCleanupFunction", cleanupFunction(_type))
			("exponentCleanupFunction", cleanupFunction(_exponentType))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedIntLiteralExpFunction(
	RationalNumberType const& _baseType,
	IntegerType const& _exponentType,
	IntegerType const& _commonType
)
{
	solAssert(!_exponentType.isSigned(), "");
	solAssert(_baseType.isNegative() == _commonType.isSigned(), "");
	solAssert(_commonType.numBits() == 256, "");

	string functionName = "checked_exp_" + _baseType.richIdentifier() + "_" + _exponentType.identifier();

	return m_functionCollector.createFunction(functionName, [&]()
	{
		// Converts a bigint number into u256 (negative numbers represented in two's complement form.)
		// We assume that `_v` fits in 256 bits.
		auto bigint2u = [&](bigint const& _v) -> u256
		{
			if (_v < 0)
				return s2u(s256(_v));
			return u256(_v);
		};

		// Calculates the upperbound for exponentiation, that is, calculate `b`, such that
		// _base**b <= _maxValue and _base**(b + 1) > _maxValue
		auto findExponentUpperbound = [](bigint const _base, bigint const _maxValue) -> unsigned
		{
			// There is no overflow for these cases
			if (_base == 0 || _base == -1 || _base == 1)
				return 0;

			unsigned first = 0;
			unsigned last = 255;
			unsigned middle;

			while (first < last)
			{
				middle = (first + last) / 2;

				if (
					// The condition on msb is a shortcut that avoids computing large powers in
					// arbitrary precision.
					boost::multiprecision::msb(_base) * middle <= boost::multiprecision::msb(_maxValue) &&
					boost::multiprecision::pow(_base, middle) <= _maxValue
				)
				{
					if (boost::multiprecision::pow(_base, middle + 1) > _maxValue)
						return middle;
					else
						first = middle + 1;
				}
				else
					last = middle;
			}

			return last;
		};

		bigint baseValue = _baseType.isNegative() ?
			u2s(_baseType.literalValue(nullptr)) :
			_baseType.literalValue(nullptr);
		bool needsOverflowCheck = !((baseValue == 0) || (baseValue == -1) || (baseValue == 1));
		unsigned exponentUpperbound;

		if (_baseType.isNegative())
		{
			// Only checks for underflow. The only case where this can be a problem is when, for a
			// negative base, say `b`, and an even exponent, say `e`, `b**e = 2**255` (which is an
			// overflow.) But this never happens because, `255 = 3*5*17`, and therefore there is no even
			// number `e` such that `b**e = 2**255`.
			exponentUpperbound = findExponentUpperbound(abs(baseValue), abs(_commonType.minValue()));

			bigint power = boost::multiprecision::pow(baseValue, exponentUpperbound);
			bigint overflowedPower = boost::multiprecision::pow(baseValue, exponentUpperbound + 1);

			if (needsOverflowCheck)
				solAssert(
					(power <= _commonType.maxValue()) && (power >= _commonType.minValue()) &&
					!((overflowedPower <= _commonType.maxValue()) && (overflowedPower >= _commonType.minValue())),
					"Incorrect exponent upper bound calculated."
				);
		}
		else
		{
			exponentUpperbound = findExponentUpperbound(baseValue, _commonType.maxValue());

			if (needsOverflowCheck)
				solAssert(
					boost::multiprecision::pow(baseValue, exponentUpperbound) <= _commonType.maxValue() &&
					boost::multiprecision::pow(baseValue, exponentUpperbound + 1) > _commonType.maxValue(),
					"Incorrect exponent upper bound calculated."
				);
		}

		return Whiskers(R"(
			function <functionName>(exponent) -> power {
				exponent := <exponentCleanupFunction>(exponent)
				<?needsOverflowCheck>
				if gt(exponent, <exponentUpperbound>) { <panic>() }
				</needsOverflowCheck>
				power := exp(<base>, exponent)
			}
			)")
			("functionName", functionName)
			("exponentCleanupFunction", cleanupFunction(_exponentType))
			("needsOverflowCheck", needsOverflowCheck)
			("exponentUpperbound", to_string(exponentUpperbound))
			("panic", panicFunction(PanicCode::UnderOverflow))
			("base", bigint2u(baseValue).str())
			.render();
	});
}

string YulUtilFunctions::overflowCheckedUnsignedExpFunction()
{
	// Checks for the "small number specialization" below.
	using namespace boost::multiprecision;
	solAssert(pow(bigint(10), 77) < pow(bigint(2), 256), "");
	solAssert(pow(bigint(11), 77) >= pow(bigint(2), 256), "");
	solAssert(pow(bigint(10), 78) >= pow(bigint(2), 256), "");

	solAssert(pow(bigint(306), 31) < pow(bigint(2), 256), "");
	solAssert(pow(bigint(307), 31) >= pow(bigint(2), 256), "");
	solAssert(pow(bigint(306), 32) >= pow(bigint(2), 256), "");

	string functionName = "checked_exp_unsigned";
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(base, exponent, max) -> power {
				// This function currently cannot be inlined because of the
				// "leave" statements. We have to improve the optimizer.

				// Note that 0**0 == 1
				if iszero(exponent) { power := 1 leave }
				if iszero(base) { power := 0 leave }

				// Specializations for small bases
				switch base
				// 0 is handled above
				case 1 { power := 1 leave }
				case 2
				{
					if gt(exponent, 255) { <panic>() }
					power := exp(2, exponent)
					if gt(power, max) { <panic>() }
					leave
				}
				if or(
					and(lt(base, 11), lt(exponent, 78)),
					and(lt(base, 307), lt(exponent, 32))
				)
				{
					power := exp(base, exponent)
					if gt(power, max) { <panic>() }
					leave
				}

				power, base := <expLoop>(1, base, exponent, max)

				if gt(power, div(max, base)) { <panic>() }
				power := mul(power, base)
			}
			)")
			("functionName", functionName)
			("panic", panicFunction(PanicCode::UnderOverflow))
			("expLoop", overflowCheckedExpLoopFunction())
			.render();
	});
}

string YulUtilFunctions::overflowCheckedSignedExpFunction()
{
	string functionName = "checked_exp_signed";
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(base, exponent, min, max) -> power {
				// Currently, `leave` avoids this function being inlined.
				// We have to improve the optimizer.

				// Note that 0**0 == 1
				switch exponent
				case 0 { power := 1 leave }
				case 1 { power := base leave }
				if iszero(base) { power := 0 leave }

				power := 1

				// We pull out the first iteration because it is the only one in which
				// base can be negative.
				// Exponent is at least 2 here.

				// overflow check for base * base
				switch sgt(base, 0)
				case 1 { if gt(base, div(max, base)) { <panic>() } }
				case 0 { if slt(base, sdiv(max, base)) { <panic>() } }
				if and(exponent, 1)
				{
					power := base
				}
				base := mul(base, base)
				exponent := <shr_1>(exponent)

				// Below this point, base is always positive.

				power, base := <expLoop>(power, base, exponent, max)

				if and(sgt(power, 0), gt(power, div(max, base))) { <panic>() }
				if and(slt(power, 0), slt(power, sdiv(min, base))) { <panic>() }
				power := mul(power, base)
			}
			)")
			("functionName", functionName)
			("panic", panicFunction(PanicCode::UnderOverflow))
			("expLoop", overflowCheckedExpLoopFunction())
			("shr_1", shiftRightFunction(1))
			.render();
	});
}

string YulUtilFunctions::overflowCheckedExpLoopFunction()
{
	// We use this loop for both signed and unsigned exponentiation
	// because we pull out the first iteration in the signed case which
	// results in the base always being positive.

	// This function does not include the final multiplication.

	string functionName = "checked_exp_helper";
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(_power, _base, exponent, max) -> power, base {
				power := _power
				base  := _base
				for { } gt(exponent, 1) {}
				{
					// overflow check for base * base
					if gt(base, div(max, base)) { <panic>() }
					if and(exponent, 1)
					{
						// No checks for power := mul(power, base) needed, because the check
						// for base * base above is sufficient, since:
						// |power| <= base (proof by induction) and thus:
						// |power * base| <= base * base <= max <= |min| (for signed)
						// (this is equally true for signed and unsigned exp)
						power := mul(power, base)
					}
					base := mul(base, base)
					exponent := <shr_1>(exponent)
				}
			}
			)")
			("functionName", functionName)
			("panic", panicFunction(PanicCode::UnderOverflow))
			("shr_1", shiftRightFunction(1))
			.render();
	});
}

string YulUtilFunctions::wrappingIntExpFunction(
	IntegerType const& _type,
	IntegerType const& _exponentType
)
{
	solAssert(!_exponentType.isSigned(), "");

	string functionName = "wrapping_exp_" + _type.identifier() + "_" + _exponentType.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return
			Whiskers(R"(
			function <functionName>(base, exponent) -> power {
				base := <baseCleanupFunction>(base)
				exponent := <exponentCleanupFunction>(exponent)
				power := <baseCleanupFunction>(exp(base, exponent))
			}
			)")
			("functionName", functionName)
			("baseCleanupFunction", cleanupFunction(_type))
			("exponentCleanupFunction", cleanupFunction(_exponentType))
			.render();
	});
}

string YulUtilFunctions::arrayLengthFunction(ArrayType const& _type)
{
	string functionName = "array_length_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers w(R"(
			function <functionName>(value<?dynamic><?calldata>, len</calldata></dynamic>) -> length {
				<?dynamic>
					<?memory>
						length := mload(value)
					</memory>
					<?storage>
						length := sload(value)
						<?byteArray>
							length := <extractByteArrayLength>(length)
						</byteArray>
					</storage>
					<?calldata>
						length := len
					</calldata>
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
		w("calldata", _type.location() == DataLocation::CallData);
		if (_type.location() == DataLocation::Storage)
		{
			w("byteArray", _type.isByteArray());
			if (_type.isByteArray())
				w("extractByteArrayLength", extractByteArrayLengthFunction());
		}

		return w.render();
	});
}

string YulUtilFunctions::extractByteArrayLengthFunction()
{
	string functionName = "extract_byte_array_length";
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers w(R"(
			function <functionName>(data) -> length {
				length := div(data, 2)
				let outOfPlaceEncoding := and(data, 1)
				if iszero(outOfPlaceEncoding) {
					length := and(length, 0x7f)
				}

				if eq(outOfPlaceEncoding, lt(length, 32)) {
					<panic>()
				}
			}
		)");
		w("functionName", functionName);
		w("panic", panicFunction(PanicCode::StorageEncodingError));
		return w.render();
	});
}

std::string YulUtilFunctions::resizeArrayFunction(ArrayType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solUnimplementedAssert(_type.baseType()->storageBytes() <= 32, "...");

	if (_type.isByteArray())
		return resizeDynamicByteArrayFunction(_type);

	string functionName = "resize_array_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(array, newLen) {
				if gt(newLen, <maxArrayLength>) {
					<panic>()
				}

				let oldLen := <fetchLength>(array)

				<?isDynamic>
					// Store new length
					sstore(array, newLen)
				</isDynamic>

				<?needsClearing>
					<cleanUpArrayEnd>(array, oldLen, newLen)
				</needsClearing>
			})");
			templ("functionName", functionName);
			templ("maxArrayLength", (u256(1) << 64).str());
			templ("panic", panicFunction(util::PanicCode::ResourceError));
			templ("fetchLength", arrayLengthFunction(_type));
			templ("isDynamic", _type.isDynamicallySized());
			bool isMappingBase = _type.baseType()->category() == Type::Category::Mapping;
			templ("needsClearing", !isMappingBase);
			if (!isMappingBase)
				templ("cleanUpArrayEnd", cleanUpStorageArrayEndFunction(_type));
			return templ.render();
	});
}

string YulUtilFunctions::cleanUpStorageArrayEndFunction(ArrayType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.baseType()->category() != Type::Category::Mapping, "");
	solAssert(!_type.isByteArray(), "");
	solUnimplementedAssert(_type.baseType()->storageBytes() <= 32, "");

	string functionName = "cleanup_storage_array_end_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&](vector<string>& _args, vector<string>&) {
		_args = {"array", "len", "startIndex"};
		return Whiskers(R"(
			if lt(startIndex, len) {
				// Size was reduced, clear end of array
				let oldSlotCount := <convertToSize>(len)
				let newSlotCount := <convertToSize>(startIndex)
				let arrayDataStart := <dataPosition>(array)
				let deleteStart := add(arrayDataStart, newSlotCount)
				let deleteEnd := add(arrayDataStart, oldSlotCount)
				<?packed>
					// if we are dealing with packed array and offset is greater than zero
					// we have  to partially clear last slot that is still used, so decreasing start by one
					let offset := mul(mod(startIndex, <itemsPerSlot>), <storageBytes>)
					if gt(offset, 0) { <partialClearStorageSlot>(sub(deleteStart, 1), offset) }
				</packed>
				<clearStorageRange>(deleteStart, deleteEnd)
			}
		)")
		("convertToSize", arrayConvertLengthToSize(_type))
		("dataPosition", arrayDataAreaFunction(_type))
		("clearStorageRange", clearStorageRangeFunction(*_type.baseType()))
		("packed", _type.baseType()->storageBytes() <= 16)
		("itemsPerSlot", to_string(32 / _type.baseType()->storageBytes()))
		("storageBytes", to_string(_type.baseType()->storageBytes()))
		("partialClearStorageSlot", partialClearStorageSlotFunction())
		.render();
	});
}

string YulUtilFunctions::resizeDynamicByteArrayFunction(ArrayType const& _type)
{
	string functionName = "resize_array_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&](vector<string>& _args, vector<string>&) {
		_args = {"array", "newLen"};
		return Whiskers(R"(
			let data := sload(array)
			let oldLen := <extractLength>(data)

			if gt(newLen, oldLen) {
				<increaseSize>(array, data, oldLen, newLen)
			}

			if lt(newLen, oldLen) {
				<decreaseSize>(array, data, oldLen, newLen)
			}
		)")
		("extractLength", extractByteArrayLengthFunction())
		("decreaseSize", decreaseByteArraySizeFunction(_type))
		("increaseSize", increaseByteArraySizeFunction(_type))
		.render();
	});
}

string YulUtilFunctions::cleanUpDynamicByteArrayEndSlotsFunction(ArrayType const& _type)
{
	solAssert(_type.isByteArray(), "");
	solAssert(_type.isDynamicallySized(), "");

	string functionName = "clean_up_bytearray_end_slots_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&](vector<string>& _args, vector<string>&) {
		_args = {"array", "len", "startIndex"};
		return Whiskers(R"(
			if gt(len, 31) {
				let dataArea := <dataLocation>(array)
				let deleteStart := add(dataArea, <div32Ceil>(startIndex))
				// If we are clearing array to be short byte array, we want to clear only data starting from array data area.
				if lt(startIndex, 32) { deleteStart := dataArea }
				<clearStorageRange>(deleteStart, add(dataArea, <div32Ceil>(len)))
			}
		)")
		("dataLocation", arrayDataAreaFunction(_type))
		("div32Ceil", divide32CeilFunction())
		("clearStorageRange", clearStorageRangeFunction(*_type.baseType()))
		.render();
	});
}

string YulUtilFunctions::decreaseByteArraySizeFunction(ArrayType const& _type)
{
	string functionName = "byte_array_decrease_size_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array, data, oldLen, newLen) {
				switch lt(newLen, 32)
				case  0 {
					let arrayDataStart := <dataPosition>(array)
					let deleteStart := add(arrayDataStart, <div32Ceil>(newLen))

					// we have to partially clear last slot that is still used
					let offset := and(newLen, 0x1f)
					if offset { <partialClearStorageSlot>(sub(deleteStart, 1), offset) }

					<clearStorageRange>(deleteStart, add(arrayDataStart, <div32Ceil>(oldLen)))

					sstore(array, or(mul(2, newLen), 1))
				}
				default {
					switch gt(oldLen, 31)
					case 1 {
						let arrayDataStart := <dataPosition>(array)
						// clear whole old array, as we are transforming to short bytes array
						<clearStorageRange>(add(arrayDataStart, 1), add(arrayDataStart, <div32Ceil>(oldLen)))
						<transitLongToShort>(array, newLen)
					}
					default {
						sstore(array, <encodeUsedSetLen>(data, newLen))
					}
				}
			})")
			("functionName", functionName)
			("dataPosition", arrayDataAreaFunction(_type))
			("partialClearStorageSlot", partialClearStorageSlotFunction())
			("clearStorageRange", clearStorageRangeFunction(*_type.baseType()))
			("transitLongToShort", byteArrayTransitLongToShortFunction(_type))
			("div32Ceil", divide32CeilFunction())
			("encodeUsedSetLen", shortByteArrayEncodeUsedAreaSetLengthFunction())
			.render();
	});
}

string YulUtilFunctions::increaseByteArraySizeFunction(ArrayType const& _type)
{
	string functionName = "byte_array_increase_size_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&](vector<string>& _args, vector<string>&) {
		_args = {"array", "data", "oldLen", "newLen"};
		return Whiskers(R"(
			if gt(newLen, <maxArrayLength>) { <panic>() }

			switch lt(oldLen, 32)
			case 0 {
				// in this case array stays unpacked, so we just set new length
				sstore(array, add(mul(2, newLen), 1))
			}
			default {
				switch lt(newLen, 32)
				case 0 {
					// we need to copy elements to data area as we changed array from packed to unpacked
					data := and(not(0xff), data)
					sstore(<dataPosition>(array), data)
					sstore(array, add(mul(2, newLen), 1))
				}
				default {
					// here array stays packed, we just need to increase length
					sstore(array, <encodeUsedSetLen>(data, newLen))
				}
			}
		)")
		("panic", panicFunction(PanicCode::ResourceError))
		("maxArrayLength", (u256(1) << 64).str())
		("dataPosition", arrayDataAreaFunction(_type))
		("encodeUsedSetLen", shortByteArrayEncodeUsedAreaSetLengthFunction())
		.render();
	});
}

string YulUtilFunctions::byteArrayTransitLongToShortFunction(ArrayType const& _type)
{
	string functionName = "transit_byte_array_long_to_short_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array, len) {
				// we need to copy elements from old array to new
				// we want to copy only elements that are part of the array after resizing
				let dataPos := <dataPosition>(array)
				let data := <extractUsedApplyLen>(sload(dataPos), len)
				sstore(array, data)
				sstore(dataPos, 0)
			})")
			("functionName", functionName)
			("dataPosition", arrayDataAreaFunction(_type))
			("extractUsedApplyLen", shortByteArrayEncodeUsedAreaSetLengthFunction())
			.render();
	});
}

string YulUtilFunctions::shortByteArrayEncodeUsedAreaSetLengthFunction()
{
	string functionName = "extract_used_part_and_set_length_of_short_byte_array";
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(data, len) -> used {
				// we want to save only elements that are part of the array after resizing
				// others should be set to zero
				data := <maskBytes>(data, len)
				used := or(data, mul(2, len))
			})")
			("functionName", functionName)
			("maskBytes", maskBytesFunctionDynamic())
			.render();
	});
}

string YulUtilFunctions::longByteArrayStorageIndexAccessNoCheckFunction()
{
	return m_functionCollector.createFunction(
		"long_byte_array_index_access_no_checks",
		[&](vector<string>& _args, vector<string>& _returnParams) {
			_args = {"array", "index"};
			_returnParams = {"slot", "offset"};
			return Whiskers(R"(
				offset := sub(31, mod(index, 0x20))
				let dataArea := <dataAreaFunc>(array)
				slot := add(dataArea, div(index, 0x20))
			)")
			("dataAreaFunc", arrayDataAreaFunction(*TypeProvider::bytesStorage()))
			.render();
		}
	);
}

string YulUtilFunctions::storageArrayPopFunction(ArrayType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	solUnimplementedAssert(_type.baseType()->storageBytes() <= 32, "Base type is not yet implemented.");
	if (_type.isByteArray())
		return storageByteArrayPopFunction(_type);

	string functionName = "array_pop_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array) {
				let oldLen := <fetchLength>(array)
				if iszero(oldLen) { <panic>() }
				let newLen := sub(oldLen, 1)
				let slot, offset := <indexAccess>(array, newLen)
				<?+setToZero><setToZero>(slot, offset)</+setToZero>
				sstore(array, newLen)
			})")
			("functionName", functionName)
			("panic", panicFunction(PanicCode::EmptyArrayPop))
			("fetchLength", arrayLengthFunction(_type))
			("indexAccess", storageArrayIndexAccessFunction(_type))
			(
				"setToZero",
				_type.baseType()->category() != Type::Category::Mapping ? storageSetToZeroFunction(*_type.baseType()) : ""
			)
			.render();
	});
}

string YulUtilFunctions::storageByteArrayPopFunction(ArrayType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	solAssert(_type.isByteArray(), "");

	string functionName = "byte_array_pop_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array) {
				let data := sload(array)
				let oldLen := <extractByteArrayLength>(data)
				if iszero(oldLen) { <panic>() }

				switch oldLen
				case 32 {
					// Here we have a special case where array transitions to shorter than 32
					// So we need to copy data
					<transitLongToShort>(array, 31)
				}
				default {
					let newLen := sub(oldLen, 1)
					switch lt(oldLen, 32)
					case 1 {
						sstore(array, <encodeUsedSetLen>(data, newLen))
					}
					default {
						let slot, offset := <indexAccessNoChecks>(array, newLen)
						<setToZero>(slot, offset)
						sstore(array, sub(data, 2))
					}
				}
			})")
			("functionName", functionName)
			("panic", panicFunction(PanicCode::EmptyArrayPop))
			("extractByteArrayLength", extractByteArrayLengthFunction())
			("transitLongToShort", byteArrayTransitLongToShortFunction(_type))
			("encodeUsedSetLen", shortByteArrayEncodeUsedAreaSetLengthFunction())
			("indexAccessNoChecks", longByteArrayStorageIndexAccessNoCheckFunction())
			("setToZero", storageSetToZeroFunction(*_type.baseType()))
			.render();
	});
}

string YulUtilFunctions::storageArrayPushFunction(ArrayType const& _type, Type const* _fromType)
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	if (!_fromType)
		_fromType = _type.baseType();
	else if (_fromType->isValueType())
		solUnimplementedAssert(*_fromType == *_type.baseType(), "");

	string functionName =
		string{"array_push_from_"} +
		_fromType->identifier() +
		"_to_" +
		_type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array <values>) {
				<?isByteArray>
					let data := sload(array)
					let oldLen := <extractByteArrayLength>(data)
					if iszero(lt(oldLen, <maxArrayLength>)) { <panic>() }

					switch gt(oldLen, 31)
					case 0 {
						let value := byte(0 <values>)
						switch oldLen
						case 31 {
							// Here we have special case when array switches from short array to long array
							// We need to copy data
							let dataArea := <dataAreaFunction>(array)
							data := and(data, not(0xff))
							sstore(dataArea, or(and(0xff, value), data))
							// New length is 32, encoded as (32 * 2 + 1)
							sstore(array, 65)
						}
						default {
							data := add(data, 2)
							let shiftBits := mul(8, sub(31, oldLen))
							let valueShifted := <shl>(shiftBits, and(0xff, value))
							let mask := <shl>(shiftBits, 0xff)
							data := or(and(data, not(mask)), valueShifted)
							sstore(array, data)
						}
					}
					default {
						sstore(array, add(data, 2))
						let slot, offset := <indexAccess>(array, oldLen)
						<storeValue>(slot, offset <values>)
					}
				<!isByteArray>
					let oldLen := sload(array)
					if iszero(lt(oldLen, <maxArrayLength>)) { <panic>() }
					sstore(array, add(oldLen, 1))
					let slot, offset := <indexAccess>(array, oldLen)
					<storeValue>(slot, offset <values>)
				</isByteArray>
			})")
			("functionName", functionName)
			("values", _fromType->sizeOnStack() == 0 ? "" : ", " + suffixedVariableNameList("value", 0, _fromType->sizeOnStack()))
			("panic", panicFunction(PanicCode::ResourceError))
			("extractByteArrayLength", _type.isByteArray() ? extractByteArrayLengthFunction() : "")
			("dataAreaFunction", arrayDataAreaFunction(_type))
			("isByteArray", _type.isByteArray())
			("indexAccess", storageArrayIndexAccessFunction(_type))
			("storeValue", updateStorageValueFunction(*_fromType, *_type.baseType()))
			("maxArrayLength", (u256(1) << 64).str())
			("shl", shiftLeftFunctionDynamic())
			.render();
	});
}

string YulUtilFunctions::storageArrayPushZeroFunction(ArrayType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");
	solAssert(_type.isDynamicallySized(), "");
	solUnimplementedAssert(_type.baseType()->storageBytes() <= 32, "Base type is not yet implemented.");

	string functionName = "array_push_zero_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array) -> slot, offset {
				<?isBytes>
					let data := sload(array)
					let oldLen := <extractLength>(data)
					<increaseBytesSize>(array, data, oldLen, add(oldLen, 1))
				<!isBytes>
					let oldLen := <fetchLength>(array)
					if iszero(lt(oldLen, <maxArrayLength>)) { <panic>() }
					sstore(array, add(oldLen, 1))
				</isBytes>
				slot, offset := <indexAccess>(array, oldLen)
			})")
			("functionName", functionName)
			("isBytes", _type.isByteArray())
			("increaseBytesSize", _type.isByteArray() ? increaseByteArraySizeFunction(_type) : "")
			("extractLength", _type.isByteArray() ? extractByteArrayLengthFunction() : "")
			("panic", panicFunction(PanicCode::ResourceError))
			("fetchLength", arrayLengthFunction(_type))
			("indexAccess", storageArrayIndexAccessFunction(_type))
			("maxArrayLength", (u256(1) << 64).str())
			.render();
	});
}

string YulUtilFunctions::partialClearStorageSlotFunction()
{
	string functionName = "partial_clear_storage_slot";
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
		function <functionName>(slot, offset) {
			let mask := <shr>(mul(8, sub(32, offset)), <ones>)
			sstore(slot, and(mask, sload(slot)))
		}
		)")
		("functionName", functionName)
		("ones", formatNumber((bigint(1) << 256) - 1))
		("shr", shiftRightFunctionDynamic())
		.render();
	});
}

string YulUtilFunctions::clearStorageRangeFunction(Type const& _type)
{
	if (_type.storageBytes() < 32)
		solAssert(_type.isValueType(), "");

	string functionName = "clear_storage_range_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(start, end) {
				for {} lt(start, end) { start := add(start, <increment>) }
				{
					<setToZero>(start, 0)
				}
			}
		)")
		("functionName", functionName)
		("setToZero", storageSetToZeroFunction(_type.storageBytes() < 32 ? *TypeProvider::uint256() : _type))
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

	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(slot) {
				<?dynamic>
					<resizeArray>(slot, 0)
				<!dynamic>
					<?+clearRange><clearRange>(slot, add(slot, <lenToSize>(<len>)))</+clearRange>
				</dynamic>
			}
		)")
		("functionName", functionName)
		("dynamic", _type.isDynamicallySized())
		("resizeArray", _type.isDynamicallySized() ? resizeArrayFunction(_type) : "")
		(
			"clearRange",
			_type.baseType()->category() != Type::Category::Mapping ?
			clearStorageRangeFunction((_type.baseType()->storageBytes() < 32) ? *TypeProvider::uint256() : *_type.baseType()) :
			""
		)
		("lenToSize", arrayConvertLengthToSize(_type))
		("len", _type.length().str())
		.render();
	});
}

string YulUtilFunctions::clearStorageStructFunction(StructType const& _type)
{
	solAssert(_type.location() == DataLocation::Storage, "");

	string functionName = "clear_struct_storage_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&] {
		MemberList::MemberMap structMembers = _type.nativeMembers(nullptr);
		vector<map<string, string>> memberSetValues;

		set<u256> slotsCleared;
		for (auto const& member: structMembers)
		{
			if (member.type->category() == Type::Category::Mapping)
				continue;
			if (member.type->storageBytes() < 32)
			{
				auto const& slotDiff = _type.storageOffsetsOfMember(member.name).first;
				if (!slotsCleared.count(slotDiff))
				{
					memberSetValues.emplace_back().emplace("clearMember", "sstore(add(slot, " + slotDiff.str() + "), 0)");
					slotsCleared.emplace(slotDiff);
				}
			}
			else
			{
				auto const& [memberSlotDiff, memberStorageOffset] = _type.storageOffsetsOfMember(member.name);
				solAssert(memberStorageOffset == 0, "");

				memberSetValues.emplace_back().emplace("clearMember", Whiskers(R"(
						<setZero>(add(slot, <memberSlotDiff>), <memberStorageOffset>)
					)")
					("setZero", storageSetToZeroFunction(*member.type))
					("memberSlotDiff",  memberSlotDiff.str())
					("memberStorageOffset", to_string(memberStorageOffset))
					.render()
				);
			}
		}

		return Whiskers(R"(
			function <functionName>(slot) {
				<#member>
					<clearMember>
				</member>
			}
		)")
		("functionName", functionName)
		("member", memberSetValues)
		.render();
	});
}

string YulUtilFunctions::copyArrayToStorageFunction(ArrayType const& _fromType, ArrayType const& _toType)
{
	solAssert(
		*_fromType.copyForLocation(_toType.location(), _toType.isPointer()) == dynamic_cast<ReferenceType const&>(_toType),
		""
	);
	if (!_toType.isDynamicallySized())
		solAssert(!_fromType.isDynamicallySized() && _fromType.length() <= _toType.length(), "");

	if (_fromType.isByteArray())
		return copyByteArrayToStorageFunction(_fromType, _toType);
	if (_fromType.dataStoredIn(DataLocation::Storage) && _toType.baseType()->isValueType())
		return copyValueArrayStorageToStorageFunction(_fromType, _toType);

	string functionName = "copy_array_to_storage_from_" + _fromType.identifier() + "_to_" + _toType.identifier();
	return m_functionCollector.createFunction(functionName, [&](){
		Whiskers templ(R"(
			function <functionName>(slot, value<?isFromDynamicCalldata>, len</isFromDynamicCalldata>) {
				<?fromStorage> if eq(slot, value) { leave } </fromStorage>
				let length := <arrayLength>(value<?isFromDynamicCalldata>, len</isFromDynamicCalldata>)

				<resizeArray>(slot, length)

				let srcPtr := <srcDataLocation>(value)

				let elementSlot := <dstDataLocation>(slot)
				let elementOffset := 0

				for { let i := 0 } lt(i, length) {i := add(i, 1)} {
					<?fromCalldata>
						let <elementValues> :=
						<?dynamicallyEncodedBase>
							<accessCalldataTail>(value, srcPtr)
						<!dynamicallyEncodedBase>
							srcPtr
						</dynamicallyEncodedBase>

						<?isValueType>
							<elementValues> := <readFromCalldataOrMemory>(<elementValues>)
						</isValueType>
					</fromCalldata>

					<?fromMemory>
						let <elementValues> := <readFromCalldataOrMemory>(srcPtr)
					</fromMemory>

					<?fromStorage>
						let <elementValues> := srcPtr
					</fromStorage>

					<updateStorageValue>(elementSlot, elementOffset, <elementValues>)

					srcPtr := add(srcPtr, <srcStride>)

					<?multipleItemsPerSlot>
						elementOffset := add(elementOffset, <storageStride>)
						if gt(elementOffset, sub(32, <storageStride>)) {
							elementOffset := 0
							elementSlot := add(elementSlot, 1)
						}
					<!multipleItemsPerSlot>
						elementSlot := add(elementSlot, <storageSize>)
					</multipleItemsPerSlot>
				}
			}
		)");
		if (_fromType.dataStoredIn(DataLocation::Storage))
			solAssert(!_fromType.isValueType(), "");
		templ("functionName", functionName);
		bool fromCalldata = _fromType.dataStoredIn(DataLocation::CallData);
		templ("isFromDynamicCalldata", _fromType.isDynamicallySized() && fromCalldata);
		templ("fromStorage", _fromType.dataStoredIn(DataLocation::Storage));
		bool fromMemory = _fromType.dataStoredIn(DataLocation::Memory);
		templ("fromMemory", fromMemory);
		templ("fromCalldata", fromCalldata);
		templ("srcDataLocation", arrayDataAreaFunction(_fromType));
		if (fromCalldata)
		{
			templ("dynamicallyEncodedBase", _fromType.baseType()->isDynamicallyEncoded());
			if (_fromType.baseType()->isDynamicallyEncoded())
				templ("accessCalldataTail", accessCalldataTailFunction(*_fromType.baseType()));
		}
		templ("resizeArray", resizeArrayFunction(_toType));
		templ("arrayLength",arrayLengthFunction(_fromType));
		templ("isValueType", _fromType.baseType()->isValueType());
		templ("dstDataLocation", arrayDataAreaFunction(_toType));
		if (fromMemory || (fromCalldata && _fromType.baseType()->isValueType()))
			templ("readFromCalldataOrMemory", readFromMemoryOrCalldata(*_fromType.baseType(), fromCalldata));
		templ("elementValues", suffixedVariableNameList(
			"elementValue_",
			0,
			_fromType.baseType()->stackItems().size()
		));
		templ("updateStorageValue", updateStorageValueFunction(*_fromType.baseType(), *_toType.baseType()));
		templ("srcStride",
			fromCalldata ?
			to_string(_fromType.calldataStride()) :
				fromMemory ?
				to_string(_fromType.memoryStride()) :
				formatNumber(_fromType.baseType()->storageSize())
		);
		templ("multipleItemsPerSlot", _toType.storageStride() <= 16);
		templ("storageStride", to_string(_toType.storageStride()));
		templ("storageSize", _toType.baseType()->storageSize().str());

		return templ.render();
	});
}


string YulUtilFunctions::copyByteArrayToStorageFunction(ArrayType const& _fromType, ArrayType const& _toType)
{
	solAssert(
		*_fromType.copyForLocation(_toType.location(), _toType.isPointer()) == dynamic_cast<ReferenceType const&>(_toType),
		""
	);
	solAssert(_fromType.isByteArray(), "");
	solAssert(_toType.isByteArray(), "");

	string functionName = "copy_byte_array_to_storage_from_" + _fromType.identifier() + "_to_" + _toType.identifier();
	return m_functionCollector.createFunction(functionName, [&](){
		Whiskers templ(R"(
			function <functionName>(slot, src<?fromCalldata>, len</fromCalldata>) {
				<?fromStorage> if eq(slot, src) { leave } </fromStorage>

				let newLen := <arrayLength>(src<?fromCalldata>, len</fromCalldata>)
				// Make sure array length is sane
				if gt(newLen, 0xffffffffffffffff) { <panic>() }

				let oldLen := <byteArrayLength>(sload(slot))

				// potentially truncate data
				<cleanUpEndArray>(slot, oldLen, newLen)

				let srcOffset := 0
				<?fromMemory>
					srcOffset := 0x20
				</fromMemory>

				switch gt(newLen, 31)
				case 1 {
					let loopEnd := and(newLen, not(0x1f))
					<?fromStorage> src := <srcDataLocation>(src) </fromStorage>
					let dstPtr := <dstDataLocation>(slot)
					let i := 0
					for { } lt(i, loopEnd) { i := add(i, 0x20) } {
						sstore(dstPtr, <read>(add(src, srcOffset)))
						dstPtr := add(dstPtr, 1)
						srcOffset := add(srcOffset, <srcIncrement>)
					}
					if lt(loopEnd, newLen) {
						let lastValue := <read>(add(src, srcOffset))
						sstore(dstPtr, <maskBytes>(lastValue, and(newLen, 0x1f)))
					}
					sstore(slot, add(mul(newLen, 2), 1))
				}
				default {
					let value := 0
					if newLen {
						value := <read>(add(src, srcOffset))
					}
					sstore(slot, <byteArrayCombineShort>(value, newLen))
				}
			}
		)");
		templ("functionName", functionName);
		bool fromStorage = _fromType.dataStoredIn(DataLocation::Storage);
		templ("fromStorage", fromStorage);
		bool fromCalldata = _fromType.dataStoredIn(DataLocation::CallData);
		templ("fromMemory", _fromType.dataStoredIn(DataLocation::Memory));
		templ("fromCalldata", fromCalldata);
		templ("arrayLength", arrayLengthFunction(_fromType));
		templ("panic", panicFunction(PanicCode::ResourceError));
		templ("byteArrayLength", extractByteArrayLengthFunction());
		templ("dstDataLocation", arrayDataAreaFunction(_toType));
		if (fromStorage)
			templ("srcDataLocation", arrayDataAreaFunction(_fromType));
		templ("cleanUpEndArray", cleanUpDynamicByteArrayEndSlotsFunction(_toType));
		templ("srcIncrement", to_string(fromStorage ? 1 : 0x20));
		templ("read", fromStorage ? "sload" : fromCalldata ? "calldataload" : "mload");
		templ("maskBytes", maskBytesFunctionDynamic());
		templ("byteArrayCombineShort", shortByteArrayEncodeUsedAreaSetLengthFunction());

		return templ.render();
	});
}


string YulUtilFunctions::copyValueArrayStorageToStorageFunction(ArrayType const& _fromType, ArrayType const& _toType)
{
	solAssert(_fromType.baseType()->isValueType(), "");
	solAssert(_toType.baseType()->isValueType(), "");
	solAssert(_fromType.baseType()->isImplicitlyConvertibleTo(*_toType.baseType()), "");

	solAssert(!_fromType.isByteArray(), "");
	solAssert(!_toType.isByteArray(), "");
	solAssert(_fromType.dataStoredIn(DataLocation::Storage), "");
	solAssert(_toType.dataStoredIn(DataLocation::Storage), "");

	solAssert(_fromType.storageStride() <= _toType.storageStride(), "");
	solAssert(_toType.storageStride() <= 32, "");

	string functionName = "copy_array_to_storage_from_" + _fromType.identifier() + "_to_" + _toType.identifier();
	return m_functionCollector.createFunction(functionName, [&](){
		Whiskers templ(R"(
			function <functionName>(dst, src) {
				if eq(dst, src) { leave }
				let length := <arrayLength>(src)
				// Make sure array length is sane
				if gt(length, 0xffffffffffffffff) { <panic>() }
				<resizeArray>(dst, length)

				let srcSlot := <srcDataLocation>(src)
				let dstSlot := <dstDataLocation>(dst)

				let fullSlots := div(length, <itemsPerSlot>)

				let srcSlotValue := sload(srcSlot)
				let srcItemIndexInSlot := 0
				for { let i := 0 } lt(i, fullSlots) { i := add(i, 1) } {
					let dstSlotValue := 0
					<?sameType>
						dstSlotValue := <maskFull>(srcSlotValue)
						<updateSrcSlotValue>
					<!sameType>
						<?multipleItemsPerSlotDst>for { let j := 0 } lt(j, <itemsPerSlot>) { j := add(j, 1) } </multipleItemsPerSlotDst>
						{
							let itemValue := <convert>(
								<extractFromSlot>(srcSlotValue, mul(<srcStride>, srcItemIndexInSlot))
							)
							itemValue := <prepareStore>(itemValue)
							dstSlotValue :=
							<?multipleItemsPerSlotDst>
								<updateByteSlice>(dstSlotValue, mul(<dstStride>, j), itemValue)
							<!multipleItemsPerSlotDst>
								itemValue
							</multipleItemsPerSlotDst>

							<updateSrcSlotValue>
						}
					</sameType>

					sstore(add(dstSlot, i), dstSlotValue)
				}

				<?multipleItemsPerSlotDst>
					let spill := sub(length, mul(fullSlots, <itemsPerSlot>))
					if gt(spill, 0) {
						let dstSlotValue := 0
						<?sameType>
							dstSlotValue := <maskBytes>(srcSlotValue, mul(spill, <srcStride>))
							<updateSrcSlotValue>
						<!sameType>
							for { let j := 0 } lt(j, spill) { j := add(j, 1) } {
								let itemValue := <convert>(
									<extractFromSlot>(srcSlotValue, mul(<srcStride>, srcItemIndexInSlot))
								)
								itemValue := <prepareStore>(itemValue)
								dstSlotValue := <updateByteSlice>(dstSlotValue, mul(<dstStride>, j), itemValue)

								<updateSrcSlotValue>
							}
						</sameType>
						sstore(add(dstSlot, fullSlots), dstSlotValue)
					}
				</multipleItemsPerSlotDst>
			}
		)");
		if (_fromType.dataStoredIn(DataLocation::Storage))
			solAssert(!_fromType.isValueType(), "");
		templ("functionName", functionName);
		templ("resizeArray", resizeArrayFunction(_toType));
		templ("arrayLength",arrayLengthFunction(_fromType));
		templ("panic", panicFunction(PanicCode::ResourceError));
		templ("srcDataLocation", arrayDataAreaFunction(_fromType));
		templ("dstDataLocation", arrayDataAreaFunction(_toType));
		templ("srcStride", to_string(_fromType.storageStride()));
		unsigned itemsPerSlot = 32 / _toType.storageStride();
		templ("itemsPerSlot", to_string(itemsPerSlot));
		templ("multipleItemsPerSlotDst", itemsPerSlot > 1);
		bool sameType = _fromType.baseType() == _toType.baseType();
		templ("sameType", sameType);
		if (sameType)
		{
			templ("maskFull", maskLowerOrderBytesFunction(itemsPerSlot * _toType.storageStride()));
			templ("maskBytes", maskLowerOrderBytesFunctionDynamic());
		}
		else
		{
			templ("dstStride", to_string(_toType.storageStride()));
			templ("extractFromSlot", extractFromStorageValueDynamic(*_fromType.baseType()));
			templ("updateByteSlice", updateByteSliceFunctionDynamic(_toType.storageStride()));
			templ("convert", conversionFunction(*_fromType.baseType(), *_toType.baseType()));
			templ("prepareStore", prepareStoreFunction(*_toType.baseType()));
		}
		templ("updateSrcSlotValue", Whiskers(R"(
			<?srcReadMultiPerSlot>
				srcItemIndexInSlot := add(srcItemIndexInSlot, 1)
				if eq(srcItemIndexInSlot, <srcItemsPerSlot>) {
					// here we are done with this slot, we need to read next one
					srcSlot := add(srcSlot, 1)
					srcSlotValue := sload(srcSlot)
					srcItemIndexInSlot := 0
				}
			<!srcReadMultiPerSlot>
				srcSlot := add(srcSlot, 1)
				srcSlotValue := sload(srcSlot)
			</srcReadMultiPerSlot>
			)")
			("srcReadMultiPerSlot", !sameType && _fromType.storageStride() <= 16)
			("srcItemsPerSlot", to_string(32 / _fromType.storageStride()))
			.render()
		);

		return templ.render();
	});
}


string YulUtilFunctions::arrayConvertLengthToSize(ArrayType const& _type)
{
	string functionName = "array_convert_length_to_size_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
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
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers w(R"(
			function <functionName>(length) -> size {
				// Make sure we can allocate memory without overflow
				if gt(length, 0xffffffffffffffff) { <panic>() }
				<?byteArray>
					size := <roundUp>(length)
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
		w("panic", panicFunction(PanicCode::ResourceError));
		w("byteArray", _type.isByteArray());
		w("roundUp", roundUpFunction());
		w("dynamic", _type.isDynamicallySized());
		return w.render();
	});
}

string YulUtilFunctions::arrayDataAreaFunction(ArrayType const& _type)
{
	string functionName = "array_dataslot_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
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
	string functionName = "storage_array_index_access_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(array, index) -> slot, offset {
				let arrayLength := <arrayLen>(array)
				if iszero(lt(index, arrayLength)) { <panic>() }

				<?multipleItemsPerSlot>
					<?isBytesArray>
						switch lt(arrayLength, 0x20)
						case 0 {
							slot, offset := <indexAccessNoChecks>(array, index)
						}
						default {
							offset := sub(31, mod(index, 0x20))
							slot := array
						}
					<!isBytesArray>
						let dataArea := <dataAreaFunc>(array)
						slot := add(dataArea, div(index, <itemsPerSlot>))
						offset := mul(mod(index, <itemsPerSlot>), <storageBytes>)
					</isBytesArray>
				<!multipleItemsPerSlot>
					let dataArea := <dataAreaFunc>(array)
					slot := add(dataArea, mul(index, <storageSize>))
					offset := 0
				</multipleItemsPerSlot>
			}
		)")
		("functionName", functionName)
		("panic", panicFunction(PanicCode::ArrayOutOfBounds))
		("arrayLen", arrayLengthFunction(_type))
		("dataAreaFunc", arrayDataAreaFunction(_type))
		("indexAccessNoChecks", longByteArrayStorageIndexAccessNoCheckFunction())
		("multipleItemsPerSlot", _type.baseType()->storageBytes() <= 16)
		("isBytesArray", _type.isByteArray())
		("storageSize", _type.baseType()->storageSize().str())
		("storageBytes", toString(_type.baseType()->storageBytes()))
		("itemsPerSlot", to_string(32 / _type.baseType()->storageBytes()))
		.render();
	});
}

string YulUtilFunctions::memoryArrayIndexAccessFunction(ArrayType const& _type)
{
	string functionName = "memory_array_index_access_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(baseRef, index) -> addr {
				if iszero(lt(index, <arrayLen>(baseRef))) {
					<panic>()
				}

				let offset := mul(index, <stride>)
				<?dynamicallySized>
					offset := add(offset, 32)
				</dynamicallySized>
				addr := add(baseRef, offset)
			}
		)")
		("functionName", functionName)
		("panic", panicFunction(PanicCode::ArrayOutOfBounds))
		("arrayLen", arrayLengthFunction(_type))
		("stride", to_string(_type.memoryStride()))
		("dynamicallySized", _type.isDynamicallySized())
		.render();
	});
}

string YulUtilFunctions::calldataArrayIndexAccessFunction(ArrayType const& _type)
{
	solAssert(_type.dataStoredIn(DataLocation::CallData), "");
	string functionName = "calldata_array_index_access_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(base_ref<?dynamicallySized>, length</dynamicallySized>, index) -> addr<?dynamicallySizedBase>, len</dynamicallySizedBase> {
				if iszero(lt(index, <?dynamicallySized>length<!dynamicallySized><arrayLen></dynamicallySized>)) { <panic>() }
				addr := add(base_ref, mul(index, <stride>))
				<?dynamicallyEncodedBase>
					addr<?dynamicallySizedBase>, len</dynamicallySizedBase> := <accessCalldataTail>(base_ref, addr)
				</dynamicallyEncodedBase>
			}
		)")
		("functionName", functionName)
		("panic", panicFunction(PanicCode::ArrayOutOfBounds))
		("stride", to_string(_type.calldataStride()))
		("dynamicallySized", _type.isDynamicallySized())
		("dynamicallyEncodedBase", _type.baseType()->isDynamicallyEncoded())
		("dynamicallySizedBase", _type.baseType()->isDynamicallySized())
		("arrayLen",  toCompactHexWithPrefix(_type.length()))
		("accessCalldataTail", _type.baseType()->isDynamicallyEncoded() ? accessCalldataTailFunction(*_type.baseType()): "")
		.render();
	});
}

string YulUtilFunctions::calldataArrayIndexRangeAccess(ArrayType const& _type)
{
	solAssert(_type.dataStoredIn(DataLocation::CallData), "");
	solAssert(_type.isDynamicallySized(), "");
	string functionName = "calldata_array_index_range_access_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(offset, length, startIndex, endIndex) -> offsetOut, lengthOut {
				if gt(startIndex, endIndex) { <revertSliceStartAfterEnd>() }
				if gt(endIndex, length) { <revertSliceGreaterThanLength>() }
				offsetOut := add(offset, mul(startIndex, <stride>))
				lengthOut := sub(endIndex, startIndex)
			}
		)")
		("functionName", functionName)
		("stride", to_string(_type.calldataStride()))
		("revertSliceStartAfterEnd", revertReasonIfDebugFunction("Slice starts after end"))
		("revertSliceGreaterThanLength", revertReasonIfDebugFunction("Slice is greater than length"))
		.render();
	});
}

string YulUtilFunctions::accessCalldataTailFunction(Type const& _type)
{
	solAssert(_type.isDynamicallyEncoded(), "");
	solAssert(_type.dataStoredIn(DataLocation::CallData), "");
	string functionName = "access_calldata_tail_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(base_ref, ptr_to_tail) -> addr<?dynamicallySized>, length</dynamicallySized> {
				let rel_offset_of_tail := calldataload(ptr_to_tail)
				if iszero(slt(rel_offset_of_tail, sub(sub(calldatasize(), base_ref), sub(<neededLength>, 1)))) { <invalidCalldataTailOffset>() }
				addr := add(base_ref, rel_offset_of_tail)
				<?dynamicallySized>
					length := calldataload(addr)
					if gt(length, 0xffffffffffffffff) { <invalidCalldataTailLength>() }
					addr := add(addr, 32)
					if sgt(addr, sub(calldatasize(), mul(length, <calldataStride>))) { <shortCalldataTail>() }
				</dynamicallySized>
			}
		)")
		("functionName", functionName)
		("dynamicallySized", _type.isDynamicallySized())
		("neededLength", toCompactHexWithPrefix(_type.calldataEncodedTailSize()))
		("calldataStride", toCompactHexWithPrefix(_type.isDynamicallySized() ? dynamic_cast<ArrayType const&>(_type).calldataStride() : 0))
		("invalidCalldataTailOffset", revertReasonIfDebugFunction("Invalid calldata tail offset"))
		("invalidCalldataTailLength", revertReasonIfDebugFunction("Invalid calldata tail length"))
		("shortCalldataTail", revertReasonIfDebugFunction("Calldata tail too short"))
		.render();
	});
}

string YulUtilFunctions::nextArrayElementFunction(ArrayType const& _type)
{
	solAssert(!_type.isByteArray(), "");
	if (_type.dataStoredIn(DataLocation::Storage))
		solAssert(_type.baseType()->storageBytes() > 16, "");
	string functionName = "array_nextElement_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
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

string YulUtilFunctions::copyArrayFromStorageToMemoryFunction(ArrayType const& _from, ArrayType const& _to)
{
	solAssert(_from.dataStoredIn(DataLocation::Storage), "");
	solAssert(_to.dataStoredIn(DataLocation::Memory), "");
	solAssert(_from.isDynamicallySized() == _to.isDynamicallySized(), "");
	if (!_from.isDynamicallySized())
		solAssert(_from.length() == _to.length(), "");

	string functionName = "copy_array_from_storage_to_memory_" + _from.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		if (_from.baseType()->isValueType())
		{
			solAssert(_from.baseType() == _to.baseType(), "");
			ABIFunctions abi(m_evmVersion, m_revertStrings, m_functionCollector);
			return Whiskers(R"(
				function <functionName>(slot) -> memPtr {
					memPtr := <allocateUnbounded>()
					let end := <encode>(slot, memPtr)
					<finalizeAllocation>(memPtr, sub(end, memPtr))
				}
			)")
			("functionName", functionName)
			("allocateUnbounded", allocateUnboundedFunction())
			(
				"encode",
				abi.abiEncodeAndReturnUpdatedPosFunction(_from, _to, ABIFunctions::EncodingOptions{})
			)
			("finalizeAllocation", finalizeAllocationFunction())
			.render();
		}
		else
		{
			solAssert(_to.memoryStride() == 32, "");
			solAssert(_to.baseType()->dataStoredIn(DataLocation::Memory), "");
			solAssert(_from.baseType()->dataStoredIn(DataLocation::Storage), "");
			solAssert(!_from.isByteArray(), "");
			solAssert(*_to.withLocation(DataLocation::Storage, _from.isPointer()) == _from, "");
			return Whiskers(R"(
				function <functionName>(slot) -> memPtr {
					let length := <lengthFunction>(slot)
					memPtr := <allocateArray>(length)
					let mpos := memPtr
					<?dynamic>mpos := add(mpos, 0x20)</dynamic>
					let spos := <arrayDataArea>(slot)
					for { let i := 0 } lt(i, length) { i := add(i, 1) } {
						mstore(mpos, <convert>(spos))
						mpos := add(mpos, 0x20)
						spos := add(spos, <baseStorageSize>)
					}
				}
			)")
			("functionName", functionName)
			("lengthFunction", arrayLengthFunction(_from))
			("allocateArray", allocateMemoryArrayFunction(_to))
			("arrayDataArea", arrayDataAreaFunction(_from))
			("dynamic", _to.isDynamicallySized())
			("convert", conversionFunction(*_from.baseType(), *_to.baseType()))
			("baseStorageSize", _from.baseType()->storageSize().str())
			.render();
		}
	});
}

string YulUtilFunctions::bytesConcatFunction(vector<Type const*> const& _argumentTypes)
{
	string functionName = "bytes_concat";
	size_t totalParams = 0;
	vector<Type const*> targetTypes;
	for (Type const* argumentType: _argumentTypes)
	{
		solAssert(
			argumentType->isImplicitlyConvertibleTo(*TypeProvider::bytesMemory()) ||
			argumentType->isImplicitlyConvertibleTo(*TypeProvider::fixedBytes(32)),
			""
		);
		if (argumentType->category() == Type::Category::FixedBytes)
			targetTypes.emplace_back(argumentType);
		else if (
			auto const* literalType = dynamic_cast<StringLiteralType const*>(argumentType);
			literalType && !literalType->value().empty() && literalType->value().size() <= 32
		)
			targetTypes.emplace_back(TypeProvider::fixedBytes(static_cast<unsigned>(literalType->value().size())));
		else
		{
			solAssert(!dynamic_cast<RationalNumberType const*>(argumentType), "");
			solAssert(argumentType->isImplicitlyConvertibleTo(*TypeProvider::bytesMemory()), "");
			targetTypes.emplace_back(TypeProvider::bytesMemory());
		}

		totalParams += argumentType->sizeOnStack();
		functionName += "_" + argumentType->identifier();
	}

	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(<parameters>) -> outPtr {
				outPtr := <allocateUnbounded>()
				let dataStart := add(outPtr, 0x20)
				let dataEnd := <encodePacked>(dataStart<?+parameters>, <parameters></+parameters>)
				mstore(outPtr, sub(dataEnd, dataStart))
				<finalizeAllocation>(outPtr, sub(dataEnd, outPtr))
			}
		)");
		templ("functionName", functionName);
		templ("parameters", suffixedVariableNameList("param_", 0, totalParams));
		templ("allocateUnbounded", allocateUnboundedFunction());
		templ("finalizeAllocation", finalizeAllocationFunction());
		templ(
			"encodePacked",
			ABIFunctions{m_evmVersion, m_revertStrings, m_functionCollector}.tupleEncoderPacked(
				_argumentTypes,
				targetTypes
			)
		);
		return templ.render();
	});
}

string YulUtilFunctions::mappingIndexAccessFunction(MappingType const& _mappingType, Type const& _keyType)
{
	string functionName = "mapping_index_access_" + _mappingType.identifier() + "_of_" + _keyType.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		if (_mappingType.keyType()->isDynamicallySized())
			return Whiskers(R"(
				function <functionName>(slot <?+key>,</+key> <key>) -> dataSlot {
					dataSlot := <hash>(<key> <?+key>,</+key> slot)
				}
			)")
			("functionName", functionName)
			("key", suffixedVariableNameList("key_", 0, _keyType.sizeOnStack()))
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
	if (_type.isValueType())
		return readFromStorageValueType(_type, _offset, _splitFunctionTypes);
	else
	{
		solAssert(_offset == 0, "");
		return readFromStorageReferenceType(_type);
	}
}

string YulUtilFunctions::readFromStorageDynamic(Type const& _type, bool _splitFunctionTypes)
{
	if (_type.isValueType())
		return readFromStorageValueType(_type, {}, _splitFunctionTypes);
	string functionName =
		"read_from_storage__dynamic_" +
		string(_splitFunctionTypes ? "split_" : "") +
		_type.identifier();

	return m_functionCollector.createFunction(functionName, [&] {
		return Whiskers(R"(
			function <functionName>(slot, offset) -> value {
				if gt(offset, 0) { <panic>() }
				value := <readFromStorage>(slot)
			}
		)")
		("functionName", functionName)
		("panic", panicFunction(util::PanicCode::Generic))
		("readFromStorage", readFromStorageReferenceType(_type))
		.render();
	});
}

string YulUtilFunctions::readFromStorageValueType(Type const& _type, optional<size_t> _offset, bool _splitFunctionTypes)
{
	solAssert(_type.isValueType(), "");

	string functionName =
			"read_from_storage_" +
			string(_splitFunctionTypes ? "split_" : "") + (
				_offset.has_value() ?
				"offset_" + to_string(*_offset) :
				"dynamic"
			) +
			"_" +
			_type.identifier();

	return m_functionCollector.createFunction(functionName, [&] {
		Whiskers templ(R"(
			function <functionName>(slot<?dynamic>, offset</dynamic>) -> <?split>addr, selector<!split>value</split> {
				<?split>let</split> value := <extract>(sload(slot)<?dynamic>, offset</dynamic>)
				<?split>
					addr, selector := <splitFunction>(value)
				</split>
			}
		)");
		templ("functionName", functionName);
		templ("dynamic", !_offset.has_value());
		if (_offset.has_value())
			templ("extract", extractFromStorageValue(_type, *_offset));
		else
			templ("extract", extractFromStorageValueDynamic(_type));
		auto const* funType = dynamic_cast<FunctionType const*>(&_type);
		bool split = _splitFunctionTypes && funType && funType->kind() == FunctionType::Kind::External;
		templ("split", split);
		if (split)
			templ("splitFunction", splitExternalFunctionIdFunction());
		return templ.render();
	});
}

string YulUtilFunctions::readFromStorageReferenceType(Type const& _type)
{
	if (auto const* arrayType = dynamic_cast<ArrayType const*>(&_type))
	{
		solAssert(arrayType->dataStoredIn(DataLocation::Memory), "");
		return copyArrayFromStorageToMemoryFunction(
			dynamic_cast<ArrayType const&>(*arrayType->copyForLocation(DataLocation::Storage, false)),
			*arrayType
		);
	}
	solAssert(_type.category() == Type::Category::Struct, "");

	string functionName = "read_from_storage_reference_type_" + _type.identifier();

	auto const& structType = dynamic_cast<StructType const&>(_type);
	solAssert(structType.location() == DataLocation::Memory, "");
	MemberList::MemberMap structMembers = structType.nativeMembers(nullptr);
	vector<map<string, string>> memberSetValues(structMembers.size());
	for (size_t i = 0; i < structMembers.size(); ++i)
	{
		auto const& [memberSlotDiff, memberStorageOffset] = structType.storageOffsetsOfMember(structMembers[i].name);
		solAssert(structMembers[i].type->isValueType() || memberStorageOffset == 0, "");

		memberSetValues[i]["setMember"] = Whiskers(R"(
			{
				let <memberValues> := <readFromStorage>(add(slot, <memberSlotDiff>))
				<writeToMemory>(add(value, <memberMemoryOffset>), <memberValues>)
			}
		)")
		("memberValues", suffixedVariableNameList("memberValue_", 0, structMembers[i].type->stackItems().size()))
		("memberMemoryOffset", structType.memoryOffsetOfMember(structMembers[i].name).str())
		("memberSlotDiff",  memberSlotDiff.str())
		("readFromStorage", readFromStorage(*structMembers[i].type, memberStorageOffset, true))
		("writeToMemory", writeToMemoryFunction(*structMembers[i].type))
		.render();
	}

	return m_functionCollector.createFunction(functionName, [&] {
		return Whiskers(R"(
			function <functionName>(slot) -> value {
				value := <allocStruct>()
				<#member>
					<setMember>
				</member>
			}
		)")
		("functionName", functionName)
		("allocStruct", allocateMemoryStructFunction(structType))
		("member", memberSetValues)
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

string YulUtilFunctions::updateStorageValueFunction(
	Type const& _fromType,
	Type const& _toType,
	std::optional<unsigned> const& _offset
)
{
	string const functionName =
		"update_storage_value_" +
		(_offset.has_value() ? ("offset_" + to_string(*_offset)) : "") +
		_fromType.identifier() +
		"_to_" +
		_toType.identifier();

	return m_functionCollector.createFunction(functionName, [&] {
		if (_toType.isValueType())
		{
			solAssert(_fromType.isImplicitlyConvertibleTo(_toType), "");
			solAssert(_toType.storageBytes() <= 32, "Invalid storage bytes size.");
			solAssert(_toType.storageBytes() > 0, "Invalid storage bytes size.");

			return Whiskers(R"(
				function <functionName>(slot, <offset><fromValues>) {
					let <toValues> := <convert>(<fromValues>)
					sstore(slot, <update>(sload(slot), <offset><prepare>(<toValues>)))
				}

			)")
			("functionName", functionName)
			("update",
				_offset.has_value() ?
					updateByteSliceFunction(_toType.storageBytes(), *_offset) :
					updateByteSliceFunctionDynamic(_toType.storageBytes())
			)
			("offset", _offset.has_value() ? "" : "offset, ")
			("convert", conversionFunction(_fromType, _toType))
			("fromValues", suffixedVariableNameList("value_", 0, _fromType.sizeOnStack()))
			("toValues", suffixedVariableNameList("convertedValue_", 0, _toType.sizeOnStack()))
			("prepare", prepareStoreFunction(_toType))
			.render();
		}

		auto const* toReferenceType = dynamic_cast<ReferenceType const*>(&_toType);
		auto const* fromReferenceType = dynamic_cast<ReferenceType const*>(&_fromType);
		solAssert(toReferenceType, "");

		if (!fromReferenceType)
		{
			solAssert(_fromType.category() == Type::Category::StringLiteral, "");
			solAssert(toReferenceType->category() == Type::Category::Array, "");
			auto const& toArrayType = dynamic_cast<ArrayType const&>(*toReferenceType);
			solAssert(toArrayType.isByteArray(), "");

			return Whiskers(R"(
				function <functionName>(slot<?dynamicOffset>, offset</dynamicOffset>) {
					<?dynamicOffset>if offset { <panic>() }</dynamicOffset>
					<copyToStorage>(slot)
				}
			)")
			("functionName", functionName)
			("dynamicOffset", !_offset.has_value())
			("panic", panicFunction(PanicCode::Generic))
			("copyToStorage", copyLiteralToStorageFunction(dynamic_cast<StringLiteralType const&>(_fromType).value()))
			.render();
		}

		solAssert(*toReferenceType->copyForLocation(
			fromReferenceType->location(),
			fromReferenceType->isPointer()
		).get() == *fromReferenceType, "");

		if (fromReferenceType->category() == Type::Category::ArraySlice)
			solAssert(toReferenceType->category() == Type::Category::Array, "");
		else
			solAssert(toReferenceType->category() == fromReferenceType->category(), "");
		solAssert(_offset.value_or(0) == 0, "");

		Whiskers templ(R"(
			function <functionName>(slot, <?dynamicOffset>offset, </dynamicOffset><value>) {
				<?dynamicOffset>if offset { <panic>() }</dynamicOffset>
				<copyToStorage>(slot, <value>)
			}
		)");
		templ("functionName", functionName);
		templ("dynamicOffset", !_offset.has_value());
		templ("panic", panicFunction(PanicCode::Generic));
		templ("value", suffixedVariableNameList("value_", 0, _fromType.sizeOnStack()));
		if (_fromType.category() == Type::Category::Array)
			templ("copyToStorage", copyArrayToStorageFunction(
				dynamic_cast<ArrayType const&>(_fromType),
				dynamic_cast<ArrayType const&>(_toType)
			));
		else if (_fromType.category() == Type::Category::ArraySlice)
		{
			solAssert(
				_fromType.dataStoredIn(DataLocation::CallData),
				"Currently only calldata array slices are supported!"
			);
			templ("copyToStorage", copyArrayToStorageFunction(
				dynamic_cast<ArraySliceType const&>(_fromType).arrayType(),
				dynamic_cast<ArrayType const&>(_toType)
			));
		}
		else
			templ("copyToStorage", copyStructToStorageFunction(
				dynamic_cast<StructType const&>(_fromType),
				dynamic_cast<StructType const&>(_toType)
			));

		return templ.render();
	});
}

string YulUtilFunctions::writeToMemoryFunction(Type const& _type)
{
	string const functionName = "write_to_memory_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&] {
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

string YulUtilFunctions::extractFromStorageValueDynamic(Type const& _type)
{
	string functionName =
		"extract_from_storage_value_dynamic" +
		_type.identifier();
	return m_functionCollector.createFunction(functionName, [&] {
		return Whiskers(R"(
			function <functionName>(slot_value, offset) -> value {
				value := <cleanupStorage>(<shr>(mul(offset, 8), slot_value))
			}
		)")
		("functionName", functionName)
		("shr", shiftRightFunctionDynamic())
		("cleanupStorage", cleanupFromStorageFunction(_type))
		.render();
	});
}

string YulUtilFunctions::extractFromStorageValue(Type const& _type, size_t _offset)
{
	string functionName = "extract_from_storage_value_offset_" + to_string(_offset) + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&] {
		return Whiskers(R"(
			function <functionName>(slot_value) -> value {
				value := <cleanupStorage>(<shr>(slot_value))
			}
		)")
		("functionName", functionName)
		("shr", shiftRightFunction(_offset * 8))
		("cleanupStorage", cleanupFromStorageFunction(_type))
		.render();
	});
}

string YulUtilFunctions::cleanupFromStorageFunction(Type const& _type)
{
	solAssert(_type.isValueType(), "");

	string functionName = string("cleanup_from_storage_") + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&] {
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

		bool leftAligned = false;
		if (
			_type.category() != Type::Category::Function ||
			dynamic_cast<FunctionType const&>(_type).kind() == FunctionType::Kind::External
		)
			leftAligned = _type.leftAligned();

		if (storageBytes == 32)
			templ("cleaned", "value");
		else if (leftAligned)
			templ("cleaned", shiftLeftFunction(256 - 8 * storageBytes) + "(value)");
		else
			templ("cleaned", "and(value, " + toCompactHexWithPrefix((u256(1) << (8 * storageBytes)) - 1) + ")");

		return templ.render();
	});
}

string YulUtilFunctions::prepareStoreFunction(Type const& _type)
{
	string functionName = "prepare_store_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		solAssert(_type.isValueType(), "");
		auto const* funType = dynamic_cast<FunctionType const*>(&_type);
		if (funType && funType->kind() == FunctionType::Kind::External)
		{
			Whiskers templ(R"(
				function <functionName>(addr, selector) -> ret {
					ret := <prepareBytes>(<combine>(addr, selector))
				}
			)");
			templ("functionName", functionName);
			templ("prepareBytes", prepareStoreFunction(*TypeProvider::fixedBytes(24)));
			templ("combine", combineExternalFunctionIdFunction());
			return templ.render();
		}
		else
		{
			solAssert(_type.sizeOnStack() == 1, "");
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
		}
	});
}

string YulUtilFunctions::allocationFunction()
{
	string functionName = "allocate_memory";
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(size) -> memPtr {
				memPtr := <allocateUnbounded>()
				<finalizeAllocation>(memPtr, size)
			}
		)")
		("functionName", functionName)
		("allocateUnbounded", allocateUnboundedFunction())
		("finalizeAllocation", finalizeAllocationFunction())
		.render();
	});
}

string YulUtilFunctions::allocateUnboundedFunction()
{
	string functionName = "allocate_unbounded";
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>() -> memPtr {
				memPtr := mload(<freeMemoryPointer>)
			}
		)")
		("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer))
		("functionName", functionName)
		.render();
	});
}

string YulUtilFunctions::finalizeAllocationFunction()
{
	string functionName = "finalize_allocation";
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(memPtr, size) {
				let newFreePtr := add(memPtr, <roundUp>(size))
				// protect against overflow
				if or(gt(newFreePtr, 0xffffffffffffffff), lt(newFreePtr, memPtr)) { <panic>() }
				mstore(<freeMemoryPointer>, newFreePtr)
			}
		)")
		("functionName", functionName)
		("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer))
		("roundUp", roundUpFunction())
		("panic", panicFunction(PanicCode::ResourceError))
		.render();
	});
}

string YulUtilFunctions::zeroMemoryArrayFunction(ArrayType const& _type)
{
	if (_type.baseType()->hasSimpleZeroValueInMemory())
		return zeroMemoryFunction(*_type.baseType());
	return zeroComplexMemoryArrayFunction(_type);
}

string YulUtilFunctions::zeroMemoryFunction(Type const& _type)
{
	solAssert(_type.hasSimpleZeroValueInMemory(), "");

	string functionName = "zero_memory_chunk_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(dataStart, dataSizeInBytes) {
				calldatacopy(dataStart, calldatasize(), dataSizeInBytes)
			}
		)")
		("functionName", functionName)
		.render();
	});
}

string YulUtilFunctions::zeroComplexMemoryArrayFunction(ArrayType const& _type)
{
	solAssert(!_type.baseType()->hasSimpleZeroValueInMemory(), "");

	string functionName = "zero_complex_memory_array_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		solAssert(_type.memoryStride() == 32, "");
		return Whiskers(R"(
			function <functionName>(dataStart, dataSizeInBytes) {
				for {let i := 0} lt(i, dataSizeInBytes) { i := add(i, <stride>) } {
					mstore(add(dataStart, i), <zeroValue>())
				}
			}
		)")
		("functionName", functionName)
		("stride", to_string(_type.memoryStride()))
		("zeroValue", zeroValueFunction(*_type.baseType(), false))
		.render();
	});
}

string YulUtilFunctions::allocateMemoryArrayFunction(ArrayType const& _type)
{
	string functionName = "allocate_memory_array_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
				function <functionName>(length) -> memPtr {
					let allocSize := <allocSize>(length)
					memPtr := <alloc>(allocSize)
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

string YulUtilFunctions::allocateAndInitializeMemoryArrayFunction(ArrayType const& _type)
{
	string functionName = "allocate_and_zero_memory_array_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
				function <functionName>(length) -> memPtr {
					memPtr := <allocArray>(length)
					let dataStart := memPtr
					let dataSize := <allocSize>(length)
					<?dynamic>
					dataStart := add(dataStart, 32)
					dataSize := sub(dataSize, 32)
					</dynamic>
					<zeroArrayFunction>(dataStart, dataSize)
				}
			)")
			("functionName", functionName)
			("allocArray", allocateMemoryArrayFunction(_type))
			("allocSize", arrayAllocationSizeFunction(_type))
			("zeroArrayFunction", zeroMemoryArrayFunction(_type))
			("dynamic", _type.isDynamicallySized())
			.render();
	});
}

string YulUtilFunctions::allocateMemoryStructFunction(StructType const& _type)
{
	string functionName = "allocate_memory_struct_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers templ(R"(
		function <functionName>() -> memPtr {
			memPtr := <alloc>(<allocSize>)
		}
		)");
		templ("functionName", functionName);
		templ("alloc", allocationFunction());
		templ("allocSize", _type.memoryDataSize().str());

		return templ.render();
	});
}

string YulUtilFunctions::allocateAndInitializeMemoryStructFunction(StructType const& _type)
{
	string functionName = "allocate_and_zero_memory_struct_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers templ(R"(
		function <functionName>() -> memPtr {
			memPtr := <allocStruct>()
			let offset := memPtr
			<#member>
				mstore(offset, <zeroValue>())
				offset := add(offset, 32)
			</member>
		}
		)");
		templ("functionName", functionName);
		templ("allocStruct", allocateMemoryStructFunction(_type));

		TypePointers const& members = _type.memoryMemberTypes();

		vector<map<string, string>> memberParams(members.size());
		for (size_t i = 0; i < members.size(); ++i)
		{
			solAssert(members[i]->memoryHeadSize() == 32, "");
			memberParams[i]["zeroValue"] = zeroValueFunction(
				*TypeProvider::withLocationIfReference(DataLocation::Memory, members[i]),
				false
			);
		}
		templ("member", memberParams);
		return templ.render();
	});
}

string YulUtilFunctions::conversionFunction(Type const& _from, Type const& _to)
{
	if (_from.category() == Type::Category::Function)
	{
		solAssert(_to.category() == Type::Category::Function, "");
		FunctionType const& fromType = dynamic_cast<FunctionType const&>(_from);
		FunctionType const& targetType = dynamic_cast<FunctionType const&>(_to);
		solAssert(
			fromType.isImplicitlyConvertibleTo(targetType) &&
			fromType.sizeOnStack() == targetType.sizeOnStack() &&
			(fromType.kind() == FunctionType::Kind::Internal || fromType.kind() == FunctionType::Kind::External) &&
			fromType.kind() == targetType.kind(),
			"Invalid function type conversion requested."
		);
		string const functionName =
			"convert_" +
			_from.identifier() +
			"_to_" +
			_to.identifier();
		return m_functionCollector.createFunction(functionName, [&]() {
			return Whiskers(R"(
				function <functionName>(<?external>addr, </external>functionId) -> <?external>outAddr, </external>outFunctionId {
					<?external>outAddr := addr</external>
					outFunctionId := functionId
				}
			)")
			("functionName", functionName)
			("external", fromType.kind() == FunctionType::Kind::External)
			.render();
		});
	}
	else if (_from.category() == Type::Category::ArraySlice)
	{
		auto const& fromType = dynamic_cast<ArraySliceType const&>(_from);
		if (_to.category() == Type::Category::FixedBytes)
		{
			solAssert(fromType.arrayType().isByteArray(), "Array types other than bytes not convertible to bytesNN.");
			return bytesToFixedBytesConversionFunction(fromType.arrayType(), dynamic_cast<FixedBytesType const &>(_to));
		}
		solAssert(_to.category() == Type::Category::Array, "");
		auto const& targetType = dynamic_cast<ArrayType const&>(_to);

		solAssert(fromType.arrayType().isImplicitlyConvertibleTo(targetType), "");
		solAssert(
			fromType.arrayType().dataStoredIn(DataLocation::CallData) &&
			fromType.arrayType().isDynamicallySized() &&
			!fromType.arrayType().baseType()->isDynamicallyEncoded(),
			""
		);

		if (!targetType.dataStoredIn(DataLocation::CallData))
			return arrayConversionFunction(fromType.arrayType(), targetType);

		string const functionName =
			"convert_" +
			_from.identifier() +
			"_to_" +
			_to.identifier();
		return m_functionCollector.createFunction(functionName, [&]() {
			return Whiskers(R"(
				function <functionName>(offset, length) -> outOffset, outLength {
					outOffset := offset
					outLength := length
				}
			)")
			("functionName", functionName)
			.render();
		});
	}
	else if (_from.category() == Type::Category::Array)
	{
		auto const& fromArrayType =  dynamic_cast<ArrayType const&>(_from);
		if (_to.category() == Type::Category::FixedBytes)
		{
			solAssert(fromArrayType.isByteArray(), "Array types other than bytes not convertible to bytesNN.");
			return bytesToFixedBytesConversionFunction(fromArrayType, dynamic_cast<FixedBytesType const &>(_to));
		}
		solAssert(_to.category() == Type::Category::Array, "");
		return arrayConversionFunction(fromArrayType, dynamic_cast<ArrayType const&>(_to));
	}

	if (_from.sizeOnStack() != 1 || _to.sizeOnStack() != 1)
		return conversionFunctionSpecial(_from, _to);

	string functionName =
		"convert_" +
		_from.identifier() +
		"_to_" +
		_to.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
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
		case Type::Category::Contract:
			body =
				Whiskers("converted := <convert>(value)")
					("convert", conversionFunction(IntegerType(160), _to))
					.render();
			break;
		case Type::Category::Integer:
		case Type::Category::RationalNumber:
		{
			solAssert(_from.mobileType(), "");
			if (RationalNumberType const* rational = dynamic_cast<RationalNumberType const*>(&_from))
				if (rational->isFractional())
					solAssert(toCategory == Type::Category::FixedPoint, "");

			if (toCategory == Type::Category::Address || toCategory == Type::Category::Contract)
				body =
					Whiskers("converted := <convert>(value)")
					("convert", conversionFunction(_from, IntegerType(160)))
					.render();
			else
			{
				Whiskers bodyTemplate(R"(
					value := <cleanInput>(value)
					converted := <cleanOutput>(<convert>)
				)");
				bodyTemplate("cleanInput", cleanupFunction(_from));
				bodyTemplate("cleanOutput", cleanupFunction(_to));

				if (toCategory == Type::Category::FixedBytes)
				{
					FixedBytesType const& toBytesType = dynamic_cast<FixedBytesType const&>(_to);
					bodyTemplate("convert", shiftLeftFunction(256 - toBytesType.numBytes() * 8) + "(value)");
				}
				else if (toCategory == Type::Category::Enum || toCategory == Type::Category::Integer)
					bodyTemplate("convert", "value");
				else if (toCategory == Type::Category::FixedPoint)
					solUnimplemented("Not yet implemented - FixedPointType.");
				else
					solAssert(false, "");
				body = bodyTemplate.render();
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
		case Type::Category::Struct:
		{
			solAssert(toCategory == Type::Category::Struct, "");
			auto const& fromStructType = dynamic_cast<StructType const &>(_from);
			auto const& toStructType = dynamic_cast<StructType const &>(_to);
			solAssert(fromStructType.structDefinition() == toStructType.structDefinition(), "");

			if (fromStructType.location() == toStructType.location() && toStructType.isPointer())
				body = "converted := value";
			else
			{
				solUnimplementedAssert(toStructType.location() == DataLocation::Memory, "");
				solUnimplementedAssert(fromStructType.location() != DataLocation::Memory, "");

				if (fromStructType.location() == DataLocation::CallData)
					body = Whiskers(R"(
						converted := <abiDecode>(value, calldatasize())
					)")
					(
						"abiDecode",
						ABIFunctions(m_evmVersion, m_revertStrings, m_functionCollector).abiDecodingFunctionStruct(
							toStructType,
							false
						)
					).render();
				else
				{
					solAssert(fromStructType.location() == DataLocation::Storage, "");

					body = Whiskers(R"(
						converted := <readFromStorage>(value)
					)")
					("readFromStorage", readFromStorage(toStructType, 0, true))
					.render();
				}
			}

			break;
		}
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
		case Type::Category::TypeType:
		{
			TypeType const& typeType = dynamic_cast<decltype(typeType)>(_from);
			if (
				auto const* contractType = dynamic_cast<ContractType const*>(typeType.actualType());
				contractType->contractDefinition().isLibrary() &&
				_to == *TypeProvider::address()
			)
				body = "converted := value";
			else
				solAssert(false, "Invalid conversion from " + _from.canonicalName() + " to " + _to.canonicalName());
			break;
		}
		case Type::Category::Mapping:
		{
			solAssert(_from == _to, "");
			body = "converted := value";
			break;
		}
		default:
			solAssert(false, "Invalid conversion from " + _from.canonicalName() + " to " + _to.canonicalName());
		}

		solAssert(!body.empty(), _from.canonicalName() + " to " + _to.canonicalName());
		templ("body", body);
		return templ.render();
	});
}

string YulUtilFunctions::bytesToFixedBytesConversionFunction(ArrayType const& _from, FixedBytesType const& _to)
{
	solAssert(_from.isByteArray() && !_from.isString(), "");
	solAssert(_from.isDynamicallySized(), "");
	string functionName = "convert_bytes_to_fixedbytes_from_" + _from.identifier() + "_to_" + _to.identifier();
	return m_functionCollector.createFunction(functionName, [&](auto& _args, auto& _returnParams) {
		_args = { "array" };
		bool fromCalldata = _from.dataStoredIn(DataLocation::CallData);
		if (fromCalldata)
			_args.emplace_back("len");
		_returnParams = {"value"};
		Whiskers templ(R"(
			let length := <arrayLen>(array<?fromCalldata>, len</fromCalldata>)
			let dataArea := array
			<?fromMemory>
				dataArea := <dataArea>(array)
			</fromMemory>
			<?fromStorage>
				if gt(length, 31) { dataArea := <dataArea>(array) }
			</fromStorage>

			<?fromCalldata>
				value := <cleanup>(calldataload(dataArea))
			<!fromCalldata>
				value := <extractValue>(dataArea)
			</fromCalldata>

			if lt(length, <fixedBytesLen>) {
				value := and(
					value,
					<shl>(
						mul(8, sub(<fixedBytesLen>, length)),
						<mask>
					)
				)
			}
		)");
		templ("fromCalldata", fromCalldata);
		templ("arrayLen", arrayLengthFunction(_from));
		templ("fixedBytesLen", to_string(_to.numBytes()));
		templ("fromMemory", _from.dataStoredIn(DataLocation::Memory));
		templ("fromStorage", _from.dataStoredIn(DataLocation::Storage));
		templ("dataArea", arrayDataAreaFunction(_from));
		if (fromCalldata)
			templ("cleanup", cleanupFunction(_to));
		else
			templ(
				"extractValue",
				_from.dataStoredIn(DataLocation::Storage) ?
				readFromStorage(_to, 32 - _to.numBytes(), false) :
				readFromMemory(_to)
			);
		templ("shl", shiftLeftFunctionDynamic());
		templ("mask", formatNumber(~((u256(1) << (256 - _to.numBytes() * 8)) - 1)));
		return templ.render();
	});
}

string YulUtilFunctions::copyStructToStorageFunction(StructType const& _from, StructType const& _to)
{
	solAssert(_to.dataStoredIn(DataLocation::Storage), "");
	solAssert(_from.structDefinition() == _to.structDefinition(), "");

	string functionName =
		"copy_struct_to_storage_from_" +
		_from.identifier() +
		"_to_" +
		_to.identifier();

	return m_functionCollector.createFunction(functionName, [&](auto& _arguments, auto&) {
		_arguments = {"slot", "value"};
		Whiskers templ(R"(
			<?fromStorage> if iszero(eq(slot, value)) { </fromStorage>
			<#member>
			{
				<updateMemberCall>
			}
			</member>
			<?fromStorage> } </fromStorage>
		)");
		templ("fromStorage", _from.dataStoredIn(DataLocation::Storage));

		MemberList::MemberMap structMembers = _from.nativeMembers(nullptr);
		MemberList::MemberMap toStructMembers = _to.nativeMembers(nullptr);

		vector<map<string, string>> memberParams(structMembers.size());
		for (size_t i = 0; i < structMembers.size(); ++i)
		{
			Type const& memberType = *structMembers[i].type;
			solAssert(memberType.memoryHeadSize() == 32, "");
			auto const&[slotDiff, offset] = _to.storageOffsetsOfMember(structMembers[i].name);

			Whiskers t(R"(
				let memberSlot := add(slot, <memberStorageSlotDiff>)
				let memberSrcPtr := add(value, <memberOffset>)

				<?fromCalldata>
					let <memberValues> :=
						<?dynamicallyEncodedMember>
							<accessCalldataTail>(value, memberSrcPtr)
						<!dynamicallyEncodedMember>
							memberSrcPtr
						</dynamicallyEncodedMember>

					<?isValueType>
						<memberValues> := <read>(<memberValues>)
					</isValueType>
				</fromCalldata>

				<?fromMemory>
					let <memberValues> := <read>(memberSrcPtr)
				</fromMemory>

				<?fromStorage>
					let <memberValues> :=
						<?isValueType>
							<read>(memberSrcPtr)
						<!isValueType>
							memberSrcPtr
						</isValueType>
				</fromStorage>

				<updateStorageValue>(memberSlot, <memberValues>)
			)");
			bool fromCalldata = _from.location() == DataLocation::CallData;
			t("fromCalldata", fromCalldata);
			bool fromMemory = _from.location() == DataLocation::Memory;
			t("fromMemory", fromMemory);
			bool fromStorage = _from.location() == DataLocation::Storage;
			t("fromStorage", fromStorage);
			t("isValueType", memberType.isValueType());
			t("memberValues", suffixedVariableNameList("memberValue_", 0, memberType.stackItems().size()));

			t("memberStorageSlotDiff", slotDiff.str());
			if (fromCalldata)
			{
				t("memberOffset", to_string(_from.calldataOffsetOfMember(structMembers[i].name)));
				t("dynamicallyEncodedMember", memberType.isDynamicallyEncoded());
				if (memberType.isDynamicallyEncoded())
					t("accessCalldataTail", accessCalldataTailFunction(memberType));
				if (memberType.isValueType())
					t("read", readFromCalldata(memberType));
			}
			else if (fromMemory)
			{
				t("memberOffset", _from.memoryOffsetOfMember(structMembers[i].name).str());
				t("read", readFromMemory(memberType));
			}
			else if (fromStorage)
			{
				auto const& [srcSlotOffset, srcOffset] = _from.storageOffsetsOfMember(structMembers[i].name);
				t("memberOffset", formatNumber(srcSlotOffset));
				if (memberType.isValueType())
					t("read", readFromStorageValueType(memberType, srcOffset, false));
				else
					solAssert(srcOffset == 0, "");

			}
			t("updateStorageValue", updateStorageValueFunction(
				memberType,
				*toStructMembers[i].type,
				optional<unsigned>{offset}
			));
			memberParams[i]["updateMemberCall"] = t.render();
		}
		templ("member", memberParams);

		return templ.render();
	});
}

string YulUtilFunctions::arrayConversionFunction(ArrayType const& _from, ArrayType const& _to)
{
	if (_to.dataStoredIn(DataLocation::CallData))
		solAssert(
			_from.dataStoredIn(DataLocation::CallData) && _from.isByteArray() && _to.isByteArray(),
			""
		);

	// Other cases are done explicitly in LValue::storeValue, and only possible by assignment.
	if (_to.location() == DataLocation::Storage)
		solAssert(
			(_to.isPointer() || (_from.isByteArray() && _to.isByteArray())) &&
			_from.location() == DataLocation::Storage,
			"Invalid conversion to storage type."
		);

	string functionName =
		"convert_array_" +
		_from.identifier() +
		"_to_" +
		_to.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(value<?fromCalldataDynamic>, length</fromCalldataDynamic>) -> converted <?toCalldataDynamic>, outLength</toCalldataDynamic> {
				<body>
				<?toCalldataDynamic>
					outLength := <length>
				</toCalldataDynamic>
			}
		)");
		templ("functionName", functionName);
		templ("fromCalldataDynamic", _from.dataStoredIn(DataLocation::CallData) && _from.isDynamicallySized());
		templ("toCalldataDynamic", _to.dataStoredIn(DataLocation::CallData) && _to.isDynamicallySized());
		templ("length", _from.isDynamicallySized() ? "length" : _from.length().str());

		if (
			_from == _to ||
			(_from.dataStoredIn(DataLocation::Memory) && _to.dataStoredIn(DataLocation::Memory)) ||
			(_from.dataStoredIn(DataLocation::CallData) && _to.dataStoredIn(DataLocation::CallData)) ||
			_to.dataStoredIn(DataLocation::Storage)
		)
			templ("body", "converted := value");
		else if (_to.dataStoredIn(DataLocation::Memory))
			templ(
				"body",
				Whiskers(R"(
					// Copy the array to a free position in memory
					converted :=
					<?fromStorage>
						<arrayStorageToMem>(value)
					</fromStorage>
					<?fromCalldata>
						<abiDecode>(value, <length>, calldatasize())
					</fromCalldata>
				)")
				("fromStorage", _from.dataStoredIn(DataLocation::Storage))
				("fromCalldata", _from.dataStoredIn(DataLocation::CallData))
				("length", _from.isDynamicallySized() ? "length" : _from.length().str())
				(
					"abiDecode",
					_from.dataStoredIn(DataLocation::CallData) ?
					ABIFunctions(
						m_evmVersion,
						m_revertStrings,
						m_functionCollector
					).abiDecodingFunctionArrayAvailableLength(_to, false) :
					""
				)
				(
					"arrayStorageToMem",
					_from.dataStoredIn(DataLocation::Storage) ? copyArrayFromStorageToMemoryFunction(_from, _to) : ""
				)
				.render()
			);
		else
			solAssert(false, "");

		return templ.render();
	});
}

string YulUtilFunctions::cleanupFunction(Type const& _type)
{
	string functionName = string("cleanup_") + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
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
			switch (dynamic_cast<FunctionType const&>(_type).kind())
			{
				case FunctionType::Kind::External:
					templ("body", "cleaned := " + cleanupFunction(FixedBytesType(24)) + "(value)");
					break;
				case FunctionType::Kind::Internal:
					templ("body", "cleaned := value");
					break;
				default:
					solAssert(false, "");
					break;
			}
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
			templ("body", "cleaned := value " + validatorFunction(_type, false) + "(value)");
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
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(value) {
				if iszero(<condition>) { <failure> }
			}
		)");
		templ("functionName", functionName);
		PanicCode panicCode = PanicCode::Generic;

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
			panicCode = PanicCode::EnumConversionError;
			templ("condition", "lt(value, " + to_string(members) + ")");
			break;
		}
		case Type::Category::InaccessibleDynamic:
			templ("condition", "1");
			break;
		default:
			solAssert(false, "Validation of type " + _type.identifier() + " requested.");
		}

		if (_revertOnFailure)
			templ("failure", "revert(0, 0)");
		else
			templ("failure", panicFunction(panicCode) + "()");

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
	return m_functionCollector.createFunction(functionName, [&]() {
		Whiskers templ(R"(
			function <functionName>(<variables>) -> hash {
				let pos := <allocateUnbounded>()
				let end := <packedEncode>(pos <comma> <variables>)
				hash := keccak256(pos, sub(end, pos))
			}
		)");
		templ("functionName", functionName);
		templ("variables", suffixedVariableNameList("var_", 1, 1 + sizeOnStack));
		templ("comma", sizeOnStack > 0 ? "," : "");
		templ("allocateUnbounded", allocateUnboundedFunction());
		templ("packedEncode", ABIFunctions(m_evmVersion, m_revertStrings, m_functionCollector).tupleEncoderPacked(_givenTypes, _targetTypes));
		return templ.render();
	});
}

string YulUtilFunctions::forwardingRevertFunction()
{
	bool forward = m_evmVersion.supportsReturndata();
	string functionName = "revert_forward_" + to_string(forward);
	return m_functionCollector.createFunction(functionName, [&]() {
		if (forward)
			return Whiskers(R"(
				function <functionName>() {
					let pos := <allocateUnbounded>()
					returndatacopy(pos, 0, returndatasize())
					revert(pos, returndatasize())
				}
			)")
			("functionName", functionName)
			("allocateUnbounded", allocateUnboundedFunction())
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
	solAssert(_type.category() == Type::Category::Integer, "");
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);

	string const functionName = "decrement_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(value) -> ret {
				value := <cleanupFunction>(value)
				if eq(value, <minval>) { <panic>() }
				ret := sub(value, 1)
			}
		)")
		("functionName", functionName)
		("panic", panicFunction(PanicCode::UnderOverflow))
		("minval", toCompactHexWithPrefix(type.min()))
		("cleanupFunction", cleanupFunction(_type))
		.render();
	});
}

std::string YulUtilFunctions::decrementWrappingFunction(Type const& _type)
{
	solAssert(_type.category() == Type::Category::Integer, "");
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);

	string const functionName = "decrement_wrapping_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(value) -> ret {
				ret := <cleanupFunction>(sub(value, 1))
			}
		)")
		("functionName", functionName)
		("cleanupFunction", cleanupFunction(type))
		.render();
	});
}

std::string YulUtilFunctions::incrementCheckedFunction(Type const& _type)
{
	solAssert(_type.category() == Type::Category::Integer, "");
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);

	string const functionName = "increment_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(value) -> ret {
				value := <cleanupFunction>(value)
				if eq(value, <maxval>) { <panic>() }
				ret := add(value, 1)
			}
		)")
		("functionName", functionName)
		("maxval", toCompactHexWithPrefix(type.max()))
		("panic", panicFunction(PanicCode::UnderOverflow))
		("cleanupFunction", cleanupFunction(_type))
		.render();
	});
}

std::string YulUtilFunctions::incrementWrappingFunction(Type const& _type)
{
	solAssert(_type.category() == Type::Category::Integer, "");
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);

	string const functionName = "increment_wrapping_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(value) -> ret {
				ret := <cleanupFunction>(add(value, 1))
			}
		)")
		("functionName", functionName)
		("cleanupFunction", cleanupFunction(type))
		.render();
	});
}

string YulUtilFunctions::negateNumberCheckedFunction(Type const& _type)
{
	solAssert(_type.category() == Type::Category::Integer, "");
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	solAssert(type.isSigned(), "Expected signed type!");

	string const functionName = "negate_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(value) -> ret {
				value := <cleanupFunction>(value)
				if eq(value, <minval>) { <panic>() }
				ret := sub(0, value)
			}
		)")
		("functionName", functionName)
		("minval", toCompactHexWithPrefix(type.min()))
		("cleanupFunction", cleanupFunction(_type))
		("panic", panicFunction(PanicCode::UnderOverflow))
		.render();
	});
}

string YulUtilFunctions::negateNumberWrappingFunction(Type const& _type)
{
	solAssert(_type.category() == Type::Category::Integer, "");
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	solAssert(type.isSigned(), "Expected signed type!");

	string const functionName = "negate_wrapping_" + _type.identifier();
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>(value) -> ret {
				ret := <cleanupFunction>(sub(0, value))
			}
		)")
		("functionName", functionName)
		("cleanupFunction", cleanupFunction(type))
		.render();
	});
}

string YulUtilFunctions::zeroValueFunction(Type const& _type, bool _splitFunctionTypes)
{
	solAssert(_type.category() != Type::Category::Mapping, "");

	string const functionName = "zero_value_for_" + string(_splitFunctionTypes ? "split_" : "") + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		FunctionType const* fType = dynamic_cast<FunctionType const*>(&_type);
		if (fType && fType->kind() == FunctionType::Kind::External && _splitFunctionTypes)
			return Whiskers(R"(
				function <functionName>() -> retAddress, retFunction {
					retAddress := 0
					retFunction := 0
				}
			)")
			("functionName", functionName)
			.render();

		if (_type.dataStoredIn(DataLocation::CallData))
		{
			solAssert(
				_type.category() == Type::Category::Struct ||
				_type.category() == Type::Category::Array,
			"");
			Whiskers templ(R"(
				function <functionName>() -> offset<?hasLength>, length</hasLength> {
					offset := calldatasize()
					<?hasLength> length := 0 </hasLength>
				}
			)");
			templ("functionName", functionName);
			templ("hasLength",
				_type.category() == Type::Category::Array &&
				dynamic_cast<ArrayType const&>(_type).isDynamicallySized()
			);

			return templ.render();
		}

		Whiskers templ(R"(
			function <functionName>() -> ret {
				ret := <zeroValue>
			}
		)");
		templ("functionName", functionName);

		if (_type.isValueType())
		{
			solAssert((
				_type.hasSimpleZeroValueInMemory() ||
				(fType && (fType->kind() == FunctionType::Kind::Internal || fType->kind() == FunctionType::Kind::External))
			), "");
			templ("zeroValue", "0");
		}
		else
		{
			solAssert(_type.dataStoredIn(DataLocation::Memory), "");
			if (auto const* arrayType = dynamic_cast<ArrayType const*>(&_type))
			{
				if (_type.isDynamicallySized())
					templ("zeroValue", to_string(CompilerUtils::zeroPointer));
				else
					templ("zeroValue", allocateAndInitializeMemoryArrayFunction(*arrayType) + "(" + to_string(unsigned(arrayType->length())) + ")");

			}
			else if (auto const* structType = dynamic_cast<StructType const*>(&_type))
				templ("zeroValue", allocateAndInitializeMemoryStructFunction(*structType) + "()");
			else
				solUnimplementedAssert(false, "");
		}

		return templ.render();
	});
}

string YulUtilFunctions::storageSetToZeroFunction(Type const& _type)
{
	string const functionName = "storage_set_to_zero_" + _type.identifier();

	return m_functionCollector.createFunction(functionName, [&]() {
		if (_type.isValueType())
			return Whiskers(R"(
				function <functionName>(slot, offset) {
					let <values> := <zeroValue>()
					<store>(slot, offset, <values>)
				}
			)")
			("functionName", functionName)
			("store", updateStorageValueFunction(_type, _type))
			("values", suffixedVariableNameList("zero_", 0, _type.sizeOnStack()))
			("zeroValue", zeroValueFunction(_type))
			.render();
		else if (_type.category() == Type::Category::Array)
			return Whiskers(R"(
				function <functionName>(slot, offset) {
					if iszero(eq(offset, 0)) { <panic>() }
					<clearArray>(slot)
				}
			)")
			("functionName", functionName)
			("clearArray", clearStorageArrayFunction(dynamic_cast<ArrayType const&>(_type)))
			("panic", panicFunction(PanicCode::Generic))
			.render();
		else if (_type.category() == Type::Category::Struct)
			return Whiskers(R"(
				function <functionName>(slot, offset) {
					if iszero(eq(offset, 0)) { <panic>() }
					<clearStruct>(slot)
				}
			)")
			("functionName", functionName)
			("clearStruct", clearStorageStructFunction(dynamic_cast<StructType const&>(_type)))
			("panic", panicFunction(PanicCode::Generic))
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
	return m_functionCollector.createFunction(functionName, [&]() {
		if (
			auto fromTuple = dynamic_cast<TupleType const*>(&_from), toTuple = dynamic_cast<TupleType const*>(&_to);
			fromTuple && toTuple && fromTuple->components().size() == toTuple->components().size()
		)
		{
			size_t sourceStackSize = 0;
			size_t destStackSize = 0;
			std::string conversions;
			for (size_t i = 0; i < fromTuple->components().size(); ++i)
			{
				auto fromComponent = fromTuple->components()[i];
				auto toComponent = toTuple->components()[i];
				solAssert(fromComponent, "");
				if (toComponent)
				{
					conversions +=
						suffixedVariableNameList("converted", destStackSize, destStackSize + toComponent->sizeOnStack()) +
						(toComponent->sizeOnStack() > 0 ? " := " : "") +
						conversionFunction(*fromComponent, *toComponent) +
						"(" +
						suffixedVariableNameList("value", sourceStackSize, sourceStackSize + fromComponent->sizeOnStack()) +
						")\n";
					destStackSize += toComponent->sizeOnStack();
				}
				sourceStackSize += fromComponent->sizeOnStack();
			}
			return Whiskers(R"(
				function <functionName>(<values>) <arrow> <converted> {
					<conversions>
				}
			)")
			("functionName", functionName)
			("values", suffixedVariableNameList("value", 0, sourceStackSize))
			("arrow", destStackSize > 0 ? "->" : "")
			("converted", suffixedVariableNameList("converted", 0, destStackSize))
			("conversions", conversions)
			.render();
		}

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
			solAssert(dynamic_cast<ArrayType const&>(_to).isByteArray(), "");
			Whiskers templ(R"(
				function <functionName>() -> converted {
					converted := <copyLiteralToMemory>()
				}
			)");
			templ("functionName", functionName);
			templ("copyLiteralToMemory", copyLiteralToMemoryFunction(data));
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

	return m_functionCollector.createFunction(functionName, [&] {
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
		Whiskers templ(R"(
			function <functionName>(ptr) -> <returnVariables> {
				<?fromCalldata>
					let value := calldataload(ptr)
					<validate>(value)
				<!fromCalldata>
					let value := <cleanup>(mload(ptr))
				</fromCalldata>

				<returnVariables> :=
				<?externalFunction>
					<splitFunction>(value)
				<!externalFunction>
					value
				</externalFunction>
			}
		)");
		templ("functionName", functionName);
		templ("fromCalldata", _fromCalldata);
		if (_fromCalldata)
			templ("validate", validatorFunction(_type, true));
		auto const* funType = dynamic_cast<FunctionType const*>(&_type);
		if (funType && funType->kind() == FunctionType::Kind::External)
		{
			templ("externalFunction", true);
			templ("splitFunction", splitExternalFunctionIdFunction());
			templ("returnVariables", "addr, selector");
		}
		else
		{
			templ("externalFunction", false);
			templ("returnVariables", "returnValue");
		}

		// Byte array elements generally need cleanup.
		// Other types are cleaned as well to account for dirty memory e.g. due to inline assembly.
		templ("cleanup", cleanupFunction(_type));
		return templ.render();
	});
}

string YulUtilFunctions::revertReasonIfDebugFunction(string const& _message)
{
	string functionName = "revert_error_" + util::toHex(util::keccak256(_message).asBytes());
	return m_functionCollector.createFunction(functionName, [&](auto&, auto&) -> string {
		return revertReasonIfDebugBody(m_revertStrings, allocateUnboundedFunction() + "()", _message);
	});
}

string YulUtilFunctions::revertReasonIfDebugBody(
	RevertStrings _revertStrings,
	string const& _allocation,
	string const& _message
)
{
	if (_revertStrings < RevertStrings::Debug || _message.empty())
		return "revert(0, 0)";

	Whiskers templ(R"(
		let start := <allocate>
		let pos := start
		mstore(pos, <sig>)
		pos := add(pos, 4)
		mstore(pos, 0x20)
		pos := add(pos, 0x20)
		mstore(pos, <length>)
		pos := add(pos, 0x20)
		<#word>
			mstore(add(pos, <offset>), <wordValue>)
		</word>
		revert(start, <overallLength>)
	)");
	templ("allocate", _allocation);
	templ("sig", util::selectorFromSignature("Error(string)").str());
	templ("length", to_string(_message.length()));

	size_t words = (_message.length() + 31) / 32;
	vector<map<string, string>> wordParams(words);
	for (size_t i = 0; i < words; ++i)
	{
		wordParams[i]["offset"] = to_string(i * 32);
		wordParams[i]["wordValue"] = formatAsStringOrNumber(_message.substr(32 * i, 32));
	}
	templ("word", wordParams);
	templ("overallLength", to_string(4 + 0x20 + 0x20 + words * 32));

	return templ.render();
}

string YulUtilFunctions::panicFunction(util::PanicCode _code)
{
	string functionName = "panic_error_" + toCompactHexWithPrefix(uint64_t(_code));
	return m_functionCollector.createFunction(functionName, [&]() {
		return Whiskers(R"(
			function <functionName>() {
				mstore(0, <selector>)
				mstore(4, <code>)
				revert(0, 0x24)
			}
		)")
		("functionName", functionName)
		("selector", util::selectorFromSignature("Panic(uint256)").str())
		("code", toCompactHexWithPrefix(static_cast<unsigned>(_code)))
		.render();
	});
}

string YulUtilFunctions::returnDataSelectorFunction()
{
	string const functionName = "return_data_selector";
	solAssert(m_evmVersion.supportsReturndata(), "");

	return m_functionCollector.createFunction(functionName, [&]() {
		return util::Whiskers(R"(
			function <functionName>() -> sig {
				if gt(returndatasize(), 3) {
					returndatacopy(0, 0, 4)
					sig := <shr224>(mload(0))
				}
			}
		)")
		("functionName", functionName)
		("shr224", shiftRightFunction(224))
		.render();
	});
}

string YulUtilFunctions::tryDecodeErrorMessageFunction()
{
	string const functionName = "try_decode_error_message";
	solAssert(m_evmVersion.supportsReturndata(), "");

	return m_functionCollector.createFunction(functionName, [&]() {
		return util::Whiskers(R"(
			function <functionName>() -> ret {
				if lt(returndatasize(), 0x44) { leave }

				let data := <allocateUnbounded>()
				returndatacopy(data, 4, sub(returndatasize(), 4))

				let offset := mload(data)
				if or(
					gt(offset, 0xffffffffffffffff),
					gt(add(offset, 0x24), returndatasize())
				) {
					leave
				}

				let msg := add(data, offset)
				let length := mload(msg)
				if gt(length, 0xffffffffffffffff) { leave }

				let end := add(add(msg, 0x20), length)
				if gt(end, add(data, sub(returndatasize(), 4))) { leave }

				<finalizeAllocation>(data, add(offset, add(0x20, length)))
				ret := msg
			}
		)")
		("functionName", functionName)
		("allocateUnbounded", allocateUnboundedFunction())
		("finalizeAllocation", finalizeAllocationFunction())
		.render();
	});
}

string YulUtilFunctions::tryDecodePanicDataFunction()
{
	string const functionName = "try_decode_panic_data";
	solAssert(m_evmVersion.supportsReturndata(), "");

	return m_functionCollector.createFunction(functionName, [&]() {
		return util::Whiskers(R"(
			function <functionName>() -> success, data {
				if gt(returndatasize(), 0x23) {
					returndatacopy(0, 4, 0x20)
					success := 1
					data := mload(0)
				}
			}
		)")
		("functionName", functionName)
		.render();
	});
}

string YulUtilFunctions::extractReturndataFunction()
{
	string const functionName = "extract_returndata";

	return m_functionCollector.createFunction(functionName, [&]() {
		return util::Whiskers(R"(
			function <functionName>() -> data {
				<?supportsReturndata>
					switch returndatasize()
					case 0 {
						data := <emptyArray>()
					}
					default {
						data := <allocateArray>(returndatasize())
						returndatacopy(add(data, 0x20), 0, returndatasize())
					}
				<!supportsReturndata>
					data := <emptyArray>()
				</supportsReturndata>
			}
		)")
		("functionName", functionName)
		("supportsReturndata", m_evmVersion.supportsReturndata())
		("allocateArray", allocateMemoryArrayFunction(*TypeProvider::bytesMemory()))
		("emptyArray", zeroValueFunction(*TypeProvider::bytesMemory()))
		.render();
	});
}

string YulUtilFunctions::copyConstructorArgumentsToMemoryFunction(
	ContractDefinition const& _contract,
	string const& _creationObjectName
)
{
	string functionName = "copy_arguments_for_constructor_" +
		toString(_contract.constructor()->id()) +
		"_object_" +
		_contract.name() +
		"_" +
		toString(_contract.id());

	return m_functionCollector.createFunction(functionName, [&]() {
		string returnParams = suffixedVariableNameList("ret_param_",0, CompilerUtils::sizeOnStack(_contract.constructor()->parameters()));
		ABIFunctions abiFunctions(m_evmVersion, m_revertStrings, m_functionCollector);

		return util::Whiskers(R"(
			function <functionName>() -> <retParams> {
				let programSize := datasize("<object>")
				let argSize := sub(codesize(), programSize)

				let memoryDataOffset := <allocate>(argSize)
				codecopy(memoryDataOffset, programSize, argSize)

				<retParams> := <abiDecode>(memoryDataOffset, add(memoryDataOffset, argSize))
			}
		)")
		("functionName", functionName)
		("retParams", returnParams)
		("object", _creationObjectName)
		("allocate", allocationFunction())
		("abiDecode", abiFunctions.tupleDecoder(FunctionType(*_contract.constructor()).parameterTypes(), true))
		.render();
	});
}

string YulUtilFunctions::externalCodeFunction()
{
	string functionName = "external_code_at";

	return m_functionCollector.createFunction(functionName, [&]() {
		return util::Whiskers(R"(
			function <functionName>(addr) -> mpos {
				let length := extcodesize(addr)
				mpos := <allocateArray>(length)
				extcodecopy(addr, add(mpos, 0x20), 0, length)
			}
		)")
		("functionName", functionName)
		("allocateArray", allocateMemoryArrayFunction(*TypeProvider::bytesMemory()))
		.render();
	});
}
