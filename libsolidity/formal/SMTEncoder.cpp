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

#include <libsolidity/formal/SMTEncoder.h>

#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/formal/SMTPortfolio.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <boost/range/adaptors.hpp>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

SMTEncoder::SMTEncoder(smt::EncodingContext& _context):
	m_errorReporter(m_smtErrors),
	m_context(_context)
{
}

bool SMTEncoder::visit(ContractDefinition const& _contract)
{
	for (auto const& contract: _contract.annotation().linearizedBaseContracts)
		for (auto var: contract->stateVariables())
			if (*contract == _contract || var->isVisibleInDerivedContracts())
				createVariable(*var);
	return true;
}

void SMTEncoder::endVisit(ContractDefinition const&)
{
	m_context.resetAllVariables();
}

void SMTEncoder::endVisit(VariableDeclaration const& _varDecl)
{
	if (_varDecl.isLocalVariable() && _varDecl.type()->isValueType() &&_varDecl.value())
		assignment(_varDecl, *_varDecl.value());
}

bool SMTEncoder::visit(ModifierDefinition const&)
{
	return false;
}

bool SMTEncoder::visit(FunctionDefinition const& _function)
{
	// Not visited by a function call
	if (m_callStack.empty())
		initFunction(_function);

	m_modifierDepthStack.push_back(-1);
	if (_function.isConstructor())
	{
		m_errorReporter.warning(
			_function.location(),
			"Assertion checker does not yet support constructors."
		);
	}
	else
	{
		_function.parameterList().accept(*this);
		if (_function.returnParameterList())
			_function.returnParameterList()->accept(*this);
		visitFunctionOrModifier();
	}
	return false;
}

void SMTEncoder::visitFunctionOrModifier()
{
	solAssert(!m_callStack.empty(), "");
	solAssert(!m_modifierDepthStack.empty(), "");

	++m_modifierDepthStack.back();
	FunctionDefinition const& function = dynamic_cast<FunctionDefinition const&>(*m_callStack.back().first);

	if (m_modifierDepthStack.back() == int(function.modifiers().size()))
	{
		if (function.isImplemented())
			function.body().accept(*this);
	}
	else
	{
		solAssert(m_modifierDepthStack.back() < int(function.modifiers().size()), "");
		ASTPointer<ModifierInvocation> const& modifierInvocation = function.modifiers()[m_modifierDepthStack.back()];
		solAssert(modifierInvocation, "");
		modifierInvocation->accept(*this);
		auto const& modifierDef = dynamic_cast<ModifierDefinition const&>(
			*modifierInvocation->name()->annotation().referencedDeclaration
		);
		vector<smt::Expression> modifierArgsExpr;
		if (modifierInvocation->arguments())
			for (auto arg: *modifierInvocation->arguments())
				modifierArgsExpr.push_back(expr(*arg));
		initializeFunctionCallParameters(modifierDef, modifierArgsExpr);
		pushCallStack({&modifierDef, modifierInvocation.get()});
		modifierDef.body().accept(*this);
		popCallStack();
	}

	--m_modifierDepthStack.back();
}

bool SMTEncoder::visit(PlaceholderStatement const&)
{
	solAssert(!m_callStack.empty(), "");
	auto lastCall = popCallStack();
	visitFunctionOrModifier();
	pushCallStack(lastCall);
	return true;
}

void SMTEncoder::endVisit(FunctionDefinition const&)
{
	popCallStack();
	solAssert(m_modifierDepthStack.back() == -1, "");
	m_modifierDepthStack.pop_back();
	if (m_callStack.empty())
		m_context.popSolver();
}

bool SMTEncoder::visit(InlineAssembly const& _inlineAsm)
{
	m_errorReporter.warning(
		_inlineAsm.location(),
		"Assertion checker does not support inline assembly."
	);
	return false;
}

bool SMTEncoder::visit(IfStatement const& _node)
{
	_node.condition().accept(*this);

	auto indicesEndTrue = visitBranch(&_node.trueStatement(), expr(_node.condition()));
	auto touchedVars = touchedVariables(_node.trueStatement());
	decltype(indicesEndTrue) indicesEndFalse;
	if (_node.falseStatement())
	{
		indicesEndFalse = visitBranch(_node.falseStatement(), !expr(_node.condition()));
		touchedVars += touchedVariables(*_node.falseStatement());
	}
	else
		indicesEndFalse = copyVariableIndices();

	mergeVariables(touchedVars, expr(_node.condition()), indicesEndTrue, indicesEndFalse);

	return false;
}

void SMTEncoder::endVisit(VariableDeclarationStatement const& _varDecl)
{
	if (_varDecl.declarations().size() != 1)
	{
		if (auto init = _varDecl.initialValue())
		{
			auto symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(*init));
			solAssert(symbTuple, "");
			auto const& components = symbTuple->components();
			auto const& declarations = _varDecl.declarations();
			if (!components.empty())
			{
				solAssert(components.size() == declarations.size(), "");
				for (unsigned i = 0; i < declarations.size(); ++i)
					if (
						components.at(i) &&
						declarations.at(i) &&
						m_context.knownVariable(*declarations.at(i))
					)
						assignment(*declarations.at(i), components.at(i)->currentValue());
			}
		}
	}
	else if (m_context.knownVariable(*_varDecl.declarations().front()))
	{
		if (_varDecl.initialValue())
			assignment(*_varDecl.declarations().front(), *_varDecl.initialValue());
	}
	else
		m_errorReporter.warning(
			_varDecl.location(),
			"Assertion checker does not yet implement such variable declarations."
		);

}

