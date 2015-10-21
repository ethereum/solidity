/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Component that translates Solidity code into the why3 programming language.
 */

#include <libsolidity/formal/Why3Translator.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

bool Why3Translator::process(SourceUnit const& _source)
{
	try
	{
		m_indentation = 0;
		if (!m_result.empty())
			fatalError(_source, "Multiple source units not yet supported");
		appendPreface();
		_source.accept(*this);
		addLine("end");
	}
	catch (FatalError& _e)
	{
		solAssert(m_errorOccured, "");
	}
	return !m_errorOccured;
}

void Why3Translator::error(ASTNode const& _node, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::FormalError);
	*err <<
		errinfo_sourceLocation(_node.location()) <<
		errinfo_comment(_description);
	m_errors.push_back(err);
	m_errorOccured = true;
}

void Why3Translator::fatalError(ASTNode const& _node, string const& _description)
{
	error(_node, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}

void Why3Translator::appendPreface()
{
	m_result += R"(
module UInt256
	use import mach.int.Unsigned
	type uint256
	constant max_uint256: int = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
	clone export mach.int.Unsigned with
		type t = uint256,
		constant max = max_uint256
end

module Solidity
use import int.Int
use import ref.Ref
use import map.Map
use import array.Array
use import int.ComputerDivision
use import mach.int.Unsigned
use import UInt256

exception Ret
type state = StateUnused
)";
}

string Why3Translator::toFormalType(Type const& _type) const
{
	if (auto type = dynamic_cast<IntegerType const*>(&_type))
	{
		if (!type->isAddress() && !type->isSigned() && type->numBits() == 256)
			return "uint256";
	}
	else if (auto type = dynamic_cast<ArrayType const*>(&_type))
		if (!type->isByteArray() && type->isDynamicallySized() && type->dataStoredIn(DataLocation::Memory))
		{
			string base = toFormalType(*type->baseType());
			if (!base.empty())
				return "array " + base;
		}

	return "";
}

void Why3Translator::addLine(string const& _line)
{
	newLine();
	add(_line);
	newLine();
}

void Why3Translator::add(string const& _str)
{
	m_currentLine += _str;
}

void Why3Translator::newLine()
{
	if (!m_currentLine.empty())
	{
		for (size_t i = 0; i < m_indentation; ++i)
			m_result.push_back('\t');
		m_result += m_currentLine;
		m_result.push_back('\n');
		m_currentLine.clear();
	}
}

void Why3Translator::unindent()
{
	solAssert(m_indentation > 0, "");
	m_indentation--;
}

bool Why3Translator::visit(ContractDefinition const& _contract)
{
	if (m_seenContract)
		error(_contract, "More than one contract not supported.");
	m_seenContract = true;
	if (_contract.isLibrary())
		error(_contract, "Libraries not supported.");

	addSourceFromDocStrings(_contract.annotation());

	return true;
}

bool Why3Translator::visit(FunctionDefinition const& _function)
{
	if (!_function.isImplemented())
	{
		error(_function, "Unimplemented functions not supported.");
		return false;
	}
	if (_function.name().empty())
	{
		error(_function, "Fallback functions not supported.");
		return false;
	}
	if (!_function.modifiers().empty())
	{
		error(_function, "Modifiers not supported.");
		return false;
	}

	add("let rec _" + _function.name());
	add(" (state: state)");
	for (auto const& param: _function.parameters())
	{
		string paramType = toFormalType(*param->annotation().type);
		if (paramType.empty())
			error(*param, "Parameter type not supported.");
		if (param->name().empty())
			error(*param, "Anonymous function parameters not supported.");
		add(" (arg_" + param->name() + ": " + paramType + ")");
	}
	add(":");
	newLine();

	indent();
	indent();
	string retString = "(";
	for (auto const& retParam: _function.returnParameters())
	{
		string paramType = toFormalType(*retParam->annotation().type);
		if (paramType.empty())
			error(*retParam, "Parameter type not supported.");
		if (retString.size() != 1)
			retString += ", ";
		retString += paramType;
	}
	addLine(retString + ")");
	unindent();

	addSourceFromDocStrings(_function.annotation());

	addLine("=");

	// initialise local variables
	for (auto const& variable: _function.parameters())
		addLine("let _" + variable->name() + " = ref arg_" + variable->name() + " in");
	for (auto const& variable: _function.returnParameters())
	{
		if (variable->name().empty())
			error(*variable, "Unnamed return variables not yet supported.");
		string varType = toFormalType(*variable->annotation().type);
		addLine("let _" + variable->name() + ": ref " + varType + " = ref (of_int 0) in");
	}
	for (VariableDeclaration const* variable: _function.localVariables())
	{
		if (variable->name().empty())
			error(*variable, "Unnamed variables not yet supported.");
		string varType = toFormalType(*variable->annotation().type);
		addLine("let _" + variable->name() + ": ref " + varType + " = ref (of_int 0) in");
	}
	addLine("try");

	_function.body().accept(*this);
	addLine("raise Ret");

	string retVals;
	for (auto const& variable: _function.returnParameters())
	{
		if (!retVals.empty())
			retVals += ", ";
		retVals += "!_" + variable->name();
	}
	addLine("with Ret -> (" + retVals + ")");
	newLine();
	unindent();
	addLine("end");
	addLine("");
	return false;
}

