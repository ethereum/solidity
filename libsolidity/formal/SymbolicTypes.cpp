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

#include <libsolidity/formal/SymbolicBoolVariable.h>
#include <libsolidity/formal/SymbolicIntVariable.h>
#include <libsolidity/formal/SymbolicAddressVariable.h>

#include <libsolidity/ast/Types.h>

#include <memory>

using namespace std;
using namespace dev::solidity;

bool dev::solidity::isSupportedType(Type::Category _category)
{
	return isNumber(_category) ||
		isBool(_category) ||
		isFunction(_category);
}

pair<bool, shared_ptr<SymbolicVariable>> dev::solidity::newSymbolicVariable(
	Type const& _type,
	std::string const& _uniqueName,
	smt::SolverInterface& _solver
)
{
	bool abstract = false;
	shared_ptr<SymbolicVariable> var;
	if (!isSupportedType(_type))
	{
		abstract = true;
		var = make_shared<SymbolicIntVariable>(IntegerType{256}, _uniqueName, _solver);
	}
	else if (isBool(_type.category()))
		var = make_shared<SymbolicBoolVariable>(_type, _uniqueName, _solver);
	else if (isFunction(_type.category()))
		var = make_shared<SymbolicIntVariable>(IntegerType{256}, _uniqueName, _solver);
	else if (isInteger(_type.category()))
		var = make_shared<SymbolicIntVariable>(_type, _uniqueName, _solver);
	else if (isAddress(_type.category()))
		var = make_shared<SymbolicAddressVariable>(_type, _uniqueName, _solver);
	else if (isRational(_type.category()))
	{
		auto rational = dynamic_cast<RationalNumberType const*>(&_type);
		solAssert(rational, "");
		if (rational->isFractional())
			var = make_shared<SymbolicIntVariable>(IntegerType{256}, _uniqueName, _solver);
		else
			var = make_shared<SymbolicIntVariable>(_type, _uniqueName, _solver);
	}
	else
		solAssert(false, "");
	return make_pair(abstract, var);
}

bool dev::solidity::isSupportedType(Type const& _type)
{
	return isSupportedType(_type.category());
}

bool dev::solidity::isInteger(Type::Category _category)
{
	return _category == Type::Category::Integer;
}

bool dev::solidity::isRational(Type::Category _category)
{
	return _category == Type::Category::RationalNumber;
}

bool dev::solidity::isAddress(Type::Category _category)
{
	return _category == Type::Category::Address;
}

bool dev::solidity::isNumber(Type::Category _category)
{
	return isInteger(_category) ||
		isRational(_category) ||
		isAddress(_category);
}

bool dev::solidity::isBool(Type::Category _category)
{
	return _category == Type::Category::Bool;
}

bool dev::solidity::isFunction(Type::Category _category)
{
	return _category == Type::Category::Function;
}

smt::Expression dev::solidity::minValue(IntegerType const& _type)
{
	return smt::Expression(_type.minValue());
}

smt::Expression dev::solidity::maxValue(IntegerType const& _type)
{
	return smt::Expression(_type.maxValue());
}
