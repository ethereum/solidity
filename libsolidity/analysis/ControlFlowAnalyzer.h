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

#include <libsolidity/analysis/ControlFlowGraph.h>
#include <liblangutil/ErrorReporter.h>
#include <set>

namespace solidity::frontend
{

class ControlFlowAnalyzer
{
public:
	explicit ControlFlowAnalyzer(CFG const& _cfg, langutil::ErrorReporter& _errorReporter):
		m_cfg(_cfg), m_errorReporter(_errorReporter) {}

	bool run();

private:
	void analyze(FunctionDefinition const& _function, ContractDefinition const* _contract, FunctionFlow const& _flow);
	/// Checks for uninitialized variable accesses in the control flow between @param _entry and @param _exit.
	/// @param _entry entry node
	/// @param _exit exit node
	/// @param _emptyBody whether the body of the function is empty (true) or not (false)
	/// @param _contractName name of the most derived contract, should be empty
	///        if the function is also defined in it
	void checkUninitializedAccess(CFGNode const* _entry, CFGNode const* _exit, bool _emptyBody, std::optional<std::string> _contractName = {});
	/// Checks for unreachable code, i.e. code ending in @param _exit, @param _revert or @param _transactionReturn
	/// that can not be reached from @param _entry.
	void checkUnreachable(CFGNode const* _entry, CFGNode const* _exit, CFGNode const* _revert, CFGNode const* _transactionReturn);

	CFG const& m_cfg;
	langutil::ErrorReporter& m_errorReporter;

	std::set<langutil::SourceLocation> m_unreachableLocationsAlreadyWarnedFor;
	std::set<VariableDeclaration const*> m_unassignedReturnVarsAlreadyWarnedFor;
};

}
