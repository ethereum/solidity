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

#include <libsolidity/analysis/CallgraphBuilder.h>

#include <libsolidity/ast/ASTVisitor.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

// If this is in the header, we get weird errors.
inline CallgraphNode operator+(CallgraphNode _left, CallgraphNode const& _right)
{
	_left += _right;
	return _left;
}



class CallgraphBuilderVisitor: public ASTConstVisitor
{
public:
	CallgraphBuilderVisitor(map<ASTNode const*, CallgraphNode>& _nodes): m_nodes(_nodes) {}

	virtual void endVisit(SourceUnit const& _sourceUnit) override
	{
		CallgraphNode& node = m_nodes[&_sourceUnit];
		for (auto const& n: _sourceUnit.nodes())
			node += m_nodes.at(n.get());
	}
	virtual void endVisit(PragmaDirective const& _pragma) override
	{
		m_nodes[&_pragma];
	}
	virtual void endVisit(ImportDirective const& _import) override
	{
		m_nodes[&_import];
	}
	virtual void endVisit(ContractDefinition const& _contract) override
	{
		CallgraphNode& node = m_nodes[&_contract];
		for (auto const& c: _contract.baseContracts())
			node += m_nodes.at(c.get());
		for (auto const& n: _contract.subNodes())
			node += m_nodes.at(n.get());
	}
	virtual void endVisit(InheritanceSpecifier const& _inheritance) override
	{
		CallgraphNode& node = m_nodes[&_inheritance];
		node += m_nodes.at(&_inheritance.name());
		for (auto const& arg: _inheritance.arguments())
			node += m_nodes.at(arg.get());
	}
	virtual void endVisit(UsingForDirective const& _usingFor) override
	{
		m_nodes[&_usingFor] = m_nodes.at(&_usingFor.libraryName());
		if (_usingFor.typeName())
			m_nodes[&_usingFor] += m_nodes.at(_usingFor.typeName());
	}
	virtual void endVisit(StructDefinition const& _struct) override
	{
		CallgraphNode& node = m_nodes[&_struct];
		for (auto const& m: _struct.members())
			node += m_nodes.at(m.get());
	}
	virtual void endVisit(EnumDefinition const& _enum) override
	{
		CallgraphNode& node = m_nodes[&_enum];
		for (auto const& m: _enum.members())
			node += m_nodes.at(m.get());
	}
	virtual void endVisit(EnumValue const& _value) override { m_nodes[&_value]; }
	virtual void endVisit(ParameterList const& _params) override
	{
		CallgraphNode& node = m_nodes[&_params];
		for (auto const& p: _params.parameters())
			node += m_nodes.at(p.get());
	}
	virtual void endVisit(FunctionDefinition const& _funDef) override
	{
		CallgraphNode& node = m_nodes[&_funDef];
		node += m_nodes.at(&_funDef.parameterList());
		if (_funDef.returnParameterList())
			node += m_nodes.at(_funDef.returnParameterList().get());
		for (auto const& mod: _funDef.modifiers())
			// This only includes the effects of the arguments, not the modifier itself
			node += m_nodes.at(mod.get());

		node += m_nodes.at(&_funDef.body());

		node += CallgraphNode::anything();
	}
	virtual void endVisit(VariableDeclaration const& _varDecl) override
	{
		m_nodes[&_varDecl];
		if (_varDecl.value())
			m_nodes[&_varDecl] += m_nodes.at(_varDecl.value().get());
	}
	virtual void endVisit(ModifierDefinition const& _modifier) override
	{
		CallgraphNode& node = m_nodes[&_modifier];
		node += m_nodes.at(&_modifier.parameterList());
		if (_modifier.returnParameterList())
			node += m_nodes.at(_modifier.returnParameterList().get());
		node += m_nodes.at(&_modifier.body());
	}
	virtual void endVisit(ModifierInvocation const& _invocation) override
	{
		// This is more like a function call, but the actual effects
		// are handled on the function only.
		CallgraphNode& node = m_nodes[&_invocation];
		for (auto const& arg: _invocation.arguments())
			node += m_nodes.at(arg.get());
	}
	virtual void endVisit(EventDefinition const& _event) override
	{
		m_nodes[&_event] = m_nodes.at(&_event.parameterList());
		if (_event.returnParameterList())
			m_nodes[&_event] += m_nodes.at(_event.returnParameterList().get());
	}
	virtual void endVisit(ElementaryTypeName const& _typeName) override
	{
		m_nodes[&_typeName];
	}
	virtual void endVisit(UserDefinedTypeName const& _typeName) override
	{
		m_nodes[&_typeName];
	}
	virtual void endVisit(FunctionTypeName const& _typeName) override
	{
		m_nodes[&_typeName] = m_nodes.at(_typeName.parameterTypeList().get());
		if (_typeName.returnParameterTypeList())
			m_nodes[&_typeName] += m_nodes.at(_typeName.returnParameterTypeList().get());
	}
	virtual void endVisit(Mapping const& _mapping) override
	{
		m_nodes[&_mapping] = m_nodes.at(&_mapping.keyType()) + m_nodes.at(&_mapping.valueType());
	}
	virtual void endVisit(ArrayTypeName const& _typeName) override
	{
		m_nodes[&_typeName] = m_nodes.at(&_typeName.baseType());
		if (_typeName.length())
			m_nodes[&_typeName] += m_nodes.at(_typeName.length());
	}
	virtual void endVisit(Block const& _block) override
	{
		// @TODO control flow
		CallgraphNode& node = m_nodes[&_block];
		for (auto const& statement: _block.statements())
			node += m_nodes.at(statement.get());
	}
	virtual void endVisit(PlaceholderStatement const& _placeholder) override
	{
		// @TODO control flow
		m_nodes[&_placeholder];
	}
	virtual void endVisit(IfStatement const& _ifStatement) override
	{
		// @TODO control flow
		m_nodes[&_ifStatement] =
			m_nodes.at(&_ifStatement.condition()) +
			m_nodes.at(&_ifStatement.trueStatement());
		if (_ifStatement.falseStatement())
			m_nodes[&_ifStatement] += m_nodes.at(_ifStatement.falseStatement());
	}
	virtual void endVisit(WhileStatement const& _whileStatement) override
	{
		// @TODO control flow
		// @TODO control flow depends on _whileStatement.isDoWhile()
		m_nodes[&_whileStatement] =
			m_nodes.at(&_whileStatement.condition()) +
			m_nodes.at(&_whileStatement.body());
	}
	virtual void endVisit(ForStatement const& _forStatement) override
	{
		// @TODO control flow
		CallgraphNode& node = m_nodes[&_forStatement];
		if (_forStatement.initializationExpression())
			node += m_nodes.at(_forStatement.initializationExpression());
		if (_forStatement.condition())
			node += m_nodes.at(_forStatement.condition());
		node += m_nodes[&_forStatement.body()];
		if (_forStatement.loopExpression())
			node += m_nodes.at(_forStatement.loopExpression());
	}
	virtual void endVisit(Break const& _break) override
	{
		// @TODO control flow
		m_nodes[&_break].hasSideEffects = true;
	}
	virtual void endVisit(Continue const& _continue) override
	{
		m_nodes[&_continue].hasSideEffects = true;
	}
	virtual void endVisit(InlineAssembly const& _inlineAsm) override
	{
		m_nodes[&_inlineAsm] = CallgraphNode::anything();
		// TODO we could analyse the inline assembly further.
		// TDOO control flow?
	}
	virtual void endVisit(Return const& _return) override
	{
		// @TODO control flow
		m_nodes[&_return].hasSideEffects = true;

		if (Expression const* expression = _return.expression())
		{
			CallgraphNode& node = m_nodes[&_return];
			solAssert(_return.annotation().functionReturnParameters, "Invalid return parameters pointer.");
			vector<ASTPointer<VariableDeclaration>> const& returnParameters =
				_return.annotation().functionReturnParameters->parameters();
			TypePointers variableTypes;
			for (auto const& retVariable: returnParameters)
				variableTypes.push_back(retVariable->annotation().type);

			TypePointers givenTypes;
			if (expression->annotation().type->category() == Type::Category::Tuple)
				givenTypes = dynamic_cast<TupleType const&>(*expression->annotation().type).components();
			else
				givenTypes.push_back(expression->annotation().type);
			solAssert(returnParameters.size() == givenTypes.size(), "");

			for (size_t i = 0; i < returnParameters.size(); ++i)
				node += processAssignment(
					returnParameters[i].get(),
					*returnParameters[i]->annotation().type,
					*givenTypes[i]
				);
		}
	}
	virtual void endVisit(Throw const& _node) override
	{
		m_nodes[&_node].hasSideEffects = true;
	}
	virtual void endVisit(VariableDeclarationStatement const& _variableDeclarationStatement) override
	{
		m_nodes[&_variableDeclarationStatement];
		if (Expression const* expression = _variableDeclarationStatement.initialValue())
		{
			CallgraphNode& node = m_nodes[&_variableDeclarationStatement] = m_nodes.at(expression);
			TypePointers valueTypes;
			if (auto tupleType = dynamic_cast<TupleType const*>(expression->annotation().type.get()))
				valueTypes = tupleType->components();
			else
				valueTypes = TypePointers{expression->annotation().type};
			auto const& assignments = _variableDeclarationStatement.annotation().assignments;
			solAssert(assignments.size() == valueTypes.size(), "");
			for (size_t i = 0; i < assignments.size(); ++i)
			{
				size_t j = assignments.size() - i - 1;
				solAssert(!!valueTypes[j], "");
				VariableDeclaration const* varDecl = assignments[j];
				if (varDecl)
					node += processAssignment(
						varDecl,
						*varDecl->annotation().type,
						*valueTypes[j]
					);
			}
		}
	}
	virtual void endVisit(ExpressionStatement const& _node) override
	{
		m_nodes[&_node] = m_nodes[&_node.expression()];
	}
	virtual void endVisit(Conditional const& _node) override
	{
		// TODO control flow
		m_nodes[&_node] =
			m_nodes.at(&_node.condition()) +
			m_nodes.at(&_node.trueExpression()) +
			m_nodes.at(&_node.falseExpression());
	}
	virtual void endVisit(Assignment const& _assignment) override
	{
		// TODO this could be more intelligent with tuples.
		// TODO finding the variable that is assigned to could be made more intelligent
		// (or whether it writes to storage or not)
		m_nodes[&_assignment] =
			m_nodes.at(&_assignment.leftHandSide()) +
			m_nodes.at(&_assignment.rightHandSide()) +
			processAssignment(
				referencedVariable(_assignment.leftHandSide()),
				*_assignment.leftHandSide().annotation().type,
				*_assignment.rightHandSide().annotation().type
			);
	}
	static CallgraphNode processAssignment(
		VariableDeclaration const* leftHandSide,
		Type const& _leftType,
		Type const& _rightType
	)
	{
		CallgraphNode node;

		if (_rightType.dataStoredIn(DataLocation::Storage))
			node.readsStorage = true;

		node.hasSideEffects = true;
		node.writes.insert(leftHandSide);

		if (!_leftType.isValueType() && !_leftType.dataStoredIn(DataLocation::Storage))
		{
			// We know it does not write to storage.
		}
		else if (!leftHandSide || leftHandSide->isStateVariable())
			node.writesStorage = true;
		return node;
	}
	virtual void endVisit(TupleExpression const& _tuple) override
	{
		auto& node = m_nodes[&_tuple];
		for (auto const& component: _tuple.components())
			if (component)
			{
				node += m_nodes.at(component.get());
				// TODO could be more intelligent here.
				if (component->annotation().type->dataStoredIn(DataLocation::Storage))
					node.readsStorage = true;
			}
	}
	virtual void endVisit(UnaryOperation const& _unaryOperation) override
	{
		m_nodes[&_unaryOperation] = m_nodes.at(&_unaryOperation.subExpression());
		if (_unaryOperation.annotation().type->category() == Type::Category::RationalNumber)
			return;

		switch (_unaryOperation.getOperator())
		{
		case Token::Not:
		case Token::BitNot:
		case Token::Add:
		case Token::Sub:
			return;
		case Token::Delete:
		case Token::Inc:
		case Token::Dec:
		{
			m_nodes[&_unaryOperation].hasSideEffects = true;
			// TODO this could be made more intelligent, especially in connecting to
			// e.g. deleting members of storage or memory structs.
			VariableDeclaration const* var = referencedVariable(_unaryOperation.subExpression());
			m_nodes[&_unaryOperation].writes.insert(var);
			auto t = _unaryOperation.subExpression().annotation().type;
			if (!t->isValueType() && !t->dataStoredIn(DataLocation::Storage))
			{
				// We know it does not write to storage.
			}
			else if (!var || var->isStateVariable())
				m_nodes[&_unaryOperation].writesStorage = true;
			break;
		}
		default:
			solAssert(false, "Invalid unary operator: " + string(Token::toString(_unaryOperation.getOperator())));
		}
	}
	virtual void endVisit(BinaryOperation const& _binaryOperation) override
	{
		Token::Value const c_op = _binaryOperation.getOperator();
		if (c_op == Token::And || c_op == Token::Or)
		{
			// TODO short-circuiting!
		}

		m_nodes[&_binaryOperation] =
			m_nodes.at(&_binaryOperation.leftExpression()) +
			m_nodes.at(&_binaryOperation.rightExpression());
	}
	virtual void endVisit(FunctionCall const& _functionCall) override
	{
		CallgraphNode& node = m_nodes[&_functionCall];
		node += m_nodes.at(&_functionCall.expression());
		for (auto const& arg: _functionCall.arguments())
			node += m_nodes.at(arg.get());

		for (auto const& arg: _functionCall.arguments())
			if (arg->annotation().type->dataStoredIn(DataLocation::Storage))
				node.readsStorage = true;
		if (auto const* ft = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type.get()))
			if (ft->bound() && _functionCall.expression().annotation().type->dataStoredIn(DataLocation::Storage))
				node.readsStorage = true;

