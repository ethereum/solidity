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
/**
 * Specific AST copier that replaces certain identifiers with expressions.
 */

#include <libyul/optimiser/Substitution.h>

#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::yul;

Expression Substitution::translate(Expression const& _expression)
{
	if (_expression.type() == typeid(Identifier))
	{
		string const& name = boost::get<Identifier>(_expression).name;
		if (m_substitutions.count(name))
			// No recursive substitution
			return ASTCopier().translate(*m_substitutions.at(name));
	}
	return ASTCopier::translate(_expression);
}
