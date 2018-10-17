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

namespace dev
{
namespace solidity
{

/**
 * Specialization of SymbolicVariable for Bool
 */
class SymbolicBoolVariable: public SymbolicVariable
{
public:
	SymbolicBoolVariable(
		Type const& _type,
		std::string const& _uniqueName,
		smt::SolverInterface& _interface
	);

	/// Sets the var to false.
	void setZeroValue();
	/// Does nothing since the SMT solver already knows the valid values for Bool.
	void setUnknownValue();

protected:
	smt::Expression valueAtIndex(int _index) const;
};

}
}
