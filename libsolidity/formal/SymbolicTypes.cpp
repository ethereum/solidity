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
	return isInteger(_category) ||
		isAddress(_category) ||
		isBool(_category);
}

shared_ptr<SymbolicVariable> dev::solidity::newSymbolicVariable(
	Type const& _type,
	std::string const& _uniqueName,
	smt::SolverInterface& _solver
)
{
	if (!isSupportedType(_type))
		return nullptr;
	if (isBool(_type.category()))
		return make_shared<SymbolicBoolVariable>(_type, _uniqueName, _solver);
	else if (isInteger(_type.category()))
		return make_shared<SymbolicIntVariable>(_type, _uniqueName, _solver);
	else if (isAddress(_type.category()))
		return make_shared<SymbolicAddressVariable>(_type, _uniqueName, _solver);
	else
		solAssert(false, "");
}

bool dev::solidity::isSupportedType(Type const& _type)
{
	return isSupportedType(_type.category());
}

bool dev::solidity::isInteger(Type::Category _category)
{
	return _category == Type::Category::Integer;
}

bool dev::solidity::isInteger(Type const& _type)
{
	return isInteger(_type.category());
}

bool dev::solidity::isAddress(Type::Category _category)
{
	return _category == Type::Category::Address;
}

bool dev::solidity::isAddress(Type const& _type)
{
	return isAddress(_type.category());
}

bool dev::solidity::isNumber(Type::Category _category)
{
	return isInteger(_category) ||
		isAddress(_category);
}

bool dev::solidity::isNumber(Type const& _type)
{
	return isNumber(_type.category());
}

bool dev::solidity::isBool(Type::Category _category)
{
	return _category == Type::Category::Bool;
}

bool dev::solidity::isBool(Type const& _type)
{
	return isBool(_type.category());
}

smt::Expression dev::solidity::minValue(IntegerType const& _type)
{
	return smt::Expression(_type.minValue());
}

smt::Expression dev::solidity::maxValue(IntegerType const& _type)
{
	return smt::Expression(_type.maxValue());
}
