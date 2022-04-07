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
	void addLEConstraint(LinearExpression _lhs, LinearExpression _rhs, optional<size_t> _reason = {})
	{
		_lhs -= _rhs;
		_lhs[0] = -_lhs[0];
		m_solver.addConstraint({move(_lhs), false}, move(_reason));
	}

	void addLEConstraint(LinearExpression _lhs, rational _rhs)
	{
		addLEConstraint(move(_lhs), constant(_rhs));
	}

	/// Adds the constraint "_lhs = _rhs".
	void addEQConstraint(LinearExpression _lhs, LinearExpression _rhs, optional<size_t> _reason = {})
	{
		_lhs -= _rhs;
		_lhs[0] = -_lhs[0];
		m_solver.addConstraint({move(_lhs), true}, move(_reason));
	}

	void addLowerBound(string _variable, rational _value)
	{
		m_solver.addLowerBound(variableIndex(_variable), _value);
	}

	void addUpperBound(string _variable, rational _value)
	{
		m_solver.addUpperBound(variableIndex(_variable), _value);
	}

	void feasible(vector<pair<string, rational>> const& _solution)
	{
		auto [result, reasonSet] = m_solver.check();
		BOOST_REQUIRE(result == LPResult::Feasible);
		Model model = m_solver.model();
		for (auto const& [var, value]: _solution)
		{
			BOOST_CHECK_MESSAGE(
				model.count(var),
				"Variable " + var + " not found in model."
			);
			BOOST_CHECK_MESSAGE(
				value == model.at(var),
				var + " = "s + ::toString(model.at(var)) + " (expected " + ::toString(value) + ")"
			);
		}
	}

	void infeasible(set<size_t> _reason = {})
	{
		auto [result, reasonSet] = m_solver.check();
		BOOST_REQUIRE(result == LPResult::Infeasible);
		BOOST_CHECK_MESSAGE(reasonSet == _reason, "Reasons are different");
	}

protected:
	size_t variableIndex(string const& _name)
	{
		if (!m_variableIndices.count(_name))
		{
			size_t index = 1 + m_variableIndices.size();
			m_solver.setVariableName(index, _name);
			m_variableIndices[_name] = index;
		}
		return m_variableIndices.at(_name);
	}

	LPSolver m_solver;
	map<string, size_t> m_variableIndices;
};


