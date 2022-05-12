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
	if (_x == bigint(1) << 256)
		return "2**256";
	else if (_x == (bigint(1) << 256) - 1)
		return "2**256-1";
	else if (_x.denominator() == 1)
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
	solAssert(_sort && (_sort->kind == Kind::Int || _sort->kind == Kind::Real || _sort->kind == Kind::Bool), "");
	solAssert(!state().variables.count(name), "");
	// TODO store the actual kind (integer, real, bool)
	declareVariable(name, _sort->kind == Kind::Bool);
}


pair<CheckResult, vector<string>> BooleanLPSolver::check(vector<Expression> const&)
{
	cerr << "Solving boolean constraint system" << endl;
	cerr << toString() << endl;
	cerr << "--------------" << endl;

	if (state().infeasible)
	{
		cerr << "----->>>>> unsatisfiable" << endl;
		return make_pair(CheckResult::UNSATISFIABLE, vector<string>{});
	}

	std::vector<std::string> booleanVariables;
	std::vector<Clause> clauses = state().clauses;

	// TODO we start building up a new set of solver
	// for each query, but we should also keep some
	// kind of cache across queries.
	std::vector<std::pair<size_t, LPSolver>> lpSolvers;
	lpSolvers.emplace_back(0, LPSolver{});
	LPSolver& lpSolver = lpSolvers.back().second;

	for (auto&& [index, bound]: state().bounds)
	{
		if (bound.lower)
			lpSolver.addLowerBound(index, *bound.lower);
		if (bound.upper)
			lpSolver.addUpperBound(index, *bound.upper);
	}
	for (Constraint const& c: state().fixedConstraints)
		lpSolver.addConstraint(c);

	// TODO this way, it will result in a lot of gaps in both sets of variables.
	// should we compress them and store a mapping?
	// Is it even a problem if the indices overlap?
	for (auto&& [name, index]: state().variables)
		if (state().isBooleanVariable.at(index) || isConditionalConstraint(index))
			resizeAndSet(booleanVariables, index, name);
		else
			lpSolver.setVariableName(index, name);

	if (lpSolver.check().first == LPResult::Infeasible)
	{
		cerr << "----->>>>> unsatisfiable" << endl;
		return {CheckResult::UNSATISFIABLE, {}};
	}

	auto theorySolver = [&](size_t _trailSize, map<size_t, bool> const& _newBooleanAssignment) -> optional<Clause>
	{
		lpSolvers.emplace_back(_trailSize, LPSolver(lpSolvers.back().second));

		for (auto&& [constraintIndex, value]: _newBooleanAssignment)
		{
			if (!value || !state().conditionalConstraints.count(constraintIndex))
				continue;
			// "reason" is already stored for those constraints.
			Constraint const& constraint = state().conditionalConstraints.at(constraintIndex);
			lpSolvers.back().second.addConstraint(constraint, constraintIndex);
		}
		auto&& [result, reasonSet] = lpSolvers.back().second.check();
		// We can only really use the result "infeasible". Everything else should be "sat".
		if (result == LPResult::Infeasible)
		{
			// TODO is it ok to ignore the non-constraint boolean variables here?
			Clause conflictClause;
			for (size_t constraintIndex: reasonSet)
				conflictClause.emplace_back(Literal{false, constraintIndex});
			return conflictClause;
		}
		else
			return nullopt;
	};
	auto backtrackNotify = [&](size_t _trailSize)
	{
		while (lpSolvers.back().first > _trailSize)
			lpSolvers.pop_back();
	};

	auto optionalModel = CDCL{move(booleanVariables), clauses, theorySolver, backtrackNotify}.solve();
	if (!optionalModel)
	{
		cerr << "==============> CDCL final result: unsatisfiable." << endl;
		return {CheckResult::UNSATISFIABLE, {}};
	}
	else
	{
		cerr << "==============> CDCL final result: SATisfiable / UNKNOWN." << endl;
		// TODO should be "unknown" later on
		return {CheckResult::SATISFIABLE, {}};
		//return {CheckResult::UNKNOWN, {}};
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
			result += bounds.lower->toString() + " <= ";
		result += variableName(index);
		if (bounds.upper)
			result += " <= " + bounds.upper->toString();
		result += "\n";
	}
	result += "-- Clauses:\n";
	for (Clause const& c: state().clauses)
		result += toString(c);
	return result;
}

