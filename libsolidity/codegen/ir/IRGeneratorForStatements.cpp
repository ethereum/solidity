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
// SPDX-License-Identifier: GPL-3.0
/**
 * Component that translates Solidity code into Yul at statement level and below.
 */

#include <libsolidity/codegen/ir/IRGeneratorForStatements.h>

#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/ir/IRGenerationContext.h>
#include <libsolidity/codegen/ir/IRLValue.h>
#include <libsolidity/codegen/ir/IRVariable.h>
#include <libsolidity/codegen/YulUtilFunctions.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/codegen/ReturnInfo.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/ast/ASTUtils.h>

#include <libevmasm/GasMeter.h>

#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/optimiser/ASTCopier.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Whiskers.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Keccak256.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/Visitor.h>

#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::frontend;
using namespace std::string_literals;

namespace
{

struct CopyTranslate: public yul::ASTCopier
{
	using ExternalRefsMap = std::map<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo>;

	CopyTranslate(yul::Dialect const& _dialect, IRGenerationContext& _context, ExternalRefsMap const& _references):
		m_dialect(_dialect), m_context(_context), m_references(_references) {}

	using ASTCopier::operator();

	yul::Expression operator()(yul::Identifier const& _identifier) override
	{
		// The operator() function is only called in lvalue context. In rvalue context,
		// only translate(yul::Identifier) is called.
		if (m_references.count(&_identifier))
			return translateReference(_identifier);
		else
			return ASTCopier::operator()(_identifier);
	}

	yul::YulString translateIdentifier(yul::YulString _name) override
	{
		// Strictly, the dialect used by inline assembly (m_dialect) could be different
		// from the Yul dialect we are compiling to. So we are assuming here that the builtin
		// functions are identical. This should not be a problem for now since everything
		// is EVM anyway.
		if (m_dialect.builtin(_name))
			return _name;
		else
			return yul::YulString{"usr$" + _name.str()};
	}

	yul::Identifier translate(yul::Identifier const& _identifier) override
	{
		if (!m_references.count(&_identifier))
			return ASTCopier::translate(_identifier);

		yul::Expression translated = translateReference(_identifier);
		solAssert(holds_alternative<yul::Identifier>(translated), "");
		return get<yul::Identifier>(std::move(translated));
	}

private:

	/// Translates a reference to a local variable, potentially including
	/// a suffix. Might return a literal, which causes this to be invalid in
	/// lvalue-context.
	yul::Expression translateReference(yul::Identifier const& _identifier)
	{
		auto const& reference = m_references.at(&_identifier);
		auto const varDecl = dynamic_cast<VariableDeclaration const*>(reference.declaration);
		solUnimplementedAssert(varDecl, "");
		string const& suffix = reference.suffix;

		string value;
		if (suffix.empty() && varDecl->isLocalVariable())
		{
			auto const& var = m_context.localVariable(*varDecl);
			solAssert(var.type().sizeOnStack() == 1, "");

			value = var.commaSeparatedList();
		}
		else if (varDecl->isConstant())
		{
			VariableDeclaration const* variable = rootConstVariableDeclaration(*varDecl);
			solAssert(variable, "");

			if (variable->value()->annotation().type->category() == Type::Category::RationalNumber)
			{
				u256 intValue = dynamic_cast<RationalNumberType const&>(*variable->value()->annotation().type).literalValue(nullptr);
				if (auto const* bytesType = dynamic_cast<FixedBytesType const*>(variable->type()))
					intValue <<= 256 - 8 * bytesType->numBytes();
				else
					solAssert(variable->type()->category() == Type::Category::Integer, "");
				value = intValue.str();
			}
			else if (auto const* literal = dynamic_cast<Literal const*>(variable->value().get()))
			{
				Type const* type = literal->annotation().type;

				switch (type->category())
				{
				case Type::Category::Bool:
				case Type::Category::Address:
					solAssert(type->category() == variable->annotation().type->category(), "");
					value = toCompactHexWithPrefix(type->literalValue(literal));
					break;
				case Type::Category::StringLiteral:
				{
					auto const& stringLiteral = dynamic_cast<StringLiteralType const&>(*type);
					solAssert(variable->type()->category() == Type::Category::FixedBytes, "");
					unsigned const numBytes = dynamic_cast<FixedBytesType const&>(*variable->type()).numBytes();
					solAssert(stringLiteral.value().size() <= numBytes, "");
					value = formatNumber(u256(h256(stringLiteral.value(), h256::AlignLeft)));
					break;
				}
				default:
					solAssert(false, "");
				}
			}
			else
				solAssert(false, "Invalid constant in inline assembly.");
		}
		else if (varDecl->isStateVariable())
		{
			if (suffix == "slot")
				value = m_context.storageLocationOfStateVariable(*varDecl).first.str();
			else if (suffix == "offset")
				value = to_string(m_context.storageLocationOfStateVariable(*varDecl).second);
			else
				solAssert(false, "");
		}
		else if (varDecl->type()->dataStoredIn(DataLocation::Storage))
		{
			solAssert(suffix == "slot" || suffix == "offset", "");
			solAssert(varDecl->isLocalVariable(), "");
			if (suffix == "slot")
				value = IRVariable{*varDecl}.part("slot").name();
			else if (varDecl->type()->isValueType())
				value = IRVariable{*varDecl}.part("offset").name();
			else
			{
				solAssert(!IRVariable{*varDecl}.hasPart("offset"), "");
				value = "0";
			}
		}
		else if (varDecl->type()->dataStoredIn(DataLocation::CallData))
		{
			solAssert(suffix == "offset" || suffix == "length", "");
			value = IRVariable{*varDecl}.part(suffix).name();
		}
		else
			solAssert(false, "");

		if (isdigit(value.front()))
			return yul::Literal{_identifier.debugData, yul::LiteralKind::Number, yul::YulString{value}, {}};
		else
			return yul::Identifier{_identifier.debugData, yul::YulString{value}};
	}


	yul::Dialect const& m_dialect;
	IRGenerationContext& m_context;
	ExternalRefsMap const& m_references;
};

}

string IRGeneratorForStatementsBase::code() const
{
	return m_code.str();
}

std::ostringstream& IRGeneratorForStatementsBase::appendCode(bool _addLocationComment)
{
	if (
		_addLocationComment &&
		m_currentLocation.isValid() &&
		m_lastLocation != m_currentLocation
	)
		m_code << sourceLocationComment(m_currentLocation, m_context) << "\n";

	m_lastLocation = m_currentLocation;

	return m_code;
}

void IRGeneratorForStatementsBase::setLocation(ASTNode const& _node)
{
	m_currentLocation = _node.location();
}

string IRGeneratorForStatements::code() const
{
	solAssert(!m_currentLValue, "LValue not reset!");
	return IRGeneratorForStatementsBase::code();
}

void IRGeneratorForStatements::generate(Block const& _block)
{
	try
	{
		_block.accept(*this);
	}
	catch (langutil::UnimplementedFeatureError const& _error)
	{
		if (!boost::get_error_info<langutil::errinfo_sourceLocation>(_error))
			_error << langutil::errinfo_sourceLocation(m_currentLocation);
		BOOST_THROW_EXCEPTION(_error);
	}
}

void IRGeneratorForStatements::initializeStateVar(VariableDeclaration const& _varDecl)
{
	try
	{
		setLocation(_varDecl);

		solAssert(_varDecl.immutable() || m_context.isStateVariable(_varDecl), "Must be immutable or a state variable.");
		solAssert(!_varDecl.isConstant(), "");
		if (!_varDecl.value())
			return;

		_varDecl.value()->accept(*this);

		writeToLValue(
			_varDecl.immutable() ?
			IRLValue{*_varDecl.annotation().type, IRLValue::Immutable{&_varDecl}} :
			IRLValue{*_varDecl.annotation().type, IRLValue::Storage{
				util::toCompactHexWithPrefix(m_context.storageLocationOfStateVariable(_varDecl).first),
				m_context.storageLocationOfStateVariable(_varDecl).second
			}},
			*_varDecl.value()
		);
	}
	catch (langutil::UnimplementedFeatureError const& _error)
	{
		if (!boost::get_error_info<langutil::errinfo_sourceLocation>(_error))
			_error << langutil::errinfo_sourceLocation(m_currentLocation);
		BOOST_THROW_EXCEPTION(_error);
	}
}

void IRGeneratorForStatements::initializeLocalVar(VariableDeclaration const& _varDecl)
{
	try
	{
		setLocation(_varDecl);

		solAssert(m_context.isLocalVariable(_varDecl), "Must be a local variable.");

		auto const* type = _varDecl.type();
		if (dynamic_cast<MappingType const*>(type))
			return;
		else if (auto const* refType = dynamic_cast<ReferenceType const*>(type))
			if (refType->dataStoredIn(DataLocation::Storage) && refType->isPointer())
				return;

		IRVariable zero = zeroValue(*type);
		assign(m_context.localVariable(_varDecl), zero);
	}
	catch (langutil::UnimplementedFeatureError const& _error)
	{
		if (!boost::get_error_info<langutil::errinfo_sourceLocation>(_error))
			_error << langutil::errinfo_sourceLocation(m_currentLocation);
		BOOST_THROW_EXCEPTION(_error);
	}
}

IRVariable IRGeneratorForStatements::evaluateExpression(Expression const& _expression, Type const& _targetType)
{
	try
	{
		setLocation(_expression);

		_expression.accept(*this);

		setLocation(_expression);
		IRVariable variable{m_context.newYulVariable(), _targetType};
		define(variable, _expression);
		return variable;
	}
	catch (langutil::UnimplementedFeatureError const& _error)
	{
		if (!boost::get_error_info<langutil::errinfo_sourceLocation>(_error))
			_error << langutil::errinfo_sourceLocation(m_currentLocation);
		BOOST_THROW_EXCEPTION(_error);
	}
}

string IRGeneratorForStatements::constantValueFunction(VariableDeclaration const& _constant)
{
	try
	{
		string functionName = IRNames::constantValueFunction(_constant);
		return m_context.functionCollector().createFunction(functionName, [&] {
			Whiskers templ(R"(
				<sourceLocationComment>
				function <functionName>() -> <ret> {
					<code>
					<ret> := <value>
				}
			)");
			templ("sourceLocationComment", sourceLocationComment(_constant, m_context));
			templ("functionName", functionName);
			IRGeneratorForStatements generator(m_context, m_utils);
			solAssert(_constant.value(), "");
			Type const& constantType = *_constant.type();
			templ("value", generator.evaluateExpression(*_constant.value(), constantType).commaSeparatedList());
			templ("code", generator.code());
			templ("ret", IRVariable("ret", constantType).commaSeparatedList());

			return templ.render();
		});
	}
	catch (langutil::UnimplementedFeatureError const& _error)
	{
		if (!boost::get_error_info<langutil::errinfo_sourceLocation>(_error))
			_error << langutil::errinfo_sourceLocation(m_currentLocation);
		BOOST_THROW_EXCEPTION(_error);
	}
}

void IRGeneratorForStatements::endVisit(VariableDeclarationStatement const& _varDeclStatement)
{
	setLocation(_varDeclStatement);

	if (Expression const* expression = _varDeclStatement.initialValue())
	{
		if (_varDeclStatement.declarations().size() > 1)
		{
			auto const* tupleType = dynamic_cast<TupleType const*>(expression->annotation().type);
			solAssert(tupleType, "Expected expression of tuple type.");
			solAssert(_varDeclStatement.declarations().size() == tupleType->components().size(), "Invalid number of tuple components.");
			for (size_t i = 0; i < _varDeclStatement.declarations().size(); ++i)
				if (auto const& decl = _varDeclStatement.declarations()[i])
				{
					solAssert(tupleType->components()[i], "");
					define(m_context.addLocalVariable(*decl), IRVariable(*expression).tupleComponent(i));
				}
		}
		else
		{
			VariableDeclaration const& varDecl = *_varDeclStatement.declarations().front();
			define(m_context.addLocalVariable(varDecl), *expression);
		}
	}
	else
		for (auto const& decl: _varDeclStatement.declarations())
			if (decl)
			{
				declare(m_context.addLocalVariable(*decl));
				initializeLocalVar(*decl);
			}
}

bool IRGeneratorForStatements::visit(Conditional const& _conditional)
{
	_conditional.condition().accept(*this);

	setLocation(_conditional);

	string condition = expressionAsType(_conditional.condition(), *TypeProvider::boolean());
	declare(_conditional);

	appendCode() << "switch " << condition << "\n" "case 0 {\n";

	_conditional.falseExpression().accept(*this);
	setLocation(_conditional);

	assign(_conditional, _conditional.falseExpression());
	appendCode() << "}\n" "default {\n";

	_conditional.trueExpression().accept(*this);
	setLocation(_conditional);

	assign(_conditional, _conditional.trueExpression());
	appendCode() << "}\n";

	return false;
}

bool IRGeneratorForStatements::visit(Assignment const& _assignment)
{
	_assignment.rightHandSide().accept(*this);
	setLocation(_assignment);

	Token assignmentOperator = _assignment.assignmentOperator();
	Token binaryOperator =
		assignmentOperator == Token::Assign ?
		assignmentOperator :
		TokenTraits::AssignmentToBinaryOp(assignmentOperator);

	if (TokenTraits::isShiftOp(binaryOperator))
		solAssert(type(_assignment.rightHandSide()).mobileType(), "");
	IRVariable value =
		type(_assignment.leftHandSide()).isValueType() ?
		convert(
			_assignment.rightHandSide(),
			TokenTraits::isShiftOp(binaryOperator) ? *type(_assignment.rightHandSide()).mobileType() : type(_assignment)
		) :
		_assignment.rightHandSide();

	_assignment.leftHandSide().accept(*this);

	solAssert(!!m_currentLValue, "LValue not retrieved.");
	setLocation(_assignment);

	if (assignmentOperator != Token::Assign)
	{
		solAssert(type(_assignment.leftHandSide()).isValueType(), "Compound operators only available for value types.");
		solAssert(binaryOperator != Token::Exp, "");
		solAssert(type(_assignment) == type(_assignment.leftHandSide()), "");

		IRVariable leftIntermediate = readFromLValue(*m_currentLValue);
		solAssert(type(_assignment) == leftIntermediate.type(), "");

		define(_assignment) << (
			TokenTraits::isShiftOp(binaryOperator) ?
			shiftOperation(binaryOperator, leftIntermediate, value) :
			binaryOperation(binaryOperator, type(_assignment), leftIntermediate.name(), value.name())
		) << "\n";

		writeToLValue(*m_currentLValue, IRVariable(_assignment));
	}
	else
	{
		writeToLValue(*m_currentLValue, value);

		if (dynamic_cast<ReferenceType const*>(&m_currentLValue->type))
			define(_assignment, readFromLValue(*m_currentLValue));
		else if (*_assignment.annotation().type != *TypeProvider::emptyTuple())
			define(_assignment, value);
	}

	m_currentLValue.reset();
	return false;
}

