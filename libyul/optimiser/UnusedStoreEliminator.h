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

#include <libevmasm/SemanticInformation.h>

#include <map>
#include <vector>

namespace solidity::yul
{
struct Dialect;
struct AssignedValue;

/**
 * Optimizer component that removes sstore statements if they
 * are overwritten in all code paths or never read from.
 *
 * The m_store member of UnusedStoreBase is only used with the empty yul string
 * as key in the first dimension.
 *
 * Best run in SSA form.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class UnusedStoreEliminator: public UnusedStoreBase
{
public:
	static constexpr char const* name{"UnusedStoreEliminator"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	explicit UnusedStoreEliminator(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> const& _functionSideEffects,
		std::map<YulString, ControlFlowSideEffects> _controlFlowSideEffects,
		std::map<YulString, AssignedValue> const& _ssaValues,
		bool _ignoreMemory
	):
		UnusedStoreBase(_dialect),
		m_ignoreMemory(_ignoreMemory),
		m_functionSideEffects(_functionSideEffects),
		m_controlFlowSideEffects(_controlFlowSideEffects),
		m_ssaValues(_ssaValues)
	{}

	using UnusedStoreBase::operator();
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(Leave const&) override;

	using UnusedStoreBase::visit;
	void visit(Statement const& _statement) override;

	using Location = evmasm::SemanticInformation::Location;
	using Effect = evmasm::SemanticInformation::Effect;
	struct Operation
	{
		Location location;
		Effect effect;
		/// Start of affected area. Unknown if not provided.
		std::optional<YulString> start;
		/// Length of affected area, unknown if not provided.
		/// Unused for storage.
		std::optional<YulString> length;
	};

private:
	void shortcutNestedLoop(TrackedStores const&) override
	{
		// We might only need to do this for newly introduced stores in the loop.
		changeUndecidedTo(State::Used);
	}
	void finalizeFunctionDefinition(FunctionDefinition const&) override;

	std::vector<Operation> operationsFromFunctionCall(FunctionCall const& _functionCall) const;
	void applyOperation(Operation const& _operation);
	bool knownUnrelated(Operation const& _op1, Operation const& _op2) const;
	bool knownCovered(Operation const& _covered, Operation const& _covering) const;

	void changeUndecidedTo(State _newState, std::optional<Location> _onlyLocation = std::nullopt);
	void scheduleUnusedForDeletion();

	std::optional<YulString> identifierNameIfSSA(Expression const& _expression) const;

	bool const m_ignoreMemory;
	std::map<YulString, SideEffects> const& m_functionSideEffects;
	std::map<YulString, ControlFlowSideEffects> m_controlFlowSideEffects;
	std::map<YulString, AssignedValue> const& m_ssaValues;

	std::map<Statement const*, Operation> m_storeOperations;
};

}
