// SPDX-License-Identifier: GPL-3.0
/**
 * Component that collects variables that are never assigned to and their
 * initial values.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/AsmData.h>

#include <map>
#include <set>

namespace solidity::yul
{

/**
 * Class that walks the AST and stores the initial value of each variable
 * that is never assigned to.
 *
 * A special zero constant expression is used for the default value of variables.
 *
 * Prerequisite: Disambiguator
 */
class SSAValueTracker: public ASTWalker
{
public:
	using ASTWalker::operator();
	void operator()(FunctionDefinition const& _funDef) override;
	void operator()(VariableDeclaration const& _varDecl) override;
	void operator()(Assignment const& _assignment) override;

	std::map<YulString, Expression const*> const& values() const { return m_values; }
	Expression const* value(YulString _name) const { return m_values.at(_name); }

	static std::set<YulString> ssaVariables(Block const& _ast);

private:
	void setValue(YulString _name, Expression const* _value);

	/// Special expression whose address will be used in m_values.
	/// YulString does not need to be reset because SSAValueTracker is short-lived.
	Expression const m_zero{Literal{{}, LiteralKind::Number, YulString{"0"}, {}}};
	std::map<YulString, Expression const*> m_values;
};

}
