// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsmtutil/SolverInterface.h>
#include <boost/noncopyable.hpp>
#include <z3++.h>

namespace solidity::smtutil
{

class Z3Interface: public SolverInterface, public boost::noncopyable
{
public:
	Z3Interface();

	void reset() override;

	void push() override;
	void pop() override;

	void declareVariable(std::string const& _name, SortPointer const& _sort) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

	z3::expr toZ3Expr(Expression const& _expr);

	std::map<std::string, z3::expr> constants() const { return m_constants; }
	std::map<std::string, z3::func_decl> functions() const { return m_functions; }

	z3::context* context() { return &m_context; }

	// Z3 "basic resources" limit.
	// This is used to make the runs more deterministic and platform/machine independent.
	// The tests start failing for Z3 with less than 10000000,
	// so using double that.
	static int const resourceLimit = 20000000;

private:
	void declareFunction(std::string const& _name, Sort const& _sort);

	z3::sort z3Sort(Sort const& _sort);
	z3::sort_vector z3Sort(std::vector<SortPointer> const& _sorts);

	z3::context m_context;
	z3::solver m_solver;

	std::map<std::string, z3::expr> m_constants;
	std::map<std::string, z3::func_decl> m_functions;
};

}
