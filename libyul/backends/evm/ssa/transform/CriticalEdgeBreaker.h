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

namespace solidity::yul::ssa
{

class SSACFG;

namespace transform
{
/// Splits every critical edge, i.e., one from a block with several successors into a block with several
/// predecessors, whose target carries phis. A new block is inserted on the edge and the predecessor's
/// upsilons for the target's phis move into it.
///
/// If an upsilon is left in the predecessor it would produce the pending value on both of its exits, and the path
/// not leading to the phi would carry the copy as junk.
///
/// For example, with `^p := x` the upsilon of B for the phi `p` of T, which has another predecessor C:
///
///   B [..., ^p := x]                          B [...]
///    | nonZero  \ zero                         | nonZero  \ zero
///    N           T [p := phi] <- C      =>     N           E [^p := x]
///                                                          |
///                                                          T [p := phi] <- C
///
/// An edge stays unsplit if one of these upsilons is read later in the predecessor itself, i.e., by a phi of a
/// self loop scheduled after it.
///
/// Best run after trivial phi elimination, otherwise this step may introduce edges and blocks for phis that are
/// about to be removed anyways.
void breakCriticalEdges(SSACFG& _cfg);
}

}
