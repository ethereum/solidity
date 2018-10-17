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

#include <libsolidity/formal/SMTChecker.h>

#include <libsolidity/formal/SMTPortfolio.h>

#include <libsolidity/formal/SSAVariable.h>
#include <libsolidity/formal/SymbolicIntVariable.h>
#include <libsolidity/formal/VariableUsage.h>
#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/interface/ErrorReporter.h>

#include <boost/range/adaptor/map.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SMTChecker::SMTChecker(ErrorReporter& _errorReporter, ReadCallback::Callback const& _readFileCallback):
	m_interface(make_shared<smt::SMTPortfolio>(_readFileCallback)),
	m_errorReporter(_errorReporter)
{
}

void SMTChecker::analyze(SourceUnit const& _source)
{
	m_variableUsage = make_shared<VariableUsage>(_source);
	if (_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker))
		_source.accept(*this);
}

bool SMTChecker::visit(ContractDefinition const& _contract)
{
	for (auto _var : _contract.stateVariables())
		if (_var->type()->isValueType())
			createVariable(*_var);
	return true;
}

void SMTChecker::endVisit(ContractDefinition const&)
{
	m_variables.clear();
}

void SMTChecker::endVisit(VariableDeclaration const& _varDecl)
{
	if (_varDecl.isLocalVariable() && _varDecl.type()->isValueType() &&_varDecl.value())
		assignment(_varDecl, *_varDecl.value(), _varDecl.location());
}

bool SMTChecker::visit(FunctionDefinition const& _function)
{
	if (!_function.modifiers().empty() || _function.isConstructor())
		m_errorReporter.warning(
			_function.location(),
			"Assertion checker does not yet support constructors and functions with modifiers."
		);
	m_functionPath.push_back(&_function);
	// Not visited by a function call
	if (isRootFunction())
	{
		m_interface->reset();
		m_pathConditions.clear();
		m_expressions.clear();
		resetStateVariables();
		initializeLocalVariables(_function);
	}

	m_loopExecutionHappened = false;
	return true;
}

void SMTChecker::endVisit(FunctionDefinition const&)
{
	// If _function was visited from a function call we don't remove
	// the local variables just yet, since we might need them for
	// future calls.
	// Otherwise we remove any local variables from the context and
	// keep the state variables.
	if (isRootFunction())
		removeLocalVariables();
	m_functionPath.pop_back();
}

bool SMTChecker::visit(IfStatement const& _node)
{
	_node.condition().accept(*this);

	// We ignore called functions here because they have
	// specific input values.
	if (isRootFunction())
		checkBooleanNotConstant(_node.condition(), "Condition is always $VALUE.");

	auto indicesEndTrue = visitBranch(_node.trueStatement(), expr(_node.condition()));
	vector<VariableDeclaration const*> touchedVariables = m_variableUsage->touchedVariables(_node.trueStatement());
	decltype(indicesEndTrue) indicesEndFalse;
	if (_node.falseStatement())
	{
		indicesEndFalse = visitBranch(*_node.falseStatement(), !expr(_node.condition()));
		touchedVariables += m_variableUsage->touchedVariables(*_node.falseStatement());
	}
	else
		indicesEndFalse = copyVariableIndices();

	mergeVariables(touchedVariables, expr(_node.condition()), indicesEndTrue, indicesEndFalse);

	return false;
}

bool SMTChecker::visit(WhileStatement const& _node)
{
	auto touchedVariables = m_variableUsage->touchedVariables(_node);
	resetVariables(touchedVariables);
	if (_node.isDoWhile())
	{
		visitBranch(_node.body());
		// TODO the assertions generated in the body should still be active in the condition
		_node.condition().accept(*this);
		if (isRootFunction())
			checkBooleanNotConstant(_node.condition(), "Do-while loop condition is always $VALUE.");
	}
	else
	{
		_node.condition().accept(*this);
		if (isRootFunction())
			checkBooleanNotConstant(_node.condition(), "While loop condition is always $VALUE.");

		visitBranch(_node.body(), expr(_node.condition()));
	}
	m_loopExecutionHappened = true;
	resetVariables(touchedVariables);

	return false;
}

