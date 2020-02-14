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
 * Optimiser component that turns complex expressions into multiple variable
 * declarations.
 */

#include <libyul/optimiser/ExpressionSplitter.h>

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/TypeInfo.h>

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

void ExpressionSplitter::run(OptimiserStepContext& _context, Block& _ast)
{
	TypeInfo typeInfo(_context.dialect, _ast);
	ExpressionSplitter{_context.dialect, _context.dispenser, typeInfo}(_ast);
}

void ExpressionSplitter::operator()(FunctionCall& _funCall)
{
	if (BuiltinFunction const* builtin = m_dialect.builtin(_funCall.functionName.name))
		if (builtin->literalArguments)
			// We cannot outline function arguments that have to be literals
			return;

	for (auto& arg: _funCall.arguments | boost::adaptors::reversed)
		outlineExpression(arg);
}

void ExpressionSplitter::operator()(If& _if)
{
	outlineExpression(*_if.condition);
	(*this)(_if.body);
}

void ExpressionSplitter::operator()(Switch& _switch)
{
	outlineExpression(*_switch.expression);
	for (auto& _case: _switch.cases)
		// Do not visit the case expression, nothing to split there.
		(*this)(_case.body);
}

void ExpressionSplitter::operator()(ForLoop& _loop)
{
	(*this)(_loop.pre);
	// Do not visit the condition because we cannot split expressions there.
	(*this)(_loop.post);
	(*this)(_loop.body);
}

void ExpressionSplitter::operator()(Block& _block)
{
	vector<Statement> saved;
	swap(saved, m_statementsToPrefix);

	function<std::optional<vector<Statement>>(Statement&)> f =
			[&](Statement& _statement) -> std::optional<vector<Statement>> {
		m_statementsToPrefix.clear();
		visit(_statement);
		if (m_statementsToPrefix.empty())
			return {};
		m_statementsToPrefix.emplace_back(std::move(_statement));
		return std::move(m_statementsToPrefix);
	};
	iterateReplacing(_block.statements, f);

	swap(saved, m_statementsToPrefix);
}

void ExpressionSplitter::outlineExpression(Expression& _expr)
{
	if (holds_alternative<Identifier>(_expr))
		return;

	visit(_expr);

	SourceLocation location = locationOf(_expr);
	YulString var = m_nameDispenser.newName({});
	YulString type = m_typeInfo.typeOf(_expr);
	m_statementsToPrefix.emplace_back(VariableDeclaration{
		location,
		{{TypedName{location, var, type}}},
		make_unique<Expression>(std::move(_expr))
	});
	_expr = Identifier{location, var};
	m_typeInfo.setVariableType(var, type);
}

