// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that renames identifiers to free up certain names.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/NameDispenser.h>

#include <set>
#include <map>

namespace solidity::yul
{
struct Dialect;

/**
 * Optimiser component that renames identifiers to free up certain names.
 *
 * Only replaces names that have been defined inside the code. If the code uses
 * names to be freed but does not define them, they remain unchanged.
 *
 * Prerequisites: Disambiguator
 */
class NameDisplacer: public ASTModifier
{
public:
	explicit NameDisplacer(
		NameDispenser& _dispenser,
		std::set<YulString> const& _namesToFree
	):
		m_nameDispenser(_dispenser),
		m_namesToFree(_namesToFree)
	{
		for (YulString n: _namesToFree)
			m_nameDispenser.markUsed(n);
	}

	using ASTModifier::operator();
	void operator()(Identifier& _identifier) override;
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(FunctionDefinition& _function) override;
	void operator()(FunctionCall& _funCall) override;
	void operator()(Block& _block) override;

protected:
	/// Check if the newly introduced identifier @a _name has to be replaced.
	void checkAndReplaceNew(YulString& _name);
	/// Replace the identifier @a _name if it is in the translation map.
	void checkAndReplace(YulString& _name) const;

	NameDispenser& m_nameDispenser;
	std::set<YulString> const& m_namesToFree;
	std::map<YulString, YulString> m_translations;
};

}
