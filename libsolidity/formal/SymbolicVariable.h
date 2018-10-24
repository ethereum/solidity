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

#include <libsolidity/formal/SSAVariable.h>

#include <libsolidity/formal/SolverInterface.h>

#include <libsolidity/ast/Types.h>

#include <memory>

namespace dev
{
namespace solidity
{

class Type;

/**
 * This class represents the symbolic version of a program variable.
 */
class SymbolicVariable
{
public:
	SymbolicVariable(
		TypePointer _type,
		std::string const& _uniqueName,
		smt::SolverInterface& _interface
	);

	virtual ~SymbolicVariable() = default;

	smt::Expression currentValue() const
	{
		return valueAtIndex(m_ssa->index());
	}

	virtual smt::Expression valueAtIndex(int _index) const = 0;

	smt::Expression increaseIndex()
	{
		++(*m_ssa);
		return currentValue();
	}

	unsigned index() const { return m_ssa->index(); }
	unsigned& index() { return m_ssa->index(); }

	/// Sets the var to the default value of its type.
	/// Inherited types must implement.
	virtual void setZeroValue() = 0;
	/// The unknown value is the full range of valid values.
	/// It is sub-type dependent, but not mandatory.
	virtual void setUnknownValue() {}

protected:
	std::string uniqueSymbol(unsigned _index) const;

	TypePointer m_type = nullptr;
	std::string m_uniqueName;
	smt::SolverInterface& m_interface;
	std::shared_ptr<SSAVariable> m_ssa = nullptr;
};

}
}
