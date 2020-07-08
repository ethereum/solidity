// SPDX-License-Identifier: GPL-3.0

#pragma once


#include <libsmtutil/SolverInterface.h>
#include <libsolidity/interface/ReadFile.h>
#include <libsolutil/FixedHash.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

namespace solidity::smtutil
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
		std::map<util::h256, std::string> const& _smtlib2Responses,
		frontend::ReadCallback::Callback const& _smtCallback,
		SMTSolverChoice _enabledSolvers
	);

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const&, SortPointer const&) override;

	void addAssertion(Expression const& _expr) override;

	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

	std::vector<std::string> unhandledQueries() override;
	unsigned solvers() override { return m_solvers.size(); }
private:
	static bool solverAnswered(CheckResult result);

	std::vector<std::unique_ptr<SolverInterface>> m_solvers;

	std::vector<Expression> m_assertions;
};

}
