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

#pragma once

#include <libsolidity/formal/SolverInterface.h>
#include <boost/noncopyable.hpp>
#include <z3++.h>

namespace dev
{
namespace solidity
{
namespace smt
{

class Z3Interface: public SolverInterface, public boost::noncopyable
{
public:
	Z3Interface();

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const& _name, Sort const& _sort) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	void declareFunction(std::string const& _name, Sort const& _sort);

	z3::expr toZ3Expr(Expression const& _expr);
	z3::sort z3Sort(smt::Sort const& _sort);
	z3::sort_vector z3Sort(std::vector<smt::SortPointer> const& _sorts);

	z3::context m_context;
	z3::solver m_solver;
	std::map<std::string, z3::expr> m_constants;
	std::map<std::string, z3::func_decl> m_functions;
};

}
}
}
