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

#include <libsolidity/analysis/ControlFlowBuilder.h>

using namespace dev;
using namespace solidity;
using namespace std;

ControlFlowBuilder::ControlFlowBuilder(CFG::NodeContainer& _nodeContainer, FunctionFlow const& _functionFlow):
	m_nodeContainer(_nodeContainer), m_currentFunctionFlow(_functionFlow), m_currentNode(_functionFlow.entry)
{
}

unique_ptr<FunctionFlow> ControlFlowBuilder::createFunctionFlow(
	CFG::NodeContainer& _nodeContainer,
	FunctionDefinition const& _function
)
{
	auto functionFlow = unique_ptr<FunctionFlow>(new FunctionFlow());
	functionFlow->entry = _nodeContainer.newNode();
	functionFlow->exit = _nodeContainer.newNode();
	functionFlow->revert = _nodeContainer.newNode();
	ControlFlowBuilder builder(_nodeContainer, *functionFlow);
	builder.appendControlFlow(_function);
	connect(builder.m_currentNode, functionFlow->exit);
	return functionFlow;
}


unique_ptr<ModifierFlow> ControlFlowBuilder::createModifierFlow(
	CFG::NodeContainer& _nodeContainer,
	ModifierDefinition const& _modifier
)
{
	auto modifierFlow = unique_ptr<ModifierFlow>(new ModifierFlow());
	modifierFlow->entry = _nodeContainer.newNode();
	modifierFlow->exit = _nodeContainer.newNode();
	modifierFlow->revert = _nodeContainer.newNode();
	modifierFlow->placeholderEntry = _nodeContainer.newNode();
	modifierFlow->placeholderExit = _nodeContainer.newNode();
	ControlFlowBuilder builder(_nodeContainer, *modifierFlow);
	builder.appendControlFlow(_modifier);
	connect(builder.m_currentNode, modifierFlow->exit);
	return modifierFlow;
}

bool ControlFlowBuilder::visit(BinaryOperation const& _operation)
{
	solAssert(!!m_currentNode, "");

	switch(_operation.getOperator())
	{
		case Token::Or:
		case Token::And:
		{
			appendControlFlow(_operation.leftExpression());

			auto nodes = splitFlow<2>();
			nodes[0] = createFlow(nodes[0], _operation.rightExpression());
			mergeFlow(nodes, nodes[1]);

			return false;
		}
		default:
			break;
	}
	return ASTConstVisitor::visit(_operation);
}

bool ControlFlowBuilder::visit(Conditional const& _conditional)
{
	solAssert(!!m_currentNode, "");

	_conditional.condition().accept(*this);

	auto nodes = splitFlow<2>();

	nodes[0] = createFlow(nodes[0], _conditional.trueExpression());
	nodes[1] = createFlow(nodes[1], _conditional.falseExpression());

	mergeFlow(nodes);

	return false;
}

bool ControlFlowBuilder::visit(IfStatement const& _ifStatement)
{
	solAssert(!!m_currentNode, "");

	_ifStatement.condition().accept(*this);

	auto nodes = splitFlow<2>();
	nodes[0] = createFlow(nodes[0], _ifStatement.trueStatement());

	if (_ifStatement.falseStatement())
	{
		nodes[1] = createFlow(nodes[1], *_ifStatement.falseStatement());
		mergeFlow(nodes);
	}
	else
		mergeFlow(nodes, nodes[1]);

	return false;
}

bool ControlFlowBuilder::visit(ForStatement const& _forStatement)
{
	solAssert(!!m_currentNode, "");

	if (_forStatement.initializationExpression())
		_forStatement.initializationExpression()->accept(*this);

	auto condition = createLabelHere();

	if (_forStatement.condition())
		appendControlFlow(*_forStatement.condition());

	auto loopExpression = newLabel();
	auto nodes = splitFlow<2>();
	auto afterFor = nodes[1];
	m_currentNode = nodes[0];

	{
		BreakContinueScope scope(*this, afterFor, loopExpression);
		appendControlFlow(_forStatement.body());
	}

	placeAndConnectLabel(loopExpression);

	if (auto expression = _forStatement.loopExpression())
		appendControlFlow(*expression);

	connect(m_currentNode, condition);
	m_currentNode = afterFor;

	return false;
}

bool ControlFlowBuilder::visit(WhileStatement const& _whileStatement)
{
	solAssert(!!m_currentNode, "");

	if (_whileStatement.isDoWhile())
	{
		auto afterWhile = newLabel();
		auto whileBody = createLabelHere();
		auto condition = newLabel();

		{
			BreakContinueScope scope(*this, afterWhile, condition);
			appendControlFlow(_whileStatement.body());
		}

		placeAndConnectLabel(condition);
		appendControlFlow(_whileStatement.condition());

		connect(m_currentNode, whileBody);
		placeAndConnectLabel(afterWhile);
	}
	else
	{
		auto whileCondition = createLabelHere();

		appendControlFlow(_whileStatement.condition());

		auto nodes = splitFlow<2>();

		auto whileBody = nodes[0];
		auto afterWhile = nodes[1];

		m_currentNode = whileBody;
		{
			BreakContinueScope scope(*this, afterWhile, whileCondition);
			appendControlFlow(_whileStatement.body());
		}

		connect(m_currentNode, whileCondition);

		m_currentNode = afterWhile;
	}


	return false;
}