		if (_functionCall.annotation().kind != FunctionCallKind::FunctionCall)
			return;

		// just about anything might happen due to reentrancy, event "sends value".
		// What cannot happen is that local variables are written to.
		bool affectStateArbitrarily = false;
		bool hasSideEffects = true;
		bool calls = false;

		FunctionType const& function = dynamic_cast<FunctionType const&>(*_functionCall.expression().annotation().type);
		switch (function.kind())
		{
		case FunctionType::Kind::Internal:
		{
			// We can only use the type signature here, not the result of the analysis
			// because this would be a recursive problem.
			affectStateArbitrarily = function.stateMutability() > StateMutability::View;
			calls = true;
			break;
		}
		case FunctionType::Kind::External:
		case FunctionType::Kind::CallCode:
		case FunctionType::Kind::DelegateCall:
		case FunctionType::Kind::BareCall:
		case FunctionType::Kind::BareCallCode:
		case FunctionType::Kind::BareDelegateCall:
		case FunctionType::Kind::Creation:
		{
			affectStateArbitrarily = true;
			calls = true;
			break;
		}
		case FunctionType::Kind::Send:
		case FunctionType::Kind::Transfer:
		{
			calls = true;
			node.sendsValue = true;
			break;
		}
		case FunctionType::Kind::Selfdestruct:
		{
			calls = true;
			node.sendsValue = true;
			node.selfdestructs = true;
			break;
		}
		case FunctionType::Kind::Revert:
		case FunctionType::Kind::Assert:
		case FunctionType::Kind::Require:
		{
			// TODO model control flow here.
			hasSideEffects = false;
			break;
		}
		case FunctionType::Kind::Log0:
		case FunctionType::Kind::Log1:
		case FunctionType::Kind::Log2:
		case FunctionType::Kind::Log3:
		case FunctionType::Kind::Log4:
		case FunctionType::Kind::Event:
		{
			node.writesLogs = true;
			break;
		}
		case FunctionType::Kind::BlockHash:
		{
			hasSideEffects = false;
			node.readsEnvironment = true;
			break;
		}
		case FunctionType::Kind::ECRecover:
		case FunctionType::Kind::SHA256:
		case FunctionType::Kind::RIPEMD160:
		{
			calls = true;
			hasSideEffects = false;
			break;
		}
		case FunctionType::Kind::SetGas:
		case FunctionType::Kind::SetValue:
		case FunctionType::Kind::SHA3:
		case FunctionType::Kind::AddMod:
		case FunctionType::Kind::MulMod:
		case FunctionType::Kind::ObjectCreation:
		{
			hasSideEffects = false;
			break;
		}
		case FunctionType::Kind::ByteArrayPush:
		case FunctionType::Kind::ArrayPush:
		{
			// TODO determine the exact effects,
			// this might only write to a single variable.
			affectStateArbitrarily = true;
			break;
		}
		default:
			solAssert(false, "Invalid function type.");
		}

