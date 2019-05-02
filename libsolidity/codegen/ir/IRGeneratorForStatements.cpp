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

#include <libsolidity/codegen/ir/IRGenerationContext.h>
#include <libsolidity/codegen/ir/IRLValue.h>
#include <libsolidity/codegen/YulUtilFunctions.h>
#include <libsolidity/ast/TypeProvider.h>

#include <libyul/AsmPrinter.h>
#include <libyul/AsmData.h>
#include <libyul/optimiser/ASTCopier.h>

#include <libdevcore/StringUtils.h>

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
	Type const* intermediateType = _assignment.rightHandSide().annotation().type->closestTemporaryType(
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

bool IRGeneratorForStatements::visit(ForStatement const& _for)
{
	m_code << "for {\n";
	if (_for.initializationExpression())
		_for.initializationExpression()->accept(*this);
	m_code << "} return_flag {\n";
	if (_for.loopExpression())
		_for.loopExpression()->accept(*this);
	m_code << "}\n";
	if (_for.condition())
	{
		_for.condition()->accept(*this);
		m_code <<
			"if iszero(" <<
			expressionAsType(*_for.condition(), *TypeProvider::boolean()) <<
			") { break }\n";
	}
	_for.body().accept(*this);
	m_code << "}\n";
	// Bubble up the return condition.
	m_code << "if iszero(return_flag) { break }\n";
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

void IRGeneratorForStatements::endVisit(BinaryOperation const& _binOp)
{
	solAssert(!!_binOp.annotation().commonType, "");
	TypePointer commonType = _binOp.annotation().commonType;

	if (_binOp.getOperator() == Token::And || _binOp.getOperator() == Token::Or)
		// special case: short-circuiting
		solUnimplementedAssert(false, "");
	else if (commonType->category() == Type::Category::RationalNumber)
		defineExpression(_binOp) <<
			toCompactHexWithPrefix(commonType->literalValue(nullptr)) <<
			"\n";
	else
	{
		solUnimplementedAssert(_binOp.getOperator() == Token::Add, "");
		if (IntegerType const* type = dynamic_cast<IntegerType const*>(commonType))
		{
			solUnimplementedAssert(!type->isSigned(), "");
			defineExpression(_binOp) <<
				m_utils.overflowCheckedUIntAddFunction(type->numBits()) <<
				"(" <<
				expressionAsType(_binOp.leftExpression(), *commonType) <<
				", " <<
				expressionAsType(_binOp.rightExpression(), *commonType) <<
				")\n";
		}
		else
			solUnimplementedAssert(false, "");
	}
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
	default:
		solUnimplemented("");
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

bool IRGeneratorForStatements::visit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
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
	else
		solUnimplemented("");
	return false;
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

void IRGeneratorForStatements::setLValue(Expression const& _expression, unique_ptr<IRLValue> _lvalue)
{
	solAssert(!m_currentLValue, "");

	if (_expression.annotation().lValueRequested)
		// Do not define the expression, so it cannot be used as value.
		m_currentLValue = std::move(_lvalue);
	else
		defineExpression(_expression) << _lvalue->retrieveValue() << "\n";
}

Type const& IRGeneratorForStatements::type(Expression const& _expression)
{
	solAssert(_expression.annotation().type, "Type of expression not set.");
	return *_expression.annotation().type;
}
