// SPDX-License-Identifier: GPL-3.0
/**
 * Specific AST copier that replaces certain identifiers with expressions.
 */

#pragma once

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/YulString.h>

#include <map>

namespace solidity::yul
{

/**
 * Specific AST copier that replaces certain identifiers with expressions.
 */
class Substitution: public ASTCopier
{
public:
	Substitution(std::map<YulString, Expression const*> const& _substitutions):
		m_substitutions(_substitutions)
	{}
	Expression translate(Expression const& _expression) override;

private:
	std::map<YulString, Expression const*> const& m_substitutions;
};

}
