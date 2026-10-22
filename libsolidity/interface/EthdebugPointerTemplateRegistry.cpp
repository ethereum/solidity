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

#include <libsolidity/interface/EthdebugPointerTemplateRegistry.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

#include <libsolutil/Numeric.h>
#include <libsolutil/Visitor.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::ethdebug;

namespace schema = solidity::evmasm::ethdebug::schema;

namespace
{

using Pointer = schema::Pointer;
using Expression = schema::Pointer::Expression;

// Pointer expressions and pointers, as ethdebug/format/pointer
// prescribes.

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
	{
		u256 const base = std::visit(util::GenericVisitor{
			[](std::uint64_t const _value) { return u256(_value); },
			[](schema::data::HexValue const& _value) { return fromBigEndian<u256>(_value.value); }
		}, literalBase->value.value);
		return literal(base + _slots);
	}
	return sum({std::move(_base), literal(_slots)});
}

/// A single region covering a value of value type as laid out in its slot,
/// in storage or transient storage.
///
/// A region's offset counts from the most significant byte of the slot,
/// while the storage layout packs values from the least significant one,
/// so a value of @a byteLength bytes at layout offset @a _layoutOffset starts
/// at byte 32 - _layoutOffset - byteLength of its slot. A region without an
/// offset and a length covers the whole slot.
Pointer valueRegion(
	Type const& _type,
	schema::Pointer::Location _location,
	Expression _slot,
	unsigned _layoutOffset,
	std::optional<std::string> _name
)
{
	solAssert(_type.storageSize() == 1, "A value type fits one slot.");
	u256 const byteLength = _type.storageBytes();
	if (byteLength == 32)
	{
		solAssert(_layoutOffset == 0, "A whole word cannot be packed.");
		return region(_location, std::move(_name), std::move(_slot));
	}
	solAssert(_layoutOffset + byteLength <= 32, "A packed value does not fit its slot.");
	u256 const offset = 32 - _layoutOffset - byteLength;
	std::optional<Expression> offsetExpression;
	if (offset != 0)
		offsetExpression = literal(offset);
	return region(_location, std::move(_name), std::move(_slot), std::move(offsetExpression), literal(byteLength));
}

}

void PointerTemplateRegistry::registerType(Type const& _type)
{
	if (!needsTemplate(_type))
		return;
	std::string const id = _type.identifier();
	// A type nested in itself references its own template, which is being
	// built; the reference is resolved lazily by the consumer.
	if (m_templates.count(id) || m_building.count(id))
		return;
	m_building.insert(id);
	Built built = build(_type);
	m_building.erase(id);
	m_templates.emplace(id, std::move(built));
}

std::map<std::string, schema::Pointer::Template> PointerTemplateRegistry::takeTemplates()
{
	std::map<std::string, schema::Pointer::Template> result;
	for (auto& [id, built]: m_templates)
		result.emplace(id, schema::Pointer::Template{
			std::move(built.expect),
			std::make_shared<Pointer const>(std::move(built.body))
		});
	m_templates.clear();
	return result;
}

bool PointerTemplateRegistry::needsTemplate(Type const& _type)
{
	return
		dynamic_cast<StructType const*>(&_type) ||
		dynamic_cast<ArrayType const*>(&_type) ||
		dynamic_cast<MappingType const*>(&_type);
}

void PointerTemplateRegistry::produce(std::vector<std::string>& _produced, std::string const& _name)
{
	if (std::find(_produced.begin(), _produced.end(), _name) == _produced.end())
		_produced.emplace_back(_name);
}

PointerTemplateRegistry::Built PointerTemplateRegistry::build(Type const& _type)
{
	if (auto const* mappingType = dynamic_cast<MappingType const*>(&_type))
		return buildMapping(*mappingType);
	if (auto const* arrayType = dynamic_cast<ArrayType const*>(&_type))
		return buildArray(*arrayType);
	auto const* structType = dynamic_cast<StructType const*>(&_type);
	solAssert(structType, "Only structs, arrays and mappings have templates.");
	return buildStruct(*structType);
}

Pointer PointerTemplateRegistry::component(
	Type const& _type,
	Expression _slot,
	unsigned _layoutOffset,
	std::string const& _name,
	std::vector<std::string>& _produced
)
{
	if (dynamic_cast<MappingType const*>(&_type))
	{
		// The entries of a mapping are at hashed slots; its base slot is what
		// its template expects to locate them.
		registerType(_type);
		produce(_produced, _name);
		return region(schema::Pointer::Location::Storage, _name, std::move(_slot));
	}
	if (needsTemplate(_type))
	{
		registerType(_type);
		std::string const id = _type.identifier();
		std::vector<std::pair<std::string, std::string>> yields;
		// A template still being built produces names not known yet; the
		// names of such a self-reference pass through unprefixed.
		if (auto const it = m_templates.find(id); it != m_templates.end())
			for (std::string const& produced: it->second.produced)
			{
				yields.emplace_back(produced, _name + "-" + produced);
				produce(_produced, _name + "-" + produced);
			}
		Pointer reference{schema::Pointer::TemplateReference{id, std::move(yields)}};
		// The referenced template expects the base slot as `slot`; a value at
		// the enclosing base slot passes it through as it is.
		if (auto const* slot = std::get_if<schema::Pointer::Variable>(&_slot.value); slot && slot->identifier == "slot")
			return reference;
		return scope({{"slot", std::move(_slot)}}, std::move(reference));
	}
	produce(_produced, _name);
	return valueRegion(_type, schema::Pointer::Location::Storage, std::move(_slot), _layoutOffset, _name);
}