bool Why3Translator::visit(Block const& _node)
{
	addSourceFromDocStrings(_node.annotation());
	addLine("begin");
	indent();
	return true;
}

bool Why3Translator::visit(IfStatement const& _node)
{
	addSourceFromDocStrings(_node.annotation());

	add("if ");
	_node.condition().accept(*this);
	add(" then");
	newLine();
	_node.trueStatement().accept(*this);
	if (_node.falseStatement())
	{
		addLine("else");
		_node.falseStatement()->accept(*this);
	}
	return false;
}

bool Why3Translator::visit(WhileStatement const& _node)
{
	addSourceFromDocStrings(_node.annotation());

	add("while ");
	_node.condition().accept(*this);
	add(" do");
	newLine();
	_node.body().accept(*this);
	addLine("done;");
	return false;
}

bool Why3Translator::visit(Return const& _node)
{
	addSourceFromDocStrings(_node.annotation());

	if (_node.expression())
	{
		solAssert(!!_node.annotation().functionReturnParameters, "");
		auto const& params = _node.annotation().functionReturnParameters->parameters();
		if (params.size() != 1)
		{
			error(_node, "Directly returning tuples not supported. Rather assign to return variable.");
			return false;
		}
		newLine();
		add("begin _" + params.front()->name() + " := ");
		_node.expression()->accept(*this);
		add("; raise Ret end");
		newLine();
	}
	else
		addLine("raise Ret;");
	return false;
}

bool Why3Translator::visit(VariableDeclarationStatement const& _node)
{
	addSourceFromDocStrings(_node.annotation());

	if (_node.declarations().size() != 1)
	{
		error(_node, "Multiple variables not supported.");
		return false;
	}
	if (_node.initialValue())
	{
		add("_" + _node.declarations().front()->name() + " := ");
		_node.initialValue()->accept(*this);
		add(";");
		newLine();
	}
	return false;
}

bool Why3Translator::visit(ExpressionStatement const& _node)
{
	addSourceFromDocStrings(_node.annotation());
	return true;
}

bool Why3Translator::visit(Assignment const& _node)
{
	if (_node.assignmentOperator() != Token::Assign)
		error(_node, "Compound assignment not supported.");

	_node.leftHandSide().accept(*this);
	add(" := ");
	_node.rightHandSide().accept(*this);

	return false;
}

bool Why3Translator::visit(TupleExpression const& _node)
{
	if (_node.components().size() != 1)
		error(_node, "Only tuples with exactly one component supported.");
	add("(");
	return true;
}

bool Why3Translator::visit(UnaryOperation const& _unaryOperation)
{
	if (toFormalType(*_unaryOperation.annotation().type).empty())
		error(_unaryOperation, "Type not supported.");

	switch (_unaryOperation.getOperator())
	{
	case Token::Not: // !
		add("(not ");
		break;
	default:
		error(_unaryOperation, "Operator not supported.");
		break;
	}

	_unaryOperation.subExpression().accept(*this);
	add(")");

	return false;
}

