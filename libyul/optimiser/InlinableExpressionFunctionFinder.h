// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that identifies functions to be inlined.
 */

#pragma once

#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>

#include <set>

namespace solidity::yul
{

/**
 * Optimiser component that finds functions that can be
 * inlined inside functional expressions, i.e. functions that
 *  - have a single return parameter r
 *  - have a body like r := <functional expression>
 *  - neither reference themselves nor r in the right hand side
 *
 * This component can only be used on sources with unique names.
 */
class InlinableExpressionFunctionFinder: public ASTWalker
{
public:

	std::map<YulString, FunctionDefinition const*> const& inlinableFunctions() const
	{
		return m_inlinableFunctions;
	}

	using ASTWalker::operator();
	void operator()(Identifier const& _identifier) override;
	void operator()(FunctionCall const& _funCall) override;
	void operator()(FunctionDefinition const& _function) override;

private:
	void checkAllowed(YulString _name)
	{
		if (m_disallowedIdentifiers.count(_name))
			m_foundDisallowedIdentifier = true;
	}

	bool m_foundDisallowedIdentifier = false;
	std::set<YulString> m_disallowedIdentifiers;
	std::map<YulString, FunctionDefinition const*> m_inlinableFunctions;
};

}
