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
using namespace langutil;
using namespace solidity;
using namespace std;

ControlFlowBuilder::ControlFlowBuilder(CFG::NodeContainer& _nodeContainer, FunctionFlow const& _functionFlow):
	m_nodeContainer(_nodeContainer),
	m_currentNode(_functionFlow.entry),
	m_returnNode(_functionFlow.exit),
	m_revertNode(_functionFlow.revert)
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

	return functionFlow;
}

bool ControlFlowBuilder::visit(BinaryOperation const& _operation)
{
	solAssert(!!m_currentNode, "");

	switch(_operation.getOperator())
	{
		case Token::Or:
		case Token::And:
		{
			visitNode(_operation);
			appendControlFlow(_operation.leftExpression());

			auto nodes = splitFlow<2>();
			nodes[0] = createFlow(nodes[0], _operation.rightExpression());
			mergeFlow(nodes, nodes[1]);

			return false;
		}
		default:
			return ASTConstVisitor::visit(_operation);
	}
}

bool ControlFlowBuilder::visit(Conditional const& _conditional)
{
	solAssert(!!m_currentNode, "");
	visitNode(_conditional);

	_conditional.condition().accept(*this);

	auto nodes = splitFlow<2>();

	nodes[0] = createFlow(nodes[0], _conditional.trueExpression());
	nodes[1] = createFlow(nodes[1], _conditional.falseExpression());

	mergeFlow(nodes);

	return false;
}

bool ControlFlowBuilder::visit(TryStatement const& _tryStatement)
{
	appendControlFlow(_tryStatement.externalCall());

	auto nodes = splitFlow(_tryStatement.clauses().size());
	for (size_t i = 0; i < _tryStatement.clauses().size(); ++i)
		nodes[i] = createFlow(nodes[i], _tryStatement.clauses()[i]->block());
	mergeFlow(nodes);

	return false;
}

bool ControlFlowBuilder::visit(IfStatement const& _ifStatement)
{
	solAssert(!!m_currentNode, "");
	visitNode(_ifStatement);

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
	visitNode(_forStatement);

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
	visitNode(_whileStatement);

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

bool ControlFlowBuilder::visit(Break const& _break)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_breakJump, "");
	visitNode(_break);
	connect(m_currentNode, m_breakJump);
	m_currentNode = newLabel();
	return false;
}

bool ControlFlowBuilder::visit(Continue const& _continue)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_continueJump, "");
	visitNode(_continue);
	connect(m_currentNode, m_continueJump);
	m_currentNode = newLabel();
	return false;
}

bool ControlFlowBuilder::visit(Throw const& _throw)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_revertNode, "");
	visitNode(_throw);
	connect(m_currentNode, m_revertNode);
	m_currentNode = newLabel();
	return false;
}

bool ControlFlowBuilder::visit(PlaceholderStatement const&)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_placeholderEntry, "");
	solAssert(!!m_placeholderExit, "");

	connect(m_currentNode, m_placeholderEntry);
	m_currentNode = newLabel();
	connect(m_placeholderExit, m_currentNode);
	return false;
}

