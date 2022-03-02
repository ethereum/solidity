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

#include <vector>
#include <tuple>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <optional>
#include <cassert>

namespace solidity::util
{

/**
 * A literal of a (potentially negated) boolean variable or an inactive constraint.
 */
struct Literal
{
	bool positive;
	// Either points to a boolean variable or to a constraint.
	size_t variable{0};

	Literal operator~() const { return Literal{!positive, variable}; }
	bool operator==(Literal const& _other) const
	{
			return std::make_tuple(positive, variable) == std::make_tuple(_other.positive, _other.variable);
	}
	bool operator!=(Literal const& _other) const { return !(*this == _other); }
	bool operator<(Literal const& _other) const
	{
		return std::make_tuple(positive, variable) < std::make_tuple(_other.positive, _other.variable);
	}
};

using Clause = std::vector<Literal>;

class TriState {
public:
	explicit constexpr TriState(const bool b) : val(b ? 1 : 0) {}
	constexpr TriState() {}

	bool operator==(const TriState& other) const
	{
		return val == other.val;
	}

	bool operator!=(const TriState& other) const
	{
		return val != other.val;
	}

	bool toBool() const {
		if (val == 1)
			return true;
		else if (val == 0)
			return false;
		else
		{
			assert(val== 2);
			assert(false && "UNSET cannot be converted to bool");
		}
	}

	std::string toString()
	{
		if (val == 1)
			return "true";
		else if (val == 0)
			return "false";
		else
		{
			assert(val == 2);
			return "unset";
		}
	}

	constexpr static TriState t_true();
	constexpr static TriState t_false();
	constexpr static TriState t_unset();

private:
	// Default value is UNSET
	uint8_t val = 2;
};

constexpr TriState TriState::t_true() {
	return TriState(true);
}

constexpr TriState TriState::t_false() {
	return TriState(false);
}

constexpr TriState TriState::t_unset() {
	return TriState();
}


class CDCL
{
public:
	using Model = std::vector<TriState>;
	CDCL(
		std::vector<std::string> _variables,
		std::vector<Clause> const& _clauses,
		std::function<std::optional<Clause>(std::vector<TriState> const&)> _theoryPropagator = {}
	);

	std::optional<Model> solve();

private:
	void setupWatches(Clause& _clause);
	std::optional<Clause> propagate();
	std::pair<Clause, size_t> analyze(Clause _conflictClause);
	size_t currentDecisionLevel() const { return m_decisionPoints.size(); }

	void addClause(const Clause& _lits);

	void enqueue(Literal const& _literal, Clause const* _reason);

	void cancelUntil(size_t _backtrackLevel);

	std::optional<size_t> nextDecisionVariable() const;

	TriState value(Literal const& _literal) const;
	TriState value(size_t const& variable) const;
	std::string toString(Literal const& _literal) const;
	std::string toString(Clause const& _clause) const;

	/// Callback that receives an assignment and uses the theory to either returns nullopt ("satisfiable")
	/// or a conflict clause, i.e. a clauses that is false in the theory with the given assignments.
	std::function<std::optional<Clause>(std::vector<TriState>)> m_theorySolver;

	std::vector<std::string> m_variables;
	/// includes the learnt clauses
	std::vector<std::unique_ptr<Clause>> m_clauses;

	/// During the execution of the algorithm, the clauses are madified to ensure that:
	/// The first two literals are either true or unknown.
	/// Those two literals are called "watched literals".
	/// This map contains the reverse pointers from the literals.
	/// The idea is that these two literals suffice to know if a clause is unsatisfied
	/// (it might be satisfied without us knowing, but that is not bad).
	std::map<Literal, std::vector<Clause*>> m_watches;

	/// Current assignments.
	std::vector<TriState> m_assignments;
	std::map<size_t, size_t> m_levelForVariable;
	/// TODO wolud be good to not have to copy the clauses
	std::map<Literal, Clause const*> m_reason;

	// TODO group those into a class

	std::vector<Literal> m_assignmentTrail;
	/// Indices into assignmentTrail where decisions were taken.
	std::vector<size_t> m_decisionPoints;
	/// Index into assignmentTrail: All assignments starting there have not yet been propagated.
	size_t m_assignmentQueuePointer = 0;

	// Current state of the solver. If FALSE, we are in an UNSAT state.
	bool ok = true;
};

inline TriState CDCL::value(Literal const& _literal) const
{
	if (m_assignments[_literal.variable] == TriState::t_unset())
		return TriState::t_unset();
	else
		return TriState(m_assignments[_literal.variable].toBool() ^ !_literal.positive);
}

inline TriState CDCL::value(size_t const& variable) const
{
	return m_assignments[variable];
}

}