void SMTEncoder::endVisit(Assignment const& _assignment)
{
	createExpr(_assignment);

	static set<Token> const compoundOps{
		Token::AssignAdd,
		Token::AssignSub,
		Token::AssignMul,
		Token::AssignDiv,
		Token::AssignMod
	};
	Token op = _assignment.assignmentOperator();
	if (op != Token::Assign && !compoundOps.count(op))
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement this assignment operator."
		);
	else if (!smt::isSupportedType(_assignment.annotation().type->category()))
	{
		// Give it a new index anyway to keep the SSA scheme sound.
		if (auto varDecl = identifierToVariable(_assignment.leftHandSide()))
			m_context.newValue(*varDecl);
	}
	else
	{
		vector<smt::Expression> rightArguments;
		if (_assignment.rightHandSide().annotation().type->category() == Type::Category::Tuple)
		{
			auto symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(_assignment.rightHandSide()));
			solAssert(symbTuple, "");
			for (auto const& component: symbTuple->components())
			{
				/// Right hand side tuple component cannot be empty.
				solAssert(component, "");
				rightArguments.push_back(component->currentValue());
			}
		}
		else
		{
			auto rightHandSide = compoundOps.count(op) ?
				compoundAssignment(_assignment) :
				expr(_assignment.rightHandSide());
			defineExpr(_assignment, rightHandSide);
			rightArguments.push_back(expr(_assignment));
		}
		assignment(
			_assignment.leftHandSide(),
			rightArguments,
			_assignment.annotation().type,
			_assignment.location()
		);
	}
}

void SMTEncoder::endVisit(TupleExpression const& _tuple)
{
	createExpr(_tuple);

	if (_tuple.isInlineArray())
		m_errorReporter.warning(
			_tuple.location(),
			"Assertion checker does not yet implement inline arrays."
		);
	else if (_tuple.annotation().type->category() == Type::Category::Tuple)
	{
		auto const& symbTuple = dynamic_pointer_cast<smt::SymbolicTupleVariable>(m_context.expression(_tuple));
		solAssert(symbTuple, "");
		if (symbTuple->components().empty())
		{
			vector<shared_ptr<smt::SymbolicVariable>> components;
			for (auto const& component: _tuple.components())
				if (component)
				{
					if (auto varDecl = identifierToVariable(*component))
						components.push_back(m_context.variable(*varDecl));
					else
					{
						solAssert(m_context.knownExpression(*component), "");
						components.push_back(m_context.expression(*component));
					}
				}
				else
					components.push_back(nullptr);
			solAssert(components.size() == _tuple.components().size(), "");
			symbTuple->setComponents(move(components));
		}
	}
	else
	{
		/// Parenthesized expressions are also TupleExpression regardless their type.
		auto const& components = _tuple.components();
		solAssert(components.size() == 1, "");
		if (smt::isSupportedType(components.front()->annotation().type->category()))
			defineExpr(_tuple, expr(*components.front()));
	}
}

void SMTEncoder::endVisit(UnaryOperation const& _op)
{
	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;

	createExpr(_op);

	switch (_op.getOperator())
	{
	case Token::Not: // !
	{
		solAssert(smt::isBool(_op.annotation().type->category()), "");
		defineExpr(_op, !expr(_op.subExpression()));
		break;
	}
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
	{

		solAssert(smt::isInteger(_op.annotation().type->category()), "");
		solAssert(_op.subExpression().annotation().lValueRequested, "");
		if (auto identifier = dynamic_cast<Identifier const*>(&_op.subExpression()))
		{
			auto decl = identifierToVariable(*identifier);
			solAssert(decl, "");
			auto innerValue = currentValue(*decl);
			auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
			defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
			assignment(*decl, newValue);
		}
		else if (dynamic_cast<IndexAccess const*>(&_op.subExpression()))
		{
			auto innerValue = expr(_op.subExpression());
			auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
			defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
			arrayIndexAssignment(_op.subExpression(), newValue);
		}
		else
			m_errorReporter.warning(
				_op.location(),
				"Assertion checker does not yet implement such increments / decrements."
			);

		break;
	}
	case Token::Sub: // -
	{
		defineExpr(_op, 0 - expr(_op.subExpression()));
		break;
	}
	case Token::Delete:
	{
		auto const& subExpr = _op.subExpression();
		if (auto decl = identifierToVariable(subExpr))
		{
			m_context.newValue(*decl);
			m_context.setZeroValue(*decl);
		}
		else
		{
			solAssert(m_context.knownExpression(subExpr), "");
			auto const& symbVar = m_context.expression(subExpr);
			symbVar->increaseIndex();
			m_context.setZeroValue(*symbVar);
			if (dynamic_cast<IndexAccess const*>(&_op.subExpression()))
				arrayIndexAssignment(_op.subExpression(), symbVar->currentValue());
			else
				m_errorReporter.warning(
					_op.location(),
					"Assertion checker does not yet implement \"delete\" for this expression."
				);
		}
		break;
	}
	default:
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
	}
}

bool SMTEncoder::visit(UnaryOperation const& _op)
{
	return !shortcutRationalNumber(_op);
}

bool SMTEncoder::visit(BinaryOperation const& _op)
{
	if (shortcutRationalNumber(_op))
		return false;
	if (TokenTraits::isBooleanOp(_op.getOperator()))
	{
		booleanOperation(_op);
		return false;
	}
	return true;
}

