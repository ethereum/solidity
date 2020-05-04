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

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/Types.h>
#include <libsolutil/CommonData.h>
#include <memory>

using namespace std;

namespace solidity::frontend::smt
{

SortPointer smtSort(frontend::Type const& _type)
{
	switch (smtKind(_type.category()))
	{
	case Kind::Int:
		return SortProvider::intSort;
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
			returnSort = SortProvider::intSort;
		else
			returnSort = smtSort(*returnTypes.front());
		return make_shared<FunctionSort>(parameterSorts, returnSort);
	}
	case Kind::Array:
	{
		if (isMapping(_type.category()))
		{
			auto mapType = dynamic_cast<frontend::MappingType const*>(&_type);
			solAssert(mapType, "");
			return make_shared<ArraySort>(smtSortAbstractFunction(*mapType->keyType()), smtSortAbstractFunction(*mapType->valueType()));
		}
		else if (isStringLiteral(_type.category()))
		{
			auto stringLitType = dynamic_cast<frontend::StringLiteralType const*>(&_type);
			solAssert(stringLitType, "");
			return make_shared<ArraySort>(SortProvider::intSort, SortProvider::intSort);
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
			return make_shared<ArraySort>(SortProvider::intSort, smtSortAbstractFunction(*arrayType->baseType()));
		}
	}
	case Kind::Tuple:
	{
		auto tupleType = dynamic_cast<frontend::TupleType const*>(&_type);
		solAssert(tupleType, "");
		vector<string> members;
		auto const& tupleName = _type.identifier();
		auto const& components = tupleType->components();
		for (unsigned i = 0; i < components.size(); ++i)
			members.emplace_back(tupleName + "_accessor_" + to_string(i));
		return make_shared<TupleSort>(
			tupleName,
			members,
			smtSortAbstractFunction(tupleType->components())
		);
	}
	default:
		// Abstract case.
		return SortProvider::intSort;
	}
}

vector<SortPointer> smtSort(vector<frontend::TypePointer> const& _types)
{
	vector<SortPointer> sorts;
	for (auto const& type: _types)
		sorts.push_back(smtSort(*type));
	return sorts;
}

SortPointer smtSortAbstractFunction(frontend::Type const& _type)
{
	if (isFunction(_type.category()))
		return SortProvider::intSort;
	return smtSort(_type);
}

vector<SortPointer> smtSortAbstractFunction(vector<frontend::TypePointer> const& _types)
{
	vector<SortPointer> sorts;
	for (auto const& type: _types)
		if (type)
			sorts.push_back(smtSortAbstractFunction(*type));
		else
			sorts.push_back(SortProvider::intSort);
	return sorts;
}

Kind smtKind(frontend::Type::Category _category)
{
	if (isNumber(_category))
		return Kind::Int;
	else if (isBool(_category))
		return Kind::Bool;
	else if (isFunction(_category))
		return Kind::Function;
	else if (isMapping(_category) || isArray(_category))
		return Kind::Array;
	else if (isTuple(_category))
		return Kind::Tuple;
	// Abstract case.
	return Kind::Int;
}

bool isSupportedType(frontend::Type::Category _category)
{
	return isNumber(_category) ||
		isBool(_category) ||
		isMapping(_category) ||
		isArray(_category) ||
		isTuple(_category);
}

bool isSupportedTypeDeclaration(frontend::Type::Category _category)
{
	return isSupportedType(_category) ||
		isFunction(_category);
}

