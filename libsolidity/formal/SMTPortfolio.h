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
#include <libdevcore/FixedHash.h>

#include <boost/noncopyable.hpp>
#include <map>
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
	SMTPortfolio(
		std::map<h256, std::string> const& _smtlib2Responses,
		ReadCallback::Callback const& _smtCallback
	);

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const&, SortPointer const&) override;

	void addAssertion(smt::Expression const& _expr) override;

	std::pair<CheckResult, std::vector<std::string>> check(std::vector<smt::Expression> const& _expressionsToEvaluate) override;

	std::vector<std::string> unhandledQueries() override;
	unsigned solvers() override { return m_solvers.size(); }
private:
	static bool solverAnswered(CheckResult result);

	std::vector<std::unique_ptr<smt::SolverInterface>> m_solvers;

	std::vector<smt::Expression> m_assertions;
};

}
}
}
