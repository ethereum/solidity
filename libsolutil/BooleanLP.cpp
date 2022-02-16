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

#include <libsolutil/BooleanLP.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>
#include <liblangutil/Exceptions.h>

#include <libsolutil/LinearExpression.h>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/iterator/operations.hpp>

#include <boost/range/algorithm_ext/erase.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::smtutil;

using rational = boost::rational<bigint>;


pair<bool, map<size_t, bool>> DPLL::simplify()
{
	// TODO use vector?
	map<size_t, bool> assignments;
	while (!clauses.empty() && !ranges::all_of(clauses, [](Clause const& _c) { return _c.literals.size() > 1; }))
	{
		// TODO this assumes that no clause contains more than one constraint
		// TODO add an assertion for that.

		for (Clause const& c: clauses)
			if (c.literals.empty())
				return {false, {}};
			else if (c.literals.size() == 1)
			{
				Literal const& literal = c.literals.front();
				if (literal.kind == Literal::Constraint)
					constraints.push_back(literal.index);
				else
				{
					bool value = literal.kind == Literal::PositiveVariable ? true : false;
					if (assignments.count(literal.index) && assignments.at(literal.index) != value)
						return {false, {}};
					//cout << "Assigning" << variableName(literal.index) << " " << value << endl;
					assignments[literal.index] = value;
				}
			}

		if (!setVariables(assignments, true))
			return {false, {}};
	}
	return {true, move(assignments)};
}

size_t DPLL::findUnassignedVariable() const
{
	for (Clause const& c: clauses)
		for (Literal const& l: c.literals)
			if (l.kind != Literal::Constraint)
				return l.index;
	solAssert(false, "");
	return 0;
}

bool DPLL::setVariable(size_t _index, bool _value)
{
	return setVariables({{_index, _value}});
}

bool DPLL::setVariables(map<size_t, bool> const& _assignments, bool _removeSingleConstraintClauses)
{
	// TODO do this in a way so that we do not have to copy twice.
	vector<Clause> prunedClauses;
	for (Clause const& c: clauses)
	{
		bool skipClause = false;
		vector<Literal> newClause;
		if (_removeSingleConstraintClauses && c.literals.size() == 1 && c.literals.front().kind == Literal::Constraint)
			continue;
		for (Literal const& l: c.literals)
			if (l.kind != Literal::Constraint && _assignments.count(l.index))
			{
				if (_assignments.at(l.index) == (l.kind == Literal::PositiveVariable))
				{
					skipClause = true;
					break;
				}
			}
			else
				newClause.push_back(l);
		if (skipClause)
			continue;
		if (newClause.empty())
			return false;
		prunedClauses.push_back(Clause{move(newClause)});
	}
	clauses = move(prunedClauses);
	return true;
}

void BooleanLPSolver::reset()
{
	m_state = vector<State>{{State{}}};
	m_internalVariableCounter = 0;
	m_constraintCounter = 0;
	// Do not reset the solver, it should retain its cache.
}

void BooleanLPSolver::push()
{
	if (m_state.back().infeasible)
	{
		m_state.emplace_back();
		m_state.back().infeasible = true;
		return;
	}
	map<string, size_t> variables = m_state.back().variables;
	map<size_t, array<optional<boost::rational<bigint>>, 2>> bounds = m_state.back().bounds;
	vector<bool> isBooleanVariable = m_state.back().isBooleanVariable;
	m_state.emplace_back();
	m_state.back().variables = move(variables);
	m_state.back().isBooleanVariable = move(isBooleanVariable);
	m_state.back().bounds = move(bounds);
}

void BooleanLPSolver::pop()
{
	m_state.pop_back();
	solAssert(!m_state.empty(), "");
}

