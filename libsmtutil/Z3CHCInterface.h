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
 * Z3 specific Horn solver interface.
 */

#pragma once

#include <libsmtutil/CHCSolverInterface.h>
#include <libsmtutil/Z3Interface.h>

#include <tuple>
#include <vector>

namespace solidity::smtutil
{

class Z3CHCInterface: public CHCSolverInterface
{
public:
	Z3CHCInterface(std::optional<unsigned> _queryTimeout = {});

	/// Forwards variable declaration to Z3Interface.
	void declareVariable(std::string const& _name, SortPointer const& _sort) override;

	void registerRelation(Expression const& _expr) override;

	void addRule(Expression const& _expr, std::string const& _name) override;

	std::pair<CheckResult, CexGraph> query(Expression const& _expr) override;

	Z3Interface* z3Interface() const { return m_z3Interface.get(); }

	void setSpacerOptions(bool _preProcessing = true);

private:
	/// Constructs a nonlinear counterexample graph from the refutation.
	CHCSolverInterface::CexGraph cexGraph(z3::expr const& _proof);
	/// @returns the fact from a proof node.
	z3::expr fact(z3::expr const& _node);
	/// @returns @a _predicate's name.
	std::string name(z3::expr const& _predicate);
	/// @returns the arguments of @a _predicate.
	std::vector<std::string> arguments(z3::expr const& _predicate);

	// Used to handle variables.
	std::unique_ptr<Z3Interface> m_z3Interface;

	z3::context* m_context;
	// Horn solver.
	z3::fixedpoint m_solver;

	std::tuple<unsigned, unsigned, unsigned, unsigned> m_version = std::tuple(0, 0, 0, 0);
};

}
