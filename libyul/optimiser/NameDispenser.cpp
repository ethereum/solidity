// SPDX-License-Identifier: GPL-3.0
/**
 * Optimiser component that can create new unique names.
 */

#include <libyul/optimiser/NameDispenser.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmParser.h>

#include <libevmasm/Instruction.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

NameDispenser::NameDispenser(Dialect const& _dialect, Block const& _ast, set<YulString> _reservedNames):
	NameDispenser(_dialect, NameCollector(_ast).names() + std::move(_reservedNames))
{
}

NameDispenser::NameDispenser(Dialect const& _dialect, set<YulString> _usedNames):
	m_dialect(_dialect),
	m_usedNames(std::move(_usedNames))
{
}

YulString NameDispenser::newName(YulString _nameHint)
{
	YulString name = _nameHint;
	while (illegalName(name))
	{
		m_counter++;
		name = YulString(_nameHint.str() + "_" + to_string(m_counter));
	}
	m_usedNames.emplace(name);
	return name;
}

bool NameDispenser::illegalName(YulString _name)
{
	if (_name.empty() || m_usedNames.count(_name) || m_dialect.builtin(_name))
		return true;
	if (dynamic_cast<EVMDialect const*>(&m_dialect))
		return Parser::instructions().count(_name.str());
	return false;
}
