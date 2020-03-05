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

#include <set>

namespace solidity::yul
{

/**
 * Side effects of code related to control flow.
 */
struct ControlFlowSideEffects
{
	/// If true, this code terminates the control flow.
	/// State may or may not be reverted as indicated by the ``reverts`` flag.
	bool terminates = false;
	/// If true, this code reverts all state changes in the transaction.
	/// Whenever this is true, ``terminates`` has to be true as well.
	bool reverts = false;
};

}
