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

#include <libsolidity/interface/Ethdebug.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

#include <libsolutil/Numeric.h>

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace solidity;
using namespace solidity::frontend;

namespace schema = solidity::evmasm::ethdebug::schema;

namespace
{

/// Type documents under construction, together with the compilation's source
/// indices: definition locations refer to the same source IDs the ethdebug
/// compilation record uses.
class TypeRegistry
{
public:
	explicit TypeRegistry(std::map<std::string, unsigned> const& _sourceIndices): m_sourceIndices(_sourceIndices) {}

	/// Registers the document of @a _type and of every type it composes, keyed
	/// by the compiler's type identifier, and @returns whether @a _type has a
	/// document. Compile-time-only types, such as literals, have none, and
	/// neither has a type composing one.
	bool registerType(Type const& _type);

	std::map<std::string, schema::Type> takeDocuments() { return std::move(m_documents); }

	std::optional<schema::materials::SourceRange> sourceRange(langutil::SourceLocation const& _location) const;

private:
	std::optional<schema::Type> document(Type const& _type);
	std::optional<schema::Type::Wrapper> wrapper(std::optional<std::string> _name, Type const& _type);
	std::optional<std::vector<schema::Type::Wrapper>> wrappers(std::vector<Type const*> const& _types);
	std::optional<schema::Type::Definition> definition(Declaration const& _declaration) const;