bool Why3Translator::visit(BinaryOperation const& _binaryOperation)
{
	Expression const& leftExpression = _binaryOperation.leftExpression();
	Expression const& rightExpression = _binaryOperation.rightExpression();
	solAssert(!!_binaryOperation.annotation().commonType, "");
	Type const& commonType = *_binaryOperation.annotation().commonType;
	Token::Value const c_op = _binaryOperation.getOperator();

	if (commonType.category() == Type::Category::IntegerConstant)
	{
		add("(of_int " + toString(commonType.literalValue(nullptr)) + ")");
		return false;
	}
	static const map<Token::Value, char const*> optrans({
		{Token::And, " && "},
		{Token::Or, " || "},
		{Token::BitOr, " lor "},
		{Token::BitXor, " lxor "},
		{Token::BitAnd, " land "},
		{Token::Add, " + "},
		{Token::Sub, " - "},
		{Token::Mul, " * "},
		{Token::Div, " / "},
		{Token::Mod, " mod "},
		{Token::Equal, " = "},
		{Token::NotEqual, " <> "},
		{Token::LessThan, " < "},
		{Token::GreaterThan, " > "},
		{Token::LessThanOrEqual, " <= "},
		{Token::GreaterThanOrEqual, " >= "}
	});
	if (!optrans.count(c_op))
		error(_binaryOperation, "Operator not supported.");

	add("(");
	leftExpression.accept(*this);
	add(optrans.at(c_op));
	rightExpression.accept(*this);
	add(")");

	return false;
}

bool Why3Translator::visit(FunctionCall const& _node)
{
	if (_node.annotation().isTypeConversion || _node.annotation().isStructConstructorCall)
	{
		error(_node, "Only ordinary function calls supported.");
		return true;
	}
	FunctionType const& function = dynamic_cast<FunctionType const&>(*_node.expression().annotation().type);
	if (function.location() != FunctionType::Location::Internal)
	{
		error(_node, "Only internal function calls supported.");
		return true;
	}
	if (!_node.names().empty())
	{
		error(_node, "Function calls with named arguments not supported.");
		return true;
	}

	//@TODO check type conversions

	add("(");
	_node.expression().accept(*this);
	add(" StateUnused");
	for (auto const& arg: _node.arguments())
	{
		add(" ");
		arg->accept(*this);
	}
	add(")");
	return false;
}

bool Why3Translator::visit(MemberAccess const& _node)
{
	if (
		_node.expression().annotation().type->category() == Type::Category::Array &&
		_node.memberName() == "length" &&
		!_node.annotation().lValueRequested
	)
	{
		add("(of_int ");
		_node.expression().accept(*this);
		add(".length");
		add(")");
	}
	else
		error(_node, "Only read-only length access for arrays supported.");
	return false;
}

bool Why3Translator::visit(IndexAccess const& _node)
{
	auto baseType = dynamic_cast<ArrayType const*>(_node.baseExpression().annotation().type.get());
	if (!baseType)
	{
		error(_node, "Index access only supported for arrays.");
		return true;
	}
	if (_node.annotation().lValueRequested)
	{
		error(_node, "Assignment to array elements not supported.");
		return true;
	}
	add("(");
	_node.baseExpression().accept(*this);
	add("[to_int ");
	_node.indexExpression()->accept(*this);
	add("]");
	add(")");

	return false;
}

bool Why3Translator::visit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
		add("_" + functionDef->name());
	else if (auto variable = dynamic_cast<VariableDeclaration const*>(declaration))
	{
		if (_identifier.annotation().lValueRequested)
			add("_" + variable->name());
		else
			add("!_" + variable->name());
	}
	else
		error(_identifier, "Not supported.");
	return false;
}

bool Why3Translator::visit(Literal const& _literal)
{
	TypePointer type = _literal.annotation().type;
	switch (type->category())
	{
	case Type::Category::Bool:
		if (type->literalValue(&_literal) == 0)
			add("false");
		else
			add("true");
		break;
	case Type::Category::IntegerConstant:
		add("(of_int " + toString(type->literalValue(&_literal)) + ")");
		break;
	default:
		error(_literal, "Not supported.");
	}
	return false;
}

void Why3Translator::addSourceFromDocStrings(const DocumentedAnnotation& _annotation)
{
	auto why3Range = _annotation.docTags.equal_range("why3");
	for (auto i = why3Range.first; i != why3Range.second; ++i)
		addLine(i->second.content);
}
