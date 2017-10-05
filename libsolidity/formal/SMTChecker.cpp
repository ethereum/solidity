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

#ifdef HAVE_Z3
#include <libsolidity/formal/Z3Interface.h>
#else
#include <libsolidity/formal/SMTLib2Interface.h>
#endif

#include <libsolidity/formal/VariableUsage.h>

#include <libsolidity/interface/ErrorReporter.h>

#include <boost/range/adaptor/map.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

SMTChecker::SMTChecker(ErrorReporter& _errorReporter, ReadCallback::Callback const& _readFileCallback):
#ifdef HAVE_Z3
	m_interface(make_shared<smt::Z3Interface>()),
#else
	m_interface(make_shared<smt::SMTLib2Interface>(_readFileCallback)),
#endif
	m_errorReporter(_errorReporter)
{
	(void)_readFileCallback;
}

void SMTChecker::analyze(SourceUnit const& _source)
{
	m_variableUsage = make_shared<VariableUsage>(_source);
	if (_source.annotation().experimentalFeatures.count(ExperimentalFeature::SMTChecker))
		_source.accept(*this);
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
	m_currentFunction = &_function;
	// We only handle local variables, so we clear at the beginning of the function.
	// If we add storage variables, those should be cleared differently.
	m_interface->reset();
	m_currentSequenceCounter.clear();
	m_nextFreeSequenceCounter.clear();
	m_conditionalExecutionHappened = false;
	initializeLocalVariables(_function);
	return true;
}

void SMTChecker::endVisit(FunctionDefinition const&)
{
	// TOOD we could check for "reachability", i.e. satisfiability here.
	// We only handle local variables, so we clear at the beginning of the function.
	// If we add storage variables, those should be cleared differently.
	m_currentFunction = nullptr;
}

bool SMTChecker::visit(IfStatement const& _node)
{
	_node.condition().accept(*this);

	checkBooleanNotConstant(_node.condition(), "Condition is always $VALUE.");

	visitBranch(_node.trueStatement(), expr(_node.condition()));
	vector<Declaration const*> touchedVariables = m_variableUsage->touchedVariables(_node.trueStatement());
	if (_node.falseStatement())
	{
		visitBranch(*_node.falseStatement(), !expr(_node.condition()));
		touchedVariables += m_variableUsage->touchedVariables(*_node.falseStatement());
	}

	resetVariables(touchedVariables);

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
		checkBooleanNotConstant(_node.condition(), "Do-while loop condition is always $VALUE.");
	}
	else
	{
		_node.condition().accept(*this);
		checkBooleanNotConstant(_node.condition(), "While loop condition is always $VALUE.");

		visitBranch(_node.body(), expr(_node.condition()));
	}
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
		checkBooleanNotConstant(*_node.condition(), "For loop condition is always $VALUE.");
	}

	VariableSequenceCounters sequenceCountersStart = m_currentSequenceCounter;
	m_interface->push();
	if (_node.condition())
		m_interface->addAssertion(expr(*_node.condition()));
	_node.body().accept(*this);
	if (_node.loopExpression())
		_node.loopExpression()->accept(*this);

	m_interface->pop();

	m_conditionalExecutionHappened = true;
	m_currentSequenceCounter = sequenceCountersStart;

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

void SMTChecker::endVisit(ExpressionStatement const&)
{
}

void SMTChecker::endVisit(Assignment const& _assignment)
{
	if (_assignment.assignmentOperator() != Token::Value::Assign)
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement compound assignment."
		);
	else if (_assignment.annotation().type->category() != Type::Category::Integer)
		m_errorReporter.warning(
			_assignment.location(),
			"Assertion checker does not yet implement type " + _assignment.annotation().type->toString()
		);
	else if (Identifier const* identifier = dynamic_cast<Identifier const*>(&_assignment.leftHandSide()))
	{
		Declaration const* decl = identifier->annotation().referencedDeclaration;
		if (knownVariable(*decl))
		{
			assignment(*decl, _assignment.rightHandSide(), _assignment.location());
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
			"Assertion checker does not yet implement tules and inline arrays."
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
		"value",
		&_value
	);
	checkCondition(
		_value > maxValue(_type),
		_location,
		"Overflow (resulting value larger than " + formatNumber(_type.maxValue()) + ")",
		"value",
		&_value
	);
}