void SMTEncoder::endVisit(BinaryOperation const& _op)
{
	if (_op.annotation().type->category() == Type::Category::RationalNumber)
		return;
	if (TokenTraits::isBooleanOp(_op.getOperator()))
		return;

	createExpr(_op);

	if (TokenTraits::isArithmeticOp(_op.getOperator()))
		arithmeticOperation(_op);
	else if (TokenTraits::isCompareOp(_op.getOperator()))
		compareOperation(_op);
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
}

void SMTEncoder::endVisit(FunctionCall const& _funCall)
{
	solAssert(_funCall.annotation().kind != FunctionCallKind::Unset, "");
	createExpr(_funCall);
	if (_funCall.annotation().kind == FunctionCallKind::StructConstructorCall)
	{
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this expression."
		);
		return;
	}

	if (_funCall.annotation().kind == FunctionCallKind::TypeConversion)
	{
		visitTypeConversion(_funCall);
		return;
	}

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);

	std::vector<ASTPointer<Expression const>> const args = _funCall.arguments();
	switch (funType.kind())
	{
	case FunctionType::Kind::Assert:
		visitAssert(_funCall);
		break;
	case FunctionType::Kind::Require:
		visitRequire(_funCall);
		break;
	case FunctionType::Kind::GasLeft:
		visitGasLeft(_funCall);
		break;
	case FunctionType::Kind::Internal:
	case FunctionType::Kind::External:
	case FunctionType::Kind::DelegateCall:
	case FunctionType::Kind::BareCall:
	case FunctionType::Kind::BareCallCode:
	case FunctionType::Kind::BareDelegateCall:
	case FunctionType::Kind::BareStaticCall:
	case FunctionType::Kind::Creation:
		break;
	case FunctionType::Kind::KECCAK256:
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::SHA256:
	case FunctionType::Kind::RIPEMD160:
	case FunctionType::Kind::BlockHash:
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
		break;
	case FunctionType::Kind::Send:
	case FunctionType::Kind::Transfer:
	{
		auto const& memberAccess = dynamic_cast<MemberAccess const&>(_funCall.expression());
		auto const& address = memberAccess.expression();
		auto const& value = args.front();
		solAssert(value, "");

		smt::Expression thisBalance = m_context.balance();
		setSymbolicUnknownValue(thisBalance, TypeProvider::uint256(), m_context);

		m_context.transfer(m_context.thisAddress(), expr(address), expr(*value));
		break;
	}
	default:
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
	}
}

void SMTEncoder::initFunction(FunctionDefinition const& _function)
{
	solAssert(m_callStack.empty(), "");
	m_context.reset();
	m_context.pushSolver();
	m_pathConditions.clear();
	pushCallStack({&_function, nullptr});
	m_uninterpretedTerms.clear();
	resetStateVariables();
	initializeLocalVariables(_function);
	m_arrayAssignmentHappened = false;
}

void SMTEncoder::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
	addPathImpliedExpression(expr(*args.front()));
}

void SMTEncoder::visitRequire(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() >= 1, "");
	solAssert(args.front()->annotation().type->category() == Type::Category::Bool, "");
	addPathImpliedExpression(expr(*args.front()));
}

void SMTEncoder::visitGasLeft(FunctionCall const& _funCall)
{
	string gasLeft = "gasleft()";
	// We increase the variable index since gasleft changes
	// inside a tx.
	defineGlobalVariable(gasLeft, _funCall, true);
	auto const& symbolicVar = m_context.globalSymbol(gasLeft);
	unsigned index = symbolicVar->index();
	// We set the current value to unknown anyway to add type constraints.
	m_context.setUnknownValue(*symbolicVar);
	if (index > 0)
		m_context.addAssertion(symbolicVar->currentValue() <= symbolicVar->valueAtIndex(index - 1));
}

void SMTEncoder::endVisit(Identifier const& _identifier)
{
	if (_identifier.annotation().lValueRequested)
	{
		// Will be translated as part of the node that requested the lvalue.
	}
	else if (_identifier.annotation().type->category() == Type::Category::Function)
		visitFunctionIdentifier(_identifier);
	else if (auto decl = identifierToVariable(_identifier))
		defineExpr(_identifier, currentValue(*decl));
	else if (_identifier.name() == "now")
		defineGlobalVariable(_identifier.name(), _identifier);
	else if (_identifier.name() == "this")
	{
		defineExpr(_identifier, m_context.thisAddress());
		m_uninterpretedTerms.insert(&_identifier);
	}
	else if (smt::isSupportedType(_identifier.annotation().type->category()))
		// TODO: handle MagicVariableDeclaration here
		m_errorReporter.warning(
			_identifier.location(),
			"Assertion checker does not yet support the type of this variable."
		);
}

void SMTEncoder::visitTypeConversion(FunctionCall const& _funCall)
{
	solAssert(_funCall.annotation().kind == FunctionCallKind::TypeConversion, "");
	solAssert(_funCall.arguments().size() == 1, "");
	auto argument = _funCall.arguments().front();
	unsigned argSize = argument->annotation().type->storageBytes();
	unsigned castSize = _funCall.annotation().type->storageBytes();
	if (argSize == castSize)
		defineExpr(_funCall, expr(*argument));
	else
	{
		m_context.setUnknownValue(*m_context.expression(_funCall));
		auto const& funCallCategory = _funCall.annotation().type->category();
		// TODO: truncating and bytesX needs a different approach because of right padding.
		if (funCallCategory == Type::Category::Integer || funCallCategory == Type::Category::Address)
		{
			if (argSize < castSize)
				defineExpr(_funCall, expr(*argument));
			else
			{
				auto const& intType = dynamic_cast<IntegerType const&>(*m_context.expression(_funCall)->type());
				defineExpr(_funCall, smt::Expression::ite(
					expr(*argument) >= smt::minValue(intType) && expr(*argument) <= smt::maxValue(intType),
					expr(*argument),
					expr(_funCall)
				));
			}
		}

		m_errorReporter.warning(
			_funCall.location(),
			"Type conversion is not yet fully supported and might yield false positives."
		);
	}
}

