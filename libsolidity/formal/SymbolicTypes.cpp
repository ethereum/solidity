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

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/formal/EncodingContext.h>

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>
#include <libsolutil/CommonData.h>
#include <memory>
#include <vector>

using namespace std;
using namespace solidity::util;
using namespace solidity::smtutil;

namespace solidity::frontend::smt
{

SortPointer smtSort(frontend::Type const& _type)
{
	switch (smtKind(_type))
	{
	case Kind::Int:
		if (auto const* intType = dynamic_cast<IntegerType const*>(&_type))
			return SortProvider::intSort(intType->isSigned());
		if (auto const* fixedType = dynamic_cast<FixedPointType const*>(&_type))
			return SortProvider::intSort(fixedType->isSigned());
		return SortProvider::uintSort;
	case Kind::Bool:
		return SortProvider::boolSort;
	case Kind::Function:
	{
		auto fType = dynamic_cast<frontend::FunctionType const*>(&_type);
		solAssert(fType, "");
		vector<SortPointer> parameterSorts = smtSort(fType->parameterTypes());
		auto returnTypes = fType->returnParameterTypes();
		SortPointer returnSort;
		// TODO change this when we support tuples.
		if (returnTypes.size() == 0)
			// We cannot declare functions without a return sort, so we use the smallest.
			returnSort = SortProvider::boolSort;
		else if (returnTypes.size() > 1)
			// Abstract sort.
			returnSort = SortProvider::uintSort;
		else
			returnSort = smtSort(*returnTypes.front());
		return make_shared<FunctionSort>(parameterSorts, returnSort);
	}
	case Kind::Array:
	{
		shared_ptr<ArraySort> array;
		if (isMapping(_type))
		{
			auto mapType = dynamic_cast<frontend::MappingType const*>(&_type);
			solAssert(mapType, "");
			array = make_shared<ArraySort>(smtSortAbstractFunction(*mapType->keyType()), smtSortAbstractFunction(*mapType->valueType()));
		}
		else if (isStringLiteral(_type))
		{
			auto stringLitType = dynamic_cast<frontend::StringLiteralType const*>(&_type);
			solAssert(stringLitType, "");
			array = make_shared<ArraySort>(SortProvider::uintSort, SortProvider::uintSort);
		}
		else
		{
			frontend::ArrayType const* arrayType = nullptr;
			if (auto const* arr = dynamic_cast<frontend::ArrayType const*>(&_type))
				arrayType = arr;
			else if (auto const* slice = dynamic_cast<frontend::ArraySliceType const*>(&_type))
				arrayType = &slice->arrayType();
			else
				solAssert(false, "");

			solAssert(arrayType, "");
			array = make_shared<ArraySort>(SortProvider::uintSort, smtSortAbstractFunction(*arrayType->baseType()));
		}

		string tupleName;
		auto sliceArrayType = dynamic_cast<ArraySliceType const*>(&_type);
		ArrayType const* arrayType = sliceArrayType ? &sliceArrayType->arrayType() : dynamic_cast<ArrayType const*>(&_type);
		if (
			(arrayType && arrayType->isByteArrayOrString()) ||
			_type.category() == frontend::Type::Category::StringLiteral
		)
			tupleName = "bytes";
		else if (arrayType)
		{
			auto baseType = arrayType->baseType();
			// Solidity allows implicit conversion also when assigning arrays.
			// So if the base type potentially has a size, that size cannot go
			// in the tuple's name.
			if (auto tupleSort = dynamic_pointer_cast<TupleSort>(array->range))
				tupleName = tupleSort->name;
			else if (
				baseType->category() == frontend::Type::Category::Integer ||
				baseType->category() == frontend::Type::Category::FixedPoint
			)
				tupleName = "uint";
			else if (baseType->category() == frontend::Type::Category::FixedBytes)
				tupleName = "fixedbytes";
			else
				tupleName = arrayType->baseType()->toString(true);

			tupleName += "_array";
		}
		else
			tupleName = _type.toString(true);

		tupleName += "_tuple";

		return make_shared<TupleSort>(
			tupleName,
			vector<string>{tupleName + "_accessor_array", tupleName + "_accessor_length"},
			vector<SortPointer>{array, SortProvider::uintSort}
		);
	}
	case Kind::Tuple:
	{
		vector<string> members;
		auto const& tupleName = _type.toString(true);
		vector<SortPointer> sorts;

		if (auto const* tupleType = dynamic_cast<frontend::TupleType const*>(&_type))
		{
			auto const& components = tupleType->components();
			for (unsigned i = 0; i < components.size(); ++i)
				members.emplace_back(tupleName + "_accessor_" + to_string(i));
			sorts = smtSortAbstractFunction(tupleType->components());
		}
		else if (auto const* structType = dynamic_cast<frontend::StructType const*>(&_type))
		{
			solAssert(!structType->recursive(), "");
			auto const& structMembers = structType->structDefinition().members();
			for (auto member: structMembers)
				members.emplace_back(tupleName + "_accessor_" + member->name());
			sorts = smtSortAbstractFunction(applyMap(
				structMembers,
				[](auto var) { return var->type(); }
			));
		}
		else
			solAssert(false, "");

		return make_shared<TupleSort>(tupleName, members, sorts);
	}
	default:
		// Abstract case.
		return SortProvider::uintSort;
	}
}

vector<SortPointer> smtSort(vector<frontend::Type const*> const& _types)
{
	vector<SortPointer> sorts;
	for (auto const& type: _types)
		sorts.push_back(smtSort(*type));
	return sorts;
}

SortPointer smtSortAbstractFunction(frontend::Type const& _type)
{
	if (isFunction(_type))
		return SortProvider::uintSort;
	return smtSort(_type);
}

vector<SortPointer> smtSortAbstractFunction(vector<frontend::Type const*> const& _types)
{
	vector<SortPointer> sorts;
	for (auto const& type: _types)
		if (type)
			sorts.push_back(smtSortAbstractFunction(*type));
		else
			sorts.push_back(SortProvider::uintSort);
	return sorts;
}

Kind smtKind(frontend::Type const& _type)
{
	if (isNumber(_type))
		return Kind::Int;
	else if (isBool(_type))
		return Kind::Bool;
	else if (isFunction(_type))
		return Kind::Function;
	else if (isMapping(_type) || isArray(_type))
		return Kind::Array;
	else if (isTuple(_type) || isNonRecursiveStruct(_type))
		return Kind::Tuple;
	// Abstract case.
	return Kind::Int;
}

bool isSupportedType(frontend::Type const& _type)
{
	return isNumber(_type) ||
		isBool(_type) ||
		isMapping(_type) ||
		isArray(_type) ||
		isTuple(_type) ||
		isNonRecursiveStruct(_type);
}

bool isSupportedTypeDeclaration(frontend::Type const& _type)
{
	return isSupportedType(_type) ||
		isFunction(_type);
}

pair<bool, shared_ptr<SymbolicVariable>> newSymbolicVariable(
	frontend::Type const& _type,
	std::string const& _uniqueName,
	EncodingContext& _context
)
{
	bool abstract = false;
	shared_ptr<SymbolicVariable> var;
	frontend::Type const* type = &_type;

	if (auto userType = dynamic_cast<UserDefinedValueType const*>(type))
		return newSymbolicVariable(userType->underlyingType(), _uniqueName, _context);

	if (!isSupportedTypeDeclaration(_type))
	{
		abstract = true;
		var = make_shared<SymbolicIntVariable>(frontend::TypeProvider::uint256(), type, _uniqueName, _context);
	}
	else if (isBool(_type))
		var = make_shared<SymbolicBoolVariable>(type, _uniqueName, _context);
	else if (isFunction(_type))
	{
		auto const& fType = dynamic_cast<FunctionType const*>(type);
		auto const& paramsIn = fType->parameterTypes();
		auto const& paramsOut = fType->returnParameterTypes();
		auto findFunctionParam = [&](auto&& params) {
			return find_if(
				begin(params),
				end(params),
				[&](frontend::Type const* _paramType) { return _paramType->category() == frontend::Type::Category::Function; }
			);
		};
		if (
			findFunctionParam(paramsIn) != end(paramsIn) ||
			findFunctionParam(paramsOut) != end(paramsOut)
		)
		{
			abstract = true;
			var = make_shared<SymbolicIntVariable>(TypeProvider::uint256(), type, _uniqueName, _context);
		}
		else
			var = make_shared<SymbolicFunctionVariable>(type, _uniqueName, _context);
	}
	else if (isInteger(_type))
		var = make_shared<SymbolicIntVariable>(type, type, _uniqueName, _context);
	else if (isFixedPoint(_type))
		var = make_shared<SymbolicIntVariable>(type, type, _uniqueName, _context);
	else if (isFixedBytes(_type))
	{
		auto fixedBytesType = dynamic_cast<frontend::FixedBytesType const*>(type);
		solAssert(fixedBytesType, "");
		var = make_shared<SymbolicFixedBytesVariable>(type, fixedBytesType->numBytes(), _uniqueName, _context);
	}
	else if (isAddress(_type) || isContract(_type))
		var = make_shared<SymbolicAddressVariable>(_uniqueName, _context);
	else if (isEnum(_type))
		var = make_shared<SymbolicEnumVariable>(type, _uniqueName, _context);
	else if (isRational(_type))
	{
		auto rational = dynamic_cast<frontend::RationalNumberType const*>(&_type);
		solAssert(rational, "");
		if (rational->isFractional())
			var = make_shared<SymbolicIntVariable>(frontend::TypeProvider::uint256(), type, _uniqueName, _context);
		else
			var = make_shared<SymbolicIntVariable>(type, type, _uniqueName, _context);
	}
	else if (isMapping(_type) || isArray(_type))
		var = make_shared<SymbolicArrayVariable>(type, type, _uniqueName, _context);
	else if (isTuple(_type))
		var = make_shared<SymbolicTupleVariable>(type, _uniqueName, _context);
	else if (isStringLiteral(_type))
	{
		auto stringType = TypeProvider::stringMemory();
		var = make_shared<SymbolicArrayVariable>(stringType, type, _uniqueName, _context);
	}
	else if (isNonRecursiveStruct(_type))
		var = make_shared<SymbolicStructVariable>(type, _uniqueName, _context);
	else
		solAssert(false, "");
	return make_pair(abstract, var);
}

bool isInteger(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Integer;
}

bool isFixedPoint(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::FixedPoint;
}

bool isRational(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::RationalNumber;
}

bool isFixedBytes(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::FixedBytes;
}

bool isAddress(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Address;
}

bool isContract(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Contract;
}

bool isEnum(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Enum;
}

bool isNumber(frontend::Type const& _type)
{
	return isInteger(_type) ||
		isFixedPoint(_type) ||
		isRational(_type) ||
		isFixedBytes(_type) ||
		isAddress(_type) ||
		isContract(_type) ||
		isEnum(_type);
}

bool isBool(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Bool;
}

bool isFunction(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Function;
}

bool isMapping(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Mapping;
}

bool isArray(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Array ||
		_type.category() == frontend::Type::Category::StringLiteral ||
		_type.category() == frontend::Type::Category::ArraySlice;
}

bool isTuple(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::Tuple;
}

bool isStringLiteral(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::StringLiteral;
}

bool isNonRecursiveStruct(frontend::Type const& _type)
{
	auto structType = dynamic_cast<StructType const*>(&_type);
	return structType && !structType->recursive();
}

bool isInaccessibleDynamic(frontend::Type const& _type)
{
	return _type.category() == frontend::Type::Category::InaccessibleDynamic;
}

smtutil::Expression minValue(frontend::IntegerType const& _type)
{
	return smtutil::Expression(_type.minValue());
}

smtutil::Expression minValue(frontend::Type const* _type)
{
	solAssert(isNumber(*_type), "");
	if (auto const* intType = dynamic_cast<IntegerType const*>(_type))
		return intType->minValue();
	if (auto const* fixedType = dynamic_cast<FixedPointType const*>(_type))
		return fixedType->minIntegerValue();
	if (
		dynamic_cast<AddressType const*>(_type) ||
		dynamic_cast<ContractType const*>(_type) ||
		dynamic_cast<EnumType const*>(_type) ||
		dynamic_cast<FixedBytesType const*>(_type)
	)
		return 0;
	solAssert(false, "");
}

smtutil::Expression maxValue(frontend::IntegerType const& _type)
{
	return smtutil::Expression(_type.maxValue());
}

smtutil::Expression maxValue(frontend::Type const* _type)
{
	solAssert(isNumber(*_type), "");
	if (auto const* intType = dynamic_cast<IntegerType const*>(_type))
		return intType->maxValue();
	if (auto const* fixedType = dynamic_cast<FixedPointType const*>(_type))
		return fixedType->maxIntegerValue();
	if (
		dynamic_cast<AddressType const*>(_type) ||
		dynamic_cast<ContractType const*>(_type)
	)
		return TypeProvider::uint(160)->maxValue();
	if (auto const* enumType = dynamic_cast<EnumType const*>(_type))
		return enumType->numberOfMembers() - 1;
	if (auto const* bytesType = dynamic_cast<FixedBytesType const*>(_type))
		return TypeProvider::uint(bytesType->numBytes() * 8)->maxValue();
	solAssert(false, "");
}

void setSymbolicZeroValue(SymbolicVariable const& _variable, EncodingContext& _context)
{
	setSymbolicZeroValue(_variable.currentValue(), _variable.type(), _context);
}

void setSymbolicZeroValue(smtutil::Expression _expr, frontend::Type const* _type, EncodingContext& _context)
{
	solAssert(_type, "");
	_context.addAssertion(_expr == zeroValue(_type));
}

smtutil::Expression zeroValue(frontend::Type const* _type)
{
	solAssert(_type, "");
	if (isSupportedType(*_type))
	{
		if (isNumber(*_type))
			return 0;
		if (isBool(*_type))
			return smtutil::Expression(false);
		if (isArray(*_type) || isMapping(*_type))
		{
			auto tupleSort = dynamic_pointer_cast<TupleSort>(smtSort(*_type));
			solAssert(tupleSort, "");
			auto sortSort = make_shared<SortSort>(tupleSort->components.front());

			std::optional<smtutil::Expression> zeroArray;
			auto length = bigint(0);
			if (auto arrayType = dynamic_cast<ArrayType const*>(_type))
			{
				zeroArray = smtutil::Expression::const_array(smtutil::Expression(sortSort), zeroValue(arrayType->baseType()));
				if (!arrayType->isDynamicallySized())
					length = bigint(arrayType->length());
			}
			else if (auto mappingType = dynamic_cast<MappingType const*>(_type))
				zeroArray = smtutil::Expression::const_array(smtutil::Expression(sortSort), zeroValue(mappingType->valueType()));
			else
				solAssert(false, "");

			solAssert(zeroArray, "");
			return smtutil::Expression::tuple_constructor(
				smtutil::Expression(std::make_shared<SortSort>(tupleSort), tupleSort->name),
				vector<smtutil::Expression>{*zeroArray, length}
			);

		}
		if (isNonRecursiveStruct(*_type))
		{
			auto const* structType = dynamic_cast<StructType const*>(_type);
			auto structSort = dynamic_pointer_cast<TupleSort>(smtSort(*_type));
			return smtutil::Expression::tuple_constructor(
				smtutil::Expression(make_shared<SortSort>(structSort), structSort->name),
				applyMap(
					structType->structDefinition().members(),
					[](auto var) { return zeroValue(var->type()); }
				)
			);
		}
		solAssert(false, "");
	}
	// Unsupported types are abstracted as Int.
	return 0;
}

bool isSigned(frontend::Type const* _type)
{
	solAssert(smt::isNumber(*_type), "");
	bool isSigned = false;
	if (auto const* numberType = dynamic_cast<RationalNumberType const*>(_type))
		isSigned |= numberType->isNegative();
	else if (auto const* intType = dynamic_cast<IntegerType const*>(_type))
		isSigned |= intType->isSigned();
	else if (auto const* fixedType = dynamic_cast<FixedPointType const*>(_type))
		isSigned |= fixedType->isSigned();
	else if (
		dynamic_cast<AddressType const*>(_type) ||
		dynamic_cast<ContractType const*>(_type) ||
		dynamic_cast<EnumType const*>(_type) ||
		dynamic_cast<FixedBytesType const*>(_type)
	)
		return false;
	else
		solAssert(false, "");

	return isSigned;
}

pair<unsigned, bool> typeBvSizeAndSignedness(frontend::Type const* _type)
{
	if (auto const* intType = dynamic_cast<IntegerType const*>(_type))
		return {intType->numBits(), intType->isSigned()};
	else if (auto const* fixedType = dynamic_cast<FixedPointType const*>(_type))
		return {fixedType->numBits(), fixedType->isSigned()};
	else if (auto const* fixedBytesType = dynamic_cast<FixedBytesType const*>(_type))
		return {fixedBytesType->numBytes() * 8, false};
	else
		solAssert(false, "");
}

void setSymbolicUnknownValue(SymbolicVariable const& _variable, EncodingContext& _context)
{
	setSymbolicUnknownValue(_variable.currentValue(), _variable.type(), _context);
}

void setSymbolicUnknownValue(smtutil::Expression _expr, frontend::Type const* _type, EncodingContext& _context)
{
	_context.addAssertion(symbolicUnknownConstraints(_expr, _type));
}

smtutil::Expression symbolicUnknownConstraints(smtutil::Expression _expr, frontend::Type const* _type)
{
	solAssert(_type, "");
	if (isEnum(*_type) || isInteger(*_type) || isAddress(*_type) || isFixedBytes(*_type))
		return _expr >= minValue(_type) && _expr <= maxValue(_type);
	else if (
		auto arrayType = dynamic_cast<ArrayType const*>(_type);
		arrayType && !arrayType->isDynamicallySized()
	)
		return smtutil::Expression::tuple_get(_expr, 1) == arrayType->length();
	else if (isArray(*_type) || isMapping(*_type))
		/// Length cannot be negative.
		return smtutil::Expression::tuple_get(_expr, 1) >= 0;

	return smtutil::Expression(true);
}

optional<smtutil::Expression> symbolicTypeConversion(frontend::Type const* _from, frontend::Type const* _to)
{
	if (_to && _from)
		// StringLiterals are encoded as SMT arrays in the generic case,
		// but they can also be compared/assigned to fixed bytes, in which
		// case they'd need to be encoded as numbers.
		if (auto strType = dynamic_cast<StringLiteralType const*>(_from))
			if (auto fixedBytesType = dynamic_cast<FixedBytesType const*>(_to))
			{
				if (strType->value().empty())
					return smtutil::Expression(size_t(0));
				auto bytesVec = util::asBytes(strType->value());
				bytesVec.resize(fixedBytesType->numBytes(), 0);
				return smtutil::Expression(u256(util::toHex(bytesVec, util::HexPrefix::Add)));
			}

	return std::nullopt;
}

smtutil::Expression member(smtutil::Expression const& _tuple, string const& _member)
{
	TupleSort const& _sort = dynamic_cast<TupleSort const&>(*_tuple.sort);
	return smtutil::Expression::tuple_get(
		_tuple,
		_sort.memberToIndex.at(_member)
	);
}

smtutil::Expression assignMember(smtutil::Expression const _tuple, map<string, smtutil::Expression> const& _values)
{
	TupleSort const& _sort = dynamic_cast<TupleSort const&>(*_tuple.sort);
	vector<smtutil::Expression> args;
	for (auto const& m: _sort.members)
		if (auto* value = util::valueOrNullptr(_values, m))
			args.emplace_back(*value);
		else
			args.emplace_back(member(_tuple, m));
	auto sortExpr = smtutil::Expression(make_shared<smtutil::SortSort>(_tuple.sort), _tuple.name);
	return smtutil::Expression::tuple_constructor(sortExpr, args);
}

}