void BooleanLPSolver::declareVariable(string const& _name, SortPointer const& _sort)
{
	// Internal variables are '$<number>', so escape `$` to `$$`.
	string name = (_name.empty() || _name.at(0) != '$') ? _name : "$$" + _name;
	// TODO This will not be an integer variable in our model.
	// Introduce a new kind?
	solAssert(_sort && (_sort->kind == Kind::Int || _sort->kind == Kind::Bool), "");
	solAssert(!m_state.back().variables.count(name), "");
	declareVariable(name, _sort->kind == Kind::Bool);
}

void BooleanLPSolver::addAssertion(Expression const& _expr)
{
	if (_expr.arguments.empty())
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{*parseLiteral(_expr)}});
	else if (_expr.name == "=")
	{
		// Try to see if both sides are linear.
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (left && right)
		{
			vector<rational> data = *left - *right;
			data[0] *= -1;
			Constraint c{move(data), _expr.name == "="};
			if (!tryAddDirectBounds(c))
			{
				size_t index = addConstraint(move(c));
				m_state.back().clauses.emplace_back(Clause{vector<Literal>{Literal{Literal::Constraint, index}}});
			}
		}
		else if (_expr.arguments.at(0).arguments.empty() && isBooleanVariable(_expr.arguments.at(0).name))
			addBooleanEquality(*parseLiteral(_expr.arguments.at(0)), _expr.arguments.at(1));
		else if (_expr.arguments.at(1).arguments.empty() && isBooleanVariable(_expr.arguments.at(1).name))
			addBooleanEquality(*parseLiteral(_expr.arguments.at(1)), _expr.arguments.at(0));
		else
		{
			Literal newBoolean = *parseLiteral(declareInternalBoolean());
			addBooleanEquality(newBoolean, _expr.arguments.at(0));
			addBooleanEquality(newBoolean, _expr.arguments.at(1));
		}
	}
	else if (_expr.name == "and")
	{
		addAssertion(_expr.arguments.at(0));
		addAssertion(_expr.arguments.at(1));
	}
	else if (_expr.name == "or")
	{
		// We could try to parse a full clause here.
		Literal left = parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0));
		Literal right = parseLiteralOrReturnEqualBoolean(_expr.arguments.at(1));
		if (left.kind == Literal::Constraint && right.kind == Literal::Constraint)
		{
			// We cannot have more than one constraint per clause.
			right = *parseLiteral(declareInternalBoolean());
			addBooleanEquality(right, _expr.arguments.at(1));
		}
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{left, right}});
	}
	else if (_expr.name == "not")
	{
		Literal l = negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0)));
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{l}});
	}
	else if (_expr.name == "<=")
	{
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return;

		vector<rational> data = *left - *right;
		data[0] *= -1;
		Constraint c{move(data), _expr.name == "="};
		if (!tryAddDirectBounds(c))
		{
			size_t index = addConstraint(move(c));
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{Literal{Literal::Constraint, index}}});
		}
	}
	else if (_expr.name == ">=")
		addAssertion(_expr.arguments.at(1) <= _expr.arguments.at(0));
	else if (_expr.name == "<")
		addAssertion(_expr.arguments.at(0) <= _expr.arguments.at(1) - 1);
	else if (_expr.name == ">")
		addAssertion(_expr.arguments.at(1) < _expr.arguments.at(0));
	else
		cout << "Unknown operator " << _expr.name << endl;
}


pair<CheckResult, vector<string>> BooleanLPSolver::check(vector<Expression> const& _expressionsToEvaluate)
{
//	cout << "Solving boolean constraint system" << endl;
//	cout << toString() << endl;
//	cout << "--------------" << endl;

	if (m_state.back().infeasible)
		return make_pair(CheckResult::UNSATISFIABLE, vector<string>{});

	// TODO compress variables because we ignore boolean variables
	SolvingState state;
	for (auto&& [name, index]: m_state.back().variables)
		resizeAndSet(state.variableNames, index, name);
	for (auto&& [index, value]: m_state.back().bounds)
		resizeAndSet(state.bounds, index, value);

	std::vector<Clause> clauses;
	for (State const& s: m_state)
		for (Clause const& clause: s.clauses)
			clauses.push_back(clause);

	auto&& [result, model] = runDPLL(state, DPLL{move(clauses), {}});
	if (result == CheckResult::SATISFIABLE)
	{
		vector<string> requestedModel;
		for (Expression const& e: _expressionsToEvaluate)
			requestedModel.emplace_back(
				e.arguments.empty() && model.count(e.name) ?
				::toString(model[e.name], 0) :
				string("unknown")
			);
		return {result, move(requestedModel)};
	}
	else
		return {result, {}};
}

