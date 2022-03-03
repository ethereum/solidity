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
#include <libsolutil/CDCL.h>
#include <libsolutil/LP.h>

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

namespace
{
template <class T>
void resizeAndSet(vector<T>& _vector, size_t _index, T _value)
{
	if (_vector.size() < _index + 1)
		_vector.resize(_index + 1);
	_vector[_index] = move(_value);
}

string toString(rational const& _x)
{
	if (_x.denominator() == 1)
		return _x.numerator().str();
	else
		return _x.numerator().str() + "/" + _x.denominator().str();
}

}

void BooleanLPSolver::reset()
{
	m_state = vector<State>{{State{}}};
}

void BooleanLPSolver::push()
{
	// TODO maybe find a way where we do not have to copy everything
	State currentState = state();
	m_state.emplace_back(move(currentState));
}

void BooleanLPSolver::pop()
{
	m_state.pop_back();
	solAssert(!m_state.empty(), "");
}

void BooleanLPSolver::declareVariable(string const& _name, SortPointer const& _sort)
{
	// Internal variables are '$<number>', or '$c<number>' so escape `$` to `$$`.
	string name = (_name.empty() || _name.at(0) != '$') ? _name : "$$" + _name;
	// TODO This will not be an integer variable in our model.
	// Introduce a new kind?
	solAssert(_sort && (_sort->kind == Kind::Int || _sort->kind == Kind::Bool), "");
	solAssert(!state().variables.count(name), "");
	declareVariable(name, _sort->kind == Kind::Bool);
}