void BooleanLPSolver::addAssertion(Expression const& _expr, map<string, size_t> _letBindings)
{
	cerr << "adding assertion" << endl;
	cerr << " - " << _expr.toString() << endl;
	solAssert(_expr.sort->kind == Kind::Bool);
	if (_expr.arguments.empty())
	{
		size_t index = _letBindings.count(_expr.name) ? _letBindings.at(_expr.name) : state().variables.at(_expr.name);
		solAssert(isBooleanVariable(index));
		state().clauses.emplace_back(Clause{Literal{true, index }});
	}
	else if (_expr.name == "let")
	{
		addLetBindings(_expr, _letBindings);
		addAssertion(_expr.arguments.back(), move(_letBindings));
	}
	else if (_expr.name == "=")
	{
		solAssert(_expr.arguments.size() == 2);
		solAssert(_expr.arguments.at(0).sort->kind == _expr.arguments.at(1).sort->kind);
		if (_expr.arguments.at(0).sort->kind == Kind::Bool)
		{
			if (_expr.arguments.at(0).arguments.empty() && isBooleanVariable(_expr.arguments.at(0).name))
				addBooleanEquality(*parseLiteral(_expr.arguments.at(0), _letBindings), _expr.arguments.at(1), _letBindings);
			else if (_expr.arguments.at(1).arguments.empty() && isBooleanVariable(_expr.arguments.at(1).name))
				addBooleanEquality(*parseLiteral(_expr.arguments.at(1), _letBindings), _expr.arguments.at(0), _letBindings);
			else
			{
				Literal newBoolean = *parseLiteral(declareInternalVariable(true), {});
				addBooleanEquality(newBoolean, _expr.arguments.at(0), _letBindings);
				addBooleanEquality(newBoolean, _expr.arguments.at(1), _letBindings);
			}
		}
		else if (_expr.arguments.at(0).sort->kind == Kind::Int || _expr.arguments.at(0).sort->kind == Kind::Real)
		{
			// Try to see if both sides are linear.
			optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0), _letBindings);
			optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1), _letBindings);
			if (left && right)
			{
				LinearExpression data = *left - *right;
				data[0] *= -1;
				Constraint c{move(data), Constraint::EQUAL};
				if (!tryAddDirectBounds(c))
					state().fixedConstraints.emplace_back(move(c));
				cerr << "Added as fixed constraint" << endl;
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
		for (auto const& arg: _expr.arguments)
			addAssertion(arg, _letBindings);
	else if (_expr.name == "or")
	{
		vector<Literal> literals;
		// We could try to parse a full clause here instead.
		for (auto const& arg: _expr.arguments)
			literals.emplace_back(parseLiteralOrReturnEqualBoolean(arg, _letBindings));
		state().clauses.emplace_back(Clause{move(literals)});
	}
	else if (_expr.name == "not")
	{
		solAssert(_expr.arguments.size() == 1);
		// TODO can we still try to add a fixed constraint?
		Literal l = negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0), move(_letBindings)));
		state().clauses.emplace_back(Clause{vector<Literal>{l}});
	}
	else if (_expr.name == "=>")
		addAssertion(!_expr.arguments.at(0) || _expr.arguments.at(1), move(_letBindings));
	else if (_expr.name == "<=" || _expr.name == "<")
	{
		optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0), _letBindings);
		optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1), _letBindings);
		solAssert(left && right);
		LinearExpression data = *left - *right;
		data[0] *= -1;
		// TODO if the type is integer, transform x < y into x <= y - 1
		Constraint c{move(data), _expr.name == "<=" ? Constraint::LESS_OR_EQUAL : Constraint::LESS_THAN};
		if (!tryAddDirectBounds(c))
			state().fixedConstraints.emplace_back(move(c));
	}
	else if (_expr.name == ">=")
		addAssertion(_expr.arguments.at(1) <= _expr.arguments.at(0), move(_letBindings));
	else if (_expr.name == ">")
		addAssertion(_expr.arguments.at(1) < _expr.arguments.at(0), move(_letBindings));
	else
	{
		cerr << "Unknown operator " << _expr.name << endl;
		solAssert(false);
	}
}