bool ControlFlowBuilder::visit(Break const&)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_breakJump, "");
	connect(m_currentNode, m_breakJump);
	m_currentNode = newLabel();
	return false;
}

bool ControlFlowBuilder::visit(Continue const&)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_continueJump, "");
	connect(m_currentNode, m_continueJump);
	m_currentNode = newLabel();
	return false;
}

bool ControlFlowBuilder::visit(Throw const&)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_currentFunctionFlow.revert, "");
	connect(m_currentNode, m_currentFunctionFlow.revert);
	m_currentNode = newLabel();
	return false;
}

bool ControlFlowBuilder::visit(Block const&)
{
	solAssert(!!m_currentNode, "");
	createLabelHere();
	return true;
}

void ControlFlowBuilder::endVisit(Block const&)
{
	solAssert(!!m_currentNode, "");
	createLabelHere();
}

bool ControlFlowBuilder::visit(Return const& _return)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_currentFunctionFlow.exit, "");
	solAssert(!m_currentNode->block.returnStatement, "");
	m_currentNode->block.returnStatement = &_return;
	connect(m_currentNode, m_currentFunctionFlow.exit);
	m_currentNode = newLabel();
	return true;
}


bool ControlFlowBuilder::visit(PlaceholderStatement const&)
{
	solAssert(!!m_currentNode, "");
	auto modifierFlow = dynamic_cast<ModifierFlow const*>(&m_currentFunctionFlow);
	solAssert(!!modifierFlow, "");

	connect(m_currentNode, modifierFlow->placeholderEntry);

	m_currentNode = newLabel();

	connect(modifierFlow->placeholderExit, m_currentNode);
	return false;
}

bool ControlFlowBuilder::visitNode(ASTNode const& node)
{
	solAssert(!!m_currentNode, "");
	if (auto const* expression = dynamic_cast<Expression const*>(&node))
		m_currentNode->block.expressions.emplace_back(expression);
	else if (auto const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(&node))
		m_currentNode->block.variableDeclarations.emplace_back(variableDeclaration);
	else if (auto const* assembly = dynamic_cast<InlineAssembly const*>(&node))
		m_currentNode->block.inlineAssemblyStatements.emplace_back(assembly);

	return true;
}

bool ControlFlowBuilder::visit(FunctionCall const& _functionCall)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!_functionCall.expression().annotation().type, "");

	if (auto functionType = dynamic_pointer_cast<FunctionType const>(_functionCall.expression().annotation().type))
		switch (functionType->kind())
		{
			case FunctionType::Kind::Revert:
				solAssert(!!m_currentFunctionFlow.revert, "");
				_functionCall.expression().accept(*this);
				ASTNode::listAccept(_functionCall.arguments(), *this);
				connect(m_currentNode, m_currentFunctionFlow.revert);
				m_currentNode = newLabel();
				return false;
			case FunctionType::Kind::Require:
			case FunctionType::Kind::Assert:
			{
				solAssert(!!m_currentFunctionFlow.revert, "");
				_functionCall.expression().accept(*this);
				ASTNode::listAccept(_functionCall.arguments(), *this);
				connect(m_currentNode, m_currentFunctionFlow.revert);
				auto nextNode = newLabel();
				connect(m_currentNode, nextNode);
				m_currentNode = nextNode;
				return false;
			}
			default:
				break;
		}
	return ASTConstVisitor::visit(_functionCall);
}

void ControlFlowBuilder::appendControlFlow(ASTNode const& _node)
{
	_node.accept(*this);
}

CFGNode* ControlFlowBuilder::createFlow(CFGNode* _entry, ASTNode const& _node)
{
	auto oldCurrentNode = m_currentNode;
	m_currentNode = _entry;
	appendControlFlow(_node);
	auto endNode = m_currentNode;
	m_currentNode = oldCurrentNode;
	return endNode;
}

void ControlFlowBuilder::connect(CFGNode* _from, CFGNode* _to)
{
	solAssert(_from, "");
	solAssert(_to, "");
	_from->exits.push_back(_to);
	_to->entries.push_back(_from);
}

CFGNode* ControlFlowBuilder::newLabel()
{
	return m_nodeContainer.newNode();
}

CFGNode* ControlFlowBuilder::createLabelHere()
{
	auto label = m_nodeContainer.newNode();
	connect(m_currentNode, label);
	m_currentNode = label;
	return label;
}

void ControlFlowBuilder::placeAndConnectLabel(CFGNode* _node)
{
	connect(m_currentNode, _node);
	m_currentNode = _node;
}

ControlFlowBuilder::BreakContinueScope::BreakContinueScope(
	ControlFlowBuilder& _parser,
	CFGNode* _breakJump,
	CFGNode* _continueJump
): m_parser(_parser), m_origBreakJump(_parser.m_breakJump), m_origContinueJump(_parser.m_continueJump)
{
	m_parser.m_breakJump = _breakJump;
	m_parser.m_continueJump = _continueJump;
}

ControlFlowBuilder::BreakContinueScope::~BreakContinueScope()
{
	m_parser.m_breakJump = m_origBreakJump;
	m_parser.m_continueJump = m_origContinueJump;
}
