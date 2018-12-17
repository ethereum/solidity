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
#include <liblangutil/Exceptions.h>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

#include <boost/noncopyable.hpp>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace dev
{
namespace solidity
{
namespace smt
{

class SMTLib2Interface: public SolverInterface, public boost::noncopyable
{
public:
	explicit SMTLib2Interface(std::map<h256, std::string> const& _queryResponses);

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const&, Sort const&) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

	std::vector<std::string> unhandledQueries() override { return m_unhandledQueries; }

private:
	void declareFunction(std::string const&, Sort const&);

	std::string toSExpr(Expression const& _expr);
	std::string toSmtLibSort(Sort const& _sort);
	std::string toSmtLibSort(std::vector<SortPointer> const& _sort);

	void write(std::string _data);

	std::string checkSatAndGetValuesCommand(std::vector<Expression> const& _expressionsToEvaluate);
	std::vector<std::string> parseValues(std::string::const_iterator _start, std::string::const_iterator _end);

	/// Communicates with the solver via the callback. Throws SMTSolverError on error.
	std::string querySolver(std::string const& _input);

	std::vector<std::string> m_accumulatedOutput;
	std::set<std::string> m_variables;

	std::map<h256, std::string> const& m_queryResponses;
	std::vector<std::string> m_unhandledQueries;
};

}
}
}
