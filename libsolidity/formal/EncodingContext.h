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
#include <libsolidity/formal/SymbolicVariables.h>

namespace dev
{
namespace solidity
{
namespace smt
{

/**
 * Stores the context of the SMT encoding.
 */
class EncodingContext
{
public:
	EncodingContext(SolverInterface& _solver);

	/// Resets the entire context.
	void reset();

	/// Value of `this` address.
	smt::Expression thisAddress();

private:
	SolverInterface& m_solver;

	/// Symbolic `this` address.
	std::unique_ptr<SymbolicAddressVariable> m_thisAddress;

};

}
}
}
