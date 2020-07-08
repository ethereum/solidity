// SPDX-License-Identifier: GPL-3.0

#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/AsmData.h>
#include <libsolutil/CommonData.h>

using namespace std;
using namespace solidity;
using namespace solidity::yul;

void ForLoopConditionIntoBody::run(OptimiserStepContext& _context, Block& _ast)
{
	ForLoopConditionIntoBody{_context.dialect}(_ast);
}

void ForLoopConditionIntoBody::operator()(ForLoop& _forLoop)
{
	if (
		m_dialect.booleanNegationFunction() &&
		!holds_alternative<Literal>(*_forLoop.condition) &&
		!holds_alternative<Identifier>(*_forLoop.condition)
	)
	{
		langutil::SourceLocation const loc = locationOf(*_forLoop.condition);

		_forLoop.body.statements.emplace(
			begin(_forLoop.body.statements),
			If {
				loc,
				make_unique<Expression>(
					FunctionCall {
						loc,
						{loc, m_dialect.booleanNegationFunction()->name},
						util::make_vector<Expression>(std::move(*_forLoop.condition))
					}
				),
				Block {loc, util::make_vector<Statement>(Break{{}})}
			}
		);
		_forLoop.condition = make_unique<Expression>(
			Literal {
				loc,
				LiteralKind::Boolean,
				"true"_yulstring,
				m_dialect.boolType
			}
		);
	}
	ASTModifier::operator()(_forLoop);
}

