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
 * Component that translates Solidity code into Yul at statement level and below.
 */

#include <libsolidity/codegen/ir/IRGeneratorForStatements.h>

#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/ir/IRGenerationContext.h>
#include <libsolidity/codegen/ir/IRLValue.h>
#include <libsolidity/codegen/YulUtilFunctions.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libyul/AsmPrinter.h>
#include <libyul/AsmData.h>
#include <libyul/optimiser/ASTCopier.h>

#include <libdevcore/Whiskers.h>
#include <libdevcore/StringUtils.h>
#include <libdevcore/Whiskers.h>
#include <libdevcore/Keccak256.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

namespace
{

struct CopyTranslate: public yul::ASTCopier
{
	using ExternalRefsMap = std::map<yul::Identifier const*, InlineAssemblyAnnotation::ExternalIdentifierInfo>;

	CopyTranslate(IRGenerationContext& _context, ExternalRefsMap const& _references):
		m_context(_context), m_references(_references) {}

	using ASTCopier::operator();

	yul::YulString translateIdentifier(yul::YulString _name) override
	{
		return yul::YulString{"usr$" + _name.str()};
	}

	yul::Identifier translate(yul::Identifier const& _identifier) override
	{
		if (!m_references.count(&_identifier))
			return ASTCopier::translate(_identifier);

		auto const& reference = m_references.at(&_identifier);
		auto const varDecl = dynamic_cast<VariableDeclaration const*>(reference.declaration);
		solUnimplementedAssert(varDecl, "");
		solUnimplementedAssert(
			reference.isOffset == false && reference.isSlot == false,
			""
		);

		return yul::Identifier{
			_identifier.location,
			yul::YulString{m_context.localVariableName(*varDecl)}
		};
	}

private:
	IRGenerationContext& m_context;
	ExternalRefsMap const& m_references;
};

}



string IRGeneratorForStatements::code() const
{
	solAssert(!m_currentLValue, "LValue not reset!");
	return m_code.str();
}

void IRGeneratorForStatements::endVisit(VariableDeclarationStatement const& _varDeclStatement)
{
	for (auto const& decl: _varDeclStatement.declarations())
		if (decl)
			m_context.addLocalVariable(*decl);

	if (Expression const* expression = _varDeclStatement.initialValue())
	{
		solUnimplementedAssert(_varDeclStatement.declarations().size() == 1, "");

		VariableDeclaration const& varDecl = *_varDeclStatement.declarations().front();
		m_code <<
			"let " <<
			m_context.localVariableName(varDecl) <<
			" := " <<
			expressionAsType(*expression, *varDecl.type()) <<
			"\n";
	}
	else
		for (auto const& decl: _varDeclStatement.declarations())
			if (decl)
				m_code << "let " << m_context.localVariableName(*decl) << "\n";
}

bool IRGeneratorForStatements::visit(Assignment const& _assignment)
{
	solUnimplementedAssert(_assignment.assignmentOperator() == Token::Assign, "");

	_assignment.rightHandSide().accept(*this);
	Type const* intermediateType = type(_assignment.rightHandSide()).closestTemporaryType(
		&type(_assignment.leftHandSide())
	);
	string intermediateValue = m_context.newYulVariable();
	m_code << "let " << intermediateValue << " := " << expressionAsType(_assignment.rightHandSide(), *intermediateType) << "\n";

	_assignment.leftHandSide().accept(*this);
	solAssert(!!m_currentLValue, "LValue not retrieved.");
	m_currentLValue->storeValue(intermediateValue, *intermediateType);
	m_currentLValue.reset();

	defineExpression(_assignment) << intermediateValue << "\n";

	return false;
}

bool IRGeneratorForStatements::visit(TupleExpression const& _tuple)
{
	if (_tuple.isInlineArray())
		solUnimplementedAssert(false, "");
	else
	{
		solUnimplementedAssert(!_tuple.annotation().lValueRequested, "");
		solUnimplementedAssert(_tuple.components().size() == 1, "");
		solAssert(_tuple.components().front(), "");
		_tuple.components().front()->accept(*this);
		defineExpression(_tuple) << m_context.variable(*_tuple.components().front()) << "\n";
	}
	return false;
}

