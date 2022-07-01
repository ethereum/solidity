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
#include "heap.h"

namespace solidity::util
{

/**
 * A literal of a (potentially negated) boolean variable or an inactive constraint.
 */
struct Literal
{
	// TODO do we need to init them?
	bool positive;
	// Either points to a boolean variable or to a constraint.
	size_t variable;

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

class CDCL
{
public:
	using Model = std::map<size_t, bool>;
	CDCL(
		std::vector<std::string> _variables,
		std::vector<Clause> const& _clauses,
		std::function<std::optional<Clause>(size_t, std::map<size_t, bool> const&)> _theorySolver = {},
		std::function<void(size_t)> _backtrackNotify = {}
	);

	std::optional<Model> solve();

private:
	struct VarOrderLt { ///Order variables according to their activities
        const std::vector<double>&  activities;
        bool operator () (const int x, const int y) const
        {
            return activities[(size_t)x] > activities[(size_t)y];
        }

        explicit VarOrderLt(const std::vector<double>& _activities) :
            activities(_activities)
        {}
    };


	bool solve_loop(const uint32_t max_conflicts, CDCL::Model& model, int& solution);
	void setupWatches(Clause& _clause);
	std::optional<Clause> propagate();
	std::pair<Clause, size_t> analyze(Clause _conflictClause);
	size_t currentDecisionLevel() const { return m_decisionPoints.size(); }

	void addClause(Clause _clause);

	void enqueue(Literal const& _literal, Clause const* _reason);

	void cancelUntil(size_t _backtrackLevel);

	std::optional<size_t> nextDecisionVariable();

	bool isAssigned(Literal const& _literal) const;
	bool isAssignedTrue(Literal const& _literal) const;
	bool isAssignedFalse(Literal const& _literal) const;
	bool isUnknownOrAssignedTrue(Literal const& _literal) const;

	std::string toString(Literal const& _literal) const;
	std::string toString(Clause const& _clause) const;

	/// Callback that receives an assignment and uses the theory to either returns nullopt ("satisfiable")
	/// or a conflict clause, i.e. a clauses that is false in the theory with the given assignments.
	std::function<std::optional<Clause>(size_t, std::map<size_t, bool>)> m_theorySolver;
	std::function<void(size_t)> m_backtrackNotify;

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
	std::map<size_t, bool> m_assignments;
	std::map<size_t, bool> m_assignments_cache; // Polarity caching. All propagated values end up here
	std::map<size_t, size_t> m_levelForVariable;
	/// TODO wolud be good to not have to copy the clauses
	std::map<Literal, Clause const*> m_reason;
#ifdef DEBUG
	uint64_t m_sumConflicts = 0;
#endif

	// Var activity
	Heap<VarOrderLt> m_order;
	std::vector<double> m_activity;
	double m_var_inc_vsids = 1;
	double m_var_decay = 0.95;
	void vsids_decay_var_act()
	{
		m_var_inc_vsids *= (1.0 / m_var_decay);
	}
	void vsidsBumpVarAct(const uint32_t var)
	{
		assert(m_activity.size() > var);
		m_activity[var] += m_var_inc_vsids;

		bool rescaled = false;
		if (m_activity[var] > 1e100) {
			// Rescale
			for (auto& a: m_activity) a *= 1e-100;
			rescaled = true;
			m_var_inc_vsids *= 1e-100;
		}

		// Update order_heap with respect to new activity:
		if (m_order.inHeap((int)var)) m_order.decrease((int)var);
		if (rescaled) assert(m_order.heap_property());
	}

	// TODO group those into a class

	std::vector<Literal> m_assignmentTrail;
	uint64_t m_longest_trail = 0;
	/// Indices into assignmentTrail where decisions were taken.
	std::vector<size_t> m_decisionPoints;
	/// Index into assignmentTrail: All assignments starting there have not yet been propagated.
	size_t m_assignmentQueuePointer = 0;

	std::vector<size_t> m_assignemntTrailSizesWeCalledSolverFor;
};


}
