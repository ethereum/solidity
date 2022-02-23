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

namespace solidity::yul
{

/**
 * Side effects of a user-defined or builtin function.
 *
 * Each of the three booleans represents a reachability condition. There is an implied
 * fourth alternative, which is going out of gas while executing the function. Since
 * this can always happen and depends on the supply of gas, it is not considered.
 *
 * If all three booleans are false, it means that the function always leads to infinite
 * recursion.
 */
struct ControlFlowSideEffects
{
	/// If true, the function contains at least one reachable branch that terminates successfully.
	bool canTerminate = false;
	/// If true, the function contains at least one reachable branch that reverts.
	bool canRevert = false;
	/// If true, the function has a regular outgoing control-flow.
	bool canContinue = true;

	bool terminatesOrReverts() const
	{
		return (canTerminate || canRevert) && !canContinue;
	}
};

}
