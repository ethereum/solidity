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

#include <libsolidity/interface/EthdebugTypeRegistry.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>

#include <libsolutil/Numeric.h>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::ethdebug;

namespace schema = solidity::evmasm::ethdebug::schema;

TypeRegistry::TypeRegistry(std::map<std::string, unsigned> const& _sourceIndices):
	m_sourceIndices(_sourceIndices)
{}

void TypeRegistry::registerType(Type const& _type)
{
	Type const& type = canonical(_type);
	std::string const id = type.identifier();
	if (m_documents.count(id))
		return;
	// Present before descending, so that a recursive type terminates.
	m_documents.emplace(id, schema::Type{schema::Type::Bool{}});
	m_documents.insert_or_assign(id, document(type));
}

void TypeRegistry::registerCallableTypes(CallableDeclaration const& _callable)
{
	for (ASTPointer<VariableDeclaration> const& parameter: _callable.parameters())
	{
		solAssert(parameter->annotation().type, "Parameter without a type.");
		registerType(*parameter->annotation().type);
	}
	// Modifiers have no return parameter list.
	if (_callable.returnParameterList())
		for (ASTPointer<VariableDeclaration> const& returnParameter: _callable.returnParameters())
		{
			solAssert(returnParameter->annotation().type, "Return parameter without a type.");
			registerType(*returnParameter->annotation().type);
		}
}

Type const& TypeRegistry::canonical(Type const& _type)
{
	if (auto const* slice = dynamic_cast<ArraySliceType const*>(&_type))
		return canonical(slice->arrayType());
	return *TypeProvider::withLocationIfReference(DataLocation::Storage, &_type);
}

std::optional<schema::materials::SourceRange> TypeRegistry::sourceRange(langutil::SourceLocation const& _location) const
{
	if (!_location.sourceName || !_location.hasText())
		return std::nullopt;
	auto const it = m_sourceIndices.find(*_location.sourceName);
	if (it == m_sourceIndices.end())
		return std::nullopt;

	return schema::materials::SourceRange{
		.source = {
			.id = schema::materials::ID{static_cast<uint64_t>(it->second)},
			.type = std::nullopt
		},
		.range = schema::materials::SourceRange::Range{
			.length = schema::data::Unsigned{_location.end - _location.start},
			.offset = schema::data::Unsigned{_location.start}
		}
	};
}

schema::Type::Definition TypeRegistry::definition(Declaration const& _declaration) const
{
	std::optional<schema::materials::SourceRange> location = sourceRange(_declaration.location());
	solAssert(location, "Declaration without a source location.");
	return schema::Type::Definition{
		_declaration.name().empty() ? std::nullopt : std::optional<std::string>{_declaration.name()},
		std::move(location)
	};
}

schema::Type::Wrapper TypeRegistry::wrapper(std::optional<std::string> _name, Type const& _type)
{
	registerType(_type);
	return schema::Type::Wrapper{
		.name = std::move(_name),
		.type = schema::Type::Specifier{schema::Type::Reference{schema::materials::ID{canonical(_type).identifier()}}}
	};
}

std::vector<schema::Type::Wrapper> TypeRegistry::wrappers(std::vector<Type const*> const& _types)
{
	std::vector<schema::Type::Wrapper> result;
	for (Type const* type: _types)
	{
		solAssert(type, "Tuple component without a type.");
		result.push_back(wrapper(std::nullopt, *type));
	}
	return result;
}

