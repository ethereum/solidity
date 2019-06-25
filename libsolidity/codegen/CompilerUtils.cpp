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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Routines used by both the compiler and the expression compiler.
 */

#include <libsolidity/codegen/CompilerUtils.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/ArrayUtils.h>
#include <libsolidity/codegen/LValue.h>
#include <libevmasm/Instruction.h>
#include <libdevcore/Whiskers.h>

using namespace std;
using namespace langutil;
using namespace dev;
using namespace dev::eth;
using namespace dev::solidity;

unsigned const CompilerUtils::dataStartOffset = 4;
size_t const CompilerUtils::freeMemoryPointer = 64;
size_t const CompilerUtils::zeroPointer = CompilerUtils::freeMemoryPointer + 32;
size_t const CompilerUtils::generalPurposeMemoryStart = CompilerUtils::zeroPointer + 32;

static_assert(CompilerUtils::freeMemoryPointer >= 64, "Free memory pointer must not overlap with scratch area.");
static_assert(CompilerUtils::zeroPointer >= CompilerUtils::freeMemoryPointer + 32, "Zero pointer must not overlap with free memory pointer.");
static_assert(CompilerUtils::generalPurposeMemoryStart >= CompilerUtils::zeroPointer + 32, "General purpose memory must not overlap with zero area.");

void CompilerUtils::initialiseFreeMemoryPointer()
{
	m_context << u256(generalPurposeMemoryStart);
	storeFreeMemoryPointer();
}

void CompilerUtils::fetchFreeMemoryPointer()
{
	m_context << u256(freeMemoryPointer) << Instruction::MLOAD;
}

void CompilerUtils::storeFreeMemoryPointer()
{
	m_context << u256(freeMemoryPointer) << Instruction::MSTORE;
}

void CompilerUtils::allocateMemory()
{
	fetchFreeMemoryPointer();
	m_context << Instruction::SWAP1 << Instruction::DUP2 << Instruction::ADD;
	storeFreeMemoryPointer();
}

void CompilerUtils::allocateMemory(u256 const& size)
{
	fetchFreeMemoryPointer();
	m_context << Instruction::DUP1 << size << Instruction::ADD;
	storeFreeMemoryPointer();
}

void CompilerUtils::toSizeAfterFreeMemoryPointer()
{
	fetchFreeMemoryPointer();
	m_context << Instruction::DUP1 << Instruction::SWAP2 << Instruction::SUB;
	m_context << Instruction::SWAP1;
}

void CompilerUtils::revertWithStringData(Type const& _argumentType)
{
	solAssert(_argumentType.isImplicitlyConvertibleTo(*TypeProvider::fromElementaryTypeName("string memory")), "");
	fetchFreeMemoryPointer();
	m_context << (u256(FixedHash<4>::Arith(FixedHash<4>(dev::keccak256("Error(string)")))) << (256 - 32));
	m_context << Instruction::DUP2 << Instruction::MSTORE;
	m_context << u256(4) << Instruction::ADD;
	// Stack: <string data> <mem pos of encoding start>
	abiEncode({&_argumentType}, {TypeProvider::array(DataLocation::Memory, true)});
	toSizeAfterFreeMemoryPointer();
	m_context << Instruction::REVERT;
}

void CompilerUtils::accessCalldataTail(Type const& _type)
{
	solAssert(_type.dataStoredIn(DataLocation::CallData), "");

	unsigned int baseEncodedSize = _type.calldataEncodedSize();
	solAssert(baseEncodedSize > 1, "");

	// returns the absolute offset of the tail in "base_ref"
	m_context.appendInlineAssembly(Whiskers(R"({
		let rel_offset_of_tail := calldataload(ptr_to_tail)
		if iszero(slt(rel_offset_of_tail, sub(sub(calldatasize(), base_ref), sub(<neededLength>, 1)))) { revert(0, 0) }
		base_ref := add(base_ref, rel_offset_of_tail)
	})")("neededLength", toCompactHexWithPrefix(baseEncodedSize)).render(), {"base_ref", "ptr_to_tail"});
	// stack layout: <absolute_offset_of_tail> <garbage>

	if (!_type.isDynamicallySized())
	{
		m_context << Instruction::POP;
		// stack layout: <absolute_offset_of_tail>
		solAssert(
			_type.category() == Type::Category::Struct ||
			_type.category() == Type::Category::Array,
			"Invalid dynamically encoded base type on tail access."
		);
	}
	else
	{
		auto const* arrayType = dynamic_cast<ArrayType const*>(&_type);
		solAssert(!!arrayType, "Invalid dynamically sized type.");
		unsigned int calldataStride = arrayType->calldataStride();
		solAssert(calldataStride > 0, "");

		// returns the absolute offset of the tail in "base_ref"
		// and the length of the tail in "length"
		m_context.appendInlineAssembly(
			Whiskers(R"({
				length := calldataload(base_ref)
				base_ref := add(base_ref, 0x20)
				if gt(length, 0xffffffffffffffff) { revert(0, 0) }
				if sgt(base_ref, sub(calldatasize(), mul(length, <calldataStride>))) { revert(0, 0) }
			})")("calldataStride", toCompactHexWithPrefix(calldataStride)).render(),
			{"base_ref", "length"}
		);
		// stack layout: <absolute_offset_of_tail> <length>
	}
}

unsigned CompilerUtils::loadFromMemory(
	unsigned _offset,
	Type const& _type,
	bool _fromCalldata,
	bool _padToWordBoundaries
)
{
	solAssert(_type.category() != Type::Category::Array, "Unable to statically load dynamic type.");
	m_context << u256(_offset);
	return loadFromMemoryHelper(_type, _fromCalldata, _padToWordBoundaries);
}

void CompilerUtils::loadFromMemoryDynamic(
	Type const& _type,
	bool _fromCalldata,
	bool _padToWordBoundaries,
	bool _keepUpdatedMemoryOffset
)
{
	if (_keepUpdatedMemoryOffset)
		m_context << Instruction::DUP1;

	if (auto arrayType = dynamic_cast<ArrayType const*>(&_type))
	{
		solAssert(!arrayType->isDynamicallySized(), "");
		solAssert(!_fromCalldata, "");
		solAssert(_padToWordBoundaries, "");
		if (_keepUpdatedMemoryOffset)
			m_context << arrayType->memorySize() << Instruction::ADD;
	}
	else
	{
		unsigned numBytes = loadFromMemoryHelper(_type, _fromCalldata, _padToWordBoundaries);
		if (_keepUpdatedMemoryOffset)
		{
			// update memory counter
			moveToStackTop(_type.sizeOnStack());
			m_context << u256(numBytes) << Instruction::ADD;
		}
	}
}

void CompilerUtils::storeInMemory(unsigned _offset)
{
	unsigned numBytes = prepareMemoryStore(*TypeProvider::uint256(), true);
	if (numBytes > 0)
		m_context << u256(_offset) << Instruction::MSTORE;
}

