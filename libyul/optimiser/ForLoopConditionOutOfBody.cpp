// SPDX-License-Identifier: GPL-3.0

#include <libyul/optimiser/ForLoopConditionOutOfBody.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>
#include <libyul/Utilities.h>
#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void ForLoopConditionOutOfBody::run(OptimiserStepContext& _context, Block& _ast)
{
	ForLoopConditionOutOfBody{_context.dialect}(_ast);
}

void ForLoopConditionOutOfBody::operator()(ForLoop& _forLoop)
{
	ASTModifier::operator()(_forLoop);

	if (
		!m_dialect.booleanNegationFunction() ||
		!holds_alternative<Literal>(*_forLoop.condition) ||
		valueOfLiteral(std::get<Literal>(*_forLoop.condition)) == u256(0) ||
		_forLoop.body.statements.empty() ||
		!holds_alternative<If>(_forLoop.body.statements.front())
	)
		return;

	If& firstStatement = std::get<If>(_forLoop.body.statements.front());
	if (
		firstStatement.body.statements.empty() ||
		!holds_alternative<Break>(firstStatement.body.statements.front())
	)
		return;
	if (!SideEffectsCollector(m_dialect, *firstStatement.condition).movable())
		return;

	YulString iszero = m_dialect.booleanNegationFunction()->name;
	langutil::SourceLocation location = locationOf(*firstStatement.condition);

	if (
		holds_alternative<FunctionCall>(*firstStatement.condition) &&
		std::get<FunctionCall>(*firstStatement.condition).functionName.name == iszero
	)
		_forLoop.condition = make_unique<Expression>(std::move(std::get<FunctionCall>(*firstStatement.condition).arguments.front()));
	else
		_forLoop.condition = make_unique<Expression>(FunctionCall{
			location,
			Identifier{location, iszero},
			util::make_vector<Expression>(
				std::move(*firstStatement.condition)
			)
		});

	_forLoop.body.statements.erase(_forLoop.body.statements.begin());
}