void BooleanLPSolver::addAssertion(Expression const& _expr)
{
	cout << " - " << _expr.toString() << endl;
	solAssert(_expr.sort->kind == Kind::Bool);
	if (_expr.arguments.empty())
	{
		solAssert(isBooleanVariable(_expr.name));
		state().clauses.emplace_back(Clause{Literal{
			true,
			state().variables.at(_expr.name)
		}});
	}
	else if (_expr.name == "=")
	{
		solAssert(_expr.arguments.at(0).sort->kind == _expr.arguments.at(1).sort->kind);
		if (_expr.arguments.at(0).sort->kind == Kind::Bool)
		{
			if (_expr.arguments.at(0).arguments.empty() && isBooleanVariable(_expr.arguments.at(0).name))
				addBooleanEquality(*parseLiteral(_expr.arguments.at(0)), _expr.arguments.at(1));
			else if (_expr.arguments.at(1).arguments.empty() && isBooleanVariable(_expr.arguments.at(1).name))
				addBooleanEquality(*parseLiteral(_expr.arguments.at(1)), _expr.arguments.at(0));
			else
			{
				Literal newBoolean = *parseLiteral(declareInternalVariable(true));
				addBooleanEquality(newBoolean, _expr.arguments.at(0));
				addBooleanEquality(newBoolean, _expr.arguments.at(1));
			}
		}
		else if (_expr.arguments.at(0).sort->kind == Kind::Int)
		{
			// Try to see if both sides are linear.
			optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0));
			optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1));
			if (left && right)
			{
				LinearExpression data = *left - *right;
				data[0] *= -1;
				Constraint c{move(data), _expr.name == "=", {}};
				if (!tryAddDirectBounds(c))
					state().fixedConstraints.emplace_back(move(c));
				cout << "Added as fixed constraint" << endl;
			}
			else
			{
				solAssert(false);
			}
		}
		else
			solAssert(false);
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
		if (isConditionalConstraint(left.variable) && isConditionalConstraint(right.variable))
		{
			// We cannot have more than one constraint per clause.
			// TODO Why?
			right = *parseLiteral(declareInternalVariable(true));
			addBooleanEquality(right, _expr.arguments.at(1));
		}
		state().clauses.emplace_back(Clause{vector<Literal>{left, right}});
	}
	else if (_expr.name == "not")
	{
		// TODO can we still try to add a fixed constraint?
		Literal l = negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0)));
		state().clauses.emplace_back(Clause{vector<Literal>{l}});
	}
	else if (_expr.name == "<=")
	{
		optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0));
		optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1));
		if (left && right)
		{
			LinearExpression data = *left - *right;
			data[0] *= -1;
			Constraint c{move(data), _expr.name == "=", {}};
			if (!tryAddDirectBounds(c))
				state().fixedConstraints.emplace_back(move(c));
		}
		else
		{
			solAssert(false);
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


pair<CheckResult, vector<string>> BooleanLPSolver::check(vector<Expression> const&)
{
	cout << "Solving boolean constraint system" << endl;
	cout << toString() << endl;
	cout << "--------------" << endl;

	if (state().infeasible)
		return make_pair(CheckResult::UNSATISFIABLE, vector<string>{});

	std::vector<std::string> booleanVariables;
	std::vector<Clause> clauses = state().clauses;
	SolvingState lpState;
	for (auto&& [index, bound]: state().bounds)
		resizeAndSet(lpState.bounds, index, bound);
	lpState.constraints = state().fixedConstraints;
	// TODO this way, it will result in a lot of gaps in both sets of variables.
	// should we compress them and store a mapping?
	// Is it even a problem if the indices overlap?
	for (auto&& [name, index]: state().variables)
		if (state().isBooleanVariable.at(index) || isConditionalConstraint(index))
			resizeAndSet(booleanVariables, index, name);
		else
			resizeAndSet(lpState.variableNames, index, name);

	//cout << "Boolean variables:" << joinHumanReadable(booleanVariables) << endl;
	//cout << "Running LP solver on fixed constraints." << endl;
	if (m_lpSolver.check(lpState).first == LPResult::Infeasible)
		return {CheckResult::UNSATISFIABLE, {}};

	auto theorySolver = [&](map<size_t, bool> const& _booleanAssignment) -> optional<Clause>
	{
		SolvingState lpStateToCheck = lpState;
		for (auto&& [constraintIndex, value]: _booleanAssignment)
		{
			if (!value || !state().conditionalConstraints.count(constraintIndex))
				continue;
			// "reason" is already stored for those constraints.
			Constraint const& constraint = state().conditionalConstraints.at(constraintIndex);
			solAssert(
				constraint.reasons.size() == 1 &&
				*constraint.reasons.begin() == constraintIndex
			);
			lpStateToCheck.constraints.emplace_back(constraint);
		}
		auto&& [result, modelOrReason] = m_lpSolver.check(move(lpStateToCheck));
		// We can only really use the result "infeasible". Everything else should be "sat".
		if (result == LPResult::Infeasible)
		{
			// TODO this could be the empty clause if the LP is already infeasible
			// with only the fixed constraints - run it beforehand!
			// TODO is it ok to ignore the non-constraint boolean variables here?
			Clause conflictClause;
			for (size_t constraintIndex: get<ReasonSet>(modelOrReason))
				conflictClause.emplace_back(Literal{false, constraintIndex});
			return conflictClause;
		}
		else
			return nullopt;
	};

	auto optionalModel = CDCL{move(booleanVariables), clauses, theorySolver}.solve();
	if (!optionalModel)
	{
		//cout << "==============> CDCL final result: unsatisfiable." << endl;
		return {CheckResult::UNSATISFIABLE, {}};
	}
	else
	{
		//cout << "==============> CDCL final result: SATisfiable / UNKNOWN." << endl;
		// TODO should be "unknown" later on
		//return {CheckResult::SATISFIABLE, {}};
		return {CheckResult::UNKNOWN, {}};
	}
}

string BooleanLPSolver::toString() const
{
	string result;

	result += "-- Fixed Constraints:\n";
	for (Constraint const& c: state().fixedConstraints)
		result += toString(c) + "\n";
	result += "-- Fixed Bounds:\n";
	for (auto&& [index, bounds]: state().bounds)
	{
		if (!bounds.lower && !bounds.upper)
			continue;
		if (bounds.lower)
			result += ::toString(*bounds.lower) + " <= ";
		result += variableName(index);
		if (bounds.upper)
			result += " <= " + ::toString(*bounds.upper);
		result += "\n";
	}
	result += "-- Clauses:\n";
	for (Clause const& c: state().clauses)
		result += toString(c);
	return result;
}

Expression BooleanLPSolver::declareInternalVariable(bool _boolean)
{
	string name = "$" + to_string(state().variables.size() + 1);
	declareVariable(name, _boolean);
	return smtutil::Expression(name, {}, _boolean ? SortProvider::boolSort : SortProvider::uintSort);
}

void BooleanLPSolver::declareVariable(string const& _name, bool _boolean)
{
	size_t index = state().variables.size() + 1;
	state().variables[_name] = index;
	resizeAndSet(state().isBooleanVariable, index, _boolean);
}

optional<Literal> BooleanLPSolver::parseLiteral(smtutil::Expression const& _expr)
{
	// TODO constanst true/false?

	if (_expr.arguments.empty())
	{
		if (isBooleanVariable(_expr.name))
			return Literal{
				true,
				state().variables.at(_expr.name)
			};
		else
			cout << "cannot encode " << _expr.name << " - not a boolean literal variable." << endl;
	}
	else if (_expr.name == "not")
		return negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0)));
	else if (_expr.name == "<=")
	{
		optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0));
		optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return {};

		LinearExpression data = *left - *right;
		data[0] *= -1;

		return Literal{true, addConditionalConstraint(Constraint{move(data), false, {}})};
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
	if (isConditionalConstraint(_lit.variable))
	{
		Constraint const& c = conditionalConstraint(_lit.variable);
		solAssert(!c.equality, "");

		// X > b
		// -x < -b
		// -x <= -b - 1

		Constraint negated = c;
		negated.data *= -1;
		negated.data[0] -= 1;
		negated.reasons.clear();
		return Literal{true, addConditionalConstraint(negated)};
	}
	else
		return ~_lit;
}