void CompilerUtils::storeInMemoryDynamic(Type const& _type, bool _padToWordBoundaries)
{
	// process special types (Reference, StringLiteral, Function)
	if (auto ref = dynamic_cast<ReferenceType const*>(&_type))
	{
		solUnimplementedAssert(
			ref->location() == DataLocation::Memory,
			"Only in-memory reference type can be stored."
		);
		storeInMemoryDynamic(*TypeProvider::uint256(), _padToWordBoundaries);
	}
	else if (auto str = dynamic_cast<StringLiteralType const*>(&_type))
	{
		m_context << Instruction::DUP1;
		storeStringData(bytesConstRef(str->value()));
		if (_padToWordBoundaries)
			m_context << u256(max<size_t>(32, ((str->value().size() + 31) / 32) * 32));
		else
			m_context << u256(str->value().size());
		m_context << Instruction::ADD;
	}
	else if (
		_type.category() == Type::Category::Function &&
		dynamic_cast<FunctionType const&>(_type).kind() == FunctionType::Kind::External
	)
	{
		combineExternalFunctionType(true);
		m_context << Instruction::DUP2 << Instruction::MSTORE;
		m_context << u256(_padToWordBoundaries ? 32 : 24) << Instruction::ADD;
	}
	else if (_type.isValueType())
	{
		unsigned numBytes = prepareMemoryStore(_type, _padToWordBoundaries);
		m_context << Instruction::DUP2 << Instruction::MSTORE;
		m_context << u256(numBytes) << Instruction::ADD;
	}
	else // Should never happen
	{
		solAssert(
			false,
			"Memory store of type " + _type.toString(true) + " not allowed."
		);
	}
}

void CompilerUtils::abiDecode(TypePointers const& _typeParameters, bool _fromMemory)
{
	/// Stack: <source_offset> <length>
	if (m_context.experimentalFeatureActive(ExperimentalFeature::ABIEncoderV2))
	{
		// Use the new Yul-based decoding function
		auto stackHeightBefore = m_context.stackHeight();
		abiDecodeV2(_typeParameters, _fromMemory);
		solAssert(m_context.stackHeight() - stackHeightBefore == sizeOnStack(_typeParameters) - 2, "");
		return;
	}

	//@todo this does not yet support nested dynamic arrays
	size_t encodedSize = 0;
	for (auto const& t: _typeParameters)
		encodedSize += t->decodingType()->calldataEncodedSize(true);
	m_context.appendInlineAssembly("{ if lt(len, " + to_string(encodedSize) + ") { revert(0, 0) } }", {"len"});

	m_context << Instruction::DUP2 << Instruction::ADD;
	m_context << Instruction::SWAP1;
	/// Stack: <input_end> <source_offset>

	// Retain the offset pointer as base_offset, the point from which the data offsets are computed.
	m_context << Instruction::DUP1;
	for (TypePointer const& parameterType: _typeParameters)
	{
		// stack: v1 v2 ... v(k-1) input_end base_offset current_offset
		TypePointer type = parameterType->decodingType();
		solUnimplementedAssert(type, "No decoding type found.");
		if (type->category() == Type::Category::Array)
		{
			auto const& arrayType = dynamic_cast<ArrayType const&>(*type);
			solUnimplementedAssert(!arrayType.baseType()->isDynamicallyEncoded(), "Nested arrays not yet implemented.");
			if (_fromMemory)
			{
				solUnimplementedAssert(
					arrayType.baseType()->isValueType(),
					"Nested memory arrays not yet implemented here."
				);
				// @todo If base type is an array or struct, it is still calldata-style encoded, so
				// we would have to convert it like below.
				solAssert(arrayType.location() == DataLocation::Memory, "");
				if (arrayType.isDynamicallySized())
				{
					// compute data pointer
					m_context << Instruction::DUP1 << Instruction::MLOAD;
					// Check that the data pointer is valid and that length times
					// item size is still inside the range.
					Whiskers templ(R"({
						if gt(ptr, 0x100000000) { revert(0, 0) }
						ptr := add(ptr, base_offset)
						let array_data_start := add(ptr, 0x20)
						if gt(array_data_start, input_end) { revert(0, 0) }
						let array_length := mload(ptr)
						if or(
							gt(array_length, 0x100000000),
							gt(add(array_data_start, mul(array_length, <item_size>)), input_end)
						) { revert(0, 0) }
					})");
					templ("item_size", to_string(arrayType.calldataStride()));
					m_context.appendInlineAssembly(templ.render(), {"input_end", "base_offset", "offset", "ptr"});
					// stack: v1 v2 ... v(k-1) input_end base_offset current_offset v(k)
					moveIntoStack(3);
					m_context << u256(0x20) << Instruction::ADD;
				}
				else
				{
					// Size has already been checked for this one.
					moveIntoStack(2);
					m_context << Instruction::DUP3;
					m_context << u256(arrayType.calldataEncodedSize(true)) << Instruction::ADD;
				}
			}
			else
			{
				// first load from calldata and potentially convert to memory if arrayType is memory
				TypePointer calldataType = TypeProvider::withLocation(&arrayType, DataLocation::CallData, false);
				if (calldataType->isDynamicallySized())
				{
					// put on stack: data_pointer length
					loadFromMemoryDynamic(*TypeProvider::uint256(), !_fromMemory);
					m_context << Instruction::SWAP1;
					// stack: input_end base_offset next_pointer data_offset
					m_context.appendInlineAssembly("{ if gt(data_offset, 0x100000000) { revert(0, 0) } }", {"data_offset"});
					m_context << Instruction::DUP3 << Instruction::ADD;
					// stack: input_end base_offset next_pointer array_head_ptr
					m_context.appendInlineAssembly(
						"{ if gt(add(array_head_ptr, 0x20), input_end) { revert(0, 0) } }",
						{"input_end", "base_offset", "next_ptr", "array_head_ptr"}
					);
					// retrieve length
					loadFromMemoryDynamic(*TypeProvider::uint256(), !_fromMemory, true);
					// stack: input_end base_offset next_pointer array_length data_pointer
					m_context << Instruction::SWAP2;
					// stack: input_end base_offset data_pointer array_length next_pointer
					m_context.appendInlineAssembly(R"({
						if or(
							gt(array_length, 0x100000000),
							gt(add(data_ptr, mul(array_length, )" + to_string(arrayType.calldataStride()) + R"()), input_end)
						) { revert(0, 0) }
					})", {"input_end", "base_offset", "data_ptr", "array_length", "next_ptr"});
				}
				else
				{
					// size has already been checked
					// stack: input_end base_offset data_offset
					m_context << Instruction::DUP1;
					m_context << u256(calldataType->calldataEncodedSize()) << Instruction::ADD;
				}
				if (arrayType.location() == DataLocation::Memory)
				{
					// stack: input_end base_offset calldata_ref [length] next_calldata
					// copy to memory
					// move calldata type up again
					moveIntoStack(calldataType->sizeOnStack());
					convertType(*calldataType, arrayType, false, false, true);
					// fetch next pointer again
					moveToStackTop(arrayType.sizeOnStack());
				}
				// move input_end up
				// stack: input_end base_offset calldata_ref [length] next_calldata
				moveToStackTop(2 + arrayType.sizeOnStack());
				m_context << Instruction::SWAP1;
				// stack: base_offset calldata_ref [length] input_end next_calldata
				moveToStackTop(2 + arrayType.sizeOnStack());
				m_context << Instruction::SWAP1;
				// stack: calldata_ref [length] input_end base_offset next_calldata
			}
		}
		else
		{
			solAssert(!type->isDynamicallyEncoded(), "Unknown dynamically sized type: " + type->toString());
			loadFromMemoryDynamic(*type, !_fromMemory, true);
			// stack: v1 v2 ... v(k-1) input_end base_offset v(k) mem_offset
			moveToStackTop(1, type->sizeOnStack());
			moveIntoStack(3, type->sizeOnStack());
		}
		// stack: v1 v2 ... v(k-1) v(k) input_end base_offset next_offset
	}
	popStackSlots(3);
}

