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
		std::map<util::h256, std::string> const& _queryResponses,
		frontend::ReadCallback::Callback _smtCallback
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

	std::map<util::h256, std::string> const& m_queryResponses;
	std::vector<std::string> m_unhandledQueries;

	frontend::ReadCallback::Callback m_smtCallback;
};

}
