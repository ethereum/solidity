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

#include <libsolidity/formal/EncodingContext.h>
#include <libsolidity/formal/SymbolicVariables.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

namespace solidity::frontend::smt
{

/// Returns the SMT sort that models the Solidity type _type.
SortPointer smtSort(frontend::Type const& _type);
std::vector<SortPointer> smtSort(std::vector<frontend::TypePointer> const& _types);
/// If _type has type Function, abstract it to Integer.
/// Otherwise return smtSort(_type).
SortPointer smtSortAbstractFunction(frontend::Type const& _type);
/// Returns the SMT kind that models the Solidity type type category _category.
Kind smtKind(frontend::Type::Category _category);

/// Returns true if type is fully supported (declaration and operations).
bool isSupportedType(frontend::Type::Category _category);
bool isSupportedType(frontend::Type const& _type);
/// Returns true if type is partially supported (declaration).
bool isSupportedTypeDeclaration(frontend::Type::Category _category);
bool isSupportedTypeDeclaration(frontend::Type const& _type);

bool isInteger(frontend::Type::Category _category);
bool isRational(frontend::Type::Category _category);
bool isFixedBytes(frontend::Type::Category _category);
bool isAddress(frontend::Type::Category _category);
bool isContract(frontend::Type::Category _category);
bool isEnum(frontend::Type::Category _category);
bool isNumber(frontend::Type::Category _category);
bool isBool(frontend::Type::Category _category);
bool isFunction(frontend::Type::Category _category);
bool isMapping(frontend::Type::Category _category);
bool isArray(frontend::Type::Category _category);
bool isTuple(frontend::Type::Category _category);
bool isStringLiteral(frontend::Type::Category _category);

/// Returns a new symbolic variable, according to _type.
/// Also returns whether the type is abstract or not,
/// which is true for unsupported types.
std::pair<bool, std::shared_ptr<SymbolicVariable>> newSymbolicVariable(frontend::Type const& _type, std::string const& _uniqueName, EncodingContext& _context);

Expression minValue(frontend::IntegerType const& _type);
Expression maxValue(frontend::IntegerType const& _type);
Expression zeroValue(frontend::TypePointer const& _type);

void setSymbolicZeroValue(SymbolicVariable const& _variable, EncodingContext& _context);
void setSymbolicZeroValue(Expression _expr, frontend::TypePointer const& _type, EncodingContext& _context);
void setSymbolicUnknownValue(SymbolicVariable const& _variable, EncodingContext& _context);
void setSymbolicUnknownValue(Expression _expr, frontend::TypePointer const& _type, EncodingContext& _context);

}