void CompilerUtils::encodeToMemory(
	TypePointers const& _givenTypes,
	TypePointers const& _targetTypes,
	bool _padToWordBoundaries,
	bool _copyDynamicDataInPlace,
	bool _encodeAsLibraryTypes
)
{
	// stack: <v1> <v2> ... <vn> <mem>
	bool const encoderV2 = m_context.experimentalFeatureActive(ExperimentalFeature::ABIEncoderV2);
	TypePointers targetTypes = _targetTypes.empty() ? _givenTypes : _targetTypes;
	solAssert(targetTypes.size() == _givenTypes.size(), "");
	for (TypePointer& t: targetTypes)
	{
		TypePointer tEncoding = t->fullEncodingType(_encodeAsLibraryTypes, encoderV2, !_padToWordBoundaries);
		solUnimplementedAssert(tEncoding, "Encoding type \"" + t->toString() + "\" not yet implemented.");
		t = std::move(tEncoding);
	}

	if (_givenTypes.empty())
		return;
	if (encoderV2)
	{
		// Use the new Yul-based encoding function
		solAssert(
			_padToWordBoundaries != _copyDynamicDataInPlace,
			"Non-padded and in-place encoding can only be combined."
		);
		auto stackHeightBefore = m_context.stackHeight();
		abiEncodeV2(_givenTypes, targetTypes, _encodeAsLibraryTypes, _padToWordBoundaries);
		solAssert(stackHeightBefore - m_context.stackHeight() == sizeOnStack(_givenTypes), "");
		return;
	}

	// Stack during operation:
	// <v1> <v2> ... <vn> <mem_start> <dyn_head_1> ... <dyn_head_r> <end_of_mem>
	// The values dyn_head_n are added during the first loop and they point to the head part
	// of the nth dynamic parameter, which is filled once the dynamic parts are processed.

	// store memory start pointer
	m_context << Instruction::DUP1;

	unsigned argSize = CompilerUtils::sizeOnStack(_givenTypes);
	unsigned stackPos = 0; // advances through the argument values
	unsigned dynPointers = 0; // number of dynamic head pointers on the stack
	for (size_t i = 0; i < _givenTypes.size(); ++i)
	{
		TypePointer targetType = targetTypes[i];
		solAssert(!!targetType, "Externalable type expected.");
		if (targetType->isDynamicallySized() && !_copyDynamicDataInPlace)
		{
			// leave end_of_mem as dyn head pointer
			m_context << Instruction::DUP1 << u256(32) << Instruction::ADD;
			dynPointers++;
			solAssert((argSize + dynPointers) < 16, "Stack too deep, try using fewer variables.");
		}
		else
		{
			copyToStackTop(argSize - stackPos + dynPointers + 2, _givenTypes[i]->sizeOnStack());
			solAssert(!!targetType, "Externalable type expected.");
			TypePointer type = targetType;
			if (_givenTypes[i]->dataStoredIn(DataLocation::Storage) && targetType->isValueType())
			{
				// special case: convert storage reference type to value type - this is only
				// possible for library calls where we just forward the storage reference
				solAssert(_encodeAsLibraryTypes, "");
				solAssert(_givenTypes[i]->sizeOnStack() == 1, "");
			}
			else if (
				_givenTypes[i]->dataStoredIn(DataLocation::Storage) ||
				_givenTypes[i]->dataStoredIn(DataLocation::CallData) ||
				_givenTypes[i]->category() == Type::Category::StringLiteral ||
				_givenTypes[i]->category() == Type::Category::Function
			)
				type = _givenTypes[i]; // delay conversion
			else
				convertType(*_givenTypes[i], *targetType, true);
			if (auto arrayType = dynamic_cast<ArrayType const*>(type))
				ArrayUtils(m_context).copyArrayToMemory(*arrayType, _padToWordBoundaries);
			else
				storeInMemoryDynamic(*type, _padToWordBoundaries);
		}
		stackPos += _givenTypes[i]->sizeOnStack();
	}

	// now copy the dynamic part
	// Stack: <v1> <v2> ... <vn> <mem_start> <dyn_head_1> ... <dyn_head_r> <end_of_mem>
	stackPos = 0;
	unsigned thisDynPointer = 0;
	for (size_t i = 0; i < _givenTypes.size(); ++i)
	{
		TypePointer targetType = targetTypes[i];
		solAssert(!!targetType, "Externalable type expected.");
		if (targetType->isDynamicallySized() && !_copyDynamicDataInPlace)
		{
			// copy tail pointer (=mem_end - mem_start) to memory
			m_context << dupInstruction(2 + dynPointers) << Instruction::DUP2;
			m_context << Instruction::SUB;
			m_context << dupInstruction(2 + dynPointers - thisDynPointer);
			m_context << Instruction::MSTORE;
			// stack: ... <end_of_mem>
			if (_givenTypes[i]->category() == Type::Category::StringLiteral)
			{
				auto const& strType = dynamic_cast<StringLiteralType const&>(*_givenTypes[i]);
				m_context << u256(strType.value().size());
				storeInMemoryDynamic(*TypeProvider::uint256(), true);
				// stack: ... <end_of_mem'>
				storeInMemoryDynamic(strType, _padToWordBoundaries);
			}
			else
			{
				solAssert(_givenTypes[i]->category() == Type::Category::Array, "Unknown dynamic type.");
				auto const& arrayType = dynamic_cast<ArrayType const&>(*_givenTypes[i]);
				// now copy the array
				copyToStackTop(argSize - stackPos + dynPointers + 2, arrayType.sizeOnStack());
				// stack: ... <end_of_mem> <value...>
				// copy length to memory
				m_context << dupInstruction(1 + arrayType.sizeOnStack());
				ArrayUtils(m_context).retrieveLength(arrayType, 1);
				// stack: ... <end_of_mem> <value...> <end_of_mem'> <length>
				storeInMemoryDynamic(*TypeProvider::uint256(), true);
				// stack: ... <end_of_mem> <value...> <end_of_mem''>
				// copy the new memory pointer
				m_context << swapInstruction(arrayType.sizeOnStack() + 1) << Instruction::POP;
				// stack: ... <end_of_mem''> <value...>
				// copy data part
				ArrayUtils(m_context).copyArrayToMemory(arrayType, _padToWordBoundaries);
				// stack: ... <end_of_mem'''>
			}

			thisDynPointer++;
		}
		stackPos += _givenTypes[i]->sizeOnStack();
	}

	// remove unneeded stack elements (and retain memory pointer)
	m_context << swapInstruction(argSize + dynPointers + 1);
	popStackSlots(argSize + dynPointers + 1);
}

