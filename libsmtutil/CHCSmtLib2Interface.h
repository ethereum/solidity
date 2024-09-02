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
#include <libsmtutil/SMTLib2Parser.h>

namespace solidity::smtutil
{

class CHCSmtLib2Interface: public CHCSolverInterface
{
public:
	explicit CHCSmtLib2Interface(
		std::map<util::h256, std::string> _queryResponses = {},
		frontend::ReadCallback::Callback _smtCallback = {},
		std::optional<unsigned> _queryTimeout = {}
	);

	void reset();

	void registerRelation(Expression const& _expr) override;

	void addRule(Expression const& _expr, std::string const& _name) override;

	/// Takes a function application _expr and checks for reachability.
	/// @returns solving result, an invariant, and counterexample graph, if possible.
	QueryResult query(Expression const& _expr) override;

	void declareVariable(std::string const& _name, SortPointer const& _sort) override;

	std::string dumpQuery(Expression const& _expr);

	std::vector<std::string> unhandledQueries() const { return m_unhandledQueries; }

protected:
	class ScopedParser
	{
	public:
		ScopedParser(SMTLib2Context const& _context): m_context(_context) {}

		smtutil::Expression toSMTUtilExpression(SMTLib2Expression const& _expr);

		SortPointer toSort(SMTLib2Expression const& _expr);

		void addVariableDeclaration(std::string _name, SortPointer _sort);

	private:
		std::optional<SortPointer> lookupKnownTupleSort(std::string const& _name) const;

		smtutil::Expression parseQuantifier(
			std::string const& _quantifierName,
			std::vector<SMTLib2Expression> const& _varList,
			SMTLib2Expression const& _coreExpression
			);

		SMTLib2Context const& m_context;
		std::unordered_map<std::string, SortPointer> m_localVariables;
	};

	/* Modifies the passed expression by inlining all let subexpressions */
	static void inlineLetExpressions(SMTLib2Expression& _expr);

	std::string toSmtLibSort(SortPointer const& _sort);
	std::vector<std::string> toSmtLibSort(std::vector<SortPointer> const& _sort);

	std::string forall(Expression const& _expr);

	static std::string createQueryAssertion(std::string _name);
	void createHeader();

	/// Communicates with the solver via the callback. Throws SMTSolverError on error.
	virtual std::string querySolver(std::string const& _input);

	/// Translates CHC solver response with a model to our representation of invariants. Returns None on error.
	std::optional<smtutil::Expression> invariantsFromSolverResponse(std::string const& _response) const;

	std::set<std::string> collectVariableNames(Expression const& _expr) const;

	SMTLib2Commands m_commands;
	SMTLib2Context m_context;

	std::map<util::h256, std::string> m_queryResponses;
	std::vector<std::string> m_unhandledQueries;

	frontend::ReadCallback::Callback m_smtCallback;
};

}