	std::map<std::string, unsigned> const& m_sourceIndices;
	std::map<std::string, schema::Type> m_documents;
};

std::optional<schema::materials::SourceRange> TypeRegistry::sourceRange(langutil::SourceLocation const& _location) const
{
	if (!_location.sourceName || !_location.hasText())
		return std::nullopt;
	auto const it = m_sourceIndices.find(*_location.sourceName);
	if (it == m_sourceIndices.end())
		return std::nullopt;

	schema::materials::SourceRange range;
	range.source.id = schema::materials::ID{static_cast<uint64_t>(it->second)};
	range.range = schema::materials::SourceRange::Range{
		.length = schema::data::Unsigned{_location.end - _location.start},
		.offset = schema::data::Unsigned{_location.start}
	};
	return range;
}

std::optional<schema::Type::Definition> TypeRegistry::definition(Declaration const& _declaration) const
{
	std::optional<std::string> name;
	if (!_declaration.name().empty())
		name = _declaration.name();
	std::optional<schema::materials::SourceRange> location = sourceRange(_declaration.location());
	if (!name && !location)
		return std::nullopt;
	return schema::Type::Definition{std::move(name), std::move(location)};
}

bool TypeRegistry::registerType(Type const& _type)
{
	std::string const id = _type.identifier();
	if (m_documents.count(id))
		return true;
	// Present before descending, so a recursive type terminates. Removed
	// again if the type turns out to have no document.
	m_documents.emplace(id, schema::Type{schema::Type::Bool{}});
	if (std::optional<schema::Type> document = this->document(_type))
	{
		m_documents.insert_or_assign(id, std::move(*document));
		return true;
	}
	m_documents.erase(id);
	return false;
}

std::optional<schema::Type::Wrapper> TypeRegistry::wrapper(std::optional<std::string> _name, Type const& _type)
{
	if (!registerType(_type))
		return std::nullopt;
	return schema::Type::Wrapper{
		std::move(_name),
		schema::Type::Specifier{schema::Type::Reference{schema::materials::ID{_type.identifier()}}}
	};
}

std::optional<std::vector<schema::Type::Wrapper>> TypeRegistry::wrappers(std::vector<Type const*> const& _types)
{
	std::vector<schema::Type::Wrapper> result;
	for (Type const* type: _types)
	{
		if (!type)
			continue;
		std::optional<schema::Type::Wrapper> wrapper = this->wrapper(std::nullopt, *type);
		if (!wrapper)
			return std::nullopt;
		result.emplace_back(std::move(*wrapper));
	}
	return result;
}

std::optional<schema::Type> TypeRegistry::document(Type const& _type)
{
	switch (_type.category())
	{
	case frontend::Type::Category::Address:
	{
		auto const& addressType = dynamic_cast<AddressType const&>(_type);
		return schema::Type{schema::Type::Address{addressType.stateMutability() == StateMutability::Payable}};
	}
	case frontend::Type::Category::Integer:
	{
		auto const& integerType = dynamic_cast<IntegerType const&>(_type);
		if (integerType.isSigned())
			return schema::Type{schema::Type::Int{integerType.numBits()}};
		return schema::Type{schema::Type::UInt{integerType.numBits()}};
	}
	case frontend::Type::Category::FixedPoint:
	{
		auto const& fixedPointType = dynamic_cast<FixedPointType const&>(_type);
		if (fixedPointType.isSigned())
			return schema::Type{schema::Type::Fixed{fixedPointType.numBits(), fixedPointType.fractionalDigits()}};
		return schema::Type{schema::Type::UFixed{fixedPointType.numBits(), fixedPointType.fractionalDigits()}};
	}
	case frontend::Type::Category::Bool:
		return schema::Type{schema::Type::Bool{}};
	case frontend::Type::Category::FixedBytes:
	{
		auto const& bytesType = dynamic_cast<FixedBytesType const&>(_type);
		return schema::Type{schema::Type::Bytes{schema::data::Unsigned{bytesType.numBytes()}}};
	}
	case frontend::Type::Category::Array:
	{
		auto const& arrayType = dynamic_cast<ArrayType const&>(_type);
		if (arrayType.isString())
			return schema::Type{schema::Type::String{}};
		if (arrayType.isByteArray())
			return schema::Type{schema::Type::Bytes{}};
		std::optional<schema::Type::Wrapper> element = wrapper(std::nullopt, *arrayType.baseType());
		if (!element)
			return std::nullopt;
		schema::Type::Array array{std::move(*element), std::nullopt};
		if (!arrayType.isDynamicallySized())
			array.count = schema::data::Unsigned{schema::data::HexValue{toCompactBigEndian(arrayType.length(), 1)}};
		return schema::Type{std::move(array)};
	}
	case frontend::Type::Category::ArraySlice:
	{
		// A slice's representation is the dynamic array it views, and no
		// consumer distinguishes the two, so no separate kind is carried.
		auto const& sliceType = dynamic_cast<ArraySliceType const&>(_type);
		std::optional<schema::Type::Wrapper> element = wrapper(std::nullopt, *sliceType.arrayType().baseType());
		if (!element)
			return std::nullopt;
		return schema::Type{schema::Type::Array{std::move(*element), std::nullopt}};
	}
	case frontend::Type::Category::Contract:
	{
		auto const& contractType = dynamic_cast<ContractType const&>(_type);
		schema::Type::Contract contract;
		if (contractType.contractDefinition().isLibrary())
			contract.kind = schema::Type::Contract::Kind::Library;
		else if (contractType.contractDefinition().isInterface())
			contract.kind = schema::Type::Contract::Kind::Interface;
		contract.payable = contractType.isPayable();
		contract.definition = definition(contractType.contractDefinition());
		return schema::Type{std::move(contract)};
	}
	case frontend::Type::Category::Struct:
	{
		// The members carry the types they have in the struct's data location,
		// e.g. a `string` member of a storage struct is a storage string.
		auto const& structType = dynamic_cast<StructType const&>(_type);
		schema::Type::Struct document;
		for (MemberList::Member const& member: structType.members(nullptr))
		{
			std::optional<schema::Type::Wrapper> field = wrapper(member.name, *member.type);
			if (!field)
				return std::nullopt;
			document.contains.emplace_back(std::move(*field));
		}
		document.definition = definition(structType.structDefinition());
		return schema::Type{std::move(document)};
	}
	case frontend::Type::Category::Enum:
	{
		auto const& enumType = dynamic_cast<EnumType const&>(_type);
		schema::Type::Enum document;
		for (ASTPointer<EnumValue> const& member: enumType.enumDefinition().members())
			document.values.emplace_back(member->name());
		document.definition = definition(enumType.enumDefinition());
		return schema::Type{std::move(document)};
	}
	case frontend::Type::Category::UserDefinedValueType:
	{
		auto const& aliasType = dynamic_cast<UserDefinedValueType const&>(_type);
		std::optional<schema::Type::Wrapper> underlying = wrapper(std::nullopt, aliasType.underlyingType());
		if (!underlying)
			return std::nullopt;
		return schema::Type{schema::Type::Alias{std::move(*underlying), definition(aliasType.definition())}};
	}
	case frontend::Type::Category::Tuple:
	{
		auto const& tupleType = dynamic_cast<TupleType const&>(_type);
		std::optional<std::vector<schema::Type::Wrapper>> components = wrappers(tupleType.components());
		if (!components)
			return std::nullopt;
		return schema::Type{schema::Type::Tuple{std::move(*components)}};
	}
	case frontend::Type::Category::Mapping:
	{
		auto const& mappingType = dynamic_cast<MappingType const&>(_type);
		std::optional<schema::Type::Wrapper> key = wrapper(std::nullopt, *mappingType.keyType());
		std::optional<schema::Type::Wrapper> value = wrapper(std::nullopt, *mappingType.valueType());
		if (!key || !value)
			return std::nullopt;
		return schema::Type{schema::Type::Mapping{std::move(*key), std::move(*value)}};
	}
	case frontend::Type::Category::Function:
	{
		// The schema describes internal and external functions; the other
		// kinds, such as builtins, are not values a variable can hold.
		auto const& functionType = dynamic_cast<FunctionType const&>(_type);
		schema::Type::Function::Visibility visibility;
		if (functionType.kind() == FunctionType::Kind::Internal)
			visibility = schema::Type::Function::Visibility::Internal;
		else if (functionType.kind() == FunctionType::Kind::External)
			visibility = schema::Type::Function::Visibility::External;
		else
			return std::nullopt;

		auto const tupleWrapper = [&](std::vector<Type const*> const& _types) -> std::optional<schema::Type::Wrapper> {
			std::optional<std::vector<schema::Type::Wrapper>> components = wrappers(_types);
			if (!components)
				return std::nullopt;
			return schema::Type::Wrapper{
				std::nullopt,
				schema::Type::Specifier{std::make_shared<schema::Type const>(schema::Type{schema::Type::Tuple{std::move(*components)}})}
			};
		};
		std::optional<schema::Type::Wrapper> parameters = tupleWrapper(functionType.parameterTypes());
		if (!parameters)
			return std::nullopt;
		schema::Type::Function document{visibility, std::move(*parameters), std::nullopt, std::nullopt};
		if (!functionType.returnParameterTypes().empty())
		{
			document.returns = tupleWrapper(functionType.returnParameterTypes());
			if (!document.returns)
				return std::nullopt;
		}
		if (functionType.hasDeclaration())
			document.definition = definition(functionType.declaration());
		return schema::Type{std::move(document)};
	}
	case frontend::Type::Category::RationalNumber:
	case frontend::Type::Category::StringLiteral:
	case frontend::Type::Category::TypeType:
	case frontend::Type::Category::Modifier:
	case frontend::Type::Category::Magic:
	case frontend::Type::Category::Module:
	case frontend::Type::Category::InaccessibleDynamic:
		// Compile-time-only types have no runtime representation to describe.
		return std::nullopt;
	}
	solAssert(false, "");
}

// Pointer expressions and pointers, in the shape ethdebug/format/pointer
// prescribes.

using schema::Pointer;
using Expression = schema::Pointer::Expression;

Expression literal(u256 const& _value)
{
	return {schema::Pointer::Literal{schema::data::Unsigned{schema::data::HexValue{toCompactBigEndian(_value, 1)}}}};
}

Expression variable(std::string _name)
{
	return {schema::Pointer::Variable{std::move(_name)}};
}

Expression wordSize()
{
	return {schema::Pointer::Constant::WordSize};
}

Expression read(std::string _region)
{
	return {schema::Pointer::Read{std::move(_region)}};
}

Expression arithmetic(schema::Pointer::Arithmetic::Operator _operator, schema::Pointer::Operands _operands)
{
	return {schema::Pointer::Arithmetic{_operator, std::move(_operands)}};
}

Expression sum(schema::Pointer::Operands _operands)
{
	return arithmetic(schema::Pointer::Arithmetic::Operator::Sum, std::move(_operands));
}

Expression product(schema::Pointer::Operands _operands)
{
	return arithmetic(schema::Pointer::Arithmetic::Operator::Product, std::move(_operands));
}

Expression difference(Expression _minuend, Expression _subtrahend)
{
	return arithmetic(schema::Pointer::Arithmetic::Operator::Difference, {std::move(_minuend), std::move(_subtrahend)});
}

Expression quotient(Expression _dividend, Expression _divisor)
{
	return arithmetic(schema::Pointer::Arithmetic::Operator::Quotient, {std::move(_dividend), std::move(_divisor)});
}

Expression remainder(Expression _dividend, Expression _divisor)
{
	return arithmetic(schema::Pointer::Arithmetic::Operator::Remainder, {std::move(_dividend), std::move(_divisor)});
}

Expression keccak256(schema::Pointer::Operands _operands)
{
	return {schema::Pointer::Keccak256{std::move(_operands)}};
}

Expression wordSized(Expression _operand)
{
	return {schema::Pointer::Resize{std::nullopt, std::make_shared<Expression const>(std::move(_operand))}};
}

Pointer region(
	schema::Pointer::Location _location,
	std::optional<std::string> _name,
	Expression _slot,
	std::optional<Expression> _offset = std::nullopt,
	std::optional<Expression> _length = std::nullopt
)
{
	return {schema::Pointer::Region{std::move(_name), _location, std::move(_slot), std::move(_offset), std::move(_length)}};
}

Pointer group(std::vector<Pointer> _members)
{
	return {schema::Pointer::Group{std::move(_members)}};
}

Pointer list(Expression _count, std::string _each, Pointer _element)
{
	return {schema::Pointer::List{std::move(_count), std::move(_each), std::make_shared<Pointer const>(std::move(_element))}};
}

Pointer conditional(Expression _condition, Pointer _then, Pointer _otherwise)
{
	return {schema::Pointer::Conditional{
		std::move(_condition),
		std::make_shared<Pointer const>(std::move(_then)),
		std::make_shared<Pointer const>(std::move(_otherwise))
	}};
}

Pointer scope(std::vector<std::pair<std::string, Expression>> _definitions, Pointer _in)
{
	return {schema::Pointer::Scope{std::move(_definitions), std::make_shared<Pointer const>(std::move(_in))}};
}

/// @returns @a _base advanced by @a _slots slots, folding the addition into
/// the literal when possible to keep emitted pointers readable.
Expression advanceSlots(Expression _base, u256 const& _slots)
{
	if (_slots == 0)
		return _base;
	if (auto const* literalBase = std::get_if<schema::Pointer::Literal>(&_base.value))
		if (auto const* hex = std::get_if<schema::data::HexValue>(&literalBase->value.value))
			return literal(u256(fromBigEndian<u256>(hex->value)) + _slots);
	return sum({std::move(_base), literal(_slots)});
}

/// An ethdebug identifier must not start with `$`, which a Solidity identifier
/// may. Every name the producer publishes passes through here.
std::string identifier(std::string const& _name)
{
	solAssert(!_name.empty());
	return _name.front() == '$' ? "_" + _name : _name;
}

/// Builds the pointer of a state variable in storage or transient storage. One
/// builder instance describes one root pointer; mapping keys encountered
/// anywhere in it become template parameters.
class StateVariablePointerBuilder
{
public:
	explicit StateVariablePointerBuilder(schema::Pointer::Location _location): m_location(_location) {}

