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
		std::map<util::h256, std::string> const& _queryResponses = {},
		frontend::ReadCallback::Callback _smtCallback = {},
		SMTSolverChoice _enabledSolvers = SMTSolverChoice::All(),
		std::optional<unsigned> _queryTimeout = {}
	);

	void reset();

	void registerRelation(Expression const& _expr) override;

	void addRule(Expression const& _expr, std::string const& _name) override;

	/// Takes a function application _expr and checks for reachability.
	/// @returns solving result, an invariant, and counterexample graph, if possible.
	std::tuple<CheckResult, Expression, CexGraph> query(Expression const& _expr) override;

	void declareVariable(std::string const& _name, SortPointer const& _sort) override;

	std::vector<std::string> unhandledQueries() const { return m_unhandledQueries; }

	SMTLib2Interface* smtlib2Interface() const { return m_smtlib2.get(); }

private:
	std::string toSmtLibSort(Sort const& _sort);
	std::string toSmtLibSort(std::vector<SortPointer> const& _sort);

	void writeHeader();
	std::string forall();

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
	SMTSolverChoice m_enabledSolvers;

	std::map<Sort const*, std::string> m_sortNames;
};

}
