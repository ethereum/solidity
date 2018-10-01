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
 * Optimiser component that undoes what the ExpressionSplitter did, i.e.
 * it more or less inlines variable declarations.
 */

#include <libjulia/optimiser/ExpressionJoiner.h>

#include <libjulia/optimiser/NameCollector.h>
#include <libjulia/optimiser/Utilities.h>
#include <libjulia/Exceptions.h>

#include <libsolidity/inlineasm/AsmData.h>

#include <libdevcore/CommonData.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace dev::julia;
using namespace dev::solidity;

void ExpressionJoiner::operator()(FunctionalInstruction& _instruction)
{
	handleArguments(_instruction.arguments);
}

void ExpressionJoiner::operator()(FunctionCall& _funCall)
{
	handleArguments(_funCall.arguments);
}

void ExpressionJoiner::operator()(If& _if)
{
	visit(*_if.condition);
	(*this)(_if.body);
}

void ExpressionJoiner::operator()(Switch& _switch)
{
	visit(*_switch.expression);
	for (auto& _case: _switch.cases)
		// Do not visit the case expression, nothing to unbreak there.
		(*this)(_case.body);
}

void ExpressionJoiner::operator()(Block& _block)
{
	resetLatestStatementPointer();
	for (size_t i = 0; i < _block.statements.size(); ++i)
	{
		visit(_block.statements[i]);
		m_currentBlock = &_block;
		m_latestStatementInBlock = i;
	}

	removeEmptyBlocks(_block);
	resetLatestStatementPointer();
}

void ExpressionJoiner::visit(Expression& _e)
{
	if (_e.type() == typeid(Identifier))
	{
		Identifier const& identifier = boost::get<Identifier>(_e);
		if (isLatestStatementVarDeclOf(identifier) && m_references[identifier.name] == 1)
		{
			VariableDeclaration& varDecl = boost::get<VariableDeclaration>(*latestStatement());
			assertThrow(varDecl.variables.size() == 1, OptimizerException, "");
			assertThrow(varDecl.value, OptimizerException, "");

			_e = std::move(*varDecl.value);
			// Delete the variable declaration (also get the moved-from structure back into a sane state)
			*latestStatement() = Block();

			decrementLatestStatementPointer();
		}
	}
	else
		ASTModifier::visit(_e);
}

void ExpressionJoiner::run(Block& _ast)
{
	ExpressionJoiner{_ast}(_ast);
}

ExpressionJoiner::ExpressionJoiner(Block& _ast)
{
	ReferencesCounter counter;
	counter(_ast);
	m_references = counter.references();
}

void ExpressionJoiner::handleArguments(vector<Expression>& _arguments)
{
	// We have to fill from left to right, but we can only
	// fill if everything to the right is just an identifier
	// or a literal.
	// Also we only descend into function calls if everything
	// on the right is an identifier or literal.

	size_t i = _arguments.size();
	for (Expression const& arg: _arguments | boost::adaptors::reversed)
	{
		--i;
		if (arg.type() != typeid(Identifier) && arg.type() != typeid(Literal))
			break;
	}
	// i points to the last element that is neither an identifier nor a literal,
	// or to the first element if all of them are identifiers or literals.

	for (; i < _arguments.size(); ++i)
		visit(_arguments.at(i));
}

void ExpressionJoiner::decrementLatestStatementPointer()
{
	if (!m_currentBlock)
		return;
	if (m_latestStatementInBlock > 0)
		--m_latestStatementInBlock;
	else
		resetLatestStatementPointer();
}

void ExpressionJoiner::resetLatestStatementPointer()
{
	m_currentBlock = nullptr;
	m_latestStatementInBlock = size_t(-1);
}

Statement* ExpressionJoiner::latestStatement()
{
	if (!m_currentBlock)
		return nullptr;
	else
		return &m_currentBlock->statements.at(m_latestStatementInBlock);
}

bool ExpressionJoiner::isLatestStatementVarDeclOf(Identifier const& _identifier)
{
	Statement const* statement = latestStatement();
	if (!statement || statement->type() != typeid(VariableDeclaration))
		return false;
	VariableDeclaration const& varDecl = boost::get<VariableDeclaration>(*statement);
	if (varDecl.variables.size() != 1 || !varDecl.value)
		return false;
	return varDecl.variables.at(0).name == _identifier.name;
}
