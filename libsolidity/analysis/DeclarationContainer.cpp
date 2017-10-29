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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Scope - object that holds declaration of names.
 */

#include <libsolidity/analysis/DeclarationContainer.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/Types.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Declaration const* DeclarationContainer::conflictingDeclaration(
	Declaration const& _declaration,
	ASTString const* _name
) const
{
	if (!_name)
		_name = &_declaration.name();
	solAssert(!_name->empty(), "");
	vector<Declaration const*> declarations;
	if (m_declarations.count(*_name))
		declarations += m_declarations.at(*_name);
	if (m_invisibleDeclarations.count(*_name))
		declarations += m_invisibleDeclarations.at(*_name);

	if (
		dynamic_cast<FunctionDefinition const*>(&_declaration) ||
		dynamic_cast<EventDefinition const*>(&_declaration)
	)
	{
		// check that all other declarations with the same name are functions or a public state variable or events.
		// And then check that the signatures are different.
		for (Declaration const* declaration: declarations)
		{
			if (auto variableDeclaration = dynamic_cast<VariableDeclaration const*>(declaration))
			{
				if (variableDeclaration->isStateVariable() && !variableDeclaration->isConstant() && variableDeclaration->isPublic())
					continue;
				return declaration;
			}
			if (
				dynamic_cast<FunctionDefinition const*>(&_declaration) &&
				!dynamic_cast<FunctionDefinition const*>(declaration)
			)
				return declaration;
			if (
				dynamic_cast<EventDefinition const*>(&_declaration) &&
				!dynamic_cast<EventDefinition const*>(declaration)
			)
				return declaration;
			// Or, continue.
		}
	}
	else if (declarations.size() == 1 && declarations.front() == &_declaration)
		return nullptr;
	else if (!declarations.empty())
		return declarations.front();

	return nullptr;
}

bool DeclarationContainer::registerDeclaration(
	Declaration const& _declaration,
	ASTString const* _name,
	bool _invisible,
	bool _update
)
{
	if (!_name)
		_name = &_declaration.name();
	if (_name->empty())
		return true;

	if (_update)
	{
		solAssert(!dynamic_cast<FunctionDefinition const*>(&_declaration), "Attempt to update function definition.");
		m_declarations.erase(*_name);
		m_invisibleDeclarations.erase(*_name);
	}
	else if (conflictingDeclaration(_declaration, _name))
		return false;

	vector<Declaration const*>& decls = _invisible ? m_invisibleDeclarations[*_name] : m_declarations[*_name];
	if (!contains(decls, &_declaration))
		decls.push_back(&_declaration);
	return true;
}

vector<Declaration const*> DeclarationContainer::resolveName(ASTString const& _name, bool _recursive) const
{
	solAssert(!_name.empty(), "Attempt to resolve empty name.");
	auto result = m_declarations.find(_name);
	if (result != m_declarations.end())
		return result->second;
	if (_recursive && m_enclosingContainer)
		return m_enclosingContainer->resolveName(_name, true);
	return vector<Declaration const*>({});
}

vector<ASTString> DeclarationContainer::similarNames(ASTString const& _name) const
{
	vector<ASTString> similar;

	for (auto const& declaration: m_declarations)
	{
		string const& declarationName = declaration.first;
		if (DeclarationContainer::areSimilarNames(_name, declarationName))
			similar.push_back(declarationName);
	}

	if (m_enclosingContainer)
	{
		vector<ASTString> enclosingSimilar = m_enclosingContainer->similarNames(_name);
		similar.insert(similar.end(), enclosingSimilar.begin(), enclosingSimilar.end());
	}

	return similar;
}

bool DeclarationContainer::areSimilarNames(ASTString const& _name1, ASTString const& _name2)
{
	if (_name1 == _name2)
		return true;

	size_t n1 = _name1.size(), n2 = _name2.size();
	vector<vector<size_t>> dp(n1 + 1, vector<size_t>(n2 + 1));

	// In this dp formulation of Damerau–Levenshtein distance we are assuming that the strings are 1-based to make base case storage easier.
	// So index accesser to _name1 and _name2 have to be adjusted accordingly
	for (size_t i1 = 0; i1 <= n1; ++i1)
	{
		for (size_t i2 = 0; i2 <= n2; ++i2)
		{
				if (min(i1, i2) == 0) // base case
					dp[i1][i2] = max(i1, i2);
				else
				{
					dp[i1][i2] = min(dp[i1-1][i2] + 1, dp[i1][i2-1] + 1);
					// Deletion and insertion
					if (_name1[i1-1] == _name2[i2-1])
						// Same chars, can skip
						dp[i1][i2] = min(dp[i1][i2], dp[i1-1][i2-1]);
					else
						// Different chars so try substitution
						dp[i1][i2] = min(dp[i1][i2], dp[i1-1][i2-1] + 1);

					if (i1 > 1 && i2 > 1 && _name1[i1-1] == _name2[i2-2] && _name1[i1-2] == _name2[i2-1])
						// Try transposing
						dp[i1][i2] = min(dp[i1][i2], dp[i1-2][i2-2] + 1);
				}
		}
	}

	size_t distance = dp[n1][n2];

	// If distance is not greater than MAXIMUM_DISTANCE, and distance is strictly less than length of both names,
	// they can be considered similar this is to avoid irrelevant suggestions
	return distance <= MAXIMUM_DISTANCE && distance < n1 && distance < n2;
}
