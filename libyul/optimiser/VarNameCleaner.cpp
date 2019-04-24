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

#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/AsmParser.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <algorithm>
#include <cctype>
#include <climits>
#include <iterator>
#include <string>
#include <regex>

using namespace yul;
using namespace std;

VarNameCleaner::VarNameCleaner(
	Block const& _ast,
	Dialect const& _dialect,
	set<YulString> _blacklist
):
	m_dialect{_dialect},
	m_blacklist{std::move(_blacklist)},
	m_translatedNames{}
{
	for (auto const& statement: _ast.statements)
		if (statement.type() == typeid(FunctionDefinition))
			m_blacklist.insert(boost::get<FunctionDefinition>(statement).name);
	m_usedNames = m_blacklist;
}

void VarNameCleaner::operator()(FunctionDefinition& _funDef)
{
	yulAssert(!m_insideFunction, "");
	m_insideFunction = true;

	set<YulString> globalUsedNames = std::move(m_usedNames);
	m_usedNames = m_blacklist;
	map<YulString, YulString> globalTranslatedNames;
	swap(globalTranslatedNames, m_translatedNames);

	renameVariables(_funDef.parameters);
	renameVariables(_funDef.returnVariables);
	ASTModifier::operator()(_funDef);

	swap(globalUsedNames, m_usedNames);
	swap(globalTranslatedNames, m_translatedNames);

	m_insideFunction = false;
}

void VarNameCleaner::operator()(VariableDeclaration& _varDecl)
{
	renameVariables(_varDecl.variables);
	ASTModifier::operator()(_varDecl);
}

void VarNameCleaner::renameVariables(vector<TypedName>& _variables)
{
	for (TypedName& typedName: _variables)
	{
		auto newName = findCleanName(typedName.name);
		if (newName != typedName.name)
		{
			m_translatedNames[typedName.name] = newName;
			typedName.name = newName;
		}
		m_usedNames.insert(typedName.name);
	}
}

void VarNameCleaner::operator()(Identifier& _identifier)
{
	auto name = m_translatedNames.find(_identifier.name);
	if (name != m_translatedNames.end())
		_identifier.name = name->second;
}

YulString VarNameCleaner::findCleanName(YulString const& _name) const
{
	auto newName = stripSuffix(_name);
	if (!isUsedName(newName))
		return newName;

	// create new name with suffix (by finding a free identifier)
	for (size_t i = 1; i < numeric_limits<decltype(i)>::max(); ++i)
	{
		YulString newNameSuffixed = YulString{newName.str() + "_" + to_string(i)};
		if (!isUsedName(newNameSuffixed))
			return newNameSuffixed;
	}
	yulAssert(false, "Exhausted by attempting to find an available suffix.");
}

bool VarNameCleaner::isUsedName(YulString const& _name) const
{
	if (_name.empty() || m_dialect.builtin(_name) || m_usedNames.count(_name))
		return true;
	if (dynamic_cast<EVMDialect const*>(&m_dialect))
		return Parser::instructions().count(_name.str());
	return false;
}

YulString VarNameCleaner::stripSuffix(YulString const& _name) const
{
	static regex const suffixRegex("(_+[0-9]+)+$");

	smatch suffixMatch;
	if (regex_search(_name.str(), suffixMatch, suffixRegex))
		return {YulString{suffixMatch.prefix().str()}};
	return _name;
}