void CompilerUtils::abiEncodeV2(
	TypePointers const& _givenTypes,
	TypePointers const& _targetTypes,
	bool _encodeAsLibraryTypes,
	bool _padToWordBoundaries
)
{
	if (!_padToWordBoundaries)
		solAssert(!_encodeAsLibraryTypes, "Library calls cannot be packed.");

	// stack: <$value0> <$value1> ... <$value(n-1)> <$headStart>

	auto ret = m_context.pushNewTag();
	moveIntoStack(sizeOnStack(_givenTypes) + 1);

	string encoderName =
		_padToWordBoundaries ?
		m_context.abiFunctions().tupleEncoder(_givenTypes, _targetTypes, _encodeAsLibraryTypes) :
		m_context.abiFunctions().tupleEncoderPacked(_givenTypes, _targetTypes);
	m_context.appendJumpTo(m_context.namedTag(encoderName));
	m_context.adjustStackOffset(-int(sizeOnStack(_givenTypes)) - 1);
	m_context << ret.tag();
}

void CompilerUtils::abiDecodeV2(TypePointers const& _parameterTypes, bool _fromMemory)
{
	// stack: <source_offset> <length> [stack top]
	auto ret = m_context.pushNewTag();
	moveIntoStack(2);
	// stack: <return tag> <source_offset> <length> [stack top]
	m_context << Instruction::DUP2 << Instruction::ADD;
	m_context << Instruction::SWAP1;
	// stack: <return tag> <end> <start>
	string decoderName = m_context.abiFunctions().tupleDecoder(_parameterTypes, _fromMemory);
	m_context.appendJumpTo(m_context.namedTag(decoderName));
	m_context.adjustStackOffset(int(sizeOnStack(_parameterTypes)) - 3);
	m_context << ret.tag();
}

void CompilerUtils::zeroInitialiseMemoryArray(ArrayType const& _type)
{
	if (_type.baseType()->hasSimpleZeroValueInMemory())
	{
		solAssert(_type.baseType()->isValueType(), "");
		Whiskers templ(R"({
			let size := mul(length, <element_size>)
			// cheap way of zero-initializing a memory range
			codecopy(memptr, codesize(), size)
			memptr := add(memptr, size)
		})");
		templ("element_size", to_string(_type.memoryStride()));
		m_context.appendInlineAssembly(templ.render(), {"length", "memptr"});
	}
	else
	{
		// TODO: Potential optimization:
		// When we create a new multi-dimensional dynamic array, each element
		// is initialized to an empty array. It actually does not hurt
		// to re-use exactly the same empty array for all elements. Currently,
		// a new one is created each time.
		auto repeat = m_context.newTag();
		m_context << repeat;
		pushZeroValue(*_type.baseType());
		storeInMemoryDynamic(*_type.baseType());
		m_context << Instruction::SWAP1 << u256(1) << Instruction::SWAP1;
		m_context << Instruction::SUB << Instruction::SWAP1;
		m_context << Instruction::DUP2;
		m_context.appendConditionalJumpTo(repeat);
	}
	m_context << Instruction::SWAP1 << Instruction::POP;
}

void CompilerUtils::memoryCopy32()
{
	// Stack here: size target source

	m_context.appendInlineAssembly(R"(
		{
			for { let i := 0 } lt(i, len) { i := add(i, 32) } {
				mstore(add(dst, i), mload(add(src, i)))
			}
		}
	)",
		{ "len", "dst", "src" }
	);
	m_context << Instruction::POP << Instruction::POP << Instruction::POP;
}

void CompilerUtils::memoryCopy()
{
	// Stack here: size target source

	m_context.appendInlineAssembly(R"(
		{
			// copy 32 bytes at once
			for
				{}
				iszero(lt(len, 32))
				{
					dst := add(dst, 32)
					src := add(src, 32)
					len := sub(len, 32)
				}
				{ mstore(dst, mload(src)) }

			// copy the remainder (0 < len < 32)
			let mask := sub(exp(256, sub(32, len)), 1)
			let srcpart := and(mload(src), not(mask))
			let dstpart := and(mload(dst), mask)
			mstore(dst, or(srcpart, dstpart))
		}
	)",
		{ "len", "dst", "src" }
	);
	m_context << Instruction::POP << Instruction::POP << Instruction::POP;
}

void CompilerUtils::splitExternalFunctionType(bool _leftAligned)
{
	// We have to split the left-aligned <address><function identifier> into two stack slots:
	// address (right aligned), function identifier (right aligned)
	if (_leftAligned)
	{
		m_context << Instruction::DUP1;
		rightShiftNumberOnStack(64 + 32);
		// <input> <address>
		m_context << Instruction::SWAP1;
		rightShiftNumberOnStack(64);
	}
	else
	{
		m_context << Instruction::DUP1;
		rightShiftNumberOnStack(32);
		m_context << ((u256(1) << 160) - 1) << Instruction::AND << Instruction::SWAP1;
	}
	m_context << u256(0xffffffffUL) << Instruction::AND;
}

void CompilerUtils::combineExternalFunctionType(bool _leftAligned)
{
	// <address> <function_id>
	m_context << u256(0xffffffffUL) << Instruction::AND << Instruction::SWAP1;
	if (!_leftAligned)
		m_context << ((u256(1) << 160) - 1) << Instruction::AND;
	leftShiftNumberOnStack(32);
	m_context << Instruction::OR;
	if (_leftAligned)
		leftShiftNumberOnStack(64);
}

void CompilerUtils::pushCombinedFunctionEntryLabel(Declaration const& _function, bool _runtimeOnly)
{
	m_context << m_context.functionEntryLabel(_function).pushTag();
	// If there is a runtime context, we have to merge both labels into the same
	// stack slot in case we store it in storage.
	if (CompilerContext* rtc = m_context.runtimeContext())
	{
		leftShiftNumberOnStack(32);
		if (_runtimeOnly)
			m_context <<
				rtc->functionEntryLabel(_function).toSubAssemblyTag(m_context.runtimeSub()) <<
				Instruction::OR;
	}
}

