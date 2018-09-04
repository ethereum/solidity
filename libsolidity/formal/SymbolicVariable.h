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

#include <libsolidity/formal/SolverInterface.h>

#include <libsolidity/ast/AST.h>

#include <memory>

namespace dev
{
namespace solidity
{

class Declaration;

/**
 * This class represents the symbolic version of a program variable.
 */
class SymbolicVariable
{
public:
	SymbolicVariable(
		Declaration const& _decl,
		smt::SolverInterface& _interface
	);
	virtual ~SymbolicVariable() = default;

	smt::Expression operator()(int _seq) const
	{
		return valueAtSequence(_seq);
	}

	std::string uniqueSymbol(int _seq) const;

	/// Sets the var to the default value of its type.
	virtual void setZeroValue(int _seq) = 0;
	/// The unknown value is the full range of valid values,
	/// and that's sub-type dependent.
	virtual void setUnknownValue(int _seq) = 0;

protected:
	virtual smt::Expression valueAtSequence(int _seq) const = 0;

	Declaration const& m_declaration;
	smt::SolverInterface& m_interface;
};

}
}
