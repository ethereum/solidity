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

struct Clause {
	const Literal& front() const
	{
		return lits.front();
	}
	const Literal& back() const
	{
		return lits.back();
	}
	Literal& front()
	{
		return lits.front();
	}
	Literal& back()
	{
		return lits.back();
	}
	const Literal& operator[](const size_t at) const
	{
		return lits.at(at);
	}
	const Literal& at(const size_t at) const
	{
		return lits.at(at);
	}
	Literal& operator[](const size_t at)
	{
		return lits.at(at);
	}
	Literal& at(const size_t at)
	{
		return lits.at(at);
	}
	auto size() const
	{
		return lits.size();
	}
	auto empty() const
	{
		return lits.empty();
	}
	auto begin() {
		return lits.begin();
	}
	auto end() {
		return lits.end();
	}
	auto begin() const {
		return lits.begin();
	}
	auto end() const {
		return lits.end();
	}
	void push_back(const Literal& l)
	{
		lits.push_back(l);
	}

	std::vector<Literal> lits;
	uint64_t ID;
};

enum class TriState {trisate_true, tristate_false, tristate_unset};
TriState boolToTriState(const bool b);
inline std::string triStateToString(const TriState v);
inline std::string triStateToString(const TriState v)
{
	if (v == TriState::trisate_true)
		return "true";
	else if (v == TriState::tristate_false)
		return "false";
	else
		return "unset";
}
inline TriState boolToTriState(const bool b) {
	if (b)
		return TriState::trisate_true;
	else
		return TriState::tristate_false;
}

inline bool triStateToBool(const TriState t) {
	if (t == TriState::trisate_true)
		return true;
	else if (t == TriState::tristate_false)
		return false;
	else
		assert(false && "UNSET cannot be converted to bool");
}

class CDCL
{
public:
	using Model = std::vector<TriState>;
	CDCL(
		std::vector<std::string> _variables,
		std::vector<std::vector<Literal>> const& _clauses,
	    std::ostream* proof = nullptr,
		std::function<std::optional<Clause>(std::vector<TriState> const&)> _theoryPropagator = {}
	);

	std::optional<Model> solve();

private:
	void setupWatches(Clause& _clause);
	std::optional<Clause> propagate();
	std::pair<Clause, size_t> analyze(Clause _conflictClause);
	size_t currentDecisionLevel() const { return m_decisionPoints.size(); }

	void addClause(const std::vector<Literal>& _lits);

	void enqueue(Literal const& _literal, Clause const* _reason);

	void cancelUntil(size_t _backtrackLevel);

	std::optional<size_t> nextDecisionVariable() const;

	inline bool isAssigned(Literal const& _literal) const;
	inline bool isAssignedTrue(Literal const& _literal) const;
	inline bool isAssignedFalse(Literal const& _literal) const;
	inline bool isUnknownOrAssignedTrue(Literal const& _literal) const;

	std::string toString(Literal const& _literal) const;
	std::string toString(Clause const& _clause) const;
	std::string toProofString(Literal const& _literal) const;
	std::string toProofString(Clause const& _clause) const;

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

	// Proof log
	std::ostream* proof = nullptr;
	uint64_t clause_ID = 1;
	std::map<Literal, uint64_t> unit_cl_IDs;
	uint64_t unsat_clause_ID = 0;
	void writeFinalProofClauses();
};

inline bool CDCL::isAssigned(Literal const& _literal) const
{
	return m_assignments[_literal.variable] != TriState::tristate_unset;
}

inline bool CDCL::isAssignedTrue(Literal const& _literal) const
{
	return isAssigned(_literal) &&
		(triStateToBool(m_assignments[_literal.variable]) ^ !_literal.positive) == true;
}

inline bool CDCL::isAssignedFalse(Literal const& _literal) const
{
	return isAssigned(_literal) &&
		(triStateToBool(m_assignments[_literal.variable]) ^ !_literal.positive) == false;

}

inline bool CDCL::isUnknownOrAssignedTrue(Literal const& _literal) const
{
	return (
		!isAssigned(_literal) ||
		isAssignedTrue(_literal)
	);
}


}
