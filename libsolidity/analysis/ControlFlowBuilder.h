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

#include <libsolidity/analysis/ControlFlowGraph.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>

#include <array>
#include <memory>

namespace dev {
namespace solidity {

/** Helper class that builds the control flow of a function or modifier.
 * Modifiers are not yet applied to the functions. This is done in a second
 * step in the CFG class.
 */
class ControlFlowBuilder: private ASTConstVisitor
{
public:
	static std::unique_ptr<FunctionFlow> createFunctionFlow(
		CFG::NodeContainer& _nodeContainer,
		FunctionDefinition const& _function
	);

private:
	explicit ControlFlowBuilder(CFG::NodeContainer& _nodeContainer, FunctionFlow const& _functionFlow);

	// Visits for constructing the control flow.
	bool visit(BinaryOperation const& _operation) override;
	bool visit(Conditional const& _conditional) override;
	bool visit(TryStatement const& _tryStatement) override;
	bool visit(IfStatement const& _ifStatement) override;
	bool visit(ForStatement const& _forStatement) override;
	bool visit(WhileStatement const& _whileStatement) override;
	bool visit(Break const&) override;
	bool visit(Continue const&) override;
	bool visit(Throw const&) override;
	bool visit(PlaceholderStatement const&) override;
	bool visit(FunctionCall const& _functionCall) override;
	bool visit(ModifierInvocation const& _modifierInvocation) override;

	// Visits for constructing the control flow as well as filling variable occurrences.
	bool visit(FunctionDefinition const& _functionDefinition) override;
	bool visit(Return const& _return) override;

	// Visits for filling variable occurrences.
	bool visit(FunctionTypeName const& _functionTypeName) override;
	bool visit(InlineAssembly const& _inlineAssembly) override;
	bool visit(VariableDeclaration const& _variableDeclaration) override;
	bool visit(VariableDeclarationStatement const& _variableDeclarationStatement) override;
	bool visit(Identifier const& _identifier) override;

protected:
	bool visitNode(ASTNode const&) override;

private:

	/// Appends the control flow of @a _node to the current control flow.
	void appendControlFlow(ASTNode const& _node);

	/// Starts at @a _entry and parses the control flow of @a _node.
	/// @returns The node at which the parsed control flow ends.
	/// m_currentNode is not affected (it is saved and restored).
	CFGNode* createFlow(CFGNode* _entry, ASTNode const& _node);

	/// Creates an arc from @a _from to @a _to.
	static void connect(CFGNode* _from, CFGNode* _to);

	/// Splits the control flow starting at the current node into n paths.
	/// m_currentNode is set to nullptr and has to be set manually or
	/// using mergeFlow later.
	template<size_t n>
	std::array<CFGNode*, n> splitFlow()
	{
		std::array<CFGNode*, n> result;
		for (auto& node: result)
		{
			node = m_nodeContainer.newNode();
			connect(m_currentNode, node);
		}
		m_currentNode = nullptr;
		return result;
	}

	/// Splits the control flow starting at the current node into @a _n paths.
	/// m_currentNode is set to nullptr and has to be set manually or
	/// using mergeFlow later.
	std::vector<CFGNode*> splitFlow(size_t n)
	{
		std::vector<CFGNode*> result(n);
		for (auto& node: result)
		{
			node = m_nodeContainer.newNode();
			connect(m_currentNode, node);
		}
		m_currentNode = nullptr;
		return result;
	}

	/// Merges the control flow of @a _nodes to @a _endNode.
	/// If @a _endNode is nullptr, a new node is creates and used as end node.
	/// Sets the merge destination as current node.
	/// Note: @a _endNode may be one of the nodes in @a _nodes.
	template<typename C>
	void mergeFlow(C const& _nodes, CFGNode* _endNode = nullptr)
	{
		CFGNode* mergeDestination = (_endNode == nullptr) ? m_nodeContainer.newNode() : _endNode;
		for (auto& node: _nodes)
			if (node != mergeDestination)
				connect(node, mergeDestination);
		m_currentNode = mergeDestination;
	}

	CFGNode* newLabel();
	CFGNode* createLabelHere();
	void placeAndConnectLabel(CFGNode *_node);

	CFG::NodeContainer& m_nodeContainer;

	CFGNode* m_currentNode = nullptr;
	CFGNode* m_returnNode = nullptr;
	CFGNode* m_revertNode = nullptr;

	/// The current jump destination of break Statements.
	CFGNode* m_breakJump = nullptr;
	/// The current jump destination of continue Statements.
	CFGNode* m_continueJump = nullptr;

	CFGNode* m_placeholderEntry = nullptr;
	CFGNode* m_placeholderExit = nullptr;

	/// Helper class that replaces the break and continue jump destinations for the
	/// current scope and restores the originals at the end of the scope.
	class BreakContinueScope
	{
	public:
		BreakContinueScope(ControlFlowBuilder& _parser, CFGNode* _breakJump, CFGNode* _continueJump);
		~BreakContinueScope();
	private:
		ControlFlowBuilder& m_parser;
		CFGNode* m_origBreakJump;
		CFGNode* m_origContinueJump;
	};
};

}
}