void SMTEncoder::visitFunctionIdentifier(Identifier const& _identifier)
{
	auto const& fType = dynamic_cast<FunctionType const&>(*_identifier.annotation().type);
	if (fType.returnParameterTypes().size() == 1)
	{
		defineGlobalVariable(fType.richIdentifier(), _identifier);
		m_context.createExpression(_identifier, m_context.globalSymbol(fType.richIdentifier()));
	}
}

void SMTEncoder::endVisit(Literal const& _literal)
{
	solAssert(_literal.annotation().type, "Expected type for AST node");
	Type const& type = *_literal.annotation().type;
	if (smt::isNumber(type.category()))
		defineExpr(_literal, smt::Expression(type.literalValue(&_literal)));
	else if (smt::isBool(type.category()))
		defineExpr(_literal, smt::Expression(_literal.token() == Token::TrueLiteral ? true : false));
	else
	{
		if (type.category() == Type::Category::StringLiteral)
		{
			auto stringType = TypeProvider::stringMemory();
			auto stringLit = dynamic_cast<StringLiteralType const*>(_literal.annotation().type);
			solAssert(stringLit, "");
			auto result = smt::newSymbolicVariable(*stringType, stringLit->richIdentifier(), m_context);
			m_context.createExpression(_literal, result.second);
		}
		m_errorReporter.warning(
			_literal.location(),
			"Assertion checker does not yet support the type of this literal (" +
			_literal.annotation().type->toString() +
			")."
		);
	}
}

void SMTEncoder::endVisit(Return const& _return)
{
	if (_return.expression() && m_context.knownExpression(*_return.expression()))
	{
		auto returnParams = m_callStack.back().first->returnParameters();
		if (returnParams.size() > 1)
		{
			auto tuple = dynamic_cast<TupleExpression const*>(_return.expression());
			solAssert(tuple, "");
			auto const& components = tuple->components();
			solAssert(components.size() == returnParams.size(), "");
			for (unsigned i = 0; i < returnParams.size(); ++i)
				if (components.at(i))
					m_context.addAssertion(expr(*components.at(i)) == m_context.newValue(*returnParams.at(i)));
		}
		else if (returnParams.size() == 1)
			m_context.addAssertion(expr(*_return.expression()) == m_context.newValue(*returnParams.front()));
	}
}

bool SMTEncoder::visit(MemberAccess const& _memberAccess)
{
	auto const& accessType = _memberAccess.annotation().type;
	if (accessType->category() == Type::Category::Function)
		return true;

	createExpr(_memberAccess);

	auto const& exprType = _memberAccess.expression().annotation().type;
	solAssert(exprType, "");
	auto identifier = dynamic_cast<Identifier const*>(&_memberAccess.expression());
	if (exprType->category() == Type::Category::Magic)
	{
		string accessedName;
		if (identifier)
			accessedName = identifier->name();
		else
			m_errorReporter.warning(
				_memberAccess.location(),
				"Assertion checker does not yet support this expression."
			);
		defineGlobalVariable(accessedName + "." + _memberAccess.memberName(), _memberAccess);
		return false;
	}
	else if (exprType->category() == Type::Category::TypeType)
	{
		if (identifier && dynamic_cast<EnumDefinition const*>(identifier->annotation().referencedDeclaration))
		{
			auto enumType = dynamic_cast<EnumType const*>(accessType);
			solAssert(enumType, "");
			defineExpr(_memberAccess, enumType->memberValue(_memberAccess.memberName()));
		}
		return false;
	}
	else if (exprType->category() == Type::Category::Address)
	{
		_memberAccess.expression().accept(*this);
		if (_memberAccess.memberName() == "balance")
		{
			defineExpr(_memberAccess, m_context.balance(expr(_memberAccess.expression())));
			setSymbolicUnknownValue(*m_context.expression(_memberAccess), m_context);
			m_uninterpretedTerms.insert(&_memberAccess);
			return false;
		}
	}
	else
		m_errorReporter.warning(
			_memberAccess.location(),
			"Assertion checker does not yet support this expression."
		);

	return true;
}

void SMTEncoder::endVisit(IndexAccess const& _indexAccess)
{
	createExpr(_indexAccess);

	shared_ptr<smt::SymbolicVariable> array;
	if (auto const& id = dynamic_cast<Identifier const*>(&_indexAccess.baseExpression()))
	{
		auto varDecl = identifierToVariable(*id);
		solAssert(varDecl, "");
		array = m_context.variable(*varDecl);
	}
	else if (auto const& innerAccess = dynamic_cast<IndexAccess const*>(&_indexAccess.baseExpression()))
	{
		solAssert(m_context.knownExpression(*innerAccess), "");
		array = m_context.expression(*innerAccess);
	}
	else
	{
		m_errorReporter.warning(
			_indexAccess.location(),
			"Assertion checker does not yet implement this expression."
		);
		return;
	}

	solAssert(array, "");
	defineExpr(_indexAccess, smt::Expression::select(
		array->currentValue(),
		expr(*_indexAccess.indexExpression())
	));
	setSymbolicUnknownValue(
		expr(_indexAccess),
		_indexAccess.annotation().type,
		m_context
	);
	m_uninterpretedTerms.insert(&_indexAccess);
}

