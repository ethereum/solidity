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

bool SyntaxChecker::visit(WhileStatement const& _whileStatement)
{
	_whileStatement.body().annotation().isInLoop = true;
	return true;
}

bool SyntaxChecker::visit(ForStatement const& _forStatement)
{
	_forStatement.body().annotation().isInLoop = true;
	return true;
}

bool SyntaxChecker::visit(Block const& _blockStatement)
{
	bool inLoop = _blockStatement.annotation().isInLoop;
	for (auto& statement : _blockStatement.statements())
		statement->annotation().isInLoop = inLoop;
	return true;
}

bool SyntaxChecker::visit(IfStatement const& _ifStatement)
{
	bool inLoop = _ifStatement.annotation().isInLoop;
	_ifStatement.trueStatement().annotation().isInLoop = inLoop;
	if (_ifStatement.falseStatement())
		_ifStatement.falseStatement()->annotation().isInLoop = inLoop;
	return true;
}

bool SyntaxChecker::visit(Continue const& _continueStatement)
{
	if (!_continueStatement.annotation().isInLoop)
		// we're not in a for/while loop, report syntax error
		syntaxError(_continueStatement.location(), "\"continue\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

bool SyntaxChecker::visit(Break const& _breakStatement)
{
	if (!_breakStatement.annotation().isInLoop)
		// we're not in a for/while loop, report syntax error
		syntaxError(_breakStatement.location(), "\"break\" has to be in a \"for\" or \"while\" loop.");
	return true;
}