Literal BooleanLPSolver::parseLiteralOrReturnEqualBoolean(Expression const& _expr)
{
	if (_expr.sort->kind != Kind::Bool)
		cout << "expected bool: " << _expr.toString() << endl;
	solAssert(_expr.sort->kind == Kind::Bool);
	// TODO when can this fail?
	if (optional<Literal> literal = parseLiteral(_expr))
		return *literal;
	else
	{
		Literal newBoolean = *parseLiteral(declareInternalVariable(true));
		addBooleanEquality(newBoolean, _expr);
		return newBoolean;
	}
}

optional<LinearExpression> BooleanLPSolver::parseLinearSum(smtutil::Expression const& _expr)
{
	if (_expr.arguments.empty() || _expr.name == "*")
		return parseProduct(_expr);
	else if (_expr.name == "+" || _expr.name == "-")
	{
		optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0));
		optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1));
		if (!left || !right)
			return std::nullopt;
		return _expr.name == "+" ? *left + *right : *left - *right;
	}
	else if (_expr.name == "ite")
	{
		Expression result = declareInternalVariable(false);
		addAssertion(_expr.arguments.at(0) || (result == _expr.arguments.at(1)));
		addAssertion(!_expr.arguments.at(0) || (result == _expr.arguments.at(2)));
		return parseLinearSum(result);
	}
	else
	{
		cout << "Invalid operator " << _expr.name << endl;
		return std::nullopt;
	}
}

optional<LinearExpression> BooleanLPSolver::parseProduct(smtutil::Expression const& _expr) const
{
	if (_expr.arguments.empty())
		return parseFactor(_expr);
	else if (_expr.name == "*")
		// The multiplication ensures that only one of them can be a variable.
		return parseFactor(_expr.arguments.at(0)) * parseFactor(_expr.arguments.at(1));
	else
		return std::nullopt;
}

optional<LinearExpression> BooleanLPSolver::parseFactor(smtutil::Expression const& _expr) const
{
	solAssert(_expr.arguments.empty(), "");
	solAssert(!_expr.name.empty(), "");
	if ('0' <= _expr.name[0] && _expr.name[0] <= '9')
		return LinearExpression::constant(rational(bigint(_expr.name)));
	else if (_expr.name == "true")
		// TODO do we want to do this?
		return LinearExpression::constant(1);
	else if (_expr.name == "false")
		// TODO do we want to do this?
		return LinearExpression::constant(0);

	size_t index = state().variables.at(_expr.name);
	solAssert(index > 0, "");
	if (isBooleanVariable(index))
		return nullopt;
	return LinearExpression::factorForVariable(index, rational(bigint(1)));
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
			state().infeasible = true;
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
	if (!state().bounds[_index].upper || _value < *state().bounds[_index].upper)
		state().bounds[_index].upper = move(_value);
}

void BooleanLPSolver::addLowerBound(size_t _index, rational _value)
{
	// Lower bound must be at least zero.
	_value = max(_value, rational{});
	//cout << "adding " << variableName(_index) << " >= " << toString(_value) << endl;
	if (!state().bounds[_index].lower || _value > *state().bounds[_index].lower)
		state().bounds[_index].lower = move(_value);
}