bool ControlFlowBuilder::visit(FunctionCall const& _functionCall)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!_functionCall.expression().annotation().type, "");

	if (auto functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type))
		switch (functionType->kind())
		{
			case FunctionType::Kind::Revert:
				solAssert(!!m_revertNode, "");
				visitNode(_functionCall);
				_functionCall.expression().accept(*this);
				ASTNode::listAccept(_functionCall.arguments(), *this);
				connect(m_currentNode, m_revertNode);
				m_currentNode = newLabel();
				return false;
			case FunctionType::Kind::Require:
			case FunctionType::Kind::Assert:
			{
				solAssert(!!m_revertNode, "");
				visitNode(_functionCall);
				_functionCall.expression().accept(*this);
				ASTNode::listAccept(_functionCall.arguments(), *this);
				connect(m_currentNode, m_revertNode);
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

bool ControlFlowBuilder::visit(ModifierInvocation const& _modifierInvocation)
{
	if (auto arguments = _modifierInvocation.arguments())
		for (auto& argument: *arguments)
			appendControlFlow(*argument);

	auto modifierDefinition = dynamic_cast<ModifierDefinition const*>(
		_modifierInvocation.name()->annotation().referencedDeclaration
	);
	if (!modifierDefinition) return false;
	solAssert(!!modifierDefinition, "");
	solAssert(!!m_returnNode, "");

	m_placeholderEntry = newLabel();
	m_placeholderExit = newLabel();

	appendControlFlow(*modifierDefinition);
	connect(m_currentNode, m_returnNode);

	m_currentNode = m_placeholderEntry;
	m_returnNode = m_placeholderExit;

	m_placeholderEntry = nullptr;
	m_placeholderExit = nullptr;

	return false;
}

bool ControlFlowBuilder::visit(FunctionDefinition const& _functionDefinition)
{
	for (auto const& parameter: _functionDefinition.parameters())
		appendControlFlow(*parameter);

	for (auto const& returnParameter: _functionDefinition.returnParameters())
	{
		appendControlFlow(*returnParameter);
		m_returnNode->variableOccurrences.emplace_back(
			*returnParameter,
			VariableOccurrence::Kind::Return,
			nullptr
		);

	}

	for (auto const& modifier: _functionDefinition.modifiers())
		appendControlFlow(*modifier);

	appendControlFlow(_functionDefinition.body());

	connect(m_currentNode, m_returnNode);
	m_currentNode = nullptr;

	return false;
}

bool ControlFlowBuilder::visit(Return const& _return)
{
	solAssert(!!m_currentNode, "");
	solAssert(!!m_returnNode, "");
	visitNode(_return);
	if (_return.expression())
	{
		appendControlFlow(*_return.expression());
		// Returns with return expression are considered to be assignments to the return parameters.
		for (auto returnParameter: _return.annotation().functionReturnParameters->parameters())
			m_currentNode->variableOccurrences.emplace_back(
				*returnParameter,
				VariableOccurrence::Kind::Assignment,
				&_return
			);
	}
	connect(m_currentNode, m_returnNode);
	m_currentNode = newLabel();
	return false;
}

bool ControlFlowBuilder::visit(FunctionTypeName const& _functionTypeName)
{
	visitNode(_functionTypeName);
	// Do not visit the parameters and return values of a function type name.
	// We do not want to consider them as variable declarations for the control flow graph.
	return false;
}

bool ControlFlowBuilder::visit(InlineAssembly const& _inlineAssembly)
{
	solAssert(!!m_currentNode, "");
	visitNode(_inlineAssembly);
	for (auto const& ref: _inlineAssembly.annotation().externalReferences)
	{
		if (auto variableDeclaration = dynamic_cast<VariableDeclaration const*>(ref.second.declaration))
			m_currentNode->variableOccurrences.emplace_back(
				*variableDeclaration,
				VariableOccurrence::Kind::InlineAssembly,
				&_inlineAssembly
			);
	}
	return true;
}

bool ControlFlowBuilder::visit(VariableDeclaration const& _variableDeclaration)
{
	solAssert(!!m_currentNode, "");
	visitNode(_variableDeclaration);

	m_currentNode->variableOccurrences.emplace_back(
		_variableDeclaration,
		VariableOccurrence::Kind::Declaration,
		nullptr
	);

	// Handle declaration with immediate assignment.
	if (_variableDeclaration.value())
		m_currentNode->variableOccurrences.emplace_back(
			_variableDeclaration,
			VariableOccurrence::Kind::Assignment,
			_variableDeclaration.value().get()
		);
	// Function arguments are considered to be immediately assigned as well (they are "externally assigned").
	else if (_variableDeclaration.isCallableOrCatchParameter() && !_variableDeclaration.isReturnParameter())
		m_currentNode->variableOccurrences.emplace_back(
			_variableDeclaration,
			VariableOccurrence::Kind::Assignment,
			nullptr
		);
	return true;
}

bool ControlFlowBuilder::visit(VariableDeclarationStatement const& _variableDeclarationStatement)
{
	solAssert(!!m_currentNode, "");
	visitNode(_variableDeclarationStatement);

	for (auto const& var: _variableDeclarationStatement.declarations())
		if (var)
			var->accept(*this);
	if (_variableDeclarationStatement.initialValue())
	{
		_variableDeclarationStatement.initialValue()->accept(*this);
		for (size_t i = 0; i < _variableDeclarationStatement.declarations().size(); i++)
			if (auto const& var = _variableDeclarationStatement.declarations()[i])
			{
				auto expression = _variableDeclarationStatement.initialValue();
				if (auto tupleExpression = dynamic_cast<TupleExpression const*>(expression))
					if (tupleExpression->components().size() > 1)
					{
						solAssert(tupleExpression->components().size() > i, "");
						expression = tupleExpression->components()[i].get();
					}
				while (auto tupleExpression = dynamic_cast<TupleExpression const*>(expression))
					if (tupleExpression->components().size() == 1)
						expression = tupleExpression->components().front().get();
					else
						break;
				m_currentNode->variableOccurrences.emplace_back(
					*var,
					VariableOccurrence::Kind::Assignment,
					expression
				);
			}
	}
	return false;
}

bool ControlFlowBuilder::visit(Identifier const& _identifier)
{
	solAssert(!!m_currentNode, "");
	visitNode(_identifier);

	if (auto const* variableDeclaration = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
		m_currentNode->variableOccurrences.emplace_back(
			*variableDeclaration,
			static_cast<Expression const&>(_identifier).annotation().lValueRequested ?
			VariableOccurrence::Kind::Assignment :
			VariableOccurrence::Kind::Access,
			&_identifier
		);

	return true;
}

bool ControlFlowBuilder::visitNode(ASTNode const& _node)
{
	solAssert(!!m_currentNode, "");
	m_currentNode->location = langutil::SourceLocation::smallestCovering(m_currentNode->location, _node.location());
	return true;
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
