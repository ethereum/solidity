/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <memory>
#include <libsolidity/ast/AST.h>
#include <libsolidity/analysis/SyntaxChecker.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;


bool SyntaxChecker::checkSyntax(SourceUnit const& _sourceUnit)
{
	_sourceUnit.accept(*this);
	return m_errors.empty();
}

void SyntaxChecker::syntaxError(SourceLocation const& _location, std::string const& _description)
{
	auto err = make_shared<Error>(Error::Type::SyntaxError);
	*err <<
		errinfo_sourceLocation(_location) <<
		errinfo_comment(_description);

	m_errors.push_back(err);
}

bool SyntaxChecker::visit(ModifierDefinition const&)
{
	m_placeholderFound = false;
	return true;
}

void SyntaxChecker::endVisit(ModifierDefinition const& _modifier)
{
	if (!m_placeholderFound)
		syntaxError(_modifier.body().location(), "Modifier body does not contain '_'.");
	m_placeholderFound = false;
}

bool SyntaxChecker::visit(WhileStatement const&)
{
	m_inLoopDepth++;
	return true;
}

void SyntaxChecker::endVisit(WhileStatement const&	)
{
	m_inLoopDepth--;
}

bool SyntaxChecker::visit(ForStatement const&)
{
	m_inLoopDepth++;
	return true;
}

void SyntaxChecker::endVisit(ForStatement const&)
{
	m_inLoopDepth--;
}

bool SyntaxChecker::visit(Continue const& _continueStatement)
{
	if (m_inLoopDepth <= 0)
		// we're not in a for/while loop, report syntax error
		syntaxError(_continueStatement.location(), "\"continue\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

bool SyntaxChecker::visit(Break const& _breakStatement)
{
	if (m_inLoopDepth <= 0)
		// we're not in a for/while loop, report syntax error
		syntaxError(_breakStatement.location(), "\"break\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

bool SyntaxChecker::visit(const PlaceholderStatement&)
{
	m_placeholderFound = true;
	return true;
}