string BooleanLPSolver::toString() const
{
	string result;

	for (State const& s: m_state)
		for (Clause const& c: s.clauses)
			result += toString(c);
	result += "-- Bounds:\n";
	for (State const& s: m_state)
		for (auto&& [index, bounds]: s.bounds)
		{
			if (!bounds[0] && !bounds[1])
				continue;
			if (bounds[0])
				result += ::toString(*bounds[0]) + " <= ";
			result += variableName(index);
			if (bounds[1])
				result += " <= " + ::toString(*bounds[1]);
			result += "\n";
		}
	return result;
}

Expression BooleanLPSolver::declareInternalBoolean()
{
	string name = "$" + to_string(m_internalVariableCounter++);
	declareVariable(name, true);
	return smtutil::Expression(name, {}, SortProvider::boolSort);
}

void BooleanLPSolver::declareVariable(string const& _name, bool _boolean)
{
	size_t index = m_state.back().variables.size() + 1;
	m_state.back().variables[_name] = index;
	resizeAndSet(m_state.back().isBooleanVariable, index, _boolean);
}

optional<Literal> BooleanLPSolver::parseLiteral(smtutil::Expression const& _expr)
{
	// TODO constanst true/false?

	if (_expr.arguments.empty())
	{
		if (isBooleanVariable(_expr.name))
			return Literal{
				Literal::PositiveVariable,
				m_state.back().variables.at(_expr.name)
			};
		else
			cout << "cannot encode " << _expr.name << " - not a boolean literal variable." << endl;
	}
	else if (_expr.name == "not")
		return negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0)));
	else if (_expr.name == "<=")
	{
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return {};

		vector<rational> data = *left - *right;
		data[0] *= -1;

		return Literal{Literal::Constraint, addConstraint(Constraint{move(data), false})};
	}
	else if (_expr.name == ">=")
		return parseLiteral(_expr.arguments.at(1) <= _expr.arguments.at(0));
	else if (_expr.name == "<")
		return parseLiteral(_expr.arguments.at(0) <= _expr.arguments.at(1) - 1);
	else if (_expr.name == ">")
		return parseLiteral(_expr.arguments.at(1) < _expr.arguments.at(0));

	return {};
}

Literal BooleanLPSolver::negate(Literal const& _lit)
{
	switch (_lit.kind)
	{
	case Literal::NegativeVariable:
		return Literal{Literal::PositiveVariable, _lit.index};
	case Literal::PositiveVariable:
		return Literal{Literal::NegativeVariable, _lit.index};
	default:
		break;
	}

	Constraint const& c = constraint(_lit.index);

	solAssert(!c.equality, "");

	// X > b
	// -x < -b
	// -x <= -b - 1

	Constraint negated = c;
	negated.data *= -1;
	negated.data[0] -= 1;

	return Literal{Literal::Constraint, addConstraint(move(negated))};
}

Literal BooleanLPSolver::parseLiteralOrReturnEqualBoolean(Expression const& _expr)
{
	if (optional<Literal> literal = parseLiteral(_expr))
		return *literal;
	else
	{
		Literal newBoolean = *parseLiteral(declareInternalBoolean());
		addBooleanEquality(newBoolean, _expr);
		return newBoolean;
	}
}

