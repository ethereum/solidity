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

#pragma once

#include <libsmtutil/SolverInterface.h>

#include <libsolidity/interface/ReadFile.h>

#include <libsolutil/Common.h>
#include <libsolutil/FixedHash.h>

#include <boost/noncopyable.hpp>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace solidity::smtutil
{

class SMTLib2Interface: public SolverInterface, public boost::noncopyable
{
public:
	explicit SMTLib2Interface(
		std::map<util::h256, std::string> _queryResponses = {},
		frontend::ReadCallback::Callback _smtCallback = {},
		std::optional<unsigned> _queryTimeout = {}
	);

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const&, SortPointer const&) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

	std::vector<std::string> unhandledQueries() override { return m_unhandledQueries; }

	// Used by CHCSmtLib2Interface
	std::string toSExpr(Expression const& _expr);
	std::string toSmtLibSort(Sort const& _sort);
	std::string toSmtLibSort(std::vector<SortPointer> const& _sort);

	std::map<std::string, SortPointer> variables() { return m_variables; }

private:
	void declareFunction(std::string const& _name, SortPointer const& _sort);

	void write(std::string _data);

	std::string checkSatAndGetValuesCommand(std::vector<Expression> const& _expressionsToEvaluate);
	std::vector<std::string> parseValues(std::string::const_iterator _start, std::string::const_iterator _end);

	/// Communicates with the solver via the callback. Throws SMTSolverError on error.
	std::string querySolver(std::string const& _input);

	std::vector<std::string> m_accumulatedOutput;
	std::map<std::string, SortPointer> m_variables;
	std::set<std::string> m_userSorts;

	std::map<util::h256, std::string> m_queryResponses;
	std::vector<std::string> m_unhandledQueries;

	frontend::ReadCallback::Callback m_smtCallback;
};

}
