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
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

bool Why3Translator::process(SourceUnit const& _source)
{
	try
	{
		if (m_lines.size() != 1 || !m_lines.back().contents.empty())
			fatalError(_source, "Multiple source units not yet supported");
		appendPreface();
		_source.accept(*this);
	}
	catch (FatalError& /*_e*/)
	{
		solAssert(m_errorOccured, "");
	}
	return !m_errorOccured;
}

string Why3Translator::translation() const
{
	string result;
	for (auto const& line: m_lines)
		result += string(line.indentation, '\t') + line.contents + "\n";
	return result;
}

void Why3Translator::error(ASTNode const& _node, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::Why3TranslatorError);
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
	m_lines.push_back(Line{R"(
module UInt256
	use import mach.int.Unsigned
	type uint256
	constant max_uint256: int = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
	clone export mach.int.Unsigned with
		type t = uint256,
		constant max = max_uint256
end
)", 0});
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
	m_lines.back().contents += _str;
}

void Why3Translator::newLine()
{
	if (!m_lines.back().contents.empty())
		m_lines.push_back({"", m_lines.back().indentation});
}

void Why3Translator::unindent()
{
	newLine();
	solAssert(m_lines.back().indentation > 0, "");
	m_lines.back().indentation--;
}

bool Why3Translator::visit(ContractDefinition const& _contract)
{
	if (m_seenContract)
		error(_contract, "More than one contract not supported.");
	m_seenContract = true;
	if (_contract.isLibrary())
		error(_contract, "Libraries not supported.");

	addLine("module Contract_" + _contract.name());
	indent();
	addLine("use import int.Int");
	addLine("use import ref.Ref");
	addLine("use import map.Map");
	addLine("use import array.Array");
	addLine("use import int.ComputerDivision");
	addLine("use import mach.int.Unsigned");
	addLine("use import UInt256");
	addLine("exception Ret");

	addLine("type state = {");
	indent();
	m_stateVariables = _contract.stateVariables();
	for (VariableDeclaration const* variable: m_stateVariables)
	{
		string varType = toFormalType(*variable->annotation().type);
		if (varType.empty())
			fatalError(*variable, "Type not supported.");
		addLine("mutable _" + variable->name() + ": ref " + varType);
	}
	unindent();
	addLine("}");

	if (!_contract.baseContracts().empty())
		error(*_contract.baseContracts().front(), "Inheritance not supported.");
	if (!_contract.definedStructs().empty())
		error(*_contract.definedStructs().front(), "User-defined types not supported.");
	if (!_contract.definedEnums().empty())
		error(*_contract.definedEnums().front(), "User-defined types not supported.");
	if (!_contract.events().empty())
		error(*_contract.events().front(), "Events not supported.");
	if (!_contract.functionModifiers().empty())
		error(*_contract.functionModifiers().front(), "Modifiers not supported.");

	ASTNode::listAccept(_contract.definedFunctions(), *this);

	return false;
}