size_t BooleanLPSolver::addConditionalConstraint(Constraint _constraint)
{
	string name = "$c" + to_string(state().variables.size() + 1);
	// It's not a boolean variable
	// TODO we actually have there kinds of variables and we should split them:
	//  - actual booleans (including internals)
	//  - conditional constraints
	//  - integers
	declareVariable(name, false);
	size_t index = state().variables.at(name);
	solAssert(_constraint.reasons.empty());
	_constraint.reasons.emplace(index);
	state().conditionalConstraints[index] = move(_constraint);
	return index;
}

void BooleanLPSolver::addBooleanEquality(Literal const& _left, smtutil::Expression const& _right)
{
	solAssert(_right.sort->kind == Kind::Bool);
	if (optional<Literal> right = parseLiteral(_right))
	{
		// includes: not, <=, <, >=, >, boolean variables.
		// a = b <=> (-a \/ b) /\ (a \/ -b)
		Literal negLeft = negate(_left);
		Literal negRight = negate(*right);
		state().clauses.emplace_back(Clause{vector<Literal>{negLeft, *right}});
		state().clauses.emplace_back(Clause{vector<Literal>{_left, negRight}});
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
		if (isConditionalConstraint(a.variable) && isConditionalConstraint(b.variable))
		{
			// We cannot have more than one constraint per clause.
			// TODO Why?
			b = *parseLiteral(declareInternalVariable(true));
			addBooleanEquality(b, _right.arguments.at(1));
		}

		if (_right.name == "and")
		{
			// a = and(x, y) <=> (-a \/ x) /\ ( -a \/ y) /\ (a \/ -x \/ -y)
			state().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a}});
			state().clauses.emplace_back(Clause{vector<Literal>{negate(_left), b}});
			state().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a), negate(b)}});
		}
		else if (_right.name == "or")
		{
			// a = or(x, y) <=> (-a \/ x \/ y) /\ (a \/ -x) /\ (a \/ -y)
			state().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a, b}});
			state().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a)}});
			state().clauses.emplace_back(Clause{vector<Literal>{_left, negate(b)}});
		}
		else if (_right.name == "=")
		{
			// l = eq(a, b) <=> (-l or -a or b) and (-l or a or -b) and (l or -a or -b) and (l or a or b)
			state().clauses.emplace_back(Clause{vector<Literal>{negate(_left), negate(a), b}});
			state().clauses.emplace_back(Clause{vector<Literal>{negate(_left), a, negate(b)}});
			state().clauses.emplace_back(Clause{vector<Literal>{_left, negate(a), negate(b)}});
			state().clauses.emplace_back(Clause{vector<Literal>{_left, a, b}});
		}
		else
			solAssert(false, "Unsupported operation: " + _right.name);
	}
}

/*
string BooleanLPSolver::toString(std::vector<SolvingState::Bounds> const& _bounds) const
{
	string result;
	for (auto&& [index, bounds]: _bounds | ranges::views::enumerate)
	{
		if (!bounds.lower && !bounds[1])
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
*/
string BooleanLPSolver::toString(Clause const& _clause) const
{
	vector<string> literals;
	for (Literal const& l: _clause)
		if (isBooleanVariable(l.variable))
			literals.emplace_back((l.positive ? "" : "!") + variableName(l.variable));
		else
		{
			solAssert(isConditionalConstraint(l.variable));
			solAssert(l.positive);
			literals.emplace_back(toString(conditionalConstraint(l.variable)));
		}
	return joinHumanReadable(literals, "  \\/  ") + "\n";
}

string BooleanLPSolver::toString(Constraint const& _constraint) const
{
	vector<string> line;
	for (auto&& [index, multiplier]: _constraint.data | ranges::views::enumerate)
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
	// TODO reasons?
	return
		joinHumanReadable(line, " + ") +
		(_constraint.equality ? "  = " : " <= ") +
		::toString(_constraint.data.front());
}

Constraint const& BooleanLPSolver::conditionalConstraint(size_t _index) const
{
	return state().conditionalConstraints.at(_index);
}

string BooleanLPSolver::variableName(size_t _index) const
{
	for (auto const& v: state().variables)
		if (v.second == _index)
			return v.first;
	return {};
}

bool BooleanLPSolver::isBooleanVariable(string const& _name) const
{
	if (!state().variables.count(_name))
		return false;
	size_t index = state().variables.at(_name);
	solAssert(index > 0, "");
	return isBooleanVariable(index);
}

bool BooleanLPSolver::isBooleanVariable(size_t _index) const
{
	return
		_index < state().isBooleanVariable.size() &&
		state().isBooleanVariable.at(_index);
}
