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

#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/ReadFile.h>

#include <libdevcore/Common.h>

#include <boost/noncopyable.hpp>

#include <map>
#include <string>
#include <vector>
#include <cstdio>
#include <set>

namespace dev
{
namespace solidity
{
namespace smt
{

class SMTLib2Interface: public SolverInterface, public boost::noncopyable
{
public:
	explicit SMTLib2Interface(ReadCallback::Callback const& _queryCallback);

	void reset() override;

	void push() override;
	void pop() override;

	void declareFunction(std::string _name, Sort _domain, Sort _codomain) override;
	void declareInteger(std::string _name) override;
	void declareBool(std::string _name) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	std::string toSExpr(Expression const& _expr);

	void write(std::string _data);

	std::string checkSatAndGetValuesCommand(std::vector<Expression> const& _expressionsToEvaluate);
	std::vector<std::string> parseValues(std::string::const_iterator _start, std::string::const_iterator _end);

	/// Communicates with the solver via the callback. Throws SMTSolverError on error.
	std::string querySolver(std::string const& _input);

	ReadCallback::Callback m_queryCallback;
	std::vector<std::string> m_accumulatedOutput;
	std::set<std::string> m_constants;
	std::set<std::string> m_functions;
};

}
}
}
