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
/**
 * Class that can answer questions about values of variables and their relations.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulString.h>

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>

#include <map>
#include <functional>

namespace solidity::yul
{

struct Dialect;
struct AssignedValue;

/**
 * Class that can answer questions about values of variables and their relations.
 *
 * Requires a callback that returns the current value of the variable.
 * The value can change any time during the lifetime of the KnowledgeBase,
 * it will update its internal data structure accordingly.
 *
 * This means that the code the KnowledgeBase is used on does not need to be in SSA
 * form.
 * The only requirement is that the assigned values are movable expressions.
 *
 * There is a constructor to provide all SSA values right at the beginning.
 * If you use this, the KnowledgeBase will be slightly more efficient.
 *
 * Internally, tries to find groups of variables that have a mutual constant
 * difference and stores these differences always relative to a specific
 * representative variable of the group.
 *
 * There is a special group which is the constant values. Those use the
 * empty YulString as representative "variable".
 */
class KnowledgeBase
{
public:
	/// Constructor for arbitrary value callback that allows for variable values
	/// to change in between calls to functions of this class.
	KnowledgeBase(std::function<AssignedValue const*(YulString)> _variableValues):
		m_variableValues(std::move(_variableValues))
	{}
	/// Constructor to use if source code is in SSA form and values are constant.
	KnowledgeBase(std::map<YulString, AssignedValue> const& _ssaValues);

	bool knownToBeDifferent(YulString _a, YulString _b);
	std::optional<u256> differenceIfKnownConstant(YulString _a, YulString _b);
	bool knownToBeDifferentByAtLeast32(YulString _a, YulString _b);
	bool knownToBeZero(YulString _a);
	std::optional<u256> valueIfKnownConstant(YulString _a);
	std::optional<u256> valueIfKnownConstant(Expression const& _expression);

private:
	/**
	 * Constant offset relative to a reference variable, or absolute constant if the
	 * reference variable is the empty YulString.
	 */
	struct VariableOffset
	{
		YulString reference;
		u256 offset;

		bool isAbsolute() const
		{
			return reference.empty();
		}

		std::optional<u256> absoluteValue() const
		{
			if (isAbsolute())
				return offset;
			else
				return std::nullopt;
		}
	};

	VariableOffset explore(YulString _var);
	std::optional<VariableOffset> explore(Expression const& _value);

	/// Retrieves the current value of a variable and potentially resets the variable if it is not up to date.
	Expression const* valueOf(YulString _var);

	/// Resets all information about the variable and removes it from its group,
	/// potentially finding a new representative.
	void reset(YulString _var);

	VariableOffset setOffset(YulString _variable, VariableOffset _value);

	/// If true, we can assume that variable values never change and skip some steps.
	bool m_valuesAreSSA = false;
	/// Callback to retrieve the current value of a variable.
	std::function<AssignedValue const*(YulString)> m_variableValues;

	/// Offsets for each variable to one representative per group.
	/// The empty string is the representative of the constant value zero.
	std::map<YulString, VariableOffset> m_offsets;
	/// Last known value of each variable we queried.
	std::map<YulString, Expression const*> m_lastKnownValue;
	/// For each representative, variables that use it to offset from.
	std::map<YulString, std::set<YulString>> m_groupMembers;
};

}
