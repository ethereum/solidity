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
#include <boost/noncopyable.hpp>

#if defined(__GLIBC__)
// The CVC4 headers includes the deprecated system headers <ext/hash_map>
// and <ext/hash_set>. These headers cause a warning that will break the
// build, unless _GLIBCXX_PERMIT_BACKWARD_HASH is set.
#define _GLIBCXX_PERMIT_BACKWARD_HASH
#endif

#include <cvc4/cvc4.h>

#if defined(__GLIBC__)
#undef _GLIBCXX_PERMIT_BACKWARD_HASH
#endif

namespace dev
{
namespace solidity
{
namespace smt
{

class CVC4Interface: public SolverInterface, public boost::noncopyable
{
public:
	CVC4Interface();

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const&, SortPointer const&) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	CVC4::Expr toCVC4Expr(Expression const& _expr);
	CVC4::Type cvc4Sort(smt::Sort const& _sort);
	std::vector<CVC4::Type> cvc4Sort(std::vector<smt::SortPointer> const& _sorts);

	CVC4::ExprManager m_context;
	CVC4::SmtEngine m_solver;
	std::map<std::string, CVC4::Expr> m_variables;
};

}
}
}