bool IRGeneratorForStatements::visit(TupleExpression const& _tuple)
{
	setLocation(_tuple);

	if (_tuple.isInlineArray())
	{
		auto const& arrayType = dynamic_cast<ArrayType const&>(*_tuple.annotation().type);
		solAssert(!arrayType.isDynamicallySized(), "Cannot create dynamically sized inline array.");
		define(_tuple) <<
			m_utils.allocateMemoryArrayFunction(arrayType) <<
			"(" <<
			_tuple.components().size() <<
			")\n";

		string mpos = IRVariable(_tuple).part("mpos").name();
		Type const& baseType = *arrayType.baseType();
		for (size_t i = 0; i < _tuple.components().size(); i++)
		{
			Expression const& component = *_tuple.components()[i];
			component.accept(*this);
			setLocation(_tuple);
			IRVariable converted = convert(component, baseType);
			appendCode() <<
				m_utils.writeToMemoryFunction(baseType) <<
				"(" <<
				("add(" + mpos + ", " + to_string(i * arrayType.memoryStride()) + ")") <<
				", " <<
				converted.commaSeparatedList() <<
				")\n";
		}
	}
	else
	{
		bool willBeWrittenTo = _tuple.annotation().willBeWrittenTo;
		if (willBeWrittenTo)
			solAssert(!m_currentLValue, "");
		if (_tuple.components().size() == 1)
		{
			solAssert(_tuple.components().front(), "");
			_tuple.components().front()->accept(*this);
			setLocation(_tuple);
			if (willBeWrittenTo)
				solAssert(!!m_currentLValue, "");
			else
				define(_tuple, *_tuple.components().front());
		}
		else
		{
			vector<optional<IRLValue>> lvalues;
			for (size_t i = 0; i < _tuple.components().size(); ++i)
				if (auto const& component = _tuple.components()[i])
				{
					component->accept(*this);
					setLocation(_tuple);
					if (willBeWrittenTo)
					{
						solAssert(!!m_currentLValue, "");
						lvalues.emplace_back(std::move(m_currentLValue));
						m_currentLValue.reset();
					}
					else
						define(IRVariable(_tuple).tupleComponent(i), *component);
				}
				else if (willBeWrittenTo)
					lvalues.emplace_back();

			if (_tuple.annotation().willBeWrittenTo)
				m_currentLValue.emplace(IRLValue{
					*_tuple.annotation().type,
					IRLValue::Tuple{std::move(lvalues)}
				});
		}
	}
	return false;
}

bool IRGeneratorForStatements::visit(Block const& _block)
{
	if (_block.unchecked())
	{
		solAssert(m_context.arithmetic() == Arithmetic::Checked, "");
		m_context.setArithmetic(Arithmetic::Wrapping);
	}
	return true;
}

void IRGeneratorForStatements::endVisit(Block const& _block)
{
	if (_block.unchecked())
	{
		solAssert(m_context.arithmetic() == Arithmetic::Wrapping, "");
		m_context.setArithmetic(Arithmetic::Checked);
	}
}

bool IRGeneratorForStatements::visit(IfStatement const& _ifStatement)
{
	_ifStatement.condition().accept(*this);
	setLocation(_ifStatement);
	string condition = expressionAsType(_ifStatement.condition(), *TypeProvider::boolean());

	if (_ifStatement.falseStatement())
	{
		appendCode() << "switch " << condition << "\n" "case 0 {\n";
		_ifStatement.falseStatement()->accept(*this);
		setLocation(_ifStatement);
		appendCode() << "}\n" "default {\n";
	}
	else
		appendCode() << "if " << condition << " {\n";
	_ifStatement.trueStatement().accept(*this);
	setLocation(_ifStatement);
	appendCode() << "}\n";

	return false;
}

void IRGeneratorForStatements::endVisit(PlaceholderStatement const& _placeholder)
{
	solAssert(m_placeholderCallback, "");
	setLocation(_placeholder);
	appendCode() << m_placeholderCallback();
}

bool IRGeneratorForStatements::visit(ForStatement const& _forStatement)
{
	setLocation(_forStatement);
	generateLoop(
		_forStatement.body(),
		_forStatement.condition(),
		_forStatement.initializationExpression(),
		_forStatement.loopExpression()
	);

	return false;
}

bool IRGeneratorForStatements::visit(WhileStatement const& _whileStatement)
{
	setLocation(_whileStatement);
	generateLoop(
		_whileStatement.body(),
		&_whileStatement.condition(),
		nullptr,
		nullptr,
		_whileStatement.isDoWhile()
	);

	return false;
}

bool IRGeneratorForStatements::visit(Continue const& _continue)
{
	setLocation(_continue);
	appendCode() << "continue\n";
	return false;
}

bool IRGeneratorForStatements::visit(Break const& _break)
{
	setLocation(_break);
	appendCode() << "break\n";
	return false;
}

void IRGeneratorForStatements::endVisit(Return const& _return)
{
	setLocation(_return);
	if (Expression const* value = _return.expression())
	{
		solAssert(_return.annotation().functionReturnParameters, "Invalid return parameters pointer.");
		vector<ASTPointer<VariableDeclaration>> const& returnParameters =
			_return.annotation().functionReturnParameters->parameters();
		if (returnParameters.size() > 1)
			for (size_t i = 0; i < returnParameters.size(); ++i)
				assign(m_context.localVariable(*returnParameters[i]), IRVariable(*value).tupleComponent(i));
		else if (returnParameters.size() == 1)
			assign(m_context.localVariable(*returnParameters.front()), *value);
	}
	appendCode() << "leave\n";
}

bool IRGeneratorForStatements::visit(UnaryOperation const& _unaryOperation)
{
	setLocation(_unaryOperation);
	Type const& resultType = type(_unaryOperation);
	Token const op = _unaryOperation.getOperator();

	if (resultType.category() == Type::Category::RationalNumber)
	{
		define(_unaryOperation) << formatNumber(resultType.literalValue(nullptr)) << "\n";
		return false;
	}

	_unaryOperation.subExpression().accept(*this);
	setLocation(_unaryOperation);

	if (op == Token::Delete)
	{
		solAssert(!!m_currentLValue, "LValue not retrieved.");
		std::visit(
			util::GenericVisitor{
				[&](IRLValue::Storage const& _storage) {
					appendCode() <<
						m_utils.storageSetToZeroFunction(m_currentLValue->type) <<
						"(" <<
						_storage.slot <<
						", " <<
						_storage.offsetString() <<
						")\n";
					m_currentLValue.reset();
				},
				[&](auto const&) {
					IRVariable zeroValue(m_context.newYulVariable(), m_currentLValue->type);
					define(zeroValue) << m_utils.zeroValueFunction(m_currentLValue->type) << "()\n";
					writeToLValue(*m_currentLValue, zeroValue);
					m_currentLValue.reset();
				}
			},
			m_currentLValue->kind
		);
	}
	else if (resultType.category() == Type::Category::Integer)
	{
		solAssert(resultType == type(_unaryOperation.subExpression()), "Result type doesn't match!");

		if (op == Token::Inc || op == Token::Dec)
		{
			solAssert(!!m_currentLValue, "LValue not retrieved.");
			IRVariable modifiedValue(m_context.newYulVariable(), resultType);
			IRVariable originalValue = readFromLValue(*m_currentLValue);

			bool checked = m_context.arithmetic() == Arithmetic::Checked;
			define(modifiedValue) <<
				(op == Token::Inc ?
					(checked ? m_utils.incrementCheckedFunction(resultType) : m_utils.incrementWrappingFunction(resultType)) :
					(checked ? m_utils.decrementCheckedFunction(resultType) : m_utils.decrementWrappingFunction(resultType))
				) <<
				"(" <<
				originalValue.name() <<
				")\n";
			writeToLValue(*m_currentLValue, modifiedValue);
			m_currentLValue.reset();

			define(_unaryOperation, _unaryOperation.isPrefixOperation() ? modifiedValue : originalValue);
		}
		else if (op == Token::BitNot)
			appendSimpleUnaryOperation(_unaryOperation, _unaryOperation.subExpression());
		else if (op == Token::Add)
			// According to SyntaxChecker...
			solAssert(false, "Use of unary + is disallowed.");
		else if (op == Token::Sub)
		{
			IntegerType const& intType = *dynamic_cast<IntegerType const*>(&resultType);
			define(_unaryOperation) << (
				m_context.arithmetic() == Arithmetic::Checked ?
				m_utils.negateNumberCheckedFunction(intType) :
				m_utils.negateNumberWrappingFunction(intType)
			) << "(" << IRVariable(_unaryOperation.subExpression()).name() << ")\n";
		}
		else
			solUnimplementedAssert(false, "Unary operator not yet implemented");
	}
	else if (resultType.category() == Type::Category::FixedBytes)
	{
		solAssert(op == Token::BitNot, "Only bitwise negation is allowed for FixedBytes");
		solAssert(resultType == type(_unaryOperation.subExpression()), "Result type doesn't match!");
		appendSimpleUnaryOperation(_unaryOperation, _unaryOperation.subExpression());
	}
	else if (resultType.category() == Type::Category::Bool)
	{
		solAssert(
			op != Token::BitNot,
			"Bitwise Negation can't be done on bool!"
		);

		appendSimpleUnaryOperation(_unaryOperation, _unaryOperation.subExpression());
	}
	else
		solUnimplementedAssert(false, "Unary operator not yet implemented");

	return false;
}

bool IRGeneratorForStatements::visit(BinaryOperation const& _binOp)
{
	setLocation(_binOp);

	solAssert(!!_binOp.annotation().commonType, "");
	Type const* commonType = _binOp.annotation().commonType;
	langutil::Token op = _binOp.getOperator();

	if (op == Token::And || op == Token::Or)
	{
		// This can short-circuit!
		appendAndOrOperatorCode(_binOp);
		return false;
	}

	if (commonType->category() == Type::Category::RationalNumber)
	{
		define(_binOp) << toCompactHexWithPrefix(commonType->literalValue(nullptr)) << "\n";
		return false; // skip sub-expressions
	}

	_binOp.leftExpression().accept(*this);
	_binOp.rightExpression().accept(*this);
	setLocation(_binOp);

	if (TokenTraits::isCompareOp(op))
	{
		if (auto type = dynamic_cast<FunctionType const*>(commonType))
		{
			solAssert(op == Token::Equal || op == Token::NotEqual, "Invalid function pointer comparison!");
			solAssert(type->kind() != FunctionType::Kind::External, "External function comparison not allowed!");
		}

		solAssert(commonType->isValueType(), "");
		bool isSigned = false;
		if (auto type = dynamic_cast<IntegerType const*>(commonType))
			isSigned = type->isSigned();

		string args =
			expressionAsType(_binOp.leftExpression(), *commonType, true) +
			", " +
			expressionAsType(_binOp.rightExpression(), *commonType, true);

		string expr;
		if (op == Token::Equal)
			expr = "eq(" + move(args) + ")";
		else if (op == Token::NotEqual)
			expr = "iszero(eq(" + move(args) + "))";
		else if (op == Token::GreaterThanOrEqual)
			expr = "iszero(" + string(isSigned ? "slt(" : "lt(") + move(args) + "))";
		else if (op == Token::LessThanOrEqual)
			expr = "iszero(" + string(isSigned ? "sgt(" : "gt(") + move(args) + "))";
		else if (op == Token::GreaterThan)
			expr = (isSigned ? "sgt(" : "gt(") + move(args) + ")";
		else if (op == Token::LessThan)
			expr = (isSigned ? "slt(" : "lt(") + move(args) + ")";
		else
			solAssert(false, "Unknown comparison operator.");
		define(_binOp) << expr << "\n";
	}
	else if (op == Token::Exp)
	{
		IRVariable left = convert(_binOp.leftExpression(), *commonType);
		IRVariable right = convert(_binOp.rightExpression(), *type(_binOp.rightExpression()).mobileType());

		if (m_context.arithmetic() == Arithmetic::Wrapping)
			define(_binOp) << m_utils.wrappingIntExpFunction(
				dynamic_cast<IntegerType const&>(left.type()),
				dynamic_cast<IntegerType const&>(right.type())
			) << "(" << left.name() << ", " << right.name() << ")\n";
		else if (auto rationalNumberType = dynamic_cast<RationalNumberType const*>(_binOp.leftExpression().annotation().type))
		{
			solAssert(rationalNumberType->integerType(), "Invalid literal as the base for exponentiation.");
			solAssert(dynamic_cast<IntegerType const*>(commonType), "");

			define(_binOp) << m_utils.overflowCheckedIntLiteralExpFunction(
				*rationalNumberType,
				dynamic_cast<IntegerType const&>(right.type()),
				dynamic_cast<IntegerType const&>(*commonType)
			) << "(" << right.name() << ")\n";
		}
		else
			define(_binOp) << m_utils.overflowCheckedIntExpFunction(
				dynamic_cast<IntegerType const&>(left.type()),
				dynamic_cast<IntegerType const&>(right.type())
			) << "(" << left.name() << ", " << right.name() << ")\n";
	}
	else if (TokenTraits::isShiftOp(op))
	{
		IRVariable left = convert(_binOp.leftExpression(), *commonType);
		IRVariable right = convert(_binOp.rightExpression(), *type(_binOp.rightExpression()).mobileType());
		define(_binOp) << shiftOperation(_binOp.getOperator(), left, right) << "\n";
	}
	else
	{
		string left = expressionAsType(_binOp.leftExpression(), *commonType);
		string right = expressionAsType(_binOp.rightExpression(), *commonType);
		define(_binOp) << binaryOperation(_binOp.getOperator(), *commonType, left, right) << "\n";
	}
	return false;
}

