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

#include <libyul/optimiser/LoopInvariantCodeMotion.h>

#include <libyul/optimiser/CallGraphGenerator.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/SSAValueTracker.h>
#include <libyul/AST.h>
#include <libsolutil/CommonData.h>

#include <utility>

using namespace solidity;
using namespace solidity::yul;

void LoopInvariantCodeMotion::run(OptimiserStepContext& _context, Block& _ast)
{
	std::map<FunctionHandle, SideEffects> functionSideEffects =
		SideEffectsPropagator::sideEffects(_context.dialect, CallGraphGenerator::callGraph(_ast));
	bool containsMSize = MSizeFinder::containsMSize(_context.dialect, _ast);
	std::set<YulName> ssaVars = SSAValueTracker::ssaVariables(_ast);
	LoopInvariantCodeMotion{_context.dialect, ssaVars, functionSideEffects, containsMSize}(_ast);
}

void LoopInvariantCodeMotion::operator()(Block& _block)
{
	util::iterateReplacing(
		_block.statements,
		[&](Statement& _s) -> std::optional<std::vector<Statement>>
		{
			visit(_s);
			if (std::holds_alternative<ForLoop>(_s))
				return rewriteLoop(std::get<ForLoop>(_s));
			else
				return {};
		}
	);
}

bool LoopInvariantCodeMotion::canBePromoted(
	VariableDeclaration const& _varDecl,
	std::set<YulName> const& _varsDefinedInCurrentScope,
	SideEffects const& _forLoopSideEffects
) const
{
	// A declaration can be promoted iff
	// 1. Its LHS is a SSA variable
	// 2. Its RHS only references SSA variables declared outside of the current scope
	// 3. Its RHS is movable

	for (auto const& var: _varDecl.variables)
		if (!m_ssaVariables.count(var.name))
			return false;
	if (_varDecl.value)
	{
		for (auto const& ref: VariableReferencesCounter::countReferences(*_varDecl.value))
			if (_varsDefinedInCurrentScope.count(ref.first) || !m_ssaVariables.count(ref.first))
				return false;
		SideEffectsCollector sideEffects{m_dialect, *_varDecl.value, &m_functionSideEffects};
		if (!sideEffects.movableRelativeTo(_forLoopSideEffects, m_containsMSize))
			return false;
	}
	return true;
}

std::optional<std::vector<Statement>> LoopInvariantCodeMotion::rewriteLoop(ForLoop& _for)
{
	assertThrow(_for.pre.statements.empty(), OptimizerException, "");

	auto forLoopSideEffects =
		SideEffectsCollector{m_dialect, _for, &m_functionSideEffects}.sideEffects();

	std::vector<Statement> replacement;
	for (Block* block: {&_for.post, &_for.body})
	{
		std::set<YulName> varsDefinedInScope;
		util::iterateReplacing(
			block->statements,
			[&](Statement& _s) -> std::optional<std::vector<Statement>>
			{
				if (std::holds_alternative<VariableDeclaration>(_s))
				{
					VariableDeclaration const& varDecl = std::get<VariableDeclaration>(_s);
					if (canBePromoted(varDecl, varsDefinedInScope, forLoopSideEffects))
					{
						replacement.emplace_back(std::move(_s));
						// Do not add the variables declared here to varsDefinedInScope because we are moving them.
						return std::vector<Statement>{};
					}
					for (auto const& var: varDecl.variables)
						varsDefinedInScope.insert(var.name);
				}
				return {};
			}
		);
	}
	if (replacement.empty())
		return {};
	else
	{
		replacement.emplace_back(std::move(_for));
		return { std::move(replacement) };
	}
}