void SMTEncoder::arrayAssignment()
{
	m_arrayAssignmentHappened = true;
}

void SMTEncoder::arrayIndexAssignment(Expression const& _expr, smt::Expression const& _rightHandSide)
{
	auto const& indexAccess = dynamic_cast<IndexAccess const&>(_expr);
	if (auto const& id = dynamic_cast<Identifier const*>(&indexAccess.baseExpression()))
	{
		auto varDecl = identifierToVariable(*id);
		solAssert(varDecl, "");

		if (varDecl->hasReferenceOrMappingType())
			m_context.resetVariables([&](VariableDeclaration const& _var) {
				if (_var == *varDecl)
					return false;
				TypePointer prefix = _var.type();
				TypePointer originalType = typeWithoutPointer(varDecl->type());
				while (
					prefix->category() == Type::Category::Mapping ||
					prefix->category() == Type::Category::Array
				)
				{
					if (*originalType == *typeWithoutPointer(prefix))
						return true;
					if (prefix->category() == Type::Category::Mapping)
					{
						auto mapPrefix = dynamic_cast<MappingType const*>(prefix);
						solAssert(mapPrefix, "");
						prefix = mapPrefix->valueType();
					}
					else
					{
						auto arrayPrefix = dynamic_cast<ArrayType const*>(prefix);
						solAssert(arrayPrefix, "");
						prefix = arrayPrefix->baseType();
					}
				}
				return false;
			});

		smt::Expression store = smt::Expression::store(
			m_context.variable(*varDecl)->currentValue(),
			expr(*indexAccess.indexExpression()),
			_rightHandSide
		);
		m_context.addAssertion(m_context.newValue(*varDecl) == store);
		// Update the SMT select value after the assignment,
		// necessary for sound models.
		defineExpr(indexAccess, smt::Expression::select(
			m_context.variable(*varDecl)->currentValue(),
			expr(*indexAccess.indexExpression())
		));
	}
	else if (dynamic_cast<IndexAccess const*>(&indexAccess.baseExpression()))
	{
		auto identifier = dynamic_cast<Identifier const*>(leftmostBase(indexAccess));
		if (identifier)
		{
			auto varDecl = identifierToVariable(*identifier);
			m_context.newValue(*varDecl);
		}

		m_errorReporter.warning(
			indexAccess.location(),
			"Assertion checker does not yet implement assignments to multi-dimensional mappings or arrays."
		);
	}
	else
		m_errorReporter.warning(
			_expr.location(),
			"Assertion checker does not yet implement this expression."
		);
}

void SMTEncoder::defineGlobalVariable(string const& _name, Expression const& _expr, bool _increaseIndex)
{
	if (!m_context.knownGlobalSymbol(_name))
	{
		bool abstract = m_context.createGlobalSymbol(_name, _expr);
		if (abstract)
			m_errorReporter.warning(
				_expr.location(),
				"Assertion checker does not yet support this global variable."
			);
	}
	else if (_increaseIndex)
		m_context.globalSymbol(_name)->increaseIndex();
	// The default behavior is not to increase the index since
	// most of the global values stay the same throughout a tx.
	if (smt::isSupportedType(_expr.annotation().type->category()))
		defineExpr(_expr, m_context.globalSymbol(_name)->currentValue());
}

bool SMTEncoder::shortcutRationalNumber(Expression const& _expr)
{
	if (_expr.annotation().type->category() == Type::Category::RationalNumber)
	{
		auto rationalType = dynamic_cast<RationalNumberType const*>(_expr.annotation().type);
		solAssert(rationalType, "");
		if (rationalType->isNegative())
			defineExpr(_expr, smt::Expression(u2s(rationalType->literalValue(nullptr))));
		else
			defineExpr(_expr, smt::Expression(rationalType->literalValue(nullptr)));
		return true;
	}
	return false;
}

void SMTEncoder::arithmeticOperation(BinaryOperation const& _op)
{
	auto type = _op.annotation().commonType;
	solAssert(type, "");
	if (type->category() == Type::Category::Integer)
	{
		switch (_op.getOperator())
		{
		case Token::Add:
		case Token::Sub:
		case Token::Mul:
		case Token::Div:
		case Token::Mod:
		{
			auto values = arithmeticOperation(
				_op.getOperator(),
				expr(_op.leftExpression()),
				expr(_op.rightExpression()),
				_op.annotation().commonType,
				_op
			);
			defineExpr(_op, values.first);
			break;
		}
		default:
			m_errorReporter.warning(
				_op.location(),
				"Assertion checker does not yet implement this operator."
			);
		}
	}
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator for type " + type->richIdentifier() + "."
		);
}

