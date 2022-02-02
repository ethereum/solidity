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

#include <libsolutil/LP.h>
#include <libsolutil/LinearExpression.h>
#include <libsolutil/CommonIO.h>
#include <libsmtutil/Sorts.h>
#include <libsolutil/StringUtils.h>
#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::smtutil;
using namespace solidity::util;


namespace solidity::util::test
{

class LPTestFramework
{
public:
	LPTestFramework()
	{
		m_solvingState.variableNames.emplace_back("");
	}

	LinearExpression constant(rational _value)
	{
		return LinearExpression::factorForVariable(0, _value);
	}

	LinearExpression variable(string const& _name)
	{
		return LinearExpression::factorForVariable(variableIndex(_name), 1);
	}

	/// Adds the constraint "_lhs <= _rhs".
	void addLEConstraint(LinearExpression _lhs, LinearExpression _rhs)
	{
		_lhs -= _rhs;
		_lhs[0] = -_lhs[0];
		m_solvingState.constraints.push_back({move(_lhs), false});
	}

	void addLEConstraint(LinearExpression _lhs, rational _rhs)
	{
		addLEConstraint(move(_lhs), constant(_rhs));
	}

	/// Adds the constraint "_lhs = _rhs".
	void addEQConstraint(LinearExpression _lhs, LinearExpression _rhs)
	{
		_lhs -= _rhs;
		_lhs[0] = -_lhs[0];
		m_solvingState.constraints.push_back({move(_lhs), true});
	}

	void addLowerBound(string _variable, rational _value)
	{
		size_t index = variableIndex(_variable);
		if (index >= m_solvingState.bounds.size())
			m_solvingState.bounds.resize(index + 1);
		m_solvingState.bounds.at(index).lower = _value;
	}

	void addUpperBound(string _variable, rational _value)
	{
		size_t index = variableIndex(_variable);
		if (index >= m_solvingState.bounds.size())
			m_solvingState.bounds.resize(index + 1);
		m_solvingState.bounds.at(index).upper = _value;
	}

	void feasible(vector<pair<string, rational>> const& _solution)
	{
		auto [result, model] = m_solver.check(m_solvingState);
		BOOST_REQUIRE(result == LPResult::Feasible);
		for (auto const& [var, value]: _solution)
			BOOST_CHECK_MESSAGE(
				value == model.at(var),
				var + " = "s + ::toString(model.at(var)) + " (expected " + ::toString(value) + ")"
			);
	}

	void infeasible()
	{
		auto [result, model] = m_solver.check(m_solvingState);
		BOOST_CHECK(result == LPResult::Infeasible);
	}

protected:
	size_t variableIndex(string const& _name)
	{
		if (m_solvingState.variableNames.empty())
			m_solvingState.variableNames.emplace_back("");
		auto index = findOffset(m_solvingState.variableNames, _name);
		if (!index)
		{
			index = m_solvingState.variableNames.size();
			m_solvingState.variableNames.emplace_back(_name);
		}
		return *index;
	}