		if (hasSideEffects)
			node.hasSideEffects = true;
		if (calls)
			node.calls = true;
		if (affectStateArbitrarily)
			// TODO does not read or write local variables
			node += CallgraphNode::anything();
	}
	virtual void endVisit(NewExpression const& _node) override
	{
		m_nodes[&_node] = m_nodes.at(&_node.typeName());
	}
	virtual void endVisit(MemberAccess const& _memberAccess) override
	{
		m_nodes[&_memberAccess] = m_nodes.at(&_memberAccess.expression());

		if (auto const& varDecl = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
			m_nodes[&_memberAccess] += variableWasReferenced(*varDecl);

		ASTString const& member = _memberAccess.memberName();
		switch (_memberAccess.expression().annotation().type->category())
		{
		case Type::Category::Contract:
		case Type::Category::Integer:
			if (member == "balance" && !_memberAccess.annotation().referencedDeclaration)
				m_nodes[&_memberAccess].readsEnvironment = true;
			break;
		case Type::Category::Magic:
			// we can ignore the kind of magic and only look at the name of the member
			if (member != "data" && member != "sig" && member != "value" && member != "sender")
				m_nodes[&_memberAccess].readsEnvironment = true;
			break;
		case Type::Category::Struct:
		{
			if (
				_memberAccess.expression().annotation().type->dataStoredIn(DataLocation::Storage) &&
				_memberAccess.annotation().type->isValueType()
			)
				m_nodes[&_memberAccess].readsStorage = true;
			break;
		}
		case Type::Category::Array:
		{
			auto const& type = dynamic_cast<ArrayType const&>(*_memberAccess.expression().annotation().type);
			if (member == "length" && type.isDynamicallySized() && type.dataStoredIn(DataLocation::Storage))
				m_nodes[&_memberAccess].readsStorage = true;
			break;
		}
		default:
			break;
		}
	}
	virtual void endVisit(IndexAccess const& _indexAccess) override
	{
		solAssert(_indexAccess.indexExpression(), "");
		m_nodes[&_indexAccess] = m_nodes.at(&_indexAccess.baseExpression()) + m_nodes.at(_indexAccess.indexExpression());

		Type const& baseType = *_indexAccess.baseExpression().annotation().type;

		if (baseType.dataStoredIn(DataLocation::Storage) && _indexAccess.annotation().type->isValueType())
			m_nodes[&_indexAccess].readsStorage = true;
	}
	virtual void endVisit(Identifier const& _identifier) override
	{
		Declaration const* declaration = _identifier.annotation().referencedDeclaration;
		solAssert(declaration, "");

		if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
		{
			m_nodes[&_identifier] = variableWasReferenced(*varDecl);
		}
		if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
		{
			switch (magicVar->type()->category())
			{
			case Type::Category::Contract:
				solAssert(_identifier.name() == "this" || _identifier.name() == "super", "");
				if (!dynamic_cast<ContractType const&>(*magicVar->type()).isSuper())
					// reads the address
					m_nodes[&_identifier].readsEnvironment = true;
				break;
			case Type::Category::Integer:
				solAssert(_identifier.name() == "now", "");
				m_nodes[&_identifier].readsEnvironment = true;
				break;
			default:
				break;
			}
		}
	}
	virtual void endVisit(ElementaryTypeNameExpression const& _node) override { m_nodes[&_node]; }
	virtual void endVisit(Literal const& _node) override { m_nodes[&_node]; }

	map<ASTNode const*, CallgraphNode>& nodes() { return m_nodes; }

private:

	/// @returns the variable referenced in an lvalue expression - if any.
	VariableDeclaration const* referencedVariable(Expression const& _expression)
	{
		Declaration const* declaration = nullptr;
		if (Identifier const* identifier = dynamic_cast<Identifier const*>(&_expression))
			declaration = identifier->annotation().referencedDeclaration;
		else if (MemberAccess const* memberAccess = dynamic_cast<MemberAccess const*>(&_expression))
			declaration = memberAccess->annotation().referencedDeclaration;
		return dynamic_cast<VariableDeclaration const*>(declaration);
	}

	/// Used by identifiers or member access nodes that access a variable.
	CallgraphNode variableWasReferenced(VariableDeclaration const& _varDecl)
	{
		CallgraphNode r;
		r.reads.insert(&_varDecl);
		if (_varDecl.isStateVariable() && !_varDecl.isConstant() && _varDecl.type()->isValueType())
			r.readsStorage = true;
		// Reference types do not really read from storage at this point.
		return r;
	}

	map<ASTNode const*, CallgraphNode>& m_nodes;
};