	/// @a _layoutOffset is the byte offset of the value in its slot as the storage
	/// layout counts it, from the least significant byte.
	Pointer build(Type const& _type, Expression _slot, unsigned _layoutOffset, std::string const& _name)
	{
		if (auto const* mappingType = dynamic_cast<MappingType const*>(&_type))
			return buildMapping(*mappingType, std::move(_slot), _name);

		if (auto const* arrayType = dynamic_cast<ArrayType const*>(&_type))
		{
			if (arrayType->isByteArrayOrString())
				return buildBytesOrString(std::move(_slot), _name);
			if (arrayType->isDynamicallySized())
				return buildDynamicArray(*arrayType, std::move(_slot), _name);
			return elementList(*arrayType, std::move(_slot), literal(arrayType->length()), _name);
		}

		if (auto const* structType = dynamic_cast<StructType const*>(&_type))
			return buildStruct(*structType, std::move(_slot), _name);

		return wholeRegion(_type, std::move(_slot), _layoutOffset, _name);
	}

	std::vector<std::string> takeExpectedParameters() { return std::move(m_expectedParameters); }

private:
	/// A single region covering the value as laid out from its base slot. Used
	/// for value types and as the fallback for compositions that are not (or
	/// cannot be) decomposed further.
	///
	/// A region's offset counts from the most significant byte of the slot,
	/// while the storage layout packs values from the least significant one,
	/// so a value of @a byteLength bytes at layout offset o starts at byte
	/// 32 - o - byteLength of its slot.
	Pointer wholeRegion(Type const& _type, Expression _slot, unsigned _layoutOffset, std::string const& _name)
	{
		u256 const byteLength = u256(_type.storageBytes()) * _type.storageSize();
		if (byteLength >= 32)
		{
			solAssert(_layoutOffset == 0, "A value spanning whole slots cannot be packed.");
			// A region without a length covers the rest of its slot. A value spanning
			// several slots states its length, which the segment addressing of
			// ethdebug/format continues into the slots following the one addressed.
			std::optional<Expression> length;
			if (byteLength != 32)
				length = literal(byteLength);
			return region(m_location, _name, std::move(_slot), std::nullopt, std::move(length));
		}
		solAssert(_layoutOffset + byteLength <= 32, "A packed value does not fit its slot.");
		u256 const offset = 32 - _layoutOffset - byteLength;
		std::optional<Expression> offsetExpression;
		if (offset != 0)
			offsetExpression = literal(offset);
		return region(m_location, _name, std::move(_slot), std::move(offsetExpression), literal(byteLength));
	}