void IRGeneratorForStatements::endVisit(FunctionCall const& _functionCall)
{
	setLocation(_functionCall);
	auto functionCallKind = *_functionCall.annotation().kind;

	if (functionCallKind == FunctionCallKind::TypeConversion)
	{
		solAssert(
			_functionCall.expression().annotation().type->category() == Type::Category::TypeType,
			"Expected category to be TypeType"
		);
		solAssert(_functionCall.arguments().size() == 1, "Expected one argument for type conversion");
		define(_functionCall, *_functionCall.arguments().front());
		return;
	}

	FunctionTypePointer functionType = nullptr;
	if (functionCallKind == FunctionCallKind::StructConstructorCall)
	{
		auto const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());
		functionType = structType.constructorType();
	}
	else
		functionType = dynamic_cast<FunctionType const*>(_functionCall.expression().annotation().type);

	TypePointers parameterTypes = functionType->parameterTypes();

	vector<ASTPointer<Expression const>> const& arguments = _functionCall.sortedArguments();

	if (functionCallKind == FunctionCallKind::StructConstructorCall)
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());

		define(_functionCall) << m_utils.allocateMemoryStructFunction(structType) << "()\n";

		MemberList::MemberMap members = structType.nativeMembers(nullptr);

		solAssert(members.size() == arguments.size(), "Struct parameter mismatch.");

		for (size_t i = 0; i < arguments.size(); i++)
		{
			IRVariable converted = convert(*arguments[i], *parameterTypes[i]);
			appendCode() <<
				m_utils.writeToMemoryFunction(*functionType->parameterTypes()[i]) <<
				"(add(" <<
				IRVariable(_functionCall).part("mpos").name() <<
				", " <<
				structType.memoryOffsetOfMember(members[i].name) <<
				"), " <<
				converted.commaSeparatedList() <<
				")\n";
		}

		return;
	}

	switch (functionType->kind())
	{
	case FunctionType::Kind::Declaration:
		solAssert(false, "Attempted to generate code for calling a function definition.");
		break;
	case FunctionType::Kind::Internal:
	{
		FunctionDefinition const* functionDef = ASTNode::resolveFunctionCall(_functionCall, &m_context.mostDerivedContract());

		solAssert(!functionType->takesArbitraryParameters(), "");

		vector<string> args;
		if (functionType->bound())
			args += IRVariable(_functionCall.expression()).part("self").stackSlots();

		for (size_t i = 0; i < arguments.size(); ++i)
			args += convert(*arguments[i], *parameterTypes[i]).stackSlots();

		if (functionDef)
		{
			solAssert(functionDef->isImplemented(), "");

			define(_functionCall) <<
				m_context.enqueueFunctionForCodeGeneration(*functionDef) <<
				"(" <<
				joinHumanReadable(args) <<
				")\n";
		}
		else
		{
			YulArity arity = YulArity::fromType(*functionType);
			m_context.internalFunctionCalledThroughDispatch(arity);

			define(_functionCall) <<
				IRNames::internalDispatch(arity) <<
				"(" <<
				IRVariable(_functionCall.expression()).part("functionIdentifier").name() <<
				joinHumanReadablePrefixed(args) <<
				")\n";
		}
		break;
	}
	case FunctionType::Kind::External:
	case FunctionType::Kind::DelegateCall:
		appendExternalFunctionCall(_functionCall, arguments);
		break;
	case FunctionType::Kind::BareCall:
	case FunctionType::Kind::BareDelegateCall:
	case FunctionType::Kind::BareStaticCall:
		appendBareCall(_functionCall, arguments);
		break;
	case FunctionType::Kind::BareCallCode:
		solAssert(false, "Callcode has been removed.");
	case FunctionType::Kind::Event:
	{
		auto const& event = dynamic_cast<EventDefinition const&>(functionType->declaration());
		TypePointers paramTypes = functionType->parameterTypes();
		ABIFunctions abi(m_context.evmVersion(), m_context.revertStrings(), m_context.functionCollector());

		vector<IRVariable> indexedArgs;
		vector<string> nonIndexedArgs;
		TypePointers nonIndexedArgTypes;
		TypePointers nonIndexedParamTypes;
		if (!event.isAnonymous())
			define(indexedArgs.emplace_back(m_context.newYulVariable(), *TypeProvider::uint256())) <<
				formatNumber(u256(h256::Arith(keccak256(functionType->externalSignature())))) << "\n";
		for (size_t i = 0; i < event.parameters().size(); ++i)
		{
			Expression const& arg = *arguments[i];
			if (event.parameters()[i]->isIndexed())
			{
				string value;
				if (auto const& referenceType = dynamic_cast<ReferenceType const*>(paramTypes[i]))
					define(indexedArgs.emplace_back(m_context.newYulVariable(), *TypeProvider::uint256())) <<
						m_utils.packedHashFunction({arg.annotation().type}, {referenceType}) <<
						"(" <<
						IRVariable(arg).commaSeparatedList() <<
						")\n";
				else if (auto functionType = dynamic_cast<FunctionType const*>(paramTypes[i]))
				{
					solAssert(
						IRVariable(arg).type() == *functionType &&
						functionType->kind() == FunctionType::Kind::External &&
						!functionType->bound(),
						""
					);
					define(indexedArgs.emplace_back(m_context.newYulVariable(), *TypeProvider::fixedBytes(32))) <<
						m_utils.combineExternalFunctionIdFunction() <<
						"(" <<
						IRVariable(arg).commaSeparatedList() <<
						")\n";
				}
				else
					indexedArgs.emplace_back(convert(arg, *paramTypes[i]));
			}
			else
			{
				nonIndexedArgs += IRVariable(arg).stackSlots();
				nonIndexedArgTypes.push_back(arg.annotation().type);
				nonIndexedParamTypes.push_back(paramTypes[i]);
			}
		}
		solAssert(indexedArgs.size() <= 4, "Too many indexed arguments.");
		Whiskers templ(R"({
			let <pos> := <allocateUnbounded>()
			let <end> := <encode>(<pos> <nonIndexedArgs>)
			<log>(<pos>, sub(<end>, <pos>) <indexedArgs>)
		})");
		templ("pos", m_context.newYulVariable());
		templ("end", m_context.newYulVariable());
		templ("allocateUnbounded", m_utils.allocateUnboundedFunction());
		templ("encode", abi.tupleEncoder(nonIndexedArgTypes, nonIndexedParamTypes));
		templ("nonIndexedArgs", joinHumanReadablePrefixed(nonIndexedArgs));
		templ("log", "log" + to_string(indexedArgs.size()));
		templ("indexedArgs", joinHumanReadablePrefixed(indexedArgs | ranges::views::transform([&](auto const& _arg) {
			return _arg.commaSeparatedList();
		})));
		appendCode() << templ.render();
		break;
	}
	case FunctionType::Kind::Error:
	{
		ErrorDefinition const* error = dynamic_cast<ErrorDefinition const*>(ASTNode::referencedDeclaration(_functionCall.expression()));
		solAssert(error, "");
		revertWithError(
			error->functionType(true)->externalSignature(),
			error->functionType(true)->parameterTypes(),
			_functionCall.sortedArguments()
		);
		break;
	}
	case FunctionType::Kind::Assert:
	case FunctionType::Kind::Require:
	{
		solAssert(arguments.size() > 0, "Expected at least one parameter for require/assert");
		solAssert(arguments.size() <= 2, "Expected no more than two parameters for require/assert");

		Type const* messageArgumentType =
			arguments.size() > 1 && m_context.revertStrings() != RevertStrings::Strip ?
			arguments[1]->annotation().type :
			nullptr;
		string requireOrAssertFunction = m_utils.requireOrAssertFunction(
			functionType->kind() == FunctionType::Kind::Assert,
			messageArgumentType
		);

		appendCode() << move(requireOrAssertFunction) << "(" << IRVariable(*arguments[0]).name();
		if (messageArgumentType && messageArgumentType->sizeOnStack() > 0)
			appendCode() << ", " << IRVariable(*arguments[1]).commaSeparatedList();
		appendCode() << ")\n";

		break;
	}
	case FunctionType::Kind::ABIEncode:
	case FunctionType::Kind::ABIEncodePacked:
	case FunctionType::Kind::ABIEncodeWithSelector:
	case FunctionType::Kind::ABIEncodeWithSignature:
	{
		bool const isPacked = functionType->kind() == FunctionType::Kind::ABIEncodePacked;
		solAssert(functionType->padArguments() != isPacked, "");
		bool const hasSelectorOrSignature =
			functionType->kind() == FunctionType::Kind::ABIEncodeWithSelector ||
			functionType->kind() == FunctionType::Kind::ABIEncodeWithSignature;

		TypePointers argumentTypes;
		TypePointers targetTypes;
		vector<string> argumentVars;
		for (size_t i = 0; i < arguments.size(); ++i)
		{
			// ignore selector
			if (hasSelectorOrSignature && i == 0)
				continue;
			argumentTypes.emplace_back(&type(*arguments[i]));
			targetTypes.emplace_back(type(*arguments[i]).fullEncodingType(false, true, isPacked));
			argumentVars += IRVariable(*arguments[i]).stackSlots();
		}

		string selector;
		if (functionType->kind() == FunctionType::Kind::ABIEncodeWithSignature)
		{
			// hash the signature
			Type const& selectorType = type(*arguments.front());
			if (auto const* stringType = dynamic_cast<StringLiteralType const*>(&selectorType))
				selector = formatNumber(util::selectorFromSignature(stringType->value()));
			else
			{
				// Used to reset the free memory pointer later.
				// TODO This is an abuse of the `allocateUnbounded` function.
				// We might want to introduce a new set of memory handling functions here
				// a la "setMemoryCheckPoint" and "freeUntilCheckPoint".
				string freeMemoryPre = m_context.newYulVariable();
				appendCode() << "let " << freeMemoryPre << " := " << m_utils.allocateUnboundedFunction() << "()\n";
				IRVariable array = convert(*arguments[0], *TypeProvider::bytesMemory());
				IRVariable hashVariable(m_context.newYulVariable(), *TypeProvider::fixedBytes(32));

				define(hashVariable) <<
					"keccak256(" <<
					m_utils.arrayDataAreaFunction(*TypeProvider::bytesMemory()) <<
					"(" <<
					array.commaSeparatedList() <<
					"), " <<
					m_utils.arrayLengthFunction(*TypeProvider::bytesMemory()) <<
					"(" <<
					array.commaSeparatedList() <<
					"))\n";
				IRVariable selectorVariable(m_context.newYulVariable(), *TypeProvider::fixedBytes(4));
				define(selectorVariable, hashVariable);
				selector = selectorVariable.name();
				appendCode() << m_utils.finalizeAllocationFunction() << "(" << freeMemoryPre << ", 0)\n";
			}
		}
		else if (functionType->kind() == FunctionType::Kind::ABIEncodeWithSelector)
			selector = convert(*arguments.front(), *TypeProvider::fixedBytes(4)).name();

		Whiskers templ(R"(
			let <data> := <allocateUnbounded>()
			let <memPtr> := add(<data>, 0x20)
			<?+selector>
				mstore(<memPtr>, <selector>)
				<memPtr> := add(<memPtr>, 4)
			</+selector>
			let <mend> := <encode>(<memPtr><arguments>)
			mstore(<data>, sub(<mend>, add(<data>, 0x20)))
			<finalizeAllocation>(<data>, sub(<mend>, <data>))
		)");
		templ("data", IRVariable(_functionCall).part("mpos").name());
		templ("allocateUnbounded", m_utils.allocateUnboundedFunction());
		templ("memPtr", m_context.newYulVariable());
		templ("mend", m_context.newYulVariable());
		templ("selector", selector);
		templ("encode",
			isPacked ?
			m_context.abiFunctions().tupleEncoderPacked(argumentTypes, targetTypes) :
			m_context.abiFunctions().tupleEncoder(argumentTypes, targetTypes, false)
		);
		templ("arguments", joinHumanReadablePrefixed(argumentVars));
		templ("finalizeAllocation", m_utils.finalizeAllocationFunction());

		appendCode() << templ.render();
		break;
	}
	case FunctionType::Kind::ABIDecode:
	{
		Whiskers templ(R"(
			<?+retVars>let <retVars> := </+retVars> <abiDecode>(<offset>, add(<offset>, <length>))
		)");

		Type const* firstArgType = arguments.front()->annotation().type;
		TypePointers targetTypes;

		if (TupleType const* targetTupleType = dynamic_cast<TupleType const*>(_functionCall.annotation().type))
			targetTypes = targetTupleType->components();
		else
			targetTypes = TypePointers{_functionCall.annotation().type};

		if (
			auto referenceType = dynamic_cast<ReferenceType const*>(firstArgType);
			referenceType && referenceType->dataStoredIn(DataLocation::CallData)
			)
		{
			solAssert(referenceType->isImplicitlyConvertibleTo(*TypeProvider::bytesCalldata()), "");
			IRVariable var = convert(*arguments[0], *TypeProvider::bytesCalldata());
			templ("abiDecode", m_context.abiFunctions().tupleDecoder(targetTypes, false));
			templ("offset", var.part("offset").name());
			templ("length", var.part("length").name());
		}
		else
		{
			IRVariable var = convert(*arguments[0], *TypeProvider::bytesMemory());
			templ("abiDecode", m_context.abiFunctions().tupleDecoder(targetTypes, true));
			templ("offset", "add(" + var.part("mpos").name() + ", 32)");
			templ("length",
				m_utils.arrayLengthFunction(*TypeProvider::bytesMemory()) + "(" + var.part("mpos").name() + ")"
			);
		}
		templ("retVars", IRVariable(_functionCall).commaSeparatedList());

		appendCode() << templ.render();
		break;
	}
	case FunctionType::Kind::Revert:
	{
		solAssert(arguments.size() == parameterTypes.size(), "");
		solAssert(arguments.size() <= 1, "");
		solAssert(
			arguments.empty() ||
			arguments.front()->annotation().type->isImplicitlyConvertibleTo(*TypeProvider::stringMemory()),
		"");
		if (m_context.revertStrings() == RevertStrings::Strip || arguments.empty())
			appendCode() << "revert(0, 0)\n";
		else
			revertWithError(
				"Error(string)",
				{TypeProvider::stringMemory()},
				{arguments.front()}
			);
		break;
	}
	// Array creation using new
	case FunctionType::Kind::ObjectCreation:
	{
		ArrayType const& arrayType = dynamic_cast<ArrayType const&>(*_functionCall.annotation().type);
		solAssert(arguments.size() == 1, "");

		IRVariable value = convert(*arguments[0], *TypeProvider::uint256());
		define(_functionCall) <<
			m_utils.allocateAndInitializeMemoryArrayFunction(arrayType) <<
			"(" <<
			value.commaSeparatedList() <<
			")\n";
		break;
	}
	case FunctionType::Kind::KECCAK256:
	{
		solAssert(arguments.size() == 1, "");

		ArrayType const* arrayType = TypeProvider::bytesMemory();

		if (auto const* stringLiteral = dynamic_cast<StringLiteralType const*>(arguments.front()->annotation().type))
		{
			// Optimization: Compute keccak256 on string literals at compile-time.
			define(_functionCall) <<
				("0x" + keccak256(stringLiteral->value()).hex()) <<
				"\n";
		}
		else
		{
			auto array = convert(*arguments[0], *arrayType);

			define(_functionCall) <<
				"keccak256(" <<
				m_utils.arrayDataAreaFunction(*arrayType) <<
				"(" <<
				array.commaSeparatedList() <<
				"), " <<
				m_utils.arrayLengthFunction(*arrayType) <<
				"(" <<
				array.commaSeparatedList() <<
				"))\n";
		}
		break;
	}
	case FunctionType::Kind::ArrayPop:
	{
		solAssert(functionType->bound(), "");
		solAssert(functionType->parameterTypes().empty(), "");
		ArrayType const* arrayType = dynamic_cast<ArrayType const*>(functionType->selfType());
		solAssert(arrayType, "");
		define(_functionCall) <<
			m_utils.storageArrayPopFunction(*arrayType) <<
			"(" <<
			IRVariable(_functionCall.expression()).commaSeparatedList() <<
			")\n";
		break;
	}
	case FunctionType::Kind::ArrayPush:
	{
		ArrayType const* arrayType = dynamic_cast<ArrayType const*>(functionType->selfType());
		solAssert(arrayType, "");

		if (arguments.empty())
		{
			auto slotName = m_context.newYulVariable();
			auto offsetName = m_context.newYulVariable();
			appendCode() << "let " << slotName << ", " << offsetName << " := " <<
				m_utils.storageArrayPushZeroFunction(*arrayType) <<
				"(" << IRVariable(_functionCall.expression()).commaSeparatedList() << ")\n";
			setLValue(_functionCall, IRLValue{
				*arrayType->baseType(),
				IRLValue::Storage{
					slotName,
					offsetName,
				}
			});
		}
		else
		{
			IRVariable argument =
				arrayType->baseType()->isValueType() ?
				convert(*arguments.front(), *arrayType->baseType()) :
				*arguments.front();

			appendCode() <<
				m_utils.storageArrayPushFunction(*arrayType, &argument.type()) <<
				"(" <<
				IRVariable(_functionCall.expression()).commaSeparatedList() <<
				(argument.stackSlots().empty() ? "" : (", " + argument.commaSeparatedList()))  <<
				")\n";
		}
		break;
	}
	case FunctionType::Kind::BytesConcat:
	{
		TypePointers argumentTypes;
		vector<string> argumentVars;
		for (ASTPointer<Expression const> const& argument: arguments)
		{
			argumentTypes.emplace_back(&type(*argument));
			argumentVars += IRVariable(*argument).stackSlots();
		}
		define(IRVariable(_functionCall)) <<
			m_utils.bytesConcatFunction(argumentTypes) <<
			"(" <<
			joinHumanReadable(argumentVars) <<
			")\n";

		break;
	}
	case FunctionType::Kind::MetaType:
	{
		break;
	}
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
	{
		static map<FunctionType::Kind, string> functions = {
			{FunctionType::Kind::AddMod, "addmod"},
			{FunctionType::Kind::MulMod, "mulmod"},
		};
		solAssert(functions.find(functionType->kind()) != functions.end(), "");
		solAssert(arguments.size() == 3 && parameterTypes.size() == 3, "");

		IRVariable modulus(m_context.newYulVariable(), *(parameterTypes[2]));
		define(modulus, *arguments[2]);
		Whiskers templ("if iszero(<modulus>) { <panic>() }\n");
		templ("modulus", modulus.name());
		templ("panic", m_utils.panicFunction(PanicCode::DivisionByZero));
		appendCode() << templ.render();

		string args;
		for (size_t i = 0; i < 2; ++i)
			args += expressionAsType(*arguments[i], *(parameterTypes[i])) + ", ";
		args += modulus.name();
		define(_functionCall) << functions[functionType->kind()] << "(" << args << ")\n";
		break;
	}
	case FunctionType::Kind::GasLeft:
	case FunctionType::Kind::Selfdestruct:
	case FunctionType::Kind::BlockHash:
	{
		static map<FunctionType::Kind, string> functions = {
			{FunctionType::Kind::GasLeft, "gas"},
			{FunctionType::Kind::Selfdestruct, "selfdestruct"},
			{FunctionType::Kind::BlockHash, "blockhash"},
		};
		solAssert(functions.find(functionType->kind()) != functions.end(), "");

		string args;
		for (size_t i = 0; i < arguments.size(); ++i)
			args += (args.empty() ? "" : ", ") + expressionAsType(*arguments[i], *(parameterTypes[i]));
		define(_functionCall) << functions[functionType->kind()] << "(" << args << ")\n";
		break;
	}
	case FunctionType::Kind::Creation:
	{
		solAssert(!functionType->gasSet(), "Gas limit set for contract creation.");
		solAssert(
			functionType->returnParameterTypes().size() == 1,
			"Constructor should return only one type"
		);

		TypePointers argumentTypes;
		vector<string> constructorParams;
		for (ASTPointer<Expression const> const& arg: arguments)
		{
			argumentTypes.push_back(arg->annotation().type);
			constructorParams += IRVariable{*arg}.stackSlots();
		}

		ContractDefinition const* contract =
			&dynamic_cast<ContractType const&>(*functionType->returnParameterTypes().front()).contractDefinition();
		m_context.subObjectsCreated().insert(contract);

		Whiskers t(R"(let <memPos> := <allocateUnbounded>()
			let <memEnd> := add(<memPos>, datasize("<object>"))
			if or(gt(<memEnd>, 0xffffffffffffffff), lt(<memEnd>, <memPos>)) { <panic>() }
			datacopy(<memPos>, dataoffset("<object>"), datasize("<object>"))
			<memEnd> := <abiEncode>(<memEnd><constructorParams>)
			<?saltSet>
				let <address> := create2(<value>, <memPos>, sub(<memEnd>, <memPos>), <salt>)
			<!saltSet>
				let <address> := create(<value>, <memPos>, sub(<memEnd>, <memPos>))
			</saltSet>
			<?isTryCall>
				let <success> := iszero(iszero(<address>))
			<!isTryCall>
				if iszero(<address>) { <forwardingRevert>() }
			</isTryCall>
		)");
		t("memPos", m_context.newYulVariable());
		t("memEnd", m_context.newYulVariable());
		t("allocateUnbounded", m_utils.allocateUnboundedFunction());
		t("object", IRNames::creationObject(*contract));
		t("panic", m_utils.panicFunction(PanicCode::ResourceError));
		t("abiEncode",
			m_context.abiFunctions().tupleEncoder(argumentTypes, functionType->parameterTypes(), false)
		);
		t("constructorParams", joinHumanReadablePrefixed(constructorParams));
		t("value", functionType->valueSet() ? IRVariable(_functionCall.expression()).part("value").name() : "0");
		t("saltSet", functionType->saltSet());
		if (functionType->saltSet())
			t("salt", IRVariable(_functionCall.expression()).part("salt").name());
		solAssert(IRVariable(_functionCall).stackSlots().size() == 1, "");
		t("address", IRVariable(_functionCall).commaSeparatedList());
		t("isTryCall", _functionCall.annotation().tryCall);
		if (_functionCall.annotation().tryCall)
			t("success", IRNames::trySuccessConditionVariable(_functionCall));
		else
			t("forwardingRevert", m_utils.forwardingRevertFunction());
		appendCode() << t.render();

		break;
	}
	case FunctionType::Kind::Send:
	case FunctionType::Kind::Transfer:
	{
		solAssert(arguments.size() == 1 && parameterTypes.size() == 1, "");
		string address{IRVariable(_functionCall.expression()).part("address").name()};
		string value{expressionAsType(*arguments[0], *(parameterTypes[0]))};
		Whiskers templ(R"(
			let <gas> := 0
			if iszero(<value>) { <gas> := <callStipend> }
			let <success> := call(<gas>, <address>, <value>, 0, 0, 0, 0)
			<?isTransfer>
				if iszero(<success>) { <forwardingRevert>() }
			</isTransfer>
		)");
		templ("gas", m_context.newYulVariable());
		templ("callStipend", toString(evmasm::GasCosts::callStipend));
		templ("address", address);
		templ("value", value);
		if (functionType->kind() == FunctionType::Kind::Transfer)
			templ("success", m_context.newYulVariable());
		else
			templ("success", IRVariable(_functionCall).commaSeparatedList());
		templ("isTransfer", functionType->kind() == FunctionType::Kind::Transfer);
		templ("forwardingRevert", m_utils.forwardingRevertFunction());
		appendCode() << templ.render();

		break;
	}
	case FunctionType::Kind::ECRecover:
	case FunctionType::Kind::RIPEMD160:
	case FunctionType::Kind::SHA256:
	{
		solAssert(!_functionCall.annotation().tryCall, "");
		solAssert(!functionType->valueSet(), "");
		solAssert(!functionType->gasSet(), "");
		solAssert(!functionType->bound(), "");

		static map<FunctionType::Kind, std::tuple<unsigned, size_t>> precompiles = {
			{FunctionType::Kind::ECRecover, std::make_tuple(1, 0)},
			{FunctionType::Kind::SHA256, std::make_tuple(2, 0)},
			{FunctionType::Kind::RIPEMD160, std::make_tuple(3, 12)},
		};
		auto [ address, offset ] = precompiles[functionType->kind()];
		TypePointers argumentTypes;
		vector<string> argumentStrings;
		for (auto const& arg: arguments)
		{
			argumentTypes.emplace_back(&type(*arg));
			argumentStrings += IRVariable(*arg).stackSlots();
		}
		Whiskers templ(R"(
			let <pos> := <allocateUnbounded>()
			let <end> := <encodeArgs>(<pos> <argumentString>)
			<?isECRecover>
				mstore(0, 0)
			</isECRecover>
			let <success> := <call>(<gas>, <address> <?isCall>, 0</isCall>, <pos>, sub(<end>, <pos>), 0, 32)
			if iszero(<success>) { <forwardingRevert>() }
			let <retVars> := <shl>(mload(0))
		)");
		templ("call", m_context.evmVersion().hasStaticCall() ? "staticcall" : "call");
		templ("isCall", !m_context.evmVersion().hasStaticCall());
		templ("shl", m_utils.shiftLeftFunction(offset * 8));
		templ("allocateUnbounded", m_utils.allocateUnboundedFunction());
		templ("pos", m_context.newYulVariable());
		templ("end", m_context.newYulVariable());
		templ("isECRecover", FunctionType::Kind::ECRecover == functionType->kind());
		if (FunctionType::Kind::ECRecover == functionType->kind())
			templ("encodeArgs", m_context.abiFunctions().tupleEncoder(argumentTypes, parameterTypes));
		else
			templ("encodeArgs", m_context.abiFunctions().tupleEncoderPacked(argumentTypes, parameterTypes));
		templ("argumentString", joinHumanReadablePrefixed(argumentStrings));
		templ("address", toString(address));
		templ("success", m_context.newYulVariable());
		templ("retVars", IRVariable(_functionCall).commaSeparatedList());
		templ("forwardingRevert", m_utils.forwardingRevertFunction());
		if (m_context.evmVersion().canOverchargeGasForCall())
			// Send all gas (requires tangerine whistle EVM)
			templ("gas", "gas()");
		else
		{
			// @todo The value 10 is not exact and this could be fine-tuned,
			// but this has worked for years in the old code generator.
			u256 gasNeededByCaller = evmasm::GasCosts::callGas(m_context.evmVersion()) + 10 + evmasm::GasCosts::callNewAccountGas;
			templ("gas", "sub(gas(), " + formatNumber(gasNeededByCaller) + ")");
		}

		appendCode() << templ.render();

		break;
	}
	default:
		solUnimplemented("FunctionKind " + toString(static_cast<int>(functionType->kind())) + " not yet implemented");
	}
}