void CompilerUtils::convertType(
	Type const& _typeOnStack,
	Type const& _targetType,
	bool _cleanupNeeded,
	bool _chopSignBits,
	bool _asPartOfArgumentDecoding
)
{
	// For a type extension, we need to remove all higher-order bits that we might have ignored in
	// previous operations.
	// @todo: store in the AST whether the operand might have "dirty" higher order bits

	if (_typeOnStack == _targetType && !_cleanupNeeded)
		return;
	Type::Category stackTypeCategory = _typeOnStack.category();
	Type::Category targetTypeCategory = _targetType.category();

	bool enumOverflowCheckPending = (targetTypeCategory == Type::Category::Enum || stackTypeCategory == Type::Category::Enum);
	bool chopSignBitsPending = _chopSignBits && targetTypeCategory == Type::Category::Integer;
	if (chopSignBitsPending)
	{
		IntegerType const& targetIntegerType = dynamic_cast<IntegerType const&>(_targetType);
		chopSignBitsPending = targetIntegerType.isSigned();
	}

	if (targetTypeCategory == Type::Category::FixedPoint)
		solUnimplemented("Not yet implemented - FixedPointType.");

	switch (stackTypeCategory)
	{
	case Type::Category::FixedBytes:
	{
		FixedBytesType const& typeOnStack = dynamic_cast<FixedBytesType const&>(_typeOnStack);
		if (targetTypeCategory == Type::Category::Integer)
		{
			// conversion from bytes to integer. no need to clean the high bit
			// only to shift right because of opposite alignment
			IntegerType const& targetIntegerType = dynamic_cast<IntegerType const&>(_targetType);
			rightShiftNumberOnStack(256 - typeOnStack.numBytes() * 8);
			if (targetIntegerType.numBits() < typeOnStack.numBytes() * 8)
				convertType(IntegerType(typeOnStack.numBytes() * 8), _targetType, _cleanupNeeded);
		}
		else if (targetTypeCategory == Type::Category::Address)
		{
			solAssert(typeOnStack.numBytes() * 8 == 160, "");
			rightShiftNumberOnStack(256 - 160);
		}
		else
		{
			// clear for conversion to longer bytes
			solAssert(targetTypeCategory == Type::Category::FixedBytes, "Invalid type conversion requested.");
			FixedBytesType const& targetType = dynamic_cast<FixedBytesType const&>(_targetType);
			if (typeOnStack.numBytes() == 0 || targetType.numBytes() == 0)
				m_context << Instruction::POP << u256(0);
			else if (targetType.numBytes() > typeOnStack.numBytes() || _cleanupNeeded)
			{
				unsigned bytes = min(typeOnStack.numBytes(), targetType.numBytes());
				m_context << ((u256(1) << (256 - bytes * 8)) - 1);
				m_context << Instruction::NOT << Instruction::AND;
			}
		}
		break;
	}
	case Type::Category::Enum:
		solAssert(_targetType == _typeOnStack || targetTypeCategory == Type::Category::Integer, "");
		if (enumOverflowCheckPending)
		{
			EnumType const& enumType = dynamic_cast<decltype(enumType)>(_typeOnStack);
			solAssert(enumType.numberOfMembers() > 0, "empty enum should have caused a parser error.");
			m_context << u256(enumType.numberOfMembers() - 1) << Instruction::DUP2 << Instruction::GT;
			if (_asPartOfArgumentDecoding)
				// TODO: error message?
				m_context.appendConditionalRevert();
			else
				m_context.appendConditionalInvalid();
			enumOverflowCheckPending = false;
		}
		break;
	case Type::Category::FixedPoint:
		solUnimplemented("Not yet implemented - FixedPointType.");
	case Type::Category::Address:
	case Type::Category::Integer:
	case Type::Category::Contract:
	case Type::Category::RationalNumber:
		if (targetTypeCategory == Type::Category::FixedBytes)
		{
			solAssert(
				stackTypeCategory == Type::Category::Address ||
				stackTypeCategory == Type::Category::Integer ||
				stackTypeCategory == Type::Category::RationalNumber,
				"Invalid conversion to FixedBytesType requested."
			);
			// conversion from bytes to string. no need to clean the high bit
			// only to shift left because of opposite alignment
			FixedBytesType const& targetBytesType = dynamic_cast<FixedBytesType const&>(_targetType);
			if (auto typeOnStack = dynamic_cast<IntegerType const*>(&_typeOnStack))
			{
				if (targetBytesType.numBytes() * 8 > typeOnStack->numBits())
					cleanHigherOrderBits(*typeOnStack);
			}
			else if (stackTypeCategory == Type::Category::Address)
				solAssert(targetBytesType.numBytes() * 8 == 160, "");
			leftShiftNumberOnStack(256 - targetBytesType.numBytes() * 8);
		}
		else if (targetTypeCategory == Type::Category::Enum)
		{
			solAssert(stackTypeCategory != Type::Category::Address, "Invalid conversion to EnumType requested.");
			solAssert(_typeOnStack.mobileType(), "");
			// just clean
			convertType(_typeOnStack, *_typeOnStack.mobileType(), true);
			EnumType const& enumType = dynamic_cast<decltype(enumType)>(_targetType);
			solAssert(enumType.numberOfMembers() > 0, "empty enum should have caused a parser error.");
			m_context << u256(enumType.numberOfMembers() - 1) << Instruction::DUP2 << Instruction::GT;
			m_context.appendConditionalInvalid();
			enumOverflowCheckPending = false;
		}
		else if (targetTypeCategory == Type::Category::FixedPoint)
		{
			solAssert(
				stackTypeCategory == Type::Category::Integer ||
				stackTypeCategory == Type::Category::RationalNumber ||
				stackTypeCategory == Type::Category::FixedPoint,
				"Invalid conversion to FixedMxNType requested."
			);
			//shift all integer bits onto the left side of the fixed type
			FixedPointType const& targetFixedPointType = dynamic_cast<FixedPointType const&>(_targetType);
			if (auto typeOnStack = dynamic_cast<IntegerType const*>(&_typeOnStack))
				if (targetFixedPointType.numBits() > typeOnStack->numBits())
					cleanHigherOrderBits(*typeOnStack);
			solUnimplemented("Not yet implemented - FixedPointType.");
		}
		else
		{
			solAssert(
				targetTypeCategory == Type::Category::Integer ||
				targetTypeCategory == Type::Category::Contract ||
				targetTypeCategory == Type::Category::Address,
				""
			);
			IntegerType addressType(160);
			IntegerType const& targetType = targetTypeCategory == Type::Category::Integer
				? dynamic_cast<IntegerType const&>(_targetType) : addressType;
			if (stackTypeCategory == Type::Category::RationalNumber)
			{
				RationalNumberType const& constType = dynamic_cast<RationalNumberType const&>(_typeOnStack);
				// We know that the stack is clean, we only have to clean for a narrowing conversion
				// where cleanup is forced.
				solUnimplementedAssert(!constType.isFractional(), "Not yet implemented - FixedPointType.");
				if (targetType.numBits() < constType.integerType()->numBits() && _cleanupNeeded)
					cleanHigherOrderBits(targetType);
			}
			else
			{
				IntegerType const& typeOnStack = stackTypeCategory == Type::Category::Integer
					? dynamic_cast<IntegerType const&>(_typeOnStack) : addressType;
				// Widening: clean up according to source type width
				// Non-widening and force: clean up according to target type bits
				if (targetType.numBits() > typeOnStack.numBits())
					cleanHigherOrderBits(typeOnStack);
				else if (_cleanupNeeded)
					cleanHigherOrderBits(targetType);
				if (chopSignBitsPending)
				{
					if (targetType.numBits() < 256)
						m_context
							<< ((u256(1) << targetType.numBits()) - 1)
							<< Instruction::AND;
					chopSignBitsPending = false;
				}
			}
		}
		break;
	case Type::Category::StringLiteral:
	{
		auto const& literalType = dynamic_cast<StringLiteralType const&>(_typeOnStack);
		string const& value = literalType.value();
		bytesConstRef data(value);
		if (targetTypeCategory == Type::Category::FixedBytes)
		{
			unsigned const numBytes = dynamic_cast<FixedBytesType const&>(_targetType).numBytes();
			solAssert(data.size() <= 32, "");
			m_context << (h256::Arith(h256(data, h256::AlignLeft)) & (~(u256(-1) >> (8 * numBytes))));
		}
		else if (targetTypeCategory == Type::Category::Array)
		{
			auto const& arrayType = dynamic_cast<ArrayType const&>(_targetType);
			solAssert(arrayType.isByteArray(), "");
			unsigned storageSize = 32 + ((data.size() + 31) / 32) * 32;
			allocateMemory(storageSize);
			// stack: mempos
			m_context << Instruction::DUP1 << u256(data.size());
			storeInMemoryDynamic(*TypeProvider::uint256());
			// stack: mempos datapos
			storeStringData(data);
		}
		else
			solAssert(
				false,
				"Invalid conversion from string literal to " + _targetType.toString(false) + " requested."
			);
		break;
	}
	case Type::Category::Array:
	{
		solAssert(targetTypeCategory == stackTypeCategory, "");
		ArrayType const& typeOnStack = dynamic_cast<ArrayType const&>(_typeOnStack);
		ArrayType const& targetType = dynamic_cast<ArrayType const&>(_targetType);
		switch (targetType.location())
		{
		case DataLocation::Storage:
			// Other cases are done explicitly in LValue::storeValue, and only possible by assignment.
			solAssert(
				(targetType.isPointer() || (typeOnStack.isByteArray() && targetType.isByteArray())) &&
				typeOnStack.location() == DataLocation::Storage,
				"Invalid conversion to storage type."
			);
			break;
		case DataLocation::Memory:
		{
			// Copy the array to a free position in memory, unless it is already in memory.
			if (typeOnStack.location() != DataLocation::Memory)
			{
				// stack: <source ref> (variably sized)
				unsigned stackSize = typeOnStack.sizeOnStack();
				ArrayUtils(m_context).retrieveLength(typeOnStack);

				// allocate memory
				// stack: <source ref> (variably sized) <length>
				m_context << Instruction::DUP1;
				ArrayUtils(m_context).convertLengthToSize(targetType, true);
				// stack: <source ref> (variably sized) <length> <size>
				if (targetType.isDynamicallySized())
					m_context << u256(0x20) << Instruction::ADD;
				allocateMemory();
				// stack: <source ref> (variably sized) <length> <mem start>
				m_context << Instruction::DUP1;
				moveIntoStack(2 + stackSize);
				if (targetType.isDynamicallySized())
				{
					m_context << Instruction::DUP2;
					storeInMemoryDynamic(*TypeProvider::uint256());
				}
				// stack: <mem start> <source ref> (variably sized) <length> <mem data pos>
				if (targetType.baseType()->isValueType())
				{
					copyToStackTop(2 + stackSize, stackSize);
					ArrayUtils(m_context).copyArrayToMemory(typeOnStack);
				}
				else
				{
					m_context << u256(0) << Instruction::SWAP1;
					// stack: <mem start> <source ref> (variably sized) <length> <counter> <mem data pos>
					auto repeat = m_context.newTag();
					m_context << repeat;
					m_context << Instruction::DUP3 << Instruction::DUP3;
					m_context << Instruction::LT << Instruction::ISZERO;
					auto loopEnd = m_context.appendConditionalJump();
					copyToStackTop(3 + stackSize, stackSize);
					copyToStackTop(2 + stackSize, 1);
					ArrayUtils(m_context).accessIndex(typeOnStack, false);
					if (typeOnStack.location() == DataLocation::Storage)
						StorageItem(m_context, *typeOnStack.baseType()).retrieveValue(SourceLocation(), true);
					convertType(*typeOnStack.baseType(), *targetType.baseType(), _cleanupNeeded);
					storeInMemoryDynamic(*targetType.baseType(), true);
					m_context << Instruction::SWAP1 << u256(1) << Instruction::ADD;
					m_context << Instruction::SWAP1;
					m_context.appendJumpTo(repeat);
					m_context << loopEnd;
					m_context << Instruction::POP;
				}
				// stack: <mem start> <source ref> (variably sized) <length> <mem data pos updated>
				popStackSlots(2 + stackSize);
				// Stack: <mem start>
			}
			break;
		}
		case DataLocation::CallData:
			solAssert(
				targetType.isByteArray() &&
				typeOnStack.isByteArray() &&
				typeOnStack.location() == DataLocation::CallData,
				"Invalid conversion to calldata type."
			);
			break;
		}
		break;
	}
	case Type::Category::Struct:
	{
		solAssert(targetTypeCategory == stackTypeCategory, "");
		auto& targetType = dynamic_cast<StructType const&>(_targetType);
		auto& typeOnStack = dynamic_cast<StructType const&>(_typeOnStack);
		solAssert(
			targetType.location() != DataLocation::CallData
		, "");
		switch (targetType.location())
		{
		case DataLocation::Storage:
			// Other cases are done explicitly in LValue::storeValue, and only possible by assignment.
			solAssert(
				targetType.isPointer() &&
				typeOnStack.location() == DataLocation::Storage,
				"Invalid conversion to storage type."
			);
			break;
		case DataLocation::Memory:
			// Copy the array to a free position in memory, unless it is already in memory.
			switch (typeOnStack.location())
			{
			case DataLocation::Storage:
			{
				auto conversionImpl =
					[typeOnStack = &typeOnStack, targetType = &targetType](CompilerContext& _context)
				{
					CompilerUtils utils(_context);
					// stack: <source ref>
					utils.allocateMemory(typeOnStack->memorySize());
					_context << Instruction::SWAP1 << Instruction::DUP2;
					// stack: <memory ptr> <source ref> <memory ptr>
					for (auto const& member: typeOnStack->members(nullptr))
					{
						if (!member.type->canLiveOutsideStorage())
							continue;
						pair<u256, unsigned> const& offsets = typeOnStack->storageOffsetsOfMember(member.name);
						_context << offsets.first << Instruction::DUP3 << Instruction::ADD;
						_context << u256(offsets.second);
						StorageItem(_context, *member.type).retrieveValue(SourceLocation(), true);
						TypePointer targetMemberType = targetType->memberType(member.name);
						solAssert(!!targetMemberType, "Member not found in target type.");
						utils.convertType(*member.type, *targetMemberType, true);
						utils.storeInMemoryDynamic(*targetMemberType, true);
					}
					_context << Instruction::POP << Instruction::POP;
				};
				if (typeOnStack.recursive())
					m_context.callLowLevelFunction(
						"$convertRecursiveArrayStorageToMemory_" + typeOnStack.identifier() + "_to_" + targetType.identifier(),
						1,
						1,
						conversionImpl
					);
				else
					conversionImpl(m_context);
				break;
			}
			case DataLocation::CallData:
			{
				solUnimplementedAssert(!typeOnStack.isDynamicallyEncoded(), "");
				m_context << Instruction::DUP1;
				m_context << Instruction::CALLDATASIZE;
				m_context << Instruction::SUB;
				abiDecode({&targetType}, false);
				break;
			}
			case DataLocation::Memory:
				// nothing to do
				break;
			}
			break;
		case DataLocation::CallData:
			solAssert(false, "Invalid type conversion target location CallData.");
			break;
		}
		break;
	}
	case Type::Category::Tuple:
	{
		TupleType const& sourceTuple = dynamic_cast<TupleType const&>(_typeOnStack);
		TupleType const& targetTuple = dynamic_cast<TupleType const&>(_targetType);
		solAssert(targetTuple.components().size() == sourceTuple.components().size(), "");
		unsigned depth = sourceTuple.sizeOnStack();
		for (size_t i = 0; i < sourceTuple.components().size(); ++i)
		{
			TypePointer sourceType = sourceTuple.components()[i];
			TypePointer targetType = targetTuple.components()[i];
			if (!sourceType)
			{
				solAssert(!targetType, "");
				continue;
			}
			unsigned sourceSize = sourceType->sizeOnStack();
			unsigned targetSize = targetType ? targetType->sizeOnStack() : 0;
			if (!targetType || *sourceType != *targetType || _cleanupNeeded)
			{
				if (targetType)
				{
					if (sourceSize > 0)
						copyToStackTop(depth, sourceSize);
					convertType(*sourceType, *targetType, _cleanupNeeded);
				}
				if (sourceSize > 0 || targetSize > 0)
				{
					// Move it back into its place.
					for (unsigned j = 0; j < min(sourceSize, targetSize); ++j)
						m_context <<
							swapInstruction(depth + targetSize - sourceSize) <<
							Instruction::POP;
					// Value shrank
					for (unsigned j = targetSize; j < sourceSize; ++j)
					{
						moveToStackTop(depth - 1, 1);
						m_context << Instruction::POP;
					}
					// Value grew
					if (targetSize > sourceSize)
						moveIntoStack(depth + targetSize - sourceSize - 1, targetSize - sourceSize);
				}
			}
			depth -= sourceSize;
		}
		break;
	}
	case Type::Category::Bool:
		solAssert(_targetType == _typeOnStack, "Invalid conversion for bool.");
		if (_cleanupNeeded)
			m_context << Instruction::ISZERO << Instruction::ISZERO;
		break;
	default:
		if (stackTypeCategory == Type::Category::Function && targetTypeCategory == Type::Category::Address)
		{
			FunctionType const& typeOnStack = dynamic_cast<FunctionType const&>(_typeOnStack);
			solAssert(typeOnStack.kind() == FunctionType::Kind::External, "Only external function type can be converted.");

			// stack: <address> <function_id>
			m_context << Instruction::POP;
		}
		else
		{
			if (stackTypeCategory == Type::Category::Function && targetTypeCategory == Type::Category::Function)
			{
				FunctionType const& typeOnStack = dynamic_cast<FunctionType const&>(_typeOnStack);
				FunctionType const& targetType = dynamic_cast<FunctionType const&>(_targetType);
				solAssert(
					typeOnStack.isImplicitlyConvertibleTo(targetType) &&
					typeOnStack.sizeOnStack() == targetType.sizeOnStack() &&
					(typeOnStack.kind() == FunctionType::Kind::Internal || typeOnStack.kind() == FunctionType::Kind::External) &&
					typeOnStack.kind() == targetType.kind(),
					"Invalid function type conversion requested."
				);
			}
			else
				// All other types should not be convertible to non-equal types.
				solAssert(_typeOnStack == _targetType, "Invalid type conversion requested.");

			if (_cleanupNeeded && _targetType.canBeStored() && _targetType.storageBytes() < 32)
				m_context
					<< ((u256(1) << (8 * _targetType.storageBytes())) - 1)
					<< Instruction::AND;
		}
		break;
	}

	solAssert(!enumOverflowCheckPending, "enum overflow checking missing.");
	solAssert(!chopSignBitsPending, "forgot to chop the sign bits.");
}

