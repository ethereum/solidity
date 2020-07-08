// SPDX-License-Identifier: GPL-3.0

/**
 * Z3 specific Horn solver interface.
 */

#pragma once

#include <libsmtutil/CHCSolverInterface.h>
#include <libsmtutil/Z3Interface.h>

namespace solidity::smtutil
{

class Z3CHCInterface: public CHCSolverInterface
{
public:
	Z3CHCInterface();

	/// Forwards variable declaration to Z3Interface.
	void declareVariable(std::string const& _name, SortPointer const& _sort) override;

	void registerRelation(Expression const& _expr) override;

	void addRule(Expression const& _expr, std::string const& _name) override;

	std::pair<CheckResult, std::vector<std::string>> query(Expression const& _expr) override;

	Z3Interface* z3Interface() const { return m_z3Interface.get(); }

private:
	// Used to handle variables.
	std::unique_ptr<Z3Interface> m_z3Interface;

	z3::context* m_context;
	// Horn solver.
	z3::fixedpoint m_solver;
};

}