void IRGeneratorForStatements::endVisit(FunctionCallOptions const& _options)
{
	setLocation(_options);
	FunctionType const& previousType = dynamic_cast<FunctionType const&>(*_options.expression().annotation().type);

	solUnimplementedAssert(!previousType.bound(), "");

	// Copy over existing values.
	for (auto const& item: previousType.stackItems())
		define(IRVariable(_options).part(get<0>(item)), IRVariable(_options.expression()).part(get<0>(item)));

	for (size_t i = 0; i < _options.names().size(); ++i)
	{
		string const& name = *_options.names()[i];
		solAssert(name == "salt" || name == "gas" || name == "value", "");

		define(IRVariable(_options).part(name), *_options.options()[i]);
	}
}

bool IRGeneratorForStatements::visit(MemberAccess const& _memberAccess)
{
	// A shortcut for <address>.code.length. We skip visiting <address>.code and directly visit
	// <address>. The actual code is generated in endVisit.
	if (
		auto innerExpression = dynamic_cast<MemberAccess const*>(&_memberAccess.expression());
		_memberAccess.memberName() == "length" &&
		innerExpression &&
		innerExpression->memberName() == "code" &&
		innerExpression->expression().annotation().type->category() == Type::Category::Address
	)
	{
		solAssert(innerExpression->annotation().type->category() == Type::Category::Array, "");
		// Skip visiting <address>.code
		innerExpression->expression().accept(*this);

		return false;
	}

	return true;
}

