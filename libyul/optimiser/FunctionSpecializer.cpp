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

#include <libyul/optimiser/FunctionSpecializer.h>

#include <libyul/optimiser/ASTCopier.h>
#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/NameDispenser.h>

#include <libyul/AST.h>
#include <libyul/YulString.h>
#include <libsolutil/CommonData.h>

#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/enumerate.hpp>

#include <variant>

using namespace std;
using namespace solidity::util;
using namespace solidity::yul;

FunctionSpecializer::LiteralArguments FunctionSpecializer::specializableArguments(
	FunctionCall const& _f
)
{
	auto heuristic = [&](Expression const& _e) -> optional<Expression>
	{
		if (holds_alternative<Literal>(_e))
			return ASTCopier{}.translate(_e);
		return nullopt;
	};

	return applyMap(_f.arguments, heuristic);
}

void FunctionSpecializer::operator()(FunctionCall& _f)
{
	ASTModifier::operator()(_f);

	// TODO When backtracking is implemented, the restriction of recursive functions can be lifted.
	if (
		m_dialect.builtin(_f.functionName.name) ||
		m_recursiveFunctions.count(_f.functionName.name)
	)
		return;

	LiteralArguments arguments = specializableArguments(_f);

	if (ranges::any_of(arguments, [](auto& _a) { return _a.has_value(); }))
	{
		YulString oldName = std::move(_f.functionName.name);
		auto newName = m_nameDispenser.newName(oldName);

		m_oldToNewMap[oldName].emplace_back(make_pair(newName, arguments));

		_f.functionName.name = newName;
		_f.arguments = util::filter(
			_f.arguments,
			applyMap(arguments, [](auto& _a) { return !_a; })
		);
	}
}

FunctionDefinition FunctionSpecializer::specialize(
	FunctionDefinition const& _f,
	YulString _newName,
	FunctionSpecializer::LiteralArguments _arguments
)
{
	yulAssert(_arguments.size() == _f.parameters.size(), "");

	map<YulString, YulString> translatedNames = applyMap(
		NameCollector{_f, NameCollector::OnlyVariables}.names(),
		[&](auto& _name) -> pair<YulString, YulString>
		{
			return make_pair(_name, m_nameDispenser.newName(_name));
		},
		map<YulString, YulString>{}
	);

	FunctionDefinition newFunction = get<FunctionDefinition>(FunctionCopier{translatedNames}(_f));

	// Function parameters that will be specialized inside the body are converted into variable
	// declarations.
	vector<Statement> missingVariableDeclarations;
	for (auto&& [index, argument]: _arguments | ranges::views::enumerate)
		if (argument)
			missingVariableDeclarations.emplace_back(
				VariableDeclaration{
					_f.debugData,
					vector<TypedName>{newFunction.parameters[index]},
					make_unique<Expression>(std::move(*argument))
				}
			);

	newFunction.body.statements =
		std::move(missingVariableDeclarations) + std::move(newFunction.body.statements);

	// Only take those indices that cannot be specialized, i.e., whose value is `nullopt`.
	newFunction.parameters =
		util::filter(
			newFunction.parameters,
			applyMap(_arguments, [&](auto const& _v) { return !_v; })
		);

	newFunction.name = std::move(_newName);

	return newFunction;
}

void FunctionSpecializer::run(OptimiserStepContext& _context, Block& _ast)
{
	FunctionSpecializer f{
		CallGraphGenerator::callGraph(_ast).recursiveFunctions(),
		_context.dispenser,
		_context.dialect
	};
	f(_ast);

	iterateReplacing(_ast.statements, [&](Statement& _s) -> optional<vector<Statement>>
	{
		if (holds_alternative<FunctionDefinition>(_s))
		{
			auto& functionDefinition = get<FunctionDefinition>(_s);

			if (f.m_oldToNewMap.count(functionDefinition.name))
			{
				vector<Statement> out = applyMap(
					f.m_oldToNewMap.at(functionDefinition.name),
					[&](auto& _p) -> Statement
					{
						return f.specialize(functionDefinition, std::move(_p.first), std::move(_p.second));
					}
				);
				return std::move(out) + make_vector<Statement>(std::move(functionDefinition));
			}
		}

		return nullopt;
	});
}
