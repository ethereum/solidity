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
