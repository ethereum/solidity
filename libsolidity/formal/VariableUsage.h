// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <libsolidity/ast/ASTVisitor.h>

#include <vector>
#include <set>

namespace solidity::frontend::smt
{

/**
 * This class computes information about which variables are modified in a certain subtree.
 */
class VariableUsage: private ASTConstVisitor
{
public:
	/// @param _outerCallstack the current callstack in the callers context.
	std::set<VariableDeclaration const*> touchedVariables(ASTNode const& _node, std::vector<CallableDeclaration const*> const& _outerCallstack);

	/// Sets whether to inline function calls.
	void setFunctionInlining(bool _inlineFunction) { m_inlineFunctionCalls = _inlineFunction; }

private:
	void endVisit(Identifier const& _node) override;
	void endVisit(IndexAccess const& _node) override;
	void endVisit(FunctionCall const& _node) override;
	bool visit(FunctionDefinition const& _node) override;
	void endVisit(FunctionDefinition const& _node) override;
	void endVisit(ModifierInvocation const& _node) override;
	void endVisit(PlaceholderStatement const& _node) override;

	/// Checks whether an identifier should be added to touchedVariables.
	void checkIdentifier(Identifier const& _identifier);

	std::set<VariableDeclaration const*> m_touchedVariables;
	std::vector<CallableDeclaration const*> m_callStack;
	CallableDeclaration const* m_lastCall = nullptr;

	bool m_inlineFunctionCalls = false;
};

}