void IRGeneratorForStatements::endVisit(MemberAccess const& _memberAccess)
{
	setLocation(_memberAccess);

	ASTString const& member = _memberAccess.memberName();
	auto memberFunctionType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type);
	Type::Category objectCategory = _memberAccess.expression().annotation().type->category();

	if (memberFunctionType && memberFunctionType->bound())
	{
		define(IRVariable(_memberAccess).part("self"), _memberAccess.expression());
		solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");
		if (memberFunctionType->kind() == FunctionType::Kind::Internal)
			assignInternalFunctionIDIfNotCalledDirectly(
				_memberAccess,
				dynamic_cast<FunctionDefinition const&>(memberFunctionType->declaration())
			);
		else if (
			memberFunctionType->kind() == FunctionType::Kind::ArrayPush ||
			memberFunctionType->kind() == FunctionType::Kind::ArrayPop
		)
		{
			// Nothing to do.
		}
		else
		{
			auto const& functionDefinition = dynamic_cast<FunctionDefinition const&>(memberFunctionType->declaration());
			solAssert(memberFunctionType->kind() == FunctionType::Kind::DelegateCall, "");
			auto contract = dynamic_cast<ContractDefinition const*>(functionDefinition.scope());
			solAssert(contract && contract->isLibrary(), "");
			define(IRVariable(_memberAccess).part("address")) << linkerSymbol(*contract) << "\n";
			define(IRVariable(_memberAccess).part("functionSelector")) << memberFunctionType->externalIdentifier() << "\n";
		}
		return;
	}

	switch (objectCategory)
	{
	case Type::Category::Contract:
	{
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.expression().annotation().type);
		if (type.isSuper())
			solAssert(false, "");

		// ordinary contract type
		else if (Declaration const* declaration = _memberAccess.annotation().referencedDeclaration)
		{
			u256 identifier;
			if (auto const* variable = dynamic_cast<VariableDeclaration const*>(declaration))
				identifier = FunctionType(*variable).externalIdentifier();
			else if (auto const* function = dynamic_cast<FunctionDefinition const*>(declaration))
				identifier = FunctionType(*function).externalIdentifier();
			else
				solAssert(false, "Contract member is neither variable nor function.");

			define(IRVariable(_memberAccess).part("address"), _memberAccess.expression());
			define(IRVariable(_memberAccess).part("functionSelector")) << formatNumber(identifier) << "\n";
		}
		else
			solAssert(false, "Invalid member access in contract");
		break;
	}
	case Type::Category::Integer:
	{
		solAssert(false, "Invalid member access to integer");
		break;
	}
	case Type::Category::Address:
	{
		if (member == "balance")
			define(_memberAccess) <<
				"balance(" <<
				expressionAsType(_memberAccess.expression(), *TypeProvider::address()) <<
				")\n";
		else if (member == "code")
			define(_memberAccess) <<
				m_utils.externalCodeFunction() <<
				"(" <<
				expressionAsType(_memberAccess.expression(), *TypeProvider::address()) <<
				")\n";
		else if (member == "codehash")
			define(_memberAccess) <<
				"extcodehash(" <<
				expressionAsType(_memberAccess.expression(), *TypeProvider::address()) <<
				")\n";
		else if (set<string>{"send", "transfer"}.count(member))
		{
			solAssert(dynamic_cast<AddressType const&>(*_memberAccess.expression().annotation().type).stateMutability() == StateMutability::Payable, "");
			define(IRVariable{_memberAccess}.part("address"), _memberAccess.expression());
		}
		else if (set<string>{"call", "callcode", "delegatecall", "staticcall"}.count(member))
			define(IRVariable{_memberAccess}.part("address"), _memberAccess.expression());
		else
			solAssert(false, "Invalid member access to address");
		break;
	}
	case Type::Category::Function:
		if (member == "selector")
		{
			FunctionType const& functionType = dynamic_cast<FunctionType const&>(
				*_memberAccess.expression().annotation().type
			);
			if (
				functionType.kind() == FunctionType::Kind::External ||
				functionType.kind() == FunctionType::Kind::DelegateCall
			)
				define(IRVariable{_memberAccess}, IRVariable(_memberAccess.expression()).part("functionSelector"));
			else if (
				functionType.kind() == FunctionType::Kind::Declaration ||
				functionType.kind() == FunctionType::Kind::Error ||
				// In some situations, internal function types also provide the "selector" member.
				// See Types.cpp for details.
				functionType.kind() == FunctionType::Kind::Internal
			)
			{
				solAssert(functionType.hasDeclaration(), "");
				solAssert(
					functionType.kind() == FunctionType::Kind::Error ||
					functionType.declaration().isPartOfExternalInterface(),
					""
				);
				define(IRVariable{_memberAccess}) << formatNumber(functionType.externalIdentifier() << 224) << "\n";
			}
			else
				solAssert(false, "Invalid use of .selector: " + functionType.toString(false));
		}
		else if (member == "address")
		{
			solUnimplementedAssert(
				dynamic_cast<FunctionType const&>(*_memberAccess.expression().annotation().type).kind() ==
				FunctionType::Kind::External, ""
			);
			define(IRVariable{_memberAccess}, IRVariable(_memberAccess.expression()).part("address"));
		}
		else
			solAssert(
				!!_memberAccess.expression().annotation().type->memberType(member),
				"Invalid member access to function."
			);
		break;
	case Type::Category::Magic:
		// we can ignore the kind of magic and only look at the name of the member
		if (member == "coinbase")
			define(_memberAccess) << "coinbase()\n";
		else if (member == "timestamp")
			define(_memberAccess) << "timestamp()\n";
		else if (member == "difficulty")
			define(_memberAccess) << "difficulty()\n";
		else if (member == "number")
			define(_memberAccess) << "number()\n";
		else if (member == "gaslimit")
			define(_memberAccess) << "gaslimit()\n";
		else if (member == "sender")
			define(_memberAccess) << "caller()\n";
		else if (member == "value")
			define(_memberAccess) << "callvalue()\n";
		else if (member == "origin")
			define(_memberAccess) << "origin()\n";
		else if (member == "gasprice")
			define(_memberAccess) << "gasprice()\n";
		else if (member == "chainid")
			define(_memberAccess) << "chainid()\n";
		else if (member == "basefee")
			define(_memberAccess) << "basefee()\n";
		else if (member == "data")
		{
			IRVariable var(_memberAccess);
			define(var.part("offset")) << "0\n";
			define(var.part("length")) << "calldatasize()\n";
		}
		else if (member == "sig")
			define(_memberAccess) <<
				"and(calldataload(0), " <<
				formatNumber(u256(0xffffffff) << (256 - 32)) <<
				")\n";
		else if (member == "gas")
			solAssert(false, "Gas has been removed.");
		else if (member == "blockhash")
			solAssert(false, "Blockhash has been removed.");
		else if (member == "creationCode" || member == "runtimeCode")
		{
			Type const* arg = dynamic_cast<MagicType const&>(*_memberAccess.expression().annotation().type).typeArgument();
			auto const& contractType = dynamic_cast<ContractType const&>(*arg);
			solAssert(!contractType.isSuper(), "");
			ContractDefinition const& contract = contractType.contractDefinition();
			m_context.subObjectsCreated().insert(&contract);
			appendCode() << Whiskers(R"(
				let <size> := datasize("<objectName>")
				let <result> := <allocationFunction>(add(<size>, 32))
				mstore(<result>, <size>)
				datacopy(add(<result>, 32), dataoffset("<objectName>"), <size>)
			)")
			("allocationFunction", m_utils.allocationFunction())
			("size", m_context.newYulVariable())
			("objectName", IRNames::creationObject(contract) + (member == "runtimeCode" ? "." + IRNames::deployedObject(contract) : ""))
			("result", IRVariable(_memberAccess).commaSeparatedList()).render();
		}
		else if (member == "name")
		{
			Type const* arg = dynamic_cast<MagicType const&>(*_memberAccess.expression().annotation().type).typeArgument();
			ContractDefinition const& contract = dynamic_cast<ContractType const&>(*arg).contractDefinition();
			define(IRVariable(_memberAccess)) << m_utils.copyLiteralToMemoryFunction(contract.name()) << "()\n";
		}
		else if (member == "interfaceId")
		{
			Type const* arg = dynamic_cast<MagicType const&>(*_memberAccess.expression().annotation().type).typeArgument();
			auto const& contractType = dynamic_cast<ContractType const&>(*arg);
			solAssert(!contractType.isSuper(), "");
			ContractDefinition const& contract = contractType.contractDefinition();
			define(_memberAccess) << formatNumber(u256{contract.interfaceId()} << (256 - 32)) << "\n";
		}
		else if (member == "min" || member == "max")
		{
			MagicType const* arg = dynamic_cast<MagicType const*>(_memberAccess.expression().annotation().type);
			IntegerType const* integerType = dynamic_cast<IntegerType const*>(arg->typeArgument());

			if (member == "min")
				define(_memberAccess) << formatNumber(integerType->min()) << "\n";
			else
				define(_memberAccess) << formatNumber(integerType->max()) << "\n";
		}
		else if (set<string>{"encode", "encodePacked", "encodeWithSelector", "encodeWithSignature", "decode"}.count(member))
		{
			// no-op
		}
		else
			solAssert(false, "Unknown magic member.");
		break;
	case Type::Category::Struct:
	{
		auto const& structType = dynamic_cast<StructType const&>(*_memberAccess.expression().annotation().type);

		IRVariable expression(_memberAccess.expression());
		switch (structType.location())
		{
		case DataLocation::Storage:
		{
			pair<u256, unsigned> const& offsets = structType.storageOffsetsOfMember(member);
			string slot = m_context.newYulVariable();
			appendCode() << "let " << slot << " := " <<
				("add(" + expression.part("slot").name() + ", " + offsets.first.str() + ")\n");
			setLValue(_memberAccess, IRLValue{
				type(_memberAccess),
				IRLValue::Storage{slot, offsets.second}
			});
			break;
		}
		case DataLocation::Memory:
		{
			string pos = m_context.newYulVariable();
			appendCode() << "let " << pos << " := " <<
				("add(" + expression.part("mpos").name() + ", " + structType.memoryOffsetOfMember(member).str() + ")\n");
			setLValue(_memberAccess, IRLValue{
				type(_memberAccess),
				IRLValue::Memory{pos}
			});
			break;
		}
		case DataLocation::CallData:
		{
			string baseRef = expression.part("offset").name();
			string offset = m_context.newYulVariable();
			appendCode() << "let " << offset << " := " << "add(" << baseRef << ", " << to_string(structType.calldataOffsetOfMember(member)) << ")\n";
			if (_memberAccess.annotation().type->isDynamicallyEncoded())
				define(_memberAccess) <<
					m_utils.accessCalldataTailFunction(*_memberAccess.annotation().type) <<
					"(" <<
					baseRef <<
					", " <<
					offset <<
					")\n";
			else if (
				dynamic_cast<ArrayType const*>(_memberAccess.annotation().type) ||
				dynamic_cast<StructType const*>(_memberAccess.annotation().type)
			)
				define(_memberAccess) << offset << "\n";
			else
				define(_memberAccess) <<
					m_utils.readFromCalldata(*_memberAccess.annotation().type) <<
					"(" <<
					offset <<
					")\n";
			break;
		}
		default:
			solAssert(false, "Illegal data location for struct.");
		}
		break;
	}
	case Type::Category::Enum:
	{
		EnumType const& type = dynamic_cast<EnumType const&>(*_memberAccess.expression().annotation().type);
		define(_memberAccess) << to_string(type.memberValue(_memberAccess.memberName())) << "\n";
		break;
	}
	case Type::Category::Array:
	{
		auto const& type = dynamic_cast<ArrayType const&>(*_memberAccess.expression().annotation().type);
		if (member == "length")
		{
			// shortcut for <address>.code.length
			if (
				auto innerExpression = dynamic_cast<MemberAccess const*>(&_memberAccess.expression());
				innerExpression &&
				innerExpression->memberName() == "code" &&
				innerExpression->expression().annotation().type->category() == Type::Category::Address
			)
				define(_memberAccess) <<
					"extcodesize(" <<
					expressionAsType(innerExpression->expression(), *TypeProvider::address()) <<
					")\n";
			else
				define(_memberAccess) <<
					m_utils.arrayLengthFunction(type) <<
					"(" <<
					IRVariable(_memberAccess.expression()).commaSeparatedList() <<
					")\n";
		}
		else if (member == "pop" || member == "push")
		{
			solAssert(type.location() == DataLocation::Storage, "");
			define(IRVariable{_memberAccess}.part("slot"), IRVariable{_memberAccess.expression()}.part("slot"));
		}
		else
			solAssert(false, "Invalid array member access.");

		break;
	}
	case Type::Category::FixedBytes:
	{
		auto const& type = dynamic_cast<FixedBytesType const&>(*_memberAccess.expression().annotation().type);
		if (member == "length")
			define(_memberAccess) << to_string(type.numBytes()) << "\n";
		else
			solAssert(false, "Illegal fixed bytes member.");
		break;
	}
	case Type::Category::TypeType:
	{
		Type const& actualType = *dynamic_cast<TypeType const&>(
			*_memberAccess.expression().annotation().type
		).actualType();

		if (actualType.category() == Type::Category::Contract)
		{
			ContractType const& contractType = dynamic_cast<ContractType const&>(actualType);
			if (contractType.isSuper())
			{
				solAssert(!!_memberAccess.annotation().referencedDeclaration, "Referenced declaration not resolved.");
				ContractDefinition const* super = contractType.contractDefinition().superContract(m_context.mostDerivedContract());
				solAssert(super, "Super contract not available.");
				FunctionDefinition const& resolvedFunctionDef =
					dynamic_cast<FunctionDefinition const&>(
						*_memberAccess.annotation().referencedDeclaration
					).resolveVirtual(m_context.mostDerivedContract(), super);

				solAssert(resolvedFunctionDef.functionType(true), "");
				solAssert(resolvedFunctionDef.functionType(true)->kind() == FunctionType::Kind::Internal, "");
				assignInternalFunctionIDIfNotCalledDirectly(_memberAccess, resolvedFunctionDef);
			}
			else if (auto const* variable = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
					handleVariableReference(*variable, _memberAccess);
			else if (memberFunctionType)
			{
				switch (memberFunctionType->kind())
				{
				case FunctionType::Kind::Declaration:
					break;
				case FunctionType::Kind::Internal:
					if (auto const* function = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration))
						assignInternalFunctionIDIfNotCalledDirectly(_memberAccess, *function);
					else
						solAssert(false, "Function not found in member access");
					break;
				case FunctionType::Kind::Event:
					solAssert(
						dynamic_cast<EventDefinition const*>(_memberAccess.annotation().referencedDeclaration),
						"Event not found"
					);
						// the call will do the resolving
					break;
				case FunctionType::Kind::Error:
					solAssert(
						dynamic_cast<ErrorDefinition const*>(_memberAccess.annotation().referencedDeclaration),
						"Error not found"
					);
					// The function call will resolve the selector.
					break;
				case FunctionType::Kind::DelegateCall:
					define(IRVariable(_memberAccess).part("address"), _memberAccess.expression());
					define(IRVariable(_memberAccess).part("functionSelector")) << formatNumber(memberFunctionType->externalIdentifier()) << "\n";
					break;
				case FunctionType::Kind::External:
				case FunctionType::Kind::Creation:
				case FunctionType::Kind::Send:
				case FunctionType::Kind::BareCall:
				case FunctionType::Kind::BareCallCode:
				case FunctionType::Kind::BareDelegateCall:
				case FunctionType::Kind::BareStaticCall:
				case FunctionType::Kind::Transfer:
				case FunctionType::Kind::ECRecover:
				case FunctionType::Kind::SHA256:
				case FunctionType::Kind::RIPEMD160:
				default:
					solAssert(false, "unsupported member function");
				}
			}
			else if (dynamic_cast<TypeType const*>(_memberAccess.annotation().type))
			{
			// no-op
			}
			else
				// The old code generator had a generic "else" case here
				// without any specific code being generated,
				// but it would still be better to have an exhaustive list.
				solAssert(false, "");
		}
		else if (EnumType const* enumType = dynamic_cast<EnumType const*>(&actualType))
			define(_memberAccess) << to_string(enumType->memberValue(_memberAccess.memberName())) << "\n";
		else if (auto const* arrayType = dynamic_cast<ArrayType const*>(&actualType))
			solAssert(arrayType->isByteArray() && member == "concat", "");
		else
			// The old code generator had a generic "else" case here
			// without any specific code being generated,
			// but it would still be better to have an exhaustive list.
			solAssert(false, "");
		break;
	}
	case Type::Category::Module:
	{
		Type::Category category = _memberAccess.annotation().type->category();
		solAssert(
			dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration) ||
			dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration) ||
			dynamic_cast<ErrorDefinition const*>(_memberAccess.annotation().referencedDeclaration) ||
			category == Type::Category::TypeType ||
			category == Type::Category::Module,
			""
		);
		if (auto variable = dynamic_cast<VariableDeclaration const*>(_memberAccess.annotation().referencedDeclaration))
		{
			solAssert(variable->isConstant(), "");
			handleVariableReference(*variable, static_cast<Expression const&>(_memberAccess));
		}
		else if (auto const* function = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration))
		{
			auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type);
			solAssert(function && function->isFree(), "");
			solAssert(function->functionType(true), "");
			solAssert(function->functionType(true)->kind() == FunctionType::Kind::Internal, "");
			solAssert(funType->kind() == FunctionType::Kind::Internal, "");
			solAssert(*_memberAccess.annotation().requiredLookup == VirtualLookup::Static, "");

			assignInternalFunctionIDIfNotCalledDirectly(_memberAccess, *function);
		}
		else if (auto const* contract = dynamic_cast<ContractDefinition const*>(_memberAccess.annotation().referencedDeclaration))
		{
			if (contract->isLibrary())
				define(IRVariable(_memberAccess).part("address")) << linkerSymbol(*contract) << "\n";
		}
		break;
	}
	default:
		solAssert(false, "Member access to unknown type.");
	}
}