optional<vector<rational>> BooleanLPSolver::parseLinearSum(smtutil::Expression const& _expr) const
{
	if (_expr.arguments.empty() || _expr.name == "*")
		return parseProduct(_expr);
	else if (_expr.name == "+" || _expr.name == "-")
	{
		optional<vector<rational>> left = parseLinearSum(_expr.arguments.at(0));
		optional<vector<rational>> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return std::nullopt;
		return _expr.name == "+" ? add(*left, *right) : *left - *right;
	}
	else
		return std::nullopt;
}

optional<vector<rational>> BooleanLPSolver::parseProduct(smtutil::Expression const& _expr) const
{
	if (_expr.arguments.empty())
		return parseFactor(_expr);
	else if (_expr.name == "*")
		// The multiplication ensures that only one of them can be a variable.
		return vectorProduct(parseFactor(_expr.arguments.at(0)), parseFactor(_expr.arguments.at(1)));
	else
		return std::nullopt;
}

optional<vector<rational>> BooleanLPSolver::parseFactor(smtutil::Expression const& _expr) const
{
	solAssert(_expr.arguments.empty(), "");
	solAssert(!_expr.name.empty(), "");
	if ('0' <= _expr.name[0] && _expr.name[0] <= '9')
		return vector<rational>{rational(bigint(_expr.name))};
	else if (_expr.name == "true")
		return vector<rational>{rational(bigint(1))};
	else if (_expr.name == "false")
		return vector<rational>{rational(bigint(0))};

	size_t index = m_state.back().variables.at(_expr.name);
	solAssert(index > 0, "");
	if (isBooleanVariable(index))
		return nullopt;
	return factorForVariable(index, rational(bigint(1)));
}

bool BooleanLPSolver::tryAddDirectBounds(Constraint const& _constraint)
{
	auto nonzero = _constraint.data | ranges::views::enumerate | ranges::views::tail | ranges::views::filter(
		[](std::pair<size_t, rational> const& _x) { return !!_x.second; }
	);
	// TODO we can exit early on in the loop above.
	if (ranges::distance(nonzero) > 1)
		return false;

	//cout << "adding direct bound." << endl;
	if (ranges::distance(nonzero) == 0)
	{
		// 0 <= b or 0 = b
		if (
			_constraint.data.front() < 0 ||
			(_constraint.equality && _constraint.data.front() != 0)
		)
		{
//			cout << "SETTING INF" << endl;
			m_state.back().infeasible = true;
		}
	}
	else
	{
		auto&& [varIndex, factor] = nonzero.front();
		// a * x <= b
		rational bound = _constraint.data[0] / factor;
		if (factor > 0 || _constraint.equality)
			addUpperBound(varIndex, bound);
		if (factor < 0 || _constraint.equality)
			addLowerBound(varIndex, bound);
	}
	return true;
}

void BooleanLPSolver::addUpperBound(size_t _index, rational _value)
{
	//cout << "adding " << variableName(_index) << " <= " << toString(_value) << endl;
	if (!m_state.back().bounds[_index][1] || _value < *m_state.back().bounds[_index][1])
		m_state.back().bounds[_index][1] = move(_value);
}

void BooleanLPSolver::addLowerBound(size_t _index, rational _value)
{
	// Lower bound must be at least zero.
	_value = max(_value, rational{});
	//cout << "adding " << variableName(_index) << " >= " << toString(_value) << endl;
	if (!m_state.back().bounds[_index][0] || _value > *m_state.back().bounds[_index][0])
		m_state.back().bounds[_index][0] = move(_value);
}

size_t BooleanLPSolver::addConstraint(Constraint _constraint)
{
	size_t index = ++m_constraintCounter;
	m_state.back().constraints[index] = move(_constraint);
	return index;
}

