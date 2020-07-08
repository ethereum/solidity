// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolidity/analysis/ControlFlowGraph.h>
#include <set>

namespace solidity::frontend
{

class ControlFlowAnalyzer: private ASTConstVisitor
{
public:
	explicit ControlFlowAnalyzer(CFG const& _cfg, langutil::ErrorReporter& _errorReporter):
		m_cfg(_cfg), m_errorReporter(_errorReporter) {}

	bool analyze(ASTNode const& _astRoot);

	bool visit(FunctionDefinition const& _function) override;

private:
	/// Checks for uninitialized variable accesses in the control flow between @param _entry and @param _exit.
	void checkUninitializedAccess(CFGNode const* _entry, CFGNode const* _exit) const;
	/// Checks for unreachable code, i.e. code ending in @param _exit, @param _revert or @param _transactionReturn
	/// that can not be reached from @param _entry.
	void checkUnreachable(CFGNode const* _entry, CFGNode const* _exit, CFGNode const* _revert, CFGNode const* _transactionReturn) const;

	CFG const& m_cfg;
	langutil::ErrorReporter& m_errorReporter;
};

}