bool IRGeneratorForStatements::visit(InlineAssembly const& _inlineAsm)
{
	setLocation(_inlineAsm);
	m_context.setInlineAssemblySeen();
	CopyTranslate bodyCopier{_inlineAsm.dialect(), m_context, _inlineAsm.annotation().externalReferences};

	yul::Statement modified = bodyCopier(_inlineAsm.operations());

	solAssert(holds_alternative<yul::Block>(modified), "");

	// Do not provide dialect so that we get the full type information.
	appendCode() << yul::AsmPrinter()(std::get<yul::Block>(modified)) << "\n";
	return false;
}


void IRGeneratorForStatements::endVisit(IndexAccess const& _indexAccess)
{
	setLocation(_indexAccess);
	Type const& baseType = *_indexAccess.baseExpression().annotation().type;

	if (baseType.category() == Type::Category::Mapping)
	{
		solAssert(_indexAccess.indexExpression(), "Index expression expected.");

		MappingType const& mappingType = dynamic_cast<MappingType const&>(baseType);
		Type const& keyType = *_indexAccess.indexExpression()->annotation().type;

		string slot = m_context.newYulVariable();
		Whiskers templ("let <slot> := <indexAccess>(<base><?+key>,<key></+key>)\n");
		templ("slot", slot);
		templ("indexAccess", m_utils.mappingIndexAccessFunction(mappingType, keyType));
		templ("base", IRVariable(_indexAccess.baseExpression()).commaSeparatedList());
		templ("key", IRVariable(*_indexAccess.indexExpression()).commaSeparatedList());
		appendCode() << templ.render();
		setLValue(_indexAccess, IRLValue{
			*_indexAccess.annotation().type,
			IRLValue::Storage{
				slot,
				0u
			}
		});
	}
	else if (baseType.category() == Type::Category::Array || baseType.category() == Type::Category::ArraySlice)
	{
		ArrayType const& arrayType =
			baseType.category() == Type::Category::Array ?
			dynamic_cast<ArrayType const&>(baseType) :
			dynamic_cast<ArraySliceType const&>(baseType).arrayType();

		if (baseType.category() == Type::Category::ArraySlice)
			solAssert(arrayType.dataStoredIn(DataLocation::CallData) && arrayType.isDynamicallySized(), "");

		solAssert(_indexAccess.indexExpression(), "Index expression expected.");

		switch (arrayType.location())
		{
			case DataLocation::Storage:
			{
				string slot = m_context.newYulVariable();
				string offset = m_context.newYulVariable();

				appendCode() << Whiskers(R"(
					let <slot>, <offset> := <indexFunc>(<array>, <index>)
				)")
				("slot", slot)
				("offset", offset)
				("indexFunc", m_utils.storageArrayIndexAccessFunction(arrayType))
				("array", IRVariable(_indexAccess.baseExpression()).part("slot").name())
				("index", IRVariable(*_indexAccess.indexExpression()).name())
				.render();

				setLValue(_indexAccess, IRLValue{
					*_indexAccess.annotation().type,
					IRLValue::Storage{slot, offset}
				});

				break;
			}
			case DataLocation::Memory:
			{
				string const memAddress =
					m_utils.memoryArrayIndexAccessFunction(arrayType) +
					"(" +
					IRVariable(_indexAccess.baseExpression()).part("mpos").name() +
					", " +
					expressionAsType(*_indexAccess.indexExpression(), *TypeProvider::uint256()) +
					")";

				setLValue(_indexAccess, IRLValue{
					*arrayType.baseType(),
					IRLValue::Memory{memAddress, arrayType.isByteArray()}
				});
				break;
			}
			case DataLocation::CallData:
			{
				string const indexAccessFunctionCall =
					m_utils.calldataArrayIndexAccessFunction(arrayType) +
					"(" +
					IRVariable(_indexAccess.baseExpression()).commaSeparatedList() +
					", " +
					expressionAsType(*_indexAccess.indexExpression(), *TypeProvider::uint256()) +
					")";
				if (arrayType.isByteArray())
					define(_indexAccess) <<
						m_utils.cleanupFunction(*arrayType.baseType()) <<
						"(calldataload(" <<
						indexAccessFunctionCall <<
						"))\n";
				else if (arrayType.baseType()->isValueType())
					define(_indexAccess) <<
						m_utils.readFromCalldata(*arrayType.baseType()) <<
						"(" <<
						indexAccessFunctionCall <<
						")\n";
				else
					define(_indexAccess) << indexAccessFunctionCall << "\n";
				break;
			}
		}
	}
	else if (baseType.category() == Type::Category::FixedBytes)
	{
		auto const& fixedBytesType = dynamic_cast<FixedBytesType const&>(baseType);
		solAssert(_indexAccess.indexExpression(), "Index expression expected.");

		IRVariable index{m_context.newYulVariable(), *TypeProvider::uint256()};
		define(index, *_indexAccess.indexExpression());
		appendCode() << Whiskers(R"(
			if iszero(lt(<index>, <length>)) { <panic>() }
			let <result> := <shl248>(byte(<index>, <array>))
		)")
		("index", index.name())
		("length", to_string(fixedBytesType.numBytes()))
		("panic", m_utils.panicFunction(PanicCode::ArrayOutOfBounds))
		("array", IRVariable(_indexAccess.baseExpression()).name())
		("shl248", m_utils.shiftLeftFunction(256 - 8))
		("result", IRVariable(_indexAccess).name())
		.render();
	}
	else if (baseType.category() == Type::Category::TypeType)
	{
		solAssert(baseType.sizeOnStack() == 0, "");
		solAssert(_indexAccess.annotation().type->sizeOnStack() == 0, "");
		// no-op - this seems to be a lone array type (`structType[];`)
	}
	else
		solAssert(false, "Index access only allowed for mappings or arrays.");
}

void IRGeneratorForStatements::endVisit(IndexRangeAccess const& _indexRangeAccess)
{
	setLocation(_indexRangeAccess);
	Type const& baseType = *_indexRangeAccess.baseExpression().annotation().type;
	solAssert(
		baseType.category() == Type::Category::Array || baseType.category() == Type::Category::ArraySlice,
		"Index range accesses is available only on arrays and array slices."
	);

	ArrayType const& arrayType =
		baseType.category() == Type::Category::Array ?
		dynamic_cast<ArrayType const &>(baseType) :
		dynamic_cast<ArraySliceType const &>(baseType).arrayType();

	switch (arrayType.location())
	{
		case DataLocation::CallData:
		{
			solAssert(baseType.isDynamicallySized(), "");
			IRVariable sliceStart{m_context.newYulVariable(), *TypeProvider::uint256()};
			if (_indexRangeAccess.startExpression())
				define(sliceStart, IRVariable{*_indexRangeAccess.startExpression()});
			else
				define(sliceStart) << u256(0) << "\n";

			IRVariable sliceEnd{
				m_context.newYulVariable(),
				*TypeProvider::uint256()
			};
			if (_indexRangeAccess.endExpression())
				define(sliceEnd, IRVariable{*_indexRangeAccess.endExpression()});
			else
				define(sliceEnd, IRVariable{_indexRangeAccess.baseExpression()}.part("length"));

			IRVariable range{_indexRangeAccess};
			define(range) <<
				m_utils.calldataArrayIndexRangeAccess(arrayType) << "(" <<
				IRVariable{_indexRangeAccess.baseExpression()}.commaSeparatedList() << ", " <<
				sliceStart.name() << ", " <<
				sliceEnd.name() << ")\n";
			break;
		}
		default:
			solUnimplementedAssert(false, "Index range accesses is implemented only on calldata arrays.");
	}
}

void IRGeneratorForStatements::endVisit(Identifier const& _identifier)
{
	setLocation(_identifier);
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		switch (magicVar->type()->category())
		{
		case Type::Category::Contract:
			solAssert(_identifier.name() == "this", "");
			define(_identifier) << "address()\n";
			break;
		case Type::Category::Integer:
			solAssert(_identifier.name() == "now", "");
			define(_identifier) << "timestamp()\n";
			break;
		case Type::Category::TypeType:
		{
			auto typeType = dynamic_cast<TypeType const*>(magicVar->type());
			if (auto contractType = dynamic_cast<ContractType const*>(typeType->actualType()))
				solAssert(!contractType->isSuper() || _identifier.name() == "super", "");
			break;
		}
		default:
			break;
		}
		return;
	}
	else if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
	{
		solAssert(*_identifier.annotation().requiredLookup == VirtualLookup::Virtual, "");
		FunctionDefinition const& resolvedFunctionDef = functionDef->resolveVirtual(m_context.mostDerivedContract());

		solAssert(resolvedFunctionDef.functionType(true), "");
		solAssert(resolvedFunctionDef.functionType(true)->kind() == FunctionType::Kind::Internal, "");
		assignInternalFunctionIDIfNotCalledDirectly(_identifier, resolvedFunctionDef);
	}
	else if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
		handleVariableReference(*varDecl, _identifier);
	else if (auto const* contract = dynamic_cast<ContractDefinition const*>(declaration))
	{
		if (contract->isLibrary())
			define(IRVariable(_identifier).part("address")) << linkerSymbol(*contract) << "\n";
	}
	else if (dynamic_cast<EventDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<ErrorDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<EnumDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<StructDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<ImportDirective const*>(declaration))
	{
		// no-op
	}
	else
	{
		solAssert(false, "Identifier type not expected in expression context.");
	}
}

bool IRGeneratorForStatements::visit(Literal const& _literal)
{
	setLocation(_literal);
	Type const& literalType = type(_literal);

	switch (literalType.category())
	{
	case Type::Category::RationalNumber:
	case Type::Category::Bool:
	case Type::Category::Address:
		define(_literal) << toCompactHexWithPrefix(literalType.literalValue(&_literal)) << "\n";
		break;
	case Type::Category::StringLiteral:
		break; // will be done during conversion
	default:
		solUnimplemented("Only integer, boolean and string literals implemented for now.");
	}
	return false;
}

void IRGeneratorForStatements::handleVariableReference(
	VariableDeclaration const& _variable,
	Expression const& _referencingExpression
)
{
	if ((_variable.isStateVariable() || _variable.isFileLevelVariable()) && _variable.isConstant())
		define(_referencingExpression) << constantValueFunction(_variable) << "()\n";
	else if (_variable.isStateVariable() && _variable.immutable())
		setLValue(_referencingExpression, IRLValue{
			*_variable.annotation().type,
			IRLValue::Immutable{&_variable}
		});
	else if (m_context.isLocalVariable(_variable))
		setLValue(_referencingExpression, IRLValue{
			*_variable.annotation().type,
			IRLValue::Stack{m_context.localVariable(_variable)}
		});
	else if (m_context.isStateVariable(_variable))
		setLValue(_referencingExpression, IRLValue{
			*_variable.annotation().type,
			IRLValue::Storage{
				toCompactHexWithPrefix(m_context.storageLocationOfStateVariable(_variable).first),
				m_context.storageLocationOfStateVariable(_variable).second
			}
		});
	else
		solAssert(false, "Invalid variable kind.");
}