Expression BooleanLPSolver::declareInternalVariable(bool _boolean)
{
	string name = "$" + to_string(state().variables.size() + 1);
	declareVariable(name, _boolean);
	// TODO also support integer
	return smtutil::Expression(name, {}, _boolean ? SortProvider::boolSort : SortProvider::realSort);
}

void BooleanLPSolver::declareVariable(string const& _name, bool _boolean)
{
	size_t index = state().variables.size() + 1;
	state().variables[_name] = index;
	resizeAndSet(state().isBooleanVariable, index, _boolean);
}

void BooleanLPSolver::addLetBindings(Expression const& _let, map<string, size_t>& _letBindings)
{
	map<string, size_t> newBindings;
	solAssert(_let.name == "let");
	for (size_t i = 0; i < _let.arguments.size() - 1; i++)
	{
		Expression binding = _let.arguments.at(i);
		bool isBool = binding.arguments.at(0).sort->kind == Kind::Bool;
		Expression var = declareInternalVariable(isBool);
		newBindings.insert({binding.name, state().variables.at(var.name)});
		addAssertion(var == binding.arguments.at(0), _letBindings);
	}
	for (auto&& [name, varIndex]: newBindings)
		_letBindings.insert({name, varIndex});
}

optional<Literal> BooleanLPSolver::parseLiteral(smtutil::Expression const& _expr, map<string, size_t> _letBindings)
{
	// TODO constanst true/false?

	if (_expr.name == "let")
	{
		addLetBindings(_expr, _letBindings);
		return parseLiteral(_expr.arguments.back(), move(_letBindings));
	}

	if (_expr.arguments.empty())
	{
		size_t index = _letBindings.count(_expr.name) ? _letBindings.at(_expr.name) : state().variables.at(_expr.name);
		solAssert(isBooleanVariable(index));
		return Literal{true, index};
	}
	else if (_expr.name == "not")
		return negate(parseLiteralOrReturnEqualBoolean(_expr.arguments.at(0), move(_letBindings)));
	else if (_expr.name == "<=" || _expr.name == "<" || _expr.name == "=")
	{
		optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0), _letBindings);
		optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1), _letBindings);
		if (!left || !right)
			return {};

		// TODO if the type is int, use x < y -> x <= y - 1
		LinearExpression data = *left - *right;
		data[0] *= -1;

		Constraint::Kind kind =
			_expr.name == "<=" ? Constraint::LESS_OR_EQUAL :
			_expr.name == "<" ? Constraint::LESS_THAN :
			Constraint::EQUAL;
		return Literal{true, addConditionalConstraint(Constraint{move(data), kind})};
	}
	else if (_expr.name == ">=")
		return parseLiteral(_expr.arguments.at(1) <= _expr.arguments.at(0), move(_letBindings));
	else if (_expr.name == ">")
		return parseLiteral(_expr.arguments.at(1) < _expr.arguments.at(0), move(_letBindings));

	return {};
}