bool SMTChecker::visit(ForStatement const& _node)
{
	if (_node.initializationExpression())
		_node.initializationExpression()->accept(*this);

	// Do not reset the init expression part.
	auto touchedVariables =
		m_variableUsage->touchedVariables(_node.body());
	if (_node.condition())
		touchedVariables += m_variableUsage->touchedVariables(*_node.condition());
	if (_node.loopExpression())
		touchedVariables += m_variableUsage->touchedVariables(*_node.loopExpression());
	// Remove duplicates
	std::sort(touchedVariables.begin(), touchedVariables.end());
	touchedVariables.erase(std::unique(touchedVariables.begin(), touchedVariables.end()), touchedVariables.end());

	resetVariables(touchedVariables);

	if (_node.condition())
	{
		_node.condition()->accept(*this);
		if (isRootFunction())
			checkBooleanNotConstant(*_node.condition(), "For loop condition is always $VALUE.");
	}

	m_interface->push();
	if (_node.condition())
		m_interface->addAssertion(expr(*_node.condition()));
	_node.body().accept(*this);
	if (_node.loopExpression())
		_node.loopExpression()->accept(*this);

	m_interface->pop();

	m_loopExecutionHappened = true;

	resetVariables(touchedVariables);

	return false;
}

void SMTChecker::endVisit(VariableDeclarationStatement const& _varDecl)
{
	if (_varDecl.declarations().size() != 1)
		m_errorReporter.warning(
			_varDecl.location(),
			"Assertion checker does not yet support such variable declarations."
		);
	else if (knownVariable(*_varDecl.declarations()[0]))
	{
		if (_varDecl.initialValue())
			assignment(*_varDecl.declarations()[0], *_varDecl.initialValue(), _varDecl.location());
	}
	else
		m_errorReporter.warning(
			_varDecl.location(),
			"Assertion checker does not yet implement such variable declarations."
		);
}

void SMTChecker::endVisit(Assignment const& _assignment)
{
	if (_assignment.assignmentOperator() != Token::Value::Assign)
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement compound assignment."
		);
	else if (!isSupportedType(_assignment.annotation().type->category()))
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement type " + _assignment.annotation().type->toString()
		);
	else if (Identifier const* identifier = dynamic_cast<Identifier const*>(&_assignment.leftHandSide()))
	{
		VariableDeclaration const& decl = dynamic_cast<VariableDeclaration const&>(*identifier->annotation().referencedDeclaration);
		if (knownVariable(decl))
		{
			assignment(decl, _assignment.rightHandSide(), _assignment.location());
			defineExpr(_assignment, expr(_assignment.rightHandSide()));
		}
		else
			m_errorReporter.warning(
				_assignment.location(),
				"Assertion checker does not yet implement such assignments."
			);
	}
	else
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement such assignments."
		);
}

void SMTChecker::endVisit(TupleExpression const& _tuple)
{
	if (_tuple.isInlineArray() || _tuple.components().size() != 1)
		m_errorReporter.warning(
			_tuple.location(),
			"Assertion checker does not yet implement tuples and inline arrays."
		);
	else
		defineExpr(_tuple, expr(*_tuple.components()[0]));
}

void SMTChecker::checkUnderOverflow(smt::Expression _value, IntegerType const& _type, SourceLocation const& _location)
{
	checkCondition(
		_value < minValue(_type),
		_location,
		"Underflow (resulting value less than " + formatNumber(_type.minValue()) + ")",
		"<result>",
		&_value
	);
	checkCondition(
		_value > maxValue(_type),
		_location,
		"Overflow (resulting value larger than " + formatNumber(_type.maxValue()) + ")",
		"<result>",
		&_value
	);
}

