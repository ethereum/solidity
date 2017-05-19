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
	catch (NoFormalType&)
	{
		solAssert(false, "There is a call to toFormalType() that does not catch NoFormalType exceptions.");
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

string Why3Translator::toFormalType(Type const& _type) const
{
	if (_type.category() == Type::Category::Bool)
		return "bool";
	else if (auto type = dynamic_cast<IntegerType const*>(&_type))
	{
		if (!type->isAddress() && !type->isSigned() && type->numBits() == 256)
			return "uint256";
	}
	else if (auto type = dynamic_cast<ArrayType const*>(&_type))
	{
		if (!type->isByteArray() && type->isDynamicallySized() && type->dataStoredIn(DataLocation::Memory))
		{
			// Not catching NoFormalType exception.  Let the caller deal with it.
			string base = toFormalType(*type->baseType());
			return "array " + base;
		}
	}
	else if (auto mappingType = dynamic_cast<MappingType const*>(&_type))
	{
		solAssert(mappingType->keyType(), "A mappingType misses a keyType.");
		if (dynamic_cast<IntegerType const*>(&*mappingType->keyType()))
		{
			//@TODO Use the information from the key type and specify the length of the array as an invariant.
			// Also the constructor need to specify the length of the array.
			solAssert(mappingType->valueType(), "A mappingType misses a valueType.");
			// Not catching NoFormalType exception.  Let the caller deal with it.
			string valueTypeFormal = toFormalType(*mappingType->valueType());
			return "array " + valueTypeFormal;
		}
	}

	BOOST_THROW_EXCEPTION(NoFormalType()
		<< errinfo_noFormalTypeFrom(_type.toString(true)));
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
	m_currentContract.contract = &_contract;
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
	addLine("exception Revert");
	addLine("exception Return");

	if (_contract.stateVariables().empty())
		addLine("type state = ()");
	else
	{
		addLine("type state = {");
		indent();
		m_currentContract.stateVariables = _contract.stateVariables();
		for (VariableDeclaration const* variable: m_currentContract.stateVariables)
		{
			string varType;
			try
			{
				varType = toFormalType(*variable->annotation().type);
			}
			catch (NoFormalType &err)
			{
				string const* typeNamePtr = boost::get_error_info<errinfo_noFormalTypeFrom>(err);
				string typeName = typeNamePtr ? " \"" + *typeNamePtr + "\"" : "";
				fatalError(*variable, "Type" + typeName + " not supported for state variable.");
			}
			addLine("mutable _" + variable->name() + ": " + varType);
		}
		unindent();
		addLine("}");
	}

	addLine("type account = {");
	indent();
	addLine("mutable balance: uint256;");
	addLine("storage: state");
	unindent();
	addLine("}");

	addLine("val external_call (this: account): bool");
	indent();
	addLine("ensures { result = false -> this = (old this) }");
	addLine("writes { this }");
	addSourceFromDocStrings(m_currentContract.contract->annotation());
	unindent();

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

void Why3Translator::endVisit(ContractDefinition const&)
{
	m_currentContract.reset();
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
	add(" (this: account)");
	for (auto const& param: _function.parameters())
	{
		string paramType;
		try
		{
			paramType = toFormalType(*param->annotation().type);
		}
		catch (NoFormalType &err)
		{
			string const* typeName = boost::get_error_info<errinfo_noFormalTypeFrom>(err);
			error(*param, "Parameter type \"" + (typeName ? *typeName : "") + "\" not supported.");
		}
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
		string paramType;
		try
		{
			paramType = toFormalType(*retParam->annotation().type);
		}
		catch (NoFormalType &err)
		{
			string const* typeName = boost::get_error_info<errinfo_noFormalTypeFrom>(err);
			error(*retParam, "Parameter type " + (typeName ? *typeName : "") + " not supported.");
		}
		if (retString.size() != 1)
			retString += ", ";
		retString += paramType;
	}
	add(retString + ")");
	unindent();

	addSourceFromDocStrings(_function.annotation());
	if (!m_currentContract.contract)
		error(_function, "Only functions inside contracts allowed.");
	addSourceFromDocStrings(m_currentContract.contract->annotation());

	if (_function.isDeclaredConst())
		addLine("ensures { (old this) = this }");
	else
		addLine("writes { this }");

	addLine("=");

	// store the prestate in the case we need to revert
	addLine("let prestate = {balance = this.balance; storage = " + copyOfStorage() + "} in ");

	// initialise local variables
	for (auto const& variable: _function.parameters())
		addLine("let _" + variable->name() + " = ref arg_" + variable->name() + " in");
	for (auto const& variable: _function.returnParameters())
	{
		if (variable->name().empty())
			error(*variable, "Unnamed return variables not yet supported.");
		string varType;
		try
		{
			varType = toFormalType(*variable->annotation().type);
		}
		catch (NoFormalType &err)
		{
			string const* typeNamePtr = boost::get_error_info<errinfo_noFormalTypeFrom>(err);
			error(*variable, "Type " + (typeNamePtr ? *typeNamePtr : "") + "in return parameter not yet supported.");
		}
		addLine("let _" + variable->name() + ": ref " + varType + " = ref (of_int 0) in");
	}
	for (VariableDeclaration const* variable: _function.localVariables())
	{
		if (variable->name().empty())
			error(*variable, "Unnamed variables not yet supported.");
		string varType;
		try
		{
			varType = toFormalType(*variable->annotation().type);
		}
		catch (NoFormalType &err)
		{
			string const* typeNamePtr = boost::get_error_info<errinfo_noFormalTypeFrom>(err);
			error(*variable, "Type " + (typeNamePtr ? *typeNamePtr : "") + "in variable declaration not yet supported.");
		}
		addLine("let _" + variable->name() + ": ref " + varType + " = ref (of_int 0) in");
	}
	addLine("try");

	_function.body().accept(*this);
	add(";");
	addLine("raise Return");

	string retVals;
	for (auto const& variable: _function.returnParameters())
	{
		if (!retVals.empty())
			retVals += ", ";
		retVals += "!_" + variable->name();
	}
	addLine("with Return -> (" + retVals + ") |");
	string reversion = "     Revert -> this.balance <- prestate.balance; ";
	for (auto const* variable: m_currentContract.stateVariables)
		reversion += "this.storage._" + variable->name() + " <- prestate.storage._" + variable->name() + "; ";
	//@TODO in case of reversion the return values are wrong - we need to change the
	// return type to include a bool to signify if an exception was thrown.
	reversion += "(" + retVals + ")";
	addLine(reversion);
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

	// Why3 does not appear to support do-while loops,
	// so we will simulate them by performing a while
	// loop with the body prepended once.

	if (_node.isDoWhile())
	{
		visitIndentedUnlessBlock(_node.body());
		newLine();
	}

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
		add("; raise Return end");
	}
	else
		add("raise Return");
	return false;
}

bool Why3Translator::visit(Throw const& _node)
{
	addSourceFromDocStrings(_node.annotation());
	add("raise Revert");
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

	add(m_currentLValueIsRef ? " := " : " <- ");
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
	try
	{
		toFormalType(*_unaryOperation.annotation().type);
	}
	catch (NoFormalType &err)
	{
		string const* typeNamePtr = boost::get_error_info<errinfo_noFormalTypeFrom>(err);
		error(_unaryOperation, "Type \"" + (typeNamePtr ? *typeNamePtr : "") + "\" supported in unary operation.");
	}

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

	if (commonType.category() == Type::Category::RationalNumber)
	{
		auto const& constantNumber = dynamic_cast<RationalNumberType const&>(commonType);
		if (constantNumber.isFractional())
			error(_binaryOperation, "Fractional numbers not supported.");
		else
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
	{
		error(_binaryOperation, "Operator not supported.");
		return true;
	}

	add("(");
	leftExpression.accept(*this);
	add(optrans.at(c_op));
	rightExpression.accept(*this);
	add(")");

	return false;
}

bool Why3Translator::visit(FunctionCall const& _node)
{
	if (_node.annotation().kind == FunctionCallKind::TypeConversion || _node.annotation().kind == FunctionCallKind::StructConstructorCall)
	{
		error(_node, "Only ordinary function calls supported.");
		return true;
	}
	FunctionType const& function = dynamic_cast<FunctionType const&>(*_node.expression().annotation().type);
	switch (function.kind())
	{
	case FunctionType::Kind::AddMod:
	case FunctionType::Kind::MulMod:
	{
		//@todo require that third parameter is not zero
		add("(of_int (mod (Int.(");
		add(function.kind() == FunctionType::Kind::AddMod ? "+" : "*");
		add(") (to_int ");
		_node.arguments().at(0)->accept(*this);
		add(") (to_int ");
		_node.arguments().at(1)->accept(*this);
		add(")) (to_int ");
		_node.arguments().at(2)->accept(*this);
		add(")))");
		return false;
	}
	case FunctionType::Kind::Internal:
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
	case FunctionType::Kind::Bare:
	{
		if (!_node.arguments().empty())
		{
			error(_node, "Function calls with named arguments not supported.");
			return true;
		}

		add("(");
		indent();
		add("let amount = 0 in ");
		_node.expression().accept(*this);
		addLine("if amount <= this.balance then");
		indent();
		addLine("let balance_precall = this.balance in");
		addLine("begin");
		indent();
		addLine("this.balance <- this.balance - amount;");
		addLine("if not (external_call this) then begin this.balance = balance_precall; false end else true");
		unindent();
		addLine("end");
		unindent();
		addLine("else false");

		unindent();
		add(")");
		return false;
	}
	case FunctionType::Kind::SetValue:
	{
		add("let amount = ");
		solAssert(_node.arguments().size() == 1, "");
		_node.arguments()[0]->accept(*this);
		add(" in ");
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
	else if (
		_node.memberName() == "call" &&
		*_node.expression().annotation().type == IntegerType(160, IntegerType::Modifier::Address)
	)
	{
		// Do nothing, do not even visit the address because this will be an external call
		//@TODO ensure that the expression itself does not have side-effects
		return false;
	}
	else
		error(_node, "Member access: Only call and array length supported.");
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
		if (isStateVar)
			add("this.storage.");
		else if (!lvalue)
			add("!(");
		add("_" + variable->name());
		if (!isStateVar && !lvalue)
			add(")");
		m_currentLValueIsRef = !isStateVar;
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
	case Type::Category::RationalNumber:
	{
		auto const& constantNumber = dynamic_cast<RationalNumberType const&>(*type);
		if (constantNumber.isFractional())
			error(_literal, "Fractional numbers not supported.");
		else
			add("(of_int " + toString(type->literalValue(&_literal)) + ")");
		break;
	}
	default:
		error(_literal, "Not supported.");
	}
	return false;
}

bool Why3Translator::visit(PragmaDirective const& _pragma)
{
	if (_pragma.tokens().empty())
		error(_pragma, "Not supported");
	else if (_pragma.literals().empty())
		error(_pragma, "Not supported");
	else if (_pragma.literals()[0] != "solidity")
		error(_pragma, "Not supported");
	else if (_pragma.tokens()[0] != Token::Identifier)
		error(_pragma, "A literal 'solidity' is not an identifier.  Strange");

	return false;
}

bool Why3Translator::isStateVariable(VariableDeclaration const* _var) const
{
	return contains(m_currentContract.stateVariables, _var);
}

bool Why3Translator::isStateVariable(string const& _name) const
{
	for (auto const& var: m_currentContract.stateVariables)
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

string Why3Translator::copyOfStorage() const
{
	if (m_currentContract.stateVariables.empty())
		return "()";
	string ret = "{";
	bool first = true;
	for (auto const* variable: m_currentContract.stateVariables)
	{
		if (first)
			first = false;
		else
			ret += "; ";
		ret += "_" + variable->name() + " = this.storage._" + variable->name();
	}
	return ret + "}";
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
			ret += "(!_" + varName + ")";
		else if (isStateVariable(varName))
			ret += "(this.storage._" + varName + ")";
		//@todo return variables
		else
			ret.append(hash, hashEnd);

		pos = hashEnd;
	}
	return ret;
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

module Address
	use import mach.int.Unsigned
	type address
	constant max_address: int = 0xffffffffffffffffffffffffffffffffffffffff (* 160 bit = 40 f's *)
	clone export mach.int.Unsigned with
		type t = address,
		constant max = max_address
end
   )", 0});
}