void IRGeneratorForStatements::appendExternalFunctionCall(
	FunctionCall const& _functionCall,
	vector<ASTPointer<Expression const>> const& _arguments
)
{
	FunctionType const& funType = dynamic_cast<FunctionType const&>(type(_functionCall.expression()));
	solAssert(!funType.takesArbitraryParameters(), "");
	solAssert(_arguments.size() == funType.parameterTypes().size(), "");
	solAssert(!funType.isBareCall(), "");
	FunctionType::Kind const funKind = funType.kind();

	solAssert(
		funKind == FunctionType::Kind::External || funKind == FunctionType::Kind::DelegateCall,
		"Can only be used for regular external calls."
	);

	bool const isDelegateCall = funKind == FunctionType::Kind::DelegateCall;
	bool const useStaticCall = funType.stateMutability() <= StateMutability::View && m_context.evmVersion().hasStaticCall();

	ReturnInfo const returnInfo{m_context.evmVersion(), funType};

	TypePointers parameterTypes = funType.parameterTypes();
	TypePointers argumentTypes;
	vector<string> argumentStrings;
	if (funType.bound())
	{
		parameterTypes.insert(parameterTypes.begin(), funType.selfType());
		argumentTypes.emplace_back(funType.selfType());
		argumentStrings += IRVariable(_functionCall.expression()).part("self").stackSlots();
	}

	for (auto const& arg: _arguments)
	{
		argumentTypes.emplace_back(&type(*arg));
		argumentStrings += IRVariable(*arg).stackSlots();
	}


	if (!m_context.evmVersion().canOverchargeGasForCall())
	{
		// Touch the end of the output area so that we do not pay for memory resize during the call
		// (which we would have to subtract from the gas left)
		// We could also just use MLOAD; POP right before the gas calculation, but the optimizer
		// would remove that, so we use MSTORE here.
		if (!funType.gasSet() && returnInfo.estimatedReturnSize > 0)
			appendCode() << "mstore(add(" << m_utils.allocateUnboundedFunction() << "() , " << to_string(returnInfo.estimatedReturnSize) << "), 0)\n";
	}

	Whiskers templ(R"(if iszero(extcodesize(<address>)) { <revertNoCode>() }

		// storage for arguments and returned data
		let <pos> := <allocateUnbounded>()
		mstore(<pos>, <shl28>(<funSel>))
		let <end> := <encodeArgs>(add(<pos>, 4) <argumentString>)

		let <success> := <call>(<gas>, <address>, <?hasValue> <value>, </hasValue> <pos>, sub(<end>, <pos>), <pos>, <reservedReturnSize>)
		<?noTryCall>
			if iszero(<success>) { <forwardingRevert>() }
		</noTryCall>
		<?+retVars> let <retVars> </+retVars>
		if <success> {
			<?dynamicReturnSize>
				// copy dynamic return data out
				returndatacopy(<pos>, 0, returndatasize())
			</dynamicReturnSize>

			// update freeMemoryPointer according to dynamic return size
			<finalizeAllocation>(<pos>, <returnSize>)

			// decode return parameters from external try-call into retVars
			<?+retVars> <retVars> := </+retVars> <abiDecode>(<pos>, add(<pos>, <returnSize>))
		}
	)");
	templ("revertNoCode", m_utils.revertReasonIfDebugFunction("Target contract does not contain code"));
	templ("pos", m_context.newYulVariable());
	templ("end", m_context.newYulVariable());
	if (_functionCall.annotation().tryCall)
		templ("success", IRNames::trySuccessConditionVariable(_functionCall));
	else
		templ("success", m_context.newYulVariable());
	templ("allocateUnbounded", m_utils.allocateUnboundedFunction());
	templ("finalizeAllocation", m_utils.finalizeAllocationFunction());
	templ("shl28", m_utils.shiftLeftFunction(8 * (32 - 4)));

	templ("funSel", IRVariable(_functionCall.expression()).part("functionSelector").name());
	templ("address", IRVariable(_functionCall.expression()).part("address").name());

	// Always use the actual return length, and not our calculated expected length, if returndatacopy is supported.
	// This ensures it can catch badly formatted input from external calls.
	if (m_context.evmVersion().supportsReturndata())
		templ("returnSize", "returndatasize()");
	else
		templ("returnSize", to_string(returnInfo.estimatedReturnSize));

	templ("reservedReturnSize", returnInfo.dynamicReturnSize ? "0" : to_string(returnInfo.estimatedReturnSize));

	string const retVars = IRVariable(_functionCall).commaSeparatedList();
	templ("retVars", retVars);
	solAssert(retVars.empty() == returnInfo.returnTypes.empty(), "");

	templ("abiDecode", m_context.abiFunctions().tupleDecoder(returnInfo.returnTypes, true));
	templ("dynamicReturnSize", returnInfo.dynamicReturnSize);

	templ("noTryCall", !_functionCall.annotation().tryCall);

	bool encodeForLibraryCall = funKind == FunctionType::Kind::DelegateCall;

	solAssert(funType.padArguments(), "");
	templ("encodeArgs", m_context.abiFunctions().tupleEncoder(argumentTypes, parameterTypes, encodeForLibraryCall));
	templ("argumentString", joinHumanReadablePrefixed(argumentStrings));

	solAssert(!isDelegateCall || !funType.valueSet(), "Value set for delegatecall");
	solAssert(!useStaticCall || !funType.valueSet(), "Value set for staticcall");

	templ("hasValue", !isDelegateCall && !useStaticCall);
	templ("value", funType.valueSet() ? IRVariable(_functionCall.expression()).part("value").name() : "0");

	if (funType.gasSet())
		templ("gas", IRVariable(_functionCall.expression()).part("gas").name());
	else if (m_context.evmVersion().canOverchargeGasForCall())
		// Send all gas (requires tangerine whistle EVM)
		templ("gas", "gas()");
	else
	{
		// send all gas except the amount needed to execute "SUB" and "CALL"
		// @todo this retains too much gas for now, needs to be fine-tuned.
		u256 gasNeededByCaller = evmasm::GasCosts::callGas(m_context.evmVersion()) + 10;
		if (funType.valueSet())
			gasNeededByCaller += evmasm::GasCosts::callValueTransferGas;
		templ("gas", "sub(gas(), " + formatNumber(gasNeededByCaller) + ")");
	}
	// Order is important here, STATICCALL might overlap with DELEGATECALL.
	if (isDelegateCall)
		templ("call", "delegatecall");
	else if (useStaticCall)
		templ("call", "staticcall");
	else
		templ("call", "call");

	templ("forwardingRevert", m_utils.forwardingRevertFunction());

	appendCode() << templ.render();
}

void IRGeneratorForStatements::appendBareCall(
	FunctionCall const& _functionCall,
	vector<ASTPointer<Expression const>> const& _arguments
)
{
	FunctionType const& funType = dynamic_cast<FunctionType const&>(type(_functionCall.expression()));
	solAssert(
		!funType.bound() &&
		!funType.takesArbitraryParameters() &&
		_arguments.size() == 1 &&
		funType.parameterTypes().size() == 1, ""
	);
	FunctionType::Kind const funKind = funType.kind();

	solAssert(funKind != FunctionType::Kind::BareStaticCall || m_context.evmVersion().hasStaticCall(), "");
	solAssert(funKind != FunctionType::Kind::BareCallCode, "Callcode has been removed.");
	solAssert(
		funKind == FunctionType::Kind::BareCall ||
		funKind == FunctionType::Kind::BareDelegateCall ||
		funKind == FunctionType::Kind::BareStaticCall, ""
	);

	solAssert(!_functionCall.annotation().tryCall, "");
	Whiskers templ(R"(
		<?needsEncoding>
			let <pos> := <allocateUnbounded>()
			let <length> := sub(<encode>(<pos> <?+arg>,</+arg> <arg>), <pos>)
		<!needsEncoding>
			let <pos> := add(<arg>, 0x20)
			let <length> := mload(<arg>)
		</needsEncoding>

		let <success> := <call>(<gas>, <address>, <?+value> <value>, </+value> <pos>, <length>, 0, 0)
		let <returndataVar> := <extractReturndataFunction>()
	)");

	templ("allocateUnbounded", m_utils.allocateUnboundedFunction());
	templ("pos", m_context.newYulVariable());
	templ("length", m_context.newYulVariable());

	templ("arg", IRVariable(*_arguments.front()).commaSeparatedList());
	Type const& argType = type(*_arguments.front());
	if (argType == *TypeProvider::bytesMemory() || argType == *TypeProvider::stringMemory())
		templ("needsEncoding", false);
	else
	{
		templ("needsEncoding", true);
		ABIFunctions abi(m_context.evmVersion(), m_context.revertStrings(), m_context.functionCollector());
		templ("encode", abi.tupleEncoderPacked({&argType}, {TypeProvider::bytesMemory()}));
	}

	templ("success", IRVariable(_functionCall).tupleComponent(0).name());
	templ("returndataVar", IRVariable(_functionCall).tupleComponent(1).commaSeparatedList());
	templ("extractReturndataFunction", m_utils.extractReturndataFunction());

	templ("address", IRVariable(_functionCall.expression()).part("address").name());

	if (funKind == FunctionType::Kind::BareCall)
	{
		templ("value", funType.valueSet() ? IRVariable(_functionCall.expression()).part("value").name() : "0");
		templ("call", "call");
	}
	else
	{
		solAssert(!funType.valueSet(), "Value set for delegatecall or staticcall.");
		templ("value", "");
		if (funKind == FunctionType::Kind::BareStaticCall)
			templ("call", "staticcall");
		else
			templ("call", "delegatecall");
	}

	if (funType.gasSet())
		templ("gas", IRVariable(_functionCall.expression()).part("gas").name());
	else if (m_context.evmVersion().canOverchargeGasForCall())
		// Send all gas (requires tangerine whistle EVM)
		templ("gas", "gas()");
	else
	{
		// send all gas except the amount needed to execute "SUB" and "CALL"
		// @todo this retains too much gas for now, needs to be fine-tuned.
		u256 gasNeededByCaller = evmasm::GasCosts::callGas(m_context.evmVersion()) + 10;
		if (funType.valueSet())
			gasNeededByCaller += evmasm::GasCosts::callValueTransferGas;
		gasNeededByCaller += evmasm::GasCosts::callNewAccountGas; // we never know
		templ("gas", "sub(gas(), " + formatNumber(gasNeededByCaller) + ")");
	}

	appendCode() << templ.render();
}

void IRGeneratorForStatements::assignInternalFunctionIDIfNotCalledDirectly(
	Expression const& _expression,
	FunctionDefinition const& _referencedFunction
)
{
	solAssert(
		dynamic_cast<MemberAccess const*>(&_expression) ||
		dynamic_cast<Identifier const*>(&_expression),
		""
	);
	if (_expression.annotation().calledDirectly)
		return;

	define(IRVariable(_expression).part("functionIdentifier")) <<
		to_string(m_context.internalFunctionID(_referencedFunction, false)) <<
		"\n";
	m_context.addToInternalDispatch(_referencedFunction);
}

IRVariable IRGeneratorForStatements::convert(IRVariable const& _from, Type const& _to)
{
	if (_from.type() == _to)
		return _from;
	else
	{
		IRVariable converted(m_context.newYulVariable(), _to);
		define(converted, _from);
		return converted;
	}
}

std::string IRGeneratorForStatements::expressionAsType(Expression const& _expression, Type const& _to, bool _forceCleanup)
{
	IRVariable from(_expression);
	if (from.type() == _to)
	{
		if (_forceCleanup)
			return m_utils.cleanupFunction(_to) + "(" + from.commaSeparatedList() + ")";
		else
			return from.commaSeparatedList();
	}
	else
		return m_utils.conversionFunction(from.type(), _to) + "(" + from.commaSeparatedList() + ")";
}

std::ostream& IRGeneratorForStatements::define(IRVariable const& _var)
{
	if (_var.type().sizeOnStack() > 0)
		appendCode() << "let " << _var.commaSeparatedList() << " := ";
	return appendCode(false);
}

void IRGeneratorForStatements::declare(IRVariable const& _var)
{
	if (_var.type().sizeOnStack() > 0)
		appendCode() << "let " << _var.commaSeparatedList() << "\n";
}

void IRGeneratorForStatements::declareAssign(IRVariable const& _lhs, IRVariable const& _rhs, bool _declare)
{
	string output;
	if (_lhs.type() == _rhs.type())
		for (auto const& [stackItemName, stackItemType]: _lhs.type().stackItems())
			if (stackItemType)
				declareAssign(_lhs.part(stackItemName), _rhs.part(stackItemName), _declare);
			else
				appendCode() << (_declare ? "let ": "") << _lhs.part(stackItemName).name() << " := " << _rhs.part(stackItemName).name() << "\n";
	else
	{
		if (_lhs.type().sizeOnStack() > 0)
			appendCode() <<
				(_declare ? "let ": "") <<
				_lhs.commaSeparatedList() <<
				" := ";
		appendCode() << m_context.utils().conversionFunction(_rhs.type(), _lhs.type()) <<
			"(" <<
			_rhs.commaSeparatedList() <<
			")\n";
	}
}

IRVariable IRGeneratorForStatements::zeroValue(Type const& _type, bool _splitFunctionTypes)
{
	IRVariable irVar{IRNames::zeroValue(_type, m_context.newYulVariable()), _type};
	define(irVar) << m_utils.zeroValueFunction(_type, _splitFunctionTypes) << "()\n";
	return irVar;
}

void IRGeneratorForStatements::appendSimpleUnaryOperation(UnaryOperation const& _operation, Expression const& _expr)
{
	string func;

	if (_operation.getOperator() == Token::Not)
		func = "iszero";
	else if (_operation.getOperator() == Token::BitNot)
		func = "not";
	else
		solAssert(false, "Invalid Token!");

	define(_operation) <<
		m_utils.cleanupFunction(type(_expr)) <<
		"(" <<
			func <<
			"(" <<
			IRVariable(_expr).commaSeparatedList() <<
			")" <<
		")\n";
}

string IRGeneratorForStatements::binaryOperation(
	langutil::Token _operator,
	Type const& _type,
	string const& _left,
	string const& _right
)
{
	solAssert(
		!TokenTraits::isShiftOp(_operator),
		"Have to use specific shift operation function for shifts."
	);
	string fun;
	if (TokenTraits::isBitOp(_operator))
	{
		solAssert(
			_type.category() == Type::Category::Integer ||
			_type.category() == Type::Category::FixedBytes,
			""
		);
		switch (_operator)
		{
		case Token::BitOr: fun = "or"; break;
		case Token::BitXor: fun = "xor"; break;
		case Token::BitAnd: fun = "and"; break;
		default: break;
		}
	}
	else if (TokenTraits::isArithmeticOp(_operator))
	{
		solUnimplementedAssert(
			_type.category() != Type::Category::FixedPoint,
			"Not yet implemented - FixedPointType."
		);
		IntegerType const* type = dynamic_cast<IntegerType const*>(&_type);
		solAssert(type, "");
		bool checked = m_context.arithmetic() == Arithmetic::Checked;
		switch (_operator)
		{
		case Token::Add:
			fun = checked ? m_utils.overflowCheckedIntAddFunction(*type) : m_utils.wrappingIntAddFunction(*type);
			break;
		case Token::Sub:
			fun = checked ? m_utils.overflowCheckedIntSubFunction(*type) : m_utils.wrappingIntSubFunction(*type);
			break;
		case Token::Mul:
			fun = checked ? m_utils.overflowCheckedIntMulFunction(*type) : m_utils.wrappingIntMulFunction(*type);
			break;
		case Token::Div:
			fun = checked ? m_utils.overflowCheckedIntDivFunction(*type) : m_utils.wrappingIntDivFunction(*type);
			break;
		case Token::Mod:
			fun = m_utils.intModFunction(*type);
			break;
		default:
			break;
		}
	}

	solUnimplementedAssert(!fun.empty(), "Type: " + _type.toString());
	return fun + "(" + _left + ", " + _right + ")\n";
}

