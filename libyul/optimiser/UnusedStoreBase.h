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
 * Base class for both UnusedAssignEliminator and UnusedStoreEliminator.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AST.h>

#include <range/v3/action/remove_if.hpp>

#include <variant>


namespace solidity::yul
{
struct Dialect;

/**
 * Base class for both UnusedAssignEliminator and UnusedStoreEliminator.
 *
 * The class tracks the state of abstract "stores" (assignments or mstore/sstore
 * statements) across the control-flow. It is the job of the derived class to create
 * the stores and track references, but the base class adjusts their "used state" at
 * control-flow splits and joins.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class UnusedStoreBase: public ASTWalker
{
public:
	explicit UnusedStoreBase(Dialect const& _dialect): m_dialect(_dialect) {}

	using ASTWalker::operator();
	void operator()(If const& _if) override;
	void operator()(Switch const& _switch) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(ForLoop const&) override;
	void operator()(Break const&) override;
	void operator()(Continue const&) override;

protected:
	class State
	{
	public:
		enum Value { Unused, Undecided, Used };
		State(Value _value = Undecided): m_value(_value) {}
		inline bool operator==(State _other) const { return m_value == _other.m_value; }
		inline bool operator!=(State _other) const { return !operator==(_other); }
		static inline void join(State& _a, State const& _b)
		{
			// Using "max" works here because of the order of the values in the enum.
			_a.m_value =  Value(std::max(int(_a.m_value), int(_b.m_value)));
		}
	private:
		Value m_value = Undecided;
	};

	using TrackedStores = std::map<YulString, std::map<Statement const*, State>>;

	/// This function is called for a loop that is nested too deep to avoid
	/// horrible runtime and should just resolve the situation in a pragmatic
	/// and correct manner.
	virtual void shortcutNestedLoop(TrackedStores const& _beforeLoop) = 0;

	/// This function is called right before the scoped restore of the function definition.
	virtual void finalizeFunctionDefinition(FunctionDefinition const& /*_functionDefinition*/) {}

	/// Joins the assignment mapping of @a _source into @a _target according to the rules laid out
	/// above.
	/// Will destroy @a _source.
	static void merge(TrackedStores& _target, TrackedStores&& _source);
	static void merge(TrackedStores& _target, std::vector<TrackedStores>&& _source);

	Dialect const& m_dialect;
	std::set<Statement const*> m_pendingRemovals;
	TrackedStores m_stores;

	/// Working data for traversing for-loops.
	struct ForLoopInfo
	{
		/// Tracked assignment states for each break statement.
		std::vector<TrackedStores> pendingBreakStmts;
		/// Tracked assignment states for each continue statement.
		std::vector<TrackedStores> pendingContinueStmts;
	};
	ForLoopInfo m_forLoopInfo;
	size_t m_forLoopNestingDepth = 0;
};

}