void CompilerUtils::pushZeroValue(Type const& _type)
{
	if (auto const* funType = dynamic_cast<FunctionType const*>(&_type))
	{
		if (funType->kind() == FunctionType::Kind::Internal)
		{
			m_context << m_context.lowLevelFunctionTag("$invalidFunction", 0, 0, [](CompilerContext& _context) {
				_context.appendInvalid();
			});
			if (CompilerContext* runCon = m_context.runtimeContext())
			{
				leftShiftNumberOnStack(32);
				m_context << runCon->lowLevelFunctionTag("$invalidFunction", 0, 0, [](CompilerContext& _context) {
					_context.appendInvalid();
				}).toSubAssemblyTag(m_context.runtimeSub());
				m_context << Instruction::OR;
			}
			return;
		}
	}
	auto const* referenceType = dynamic_cast<ReferenceType const*>(&_type);
	if (!referenceType || referenceType->location() == DataLocation::Storage)
	{
		for (size_t i = 0; i < _type.sizeOnStack(); ++i)
			m_context << u256(0);
		return;
	}
	solAssert(referenceType->location() == DataLocation::Memory, "");
	if (auto arrayType = dynamic_cast<ArrayType const*>(&_type))
		if (arrayType->isDynamicallySized())
		{
			// Push a memory location that is (hopefully) always zero.
			pushZeroPointer();
			return;
		}

	TypePointer type = &_type;
	m_context.callLowLevelFunction(
		"$pushZeroValue_" + referenceType->identifier(),
		0,
		1,
		[type](CompilerContext& _context) {
			CompilerUtils utils(_context);
			utils.allocateMemory(max(32u, type->calldataEncodedSize()));
			_context << Instruction::DUP1;

			if (auto structType = dynamic_cast<StructType const*>(type))
				for (auto const& member: structType->members(nullptr))
				{
					utils.pushZeroValue(*member.type);
					utils.storeInMemoryDynamic(*member.type);
				}
			else if (auto arrayType = dynamic_cast<ArrayType const*>(type))
			{
				solAssert(!arrayType->isDynamicallySized(), "");
				if (arrayType->length() > 0)
				{
					_context << arrayType->length() << Instruction::SWAP1;
					// stack: items_to_do memory_pos
					utils.zeroInitialiseMemoryArray(*arrayType);
					// stack: updated_memory_pos
				}
			}
			else
				solAssert(false, "Requested initialisation for unknown type: " + type->toString());

			// remove the updated memory pointer
			_context << Instruction::POP;
		}
	);
}

