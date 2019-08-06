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
/**
 * Helper class to find "irregular" control flow.
 */

#pragma once

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>

#include <vector>

namespace yul
{
struct Dialect;

/**
 * Helper class to find "irregular" control flow.
 * This includes termination, break and continue.
 */
class TerminationFinder
{
public:
	enum class ControlFlow { FlowOut, Break, Continue, Terminate };

	TerminationFinder(Dialect const& _dialect): m_dialect(_dialect) {}

	/// @returns the index of the first statement in the provided sequence
	/// that is an unconditional ``break``, ``continue`` or a
	/// call to a terminating builtin function.
	/// If control flow can continue at the end of the list,
	/// returns `FlowOut` and ``size_t(-1)``.
	/// The function might return ``FlowOut`` even though control
	/// flow cannot actually continue.
	std::pair<ControlFlow, size_t> firstUnconditionalControlFlowChange(
		std::vector<Statement> const& _statements
	);

	/// @returns the control flow type of the given statement.
	/// This function could return FlowOut even if control flow never continues.
	ControlFlow controlFlowKind(Statement const& _statement);

	/// @returns true if the expression statement is a direct
	/// call to a builtin terminating function like
	/// ``stop``, ``revert`` or ``return``.
	bool isTerminatingBuiltin(ExpressionStatement const& _exprStmnt);

private:
	Dialect const& m_dialect;
};

}