void SMTChecker::endVisit(UnaryOperation const& _op)
{
	switch (_op.getOperator())
	{
	case Token::Not: // !
	{
		solAssert(isBool(_op.annotation().type->category()), "");
		defineExpr(_op, !expr(_op.subExpression()));
		break;
	}
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
	{

		solAssert(isInteger(_op.annotation().type->category()), "");
		solAssert(_op.subExpression().annotation().lValueRequested, "");
		if (Identifier const* identifier = dynamic_cast<Identifier const*>(&_op.subExpression()))
		{
			VariableDeclaration const& decl = dynamic_cast<VariableDeclaration const&>(*identifier->annotation().referencedDeclaration);
			if (knownVariable(decl))
			{
				auto innerValue = currentValue(decl);
				auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
				assignment(decl, newValue, _op.location());
				defineExpr(_op, _op.isPrefixOperation() ? newValue : innerValue);
			}
			else
				m_errorReporter.warning(
					_op.location(),
					"Assertion checker does not yet implement such assignments."
				);
		}
		else
			m_errorReporter.warning(
				_op.location(),
				"Assertion checker does not yet implement such increments / decrements."
			);
		break;
	}
	case Token::Add: // +
		defineExpr(_op, expr(_op.subExpression()));
		break;
	case Token::Sub: // -
	{
		defineExpr(_op, 0 - expr(_op.subExpression()));
		if (auto intType = dynamic_cast<IntegerType const*>(_op.annotation().type.get()))
			checkUnderOverflow(expr(_op), *intType, _op.location());
		break;
	}
	default:
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
	}
}

void SMTChecker::endVisit(BinaryOperation const& _op)
{
	if (Token::isArithmeticOp(_op.getOperator()))
		arithmeticOperation(_op);
	else if (Token::isCompareOp(_op.getOperator()))
		compareOperation(_op);
	else if (Token::isBooleanOp(_op.getOperator()))
		booleanOperation(_op);
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
}

void SMTChecker::endVisit(FunctionCall const& _funCall)
{
	solAssert(_funCall.annotation().kind != FunctionCallKind::Unset, "");
	if (_funCall.annotation().kind != FunctionCallKind::FunctionCall)
	{
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this expression."
		);
		return;
	}

	FunctionType const& funType = dynamic_cast<FunctionType const&>(*_funCall.expression().annotation().type);

	std::vector<ASTPointer<Expression const>> const args = _funCall.arguments();
	if (funType.kind() == FunctionType::Kind::Assert)
		visitAssert(_funCall);
	else if (funType.kind() == FunctionType::Kind::Require)
		visitRequire(_funCall);
	else if (funType.kind() == FunctionType::Kind::Internal)
		inlineFunctionCall(_funCall);
	else
	{
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
	}
}

void SMTChecker::visitAssert(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args[0]->annotation().type->category() == Type::Category::Bool, "");
	checkCondition(!(expr(*args[0])), _funCall.location(), "Assertion violation");
	addPathImpliedExpression(expr(*args[0]));
}

void SMTChecker::visitRequire(FunctionCall const& _funCall)
{
	auto const& args = _funCall.arguments();
	solAssert(args.size() == 1, "");
	solAssert(args[0]->annotation().type->category() == Type::Category::Bool, "");
	if (isRootFunction())
		checkBooleanNotConstant(*args[0], "Condition is always $VALUE.");
	addPathImpliedExpression(expr(*args[0]));
}