bool IRGeneratorForStatements::visit(IfStatement const& _ifStatement)
{
	_ifStatement.condition().accept(*this);
	string condition = expressionAsType(_ifStatement.condition(), *TypeProvider::boolean());

	if (_ifStatement.falseStatement())
	{
		m_code << "switch " << condition << "\n" "case 0 {\n";
		_ifStatement.falseStatement()->accept(*this);
		m_code << "}\n" "default {\n";
	}
	else
		m_code << "if " << condition << " {\n";
	_ifStatement.trueStatement().accept(*this);
	m_code << "}\n";

	return false;
}

bool IRGeneratorForStatements::visit(ForStatement const& _forStatement)
{
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
	generateLoop(
		_whileStatement.body(),
		&_whileStatement.condition(),
		nullptr,
		nullptr,
		_whileStatement.isDoWhile()
	);

	return false;
}

bool IRGeneratorForStatements::visit(Continue const&)
{
	m_code << "continue\n";
	return false;
}

bool IRGeneratorForStatements::visit(Break const&)
{
	m_code << "break\n";
	return false;
}

void IRGeneratorForStatements::endVisit(Return const& _return)
{
	if (Expression const* value = _return.expression())
	{
		solAssert(_return.annotation().functionReturnParameters, "Invalid return parameters pointer.");
		vector<ASTPointer<VariableDeclaration>> const& returnParameters =
			_return.annotation().functionReturnParameters->parameters();
		TypePointers types;
		for (auto const& retVariable: returnParameters)
			types.push_back(retVariable->annotation().type);

		// TODO support tuples
		solUnimplementedAssert(types.size() == 1, "Multi-returns not implemented.");
		m_code <<
			m_context.localVariableName(*returnParameters.front()) <<
			" := " <<
			expressionAsType(*value, *types.front()) <<
			"\n";
	}
	m_code << "return_flag := 0\n" << "break\n";
}

void IRGeneratorForStatements::endVisit(UnaryOperation const& _unaryOperation)
{
	if (type(_unaryOperation).category() == Type::Category::RationalNumber)
		defineExpression(_unaryOperation) <<
			formatNumber(type(_unaryOperation).literalValue(nullptr)) <<
			"\n";
	else
		solUnimplementedAssert(false, "");
}

bool IRGeneratorForStatements::visit(BinaryOperation const& _binOp)
{
	solAssert(!!_binOp.annotation().commonType, "");
	TypePointer commonType = _binOp.annotation().commonType;
	langutil::Token op = _binOp.getOperator();

	if (op == Token::And || op == Token::Or)
	{
		// This can short-circuit!
		appendAndOrOperatorCode(_binOp);
		return false;
	}

	_binOp.leftExpression().accept(*this);
	_binOp.rightExpression().accept(*this);

	if (commonType->category() == Type::Category::RationalNumber)
		defineExpression(_binOp) <<
			toCompactHexWithPrefix(commonType->literalValue(nullptr)) <<
			"\n";
	else if (TokenTraits::isCompareOp(op))
	{
		solUnimplementedAssert(commonType->category() != Type::Category::Function, "");
		solAssert(commonType->isValueType(), "");
		bool isSigned = false;
		if (auto type = dynamic_cast<IntegerType const*>(commonType))
			isSigned = type->isSigned();

		string args =
			expressionAsType(_binOp.leftExpression(), *commonType) +
			", " +
			expressionAsType(_binOp.rightExpression(), *commonType);

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
		defineExpression(_binOp) << expr << "\n";
	}
	else
	{
		if (IntegerType const* type = dynamic_cast<IntegerType const*>(commonType))
		{
			solUnimplementedAssert(!type->isSigned(), "");
			string left = expressionAsType(_binOp.leftExpression(), *commonType);
			string right = expressionAsType(_binOp.rightExpression(), *commonType);
			string fun;
			if (_binOp.getOperator() == Token::Add)
				fun = m_utils.overflowCheckedUIntAddFunction(type->numBits());
			else if (_binOp.getOperator() == Token::Sub)
				fun = m_utils.overflowCheckedUIntSubFunction();
			else
				solUnimplementedAssert(false, "");
			defineExpression(_binOp) << fun << "(" << left << ", " << right << ")\n";
		}
		else
			solUnimplementedAssert(false, "");
	}
	return false;
}

