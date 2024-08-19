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
 * Optimiser component that turns complex expressions into multiple variable
 * declarations.
 */

#include <libyul/optimiser/ExpressionSplitter.h>

#include <libyul/optimiser/OptimiserStep.h>

#include <libyul/AST.h>
#include <libyul/Dialect.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;

void ExpressionSplitter::run(OptimiserStepContext& _context, Block& _ast)
{
	ExpressionSplitter{_context.dialect, _context.dispenser}(_ast);
}

void ExpressionSplitter::operator()(FunctionCall& _funCall)
{
	GenericVisitor visitor
	{
		[&](Builtin const& _builtin)
		{
			auto const& fun = m_dialect.builtinFunction(_builtin.handle);
			for (size_t i = _funCall.arguments.size(); i > 0; i--)
				if (!fun.literalArgument(i - 1))
					outlineExpression(_funCall.arguments[i - 1]);
		},
		[&](Verbatim const& _verbatim)
		{
			// all literal, nothing to do but assert that this is really the case
			auto const& fun = m_dialect.verbatimFunction(_verbatim.handle);
			yulAssert(fun.literalArguments.size() == fun.numParameters);
			yulAssert(std::all_of(fun.literalArguments.begin(), fun.literalArguments.end(), [](auto const& arg) { return arg.has_value(); }));
		},
		[&](Identifier const&)
		{
			for (size_t i = _funCall.arguments.size(); i > 0; i--)
				outlineExpression(_funCall.arguments[i - 1]);
		}
	};
	std::visit(visitor, _funCall.functionName);
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
	std::vector<Statement> saved;
	swap(saved, m_statementsToPrefix);

	std::function<std::optional<std::vector<Statement>>(Statement&)> f =
			[&](Statement& _statement) -> std::optional<std::vector<Statement>> {
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
	if (std::holds_alternative<Identifier>(_expr))
		return;

	visit(_expr);

	langutil::DebugData::ConstPtr debugData = debugDataOf(_expr);
	YulName var = m_nameDispenser.newName({});
	m_statementsToPrefix.emplace_back(VariableDeclaration{
		debugData,
		{{NameWithDebugData{debugData, var}}},
		std::make_unique<Expression>(std::move(_expr))
	});
	_expr = Identifier{debugData, var};
}