void SMTChecker::inlineFunctionCall(FunctionCall const& _funCall)
{
	FunctionDefinition const* _funDef = nullptr;
	Expression const* _calledExpr = &_funCall.expression();

	if (TupleExpression const* _fun = dynamic_cast<TupleExpression const*>(&_funCall.expression()))
	{
		solAssert(_fun->components().size() == 1, "");
		_calledExpr = _fun->components().at(0).get();
	}

	if (Identifier const* _fun = dynamic_cast<Identifier const*>(_calledExpr))
		_funDef = dynamic_cast<FunctionDefinition const*>(_fun->annotation().referencedDeclaration);
	else if (MemberAccess const* _fun = dynamic_cast<MemberAccess const*>(_calledExpr))
		_funDef = dynamic_cast<FunctionDefinition const*>(_fun->annotation().referencedDeclaration);
	else
	{
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not yet implement this type of function call."
		);
		return;
	}
	solAssert(_funDef, "");

	if (visitedFunction(_funDef))
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not support recursive function calls.",
			SecondarySourceLocation().append("Starting from function:", _funDef->location())
		);
	else if (_funDef && _funDef->isImplemented())
	{
		vector<smt::Expression> funArgs;
		for (auto arg: _funCall.arguments())
			funArgs.push_back(expr(*arg));
		initializeFunctionCallParameters(*_funDef, funArgs);
		_funDef->accept(*this);
		auto const& returnParams = _funDef->returnParameters();
		if (_funDef->returnParameters().size())
		{
			if (returnParams.size() > 1)
				m_errorReporter.warning(
					_funCall.location(),
					"Assertion checker does not yet support calls to functions that return more than one value."
				);
			else
				defineExpr(_funCall, currentValue(*returnParams[0]));
		}
	}
	else
	{
		m_errorReporter.warning(
			_funCall.location(),
			"Assertion checker does not support calls to functions without implementation."
		);
	}
}

void SMTChecker::endVisit(Identifier const& _identifier)
{
	if (_identifier.annotation().lValueRequested)
	{
		// Will be translated as part of the node that requested the lvalue.
	}
	else if (isSupportedType(_identifier.annotation().type->category()))
	{
		if (VariableDeclaration const* decl = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
			defineExpr(_identifier, currentValue(*decl));
		else
			// TODO: handle MagicVariableDeclaration here
			m_errorReporter.warning(
				_identifier.location(),
				"Assertion checker does not yet support the type of this variable."
			);
	}
	else if (FunctionType const* fun = dynamic_cast<FunctionType const*>(_identifier.annotation().type.get()))
	{
		if (fun->kind() == FunctionType::Kind::Assert || fun->kind() == FunctionType::Kind::Require)
			return;
		createExpr(_identifier);
	}
}

void SMTChecker::endVisit(Literal const& _literal)
{
	Type const& type = *_literal.annotation().type;
	if (type.category() == Type::Category::Integer || type.category() == Type::Category::Address || type.category() == Type::Category::RationalNumber)
	{
		if (RationalNumberType const* rational = dynamic_cast<RationalNumberType const*>(&type))
			solAssert(!rational->isFractional(), "");

		defineExpr(_literal, smt::Expression(type.literalValue(&_literal)));
	}
	else if (type.category() == Type::Category::Bool)
		defineExpr(_literal, smt::Expression(_literal.token() == Token::TrueLiteral ? true : false));
	else
		m_errorReporter.warning(
			_literal.location(),
			"Assertion checker does not yet support the type of this literal (" +
			_literal.annotation().type->toString() +
			")."
		);
}

void SMTChecker::endVisit(Return const& _return)
{
	if (hasExpr(*_return.expression()))
	{
		auto returnParams = m_functionPath.back()->returnParameters();
		if (returnParams.size() > 1)
			m_errorReporter.warning(
				_return.location(),
				"Assertion checker does not yet support more than one return value."
			);
		else if (returnParams.size() == 1)
			m_interface->addAssertion(expr(*_return.expression()) == newValue(*returnParams[0]));
	}
}

void SMTChecker::arithmeticOperation(BinaryOperation const& _op)
{
	switch (_op.getOperator())
	{
	case Token::Add:
	case Token::Sub:
	case Token::Mul:
	case Token::Div:
	{
		solAssert(_op.annotation().commonType, "");
		if (_op.annotation().commonType->category() != Type::Category::Integer)
		{
			m_errorReporter.warning(
				_op.location(),
				"Assertion checker does not yet implement this operator on non-integer types."
			);
			break;
		}
		auto const& intType = dynamic_cast<IntegerType const&>(*_op.annotation().commonType);
		smt::Expression left(expr(_op.leftExpression()));
		smt::Expression right(expr(_op.rightExpression()));
		Token::Value op = _op.getOperator();
		smt::Expression value(
			op == Token::Add ? left + right :
			op == Token::Sub ? left - right :
			op == Token::Div ? division(left, right, intType) :
			/*op == Token::Mul*/ left * right
		);

		if (_op.getOperator() == Token::Div)
		{
			checkCondition(right == 0, _op.location(), "Division by zero", "<result>", &right);
			m_interface->addAssertion(right != 0);
		}

		checkUnderOverflow(value, intType, _op.location());

		defineExpr(_op, value);
		break;
	}
	default:
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement this operator."
		);
	}
}

