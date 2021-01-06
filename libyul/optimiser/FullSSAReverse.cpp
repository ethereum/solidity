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
#include <libyul/optimiser/FullSSAReverse.h>
#include <libyul/optimiser/ASTCopier.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;

class FullSSAReverseLoad: public ASTCopier
{
public:
	FullSSAReverseLoad(std::map<YulString, YulString> const& _variableNames): m_variableNames(_variableNames) {}
	Expression operator()(FunctionCall const& _funCall) override
	{
		if (_funCall.functionName.name == "phi_load"_yulstring)
		{
			yulAssert(_funCall.arguments.size() == 1, "");
			Literal const* literal = std::get_if<Literal>(&_funCall.arguments.front());
			yulAssert(literal && literal->kind == LiteralKind::String, "");
			yulAssert(m_variableNames.count(literal->value), "");
			return Identifier{
				_funCall.location,
				m_variableNames.at(literal->value)
			};
		}
		return ASTCopier::operator()(_funCall);
	}
private:
	map<YulString, YulString> const& m_variableNames;
};

void FullSSAReverse::run(OptimiserStepContext& _context, Block& _ast)
{
	FullSSAReverse fullSSAReverse{_context.dispenser};
	fullSSAReverse(_ast);
	_ast = FullSSAReverseLoad{fullSSAReverse.m_variableNames}.translate(_ast);
}

void FullSSAReverse::operator()(Block& _block)
{
	util::iterateReplacing(
		_block.statements,
		[&](Statement& _stmt) -> std::optional<vector<Statement>>
		{
			if (auto* expressionStatement = std::get_if<ExpressionStatement>(&_stmt))
				if (auto* functionCall = std::get_if<FunctionCall>(&expressionStatement->expression))
					if (functionCall->functionName.name == "phi_store"_yulstring)
					{
						yulAssert(functionCall->arguments.size() == 2, "");
						Literal const* literal = std::get_if<Literal>(&functionCall->arguments.front());
						yulAssert(literal && literal->kind == LiteralKind::String, "");

						vector<Statement> result;
						if (m_variableNames.count(literal->value))
							result.emplace_back(Assignment{
								functionCall->location,
								{Identifier{literal->location, m_variableNames.at(literal->value)}},
								make_unique<Expression>(move(functionCall->arguments.back()))
							});
						else
						{
							YulString newName = m_nameDispenser.newName(literal->value);
							m_variableNames[literal->value] = newName;
							result.emplace_back(VariableDeclaration{
								functionCall->location,
								{TypedName{literal->location, newName, YulString{}}},
								make_unique<Expression>(move(functionCall->arguments.back()))
							});
						}

						return result;
					}

			visit(_stmt);
			return {};
		}
	);
}

void FullSSAReverse::operator()(FunctionDefinition& _funDef)
{
	set<YulString> oldFunctionReturns;
	swap(m_currentFunctionReturns, oldFunctionReturns);
	vector<Statement> bodyPrefix;
	for (auto& var: _funDef.returnVariables)
	{
		YulString newName = m_nameDispenser.newName(var.name);
		m_variableNames[var.name] = newName;
		m_currentFunctionReturns.emplace(var.name);
		bodyPrefix.emplace_back(VariableDeclaration{
			var.location,
			{var},
			make_unique<Expression>(Identifier{var.location, newName})
		});
		var.name = newName;
	}

	bodyPrefix += std::move(_funDef.body.statements);
	_funDef.body.statements = std::move(bodyPrefix);

	ASTModifier::operator()(_funDef);

	swap(m_currentFunctionReturns, oldFunctionReturns);
}