void CompilerUtils::pushZeroPointer()
{
	m_context << u256(zeroPointer);
}

void CompilerUtils::moveToStackVariable(VariableDeclaration const& _variable)
{
	unsigned const stackPosition = m_context.baseToCurrentStackOffset(m_context.baseStackOffsetOfVariable(_variable));
	unsigned const size = _variable.annotation().type->sizeOnStack();
	solAssert(stackPosition >= size, "Variable size and position mismatch.");
	// move variable starting from its top end in the stack
	if (stackPosition - size + 1 > 16)
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_variable.location()) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	for (unsigned i = 0; i < size; ++i)
		m_context << swapInstruction(stackPosition - size + 1) << Instruction::POP;
}

void CompilerUtils::copyToStackTop(unsigned _stackDepth, unsigned _itemSize)
{
	solAssert(_stackDepth <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = 0; i < _itemSize; ++i)
		m_context << dupInstruction(_stackDepth);
}

void CompilerUtils::moveToStackTop(unsigned _stackDepth, unsigned _itemSize)
{
	moveIntoStack(_itemSize, _stackDepth);
}

void CompilerUtils::moveIntoStack(unsigned _stackDepth, unsigned _itemSize)
{
	if (_stackDepth <= _itemSize)
		for (unsigned i = 0; i < _stackDepth; ++i)
			rotateStackDown(_stackDepth + _itemSize);
	else
		for (unsigned i = 0; i < _itemSize; ++i)
			rotateStackUp(_stackDepth + _itemSize);
}

