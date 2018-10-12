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

#include <libsolidity/ast/AST.h>

namespace dev
{
namespace solidity
{

class MemberAccess;

/**
 * This class encodes the special variables block.*, msg.*, tx.* and now.
 */
class SpecialVariables
{
public:
	/// @param _interface Forwarded to SSAVariable such that it can give constraints to the solver.
	SpecialVariables(
		smt::SolverInterface& _interface
	);

	void reset();
	void resetMember(MemberAccess const& _memberAccess);

	smt::Expression operator[](std::string const& _member) const
	{
		return m_ssaVariables.at(_member)();
	}

	smt::Expression operator[](MemberAccess const& _memberAccess)
	{
		ASTString member = _memberAccess.memberName();
		if (!m_ssaVariables.count(member))
			resetMember(_memberAccess);
		return m_ssaVariables.at(member)();
	}

private:
	std::map<std::string, SSAVariable> m_ssaVariables;
	smt::SolverInterface& m_interface;
};

}
}