	/// The mapping value lives at `keccak256(pad(key) . slot)`. The key is not
	/// stored anywhere; it becomes a template parameter the debugger must bind.
	Pointer buildMapping(MappingType const& _mappingType, Expression _slot, std::string const& _name)
	{
		std::string const keyParameter = m_expectedParameters.empty()
			? "key"
			: "key" + std::to_string(m_expectedParameters.size());
		m_expectedParameters.emplace_back(keyParameter);

		Expression keyExpression = variable(keyParameter);
		// Value-type keys are hashed as full words; bytes and string keys are
		// hashed as their raw bytes.
		if (_mappingType.keyType()->isValueType())
			keyExpression = wordSized(std::move(keyExpression));

		Expression valueSlot = keccak256({std::move(keyExpression), wordSized(std::move(_slot))});
		return build(*_mappingType.valueType(), std::move(valueSlot), 0, _name);
	}

	/// Dynamic arrays store their element count in the base slot and their data
	/// starting at `keccak256(slot)`.
	Pointer buildDynamicArray(ArrayType const& _arrayType, Expression _slot, std::string const& _name)
	{
		std::string const lengthName = _name + "-length";
		std::string const dataVariable = _name + "-data";

		Pointer lengthRegion = region(m_location, lengthName, _slot);
		Pointer elements = scope(
			{{dataVariable, keccak256({wordSized(std::move(_slot))})}},
			elementList(_arrayType, variable(dataVariable), read(lengthName), _name)
		);

		std::vector<Pointer> members;
		members.emplace_back(std::move(lengthRegion));
		members.emplace_back(std::move(elements));
		return group(std::move(members));
	}