void SMTChecker::compareOperation(BinaryOperation const& _op)
{
	solAssert(_op.annotation().commonType, "");
	if (isSupportedType(_op.annotation().commonType->category()))
	{
		smt::Expression left(expr(_op.leftExpression()));
		smt::Expression right(expr(_op.rightExpression()));
		Token::Value op = _op.getOperator();
		shared_ptr<smt::Expression> value;
		if (isNumber(_op.annotation().commonType->category()))
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
			solUnimplementedAssert(isBool(_op.annotation().commonType->category()), "Operation not yet supported");
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

void SMTChecker::booleanOperation(BinaryOperation const& _op)
{
	solAssert(_op.getOperator() == Token::And || _op.getOperator() == Token::Or, "");
	solAssert(_op.annotation().commonType, "");
	if (_op.annotation().commonType->category() == Type::Category::Bool)
	{
		// @TODO check that both of them are not constant
		if (_op.getOperator() == Token::And)
			defineExpr(_op, expr(_op.leftExpression()) && expr(_op.rightExpression()));
		else
			defineExpr(_op, expr(_op.leftExpression()) || expr(_op.rightExpression()));
	}
	else
		m_errorReporter.warning(
			_op.location(),
			"Assertion checker does not yet implement the type " + _op.annotation().commonType->toString() + " for boolean operations"
					);
}

smt::Expression SMTChecker::division(smt::Expression _left, smt::Expression _right, IntegerType const& _type)
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

void SMTChecker::assignment(VariableDeclaration const& _variable, Expression const& _value, SourceLocation const& _location)
{
	assignment(_variable, expr(_value), _location);
}

void SMTChecker::assignment(VariableDeclaration const& _variable, smt::Expression const& _value, SourceLocation const& _location)
{
	TypePointer type = _variable.type();
	if (auto const* intType = dynamic_cast<IntegerType const*>(type.get()))
		checkUnderOverflow(_value, *intType, _location);
	else if (dynamic_cast<AddressType const*>(type.get()))
		checkUnderOverflow(_value, IntegerType(160), _location);
	m_interface->addAssertion(newValue(_variable) == _value);
}

SMTChecker::VariableIndices SMTChecker::visitBranch(Statement const& _statement, smt::Expression _condition)
{
	return visitBranch(_statement, &_condition);
}

SMTChecker::VariableIndices SMTChecker::visitBranch(Statement const& _statement, smt::Expression const* _condition)
{
	auto indicesBeforeBranch = copyVariableIndices();
	if (_condition)
		pushPathCondition(*_condition);
	_statement.accept(*this);
	if (_condition)
		popPathCondition();
	auto indicesAfterBranch = copyVariableIndices();
	resetVariableIndices(indicesBeforeBranch);
	return indicesAfterBranch;
}

void SMTChecker::checkCondition(
	smt::Expression _condition,
	SourceLocation const& _location,
	string const& _description,
	string const& _additionalValueName,
	smt::Expression* _additionalValue
)
{
	m_interface->push();
	addPathConjoinedExpression(_condition);

	vector<smt::Expression> expressionsToEvaluate;
	vector<string> expressionNames;
	if (m_functionPath.size())
	{
		if (_additionalValue)
		{
			expressionsToEvaluate.emplace_back(*_additionalValue);
			expressionNames.push_back(_additionalValueName);
		}
		for (auto const& var: m_variables)
			if (knownVariable(*var.first))
			{
				expressionsToEvaluate.emplace_back(currentValue(*var.first));
				expressionNames.push_back(var.first->name());
			}
	}
	smt::CheckResult result;
	vector<string> values;
	tie(result, values) = checkSatisfiableAndGenerateModel(expressionsToEvaluate);

	string loopComment;
	if (m_loopExecutionHappened)
		loopComment =
			"\nNote that some information is erased after the execution of loops.\n"
			"You can re-introduce information using require().";
	switch (result)
	{
	case smt::CheckResult::SATISFIABLE:
	{
		std::ostringstream message;
		message << _description << " happens here";
		if (m_functionPath.size())
		{
			std::ostringstream modelMessage;
			modelMessage << "  for:\n";
			solAssert(values.size() == expressionNames.size(), "");
			map<string, string> sortedModel;
			for (size_t i = 0; i < values.size(); ++i)
				if (expressionsToEvaluate.at(i).name != values.at(i))
					sortedModel[expressionNames.at(i)] = values.at(i);

			for (auto const& eval: sortedModel)
				modelMessage << "  " << eval.first << " = " << eval.second << "\n";
			m_errorReporter.warning(_location, message.str() + loopComment, SecondarySourceLocation().append(modelMessage.str(), SourceLocation()));
		}
		else
		{
			message << ".";
			m_errorReporter.warning(_location, message.str() + loopComment);
		}
		break;
	}
	case smt::CheckResult::UNSATISFIABLE:
		break;
	case smt::CheckResult::UNKNOWN:
		m_errorReporter.warning(_location, _description + " might happen here." + loopComment);
		break;
	case smt::CheckResult::CONFLICTING:
		m_errorReporter.warning(_location, "At least two SMT solvers provided conflicting answers. Results might not be sound.");
		break;
	case smt::CheckResult::ERROR:
		m_errorReporter.warning(_location, "Error trying to invoke SMT solver.");
		break;
	}
	m_interface->pop();
}

void SMTChecker::checkBooleanNotConstant(Expression const& _condition, string const& _description)
{
	// Do not check for const-ness if this is a constant.
	if (dynamic_cast<Literal const*>(&_condition))
		return;

	m_interface->push();
	addPathConjoinedExpression(expr(_condition));
	auto positiveResult = checkSatisfiable();
	m_interface->pop();

	m_interface->push();
	addPathConjoinedExpression(!expr(_condition));
	auto negatedResult = checkSatisfiable();
	m_interface->pop();

	if (positiveResult == smt::CheckResult::ERROR || negatedResult == smt::CheckResult::ERROR)
		m_errorReporter.warning(_condition.location(), "Error trying to invoke SMT solver.");
	else if (positiveResult == smt::CheckResult::CONFLICTING || negatedResult == smt::CheckResult::CONFLICTING)
		m_errorReporter.warning(_condition.location(), "At least two SMT solvers provided conflicting answers. Results might not be sound.");
	else if (positiveResult == smt::CheckResult::SATISFIABLE && negatedResult == smt::CheckResult::SATISFIABLE)
	{
		// everything fine.
	}
	else if (positiveResult == smt::CheckResult::UNSATISFIABLE && negatedResult == smt::CheckResult::UNSATISFIABLE)
		m_errorReporter.warning(_condition.location(), "Condition unreachable.");
	else
	{
		string value;
		if (positiveResult == smt::CheckResult::SATISFIABLE)
		{
			solAssert(negatedResult == smt::CheckResult::UNSATISFIABLE, "");
			value = "true";
		}
		else
		{
			solAssert(positiveResult == smt::CheckResult::UNSATISFIABLE, "");
			solAssert(negatedResult == smt::CheckResult::SATISFIABLE, "");
			value = "false";
		}
		m_errorReporter.warning(_condition.location(), boost::algorithm::replace_all_copy(_description, "$VALUE", value));
	}
}

pair<smt::CheckResult, vector<string>>
SMTChecker::checkSatisfiableAndGenerateModel(vector<smt::Expression> const& _expressionsToEvaluate)
{
	smt::CheckResult result;
	vector<string> values;
	try
	{
		tie(result, values) = m_interface->check(_expressionsToEvaluate);
	}
	catch (smt::SolverError const& _e)
	{
		string description("Error querying SMT solver");
		if (_e.comment())
			description += ": " + *_e.comment();
		m_errorReporter.warning(description);
		result = smt::CheckResult::ERROR;
	}

	for (string& value: values)
	{
		try
		{
			// Parse and re-format nicely
			value = formatNumber(bigint(value));
		}
		catch (...) { }
	}

	return make_pair(result, values);
}

smt::CheckResult SMTChecker::checkSatisfiable()
{
	return checkSatisfiableAndGenerateModel({}).first;
}

void SMTChecker::initializeFunctionCallParameters(FunctionDefinition const& _function, vector<smt::Expression> const& _callArgs)
{
	auto const& funParams = _function.parameters();
	solAssert(funParams.size() == _callArgs.size(), "");
	for (unsigned i = 0; i < funParams.size(); ++i)
		if (createVariable(*funParams[i]))
			m_interface->addAssertion(_callArgs[i] == newValue(*funParams[i]));

	for (auto const& variable: _function.localVariables())
		if (createVariable(*variable))
		{
			newValue(*variable);
			setZeroValue(*variable);
		}

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
			{
				newValue(*retParam);
				setZeroValue(*retParam);
			}
}

void SMTChecker::initializeLocalVariables(FunctionDefinition const& _function)
{
	for (auto const& variable: _function.localVariables())
		if (createVariable(*variable))
			setZeroValue(*variable);

	for (auto const& param: _function.parameters())
		if (createVariable(*param))
			setUnknownValue(*param);

	if (_function.returnParameterList())
		for (auto const& retParam: _function.returnParameters())
			if (createVariable(*retParam))
				setZeroValue(*retParam);
}

void SMTChecker::removeLocalVariables()
{
	for (auto it = m_variables.begin(); it != m_variables.end(); )
	{
		if (it->first->isLocalVariable())
			it = m_variables.erase(it);
		else
			++it;
	}
}

void SMTChecker::resetStateVariables()
{
	for (auto const& variable: m_variables)
	{
		if (variable.first->isStateVariable())
		{
			newValue(*variable.first);
			setUnknownValue(*variable.first);
		}
	}
}

void SMTChecker::resetVariables(vector<VariableDeclaration const*> _variables)
{
	for (auto const* decl: _variables)
	{
		newValue(*decl);
		setUnknownValue(*decl);
	}
}

void SMTChecker::mergeVariables(vector<VariableDeclaration const*> const& _variables, smt::Expression const& _condition, VariableIndices const& _indicesEndTrue, VariableIndices const& _indicesEndFalse)
{
	set<VariableDeclaration const*> uniqueVars(_variables.begin(), _variables.end());
	for (auto const* decl: uniqueVars)
	{
		solAssert(_indicesEndTrue.count(decl) && _indicesEndFalse.count(decl), "");
		int trueIndex = _indicesEndTrue.at(decl);
		int falseIndex = _indicesEndFalse.at(decl);
		solAssert(trueIndex != falseIndex, "");
		m_interface->addAssertion(newValue(*decl) == smt::Expression::ite(
			_condition,
			valueAtIndex(*decl, trueIndex),
			valueAtIndex(*decl, falseIndex))
		);
	}
}

bool SMTChecker::createVariable(VariableDeclaration const& _varDecl)
{
	// This might be the case for multiple calls to the same function.
	if (hasVariable(_varDecl))
		return true;
	auto const& type = _varDecl.type();
	if (isSupportedType(type->category()))
	{
		solAssert(m_variables.count(&_varDecl) == 0, "");
		m_variables.emplace(&_varDecl, newSymbolicVariable(*type, _varDecl.name() + "_" + to_string(_varDecl.id()), *m_interface));
		return true;
	}
	else
	{
		m_errorReporter.warning(
			_varDecl.location(),
			"Assertion checker does not yet support the type of this variable."
		);
		return false;
	}
}

string SMTChecker::uniqueSymbol(Expression const& _expr)
{
	return "expr_" + to_string(_expr.id());
}

bool SMTChecker::knownVariable(VariableDeclaration const& _decl)
{
	return m_variables.count(&_decl);
}

smt::Expression SMTChecker::currentValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->currentValue();
}

smt::Expression SMTChecker::valueAtIndex(VariableDeclaration const& _decl, int _index)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->valueAtIndex(_index);
}

