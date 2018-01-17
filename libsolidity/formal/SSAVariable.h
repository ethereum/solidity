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

#include <libsolidity/formal/SymbolicVariable.h>

#include <memory>

namespace dev
{
namespace solidity
{

class Declaration;

/**
 * This class represents the SSA representation of a program variable.
 */
class SSAVariable
{
public:
	explicit SSAVariable(Declaration const* _decl,
		smt::SolverInterface& _interface);
	SSAVariable(SSAVariable const&) = default;
	SSAVariable(SSAVariable&&) = default;
	SSAVariable& operator=(SSAVariable const&) = default;
	SSAVariable& operator=(SSAVariable&&) = default;

	void resetIndex();

	int index() const;
	int next() const;

	int operator++()
	{
		return m_currentSequenceCounter = (*m_nextFreeSequenceCounter)++;
	}

	smt::Expression operator()() const
	{
		return valueAtSequence(index());
	}

	smt::Expression operator()(int _seq) const
	{
		return valueAtSequence(_seq);
	}

	void setZeroValue();
	void setUnknownValue();

private:
	smt::Expression valueAtSequence(int _seq) const
	{
		return (*m_symbVar)(_seq);
	}

	std::shared_ptr<SymbolicVariable> m_symbVar = nullptr;
	int m_currentSequenceCounter;
	std::shared_ptr<int> m_nextFreeSequenceCounter;
};

}
}