	/// A list of element pointers laid out from @a _dataStart. Value-type
	/// elements narrower than a word are packed multiple to a slot; everything
	/// else advances in whole slots.
	Pointer elementList(ArrayType const& _arrayType, Expression _dataStart, Expression _count, std::string const& _name)
	{
		Type const& elementType = *_arrayType.baseType();
		std::string const indexName = _name + "-index";
		std::string const elementName = _name + "-item";

		Pointer element = [&]() {
			if (elementType.storageBytes() < 32)
			{
				// Packed k to a slot from the least significant byte: element i is
				// in slot i / k, starting at byte 32 - (i % k + 1) * size.
				solAssert(elementType.isValueType(), "Only value types can be packed.");
				u256 const elementBytes = elementType.storageBytes();
				u256 const elementsPerSlot = 32 / elementBytes;
				return region(
					m_location,
					elementName,
					sum({std::move(_dataStart), quotient(variable(indexName), literal(elementsPerSlot))}),
					difference(
						wordSize(),
						product({sum({remainder(variable(indexName), literal(elementsPerSlot)), literal(1)}), literal(elementBytes)})
					),
					literal(elementBytes)
				);
			}
			u256 const slotsPerElement = elementType.storageSize();
			Expression stride = slotsPerElement == 1
				? variable(indexName)
				: product({variable(indexName), literal(slotsPerElement)});
			return build(elementType, sum({std::move(_dataStart), std::move(stride)}), 0, elementName);
		}();

		return list(std::move(_count), indexName, std::move(element));
	}