smt::Expression SMTChecker::newValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->increaseIndex();
}

void SMTChecker::setZeroValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	m_variables.at(&_decl)->setZeroValue();
}

void SMTChecker::setUnknownValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	m_variables.at(&_decl)->setUnknownValue();
}

smt::Expression SMTChecker::expr(Expression const& _e)
{
	if (!hasExpr(_e))
	{
		m_errorReporter.warning(_e.location(), "Internal error: Expression undefined for SMT solver." );
		createExpr(_e);
	}
	return prev(m_expressions.upper_bound(&_e))->second;
}

bool SMTChecker::hasExpr(Expression const& _e) const
{
	return m_expressions.count(&_e);
}

bool SMTChecker::hasVariable(VariableDeclaration const& _var) const
{
	return m_variables.count(&_var);
}

void SMTChecker::createExpr(Expression const& _e)
{
	solAssert(_e.annotation().type, "");
	string exprSymbol = uniqueSymbol(_e) + "_" + to_string(m_expressions.count(&_e));
	switch (_e.annotation().type->category())
	{
	case Type::Category::RationalNumber:
	{
		if (RationalNumberType const* rational = dynamic_cast<RationalNumberType const*>(_e.annotation().type.get()))
			solAssert(!rational->isFractional(), "");
		m_expressions.emplace(&_e, m_interface->newInteger(exprSymbol));
		break;
	}
	case Type::Category::Address:
	case Type::Category::Integer:
		m_expressions.emplace(&_e, m_interface->newInteger(exprSymbol));
		break;
	case Type::Category::Bool:
		m_expressions.emplace(&_e, m_interface->newBool(exprSymbol));
		break;
	case Type::Category::Function:
		// This should be replaced by a `non-deterministic` type in the future.
		m_expressions.emplace(&_e, m_interface->newInteger(exprSymbol));
		break;
	default:
		m_expressions.emplace(&_e, m_interface->newInteger(exprSymbol));
		m_errorReporter.warning(
			_e.location(),
			"Assertion checker does not yet implement this type."
		);
	}
}