PointerTemplateRegistry::Built PointerTemplateRegistry::buildStruct(StructType const& _structType)
{
	std::vector<std::string> produced;
	std::vector<Pointer> members;
	for (MemberList::Member const& member: _structType.members(nullptr))
	{
		auto const& [slotOffset, byteOffset] = _structType.storageOffsetsOfMember(member.name);
		members.emplace_back(component(
			*member.type,
			advanceSlots(variable("slot"), slotOffset),
			byteOffset,
			member.name,
			produced
		));
	}
	solAssert(!members.empty(), "A struct has members.");
	return Built{{"slot"}, group(std::move(members)), std::move(produced)};
}

PointerTemplateRegistry::Built PointerTemplateRegistry::buildArray(ArrayType const& _arrayType)
{
	std::vector<std::string> produced;
	Pointer body = [&]() {
		if (_arrayType.isByteArrayOrString())
			return bytesOrString(produced);
		if (_arrayType.isDynamicallySized())
			return dynamicArray(_arrayType, produced);
		return elementList(_arrayType, variable("slot"), literal(_arrayType.length()), produced);
	}();
	return Built{{"slot"}, std::move(body), std::move(produced)};
}

PointerTemplateRegistry::Built PointerTemplateRegistry::buildMapping(MappingType const& _mappingType)
{
	std::vector<std::string> produced;
	Expression keyExpression = variable("key");
	// Value-type keys are hashed as full words; bytes and string keys are
	// hashed as their raw bytes.
	if (_mappingType.keyType()->isValueType())
		keyExpression = wordSized(std::move(keyExpression));
	Expression valueSlot = keccak256({std::move(keyExpression), wordSized(variable("slot"))});
	Pointer body = component(*_mappingType.valueType(), std::move(valueSlot), 0, "value", produced);
	return Built{{"slot", "key"}, std::move(body), std::move(produced)};
}

Pointer PointerTemplateRegistry::dynamicArray(ArrayType const& _arrayType, std::vector<std::string>& _produced)
{
	Pointer lengthRegion = region(schema::Pointer::Location::Storage, "length", variable("slot"));
	produce(_produced, "length");
	Pointer elements = scope(
		{{"data", keccak256({wordSized(variable("slot"))})}},
		elementList(_arrayType, variable("data"), read("length"), _produced)
	);

	std::vector<Pointer> members;
	members.emplace_back(std::move(lengthRegion));
	members.emplace_back(std::move(elements));
	return group(std::move(members));
}

Pointer PointerTemplateRegistry::elementList(
	ArrayType const& _arrayType,
	Expression _dataStart,
	Expression _count,
	std::vector<std::string>& _produced
)
{
	Type const& elementType = *_arrayType.baseType();
	Pointer element = [&]() {
		if (elementType.storageBytes() < 32)
		{
			// Packed k to a slot from the least significant byte: element i is
			// in slot i / k, starting at byte 32 - (i % k + 1) * size.
			solAssert(elementType.isValueType(), "Only value types can be packed.");
			u256 const elementBytes = elementType.storageBytes();
			u256 const elementsPerSlot = 32 / elementBytes;
			Expression position = sum({remainder(variable("index"), literal(elementsPerSlot)), literal(1)});
			if (elementBytes != 1)
				position = product({std::move(position), literal(elementBytes)});
			produce(_produced, "item");
			return region(
				schema::Pointer::Location::Storage,
				"item",
				sum({std::move(_dataStart), quotient(variable("index"), literal(elementsPerSlot))}),
				difference(wordSize(), std::move(position)),
				literal(elementBytes)
			);
		}
		u256 const slotsPerElement = elementType.storageSize();
		Expression stride = slotsPerElement == 1
			? variable("index")
			: product({variable("index"), literal(slotsPerElement)});
		return component(elementType, sum({std::move(_dataStart), std::move(stride)}), 0, "item", _produced);
	}();

	return list(std::move(_count), "index", std::move(element));
}

Pointer PointerTemplateRegistry::bytesOrString(std::vector<std::string>& _produced)
{
	Pointer lengthFlagRegion = region(
		schema::Pointer::Location::Storage,
		"length-flag",
		variable("slot"),
		difference(wordSize(), literal(1)),
		literal(1)
	);

	Pointer shortValue = scope(
		{{"length", quotient(read("length-flag"), literal(2))}},
		region(schema::Pointer::Location::Storage, "data", variable("slot"), std::nullopt, variable("length"))
	);

	Pointer longLengthRegion = region(schema::Pointer::Location::Storage, "long-length", variable("slot"));
	Pointer longData = scope(
		{
			{"length", quotient(difference(read("long-length"), literal(1)), literal(2))},
			{"start", keccak256({wordSized(variable("slot"))})}
		},
		region(schema::Pointer::Location::Storage, "data", variable("start"), std::nullopt, variable("length"))
	);
	std::vector<Pointer> longMembers;
	longMembers.emplace_back(std::move(longLengthRegion));
	longMembers.emplace_back(std::move(longData));

	// The flag byte is even (2 * length) for short values and odd
	// (2 * length + 1) for long ones, so `(flag + 1) % 2` selects short.
	Pointer value = conditional(
		remainder(sum({read("length-flag"), literal(1)}), literal(2)),
		std::move(shortValue),
		group(std::move(longMembers))
	);

	for (char const* name: {"length-flag", "data", "long-length"})
		produce(_produced, name);
	std::vector<Pointer> members;
	members.emplace_back(std::move(lengthFlagRegion));
	members.emplace_back(std::move(value));
	return group(std::move(members));
}
