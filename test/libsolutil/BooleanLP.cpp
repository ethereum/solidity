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
#include <libsolutil/LinearExpression.h>
#include <libsmtutil/Sorts.h>
#include <libsolutil/StringUtils.h>
#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::smtutil;
using namespace solidity::util;


namespace solidity::util::test
{


class BooleanLPTestFramework
{
protected:
	BooleanLPSolver solver;

	Expression variable(string const& _name)
	{
		return solver.newVariable(_name, smtutil::SortProvider::sintSort);
	}

	Expression booleanVariable(string const& _name)
	{
		return solver.newVariable(_name, smtutil::SortProvider::boolSort);
	}

	void addAssertion(Expression const& _expr) { solver.addAssertion(_expr); }

	void feasible(vector<pair<Expression, string>> const& _solution)
	{
		vector<Expression> variables;
		vector<string> values;
		for (auto const& [var, val]: _solution)
		{
			variables.emplace_back(var);
			values.emplace_back(val);
		}
		auto [result, model] = solver.check(variables);
		// TODO it actually never returns "satisfiable".
		BOOST_CHECK(result == smtutil::CheckResult::SATISFIABLE);
		//BOOST_CHECK_EQUAL(joinHumanReadable(model), joinHumanReadable(values));
	}

	void infeasible()
	{
		auto [result, model] = solver.check({});
		BOOST_CHECK(result == smtutil::CheckResult::UNSATISFIABLE);
	}
};



BOOST_FIXTURE_TEST_SUITE(BooleanLP, BooleanLPTestFramework, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(lower_bound)
{
	Expression x = variable("x");
	Expression y = variable("y");
	addAssertion(y >= 1);
	addAssertion(x <= 10);
	addAssertion(2 * x + y <= 2);
	feasible({{x, "0"}, {y, "2"}});
}

BOOST_AUTO_TEST_CASE(check_infeasible)
{
	Expression x = variable("x");
	addAssertion(x <= 3 && x >= 5);
	infeasible();
}

BOOST_AUTO_TEST_CASE(unbounded)
{
	Expression x = variable("x");
	addAssertion(x >= 2);
	feasible({{x, "2"}});
}

BOOST_AUTO_TEST_CASE(unbounded_two)
{
	Expression x = variable("x");
	Expression y = variable("y");
	addAssertion(x + y >= 2);
	addAssertion(x <= 10);
	feasible({{x, "10"}, {y, "0"}});
}

BOOST_AUTO_TEST_CASE(equal)
{
	Expression x = variable("x");
	Expression y = variable("y");
	solver.addAssertion(x == y + 10);
	solver.addAssertion(x <= 20);
	feasible({{x, "20"}, {y, "10"}});
}

BOOST_AUTO_TEST_CASE(push_pop)
{
	Expression x = variable("x");
	Expression y = variable("y");
	solver.addAssertion(x + y <= 20);
	feasible({{x, "20"}, {y, "0"}});

	solver.push();
	solver.addAssertion(x <= 5);
	solver.addAssertion(y <= 5);
	feasible({{x, "5"}, {y, "5"}});

	solver.push();
	solver.addAssertion(x >= 7);
	infeasible();
	solver.pop();

	feasible({{x, "5"}, {y, "5"}});
	solver.pop();

	feasible({{x, "20"}, {y, "0"}});
}

BOOST_AUTO_TEST_CASE(less_than)
{
	Expression x = variable("x");
	Expression y = variable("y");
	solver.addAssertion(x == y + 1);
	solver.push();
	solver.addAssertion(y < x);
	feasible({{x, "1"}, {y, "0"}});
	solver.pop();
	solver.push();
	solver.addAssertion(y > x);
	infeasible();
	solver.pop();
}

BOOST_AUTO_TEST_CASE(equal_constant)
{
	Expression x = variable("x");
	Expression y = variable("y");
	solver.addAssertion(x < y);
	solver.addAssertion(y == 5);
	feasible({{x, "4"}, {y, "5"}});
}

BOOST_AUTO_TEST_CASE(chained_less_than)
{
	Expression x = variable("x");
	Expression y = variable("y");
	Expression z = variable("z");
	solver.addAssertion(x < y && y < z);

	solver.push();
	solver.addAssertion(z == 0);
	infeasible();
	solver.pop();

	solver.push();
	solver.addAssertion(z == 1);
	infeasible();
	solver.pop();

	solver.push();
	solver.addAssertion(z == 2);
	feasible({{x, "0"}, {y, "1"}, {z, "2"}});
	solver.pop();
}

BOOST_AUTO_TEST_CASE(splittable)
{
	Expression x = variable("x");
	Expression y = variable("y");
	Expression z = variable("z");
	Expression w = variable("w");
	solver.addAssertion(x < y);
	solver.addAssertion(x < y - 2);
	solver.addAssertion(z + w == 28);

	solver.push();
	solver.addAssertion(z >= 30);
	infeasible();
	solver.pop();

	solver.addAssertion(z >= 2);
	feasible({{x, "0"}, {y, "3"}, {z, "2"}, {w, "26"}});
	solver.push();
	solver.addAssertion(z >= 4);
	feasible({{x, "0"}, {y, "3"}, {z, "4"}, {w, "24"}});

	solver.push();
	solver.addAssertion(z < 4);
	infeasible();
	solver.pop();

	// z >= 4 is still active
	solver.addAssertion(z >= 3);
	feasible({{x, "0"}, {y, "3"}, {z, "4"}, {w, "24"}});
}


BOOST_AUTO_TEST_CASE(boolean)
{
	Expression x = variable("x");
	Expression y = variable("y");
	Expression z = variable("z");
	solver.addAssertion(x <= 5);
	solver.addAssertion(y <= 2);
	solver.push();
	solver.addAssertion(x < y && x > y);
	infeasible();
	solver.pop();
	Expression w = booleanVariable("w");
	solver.addAssertion(w == (x < y));
	solver.addAssertion(w || x > y);
	feasible({{x, "0"}, {y, "3"}, {z, "2"}, {w, "26"}});
}

BOOST_AUTO_TEST_CASE(boolean_complex)
{
	Expression x = variable("x");
	Expression y = variable("y");
	Expression a = booleanVariable("a");
	Expression b = booleanVariable("b");
	solver.addAssertion(x <= 5);
	solver.addAssertion(y <= 2);
	solver.addAssertion(a == (x >= 2));
	solver.addAssertion(a || b);
	solver.addAssertion(b == !a);
	solver.addAssertion(b == (x < 2));
	feasible({{a, "1"}, {b, "0"}, {x, "5"}, {y, "2"}});
	solver.addAssertion(a && b);
	infeasible();
}

BOOST_AUTO_TEST_CASE(magic_square_3)
{
	vector<Expression> vars;
	for (size_t i = 0; i < 9; i++)
		vars.push_back(variable(string{static_cast<char>('a' + i)}));
	Expression sum = variable("sum");
	for (Expression const& var: vars)
		solver.addAssertion(1 <= var && var <= 9);
	for (size_t i = 0; i < 9; i++)
		for (size_t j = i + 1; j < 9; j++)
			solver.addAssertion(vars[i] != vars[j]);
	for (size_t i = 0; i < 3; i++)
		solver.addAssertion(vars[i] + vars[i + 3] + vars[i + 6] == sum);
	for (size_t i = 0; i < 9; i += 3)
		solver.addAssertion(vars[i] + vars[i + 1] + vars[i + 2] == sum);
	solver.addAssertion(vars[0] + vars[4] + vars[8] == sum);
	solver.addAssertion(vars[2] + vars[4] + vars[6] == sum);
	feasible({
		{sum, "15"},
		{vars[0], "8"}, {vars[1], "3"}, {vars[2], "4"},
		{vars[3], "1"}, {vars[4], "5"}, {vars[5], "9"},
		{vars[6], "6"}, {vars[7], "7"}, {vars[8], "2"}
	});
}

BOOST_AUTO_TEST_CASE(magic_square_4)
{
	vector<Expression> vars;
	for (size_t i = 0; i < 16; i++)
		vars.push_back(variable(string{static_cast<char>('a' + i)}));
	Expression sum = variable("sum");
	for (Expression const& var: vars)
		solver.addAssertion(1 <= var && var <= 16);
	for (size_t i = 0; i < 16; i++)
		for (size_t j = i + 1; j < 16; j++)
			solver.addAssertion(vars[i] != vars[j]);
	for (size_t i = 0; i < 4; i++)
		solver.addAssertion(vars[i] + vars[i + 4] + vars[i + 8] + vars[i + 12] == sum);
	for (size_t i = 0; i < 16; i += 4)
		solver.addAssertion(vars[i] + vars[i + 1] + vars[i + 2] + vars[i + 3] == sum);
	solver.addAssertion(vars[0] + vars[5] + vars[10] + vars[15] == sum);
	solver.addAssertion(vars[3] + vars[6] + vars[9] + vars[12] == sum);
	feasible({
		{vars[0], "9"}, {vars[1], "5"}, {vars[2], "1"},
		{vars[3], "4"}, {vars[4], "3"}, {vars[5], "8"},
		{vars[6], "2"}, {vars[7], "7"}, {vars[8], "6"}
	});
}

// Magic squares 5 takes almost 2 minutes.

BOOST_AUTO_TEST_CASE(boolean_complex_2)
{
	Expression x = variable("x");
	Expression y = variable("y");
	Expression a = booleanVariable("a");
	Expression b = booleanVariable("b");
	solver.addAssertion(x != 20);
	feasible({{x, "21"}});
	solver.addAssertion(x <= 5 || (x > 7 && x != 8));
	solver.addAssertion(a == (x == 9));
	feasible({{a, "0"}, {b, "unknown"}, {x, "21"}});
	solver.addAssertion(!a || (x == 10));
	solver.addAssertion(b == !a);
	solver.addAssertion(b == (x < 200));
	feasible({{a, "0"}, {b, "1"}, {x, "199"}});
	solver.addAssertion(a && b);
	infeasible();
}


BOOST_AUTO_TEST_CASE(pure_boolean)
{
	Expression a = booleanVariable("a");
	Expression b = booleanVariable("b");
	Expression c = booleanVariable("c");
	Expression d = booleanVariable("d");
	Expression e = booleanVariable("e");
	Expression f = booleanVariable("f");
	solver.addAssertion(a && !b);
	solver.addAssertion(b || c);
	solver.addAssertion(c == (d || c));
	solver.addAssertion(f == (b && !c));
	solver.addAssertion(!f || e);
	solver.addAssertion(c || d);
	feasible({});
	solver.addAssertion(a && b);
	infeasible();
}

BOOST_AUTO_TEST_SUITE_END()

}