pair<smt::Expression, smt::Expression> SMTEncoder::arithmeticOperation(
	Token _op,
	smt::Expression const& _left,
	smt::Expression const& _right,
	TypePointer const& _commonType,
	Expression const&
)
{
	static set<Token> validOperators{
		Token::Add,
		Token::Sub,
		Token::Mul,
		Token::Div,
		Token::Mod
	};
	solAssert(validOperators.count(_op), "");
	solAssert(_commonType, "");
	solAssert(_commonType->category() == Type::Category::Integer, "");

	auto const& intType = dynamic_cast<IntegerType const&>(*_commonType);
	smt::Expression valueNoMod(
		_op == Token::Add ? _left + _right :
		_op == Token::Sub ? _left - _right :
		_op == Token::Div ? division(_left, _right, intType) :
		_op == Token::Mul ? _left * _right :
		/*_op == Token::Mod*/ _left % _right
	);

	if (_op == Token::Div || _op == Token::Mod)
		m_context.addAssertion(_right != 0);

	smt::Expression intValueRange = (0 - smt::minValue(intType)) + smt::maxValue(intType) + 1;
	auto value = smt::Expression::ite(
		valueNoMod > smt::maxValue(intType) || valueNoMod < smt::minValue(intType),
		valueNoMod % intValueRange,
		valueNoMod
	);
	if (intType.isSigned())
	{
		value = smt::Expression::ite(
			value > smt::maxValue(intType),
			value - intValueRange,
			value
		);
	}

	return {value, valueNoMod};
}

void SMTEncoder::compareOperation(BinaryOperation const& _op)
{
	solAssert(_op.annotation().commonType, "");
	if (smt::isSupportedType(_op.annotation().commonType->category()))
	{
		smt::Expression left(expr(_op.leftExpression()));
		smt::Expression right(expr(_op.rightExpression()));
		Token op = _op.getOperator();
		shared_ptr<smt::Expression> value;
		if (smt::isNumber(_op.annotation().commonType->category()))
		{
			value = make_shared<smt::Expression>(
				op == Token::Equal ? (left == right) :
				op == Token::NotEqual ? (left != right) :
				op == Token::LessThan ? (left < right) :
				op == Token::LessThanOrEqual ? (left <= right) :
				op == Token::GreaterThan ? (left > right) :
				/*op == Token::GreaterThanOrEqual*/ (left >= right)
			);
		}
		else // Bool
		{
			solUnimplementedAssert(smt::isBool(_op.annotation().commonType->category()), "Operation not yet supported");
			value = make_shared<smt::Expression>(
				op == Token::Equal ? (left == right) :
				/*op == Token::NotEqual*/ (left != right)
			);
		}
		// TODO: check that other values for op are not possible.
		defineExpr(_op, *value);
	}
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for comparisons"
		);
}

void SMTEncoder::booleanOperation(BinaryOperation const& _op)
{
	solAssert(_op.getOperator() == Token::And || _op.getOperator() == Token::Or, "");
	solAssert(_op.annotation().commonType, "");
	if (_op.annotation().commonType->category() == Type::Category::Bool)
	{
		// @TODO check that both of them are not constant
		_op.leftExpression().accept(*this);
		if (_op.getOperator() == Token::And)
		{
			auto indicesAfterSecond = visitBranch(&_op.rightExpression(), expr(_op.leftExpression()));
			mergeVariables(touchedVariables(_op.rightExpression()), !expr(_op.leftExpression()), copyVariableIndices(), indicesAfterSecond);
			defineExpr(_op, expr(_op.leftExpression()) && expr(_op.rightExpression()));
		}
		else
		{
			auto indicesAfterSecond = visitBranch(&_op.rightExpression(), !expr(_op.leftExpression()));
			mergeVariables(touchedVariables(_op.rightExpression()), expr(_op.leftExpression()), copyVariableIndices(), indicesAfterSecond);
			defineExpr(_op, expr(_op.leftExpression()) || expr(_op.rightExpression()));
		}
	}
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for boolean operations"
		);
}

smt::Expression SMTEncoder::division(smt::Expression _left, smt::Expression _right, IntegerType const& _type)
{
	// Signed division in SMTLIB2 rounds differently for negative division.
	if (_type.isSigned())
		return (smt::Expression::ite(
			_left >= 0,
			smt::Expression::ite(_right >= 0, _left / _right, 0 - (_left / (0 - _right))),
			smt::Expression::ite(_right >= 0, 0 - ((0 - _left) / _right), (0 - _left) / (0 - _right))
		));
	else
		return _left / _right;
}

void SMTEncoder::assignment(
	Expression const& _left,
	vector<smt::Expression> const& _right,
	TypePointer const& _type,
	langutil::SourceLocation const& _location
)
{
	if (!smt::isSupportedType(_type->category()))
		m_errorReporter.warning(
			_location,
			"Assertion checker does not yet implement type " + _type->toString()
		);
	else if (auto varDecl = identifierToVariable(_left))
	{
		solAssert(_right.size() == 1, "");
		assignment(*varDecl, _right.front());
	}
	else if (dynamic_cast<IndexAccess const*>(&_left))
	{
		solAssert(_right.size() == 1, "");
		arrayIndexAssignment(_left, _right.front());
	}
	else if (auto tuple = dynamic_cast<TupleExpression const*>(&_left))
	{
		auto const& components = tuple->components();
		if (!_right.empty())
		{
			solAssert(_right.size() == components.size(), "");
			for (unsigned i = 0; i < _right.size(); ++i)
				if (auto component = components.at(i))
					assignment(*component, {_right.at(i)}, component->annotation().type, component->location());
		}
	}
	else
		m_errorReporter.warning(
			_location,
			"Assertion checker does not yet implement such assignments."
		);
}

