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

#include <memory>

namespace dev
{
namespace solidity
{

/**
 * This class represents the SSA representation of a program variable.
 */
class SSAVariable
{
public:
	SSAVariable();
	void resetIndex();

	/// This function returns the current index of this SSA variable.
	int index() const { return m_currentIndex; }
	int& index() { return m_currentIndex; }

	int operator++()
	{
		return m_currentIndex = (*m_nextFreeIndex)++;
	}

private:
	int m_currentIndex;
	/// The next free index is a shared pointer because we want
	/// the copy and the copied to share it.
	std::shared_ptr<int> m_nextFreeIndex;
};

}
}