void CompilerUtils::rotateStackUp(unsigned _items)
{
	solAssert(_items - 1 <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = 1; i < _items; ++i)
		m_context << swapInstruction(_items - i);
}

void CompilerUtils::rotateStackDown(unsigned _items)
{
	solAssert(_items - 1 <= 16, "Stack too deep, try removing local variables.");
	for (unsigned i = 1; i < _items; ++i)
		m_context << swapInstruction(i);
}

void CompilerUtils::popStackElement(Type const& _type)
{
	popStackSlots(_type.sizeOnStack());
}

void CompilerUtils::popStackSlots(size_t _amount)
{
	for (size_t i = 0; i < _amount; ++i)
		m_context << Instruction::POP;
}

void CompilerUtils::popAndJump(unsigned _toHeight, eth::AssemblyItem const& _jumpTo)
{
	solAssert(m_context.stackHeight() >= _toHeight, "");
	unsigned amount = m_context.stackHeight() - _toHeight;
	popStackSlots(amount);
	m_context.appendJumpTo(_jumpTo);
	m_context.adjustStackOffset(amount);
}

unsigned CompilerUtils::sizeOnStack(vector<Type const*> const& _variableTypes)
{
	unsigned size = 0;
	for (Type const* const& type: _variableTypes)
		size += type->sizeOnStack();
	return size;
}

void CompilerUtils::computeHashStatic()
{
	storeInMemory(0);
	m_context << u256(32) << u256(0) << Instruction::KECCAK256;
}

void CompilerUtils::copyContractCodeToMemory(ContractDefinition const& contract, bool _creation)
{
	string which = _creation ? "Creation" : "Runtime";
	m_context.callLowLevelFunction(
		"$copyContract" + which + "CodeToMemory_" + contract.type()->identifier(),
		1,
		1,
		[&contract, _creation](CompilerContext& _context)
		{
			// copy the contract's code into memory
			shared_ptr<eth::Assembly> assembly =
				_creation ?
				_context.compiledContract(contract) :
				_context.compiledContractRuntime(contract);
			// pushes size
			auto subroutine = _context.addSubroutine(assembly);
			_context << Instruction::DUP1 << subroutine;
			_context << Instruction::DUP4 << Instruction::CODECOPY;
			_context << Instruction::ADD;
		}
	);
}

void CompilerUtils::storeStringData(bytesConstRef _data)
{
	//@todo provide both alternatives to the optimiser
	// stack: mempos
	if (_data.size() <= 32)
	{
		for (unsigned i = 0; i < _data.size(); i += 32)
		{
			m_context << h256::Arith(h256(_data.cropped(i), h256::AlignLeft));
			storeInMemoryDynamic(*TypeProvider::uint256());
		}
		m_context << Instruction::POP;
	}
	else
	{
		// stack: mempos mempos_data
		m_context.appendData(_data.toBytes());
		m_context << u256(_data.size()) << Instruction::SWAP2;
		m_context << Instruction::CODECOPY;
	}
}

unsigned CompilerUtils::loadFromMemoryHelper(Type const& _type, bool _fromCalldata, bool _padToWords)
{
	unsigned numBytes = _type.calldataEncodedSize(_padToWords);
	bool isExternalFunctionType = false;
	if (auto const* funType = dynamic_cast<FunctionType const*>(&_type))
		if (funType->kind() == FunctionType::Kind::External)
			isExternalFunctionType = true;
	if (numBytes == 0)
	{
		m_context << Instruction::POP << u256(0);
		return numBytes;
	}
	solAssert(numBytes <= 32, "Static memory load of more than 32 bytes requested.");
	m_context << (_fromCalldata ? Instruction::CALLDATALOAD : Instruction::MLOAD);
	bool cleanupNeeded = true;
	if (isExternalFunctionType)
		splitExternalFunctionType(true);
	else if (numBytes != 32)
	{
		bool leftAligned = _type.category() == Type::Category::FixedBytes;
		// add leading or trailing zeros by dividing/multiplying depending on alignment
		int shiftFactor = (32 - numBytes) * 8;
		rightShiftNumberOnStack(shiftFactor);
		if (leftAligned)
		{
			leftShiftNumberOnStack(shiftFactor);
			cleanupNeeded = false;
		}
		else if (IntegerType const* intType = dynamic_cast<IntegerType const*>(&_type))
			if (!intType->isSigned())
				cleanupNeeded = false;
	}
	if (_fromCalldata)
		convertType(_type, _type, cleanupNeeded, false, true);

	return numBytes;
}

void CompilerUtils::cleanHigherOrderBits(IntegerType const& _typeOnStack)
{
	if (_typeOnStack.numBits() == 256)
		return;
	else if (_typeOnStack.isSigned())
		m_context << u256(_typeOnStack.numBits() / 8 - 1) << Instruction::SIGNEXTEND;
	else
		m_context << ((u256(1) << _typeOnStack.numBits()) - 1) << Instruction::AND;
}

void CompilerUtils::leftShiftNumberOnStack(unsigned _bits)
{
	solAssert(_bits < 256, "");
	if (m_context.evmVersion().hasBitwiseShifting())
		m_context << _bits << Instruction::SHL;
	else
		m_context << (u256(1) << _bits) << Instruction::MUL;
}

void CompilerUtils::rightShiftNumberOnStack(unsigned _bits)
{
	solAssert(_bits < 256, "");
	// NOTE: If we add signed right shift, SAR rounds differently than SDIV
	if (m_context.evmVersion().hasBitwiseShifting())
		m_context << _bits << Instruction::SHR;
	else
		m_context << (u256(1) << _bits) << Instruction::SWAP1 << Instruction::DIV;
}

unsigned CompilerUtils::prepareMemoryStore(Type const& _type, bool _padToWords)
{
	solAssert(
		_type.sizeOnStack() == 1,
		"Memory store of types with stack size != 1 not allowed (Type: " + _type.toString(true) + ")."
	);

	unsigned numBytes = _type.calldataEncodedSize(_padToWords);

	solAssert(
		numBytes > 0,
		"Memory store of 0 bytes requested (Type: " + _type.toString(true) + ")."
	);

	solAssert(
		numBytes <= 32,
		"Memory store of more than 32 bytes requested (Type: " + _type.toString(true) + ")."
	);

	bool leftAligned = _type.category() == Type::Category::FixedBytes;

	convertType(_type, _type, true);
	if (numBytes != 32 && !leftAligned && !_padToWords)
		// shift the value accordingly before storing
		leftShiftNumberOnStack((32 - numBytes) * 8);

	return numBytes;
}