void Why3Translator::endVisit(ContractDefinition const& _contract)
{
	m_stateVariables.clear();
	addSourceFromDocStrings(_contract.annotation());
	unindent();
	addLine("end");
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

	m_localVariables.clear();
	for (auto const& var: _function.parameters())
		m_localVariables[var->name()] = var.get();
	for (auto const& var: _function.returnParameters())
		m_localVariables[var->name()] = var.get();
	for (auto const& var: _function.localVariables())
		m_localVariables[var->name()] = var;

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
	add(retString + ")");
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
	add(";");
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

void Why3Translator::endVisit(FunctionDefinition const&)
{
	m_localVariables.clear();
}

bool Why3Translator::visit(Block const& _node)
{
	addSourceFromDocStrings(_node.annotation());
	add("begin");
	indent();
	for (size_t i = 0; i < _node.statements().size(); ++i)
	{
		_node.statements()[i]->accept(*this);
		if (i != _node.statements().size() - 1)
		{
			auto it = m_lines.end() - 1;
			while (it != m_lines.begin() && it->contents.empty())
				--it;
			if (!boost::algorithm::ends_with(it->contents, "begin"))
				it->contents += ";";
		}
		newLine();
	}
	unindent();
	add("end");
	return false;
}

bool Why3Translator::visit(IfStatement const& _node)
{
	addSourceFromDocStrings(_node.annotation());

	add("if ");
	_node.condition().accept(*this);
	add(" then");
	visitIndentedUnlessBlock(_node.trueStatement());
	if (_node.falseStatement())
	{
		newLine();
		add("else");
		visitIndentedUnlessBlock(*_node.falseStatement());
	}
	return false;
}

bool Why3Translator::visit(WhileStatement const& _node)
{
	addSourceFromDocStrings(_node.annotation());

	add("while ");
	_node.condition().accept(*this);
	newLine();
	add("do");
	visitIndentedUnlessBlock(_node.body());
	add("done");
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
		add("begin _" + params.front()->name() + " := ");
		_node.expression()->accept(*this);
		add("; raise Ret end");
	}
	else
		add("raise Ret");
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
	switch (function.location())
	{
	case FunctionType::Location::AddMod:
	case FunctionType::Location::MulMod:
	{
		//@todo require that third parameter is not zero
		add("(of_int (mod (Int.(");
		add(function.location() == FunctionType::Location::AddMod ? "+" : "*");
		add(") (to_int ");
		_node.arguments().at(0)->accept(*this);
		add(") (to_int ");
		_node.arguments().at(1)->accept(*this);
		add(")) (to_int ");
		_node.arguments().at(2)->accept(*this);
		add(")))");
		return false;
	}
	case FunctionType::Location::Internal:
	{
		if (!_node.names().empty())
		{
			error(_node, "Function calls with named arguments not supported.");
			return true;
		}

		//@TODO check type conversions

		add("(");
		_node.expression().accept(*this);
		add(" state");
		for (auto const& arg: _node.arguments())
		{
			add(" ");
			arg->accept(*this);
		}
		add(")");
		return false;
	}
	default:
		error(_node, "Only internal function calls supported.");
		return true;
	}
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
		bool isStateVar = isStateVariable(variable);
		bool lvalue = _identifier.annotation().lValueRequested;
		if (!lvalue)
			add("!(");
		if (isStateVar)
			add("state.");
		add("_" + variable->name());
		if (!lvalue)
			add(")");
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

bool Why3Translator::isStateVariable(VariableDeclaration const* _var) const
{
	return contains(m_stateVariables, _var);
}

bool Why3Translator::isStateVariable(string const& _name) const
{
	for (auto const& var: m_stateVariables)
		if (var->name() == _name)
			return true;
	return false;
}

bool Why3Translator::isLocalVariable(VariableDeclaration const* _var) const
{
	for (auto const& var: m_localVariables)
		if (var.second == _var)
			return true;
	return false;
}

bool Why3Translator::isLocalVariable(string const& _name) const
{
	return m_localVariables.count(_name);
}

void Why3Translator::visitIndentedUnlessBlock(Statement const& _statement)
{
	bool isBlock = !!dynamic_cast<Block const*>(&_statement);
	if (isBlock)
		newLine();
	else
		indent();
	_statement.accept(*this);
	if (isBlock)
		newLine();
	else
		unindent();
}

void Why3Translator::addSourceFromDocStrings(DocumentedAnnotation const& _annotation)
{
	auto why3Range = _annotation.docTags.equal_range("why3");
	for (auto i = why3Range.first; i != why3Range.second; ++i)
		addLine(transformVariableReferences(i->second.content));
}

string Why3Translator::transformVariableReferences(string const& _annotation)
{
	string ret;
	auto pos = _annotation.begin();
	while (true)
	{
		auto hash = find(pos, _annotation.end(), '#');
		ret.append(pos, hash);
		if (hash == _annotation.end())
			break;

		auto hashEnd = find_if(hash + 1, _annotation.end(), [](char _c)
		{
			return
				(_c != '_' && _c != '$') &&
				!('a' <= _c && _c <= 'z') &&
				!('A' <= _c && _c <= 'Z') &&
				!('0' <= _c && _c <= '9');
		});
		string varName(hash + 1, hashEnd);
		if (isLocalVariable(varName))
			ret += "(to_int !_" + varName + ")";
		else if (isStateVariable(varName))
			ret += "(to_int !(state._" + varName + "))";
		else if (varName == "result") //@todo actually use the name of the return parameters
			ret += "(to_int result)";
		else
			ret.append(hash, hashEnd);

		pos = hashEnd;
	}
	return ret;
}

