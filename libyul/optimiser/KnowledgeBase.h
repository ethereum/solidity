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
#include <libyul/Dialect.h>
#include <libyul/YulName.h>

#include <libsolutil/Common.h>
#include <libsolutil/Numeric.h>

#include <map>
#include <functional>

namespace solidity::yul
{

class Dialect;
struct AssignedValue;

/**
 * Class that can answer questions about values of variables and their relations.
 *
 * Requires a callback that returns the current value of the variable.
 * The value can change during the lifetime of the KnowledgeBase, so the code it is
 * used on does not need to be in SSA form. In that case, the caller has to ensure:
 *  - The values are movable expressions.
 *  - ``variableChanged`` is called whenever the value of a variable changes.
 *  - A value only references variables that have not changed since it was assigned.
 *    In particular, it does not reference the variable it is assigned to.
 *    Values are evaluated lazily. So if a referenced variable changes, the value
 *    has to be forgotten, otherwise it would be evaluated using the new value of
 *    the referenced variable. This also rules out cycles, which is needed
 *    for exploration to terminate.
 * The DataFlowAnalyzer ensures this in ``handleAssignment`` and ``clearValues``.
 *
 * There is a constructor to provide all SSA values right at the beginning.
 * If you use this, the KnowledgeBase will be slightly more efficient.
 *
 * Internally, tries to find groups of variables that have a mutual constant
 * difference and stores these differences always relative to a specific
 * representative variable of the group.
 * Variables are versioned, so these differences never have to be invalidated,
 * in particular, after a change, the variable is treated as a new one.
 *
 * There is a special group which is the constant values. Those use the
 * empty YulName as representative "variable".
 */
class KnowledgeBase
{
public:
	/// Constructor for arbitrary value callback that allows for variable values
	/// to change in between calls to functions of this class.
	explicit KnowledgeBase(std::function<AssignedValue const*(YulName)> _variableValues, Dialect const& _dialect):
		m_variableValues(std::move(_variableValues)),
		m_addBuiltinHandle(_dialect.findBuiltin("add")),
		m_subBuiltinHandle(_dialect.findBuiltin("sub"))
	{}
	/// Constructor to use if source code is in SSA form and values are constant.
	explicit KnowledgeBase(std::map<YulName, AssignedValue> const& _ssaValues, Dialect const& _dialect);

	bool knownToBeDifferent(YulName _a, YulName _b);
	std::optional<u256> differenceIfKnownConstant(YulName _a, YulName _b);
	bool knownToBeDifferentByAtLeast32(YulName _a, YulName _b);
	bool knownToBeZero(YulName _a);
	std::optional<u256> valueIfKnownConstant(YulName _a);
	std::optional<u256> valueIfKnownConstant(Expression const& _expression);

	/// Has to be called whenever the value of @a _var changes.
	void variableChanged(YulName const _var)
	{
		yulAssert(!m_valuesAreSSA);
		yulAssert(!_var.empty());
		++m_currentVersions[_var];
	}

private:
	struct VersionedVariable
	{
		YulName name;
		std::size_t version = 0;

		auto operator<=>(VersionedVariable const&) const = default;
	};

	/**
	 * Constant offset relative to a reference variable, or absolute constant if the
	 * reference variable is the empty YulName.
	 */
	struct VariableOffset
	{
		VersionedVariable reference;
		u256 offset;

		bool isAbsolute() const
		{
			return reference.name.empty();
		}

		std::optional<u256> absoluteValue() const
		{
			if (isAbsolute())
				return offset;
			else
				return std::nullopt;
		}
	};

	struct CachedOffset
	{
		VariableOffset offset;
		/// Value of the variable the offset was computed from, nullptr if it was unknown
		Expression const* computedFrom = nullptr;
	};

	/// @returns @a _var at its current version, which is zero if it never changed.
	VersionedVariable withCurrentVersion(YulName _var) const;

	/// Determines the offset of @a _var relative to the representative of its group by recursively exploring the value
	/// provided by the value callback. The result is memoized. If the value is unknown or cannot be expressed as a
	/// constant offset, @a _var becomes its own representative with offset zero.
	/// @returns the offset
	VariableOffset explore(YulName _var);
	/// Determines the offset of @a _value relative to a representative. Handles literals, identifiers (via the
	/// overload above), and `add`/`sub` if the result is a constant offset from a single representative or a constant.
	/// Arithmetic is modulo 2**256.
	/// @returns the offset or nullopt if @a _value is not of one of these forms
	std::optional<VariableOffset> explore(Expression const& _value);

	/// If true, we can assume that variable values never change and skip some steps.
	bool m_valuesAreSSA = false;
	/// Callback to retrieve the current value of a variable.
	std::function<AssignedValue const*(YulName)> m_variableValues;
	std::optional<BuiltinHandle> m_addBuiltinHandle;
	std::optional<BuiltinHandle> m_subBuiltinHandle;

	/// Offsets for each version of a variable to one representative per group.
	/// The empty string is the representative of the constant value zero.
	std::map<VersionedVariable, CachedOffset> m_offsets;

	/// Current version of each variable that changed at least once.
	std::map<YulName, std::size_t> m_currentVersions;
};

}