	/// `bytes` and `string` use the compact encoding: short values keep their
	/// data in the base slot with the doubled length in the last byte; long
	/// values keep `2 * length + 1` in the base slot and their data starting at
	/// `keccak256(slot)`.
	Pointer buildBytesOrString(Expression _slot, std::string const& _name)
	{
		std::string const lengthFlagName = _name + "-length-flag";
		std::string const longLengthName = _name + "-long-length";
		std::string const lengthVariable = _name + "-length";
		std::string const dataVariable = _name + "-data";

		Pointer lengthFlagRegion = region(
			m_location,
			lengthFlagName,
			_slot,
			difference(wordSize(), literal(1)),
			literal(1)
		);

		Pointer shortValue = scope(
			{{lengthVariable, quotient(read(lengthFlagName), literal(2))}},
			region(m_location, _name, _slot, std::nullopt, variable(lengthVariable))
		);

		Pointer longLengthRegion = region(m_location, longLengthName, _slot);
		Pointer longData = scope(
			{
				{lengthVariable, quotient(difference(read(longLengthName), literal(1)), literal(2))},
				{dataVariable, keccak256({wordSized(std::move(_slot))})}
			},
			region(m_location, _name, variable(dataVariable), std::nullopt, variable(lengthVariable))
		);
		std::vector<Pointer> longMembers;
		longMembers.emplace_back(std::move(longLengthRegion));
		longMembers.emplace_back(std::move(longData));

		// The flag byte is even (2 * length) for short values and odd
		// (2 * length + 1) for long ones, so `(flag + 1) % 2` selects short.
		Pointer value = conditional(
			remainder(sum({read(lengthFlagName), literal(1)}), literal(2)),
			std::move(shortValue),
			group(std::move(longMembers))
		);

		std::vector<Pointer> members;
		members.emplace_back(std::move(lengthFlagRegion));
		members.emplace_back(std::move(value));
		return group(std::move(members));
	}

	Pointer buildStruct(StructType const& _structType, Expression _slot, std::string const& _name)
	{
		// Recursive structs and pathological nesting fall back to a region
		// covering the struct's slots.
		if (m_depth >= maxCompositionDepth || m_structsOnPath.count(_structType.identifier()))
			return wholeRegion(_structType, std::move(_slot), 0, _name);

		m_structsOnPath.insert(_structType.identifier());
		++m_depth;

		std::vector<Pointer> members;
		for (ASTPointer<VariableDeclaration> const& member: _structType.structDefinition().members())
		{
			if (!member->annotation().type)
				continue;
			auto const& [slotOffset, byteOffset] = _structType.storageOffsetsOfMember(member->name());
			members.emplace_back(build(
				*member->annotation().type,
				advanceSlots(_slot, slotOffset),
				byteOffset,
				_name + "-" + member->name()
			));
		}

		--m_depth;
		m_structsOnPath.erase(_structType.identifier());

		if (members.empty())
			return wholeRegion(_structType, std::move(_slot), 0, _name);
		return group(std::move(members));
	}

	static constexpr unsigned maxCompositionDepth = 16;