BOOST_FIXTURE_TEST_SUITE(LP, LPTestFramework, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(empty)
{
	feasible({});
}

BOOST_AUTO_TEST_CASE(basic)
{
	auto x = variable("x");
	addLEConstraint(2 * x, 10);
	feasible({{"x", 0}});
}

BOOST_AUTO_TEST_CASE(not_linear_independent)
{
	addLEConstraint(2 * variable("x"), 10);
	addLEConstraint(4 * variable("x"), 20);
	feasible({{"x", 0}});
}

BOOST_AUTO_TEST_CASE(not_linear_independent_eq)
{
	addLEConstraint(2 * variable("x"), 10);
	addEQConstraint(4 * variable("x"), constant(20));
	feasible({{"x", 5}});
}

BOOST_AUTO_TEST_CASE(two_vars)
{
	addLEConstraint(variable("y"), 3);
	addLowerBound("x", 10);
	addLEConstraint(variable("x") + variable("y"), 4);
	feasible({{"x", 10}, {"y", -6}});
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
	addLEConstraint(constant(2), x + y);
	feasible({{"x", rational(5) / 8}, {"y", rational(11) / 8}});
}

BOOST_AUTO_TEST_CASE(cache)
{
	// This should use the cache already for the second part of the problem.
	// We cannot really test that the cache has been used, but we can test
	// that it results in the same value.
	auto x = variable("x");
	auto y = variable("y");
	addLEConstraint(constant(3), 2 * y);
	addLEConstraint(constant(3), 2 * x);
	feasible({{"x", rational(3) / 2}, {"y", rational(3) / 2}});
	feasible({{"x", rational(3) / 2}, {"y", rational(3) / 2}});
}

BOOST_AUTO_TEST_CASE(bounds)
{
	addUpperBound("x", 200);
	feasible({{"x", 0}});

	addLEConstraint(variable("x"), 100);
	feasible({{"x", 0}});

	addLEConstraint(constant(5), variable("x"));
	feasible({{"x", 5}});

	addLowerBound("x", 20);
	feasible({{"x", 20}});
	addLowerBound("x", 25);
	feasible({{"x", 25}});
	addLowerBound("x", 21);
	feasible({{"x", 25}});

	addUpperBound("x", 26);
	feasible({{"x", 25}});
	addUpperBound("x", 28);
	feasible({{"x", 25}});
	addUpperBound("x", 20);
	infeasible();
}

BOOST_AUTO_TEST_CASE(bounds2)
{
	addLowerBound("x", 200);
	addUpperBound("x", 250);
	addLowerBound("y", 2);
	addUpperBound("y", 3);
	feasible({{"x", 200}, {"y", 2}});

	addLEConstraint(variable("y"), variable("x"));
	feasible({{"x", 200}, {"y", 2}});

	addEQConstraint(variable("y") + constant(231), variable("x"));
	cout << m_solver.toString() << endl;
	feasible({{"x", 233}, {"y", 2}});
/*
	addEQConstraint(variable("y") + constant(10), variable("x") - variable("z"));
	feasible({{"x", 234}, {"y", 3}, {"z", 0}});

	addEQConstraint(variable("z") + variable("x"), constant(2));
	infeasible();*/
}

BOOST_AUTO_TEST_CASE(lower_bound)
{
	addLEConstraint(constant(1), variable("y"));
	addLEConstraint(variable("x"), constant(10));
	addLEConstraint(2 * variable("x") + variable("y"), 2);
	feasible({{"x", 0}, {"y", 1}});
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
	feasible({{"x", 2}, {"y", 0}});
}

BOOST_AUTO_TEST_CASE(unbounded3)
{
	addLowerBound("y", 0);
	addLowerBound("x", 0);
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
	feasible({{"x", 10}, {"y", 0}});
}


BOOST_AUTO_TEST_CASE(equal_constant)
{
	auto x = variable("x");
	auto y = variable("y");
	addLowerBound("x", 5);
	addLEConstraint(x, y);
	addEQConstraint(y, constant(5));
	feasible({{"x", 5}, {"y", 5}});
}

BOOST_AUTO_TEST_CASE(all_equality)
{
	auto x = variable("x");
	auto y = variable("y");
	addEQConstraint(-6 * x - 6 * y, constant(8));
	feasible({{"x", -rational(4) / 3}, {"y", 0}});
}

BOOST_AUTO_TEST_CASE(negative_values)
{
	auto x = variable("x");
	auto y = variable("y");
	addEQConstraint(-2 * x, constant(8));
	addLowerBound("y", -2);
	addUpperBound("y", -1);
	feasible({{"x", -4}, {"y", -1}});
}

BOOST_AUTO_TEST_CASE(basic_basic)
{
	auto x = variable("x");
	addLEConstraint(x, constant(5));
	feasible({{"x", 0}});
	addEQConstraint(x, constant(5));
	feasible({{"x", 5}});
}

BOOST_AUTO_TEST_CASE(linear_dependent)
{
	auto x = variable("x");
	auto y = variable("y");
	auto z = variable("z");
	addLEConstraint(x, -5);
	addLEConstraint(2 * y, -10);
	addLEConstraint(3 * z, -15);
	// Here, they should be split into three independent problems.
	feasible({{"x", -5}, {"y", -5}, {"z", -5}});

	addLEConstraint((x + y) + z, 100);
	feasible({{"x", -5}, {"y", -5}, {"z", -5}});

	addLEConstraint((x + y) + z, -1);
	feasible({{"x", -5}, {"y", -5}, {"z", -5}});

	addLEConstraint(constant(-20), (x + y) + z);
	feasible({{"x", -5}, {"y", -5}, {"z", -5}});

	addEQConstraint(constant(-18), (x + y) + z);
	feasible({{"x", -8}, {"y", -5}, {"z", -5}});
}

BOOST_AUTO_TEST_CASE(reasons_simple)
{
	auto x = variable("x");
	addLEConstraint(2 * x, constant(20), {0});
	addLowerBound("x", 12);
	infeasible({0});
}

BOOST_AUTO_TEST_CASE(reasons_bounds)
{
	auto x = variable("x");
	addLowerBound("x", 12);
	addUpperBound("x", 11);
	addLEConstraint(variable("y"), constant(200), {2}); // unrelated
	infeasible({});
}

BOOST_AUTO_TEST_CASE(reasons_forwarded)
{
	auto x = variable("x");
	auto y = variable("y");
	addEQConstraint(x, constant(2), {0});
	addEQConstraint(x, y, {1});
	feasible({});
	addLEConstraint(x, constant(200), {5});
	addLEConstraint(y, constant(1), {2});
	infeasible({0, 1, 2, 5});
}

BOOST_AUTO_TEST_CASE(reasons_forwarded2)
{
	auto x = variable("x");
	auto y = variable("y");
	addEQConstraint(x, constant(2), {0});
	addEQConstraint(x, y, {1});
	feasible({});
	addEQConstraint(y, constant(3), {2});
	addLEConstraint(x, constant(200), {6});
	addLEConstraint(y, constant(202), {7});
	infeasible({0, 1, 2, 6, 7});
}

BOOST_AUTO_TEST_CASE(reasons_split)
{
	auto x = variable("x");
	auto y = variable("y");
	auto a = variable("a");
	auto b = variable("b");
	addLEConstraint(x + y, constant(2), {0});
	addEQConstraint(a + b, constant(20), {1});
	feasible({});
	addLEConstraint(constant(20), x + y, {2});
	infeasible({0, 2});
}

BOOST_AUTO_TEST_CASE(reasons_joined)
{
	auto x = variable("x");
	auto y = variable("y");
	auto z = variable("z");
	addLEConstraint(x + y, constant(2), {0});
	feasible({});
	addLEConstraint(constant(20), x + y, {2});
	infeasible({0, 2});
	addLEConstraint(z, constant(200), {3});
	infeasible({0, 2});
	addLEConstraint(x, z);
	infeasible({0, 2, 3});
}


BOOST_AUTO_TEST_CASE(fuzzer2)
{
	/*
		1,-1,0,0,0,0,90,9
		0,0,0,0,-76,0,0,74,
		0,0,0,0,0,31,0,0,0,0,-71
		0,0,5,0,-85,60
		1,0,0,-9,0,0,63,-31,-2,0,-78,
		0,0,50,-70,2,-76,94
		1,1,0,-3,51,
		1,0,0,-33,0,0,0,60
	*/
	// x0...x8
	auto x0 = variable("x0");
	auto x1 = variable("x1");
	auto x2 = variable("x2");
	auto x3 = variable("x3");
	auto x4 = variable("x4");
	auto x5 = variable("x5");
	auto x6 = variable("x6");
	auto x7 = variable("x7");
	auto x8 = variable("x8");
	addLowerBound("x0", 0);
	addLowerBound("x1", 0);
	addLowerBound("x2", 0);
	addLowerBound("x3", 0);
	addLowerBound("x4", 0);
	addLowerBound("x5", 0);
	addLowerBound("x6", 0);
	addLowerBound("x7", 0);
	addLowerBound("x8", 0);
	addEQConstraint(90 * x4 + 9 * x5, constant(-1));
	addLEConstraint(-76 * x2 + 74 * x5, constant(0));
	addLEConstraint(31 * x3 - 71 * x8, constant(0));
	addLEConstraint(5 * x0 - 85 * x2 + 60 * x3, constant(0));
	addEQConstraint(-9 * x1 + 63 * x4 - 31 * x5 - 2 * x6 - 78 *x8, constant(0));
	addLEConstraint(50 * x0 - 70 * x1 + 2 * x2 - 76 * x3 + 94 * x4, constant(0));
	addEQConstraint(-3 * x1 + 51 * x2, constant(1));
	addEQConstraint(-33 * x1 + 60 * x5, constant(0));
	infeasible();
}


BOOST_AUTO_TEST_SUITE_END()

}