void BooleanLPSolver::addBooleanEquality(Literal const& _left, smtutil::Expression const& _right)
{
	if (optional<Literal> right = parseLiteral(_right))
	{
		// includes: not, <=, <, >=, >, boolean variables.
		// a = b <=> (-a \/ b) /\ (a \/ -b)
		Literal negLeft = negate(_left);
		Literal negRight = negate(*right);
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{negLeft, *right}});
		m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negRight}});
	}
	else if (_right.name == "=" && parseLinearSum(_right.arguments.at(0)) && parseLinearSum(_right.arguments.at(1)))
		// a = (x = y)  <=>  a = (x <= y && x >= y)
		addBooleanEquality(
			_left,
			_right.arguments.at(0) <= _right.arguments.at(1) &&
			_right.arguments.at(1) <= _right.arguments.at(0)
		);
	else
	{
		Literal a = parseLiteralOrReturnEqualBoolean(_right.arguments.at(0));
		Literal b = parseLiteralOrReturnEqualBoolean(_right.arguments.at(1));
		if (a.kind == Literal::Constraint && b.kind == Literal::Constraint)
		{
			// We cannot have more than one constraint per clause.
			b = *parseLiteral(declareInternalBoolean());
			addBooleanEquality(b, _right.arguments.at(1));
		}

		if (_right.name == "and")
		{
			// a = and(x, y) <=> (-a \/ x) /\ ( -a \/ y) /\ (a \/ -x \/ -y)
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), b}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a), negate(b)}});
		}
		else if (_right.name == "or")
		{
			// a = or(x, y) <=> (-a \/ x \/ y) /\ (a \/ -x) /\ (a \/ -y)
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a, b}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a)}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(b)}});
		}
		else if (_right.name == "=")
		{
			// l = eq(a, b) <=> (-l or -a or b) and (-l or a or -b) and (l or -a or -b) and (l or a or b)
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), negate(a), b}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a, negate(b)}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a), negate(b)}});
			m_state.back().clauses.emplace_back(Clause{vector<Literal>{_left, a, b}});
		}
		else
			solAssert(false, "Unsupported operation: " + _right.name);
	}
}


// TODO as input we do not need the full solving state, only the bounds
// (and the variable names)
pair<CheckResult, map<string, rational>> BooleanLPSolver::runDPLL(SolvingState& _solvingState, DPLL _dpll)
{
	//cout << "Running dpll on" << endl << toString(_solvingState.bounds) << "\nwith clauses\n" << toString(_dpll) << endl;

	// Simplify clauses with only one literal.
	// TODO Maybe this could already analyze clauses and add bounds?
	auto&& [simplifyResult, booleanModel] = _dpll.simplify();
	if (!simplifyResult)
		return {CheckResult::UNSATISFIABLE, {}};

//	cout << "Simplified to" << endl << toString(_solvingState.bounds) << "\nwith clauses\n" << toString(_dpll) << endl;
//	cout << "----------" << endl;

	CheckResult result = CheckResult::UNKNOWN;
	map<string, rational> model;

	{
		//cout << "Invoking LP..." << endl;
		_solvingState.constraints.clear();
		for (size_t c: _dpll.constraints)
			_solvingState.constraints.emplace_back(constraint(c));
		LPResult lpResult;
		tie(lpResult, model) = m_lpSolver.check(_solvingState);
		// LP solver is a rational solver, not an integer solver,
		// so "feasible" does not mean there is an integer solution.

		// TODO we could check the model to see if all variables are integer....s

		// TODO if it is a pure boolean problem, we can actually answer "satisfiable"
		switch (lpResult)
		{
		case LPResult::Infeasible:
//			cout << "Infeasible." << endl;
			result = CheckResult::UNSATISFIABLE;
			break;
		case LPResult::Feasible:
		case LPResult::Unbounded:
//			cout << "Feasible." << endl;
			// TODO this is actually wrong, but difficult to test otherwise.
			result = CheckResult::SATISFIABLE;
			break;
		case LPResult::Unknown:
//			cout << "Unknown." << endl;
			result = CheckResult::UNKNOWN;
			break;
		}
	}
	if (result != CheckResult::UNSATISFIABLE && !_dpll.clauses.empty())
	{
		size_t varIndex = _dpll.findUnassignedVariable();

		DPLL copy = _dpll;
		if (_dpll.setVariable(varIndex, true))
		{
			booleanModel[varIndex] = true;
			//cout << "Trying " << variableName(varIndex) << " = true\n";
			tie(result, model) = runDPLL(_solvingState, move(_dpll));
			// TODO actually we should also handle UNKNOWN here.
		}
		// TODO it will never be "satisfiable"
		if (result != CheckResult::SATISFIABLE)
		{
			//cout << "Trying " << variableName(varIndex) << " = false\n";
			if (!copy.setVariable(varIndex, false))
				return {CheckResult::UNSATISFIABLE, {}};
			booleanModel[varIndex] = false;
			tie(result, model) = runDPLL(_solvingState, move(copy));
		}
	}
	if (result == CheckResult::SATISFIABLE)
	{
		for (auto const& [index, value]: booleanModel)
			model[variableName(index)] = value ? 1 : 0;
		return {result, move(model)};
	}
	else
		return {result, {}};
}