	schema::Pointer::Location m_location;
	std::vector<std::string> m_expectedParameters;
	std::set<std::string> m_structsOnPath;
	unsigned m_depth = 0;
};

/// The name the state variable's pointer template is published under: the
/// location and the AST IDs of the most derived contract and the variable, so
/// that an inherited variable at a different slot in another contract of the
/// same compilation does not clash.
std::string templateName(
	ContractDefinition const& _contract,
	VariableDeclaration const& _variable,
	schema::Pointer::Location _location
)
{
	solAssert(_location == schema::Pointer::Location::Storage || _location == schema::Pointer::Location::Transient);
	std::string const prefix = _location == schema::Pointer::Location::Storage ? "storage_" : "transient_";
	return prefix + std::to_string(_contract.id()) + "_" + std::to_string(_variable.id());
}

schema::Pointer::Template stateVariableTemplate(
	VariableDeclaration const& _variable,
	u256 const& _slot,
	unsigned _offset,
	schema::Pointer::Location _location
)
{
	solAssert(_variable.annotation().type, "State variable type expected.");

	StateVariablePointerBuilder builder{_location};
	Pointer pointer = builder.build(
		*_variable.annotation().type,
		literal(_slot),
		_offset,
		identifier(_variable.name())
	);
	return schema::Pointer::Template{builder.takeExpectedParameters(), std::make_shared<Pointer const>(std::move(pointer))};
}

void registerCallableTypes(TypeRegistry& _types, CallableDeclaration const& _callable)
{
	for (ASTPointer<VariableDeclaration> const& parameter: _callable.parameters())
		if (parameter->annotation().type)
			_types.registerType(*parameter->annotation().type);
	// Modifiers have no return parameter list.
	if (_callable.returnParameterList())
		for (ASTPointer<VariableDeclaration> const& returnParameter: _callable.returnParameters())
			if (returnParameter->annotation().type)
				_types.registerType(*returnParameter->annotation().type);
}

}

void ethdebug::Resources::merge(Resources _other)
{
	for (auto& [id, document]: _other.types)
		types.insert_or_assign(id, std::move(document));
	for (auto& [name, pointerTemplate]: _other.pointers)
		pointers.insert_or_assign(name, std::move(pointerTemplate));
}

ethdebug::Resources ethdebug::resources(ContractDefinition const& _contract, std::map<std::string, unsigned> const& _sourceIndices)
{
	TypeRegistry types{_sourceIndices};
	Resources result;

	auto const* typeType = dynamic_cast<TypeType const*>(_contract.type());
	solAssert(typeType, "Contract TypeType expected.");
	auto const* contractType = dynamic_cast<ContractType const*>(typeType->actualType());
	solAssert(contractType, "Contract type expected.");

	auto const addStateVariables = [&](DataLocation _dataLocation, schema::Pointer::Location _location) {
		for (auto const& [variable, slot, offset]: contractType->linearizedStateVariables(_dataLocation))
		{
			if (variable->name().empty())
				continue;
			types.registerType(*variable->annotation().type);
			result.pointers.insert_or_assign(
				templateName(_contract, *variable, _location),
				stateVariableTemplate(*variable, slot, offset, _location)
			);
		}
	};
	addStateVariables(DataLocation::Storage, schema::Pointer::Location::Storage);
	addStateVariables(DataLocation::Transient, schema::Pointer::Location::Transient);

	// Inherited functions and modifiers are compiled into the most derived
	// contract, so every linearized base contract contributes its types.
	for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts)
	{
		for (FunctionDefinition const* function: contract->definedFunctions())
			registerCallableTypes(types, *function);
		for (ModifierDefinition const* modifier: contract->functionModifiers())
			registerCallableTypes(types, *modifier);
	}

	// Free functions and internal library functions reachable through imports
	// are compiled into the contract as well.
	SourceUnit const& sourceUnit = _contract.sourceUnit();
	std::set<SourceUnit const*> sourceUnits = sourceUnit.referencedSourceUnits(true);
	sourceUnits.insert(&sourceUnit);
	for (SourceUnit const* unit: sourceUnits)
		for (ASTPointer<ASTNode> const& node: unit->nodes())
		{
			if (auto const* freeFunction = dynamic_cast<FunctionDefinition const*>(node.get()))
				registerCallableTypes(types, *freeFunction);
			else if (auto const* library = dynamic_cast<ContractDefinition const*>(node.get()); library && library->isLibrary())
				for (FunctionDefinition const* function: library->definedFunctions())
					if (function->visibility() <= Visibility::Internal)
						registerCallableTypes(types, *function);
		}

	result.types = types.takeDocuments();
	return result;
}