Literal BooleanLPSolver::negate(Literal const& _lit)
{
	if (isConditionalConstraint(_lit.variable))
	{
		Constraint const& c = conditionalConstraint(_lit.variable);
		if (c.kind == Constraint::EQUAL)
		{
			// X = b


			/* This is the integer case
			// X <= b - 1
			Constraint le = c;
			le.equality = false;
			le.data[0] -= 1;
			Literal leL{true, addConditionalConstraint(le)};

			// X >= b + 1
			// -X <= -b - 1
			Constraint ge = c;
			ge.equality = false;
			ge.data *= -1;
			ge.data[0] -= 1;
			Literal geL{true, addConditionalConstraint(ge)};

			*/

			// X < b
			Constraint lt = c;
			lt.kind = Constraint::LESS_THAN;
			Literal ltL{true, addConditionalConstraint(lt)};

			// X > b <=> -X < -b
			Constraint gt = c;
			gt.kind = Constraint::LESS_THAN;
			gt.data *= -1;
			Literal gtL{true, addConditionalConstraint(gt)};

			Literal equalBoolean = *parseLiteral(declareInternalVariable(true), {});
			// a = or(x, y) <=> (-a \/ x \/ y) /\ (a \/ -x) /\ (a \/ -y)
			state().clauses.emplace_back(Clause{vector<Literal>{negate(equalBoolean), ltL, gtL}});
			state().clauses.emplace_back(Clause{vector<Literal>{equalBoolean, negate(ltL)}});
			state().clauses.emplace_back(Clause{vector<Literal>{equalBoolean, negate(gtL)}});
			return equalBoolean;
		}
		else
		{
			/* This is the integer case
			// -x < -b
			// -x <= -b - 1

			Constraint negated = c;
			negated.data *= -1;
			negated.data[0] -= 1;
			*/

			// !(X <= b) <=> X > b <=> -X < -b
			// !(X < b) <=> X >= b <=> -X <= -b
			Constraint negated = c;
			negated.data *= -1;
			negated.kind = c.kind == Constraint::LESS_THAN ? Constraint::LESS_OR_EQUAL : Constraint::LESS_THAN;
			return Literal{true, addConditionalConstraint(negated)};
		}
	}
	else
		return ~_lit;
}

Literal BooleanLPSolver::parseLiteralOrReturnEqualBoolean(Expression const& _expr, map<string, size_t> _letBindings)
{
	if (_expr.sort->kind != Kind::Bool)
		cerr << "expected bool: " << _expr.toString() << endl;
	solAssert(_expr.sort->kind == Kind::Bool);
	// TODO when can this fail?
	if (optional<Literal> literal = parseLiteral(_expr, _letBindings))
		return *literal;
	else
	{
		Literal newBoolean = *parseLiteral(declareInternalVariable(true), _letBindings);
		addBooleanEquality(newBoolean, _expr, _letBindings);
		return newBoolean;
	}
}

optional<LinearExpression> BooleanLPSolver::parseLinearSum(smtutil::Expression const& _expr, map<string, size_t> _letBindings)
{
	if (_expr.name == "let")
	{
		addLetBindings(_expr, _letBindings);
		return parseLinearSum(_expr.arguments.back(), move(_letBindings));
	}

	if (_expr.arguments.empty())
		return parseFactor(_expr, move(_letBindings));
	else if (_expr.name == "+" || (_expr.name == "-" && _expr.arguments.size() == 2))
	{
		optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0), _letBindings);
		optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1), _letBindings);
		if (!left || !right)
			return std::nullopt;
		return _expr.name == "+" ? *left + *right : *left - *right;
	}
	else if (_expr.name == "-" && _expr.arguments.size() == 1)
	{
		optional<LinearExpression> arg = parseLinearSum(_expr.arguments.at(0), move(_letBindings));
		if (arg)
			return LinearExpression::constant(0) - *arg;
		else
			return std::nullopt;
	}
	else if (_expr.name == "*")
		// This will result in nullopt unless one of them is a constant.
		return parseLinearSum(_expr.arguments.at(0), _letBindings) * parseLinearSum(_expr.arguments.at(1), _letBindings);
	else if (_expr.name == "/")
	{
		optional<LinearExpression> left = parseLinearSum(_expr.arguments.at(0), _letBindings);
		optional<LinearExpression> right = parseLinearSum(_expr.arguments.at(1), move(_letBindings));
		if (!left || !right || !right->isConstant())
			return std::nullopt;
		*left /= right->get(0);
		return left;
	}
	else if (_expr.name == "ite")
	{
		Expression result = declareInternalVariable(false);
		addAssertion(!_expr.arguments.at(0) || (result == _expr.arguments.at(1)), _letBindings);
		addAssertion(_expr.arguments.at(0) || (result == _expr.arguments.at(2)), _letBindings);
		return parseLinearSum(result, {});
	}
	else
	{
		cerr << "Invalid operator " << _expr.name << endl;
		return std::nullopt;
	}
}


optional<LinearExpression> BooleanLPSolver::parseFactor(smtutil::Expression const& _expr, map<string, size_t> _letBindings) const
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

	size_t index = _letBindings.count(_expr.name) ? _letBindings.at(_expr.name) : state().variables.at(_expr.name);
	solAssert(index > 0, "");
	if (isBooleanVariable(index))
		return nullopt;
	return LinearExpression::factorForVariable(index, rational(bigint(1)));
}

