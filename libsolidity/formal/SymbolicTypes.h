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

#pragma once

#include <libsolidity/formal/SymbolicVariables.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

namespace solidity::frontend::smt
{

class EncodingContext;

/// Returns the SMT sort that models the Solidity type _type.
smtutil::SortPointer smtSort(frontend::Type const& _type);
std::vector<smtutil::SortPointer> smtSort(std::vector<frontend::Type const*> const& _types);
/// If _type has type Function, abstract it to Integer.
/// Otherwise return smtSort(_type).
smtutil::SortPointer smtSortAbstractFunction(frontend::Type const& _type);
std::vector<smtutil::SortPointer> smtSortAbstractFunction(std::vector<frontend::Type const*> const& _types);
/// Returns the SMT kind that models the Solidity type type category _category.
smtutil::Kind smtKind(frontend::Type const& _type);

/// Returns true if type is fully supported (declaration and operations).
bool isSupportedType(frontend::Type const& _type);
bool isSupportedType(frontend::Type const& _type);
/// Returns true if type is partially supported (declaration).
bool isSupportedTypeDeclaration(frontend::Type const& _type);
bool isSupportedTypeDeclaration(frontend::Type const& _type);

bool isInteger(frontend::Type const& _type);
bool isFixedPoint(frontend::Type const& _type);
bool isRational(frontend::Type const& _type);
bool isFixedBytes(frontend::Type const& _type);
bool isAddress(frontend::Type const& _type);
bool isContract(frontend::Type const& _type);
bool isEnum(frontend::Type const& _type);
bool isNumber(frontend::Type const& _type);
bool isBool(frontend::Type const& _type);
bool isFunction(frontend::Type const& _type);
bool isMapping(frontend::Type const& _type);
bool isArray(frontend::Type const& _type);
bool isTuple(frontend::Type const& _type);
bool isStringLiteral(frontend::Type const& _type);
bool isNonRecursiveStruct(frontend::Type const& _type);
bool isInaccessibleDynamic(frontend::Type const& _type);

/// Returns a new symbolic variable, according to _type.
/// Also returns whether the type is abstract or not,
/// which is true for unsupported types.
std::pair<bool, std::shared_ptr<SymbolicVariable>> newSymbolicVariable(frontend::Type const& _type, std::string const& _uniqueName, EncodingContext& _context);

smtutil::Expression minValue(frontend::IntegerType const& _type);
smtutil::Expression minValue(frontend::Type const* _type);
smtutil::Expression maxValue(frontend::IntegerType const& _type);
smtutil::Expression maxValue(frontend::Type const* _type);
smtutil::Expression zeroValue(frontend::Type const* _type);
bool isSigned(frontend::Type const* _type);

std::pair<unsigned, bool> typeBvSizeAndSignedness(frontend::Type const* type);

void setSymbolicZeroValue(SymbolicVariable const& _variable, EncodingContext& _context);
void setSymbolicZeroValue(smtutil::Expression _expr, frontend::Type const* _type, EncodingContext& _context);
void setSymbolicUnknownValue(SymbolicVariable const& _variable, EncodingContext& _context);
void setSymbolicUnknownValue(smtutil::Expression _expr, frontend::Type const* _type, EncodingContext& _context);
smtutil::Expression symbolicUnknownConstraints(smtutil::Expression _expr, frontend::Type const* _type);

std::optional<smtutil::Expression> symbolicTypeConversion(frontend::Type const* _from, frontend::Type const* _to);

smtutil::Expression member(smtutil::Expression const& _tuple, std::string const& _member);
smtutil::Expression assignMember(smtutil::Expression const _tuple, std::map<std::string, smtutil::Expression> const& _values);

}