smt::Expression SMTEncoder::compoundAssignment(Assignment const& _assignment)
{
	static map<Token, Token> const compoundToArithmetic{
		{Token::AssignAdd, Token::Add},
		{Token::AssignSub, Token::Sub},
		{Token::AssignMul, Token::Mul},
		{Token::AssignDiv, Token::Div},
		{Token::AssignMod, Token::Mod}
	};
	Token op = _assignment.assignmentOperator();
	solAssert(compoundToArithmetic.count(op), "");
	auto decl = identifierToVariable(_assignment.leftHandSide());
	auto values = arithmeticOperation(
		compoundToArithmetic.at(op),
		decl ? currentValue(*decl) : expr(_assignment.leftHandSide()),
		expr(_assignment.rightHandSide()),
		_assignment.annotation().type,
		_assignment
	);
	return values.first;
}

void SMTEncoder::assignment(VariableDeclaration const& _variable, Expression const& _value)
{
	assignment(_variable, expr(_value));
}

void SMTEncoder::assignment(VariableDeclaration const& _variable, smt::Expression const& _value)
{
	TypePointer type = _variable.type();
	if (type->category() == Type::Category::Mapping)
		arrayAssignment();
	m_context.addAssertion(m_context.newValue(_variable) == _value);
}

SMTEncoder::VariableIndices SMTEncoder::visitBranch(ASTNode const* _statement, smt::Expression _condition)
{
	return visitBranch(_statement, &_condition);
}

SMTEncoder::VariableIndices SMTEncoder::visitBranch(ASTNode const* _statement, smt::Expression const* _condition)
{
	auto indicesBeforeBranch = copyVariableIndices();
	if (_condition)
		pushPathCondition(*_condition);
	_statement->accept(*this);
	if (_condition)
		popPathCondition();
	auto indicesAfterBranch = copyVariableIndices();
	resetVariableIndices(indicesBeforeBranch);
	return indicesAfterBranch;
}

void SMTEncoder::initializeFunctionCallParameters(CallableDeclaration const& _function, vector<smt::Expression> const& _callArgs)
{
	auto const& funParams = _function.parameters();
	solAssert(funParams.size() == _callArgs.size(), "");
	for (unsigned i = 0; i < funParams.size(); ++i)
		if (createVariable(*funParams[i]))
		{
			m_context.addAssertion(_callArgs[i] == m_context.newValue(*funParams[i]));
			if (funParams[i]->annotation().type->category() == Type::Category::Mapping)
				m_arrayAssignmentHappened = true;
		}

	for (auto const& variable: _function.localVariables())
		if (createVariable(*variable))
		{
			m_context.newValue(*variable);
			m_context.setZeroValue(*variable);
		}

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
			{
				m_context.newValue(*retParam);
				m_context.setZeroValue(*retParam);
			}
}

void SMTEncoder::initializeLocalVariables(FunctionDefinition const& _function)
{
	for (auto const& variable: _function.localVariables())
		if (createVariable(*variable))
			m_context.setZeroValue(*variable);

	for (auto const& param: _function.parameters())
		if (createVariable(*param))
			m_context.setUnknownValue(*param);

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
				m_context.setZeroValue(*retParam);
}

void SMTEncoder::resetStateVariables()
{
	m_context.resetVariables([&](VariableDeclaration const& _variable) { return _variable.isStateVariable(); });
}

TypePointer SMTEncoder::typeWithoutPointer(TypePointer const& _type)
{
	if (auto refType = dynamic_cast<ReferenceType const*>(_type))
		return TypeProvider::withLocationIfReference(refType->location(), _type);
	return _type;
}

void SMTEncoder::mergeVariables(set<VariableDeclaration const*> const& _variables, smt::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse)
{
	auto cmp = [] (VariableDeclaration const* var1, VariableDeclaration const* var2) {
		return var1->id() < var2->id();
	};
	set<VariableDeclaration const*, decltype(cmp)> sortedVars(begin(_variables), end(_variables), cmp);

	/// Knowledge about references is erased if a reference is assigned,
	/// so those also need their SSA's merged.
	/// This does not cause scope harm since the symbolic variables
	/// are kept alive.
	for (auto const& var: _indicesEndTrue)
	{
		solAssert(_indicesEndFalse.count(var.first), "");
		if (
			_indicesEndFalse.at(var.first) != var.second &&
			!sortedVars.count(var.first)
		)
			sortedVars.insert(var.first);
	}

	for (auto const* decl: sortedVars)
	{
		solAssert(_indicesEndTrue.count(decl) && _indicesEndFalse.count(decl), "");
		int trueIndex = _indicesEndTrue.at(decl);
		int falseIndex = _indicesEndFalse.at(decl);
		solAssert(trueIndex != falseIndex, "");
		m_context.addAssertion(m_context.newValue(*decl) == smt::Expression::ite(
			_condition,
			valueAtIndex(*decl, trueIndex),
			valueAtIndex(*decl, falseIndex))
		);
	}
}

smt::Expression SMTEncoder::currentValue(VariableDeclaration const& _decl)
{
	solAssert(m_context.knownVariable(_decl), "");
	return m_context.variable(_decl)->currentValue();
}

smt::Expression SMTEncoder::valueAtIndex(VariableDeclaration const& _decl, int _index)
{
	solAssert(m_context.knownVariable(_decl), "");
	return m_context.variable(_decl)->valueAtIndex(_index);
}

bool SMTEncoder::createVariable(VariableDeclaration const& _varDecl)
{
	if (m_context.knownVariable(_varDecl))
		return true;
	bool abstract = m_context.createVariable(_varDecl);
	if (abstract)
	{
		m_errorReporter.warning(
			_varDecl.location(),
			"Assertion checker does not yet support the type of this variable."
		);
		return false;
	}
	return true;
}

