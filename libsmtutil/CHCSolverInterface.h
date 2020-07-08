// SPDX-License-Identifier: GPL-3.0

/**
 * Interface for constrained Horn solvers.
 */

#pragma once

#include <libsmtutil/SolverInterface.h>

namespace solidity::smtutil
{

class CHCSolverInterface
{
public:
	virtual ~CHCSolverInterface() = default;

	virtual void declareVariable(std::string const& _name, SortPointer const& _sort) = 0;

	/// Takes a function declaration as a relation.
	virtual void registerRelation(Expression const& _expr) = 0;

	/// Takes an implication and adds as rule.
	/// Needs to bound all vars as universally quantified.
	virtual void addRule(Expression const& _expr, std::string const& _name) = 0;

	/// Takes a function application and checks
	/// for reachability.
	virtual std::pair<CheckResult, std::vector<std::string>> query(
		Expression const& _expr
	) = 0;
};

}
