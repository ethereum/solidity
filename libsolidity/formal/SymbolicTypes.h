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

#pragma once

#include <libsolidity/formal/SolverInterface.h>
#include <libsolidity/formal/SymbolicVariables.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

namespace dev
{
namespace solidity
{

/// Returns the SMT sort that models the Solidity type _type.
smt::SortPointer smtSort(Type const& _type);
std::vector<smt::SortPointer> smtSort(std::vector<TypePointer> const& _types);
/// Returns the SMT kind that models the Solidity type type category _category.
smt::Kind smtKind(Type::Category _category);

/// Returns true if type is fully supported (declaration and operations).
bool isSupportedType(Type::Category _category);
bool isSupportedType(Type const& _type);
/// Returns true if type is partially supported (declaration).
bool isSupportedTypeDeclaration(Type::Category _category);
bool isSupportedTypeDeclaration(Type const& _type);

bool isInteger(Type::Category _category);
bool isRational(Type::Category _category);
bool isFixedBytes(Type::Category _category);
bool isAddress(Type::Category _category);
bool isEnum(Type::Category _category);
bool isNumber(Type::Category _category);
bool isBool(Type::Category _category);
bool isFunction(Type::Category _category);
bool isMapping(Type::Category _category);
bool isArray(Type::Category _category);

/// Returns a new symbolic variable, according to _type.
/// Also returns whether the type is abstract or not,
/// which is true for unsupported types.
std::pair<bool, std::shared_ptr<SymbolicVariable>> newSymbolicVariable(Type const& _type, std::string const& _uniqueName, smt::SolverInterface& _solver);

smt::Expression minValue(IntegerType const& _type);
smt::Expression maxValue(IntegerType const& _type);

namespace smt
{

void setSymbolicZeroValue(SymbolicVariable const& _variable, smt::SolverInterface& _interface);
void setSymbolicZeroValue(smt::Expression _expr, TypePointer const& _type, smt::SolverInterface& _interface);
void setSymbolicUnknownValue(SymbolicVariable const& _variable, smt::SolverInterface& _interface);
void setSymbolicUnknownValue(smt::Expression _expr, TypePointer const& _type, smt::SolverInterface& _interface);

}

}
}