pair<bool, shared_ptr<SymbolicVariable>> newSymbolicVariable(
	frontend::Type const& _type,
	std::string const& _uniqueName,
	EncodingContext& _context
)
{
	bool abstract = false;
	shared_ptr<SymbolicVariable> var;
	frontend::TypePointer type = &_type;
	if (!isSupportedTypeDeclaration(_type))
	{
		abstract = true;
		var = make_shared<SymbolicIntVariable>(frontend::TypeProvider::uint256(), type, _uniqueName, _context);
	}
	else if (isBool(_type.category()))
		var = make_shared<SymbolicBoolVariable>(type, _uniqueName, _context);
	else if (isFunction(_type.category()))
	{
		auto const& fType = dynamic_cast<FunctionType const*>(type);
		auto const& paramsIn = fType->parameterTypes();
		auto const& paramsOut = fType->returnParameterTypes();
		auto findFunctionParam = [&](auto&& params) {
			return find_if(
				begin(params),
				end(params),
				[&](TypePointer _paramType) { return _paramType->category() == frontend::Type::Category::Function; }
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
	else if (isInteger(_type.category()))
		var = make_shared<SymbolicIntVariable>(type, type, _uniqueName, _context);
	else if (isFixedBytes(_type.category()))
	{
		auto fixedBytesType = dynamic_cast<frontend::FixedBytesType const*>(type);
		solAssert(fixedBytesType, "");
		var = make_shared<SymbolicFixedBytesVariable>(type, fixedBytesType->numBytes(), _uniqueName, _context);
	}
	else if (isAddress(_type.category()) || isContract(_type.category()))
		var = make_shared<SymbolicAddressVariable>(_uniqueName, _context);
	else if (isEnum(_type.category()))
		var = make_shared<SymbolicEnumVariable>(type, _uniqueName, _context);
	else if (isRational(_type.category()))
	{
		auto rational = dynamic_cast<frontend::RationalNumberType const*>(&_type);
		solAssert(rational, "");
		if (rational->isFractional())
			var = make_shared<SymbolicIntVariable>(frontend::TypeProvider::uint256(), type, _uniqueName, _context);
		else
			var = make_shared<SymbolicIntVariable>(type, type, _uniqueName, _context);
	}
	else if (isMapping(_type.category()))
		var = make_shared<SymbolicMappingVariable>(type, _uniqueName, _context);
	else if (isArray(_type.category()))
		var = make_shared<SymbolicArrayVariable>(type, type, _uniqueName, _context);
	else if (isTuple(_type.category()))
		var = make_shared<SymbolicTupleVariable>(type, _uniqueName, _context);
	else if (isStringLiteral(_type.category()))
	{
		auto stringType = TypeProvider::stringMemory();
		var = make_shared<SymbolicArrayVariable>(stringType, type, _uniqueName, _context);
	}
	else
		solAssert(false, "");
	return make_pair(abstract, var);
}

bool isSupportedType(frontend::Type const& _type)
{
	return isSupportedType(_type.category());
}

bool isSupportedTypeDeclaration(frontend::Type const& _type)
{
	return isSupportedTypeDeclaration(_type.category());
}

bool isInteger(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Integer;
}

bool isRational(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::RationalNumber;
}

bool isFixedBytes(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::FixedBytes;
}

bool isAddress(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Address;
}

bool isContract(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Contract;
}

bool isEnum(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Enum;
}

bool isNumber(frontend::Type::Category _category)
{
	return isInteger(_category) ||
		isRational(_category) ||
		isFixedBytes(_category) ||
		isAddress(_category) ||
		isContract(_category) ||
		isEnum(_category);
}

bool isBool(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Bool;
}

bool isFunction(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Function;
}

bool isMapping(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Mapping;
}

bool isArray(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Array ||
		_category == frontend::Type::Category::StringLiteral ||
		_category == frontend::Type::Category::ArraySlice;
}

bool isTuple(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::Tuple;
}

bool isStringLiteral(frontend::Type::Category _category)
{
	return _category == frontend::Type::Category::StringLiteral;
}

Expression minValue(frontend::IntegerType const& _type)
{
	return Expression(_type.minValue());
}

Expression maxValue(frontend::IntegerType const& _type)
{
	return Expression(_type.maxValue());
}

void setSymbolicZeroValue(SymbolicVariable const& _variable, EncodingContext& _context)
{
	setSymbolicZeroValue(_variable.currentValue(), _variable.type(), _context);
}

void setSymbolicZeroValue(Expression _expr, frontend::TypePointer const& _type, EncodingContext& _context)
{
	solAssert(_type, "");
	_context.addAssertion(_expr == zeroValue(_type));
}

Expression zeroValue(frontend::TypePointer const& _type)
{
	solAssert(_type, "");
	if (isSupportedType(_type->category()))
	{
		if (isNumber(_type->category()))
			return 0;
		if (isBool(_type->category()))
			return Expression(false);
		if (isArray(_type->category()) || isMapping(_type->category()))
		{
			if (auto arrayType = dynamic_cast<ArrayType const*>(_type))
				return Expression::const_array(Expression(arrayType), zeroValue(arrayType->baseType()));
			auto mappingType = dynamic_cast<MappingType const*>(_type);
			solAssert(mappingType, "");
			return Expression::const_array(Expression(mappingType), zeroValue(mappingType->valueType()));
		}
		solAssert(false, "");
	}
	// Unsupported types are abstracted as Int.
	return 0;
}

void setSymbolicUnknownValue(SymbolicVariable const& _variable, EncodingContext& _context)
{
	setSymbolicUnknownValue(_variable.currentValue(), _variable.type(), _context);
}

void setSymbolicUnknownValue(Expression _expr, frontend::TypePointer const& _type, EncodingContext& _context)
{
	solAssert(_type, "");
	if (isEnum(_type->category()))
	{
		auto enumType = dynamic_cast<frontend::EnumType const*>(_type);
		solAssert(enumType, "");
		_context.addAssertion(_expr >= 0);
		_context.addAssertion(_expr < enumType->numberOfMembers());
	}
	else if (isInteger(_type->category()))
	{
		auto intType = dynamic_cast<frontend::IntegerType const*>(_type);
		solAssert(intType, "");
		_context.addAssertion(_expr >= minValue(*intType));
		_context.addAssertion(_expr <= maxValue(*intType));
	}
}

optional<Expression> symbolicTypeConversion(TypePointer _from, TypePointer _to)
{
	if (_to && _from)
		// StringLiterals are encoded as SMT arrays in the generic case,
		// but they can also be compared/assigned to fixed bytes, in which
		// case they'd need to be encoded as numbers.
		if (auto strType = dynamic_cast<StringLiteralType const*>(_from))
			if (_to->category() == frontend::Type::Category::FixedBytes)
				return smt::Expression(u256(toHex(util::asBytes(strType->value()), util::HexPrefix::Add)));

	return std::nullopt;
}

}
