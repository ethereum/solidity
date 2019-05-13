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
namespace smt
{

/// Returns the SMT sort that models the Solidity type _type.
SortPointer smtSort(solidity::Type const& _type);
std::vector<SortPointer> smtSort(std::vector<solidity::TypePointer> const& _types);
/// Returns the SMT kind that models the Solidity type type category _category.
Kind smtKind(solidity::Type::Category _category);

/// Returns true if type is fully supported (declaration and operations).
bool isSupportedType(solidity::Type::Category _category);
bool isSupportedType(solidity::Type const& _type);
/// Returns true if type is partially supported (declaration).
bool isSupportedTypeDeclaration(solidity::Type::Category _category);
bool isSupportedTypeDeclaration(solidity::Type const& _type);

bool isInteger(solidity::Type::Category _category);
bool isRational(solidity::Type::Category _category);
bool isFixedBytes(solidity::Type::Category _category);
bool isAddress(solidity::Type::Category _category);
bool isContract(solidity::Type::Category _category);
bool isEnum(solidity::Type::Category _category);
bool isNumber(solidity::Type::Category _category);
bool isBool(solidity::Type::Category _category);
bool isFunction(solidity::Type::Category _category);
bool isMapping(solidity::Type::Category _category);
bool isArray(solidity::Type::Category _category);
bool isTuple(solidity::Type::Category _category);

/// Returns a new symbolic variable, according to _type.
/// Also returns whether the type is abstract or not,
/// which is true for unsupported types.
std::pair<bool, std::shared_ptr<SymbolicVariable>> newSymbolicVariable(solidity::Type const& _type, std::string const& _uniqueName, SolverInterface& _solver);

Expression minValue(solidity::IntegerType const& _type);
Expression maxValue(solidity::IntegerType const& _type);

void setSymbolicZeroValue(SymbolicVariable const& _variable, SolverInterface& _interface);
void setSymbolicZeroValue(Expression _expr, solidity::TypePointer const& _type, SolverInterface& _interface);
void setSymbolicUnknownValue(SymbolicVariable const& _variable, SolverInterface& _interface);
void setSymbolicUnknownValue(Expression _expr, solidity::TypePointer const& _type, SolverInterface& _interface);

}
}
}
