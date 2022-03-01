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
#include <libsolutil/CDCL.h>

#include <boost/rational.hpp>

#include <vector>
#include <variant>
#include <stack>

namespace solidity::util
{

struct State
{
	bool infeasible = false;
	std::map<std::string, size_t> variables;
	std::vector<bool> isBooleanVariable;
	// Potential constraints, referenced through clauses
	std::map<size_t, Constraint> conditionalConstraints;
	std::vector<Clause> clauses;

	// Unconditional bounds on variables
	std::map<size_t, SolvingState::Bounds> bounds;
	// Unconditional constraints
	std::vector<Constraint> fixedConstraints;
};

/**
 * Component that satisfies the SMT SolverInterface and uses an LP solver plus the CDCL
 * algorithm internally.
 * It uses a rational relaxation of the integer program and thus will not be able to answer
 * "satisfiable", but its answers are still correct.
 *
 * Contrary to the usual SMT type system, it adds an implicit constraint for all variables
 * and sub-expressions to be non-negative.
 * TODO this does not apply to e.g. `x + y - something`
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

	smtutil::Expression declareInternalVariable(bool _boolean);
	void declareVariable(std::string const& _name, bool _boolean);

	/// Parses an expression of sort bool and returns a literal.
	std::optional<Literal> parseLiteral(smtutil::Expression const& _expr);
	Literal negate(Literal const& _lit);

	Literal parseLiteralOrReturnEqualBoolean(smtutil::Expression const& _expr);
	smtutil::Expression parseLinearSumOrReturnEqualVariable(smtutil::Expression const& _expr);

	/// Parses the expression and expects a linear sum of variables.
	/// Returns a vector with the first element being the constant and the
	/// other elements the factors for the respective variables.
	/// If the expression cannot be properly parsed or is not linear,
	/// returns an empty vector.
	std::optional<LinearExpression> parseLinearSum(smtutil::Expression const& _expression) const;
	std::optional<LinearExpression> parseProduct(smtutil::Expression const& _expression) const;
	std::optional<LinearExpression> parseFactor(smtutil::Expression const& _expression) const;

	bool tryAddDirectBounds(Constraint const& _constraint);
	void addUpperBound(size_t _index, rational _value);
	void addLowerBound(size_t _index, rational _value);

	size_t addConditionalConstraint(Constraint _constraint);

	void addBooleanEquality(Literal const& _left, smtutil::Expression const& _right);

	//std::string toString(std::vector<SolvingState::Bounds> const& _bounds) const;
	std::string toString(Clause const& _clause) const;
	std::string toString(Constraint const& _constraint) const;

	Constraint const& conditionalConstraint(size_t _index) const;

	std::string variableName(size_t _index) const;

	bool isBooleanVariable(std::string const& _name) const;
	bool isBooleanVariable(size_t _index) const;
	bool isConditionalConstraint(size_t _index) const { return state().conditionalConstraints.count(_index); }

	State& state() { return m_state.back(); }
	State const& state() const { return m_state.back(); }

	/// Stack of state, to allow for push()/pop().
	std::vector<State> m_state{{State{}}};
	// TODO this is only here so that it can keep its cache.
	// It might be better to just have the cache here.
	// Although its stote is only the cache in the end...
	LPSolver m_lpSolver{false};
};


}
