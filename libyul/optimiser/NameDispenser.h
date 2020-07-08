// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that can create new unique names.
 */
#pragma once

#include <libyul/AsmDataForward.h>

#include <libyul/YulString.h>

#include <set>

namespace solidity::yul
{
struct Dialect;

/**
 * Optimizer component that can be used to generate new names that
 * do not conflict with existing names.
 *
 * Tries to keep names short and appends decimals to disambiguate.
 */
class NameDispenser
{
public:
	/// Initialize the name dispenser with all the names used in the given AST.
	explicit NameDispenser(Dialect const& _dialect, Block const& _ast, std::set<YulString> _reservedNames = {});
	/// Initialize the name dispenser with the given used names.
	explicit NameDispenser(Dialect const& _dialect, std::set<YulString> _usedNames);

	/// @returns a currently unused name that should be similar to _nameHint.
	YulString newName(YulString _nameHint);

	/// Mark @a _name as used, i.e. the dispenser's newName function will not
	/// return it.
	void markUsed(YulString _name) { m_usedNames.insert(_name); }

private:
	bool illegalName(YulString _name);

	Dialect const& m_dialect;
	std::set<YulString> m_usedNames;
	size_t m_counter = 0;
};

}
