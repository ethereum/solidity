// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsmtutil/SolverInterface.h>
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

namespace solidity::smtutil
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
	CVC4::Type cvc4Sort(Sort const& _sort);
	std::vector<CVC4::Type> cvc4Sort(std::vector<SortPointer> const& _sorts);

	CVC4::ExprManager m_context;
	CVC4::SmtEngine m_solver;
	std::map<std::string, CVC4::Expr> m_variables;

	// CVC4 "basic resources" limit.
	// This is used to make the runs more deterministic and platform/machine independent.
	// The tests start failing for CVC4 with less than 6000,
	// so using double that.
	static int const resourceLimit = 12000;
};

}