void IRGeneratorForStatements::endVisit(FunctionCall const& _functionCall)
{
	solUnimplementedAssert(
		_functionCall.annotation().kind == FunctionCallKind::FunctionCall ||
		_functionCall.annotation().kind == FunctionCallKind::TypeConversion,
		"This type of function call is not yet implemented"
	);

	Type const& funcType = type(_functionCall.expression());

	if (_functionCall.annotation().kind == FunctionCallKind::TypeConversion)
	{
		solAssert(funcType.category() == Type::Category::TypeType, "Expected category to be TypeType");
		solAssert(_functionCall.arguments().size() == 1, "Expected one argument for type conversion");

		defineExpression(_functionCall) <<
			expressionAsType(*_functionCall.arguments().front(), type(_functionCall)) <<
			"\n";

		return;
	}

	FunctionTypePointer functionType = dynamic_cast<FunctionType const*>(&funcType);

	TypePointers parameterTypes = functionType->parameterTypes();
	vector<ASTPointer<Expression const>> const& callArguments = _functionCall.arguments();
	vector<ASTPointer<ASTString>> const& callArgumentNames = _functionCall.names();
	if (!functionType->takesArbitraryParameters())
		solAssert(callArguments.size() == parameterTypes.size(), "");

	vector<ASTPointer<Expression const>> arguments;
	if (callArgumentNames.empty())
		// normal arguments
		arguments = callArguments;
	else
		// named arguments
		for (auto const& parameterName: functionType->parameterNames())
		{
			auto const it = std::find_if(callArgumentNames.cbegin(), callArgumentNames.cend(), [&](ASTPointer<ASTString> const& _argName) {
				return *_argName == parameterName;
			});

			solAssert(it != callArgumentNames.cend(), "");
			arguments.push_back(callArguments[std::distance(callArgumentNames.begin(), it)]);
		}

	solUnimplementedAssert(!functionType->bound(), "");
	switch (functionType->kind())
	{
	case FunctionType::Kind::Internal:
	{
		vector<string> args;
		for (unsigned i = 0; i < arguments.size(); ++i)
			if (functionType->takesArbitraryParameters())
				args.emplace_back(m_context.variable(*arguments[i]));
			else
				args.emplace_back(expressionAsType(*arguments[i], *parameterTypes[i]));

		if (auto identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
		{
			solAssert(!functionType->bound(), "");
			if (auto functionDef = dynamic_cast<FunctionDefinition const*>(identifier->annotation().referencedDeclaration))
			{
				// @TODO The function can very well return multiple vars.
				defineExpression(_functionCall) <<
					m_context.virtualFunctionName(*functionDef) <<
					"(" <<
					joinHumanReadable(args) <<
					")\n";
				return;
			}
		}

		// @TODO The function can very well return multiple vars.
		args = vector<string>{m_context.variable(_functionCall.expression())} + args;
		defineExpression(_functionCall) <<
			m_context.internalDispatch(functionType->parameterTypes().size(), functionType->returnParameterTypes().size()) <<
			"(" <<
			joinHumanReadable(args) <<
			")\n";
		break;
	}
	case FunctionType::Kind::Event:
	{
		auto const& event = dynamic_cast<EventDefinition const&>(functionType->declaration());
		TypePointers paramTypes = functionType->parameterTypes();
		ABIFunctions abi(m_context.evmVersion(), m_context.functionCollector());

		vector<string> indexedArgs;
		string nonIndexedArgs;
		TypePointers nonIndexedArgTypes;
		TypePointers nonIndexedParamTypes;
		if (!event.isAnonymous())
		{
			indexedArgs.emplace_back(m_context.newYulVariable());
			string signature = formatNumber(u256(h256::Arith(dev::keccak256(functionType->externalSignature()))));
			m_code << "let " << indexedArgs.back() << " := " << signature << "\n";
		}
		for (size_t i = 0; i < event.parameters().size(); ++i)
		{
			Expression const& arg = *arguments[i];
			if (event.parameters()[i]->isIndexed())
			{
				string value;
				indexedArgs.emplace_back(m_context.newYulVariable());
				if (auto const& referenceType = dynamic_cast<ReferenceType const*>(paramTypes[i]))
					value =
						m_utils.packedHashFunction({arg.annotation().type}, {referenceType}) +
						"(" +
						m_context.variable(arg) +
						")";
				else
					value = expressionAsType(arg, *paramTypes[i]);
				m_code << "let " << indexedArgs.back() << " := " << value << "\n";
			}
			else
			{
				string vars = m_context.variable(arg);
				if (!vars.empty())
					// In reverse because abi_encode expects it like that.
					nonIndexedArgs = ", " + move(vars) + nonIndexedArgs;
				nonIndexedArgTypes.push_back(arg.annotation().type);
				nonIndexedParamTypes.push_back(paramTypes[i]);
			}
		}
		solAssert(indexedArgs.size() <= 4, "Too many indexed arguments.");
		Whiskers templ(R"({
			let <pos> := mload(<freeMemoryPointer>)
			let <end> := <encode>(<pos> <nonIndexedArgs>)
			<log>(<pos>, sub(<end>, <pos>) <indexedArgs>)
		})");
		templ("pos", m_context.newYulVariable());
		templ("end", m_context.newYulVariable());
		templ("freeMemoryPointer", to_string(CompilerUtils::freeMemoryPointer));
		templ("encode", abi.tupleEncoder(nonIndexedArgTypes, nonIndexedParamTypes));
		templ("nonIndexedArgs", nonIndexedArgs);
		templ("log", "log" + to_string(indexedArgs.size()));
		templ("indexedArgs", joinHumanReadablePrefixed(indexedArgs));
		m_code << templ.render();
		break;
	}
	case FunctionType::Kind::Assert:
	case FunctionType::Kind::Require:
	{
		solAssert(arguments.size() > 0, "Expected at least one parameter for require/assert");
		solAssert(arguments.size() <= 2, "Expected no more than two parameters for require/assert");

		string requireOrAssertFunction = m_utils.requireOrAssertFunction(
			functionType->kind() == FunctionType::Kind::Assert,
			arguments.size() > 1 ? arguments[1]->annotation().type : nullptr
		);

		m_code << move(requireOrAssertFunction) << "(" << m_context.variable(*arguments[0]);
		if (arguments.size() > 1)
			m_code << ", " << m_context.variable(*arguments[1]);
		m_code << ")\n";

		break;
	}
	default:
		solUnimplemented("");
	}
}

