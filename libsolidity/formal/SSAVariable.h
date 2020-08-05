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

#include <memory>

namespace solidity::frontend::smt
{

/**
 * This class represents the SSA representation of a program variable.
 */
class SSAVariable
{
public:
	SSAVariable();
	/// Resets index to 0 and next index to 1.
	void resetIndex();
	/// Sets index to _index and only adjusts next if next <= _index.
	void setIndex(unsigned _index);

	/// This function returns the current index of this SSA variable.
	unsigned index() const { return m_currentIndex; }
	unsigned& index() { return m_currentIndex; }

	unsigned operator++()
	{
		return m_currentIndex = m_nextFreeIndex++;
	}

private:
	unsigned m_currentIndex;
	unsigned m_nextFreeIndex;
};

}