smt::Expression SMTEncoder::expr(Expression const& _e)
{
	if (!m_context.knownExpression(_e))
	{
		m_errorReporter.warning(_e.location(), "Internal error: Expression undefined for SMT solver." );
		createExpr(_e);
	}
	return m_context.expression(_e)->currentValue();
}

void SMTEncoder::createExpr(Expression const& _e)
{
	bool abstract = m_context.createExpression(_e);
	if (abstract)
		m_errorReporter.warning(
			_e.location(),
			"Assertion checker does not yet implement type " + _e.annotation().type->toString()
		);
}

void SMTEncoder::defineExpr(Expression const& _e, smt::Expression _value)
{
	createExpr(_e);
	solAssert(smt::smtKind(_e.annotation().type->category()) != smt::Kind::Function, "Equality operator applied to type that is not fully supported");
	m_context.addAssertion(expr(_e) == _value);
}

void SMTEncoder::popPathCondition()
{
	solAssert(m_pathConditions.size() > 0, "Cannot pop path condition, empty.");
	m_pathConditions.pop_back();
}

void SMTEncoder::pushPathCondition(smt::Expression const& _e)
{
	m_pathConditions.push_back(currentPathConditions() && _e);
}

smt::Expression SMTEncoder::currentPathConditions()
{
	if (m_pathConditions.empty())
		return smt::Expression(true);
	return m_pathConditions.back();
}

SecondarySourceLocation SMTEncoder::callStackMessage(vector<CallStackEntry> const& _callStack)
{
	SecondarySourceLocation callStackLocation;
	solAssert(!_callStack.empty(), "");
	callStackLocation.append("Callstack: ", SourceLocation());
	for (auto const& call: _callStack | boost::adaptors::reversed)
		if (call.second)
			callStackLocation.append("", call.second->location());
	return callStackLocation;
}

pair<CallableDeclaration const*, ASTNode const*> SMTEncoder::popCallStack()
{
	solAssert(!m_callStack.empty(), "");
	auto lastCalled = m_callStack.back();
	m_callStack.pop_back();
	return lastCalled;
}

void SMTEncoder::pushCallStack(CallStackEntry _entry)
{
	m_callStack.push_back(_entry);
}

void SMTEncoder::addPathImpliedExpression(smt::Expression const& _e)
{
	m_context.addAssertion(smt::Expression::implies(currentPathConditions(), _e));
}

bool SMTEncoder::isRootFunction()
{
	return m_callStack.size() == 1;
}

bool SMTEncoder::visitedFunction(FunctionDefinition const* _funDef)
{
	for (auto const& call: m_callStack)
		if (call.first == _funDef)
			return true;
	return false;
}

SMTEncoder::VariableIndices SMTEncoder::copyVariableIndices()
{
	VariableIndices indices;
	for (auto const& var: m_context.variables())
		indices.emplace(var.first, var.second->index());
	return indices;
}

void SMTEncoder::resetVariableIndices(VariableIndices const& _indices)
{
	for (auto const& var: _indices)
		m_context.variable(*var.first)->index() = var.second;
}

Expression const* SMTEncoder::leftmostBase(IndexAccess const& _indexAccess)
{
	Expression const* base = &_indexAccess.baseExpression();
	while (auto access = dynamic_cast<IndexAccess const*>(base))
		base = &access->baseExpression();
	return base;
}

set<VariableDeclaration const*> SMTEncoder::touchedVariables(ASTNode const& _node)
{
	solAssert(!m_callStack.empty(), "");
	vector<CallableDeclaration const*> callStack;
	for (auto const& call: m_callStack)
		callStack.push_back(call.first);
	return m_variableUsage.touchedVariables(_node, callStack);
}

VariableDeclaration const* SMTEncoder::identifierToVariable(Expression const& _expr)
{
	if (auto identifier = dynamic_cast<Identifier const*>(&_expr))
	{
		if (auto decl = dynamic_cast<VariableDeclaration const*>(identifier->annotation().referencedDeclaration))
		{
			solAssert(m_context.knownVariable(*decl), "");
			return decl;
		}
	}
	return nullptr;
}

string SMTEncoder::extraComment()
{
	string extra;
	if (m_arrayAssignmentHappened)
		extra +=
			"\nNote that array aliasing is not supported,"
			" therefore all mapping information is erased after"
			" a mapping local variable/parameter is assigned.\n"
			"You can re-introduce information using require().";
	return extra;
}

FunctionDefinition const* SMTEncoder::functionCallToDefinition(FunctionCall const& _funCall)
{
	if (_funCall.annotation().kind != FunctionCallKind::FunctionCall)
		return nullptr;

	FunctionDefinition const* funDef = nullptr;
	Expression const* calledExpr = &_funCall.expression();

	if (TupleExpression const* fun = dynamic_cast<TupleExpression const*>(&_funCall.expression()))
	{
		solAssert(fun->components().size() == 1, "");
		calledExpr = fun->components().front().get();
	}

	if (Identifier const* fun = dynamic_cast<Identifier const*>(calledExpr))
		funDef = dynamic_cast<FunctionDefinition const*>(fun->annotation().referencedDeclaration);
	else if (MemberAccess const* fun = dynamic_cast<MemberAccess const*>(calledExpr))
		funDef = dynamic_cast<FunctionDefinition const*>(fun->annotation().referencedDeclaration);

	return funDef;
}