bool BooleanLPSolver::tryAddDirectBounds(Constraint const& _constraint)
{
	auto nonzero = _constraint.data.enumerateTail() | ranges::views::filter(
		[](std::pair<size_t, rational> const& _x) { return !!_x.second; }
	);
	// TODO we can exit early on in the loop above.
	if (ranges::distance(nonzero) > 1)
		return false;

	//cerr << "adding direct bound." << endl;
	if (ranges::distance(nonzero) == 0)
	{
		// 0 < b or 0 <= b or 0 = b
		if (
			(_constraint.kind == Constraint::LESS_THAN && _constraint.data.front() <= 0) ||
			(_constraint.kind == Constraint::LESS_OR_EQUAL && _constraint.data.front() < 0) ||
			(_constraint.kind == Constraint::EQUAL && _constraint.data.front() != 0)
		)
		{
//			cerr << "SETTING INF" << endl;
			state().infeasible = true;
		}
	}
	else
	{
		auto&& [varIndex, factor] = nonzero.front();
		// a * x <= b or a * x < b or a * x = b

		RationalWithDelta bound = _constraint.data[0];
		if (_constraint.kind == Constraint::LESS_THAN)
			bound -= RationalWithDelta::delta();
		bound /= factor;
		if (factor > 0 || _constraint.kind == Constraint::EQUAL)
			addUpperBound(varIndex, bound);
		if (factor < 0 || _constraint.kind == Constraint::EQUAL)
			addLowerBound(varIndex, bound);
	}
	return true;
}

void BooleanLPSolver::addUpperBound(size_t _index, RationalWithDelta _value)
{
	//cerr << "adding " << variableName(_index) << " <= " << toString(_value) << endl;
	if (!state().bounds[_index].upper || _value < *state().bounds[_index].upper)
		state().bounds[_index].upper = move(_value);
}

void BooleanLPSolver::addLowerBound(size_t _index, RationalWithDelta _value)
{
	//cerr << "adding " << variableName(_index) << " >= " << toString(_value) << endl;
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
	state().conditionalConstraints[index] = move(_constraint);
	return index;
}

void BooleanLPSolver::addBooleanEquality(Literal const& _left, smtutil::Expression const& _right, map<string, size_t> _letBindings)
{
	solAssert(_right.sort->kind == Kind::Bool);
	if (optional<Literal> right = parseLiteral(_right, _letBindings))
	{
		// includes: not, <=, <, >=, >, boolean variables.
		// a = b <=> (-a \/ b) /\ (a \/ -b)
		Literal negLeft = negate(_left);
		Literal negRight = negate(*right);
		state().clauses.emplace_back(Clause{vector<Literal>{negLeft, *right}});
		state().clauses.emplace_back(Clause{vector<Literal>{_left, negRight}});
	}
	// TODO This parses twice
	else if (_right.name == "=" && parseLinearSum(_right.arguments.at(0), _letBindings) && parseLinearSum(_right.arguments.at(1), _letBindings))
		// a = (x = y)  <=>  a = (x <= y && x >= y)
		addBooleanEquality(
			_left,
			_right.arguments.at(0) <= _right.arguments.at(1) &&
			_right.arguments.at(1) <= _right.arguments.at(0),
			move(_letBindings)
		);
	else
	{
		Literal a = parseLiteralOrReturnEqualBoolean(_right.arguments.at(0), _letBindings);
		Literal b = parseLiteralOrReturnEqualBoolean(_right.arguments.at(1), _letBindings);
		/*
		if (isConditionalConstraint(a.variable) && isConditionalConstraint(b.variable))
		{
			// We cannot have more than one constraint per clause.
			// TODO Why?
			b = *parseLiteral(declareInternalVariable(true));
			addBooleanEquality(b, _right.arguments.at(1));
		}*/

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
	for (auto&& [index, multiplier]: _constraint.data.enumerate())
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
		(
			_constraint.kind == Constraint::EQUAL ? "  = " :
			_constraint.kind == Constraint::LESS_OR_EQUAL ? " <= " :
			" <  "
		) +
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