string BooleanLPSolver::toString(DPLL const& _dpll) const
{
	string result;
	for (size_t c: _dpll.constraints)
		result += toString(Clause{{Literal{Literal::Constraint, c}}});
	for (Clause const& c: _dpll.clauses)
		result += toString(c);
	return result;
}

string BooleanLPSolver::toString(std::vector<std::array<std::optional<boost::rational<bigint>>, 2>> const& _bounds) const
{
	string result;
	for (auto&& [index, bounds]: _bounds | ranges::views::enumerate)
	{
		if (!bounds[0] && !bounds[1])
			continue;
		if (bounds[0])
			result += ::toString(*bounds[0]) + " <= ";
		// TODO If the variables are compressed, this does no longer work.
		result += variableName(index);
		if (bounds[1])
			result += " <= " + ::toString(*bounds[1]);
		result += "\n";
	}
	return result;
}

string BooleanLPSolver::toString(Clause const& _clause) const
{
	vector<string> literals;
	for (Literal const& l: _clause.literals)
		if (l.kind == Literal::Constraint)
		{
			Constraint const& constr = constraint(l.index);
			vector<string> line;
			for (auto&& [index, multiplier]: constr.data | ranges::views::enumerate)
				if (index > 0 && multiplier != 0)
				{
					string mult =
						multiplier == -1 ?
						"-" :
						multiplier == 1 ?
						"" :
						::toString(multiplier) + " ";
					line.emplace_back(mult + variableName(index));
				}
			literals.emplace_back(joinHumanReadable(line, " + ") + (constr.equality ? "  = " : " <= ") + ::toString(constr.data.front()));
		}
		else
			literals.emplace_back((l.kind == Literal::NegativeVariable ? "!" : "") + variableName(l.index));
	return joinHumanReadable(literals, "  \\/  ") + "\n";
}

Constraint const& BooleanLPSolver::constraint(size_t _index) const
{
	for (State const& state: m_state)
		if (state.constraints.count(_index))
			return state.constraints.at(_index);
	solAssert(false, "");
}


string BooleanLPSolver::variableName(size_t _index) const
{
	for (auto const& v: m_state.back().variables)
		if (v.second == _index)
			return v.first;
	return {};
}

bool BooleanLPSolver::isBooleanVariable(string const& _name) const
{
	if (!m_state.back().variables.count(_name))
		return false;
	size_t index = m_state.back().variables.at(_name);
	solAssert(index > 0, "");
	return isBooleanVariable(index);
}

bool BooleanLPSolver::isBooleanVariable(size_t _index) const
{
	return
		_index < m_state.back().isBooleanVariable.size() &&
		m_state.back().isBooleanVariable.at(_index);
}
