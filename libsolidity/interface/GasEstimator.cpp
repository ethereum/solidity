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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Gas consumption estimator working alongside the AST.
 */

#include <libsolidity/interface/GasEstimator.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/CompilerUtils.h>

#include <libevmasm/ControlFlowGraph.h>
#include <libevmasm/KnownState.h>
#include <libevmasm/PathGasMeter.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/Keccak256.h>

#include <functional>
#include <map>
#include <memory>

using namespace solidity;
using namespace solidity::evmasm;
using namespace solidity::frontend;
using namespace solidity::langutil;

GasEstimator::GasConsumption GasEstimator::functionalEstimation(
	AssemblyItems const& _items,
	std::string const& _signature
) const
{
	auto state = std::make_shared<KnownState>();

	if (!_signature.empty())
	{
		ExpressionClasses& classes = state->expressionClasses();
		using Id = ExpressionClasses::Id;
		using Ids = std::vector<Id>;
		Id hashValue = classes.find(u256(util::selectorFromSignatureU32(_signature)));
		Id calldata = classes.find(Instruction::CALLDATALOAD, Ids{classes.find(u256(0))});
		if (!m_evmVersion.hasBitwiseShifting())
			// div(calldataload(0), 1 << 224) equals to hashValue
			classes.forceEqual(
				hashValue,
				Instruction::DIV,
				Ids{calldata, classes.find(u256(1) << 224)}
			);
		else
			// shr(0xe0, calldataload(0)) equals to hashValue
			classes.forceEqual(
				hashValue,
				Instruction::SHR,
				Ids{classes.find(u256(0xe0)), calldata}
			);
		// lt(calldatasize(), 4) equals to 0 (ignore the shortcut for fallback functions)
		classes.forceEqual(
			classes.find(u256(0)),
			Instruction::LT,
			Ids{classes.find(Instruction::CALLDATASIZE), classes.find(u256(4))}
		);
	}

	return PathGasMeter::estimateMax(_items, m_evmVersion, 0, state);
}

GasEstimator::GasConsumption GasEstimator::functionalEstimation(
	AssemblyItems const& _items,
	size_t const& _offset,
	FunctionDefinition const& _function
) const
{
	auto state = std::make_shared<KnownState>();

	unsigned parametersSize = CompilerUtils::sizeOnStack(_function.parameters());
	if (parametersSize > 16)
		return GasConsumption::infinite();

	// Store an invalid return value on the stack, so that the path estimator breaks upon reaching
	// the return jump.
	AssemblyItem invalidTag(PushTag, u256(-0x10));
	state->feedItem(invalidTag, true);
	if (parametersSize > 0)
		state->feedItem(swapInstruction(parametersSize));

	return PathGasMeter::estimateMax(_items, m_evmVersion, _offset, state);
}

std::set<ASTNode const*> GasEstimator::finestNodesAtLocation(
	std::vector<ASTNode const*> const& _roots
)
{
	std::map<SourceLocation, ASTNode const*> locations;
	std::set<ASTNode const*> nodes;
	SimpleASTVisitor visitor(std::function<bool(ASTNode const&)>(), [&](ASTNode const& _n)
	{
		if (!locations.count(_n.location()))
		{
			locations[_n.location()] = &_n;
			nodes.insert(&_n);
		}
	});

	for (ASTNode const* root: _roots)
		root->accept(visitor);
	return nodes;
}
