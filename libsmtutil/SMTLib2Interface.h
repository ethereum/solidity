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

#include <libsmtutil/BMCSolverInterface.h>

#include <libsmtutil/SMTLib2Context.h>

#include <libsolidity/interface/ReadFile.h>

#include <libsolutil/Common.h>
#include <libsolutil/FixedHash.h>

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace solidity::smtutil
{

class SMTLib2Commands
{
public:
	void push();
	void pop();

	void clear();

	void assertion(std::string _expr);

	void setOption(std::string _name, std::string _value);

	void setLogic(std::string _logic);

	void declareVariable(std::string _name, std::string _sort);
	void declareFunction(std::string const& _name, std::vector<std::string> const& _domain, std::string const& _codomain);
	void declareTuple(
		std::string const& _name,
		std::vector<std::string> const& _memberNames,
		std::vector<std::string> const& _memberSorts
	);

	[[nodiscard]] std::string toString() const;
private:
	std::vector<std::string> m_commands;
	std::vector<std::size_t> m_frameLimits;

};

class SMTLib2Interface: public BMCSolverInterface
{
public:
	/// Noncopyable.
	SMTLib2Interface(SMTLib2Interface const&) = delete;
	SMTLib2Interface& operator=(SMTLib2Interface const&) = delete;

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
	std::string toSmtLibSort(SortPointer _sort);
	std::vector<std::string> toSmtLibSort(std::vector<SortPointer> const& _sort);

	std::string dumpQuery(std::vector<Expression> const& _expressionsToEvaluate);

protected:
	virtual void setupSmtCallback() {}

	void declareFunction(std::string const& _name, SortPointer const& _sort);

	std::string checkSatAndGetValuesCommand(std::vector<Expression> const& _expressionsToEvaluate);

	/// Communicates with the solver via the callback. Throws SMTSolverError on error.
	virtual std::string querySolver(std::string const& _input);

	SMTLib2Commands m_commands;
	SMTLib2Context m_context;

	std::map<util::h256, std::string> m_queryResponses;
	std::vector<std::string> m_unhandledQueries;

	frontend::ReadCallback::Callback m_smtCallback;
};

}