void IRGeneratorForStatements::endVisit(MemberAccess const& _memberAccess)
{
	ASTString const& member = _memberAccess.memberName();
	if (auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type))
		if (funType->bound())
		{
			solUnimplementedAssert(false, "");
		}

	switch (_memberAccess.expression().annotation().type->category())
	{
	case Type::Category::Contract:
	{
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.expression().annotation().type);
		if (type.isSuper())
		{
			solUnimplementedAssert(false, "");
		}
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
			// TODO here, we need to assign address and function identifier to two variables.
			// We migt also just combine them into a single variable already....
			solUnimplementedAssert(false, "");
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
			defineExpression(_memberAccess) <<
				"balance(" <<
				expressionAsType(_memberAccess.expression(), *TypeProvider::address()) <<
				")\n";
		else if (set<string>{"send", "transfer"}.count(member))
		{
			solAssert(dynamic_cast<AddressType const&>(*_memberAccess.expression().annotation().type).stateMutability() == StateMutability::Payable, "");
			defineExpression(_memberAccess) <<
				expressionAsType(_memberAccess.expression(), *TypeProvider::payableAddress()) <<
				"\n";
		}
		else if (set<string>{"call", "callcode", "delegatecall", "staticcall"}.count(member))
			defineExpression(_memberAccess) <<
				expressionAsType(_memberAccess.expression(), *TypeProvider::address()) <<
				"\n";
		else
			solAssert(false, "Invalid member access to address");
		break;
	}
	case Type::Category::Function:
		if (member == "selector")
		{
			solUnimplementedAssert(false, "");
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
			defineExpression(_memberAccess) << "coinbase()\n";
		else if (member == "timestamp")
			defineExpression(_memberAccess) << "timestamp()\n";
		else if (member == "difficulty")
			defineExpression(_memberAccess) << "difficulty()\n";
		else if (member == "number")
			defineExpression(_memberAccess) << "number()\n";
		else if (member == "gaslimit")
			defineExpression(_memberAccess) << "gaslimit()\n";
		else if (member == "sender")
			defineExpression(_memberAccess) << "caller()\n";
		else if (member == "value")
			defineExpression(_memberAccess) << "callvalue()\n";
		else if (member == "origin")
			defineExpression(_memberAccess) << "origin()\n";
		else if (member == "gasprice")
			defineExpression(_memberAccess) << "gasprice()\n";
		else if (member == "data")
			solUnimplementedAssert(false, "");
		else if (member == "sig")
			defineExpression(_memberAccess) <<
				"and(calldataload(0), " <<
				formatNumber(u256(0xffffffff) << (256 - 32)) <<
				")\n";
		else if (member == "gas")
			solAssert(false, "Gas has been removed.");
		else if (member == "blockhash")
			solAssert(false, "Blockhash has been removed.");
		else if (member == "creationCode" || member == "runtimeCode")
		{
			solUnimplementedAssert(false, "");
		}
		else if (member == "name")
		{
			solUnimplementedAssert(false, "");
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
		solUnimplementedAssert(false, "");
	}
	case Type::Category::Enum:
	{
		EnumType const& type = dynamic_cast<EnumType const&>(*_memberAccess.expression().annotation().type);
		defineExpression(_memberAccess) << to_string(type.memberValue(_memberAccess.memberName())) << "\n";
		break;
	}
	case Type::Category::Array:
	{
		solUnimplementedAssert(false, "");
	}
	case Type::Category::FixedBytes:
	{
		auto const& type = dynamic_cast<FixedBytesType const&>(*_memberAccess.expression().annotation().type);
		if (member == "length")
			defineExpression(_memberAccess) << to_string(type.numBytes());
		else
			solAssert(false, "Illegal fixed bytes member.");
		break;
	}
	default:
		solAssert(false, "Member access to unknown type.");
	}
}