void SMTChecker::defineExpr(Expression const& _e, smt::Expression _value)
{
	createExpr(_e);
	m_interface->addAssertion(expr(_e) == _value);
}

void SMTChecker::popPathCondition()
{
	solAssert(m_pathConditions.size() > 0, "Cannot pop path condition, empty.");
	m_pathConditions.pop_back();
}

void SMTChecker::pushPathCondition(smt::Expression const& _e)
{
	m_pathConditions.push_back(currentPathConditions() && _e);
}

smt::Expression SMTChecker::currentPathConditions()
{
	if (m_pathConditions.empty())
		return smt::Expression(true);
	return m_pathConditions.back();
}

void SMTChecker::addPathConjoinedExpression(smt::Expression const& _e)
{
	m_interface->addAssertion(currentPathConditions() && _e);
}

void SMTChecker::addPathImpliedExpression(smt::Expression const& _e)
{
	m_interface->addAssertion(smt::Expression::implies(currentPathConditions(), _e));
}

bool SMTChecker::isRootFunction()
{
	return m_functionPath.size() == 1;
}

bool SMTChecker::visitedFunction(FunctionDefinition const* _funDef)
{
	return contains(m_functionPath, _funDef);
}

SMTChecker::VariableIndices SMTChecker::copyVariableIndices()
{
	VariableIndices indices;
	for (auto const& var: m_variables)
		indices.emplace(var.first, var.second->index());
	return indices;
}

void SMTChecker::resetVariableIndices(VariableIndices const& _indices)
{
	for (auto const& var: _indices)
		m_variables.at(var.first)->index() = var.second;
}
