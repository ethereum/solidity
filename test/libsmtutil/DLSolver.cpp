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

#include <libsmtutil/DLSolver.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::smtutil;

namespace solidity::smtutil::test
{

BOOST_AUTO_TEST_SUITE(DLSolverTest)

BOOST_AUTO_TEST_CASE(test_empty_sat)
{
	DLSolver solver;

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::SATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_basic_sat)
{
	DLSolver solver;

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};

	Expression k2{"2", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);

	solver.addAssertion(a - b <= k2);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::SATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_triangle_sat)
{
	DLSolver solver;

	// a <= b -> a - b <= 0
	// b <= c -> b - c <= 0
	// c <= a -> c - a <= 0

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};
	Expression c{"c", {}, SortProvider::sintSort};

	Expression k0{"0", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);
	solver.declareVariable(c.name, c.sort);

	solver.addAssertion(a - b <= k0);
	solver.addAssertion(b - c <= k0);
	solver.addAssertion(c - a <= k0);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::SATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_basic_unsat)
{
	DLSolver solver;

	// a <= b -> a - b <= 0
	// b < a  -> b - a < 0 -> b - a <= -1

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};

	Expression k0{"0", {}, SortProvider::sintSort};
	Expression kMinus1{"-1", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);

	solver.addAssertion(a - b <= k0);
	solver.addAssertion(b - a <= kMinus1);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::UNSATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_triangle_unsat)
{
	DLSolver solver;

	// a <= b -> a - b <= 0
	// b <= c -> b - c <= 0
	// c < a -> c - a < 0 -> c - a <= -1

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};
	Expression c{"c", {}, SortProvider::sintSort};

	Expression k0{"0", {}, SortProvider::sintSort};
	Expression kMinus1{"-1", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);
	solver.declareVariable(c.name, c.sort);

	solver.addAssertion(a - b <= k0);
	solver.addAssertion(b - c <= k0);
	solver.addAssertion(c - a <= kMinus1);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::UNSATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_triangle_unsat_reset_sat)
{
	DLSolver solver;

	// a <= b -> a - b <= 0
	// b <= c -> b - c <= 0
	// c < a -> c - a < 0 -> c - a <= -1

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};
	Expression c{"c", {}, SortProvider::sintSort};

	Expression k0{"0", {}, SortProvider::sintSort};
	Expression kMinus1{"-1", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);
	solver.declareVariable(c.name, c.sort);

	solver.addAssertion(a - b <= k0);
	solver.addAssertion(b - c <= k0);
	solver.addAssertion(c - a <= kMinus1);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::UNSATISFIABLE);

	solver.reset();

	auto [res2, model2] = solver.check({});
	BOOST_CHECK(res2 == CheckResult::SATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_triangle_incremental)
{
	DLSolver solver;

	// a <= b -> a - b <= 0
	// b <= c -> b - c <= 0
	// c < a -> c - a < 0 -> c - a <= -1

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};
	Expression c{"c", {}, SortProvider::sintSort};

	Expression k0{"0", {}, SortProvider::sintSort};
	Expression kMinus1{"-1", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);
	solver.declareVariable(c.name, c.sort);

	solver.addAssertion(a - b <= k0);
	solver.addAssertion(b - c <= k0);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::SATISFIABLE);

	solver.push();

	solver.addAssertion(c - a <= kMinus1);

	auto [res2, model2] = solver.check({});
	BOOST_CHECK(res2 == CheckResult::UNSATISFIABLE);

	solver.pop();

	auto [res3, model3] = solver.check({});
	BOOST_CHECK(res3 == CheckResult::SATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_triangle_complex)
{
	DLSolver solver;

	// a−b ≤ 2, b−c ≤ 3, c−a ≤ −7

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};
	Expression c{"c", {}, SortProvider::sintSort};

	Expression k2{"2", {}, SortProvider::sintSort};
	Expression k3{"3", {}, SortProvider::sintSort};
	Expression kMinus7{"-7", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);
	solver.declareVariable(c.name, c.sort);

	solver.addAssertion(a - b <= k2);
	solver.addAssertion(b - c <= k3);
	solver.addAssertion(c - a <= kMinus7);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::UNSATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_triangle_complex2)
{
	DLSolver solver;

	// a−b ≤ 2, b−c ≤ 10, c−a ≤ −7

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};
	Expression c{"c", {}, SortProvider::sintSort};

	Expression k2{"2", {}, SortProvider::sintSort};
	Expression k10{"10", {}, SortProvider::sintSort};
	Expression kMinus7{"-7", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);
	solver.declareVariable(c.name, c.sort);

	solver.addAssertion(a - b <= k2);
	solver.addAssertion(b - c <= k10);
	solver.addAssertion(c - a <= kMinus7);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::SATISFIABLE);
}

BOOST_AUTO_TEST_CASE(test_triangle_complex2_incremental)
{
	DLSolver solver;

	// a−b ≤ 2, b−c ≤ 10, c−a ≤ −7

	Expression a{"a", {}, SortProvider::sintSort};
	Expression b{"b", {}, SortProvider::sintSort};
	Expression c{"c", {}, SortProvider::sintSort};

	Expression k2{"2", {}, SortProvider::sintSort};
	Expression k3{"3", {}, SortProvider::sintSort};
	Expression k10{"10", {}, SortProvider::sintSort};
	Expression kMinus7{"-7", {}, SortProvider::sintSort};

	solver.declareVariable(a.name, a.sort);
	solver.declareVariable(b.name, b.sort);
	solver.declareVariable(c.name, c.sort);

	solver.addAssertion(a - b <= k2);
	solver.addAssertion(c - a <= kMinus7);

	auto [res, model] = solver.check({});
	BOOST_CHECK(res == CheckResult::SATISFIABLE);

	solver.push();
	solver.addAssertion(b - c <= k3);

	auto [res2, model2] = solver.check({});
	BOOST_CHECK(res2 == CheckResult::UNSATISFIABLE);
	
	solver.pop();

	solver.addAssertion(b - c <= k10);
	auto [res3, model3] = solver.check({});
	BOOST_CHECK(res3 == CheckResult::SATISFIABLE);
}


BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