bool IRGeneratorForStatements::visit(InlineAssembly const& _inlineAsm)
{
	CopyTranslate bodyCopier{m_context, _inlineAsm.annotation().externalReferences};

	yul::Statement modified = bodyCopier(_inlineAsm.operations());

	solAssert(modified.type() == typeid(yul::Block), "");

	m_code << yul::AsmPrinter()(boost::get<yul::Block>(std::move(modified))) << "\n";
	return false;
}


void IRGeneratorForStatements::endVisit(IndexAccess const& _indexAccess)
{
	Type const& baseType = *_indexAccess.baseExpression().annotation().type;

	if (baseType.category() == Type::Category::Mapping)
	{
		solAssert(_indexAccess.indexExpression(), "Index expression expected.");

		MappingType const& mappingType = dynamic_cast<MappingType const&>(baseType);
		Type const& keyType = *_indexAccess.indexExpression()->annotation().type;
		solAssert(keyType.sizeOnStack() <= 1, "");

		string slot = m_context.newYulVariable();
		Whiskers templ("let <slot> := <indexAccess>(<base> <key>)\n");
		templ("slot", slot);
		templ("indexAccess", m_utils.mappingIndexAccessFunction(mappingType, keyType));
		templ("base", m_context.variable(_indexAccess.baseExpression()));
		if (keyType.sizeOnStack() == 0)
			templ("key", "");
		else
			templ("key", ", " + m_context.variable(*_indexAccess.indexExpression()));
		m_code << templ.render();
		setLValue(_indexAccess, make_unique<IRStorageItem>(
			m_code,
			m_context,
			slot,
			0,
			*_indexAccess.annotation().type
		));
	}
	else if (baseType.category() == Type::Category::Array)
		solUnimplementedAssert(false, "");
	else if (baseType.category() == Type::Category::FixedBytes)
		solUnimplementedAssert(false, "");
	else if (baseType.category() == Type::Category::TypeType)
	{
		solAssert(baseType.sizeOnStack() == 0, "");
		solAssert(_indexAccess.annotation().type->sizeOnStack() == 0, "");
		// no-op - this seems to be a lone array type (`structType[];`)
	}
	else
		solAssert(false, "Index access only allowed for mappings or arrays.");
}

