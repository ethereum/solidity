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
 * Optimiser component that removes stores to memory and storage slots that are not used
 * or overwritten later on.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/optimiser/UnusedStoreBase.h>
#include <libyul/optimiser/KnowledgeBase.h>

#include <libevmasm/SemanticInformation.h>

#include <map>
#include <vector>

namespace solidity::yul
{
struct Dialect;
struct AssignedValue;

/**
 * Optimizer component that removes sstore and memory store statements if conditions are met for their removal.
 * In case of an sstore, if all outgoing code paths revert (due to an explicit revert(), invalid(),
 * or infinite recursion) or lead to another ``sstore`` for which the optimizer can tell that it will overwrite the first store,
 * the statement will be removed.
 *
 * For memory store operations, things are generally simpler, at least in the outermost yul block as all such statements
 * will be removed if they are never read from in any code path. At function analysis level however, the approach is similar
 * to sstore, as we don't know whether the memory location will be read once we leave the function's scope,
 * so the statement will be removed only if all code code paths lead to a memory overwrite.
 *
 * Best run in SSA form.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class UnusedStoreEliminator: public UnusedStoreBase<UnusedStoreEliminatorKey>
{
public:
	static constexpr char const* name{"UnusedStoreEliminator"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	explicit UnusedStoreEliminator(
		Dialect const& _dialect,
		std::map<YulName, SideEffects> const& _functionSideEffects,
		std::map<YulName, ControlFlowSideEffects> _controlFlowSideEffects,
		std::map<YulName, AssignedValue> const& _ssaValues,
		bool _ignoreMemory
	);

	using UnusedStoreBase::operator();
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(Leave const&) override;

	using UnusedStoreBase::visit;
	void visit(Statement const& _statement) override;

	using Location = evmasm::SemanticInformation::Location;
	using Effect = evmasm::SemanticInformation::Effect;
	using OperationLength = std::variant<YulName, u256>;
	struct Operation
	{
		Location location;
		Effect effect;
		/// Start of affected area. Unknown if not provided.
		std::optional<YulName> start;
		/// Length of affected area, unknown if not provided.
		/// Unused for storage.
		std::optional<OperationLength> length;
	};

private:
	std::set<Statement const*>& activeMemoryStores() { return m_activeStores[UnusedStoreEliminatorKey::Memory]; }
	std::set<Statement const*>& activeStorageStores() { return m_activeStores[UnusedStoreEliminatorKey::Storage]; }
	std::optional<u256> lengthValue(OperationLength const& _length) const
	{
		if (YulName const* length = std::get_if<YulName>(&_length))
			return m_knowledgeBase.valueIfKnownConstant(*length);
		else
			return std::get<u256>(_length);
	}

	void shortcutNestedLoop(ActiveStores const&) override
	{
		// We might only need to do this for newly introduced stores in the loop.
		markActiveAsUsed();
	}
	void finalizeFunctionDefinition(FunctionDefinition const&) override
	{
		markActiveAsUsed();
	}

	std::vector<Operation> operationsFromFunctionCall(FunctionCall const& _functionCall) const;
	void applyOperation(Operation const& _operation);
	bool knownUnrelated(Operation const& _op1, Operation const& _op2) const;
	bool knownCovered(Operation const& _covered, Operation const& _covering) const;

	void markActiveAsUsed(std::optional<Location> _onlyLocation = std::nullopt);
	void clearActive(std::optional<Location> _onlyLocation = std::nullopt);

	std::optional<YulName> identifierNameIfSSA(Expression const& _expression) const;

	bool const m_ignoreMemory;
	std::map<YulName, SideEffects> const& m_functionSideEffects;
	std::map<YulName, ControlFlowSideEffects> m_controlFlowSideEffects;
	std::map<YulName, AssignedValue> const& m_ssaValues;

	std::map<Statement const*, Operation> m_storeOperations;

	KnowledgeBase mutable m_knowledgeBase;
};

}
