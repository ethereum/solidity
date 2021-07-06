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

#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/analysis/ControlFlowGraph.h>

#include <libsolutil/Algorithms.h>

namespace solidity::frontend
{

/**
 * Analyses all function flows and recursively removes all exit edges from CFG
 * nodes that make function calls that will always revert.
 */
class ControlFlowRevertPruner
{
public:
	ControlFlowRevertPruner(CFG& _cfg): m_cfg(_cfg) {}

	void run();
private:
	/// Possible revert states of a function call
	enum class RevertState
	{
		AllPathsRevert,
		HasNonRevertingPath,
		Unknown,
	};

	/// Identify revert states of all function flows
	void findRevertStates();

	/// Modify function flows so that edges with reverting function calls are removed
	void modifyFunctionFlows();

	/// Control Flow Graph object.
	CFG& m_cfg;

	/// function/contract pairs mapped to their according revert state
	std::map<CFG::FunctionContractTuple, RevertState> m_functions;

	std::map<
		std::tuple<FunctionCall const*, ContractDefinition const*>,
		FunctionDefinition const*
	> m_resolveCache;

};
}