schema::Type TypeRegistry::document(Type const& _type)
{
	switch (_type.category())
	{
	case frontend::Type::Category::Address:
	{
		auto const& addressType = dynamic_cast<AddressType const&>(_type);
		return schema::Type{schema::Type::Address{.payable = addressType.stateMutability() == StateMutability::Payable}};
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
		return schema::Type{schema::Type::Bytes{.size = schema::data::Unsigned{bytesType.numBytes()}}};
	}
	case frontend::Type::Category::Array:
	{
		auto const& arrayType = dynamic_cast<ArrayType const&>(_type);
		if (arrayType.isString())
			return schema::Type{schema::Type::String{}};
		if (arrayType.isByteArray())
			return schema::Type{schema::Type::Bytes{}};
		std::optional<schema::data::Unsigned> count;
		if (!arrayType.isDynamicallySized())
			count = schema::data::Unsigned{schema::data::HexValue{toCompactBigEndian(arrayType.length(), 1)}};
		return schema::Type{schema::Type::Array{
			.contains = wrapper(std::nullopt, *arrayType.baseType()),
			.count = std::move(count)
		}};
	}
	case frontend::Type::Category::ArraySlice:
		solAssert(false, "Array slices are documented as the arrays they view.");
	case frontend::Type::Category::Contract:
	{
		auto const& contractType = dynamic_cast<ContractType const&>(_type);
		ContractDefinition const& contract = contractType.contractDefinition();
		schema::Type::Contract::Kind kind = schema::Type::Contract::Kind::Contract;
		if (contract.isLibrary())
			kind = schema::Type::Contract::Kind::Library;
		else if (contract.isInterface())
			kind = schema::Type::Contract::Kind::Interface;
		return schema::Type{schema::Type::Contract{
			.kind = kind,
			.payable = contractType.isPayable(),
			.definition = definition(contract)
		}};
	}
	case frontend::Type::Category::Struct:
	{
		// The members carry the types they have in the struct's data location,
		// e.g. a `string` member of a storage struct is a storage string.
		auto const& structType = dynamic_cast<StructType const&>(_type);
		std::vector<schema::Type::Wrapper> members;
		for (MemberList::Member const& member: structType.members(nullptr))
			members.push_back(wrapper(member.name, *member.type));
		return schema::Type{schema::Type::Struct{
			.contains = std::move(members),
			.definition = definition(structType.structDefinition())
		}};
	}
	case frontend::Type::Category::Enum:
	{
		auto const& enumType = dynamic_cast<EnumType const&>(_type);
		std::vector<std::string> values;
		for (ASTPointer<EnumValue> const& member: enumType.enumDefinition().members())
			values.emplace_back(member->name());
		return schema::Type{schema::Type::Enum{
			.values = std::move(values),
			.definition = definition(enumType.enumDefinition())
		}};
	}
	case frontend::Type::Category::UserDefinedValueType:
	{
		auto const& aliasType = dynamic_cast<UserDefinedValueType const&>(_type);
		return schema::Type{schema::Type::Alias{
			.contains = wrapper(std::nullopt, aliasType.underlyingType()),
			.definition = definition(aliasType.definition())
		}};
	}
	case frontend::Type::Category::Tuple:
	{
		auto const& tupleType = dynamic_cast<TupleType const&>(_type);
		return schema::Type{schema::Type::Tuple{.contains = wrappers(tupleType.components())}};
	}
	case frontend::Type::Category::Mapping:
	{
		auto const& mappingType = dynamic_cast<MappingType const&>(_type);
		return schema::Type{schema::Type::Mapping{
			.key = wrapper(std::nullopt, *mappingType.keyType()),
			.value = wrapper(std::nullopt, *mappingType.valueType())
		}};
	}
	case frontend::Type::Category::Function:
	{
		// Of the kinds of functions, only internal and external ones are values
		// a variable can hold.
		auto const& functionType = dynamic_cast<FunctionType const&>(_type);
		solAssert(
			functionType.kind() == FunctionType::Kind::Internal || functionType.kind() == FunctionType::Kind::External,
			"Only internal and external function types are nameable."
		);

		// The parameter and return lists are tuple types, documented and
		// referenced like any other type.
		auto const tupleWrapper = [&](std::vector<Type const*> const& _types) {
			return wrapper(std::nullopt, *TypeProvider::tuple(_types));
		};
		schema::Type::Wrapper parameters = tupleWrapper(functionType.parameterTypes());
		std::optional<schema::Type::Wrapper> returns;
		if (!functionType.returnParameterTypes().empty())
			returns = tupleWrapper(functionType.returnParameterTypes());
		std::optional<schema::Type::Definition> definition;
		if (functionType.hasDeclaration())
			definition = this->definition(functionType.declaration());
		return schema::Type{schema::Type::Function{
			.visibility = functionType.kind() == FunctionType::Kind::Internal ?
				schema::Type::Function::Visibility::Internal :
				schema::Type::Function::Visibility::External,
			.parameters = std::move(parameters),
			.returns = std::move(returns),
			.definition = std::move(definition)
		}};
	}
	case frontend::Type::Category::RationalNumber:
	case frontend::Type::Category::StringLiteral:
	case frontend::Type::Category::TypeType:
	case frontend::Type::Category::Modifier:
	case frontend::Type::Category::Magic:
	case frontend::Type::Category::Module:
	case frontend::Type::Category::InaccessibleDynamic:
		solAssert(false, "Unnameable types have no runtime representation to describe.");
	}
	solAssert(false, "");
}