void IRGeneratorForStatements::endVisit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		switch (magicVar->type()->category())
		{
		case Type::Category::Contract:
			if (dynamic_cast<ContractType const&>(*magicVar->type()).isSuper())
				solAssert(_identifier.name() == "super", "");
			else
			{
				solAssert(_identifier.name() == "this", "");
				defineExpression(_identifier) << "address()\n";
			}
			break;
		case Type::Category::Integer:
			solAssert(_identifier.name() == "now", "");
			defineExpression(_identifier) << "timestamp()\n";
			break;
		default:
			break;
		}
		return;
	}
	else if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
		defineExpression(_identifier) << to_string(m_context.virtualFunction(*functionDef).id()) << "\n";
	else if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
	{
		// TODO for the constant case, we have to be careful:
		// If the value is visited twice, `defineExpression` is called twice on
		// the same expression.
		solUnimplementedAssert(!varDecl->isConstant(), "");
		unique_ptr<IRLValue> lvalue;
		if (m_context.isLocalVariable(*varDecl))
			lvalue = make_unique<IRLocalVariable>(m_code, m_context, *varDecl);
		else if (m_context.isStateVariable(*varDecl))
			lvalue = make_unique<IRStorageItem>(m_code, m_context, *varDecl);
		else
			solAssert(false, "Invalid variable kind.");

		setLValue(_identifier, move(lvalue));
	}
	else if (auto contract = dynamic_cast<ContractDefinition const*>(declaration))
	{
		solUnimplementedAssert(!contract->isLibrary(), "Libraries not yet supported.");
	}
	else if (dynamic_cast<EventDefinition const*>(declaration))
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
	else
	{
		solAssert(false, "Identifier type not expected in expression context.");
	}
}

bool IRGeneratorForStatements::visit(Literal const& _literal)
{
	Type const& literalType = type(_literal);

	switch (literalType.category())
	{
	case Type::Category::RationalNumber:
	case Type::Category::Bool:
	case Type::Category::Address:
		defineExpression(_literal) << toCompactHexWithPrefix(literalType.literalValue(&_literal)) << "\n";
		break;
	case Type::Category::StringLiteral:
		break; // will be done during conversion
	default:
		solUnimplemented("Only integer, boolean and string literals implemented for now.");
	}
	return false;
}

string IRGeneratorForStatements::expressionAsType(Expression const& _expression, Type const& _to)
{
	Type const& from = type(_expression);
	if (from.sizeOnStack() == 0)
	{
		solAssert(from != _to, "");
		return m_utils.conversionFunction(from, _to) + "()";
	}
	else
	{
		string varName = m_context.variable(_expression);

		if (from == _to)
			return varName;
		else
			return m_utils.conversionFunction(from, _to) + "(" + std::move(varName) + ")";
	}
}

ostream& IRGeneratorForStatements::defineExpression(Expression const& _expression)
{
	return m_code << "let " << m_context.variable(_expression) << " := ";
}

void IRGeneratorForStatements::appendAndOrOperatorCode(BinaryOperation const& _binOp)
{
	langutil::Token const op = _binOp.getOperator();
	solAssert(op == Token::Or || op == Token::And, "");

	_binOp.leftExpression().accept(*this);

	string value = m_context.variable(_binOp);
	m_code << "let " << value << " := " << m_context.variable(_binOp.leftExpression()) << "\n";
	if (op == Token::Or)
		m_code << "if iszero(" << value << ") {\n";
	else
		m_code << "if " << value << " {\n";
	_binOp.rightExpression().accept(*this);
	m_code << value << " := " + m_context.variable(_binOp.rightExpression()) << "\n";
	m_code << "}\n";
}

void IRGeneratorForStatements::setLValue(Expression const& _expression, unique_ptr<IRLValue> _lvalue)
{
	solAssert(!m_currentLValue, "");

	if (_expression.annotation().lValueRequested)
		// Do not define the expression, so it cannot be used as value.
		m_currentLValue = std::move(_lvalue);
	else
		defineExpression(_expression) << _lvalue->retrieveValue() << "\n";
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
		m_code << "let " << firstRun << " := 1\n";
	}

	m_code << "for {\n";
	if (_initExpression)
		_initExpression->accept(*this);
	m_code << "} return_flag {\n";
	if (_loopExpression)
		_loopExpression->accept(*this);
	m_code << "}\n";
	m_code << "{\n";

	if (_conditionExpression)
	{
		if (_isDoWhile)
			m_code << "if iszero(" << firstRun << ") {\n";

		_conditionExpression->accept(*this);
		m_code <<
			"if iszero(" <<
			expressionAsType(*_conditionExpression, *TypeProvider::boolean()) <<
			") { break }\n";

		if (_isDoWhile)
			m_code << "}\n" << firstRun << " := 0\n";
	}

	_body.accept(*this);

	m_code << "}\n";
	// Bubble up the return condition.
	m_code << "if iszero(return_flag) { break }\n";
}

Type const& IRGeneratorForStatements::type(Expression const& _expression)
{
	solAssert(_expression.annotation().type, "Type of expression not set.");
	return *_expression.annotation().type;
}
