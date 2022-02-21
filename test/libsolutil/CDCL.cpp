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

#include <libsolutil/CDCL.h>
#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::util;


namespace solidity::util::test
{

class CDCLTestFramework
{
public:
	Literal variable(string const& _name)
	{
		m_variables.emplace_back(_name);
		return Literal{true, m_variables.size() - 1};
	}

	void satisfiable(vector<Clause> _clauses)
	{
		auto model = CDCL{m_variables, move(_clauses)}.solve();
		BOOST_REQUIRE(!!model);
	}

	void unsatisfiable(vector<Clause> _clauses)
	{
		auto model = CDCL{m_variables, move(_clauses)}.solve();
		BOOST_REQUIRE(!model);
	}

protected:

	vector<string> m_variables;
};


BOOST_FIXTURE_TEST_SUITE(CDCL, CDCLTestFramework, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(basic)
{
	auto x = variable("x");
	Clause c1{x, ~x};
	satisfiable({c1});
}

BOOST_AUTO_TEST_CASE(basic_unsat1)
{
	auto x = variable("x");
	unsatisfiable({{x}, {~x}});
}

BOOST_AUTO_TEST_CASE(basic_unsat2)
{
	auto x1 = variable("x1");
	auto x2 = variable("x2");
	Clause c1{x1, ~x2};
	Clause c2{~x1, x2};
	Clause c3{x1, x2};
	Clause c4{~x1, ~x2};
	unsatisfiable({c1, c2, c3, c4});
}

BOOST_AUTO_TEST_CASE(basic_sat)
{
	auto x1 = variable("x1");
	auto x2 = variable("x2");
	Clause c1{x1, ~x2};
	Clause c2{~x1, x2};
	Clause c3{x1, x2};
	Clause c4{~x1, ~x2};
	unsatisfiable({c1, c2, c3, c4});
}

BOOST_AUTO_TEST_CASE(learning)
{
	auto x1 = variable("x1");
	auto x2 = variable("x2");
	auto x3 = variable("x3");
	auto x4 = variable("x4");
	auto x7 = variable("x7");
	auto x8 = variable("x8");
	auto x9 = variable("x9");
	auto x10 = variable("x10");
	auto x11 = variable("x11");
	auto x12 = variable("x12");
	Clause c1{x1, x4};
	Clause c2{x1, ~x3, ~x8};
	Clause c3{x1, x8, x12};
	Clause c4{x2, x11};
	Clause c5{~x7, ~x3, x9};
	Clause c6{~x7, x8, ~x9};
	Clause c7{x7, x8, ~x10};
	Clause c8{x7, x10, ~x12};
	satisfiable({c1, c2, c3, c4, c5, c6, c7, c8});
}


BOOST_AUTO_TEST_SUITE_END()

}
