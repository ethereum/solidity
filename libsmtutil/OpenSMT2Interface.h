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
#include <boost/noncopyable.hpp>

#include <opensmt/opensmt2.h>

namespace solidity::smtutil
{

class OpenSMT2Interface: public SolverInterface, public boost::noncopyable
{
public:
	OpenSMT2Interface(std::optional<unsigned> _queryTimeout = {});

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const&, SortPointer const&) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	PTRef toOpenSMT2Expr(Expression const& _expr);
	SRef openSMT2Sort(Sort const& _sort);
	std::vector<SRef> openSMT2Sort(std::vector<SortPointer> const& _sorts);

	std::unique_ptr<Opensmt> m_opensmt;
	LIALogic& m_logic;
	MainSolver& m_solver;
	std::map<std::string, PTRef> m_variables;
};

}
