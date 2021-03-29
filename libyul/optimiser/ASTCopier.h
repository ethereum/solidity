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
// SPDX-License-Identifier: GPL-3.0
/**
 * Creates an independent copy of an AST, renaming identifiers to be unique.
 */

#pragma once

#include <libyul/ASTForward.h>

#include <libyul/YulString.h>

#include <memory>
#include <optional>
#include <set>
#include <vector>
#include <map>

namespace solidity::yul
{

class ExpressionCopier
{
public:
	virtual ~ExpressionCopier() = default;
	virtual Expression operator()(Literal const& _literal) = 0;
	virtual Expression operator()(Identifier const& _identifier) = 0;
	virtual Expression operator()(FunctionCall const&) = 0;
};

class StatementCopier
{
public:
	virtual ~StatementCopier() = default;
	virtual Statement operator()(ExpressionStatement const& _statement) = 0;
	virtual Statement operator()(Assignment const& _assignment) = 0;
	virtual Statement operator()(VariableDeclaration const& _varDecl) = 0;
	virtual Statement operator()(If const& _if) = 0;
	virtual Statement operator()(Switch const& _switch) = 0;
	virtual Statement operator()(FunctionDefinition const&) = 0;
	virtual Statement operator()(ForLoop const&) = 0;
	virtual Statement operator()(Break const&) = 0;
	virtual Statement operator()(Continue const&) = 0;
	virtual Statement operator()(Leave const&) = 0;
	virtual Statement operator()(Block const& _block) = 0;
};

/**
 * Creates a copy of a Yul AST potentially replacing identifier names.
 * Base class to be extended.
 */
class ASTCopier: public ExpressionCopier, public StatementCopier
{
public:
	~ASTCopier() override = default;
	Expression operator()(Literal const& _literal) override;
	Expression operator()(Identifier const& _identifier) override;
	Expression operator()(FunctionCall const&) override;
	Statement operator()(ExpressionStatement const& _statement) override;
	Statement operator()(Assignment const& _assignment) override;
	Statement operator()(VariableDeclaration const& _varDecl) override;
	Statement operator()(If const& _if) override;
	Statement operator()(Switch const& _switch) override;
	Statement operator()(FunctionDefinition const&) override;
	Statement operator()(ForLoop const&) override;
	Statement operator()(Break const&) override;
	Statement operator()(Continue const&) override;
	Statement operator()(Leave const&) override;
	Statement operator()(Block const& _block) override;

	virtual Expression translate(Expression const& _expression);
	virtual Statement translate(Statement const& _statement);

	Block translate(Block const& _block);
protected:
	template <typename T>
	std::vector<T> translateVector(std::vector<T> const& _values);

	template <typename T>
	std::unique_ptr<T> translate(std::unique_ptr<T> const& _v)
	{
		return _v ? std::make_unique<T>(translate(*_v)) : nullptr;
	}

	Case translate(Case const& _case);
	virtual Identifier translate(Identifier const& _identifier);
	Literal translate(Literal const& _literal);
	TypedName translate(TypedName const& _typedName);

	virtual void enterScope(Block const&) { }
	virtual void leaveScope(Block const&) { }
	virtual void enterFunction(FunctionDefinition const&) { }
	virtual void leaveFunction(FunctionDefinition const&) { }
	virtual YulString translateIdentifier(YulString _name) { return _name; }
};

template <typename T>
std::vector<T> ASTCopier::translateVector(std::vector<T> const& _values)
{
	std::vector<T> translated;
	for (auto const& v: _values)
		translated.emplace_back(translate(v));
	return translated;
}

/// Helper class that creates a copy of the function definition, replacing the names of the variable
/// declarations with new names.
class FunctionCopier: public ASTCopier
{
public:
	FunctionCopier(
		std::map<YulString, YulString> const& _translations
	):
		m_translations(_translations)
	{}
	using ASTCopier::operator();
	YulString translateIdentifier(YulString _name) override;
private:
	/// A mapping between old and new names. We replace the names of variable declarations contained
	/// in the mapping with their new names.
	std::map<YulString, YulString> const& m_translations;
};

}
