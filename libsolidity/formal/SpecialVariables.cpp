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

#include <libsolidity/formal/SpecialVariables.h>

#include <libsolidity/ast/Types.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SpecialVariables::SpecialVariables(
	smt::SolverInterface& _interface
):
	m_interface(_interface)
{
	reset();
}

void SpecialVariables::reset()
{
	m_ssaVariables.clear();
}

void SpecialVariables::resetMember(MemberAccess const& _memberAccess)
{
	ASTString member = _memberAccess.memberName();
	if (m_ssaVariables.count(member))
		m_ssaVariables.erase(member);
	m_ssaVariables.emplace(member, SSAVariable(*_memberAccess.annotation().type, member, m_interface));
	m_ssaVariables.at(member).setUnknownValue();
}