map<ASTNode const*, CallgraphNode>&& CallgraphBuilder::build()
{
	CallgraphBuilderVisitor v(m_nodes);

	for (auto const& node: m_ast)
		node->accept(v);

	applyModifiersAndBaseConstructors();

	solAssert(!m_nodes.count(nullptr), "Invalid pointer inserted");

	return move(m_nodes);
}

void CallgraphBuilder::applyModifiersAndBaseConstructors()
{
	vector<ContractDefinition const*> contracts;

	for (auto const& node: m_ast)
	{
		SourceUnit const* source = dynamic_cast<SourceUnit const*>(node.get());
		solAssert(source, "");
		for (auto const& topLevelNode: source->nodes())
		{
			ContractDefinition const* contract = dynamic_cast<ContractDefinition const*>(topLevelNode.get());
			if (contract)
				contracts.push_back(contract);
		}
	}

	// Apply modifiers first, because constructors can also have modifiers
	for (auto const* contract: contracts)
	{
		for (FunctionDefinition const* fun: contract->definedFunctions())
			for (auto const& mod: fun->modifiers())
				m_nodes[fun] += m_nodes.at(mod.get());
	}

	for (auto const* contract: contracts)
	{
		FunctionDefinition const* constructor = contract->constructor();
		if (!constructor)
			continue;
		for (auto const& base: contract->annotation().linearizedBaseContracts)
			if (base->constructor())
				m_nodes[constructor] += m_nodes.at(base->constructor());
	}
}
