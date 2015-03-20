/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file ExpressionClasses.h
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Container for equivalence classes of expressions for use in common subexpression elimination.
 */

#pragma once

#include <vector>
#include <set>
#include <memory>

namespace dev
{
namespace eth
{

class AssemblyItem;

/**
 * Collection of classes of equivalent expressions that can also determine the class of an expression.
 * Identifiers are contiguously assigned to new classes starting from zero.
 */
class ExpressionClasses
{
public:
	using Id = unsigned;
	using Ids = std::vector<Id>;

	struct Expression
	{
		Id id;
		AssemblyItem const* item;
		Ids arguments;
	};

	/// Retrieves the id of the expression equivalence class resulting from the given item applied to the
	/// given classes, might also create a new one.
	Id find(AssemblyItem const& _item, Ids const& _arguments = {});
	/// @returns the canonical representative of an expression class.
	Expression const& representative(Id _id) const { return m_representatives.at(_id); }
	/// @returns the number of classes.
	Id size() const { return m_representatives.size(); }

private:

	/// Expression equivalence class representatives - we only store one item of an equivalence.
	std::vector<Expression> m_representatives;
	std::vector<std::shared_ptr<AssemblyItem>> m_spareAssemblyItem;
};

}
}