std::string IRGeneratorForStatements::shiftOperation(
	langutil::Token _operator,
	IRVariable const& _value,
	IRVariable const& _amountToShift
)
{
	solUnimplementedAssert(
		_amountToShift.type().category() != Type::Category::FixedPoint &&
		_value.type().category() != Type::Category::FixedPoint,
		"Not yet implemented - FixedPointType."
	);
	IntegerType const* amountType = dynamic_cast<IntegerType const*>(&_amountToShift.type());
	solAssert(amountType, "");

	solAssert(_operator == Token::SHL || _operator == Token::SAR, "");

	return
		Whiskers(R"(
			<shift>(<value>, <amount>)
		)")
		("shift",
			_operator == Token::SHL ?
			m_utils.typedShiftLeftFunction(_value.type(), *amountType) :
			m_utils.typedShiftRightFunction(_value.type(), *amountType)
		)
		("value", _value.name())
		("amount", _amountToShift.name())
		.render();
}

void IRGeneratorForStatements::appendAndOrOperatorCode(BinaryOperation const& _binOp)
{
	langutil::Token const op = _binOp.getOperator();
	solAssert(op == Token::Or || op == Token::And, "");

	_binOp.leftExpression().accept(*this);
	setLocation(_binOp);

	IRVariable value(_binOp);
	define(value, _binOp.leftExpression());
	if (op == Token::Or)
		appendCode() << "if iszero(" << value.name() << ") {\n";
	else
		appendCode() << "if " << value.name() << " {\n";
	_binOp.rightExpression().accept(*this);
	setLocation(_binOp);
	assign(value, _binOp.rightExpression());
	appendCode() << "}\n";
}

void IRGeneratorForStatements::writeToLValue(IRLValue const& _lvalue, IRVariable const& _value)
{
	std::visit(
		util::GenericVisitor{
			[&](IRLValue::Storage const& _storage) {
				string offsetArgument;
				optional<unsigned> offsetStatic;

				std::visit(GenericVisitor{
					[&](unsigned _offset) { offsetStatic = _offset; },
					[&](string const& _offset) { offsetArgument = ", " + _offset; }
				}, _storage.offset);

				appendCode() <<
					m_utils.updateStorageValueFunction(_value.type(), _lvalue.type, offsetStatic) <<
					"(" <<
					_storage.slot <<
					offsetArgument <<
					_value.commaSeparatedListPrefixed() <<
					")\n";

			},
			[&](IRLValue::Memory const& _memory) {
				if (_lvalue.type.isValueType())
				{
					IRVariable prepared(m_context.newYulVariable(), _lvalue.type);
					define(prepared, _value);

					if (_memory.byteArrayElement)
					{
						solAssert(_lvalue.type == *TypeProvider::byte(), "");
						appendCode() << "mstore8(" + _memory.address + ", byte(0, " + prepared.commaSeparatedList() + "))\n";
					}
					else
						appendCode() << m_utils.writeToMemoryFunction(_lvalue.type) <<
							"(" <<
							_memory.address <<
							", " <<
							prepared.commaSeparatedList() <<
							")\n";
				}
				else if (auto const* literalType = dynamic_cast<StringLiteralType const*>(&_value.type()))
					appendCode() <<
						m_utils.writeToMemoryFunction(*TypeProvider::uint256()) <<
						"(" <<
						_memory.address <<
						", " <<
						m_utils.copyLiteralToMemoryFunction(literalType->value()) + "()" <<
						")\n";
				else
				{
					solAssert(_lvalue.type.sizeOnStack() == 1, "");
					auto const* valueReferenceType = dynamic_cast<ReferenceType const*>(&_value.type());
					solAssert(valueReferenceType && valueReferenceType->dataStoredIn(DataLocation::Memory), "");
					appendCode() << "mstore(" + _memory.address + ", " + _value.part("mpos").name() + ")\n";
				}
			},
			[&](IRLValue::Stack const& _stack) { assign(_stack.variable, _value); },
			[&](IRLValue::Immutable const& _immutable)
			{
				solUnimplementedAssert(_lvalue.type.isValueType(), "");
				solUnimplementedAssert(_lvalue.type.sizeOnStack() == 1, "");
				solAssert(_lvalue.type == *_immutable.variable->type(), "");
				size_t memOffset = m_context.immutableMemoryOffset(*_immutable.variable);

				IRVariable prepared(m_context.newYulVariable(), _lvalue.type);
				define(prepared, _value);

				appendCode() << "mstore(" << to_string(memOffset) << ", " << prepared.commaSeparatedList() << ")\n";
			},
			[&](IRLValue::Tuple const& _tuple) {
				auto components = std::move(_tuple.components);
				for (size_t i = 0; i < components.size(); i++)
				{
					size_t idx = components.size() - i - 1;
					if (components[idx])
						writeToLValue(*components[idx], _value.tupleComponent(idx));
				}
			}
		},
		_lvalue.kind
	);
}

IRVariable IRGeneratorForStatements::readFromLValue(IRLValue const& _lvalue)
{
	IRVariable result{m_context.newYulVariable(), _lvalue.type};
	std::visit(GenericVisitor{
		[&](IRLValue::Storage const& _storage) {
			if (!_lvalue.type.isValueType())
				define(result) << _storage.slot << "\n";
			else if (std::holds_alternative<string>(_storage.offset))
				define(result) <<
					m_utils.readFromStorageDynamic(_lvalue.type, true) <<
					"(" <<
					_storage.slot <<
					", " <<
					std::get<string>(_storage.offset) <<
					")\n";
			else
				define(result) <<
					m_utils.readFromStorage(_lvalue.type, std::get<unsigned>(_storage.offset), true) <<
					"(" <<
					_storage.slot <<
					")\n";
		},
		[&](IRLValue::Memory const& _memory) {
			if (_lvalue.type.isValueType())
				define(result) <<
					m_utils.readFromMemory(_lvalue.type) <<
					"(" <<
					_memory.address <<
					")\n";
			else
				define(result) << "mload(" << _memory.address << ")\n";
		},
		[&](IRLValue::Stack const& _stack) {
			define(result, _stack.variable);
		},
		[&](IRLValue::Immutable const& _immutable) {
			solUnimplementedAssert(_lvalue.type.isValueType(), "");
			solUnimplementedAssert(_lvalue.type.sizeOnStack() == 1, "");
			solAssert(_lvalue.type == *_immutable.variable->type(), "");
			if (m_context.executionContext() == IRGenerationContext::ExecutionContext::Creation)
				define(result) <<
					m_utils.readFromMemory(*_immutable.variable->type()) <<
					"(" <<
					to_string(m_context.immutableMemoryOffset(*_immutable.variable)) <<
					")\n";
			else
				define(result) << "loadimmutable(\"" << to_string(_immutable.variable->id()) << "\")\n";
		},
		[&](IRLValue::Tuple const&) {
			solAssert(false, "Attempted to read from tuple lvalue.");
		}
	}, _lvalue.kind);
	return result;
}

void IRGeneratorForStatements::setLValue(Expression const& _expression, IRLValue _lvalue)
{
	solAssert(!m_currentLValue, "");

	if (_expression.annotation().willBeWrittenTo)
	{
		m_currentLValue.emplace(std::move(_lvalue));
		if (_lvalue.type.dataStoredIn(DataLocation::CallData))
			solAssert(holds_alternative<IRLValue::Stack>(_lvalue.kind), "");
	}
	else
		// Only define the expression, if it will not be written to.
		define(_expression, readFromLValue(_lvalue));
}

void IRGeneratorForStatements::generateLoop(
	Statement const& _body,
	Expression const* _conditionExpression,
	Statement const*  _initExpression,
	ExpressionStatement const* _loopExpression,
	bool _isDoWhile
)
{
	string firstRun;

	if (_isDoWhile)
	{
		solAssert(_conditionExpression, "Expected condition for doWhile");
		firstRun = m_context.newYulVariable();
		appendCode() << "let " << firstRun << " := 1\n";
	}

	appendCode() << "for {\n";
	if (_initExpression)
		_initExpression->accept(*this);
	appendCode() << "} 1 {\n";
	if (_loopExpression)
		_loopExpression->accept(*this);
	appendCode() << "}\n";
	appendCode() << "{\n";

	if (_conditionExpression)
	{
		if (_isDoWhile)
			appendCode() << "if iszero(" << firstRun << ") {\n";

		_conditionExpression->accept(*this);
		appendCode() <<
			"if iszero(" <<
			expressionAsType(*_conditionExpression, *TypeProvider::boolean()) <<
			") { break }\n";

		if (_isDoWhile)
			appendCode() << "}\n" << firstRun << " := 0\n";
	}

	_body.accept(*this);

	appendCode() << "}\n";
}

Type const& IRGeneratorForStatements::type(Expression const& _expression)
{
	solAssert(_expression.annotation().type, "Type of expression not set.");
	return *_expression.annotation().type;
}

bool IRGeneratorForStatements::visit(TryStatement const& _tryStatement)
{
	Expression const& externalCall = _tryStatement.externalCall();
	externalCall.accept(*this);
	setLocation(_tryStatement);

	appendCode() << "switch iszero(" << IRNames::trySuccessConditionVariable(externalCall) << ")\n";

	appendCode() << "case 0 { // success case\n";
	TryCatchClause const& successClause = *_tryStatement.clauses().front();
	if (successClause.parameters())
	{
		size_t i = 0;
		for (ASTPointer<VariableDeclaration> const& varDecl: successClause.parameters()->parameters())
		{
			solAssert(varDecl, "");
			define(m_context.addLocalVariable(*varDecl),
				successClause.parameters()->parameters().size() == 1 ?
				IRVariable(externalCall) :
				IRVariable(externalCall).tupleComponent(i++)
			);
		}
	}

	successClause.block().accept(*this);
	setLocation(_tryStatement);
	appendCode() << "}\n";

	appendCode() << "default { // failure case\n";
	handleCatch(_tryStatement);
	appendCode() << "}\n";

	return false;
}

void IRGeneratorForStatements::handleCatch(TryStatement const& _tryStatement)
{
	setLocation(_tryStatement);
	string const runFallback = m_context.newYulVariable();
	appendCode() << "let " << runFallback << " := 1\n";

	// This function returns zero on "short returndata". We have to add a success flag
	// once we implement custom error codes.
	if (_tryStatement.errorClause() || _tryStatement.panicClause())
		appendCode() << "switch " << m_utils.returnDataSelectorFunction() << "()\n";

	if (TryCatchClause const* errorClause = _tryStatement.errorClause())
	{
		appendCode() << "case " << selectorFromSignature32("Error(string)") << " {\n";
		setLocation(*errorClause);
		string const dataVariable = m_context.newYulVariable();
		appendCode() << "let " << dataVariable << " := " << m_utils.tryDecodeErrorMessageFunction() << "()\n";
		appendCode() << "if " << dataVariable << " {\n";
		appendCode() << runFallback << " := 0\n";
		if (errorClause->parameters())
		{
			solAssert(errorClause->parameters()->parameters().size() == 1, "");
			IRVariable const& var = m_context.addLocalVariable(*errorClause->parameters()->parameters().front());
			define(var) << dataVariable << "\n";
		}
		errorClause->accept(*this);
		setLocation(*errorClause);
		appendCode() << "}\n";
		setLocation(_tryStatement);
		appendCode() << "}\n";
	}
	if (TryCatchClause const* panicClause = _tryStatement.panicClause())
	{
		appendCode() << "case " << selectorFromSignature32("Panic(uint256)") << " {\n";
		setLocation(*panicClause);
		string const success = m_context.newYulVariable();
		string const code = m_context.newYulVariable();
		appendCode() << "let " << success << ", " << code << " := " << m_utils.tryDecodePanicDataFunction() << "()\n";
		appendCode() << "if " << success << " {\n";
		appendCode() << runFallback << " := 0\n";
		if (panicClause->parameters())
		{
			solAssert(panicClause->parameters()->parameters().size() == 1, "");
			IRVariable const& var = m_context.addLocalVariable(*panicClause->parameters()->parameters().front());
			define(var) << code << "\n";
		}
		panicClause->accept(*this);
		setLocation(*panicClause);
		appendCode() << "}\n";
		setLocation(_tryStatement);
		appendCode() << "}\n";
	}

	setLocation(_tryStatement);
	appendCode() << "if " << runFallback << " {\n";
	if (_tryStatement.fallbackClause())
		handleCatchFallback(*_tryStatement.fallbackClause());
	else
		appendCode() << m_utils.forwardingRevertFunction() << "()\n";
	setLocation(_tryStatement);
	appendCode() << "}\n";
}

void IRGeneratorForStatements::handleCatchFallback(TryCatchClause const& _fallback)
{
	setLocation(_fallback);
	if (_fallback.parameters())
	{
		solAssert(m_context.evmVersion().supportsReturndata(), "");
		solAssert(
			_fallback.parameters()->parameters().size() == 1 &&
			_fallback.parameters()->parameters().front() &&
			*_fallback.parameters()->parameters().front()->annotation().type == *TypeProvider::bytesMemory(),
			""
		);

		VariableDeclaration const& paramDecl = *_fallback.parameters()->parameters().front();
		define(m_context.addLocalVariable(paramDecl)) << m_utils.extractReturndataFunction() << "()\n";
	}
	_fallback.accept(*this);
}

void IRGeneratorForStatements::revertWithError(
	string const& _signature,
	vector<Type const*> const& _parameterTypes,
	vector<ASTPointer<Expression const>> const& _errorArguments
)
{
	Whiskers templ(R"({
		let <pos> := <allocateUnbounded>()
		mstore(<pos>, <hash>)
		let <end> := <encode>(add(<pos>, 4) <argumentVars>)
		revert(<pos>, sub(<end>, <pos>))
	})");
	templ("pos", m_context.newYulVariable());
	templ("end", m_context.newYulVariable());
	templ("hash", util::selectorFromSignature(_signature).str());
	templ("allocateUnbounded", m_utils.allocateUnboundedFunction());

	vector<string> errorArgumentVars;
	vector<Type const*> errorArgumentTypes;
	for (ASTPointer<Expression const> const& arg: _errorArguments)
	{
		errorArgumentVars += IRVariable(*arg).stackSlots();
		solAssert(arg->annotation().type, "");
		errorArgumentTypes.push_back(arg->annotation().type);
	}
	templ("argumentVars", joinHumanReadablePrefixed(errorArgumentVars));
	templ("encode", m_context.abiFunctions().tupleEncoder(errorArgumentTypes, _parameterTypes));

	appendCode() << templ.render();
}


bool IRGeneratorForStatements::visit(TryCatchClause const& _clause)
{
	_clause.block().accept(*this);
	return false;
}

string IRGeneratorForStatements::linkerSymbol(ContractDefinition const& _library) const
{
	solAssert(_library.isLibrary(), "");
	return "linkersymbol(" + util::escapeAndQuoteString(_library.fullyQualifiedName()) + ")";
}