	LPSolver m_solver;
	SolvingState m_solvingState;
};


BOOST_FIXTURE_TEST_SUITE(LP, LPTestFramework, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(basic)
{
	auto x = variable("x");
	addLEConstraint(2 * x, 10);
	feasible({{"x", 5}});
}

BOOST_AUTO_TEST_CASE(not_linear_independent)
{
	addLEConstraint(2 * variable("x"), 10);
	addLEConstraint(4 * variable("x"), 20);
	feasible({{"x", 5}});
}

BOOST_AUTO_TEST_CASE(two_vars)
{
	addLEConstraint(variable("y"), 3);
	addLEConstraint(variable("x"), 10);
	addLEConstraint(variable("x") + variable("y"), 4);
	feasible({{"x", 1}, {"y", 3}});
}

BOOST_AUTO_TEST_CASE(one_le_the_other)
{
	addLEConstraint(variable("x") + constant(2), variable("y") - constant(1));
	feasible({{"x", 0}, {"y", 3}});
}

BOOST_AUTO_TEST_CASE(factors)
{
	auto x = variable("x");
	auto y = variable("y");
	addLEConstraint(2 * y, 3);
	addLEConstraint(16 * x, 10);
	addLEConstraint(x + y, 4);
	feasible({{"x", rational(5) / 8}, {"y", rational(3) / 2}});
}

BOOST_AUTO_TEST_CASE(cache)
{
	// This should use the cache already for the second part of the problem.
	// We cannot really test that the cache has been used, but we can test
	// that it results in the same value.
	auto x = variable("x");
	auto y = variable("y");
	addLEConstraint(2 * y, 3);
	addLEConstraint(2 * x, 3);
	feasible({{"x", rational(3) / 2}, {"y", rational(3) / 2}});
	feasible({{"x", rational(3) / 2}, {"y", rational(3) / 2}});
}

BOOST_AUTO_TEST_CASE(bounds)
{
	addUpperBound("x", 200);
	feasible({{"x", 200}});

	addLEConstraint(variable("x"), 100);
	feasible({{"x", 100}});

	addLEConstraint(constant(5), variable("x"));
	feasible({{"x", 100}});

	addLowerBound("x", 20);
	feasible({{"x", 100}});
	addLowerBound("x", 25);
	feasible({{"x", 100}});

	addUpperBound("x", 20);
	infeasible();
}

BOOST_AUTO_TEST_CASE(bounds2)
{
	addLowerBound("x", 200);
	addUpperBound("x", 250);
	addLowerBound("y", 2);
	addUpperBound("y", 3);
	feasible({{"x", 250}, {"y", 3}});

	addLEConstraint(variable("y"), variable("x"));
	feasible({{"x", 250}, {"y", 3}});

	addEQConstraint(variable("y") + constant(231), variable("x"));
	feasible({{"x", 234}, {"y", 3}});

	addEQConstraint(variable("y") + constant(10), variable("x") - variable("z"));
	feasible({{"x", 234}, {"y", 3}});

	addEQConstraint(variable("z") + variable("x"), constant(2));
	infeasible();
}

BOOST_AUTO_TEST_CASE(lower_bound)
{
	addLEConstraint(constant(1), variable("y"));
	addLEConstraint(variable("x"), constant(10));
	addLEConstraint(2 * variable("x") + variable("y"), 2);
	feasible({{"x", 0}, {"y", 2}});
}

BOOST_AUTO_TEST_CASE(check_infeasible)
{
	addLEConstraint(variable("x"), 3);
	addLEConstraint(constant(5), variable("x"));
	infeasible();
}

BOOST_AUTO_TEST_CASE(unbounded1)
{
	addLEConstraint(constant(2), variable("x"));
	feasible({{"x", 2}});
}

BOOST_AUTO_TEST_CASE(unbounded2)
{
	auto x = variable("x");
	auto y = variable("y");
	addLEConstraint(constant(2), x + y);
	addLEConstraint(x, 10);
	feasible({{"x", 10}, {"y", 0}});
}

BOOST_AUTO_TEST_CASE(unbounded3)
{
	addLEConstraint(constant(0) - variable("x") - variable("y"), constant(10));
	feasible({{"x", 0}, {"y", 0}});

	addLEConstraint(constant(0) - variable("x"), constant(10));
	feasible({{"x", 0}, {"y", 0}});

	addEQConstraint(variable("y") + constant(3), variable("x"));
	feasible({{"x", 3}, {"y", 0}});

	addLEConstraint(variable("y") + variable("x"), constant(2));
	infeasible();
}


BOOST_AUTO_TEST_CASE(equal)
{
	auto x = variable("x");
	auto y = variable("y");
	addEQConstraint(x, y + constant(10));
	addLEConstraint(x, 20);
	feasible({{"x", 20}, {"y", 10}});
}


BOOST_AUTO_TEST_CASE(equal_constant)
{
	auto x = variable("x");
	auto y = variable("y");
	addLEConstraint(x, y);
	addEQConstraint(y, constant(5));
	feasible({{"x", 5}, {"y", 5}});
}

BOOST_AUTO_TEST_CASE(linear_dependent)
{
	auto x = variable("x");
	auto y = variable("y");
	auto z = variable("z");
	addLEConstraint(x, 5);
	addLEConstraint(2 * y, 10);
	addLEConstraint(3 * z, 15);
	// Here, they should be split into three independent problems.
	feasible({{"x", 5}, {"y", 5}, {"z", 5}});

	addLEConstraint((x + y) + z, 100);
	feasible({{"x", 5}, {"y", 5}, {"z", 5}});

	addLEConstraint((x + y) + z, 2);
	feasible({{"x", 2}, {"y", 0}, {"z", 0}});

	addLEConstraint(constant(2), (x + y) + z);
	feasible({{"x", 2}, {"y", 0}, {"z", 0}});

	addEQConstraint(constant(2), (x + y) + z);
	feasible({{"x", 2}, {"y", 0}, {"z", 0}});
}



BOOST_AUTO_TEST_SUITE_END()

}
