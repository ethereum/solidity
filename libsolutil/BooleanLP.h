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

#include <libsmtutil/SolverInterface.h>

#include <libsolutil/LP.h>

#include <boost/rational.hpp>

#include <vector>
#include <variant>
#include <stack>
#include <compare>

namespace solidity::util
{

/**
 * A literal of a (potentially negated) boolean variable or an inactive constraint.
 */
struct Literal
{
	enum { PositiveVariable, NegativeVariable, Constraint } kind;
	// Either points to a boolean variable or to a constraint.
	size_t index{0};
};

/**
 * A clause is a disjunction of literals.
 */
struct Clause
{
	std::vector<Literal> literals;
};


struct State
{
	bool infeasible = false;
	std::map<std::string, size_t> variables;
	std::vector<bool> isBooleanVariable;
	// Potential constraints, referenced through clauses
	std::map<size_t, Constraint> constraints;
	// Unconditional bounds on variables
	std::map<size_t, std::array<std::optional<boost::rational<bigint>>, 2>> bounds;

	std::vector<Clause> clauses;
};

struct DPLL
{
	/// Try to simplify the set of clauses without branching.
	/// @returns false if the set of clauses is unsatisfiable (without considering the constraints).
	std::pair<bool, std::map<size_t, bool>> simplify();
	size_t findUnassignedVariable() const;
	bool setVariable(size_t _index, bool _value);
	/// Sets variables and removes clauses or literals from clauses.
	/// @returns false if the clauses are unsatisfiable.
	bool setVariables(std::map<size_t, bool> const& _assignments, bool _removeSingleConstraintClauses = false);

	std::vector<Clause> clauses;
	std::vector<size_t> constraints;
};


/**
 * Component that satisfies the SMT SolverInterface and uses an LP solver plus the DPLL
 * algorithm internally.
 * It uses a rational relaxation of the integer program and thus will not be able to answer
 * "satisfiable", but its answers are still correct.
 *
 * TODO are integers always non-negative?
 *
 * Integers are unbounded.
 */
class BooleanLPSolver: public smtutil::SolverInterface
{
public:
	void reset() override;
	void push() override;
	void pop() override;

	void declareVariable(std::string const& _name, smtutil::SortPointer const& _sort) override;

	void addAssertion(smtutil::Expression const& _expr) override;

	std::pair<smtutil::CheckResult, std::vector<std::string>>
	check(std::vector<smtutil::Expression> const& _expressionsToEvaluate) override;

	std::pair<smtutil::CheckResult, std::map<std::string, boost::rational<bigint>>> check();

	std::string toString() const;

private:
	using rational = boost::rational<bigint>;

	smtutil::Expression declareInternalBoolean();
	void declareVariable(std::string const& _name, bool _boolean);

	std::optional<Literal> parseLiteral(smtutil::Expression const& _expr);
	Literal negate(Literal const& _lit);

	Literal parseLiteralOrReturnEqualBoolean(smtutil::Expression const& _expr);

	/// Parses the expression and expects a linear sum of variables.
	/// Returns a vector with the first element being the constant and the
	/// other elements the factors for the respective variables.
	/// If the expression cannot be properly parsed or is not linear,
	/// returns an empty vector.
	std::optional<std::vector<rational>> parseLinearSum(smtutil::Expression const& _expression) const;
	std::optional<std::vector<rational>> parseProduct(smtutil::Expression const& _expression) const;
	std::optional<std::vector<rational>> parseFactor(smtutil::Expression const& _expression) const;

	bool tryAddDirectBounds(Constraint const& _constraint);
	void addUpperBound(size_t _index, rational _value);
	void addLowerBound(size_t _index, rational _value);

	size_t addConstraint(Constraint _constraint);

	void addBooleanEquality(Literal const& _left, smtutil::Expression const& _right);

	std::pair<smtutil::CheckResult, std::map<std::string, rational>> runDPLL(SolvingState& _solvingState, DPLL _dpll);
	std::string toString(DPLL const& _dpll) const;
	std::string toString(std::vector<std::array<std::optional<boost::rational<bigint>>, 2>> const& _bounds) const;
	std::string toString(Clause const& _clause) const;

	Constraint const& constraint(size_t _index) const;

	std::string variableName(size_t _index) const;

	bool isBooleanVariable(std::string const& _name) const;
	bool isBooleanVariable(size_t _index) const;

	size_t m_constraintCounter = 0;
	size_t m_internalVariableCounter = 0;
	/// Stack of state, to allow for push()/pop().
	std::vector<State> m_state{{State{}}};
	LPSolver m_lpSolver;
};


}