void SMTChecker::endVisit(UnaryOperation const& _op)
{
	switch (_op.getOperator())
	{
	case Token::Not: // !
	{
		solAssert(_op.annotation().type->category() == Type::Category::Bool, "");
		defineExpr(_op, !expr(_op.subExpression()));
		break;
	}
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
	{
		solAssert(_op.annotation().type->category() == Type::Category::Integer, "");
		solAssert(_op.subExpression().annotation().lValueRequested, "");
		if (Identifier const* identifier = dynamic_cast<Identifier const*>(&_op.subExpression()))
		{
			Declaration const* decl = identifier->annotation().referencedDeclaration;
			if (knownVariable(*decl))
			{
				auto innerValue = currentValue(*decl);
				auto newValue = _op.getOperator() == Token::Inc ? innerValue + 1 : innerValue - 1;
				assignment(*decl, newValue, _op.location());
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
	{
		solAssert(args.size() == 1, "");
		solAssert(args[0]->annotation().type->category() == Type::Category::Bool, "");
		checkCondition(!(expr(*args[0])), _funCall.location(), "Assertion violation");
		m_interface->addAssertion(expr(*args[0]));
	}
	else if (funType.kind() == FunctionType::Kind::Require)
	{
		solAssert(args.size() == 1, "");
		solAssert(args[0]->annotation().type->category() == Type::Category::Bool, "");
		checkBooleanNotConstant(*args[0], "Condition is always $VALUE.");
		m_interface->addAssertion(expr(*args[0]));
	}
}

void SMTChecker::endVisit(Identifier const& _identifier)
{
	Declaration const* decl = _identifier.annotation().referencedDeclaration;
	solAssert(decl, "");
	if (_identifier.annotation().lValueRequested)
	{
		// Will be translated as part of the node that requested the lvalue.
	}
	else if (dynamic_cast<IntegerType const*>(_identifier.annotation().type.get()))
		defineExpr(_identifier, currentValue(*decl));
	else if (FunctionType const* fun = dynamic_cast<FunctionType const*>(_identifier.annotation().type.get()))
	{
		if (fun->kind() == FunctionType::Kind::Assert || fun->kind() == FunctionType::Kind::Require)
			return;
	}
}

void SMTChecker::endVisit(Literal const& _literal)
{
	Type const& type = *_literal.annotation().type;
	if (type.category() == Type::Category::Integer || type.category() == Type::Category::RationalNumber)
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
		solAssert(_op.annotation().commonType->category() == Type::Category::Integer, "");
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
			checkCondition(right == 0, _op.location(), "Division by zero", "value", &right);
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
	if (_op.annotation().commonType->category() == Type::Category::Integer)
	{
		smt::Expression left(expr(_op.leftExpression()));
		smt::Expression right(expr(_op.rightExpression()));
		Token::Value op = _op.getOperator();
		smt::Expression value = (
			op == Token::Equal ? (left == right) :
			op == Token::NotEqual ? (left != right) :
			op == Token::LessThan ? (left < right) :
			op == Token::LessThanOrEqual ? (left <= right) :
			op == Token::GreaterThan ? (left > right) :
			/*op == Token::GreaterThanOrEqual*/ (left >= right)
		);
		// TODO: check that other values for op are not possible.
		defineExpr(_op, value);
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

void SMTChecker::assignment(Declaration const& _variable, Expression const& _value, SourceLocation const& _location)
{
	assignment(_variable, expr(_value), _location);
}

void SMTChecker::assignment(Declaration const& _variable, smt::Expression const& _value, SourceLocation const& _location)
{
	TypePointer type = _variable.type();
	if (auto const* intType = dynamic_cast<IntegerType const*>(type.get()))
		checkUnderOverflow(_value, *intType, _location);
	m_interface->addAssertion(newValue(_variable) == _value);
}

void SMTChecker::visitBranch(Statement const& _statement, smt::Expression _condition)
{
	visitBranch(_statement, &_condition);
}

void SMTChecker::visitBranch(Statement const& _statement, smt::Expression const* _condition)
{
	VariableSequenceCounters sequenceCountersStart = m_currentSequenceCounter;

	m_interface->push();
	if (_condition)
		m_interface->addAssertion(*_condition);
	_statement.accept(*this);
	m_interface->pop();

	m_conditionalExecutionHappened = true;
	m_currentSequenceCounter = sequenceCountersStart;
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
	m_interface->addAssertion(_condition);

	vector<smt::Expression> expressionsToEvaluate;
	vector<string> expressionNames;
	if (m_currentFunction)
	{
		if (_additionalValue)
		{
			expressionsToEvaluate.emplace_back(*_additionalValue);
			expressionNames.push_back(_additionalValueName);
		}
		for (auto const& param: m_currentFunction->parameters())
			if (knownVariable(*param))
			{
				expressionsToEvaluate.emplace_back(currentValue(*param));
				expressionNames.push_back(param->name());
			}
		for (auto const& var: m_currentFunction->localVariables())
			if (knownVariable(*var))
			{
				expressionsToEvaluate.emplace_back(currentValue(*var));
				expressionNames.push_back(var->name());
			}
	}
	smt::CheckResult result;
	vector<string> values;
	tie(result, values) = checkSatisifableAndGenerateModel(expressionsToEvaluate);

	string conditionalComment;
	if (m_conditionalExecutionHappened)
		conditionalComment =
			"\nNote that some information is erased after conditional execution of parts of the code.\n"
			"You can re-introduce information using require().";
	switch (result)
	{
	case smt::CheckResult::SATISFIABLE:
	{
		std::ostringstream message;
		message << _description << " happens here";
		if (m_currentFunction)
		{
			message << " for:\n";
			solAssert(values.size() == expressionNames.size(), "");
			for (size_t i = 0; i < values.size(); ++i)
				message << "  " << expressionNames.at(i) << " = " << values.at(i) << "\n";
		}
		else
			message << ".";
		m_errorReporter.warning(_location, message.str() + conditionalComment);
		break;
	}
	case smt::CheckResult::UNSATISFIABLE:
		break;
	case smt::CheckResult::UNKNOWN:
		m_errorReporter.warning(_location, _description + " might happen here." + conditionalComment);
		break;
	case smt::CheckResult::ERROR:
		m_errorReporter.warning(_location, "Error trying to invoke SMT solver.");
		break;
	default:
		solAssert(false, "");
	}
	m_interface->pop();
}

void SMTChecker::checkBooleanNotConstant(Expression const& _condition, string const& _description)
{
	// Do not check for const-ness if this is a constant.
	if (dynamic_cast<Literal const*>(&_condition))
		return;

	m_interface->push();
	m_interface->addAssertion(expr(_condition));
	auto positiveResult = checkSatisifable();
	m_interface->pop();

	m_interface->push();
	m_interface->addAssertion(!expr(_condition));
	auto negatedResult = checkSatisifable();
	m_interface->pop();

	if (positiveResult == smt::CheckResult::ERROR || negatedResult == smt::CheckResult::ERROR)
		m_errorReporter.warning(_condition.location(), "Error trying to invoke SMT solver.");
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
SMTChecker::checkSatisifableAndGenerateModel(vector<smt::Expression> const& _expressionsToEvaluate)
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

smt::CheckResult SMTChecker::checkSatisifable()
{
	return checkSatisifableAndGenerateModel({}).first;
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

void SMTChecker::resetVariables(vector<Declaration const*> _variables)
{
	for (auto const* decl: _variables)
	{
		newValue(*decl);
		setUnknownValue(*decl);
	}
}

bool SMTChecker::createVariable(VariableDeclaration const& _varDecl)
{
	if (dynamic_cast<IntegerType const*>(_varDecl.type().get()))
	{
		solAssert(m_currentSequenceCounter.count(&_varDecl) == 0, "");
		solAssert(m_nextFreeSequenceCounter.count(&_varDecl) == 0, "");
		solAssert(m_variables.count(&_varDecl) == 0, "");
		m_currentSequenceCounter[&_varDecl] = 0;
		m_nextFreeSequenceCounter[&_varDecl] = 1;
		m_variables.emplace(&_varDecl, m_interface->newFunction(uniqueSymbol(_varDecl), smt::Sort::Int, smt::Sort::Int));
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

string SMTChecker::uniqueSymbol(Declaration const& _decl)
{
	return _decl.name() + "_" + to_string(_decl.id());
}

string SMTChecker::uniqueSymbol(Expression const& _expr)
{
	return "expr_" + to_string(_expr.id());
}

bool SMTChecker::knownVariable(Declaration const& _decl)
{
	return m_currentSequenceCounter.count(&_decl);
}

smt::Expression SMTChecker::currentValue(Declaration const& _decl)
{
	solAssert(m_currentSequenceCounter.count(&_decl), "");
	return valueAtSequence(_decl, m_currentSequenceCounter.at(&_decl));
}

smt::Expression SMTChecker::valueAtSequence(const Declaration& _decl, int _sequence)
{
	return var(_decl)(_sequence);
}

smt::Expression SMTChecker::newValue(Declaration const& _decl)
{
	solAssert(m_nextFreeSequenceCounter.count(&_decl), "");
	m_currentSequenceCounter[&_decl] = m_nextFreeSequenceCounter[&_decl]++;
	return currentValue(_decl);
}

void SMTChecker::setZeroValue(Declaration const& _decl)
{
	solAssert(_decl.type()->category() == Type::Category::Integer, "");
	m_interface->addAssertion(currentValue(_decl) == 0);
}

void SMTChecker::setUnknownValue(Declaration const& _decl)
{
	auto const& intType = dynamic_cast<IntegerType const&>(*_decl.type());
	m_interface->addAssertion(currentValue(_decl) >= minValue(intType));
	m_interface->addAssertion(currentValue(_decl) <= maxValue(intType));
}

smt::Expression SMTChecker::minValue(IntegerType const& _t)
{
	return smt::Expression(_t.minValue());
}

smt::Expression SMTChecker::maxValue(IntegerType const& _t)
{
	return smt::Expression(_t.maxValue());
}

smt::Expression SMTChecker::expr(Expression const& _e)
{
	if (!m_expressions.count(&_e))
	{
		m_errorReporter.warning(_e.location(), "Internal error: Expression undefined for SMT solver." );
		createExpr(_e);
	}
	return m_expressions.at(&_e);
}

void SMTChecker::createExpr(Expression const& _e)
{
	if (m_expressions.count(&_e))
		m_errorReporter.warning(_e.location(), "Internal error: Expression created twice in SMT solver." );
	else
	{
		solAssert(_e.annotation().type, "");
		switch (_e.annotation().type->category())
		{
		case Type::Category::RationalNumber:
		{
			if (RationalNumberType const* rational = dynamic_cast<RationalNumberType const*>(_e.annotation().type.get()))
				solAssert(!rational->isFractional(), "");
			m_expressions.emplace(&_e, m_interface->newInteger(uniqueSymbol(_e)));
			break;
		}
		case Type::Category::Integer:
			m_expressions.emplace(&_e, m_interface->newInteger(uniqueSymbol(_e)));
			break;
		case Type::Category::Bool:
			m_expressions.emplace(&_e, m_interface->newBool(uniqueSymbol(_e)));
			break;
		default:
			solAssert(false, "Type not implemented.");
		}
	}
}

void SMTChecker::defineExpr(Expression const& _e, smt::Expression _value)
{
	createExpr(_e);
	m_interface->addAssertion(expr(_e) == _value);
}

smt::Expression SMTChecker::var(Declaration const& _decl)
{
	solAssert(m_variables.count(&_decl), "");
	return m_variables.at(&_decl);
}
