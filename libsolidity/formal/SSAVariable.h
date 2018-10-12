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

/**
 * This class represents the SSA representation of a program variable.
 */
class SSAVariable
{
public:
	/// @param _type Forwarded to the symbolic var.
	/// @param _interface Forwarded to the symbolic var such that it can give constraints to the solver.
	SSAVariable(
		Type const& _type,
		std::string const& _uniqueName,
		smt::SolverInterface& _interface
	);

	void resetIndex();

	/// This function returns the current index of this SSA variable.
	int index() const;
	/// This function returns the next free index of this SSA variable.
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

	/// These two functions forward the call to the symbolic var
	/// which generates the constraints according to the type.
	void setZeroValue();
	void setUnknownValue();

	/// So far Int and Bool are supported.
	static bool isSupportedType(Type::Category _category);
	static bool isInteger(Type::Category _category);
	static bool isBool(Type::Category _category);

private:
	smt::Expression valueAtSequence(int _seq) const
	{
		return (*m_symbolicVar)(_seq);
	}

	std::shared_ptr<SymbolicVariable> m_symbolicVar = nullptr;
	int m_currentSequenceCounter;
	/// The next free sequence counter is a shared pointer because we want
	/// the copy and the copied to share it.
	std::shared_ptr<int> m_nextFreeSequenceCounter;
};

}
}
