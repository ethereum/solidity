// SPDX-License-Identifier: GPL-3.0
/**
 * Specific AST copier that replaces certain identifiers with expressions.
 */

#include <libyul/optimiser/Substitution.h>

#include <libyul/AsmData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

Expression Substitution::translate(Expression const& _expression)
{
	if (holds_alternative<Identifier>(_expression))
	{
		YulString name = std::get<Identifier>(_expression).name;
		if (m_substitutions.count(name))
			// No recursive substitution
			return ASTCopier().translate(*m_substitutions.at(name));
	}
	return ASTCopier::translate(_expression);
}
