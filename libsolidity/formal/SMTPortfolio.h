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

#include <libsolidity/interface/ReadFile.h>

#include <boost/noncopyable.hpp>

#include <vector>

namespace dev
{
namespace solidity
{
namespace smt
{

/**
 * The SMTPortfolio wraps all available solvers within a single interface,
 * propagating the functionalities to all solvers.
 * It also checks whether different solvers give conflicting answers
 * to SMT queries.
 */
class SMTPortfolio: public SolverInterface, public boost::noncopyable
{
public:
	SMTPortfolio(ReadCallback::Callback const& _readCallback);

	void reset() override;

	void push() override;
	void pop() override;

	void declareFunction(std::string _name, Sort _domain, Sort _codomain) override;
	void declareInteger(std::string _name) override;
	void declareBool(std::string _name) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	static bool solverAnswered(CheckResult result);

	std::vector<std::shared_ptr<smt::SolverInterface>> m_solvers;
};

}
}
}
