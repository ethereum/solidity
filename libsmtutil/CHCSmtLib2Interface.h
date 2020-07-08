// SPDX-License-Identifier: GPL-3.0

/**
 * Interface for solving Horn systems via smtlib2.
 */

#pragma once

#include <libsmtutil/CHCSolverInterface.h>

#include <libsmtutil/SMTLib2Interface.h>

namespace solidity::smtutil
{

class CHCSmtLib2Interface: public CHCSolverInterface
{
public:
	explicit CHCSmtLib2Interface(
		std::map<util::h256, std::string> const& _queryResponses,
		frontend::ReadCallback::Callback const& _smtCallback
	);

	void reset();

	void registerRelation(Expression const& _expr) override;

	void addRule(Expression const& _expr, std::string const& _name) override;

	std::pair<CheckResult, std::vector<std::string>> query(Expression const& _expr) override;

	void declareVariable(std::string const& _name, SortPointer const& _sort) override;

	std::vector<std::string> unhandledQueries() const { return m_unhandledQueries; }

	SMTLib2Interface* smtlib2Interface() const { return m_smtlib2.get(); }

private:
	void declareFunction(std::string const& _name, SortPointer const& _sort);

	void write(std::string _data);

	/// Communicates with the solver via the callback. Throws SMTSolverError on error.
	std::string querySolver(std::string const& _input);

	/// Used to access toSmtLibSort, SExpr, and handle variables.
	std::unique_ptr<SMTLib2Interface> m_smtlib2;

	std::string m_accumulatedOutput;
	std::set<std::string> m_variables;

	std::map<util::h256, std::string> const& m_queryResponses;
	std::vector<std::string> m_unhandledQueries;

	frontend::ReadCallback::Callback m_smtCallback;
};

}
