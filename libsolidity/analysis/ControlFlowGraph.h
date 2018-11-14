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

#pragma once

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <liblangutil/ErrorReporter.h>

#include <map>
#include <memory>
#include <stack>
#include <vector>

namespace dev
{
namespace solidity
{

/** Basic Control Flow Block.
 * Basic block of control flow. Consists of a set of (unordered) AST nodes
 * for which control flow is always linear. A basic control flow block
 * encompasses at most one scope. Reverts are considered to break the control
 * flow.
 * @todo Handle function calls correctly. So far function calls are not considered
 * to change the control flow.
 */
struct ControlFlowBlock
{
	/// All variable declarations inside this control flow block.
	std::vector<VariableDeclaration const*> variableDeclarations;
	/// All expressions inside this control flow block (this includes all subexpressions!).
	std::vector<Expression const*> expressions;
	/// All inline assembly statements inside in this control flow block.
	std::vector<InlineAssembly const*> inlineAssemblyStatements;
	/// If control flow returns in this node, the return statement is stored in returnStatement,
	/// otherwise returnStatement is nullptr.
	Return const* returnStatement = nullptr;
};

/** Node of the Control Flow Graph.
  * The control flow is a directed graph connecting control flow blocks.
  * An arc between two nodes indicates that the control flow can possibly
  * move from its start node to its end node during execution.
  */
struct CFGNode
{
	/// Entry nodes. All CFG nodes from which control flow may move into this node.
	std::vector<CFGNode*> entries;
	/// Exit nodes. All CFG nodes to which control flow may continue after this node.
	std::vector<CFGNode*> exits;

	/// Control flow in the node.
	ControlFlowBlock block;
};

/** Describes the control flow of a function. */
struct FunctionFlow
{
	virtual ~FunctionFlow() {}
	/// Entry node. Control flow of the function starts here.
	/// This node is empty and does not have any entries.
	CFGNode* entry = nullptr;
	/// Exit node. All non-reverting control flow of the function ends here.
	/// This node is empty and does not have any exits, but may have multiple entries
	/// (e.g. all return statements of the function).
	CFGNode* exit = nullptr;
	/// Revert node. Control flow of the function in case of revert.
	/// This node is empty does not have any exits, but may have multiple entries
	/// (e.g. all assert, require, revert and throw statements).
	CFGNode* revert = nullptr;
};

/** Describes the control flow of a modifier.
 * Every placeholder breaks the control flow. The node preceding the
 * placeholder is assigned placeholderEntry as exit and the node
 * following the placeholder is assigned placeholderExit as entry.
 */
struct ModifierFlow: FunctionFlow
{
	/// Control flow leading towards a placeholder exit in placeholderEntry.
	CFGNode* placeholderEntry = nullptr;
	/// Control flow coming from a placeholder enter from placeholderExit.
	CFGNode* placeholderExit = nullptr;
};

class CFG: private ASTConstVisitor
{
public:
	explicit CFG(ErrorReporter& _errorReporter): m_errorReporter(_errorReporter) {}

	bool constructFlow(ASTNode const& _astRoot);

	bool visit(ModifierDefinition const& _modifier) override;
	bool visit(FunctionDefinition const& _function) override;

	FunctionFlow const& functionFlow(FunctionDefinition const& _function) const;

	class NodeContainer
	{
	public:
		CFGNode* newNode();
	private:
		std::vector<std::unique_ptr<CFGNode>> m_nodes;
	};
private:
	/// Initially the control flow for all functions *ignoring* modifiers and for
	/// all modifiers is constructed. Afterwards the control flow of functions
	/// is adjusted by applying all modifiers.
	void applyModifiers();

	/// Creates a copy of the modifier flow @a _modifierFlow, while replacing the
	/// placeholder entry and exit with the function entry and exit, as well as
	/// replacing the modifier revert node with the function's revert node.
	/// The resulting control flow is the new function flow with the modifier applied.
	/// @a _functionFlow is updated in-place.
	void applyModifierFlowToFunctionFlow(
		ModifierFlow const& _modifierFlow,
		FunctionFlow* _functionFlow
	);

	ErrorReporter& m_errorReporter;

	/// Node container.
	/// All nodes allocated during the construction of the control flow graph
	/// are owned by the CFG class and stored in this container.
	NodeContainer m_nodeContainer;

	std::map<FunctionDefinition const*, std::unique_ptr<FunctionFlow>> m_functionControlFlow;
	std::map<ModifierDefinition const*, std::unique_ptr<ModifierFlow>> m_modifierControlFlow;
};

}
}
