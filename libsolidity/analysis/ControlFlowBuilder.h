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
	static std::unique_ptr<ModifierFlow> createModifierFlow(
		CFG::NodeContainer& _nodeContainer,
		ModifierDefinition const& _modifier
	);

private:
	explicit ControlFlowBuilder(CFG::NodeContainer& _nodeContainer, FunctionFlow const& _functionFlow);

	virtual bool visit(BinaryOperation const& _operation) override;
	virtual bool visit(Conditional const& _conditional) override;
	virtual bool visit(IfStatement const& _ifStatement) override;
	virtual bool visit(ForStatement const& _forStatement) override;
	virtual bool visit(WhileStatement const& _whileStatement) override;
	virtual bool visit(Break const&) override;
	virtual bool visit(Continue const&) override;
	virtual bool visit(Throw const&) override;
	virtual bool visit(Block const&) override;
	virtual void endVisit(Block const&) override;
	virtual bool visit(Return const& _return) override;
	virtual bool visit(PlaceholderStatement const&) override;
	virtual bool visit(FunctionCall const& _functionCall) override;


	/// Appends the control flow of @a _node to the current control flow.
	void appendControlFlow(ASTNode const& _node);

	/// Starts at @a _entry and parses the control flow of @a _node.
	/// @returns The node at which the parsed control flow ends.
	/// m_currentNode is not affected (it is saved and restored).
	CFGNode* createFlow(CFGNode* _entry, ASTNode const& _node);

	/// Creates an arc from @a _from to @a _to.
	static void connect(CFGNode* _from, CFGNode* _to);


protected:
	virtual bool visitNode(ASTNode const& node) override;

private:

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

	/// Merges the control flow of @a _nodes to @a _endNode.
	/// If @a _endNode is nullptr, a new node is creates and used as end node.
	/// Sets the merge destination as current node.
	/// Note: @a _endNode may be one of the nodes in @a _nodes.
	template<size_t n>
	void mergeFlow(std::array<CFGNode*, n> const& _nodes, CFGNode* _endNode = nullptr)
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

	/// The control flow of the function that is currently parsed.
	/// Note: this can also be a ModifierFlow
	FunctionFlow const& m_currentFunctionFlow;

	CFGNode* m_currentNode = nullptr;

	/// The current jump destination of break Statements.
	CFGNode* m_breakJump = nullptr;
	/// The current jump destination of continue Statements.
	CFGNode* m_continueJump = nullptr;

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
