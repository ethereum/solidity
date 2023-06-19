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


#include <libsolidity/ast/experimental/TypeSystem.h>
#include <liblangutil/Exceptions.h>

using namespace std;
using namespace solidity::frontend;
using namespace solidity::frontend::experimental;

void TypeEnvironment::assignType(TypeSystem& _typeSystem, Declaration const* _declaration, Type _typeAssignment)
{
	auto&& [type, newlyInserted] = m_types.emplace(std::piecewise_construct, std::forward_as_tuple(_declaration), std::forward_as_tuple(std::move(_typeAssignment)));
	if (!newlyInserted)
	{
		unify(_typeSystem, type->second, _typeAssignment);
	}
}
Type TypeEnvironment::lookup(TypeSystem& _typeSystem, Declaration const* _declaration)
{
	if (m_types.count(_declaration))
		return m_types[_declaration];
	Type result = _typeSystem.freshTypeVariable();
	m_types.emplace(std::piecewise_construct, std::forward_as_tuple(_declaration), std::forward_as_tuple(result));
	return result;
}
Type TypeEnvironment::freshFreeType()
{
	return FreeType{m_numFreeTypes++};
}


void TypeEnvironment::unify(TypeSystem& _context, Type _a, Type _b)
{
	_a = _context.resolve(_a);
	_b = _context.resolve(_b);
	if (auto* varA = get_if<TypeVariable>(&_a))
		_context.instantiate(*varA, _b);
	else if (holds_alternative<WordType>(_a))
	{
		if (holds_alternative<WordType>(_b))
			return;
		else
			solAssert(false, "unification failed");
	}

	solAssert(false, fmt::format("cannot unify {} and {}", typeToString(_a), typeToString(_b)));
}