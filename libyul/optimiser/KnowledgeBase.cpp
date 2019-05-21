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
 * Class that can answer questions about values of variables and their relations.
 */

#include <libyul/optimiser/KnowledgeBase.h>

using namespace yul;

bool KnowledgeBase::knownToBeDifferent(YulString, YulString) const
{
	// TODO try to use the simplification rules together with the
	// current values to turn `sub(_a, _b)` into a nonzero constant.
	// If that fails, try `eq(_a, _b)`.
	return false;
}
