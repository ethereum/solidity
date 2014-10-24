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
 * @date 2014
 * Solidity abstract syntax tree.
 */

#include <algorithm>

#include <libsolidity/AST.h>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/Exceptions.h>

namespace dev
{
namespace solidity
{

void ContractDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		listAccept(m_definedStructs, _visitor);
		listAccept(m_stateVariables, _visitor);
		listAccept(m_definedFunctions, _visitor);
	}
	_visitor.endVisit(*this);
}

void StructDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		listAccept(m_members, _visitor);
	_visitor.endVisit(*this);
}

void ParameterList::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		listAccept(m_parameters, _visitor);
	_visitor.endVisit(*this);
}

void FunctionDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_parameters->accept(_visitor);
		if (m_returnParameters)
			m_returnParameters->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void VariableDeclaration::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		if (m_typeName)
			m_typeName->accept(_visitor);
	_visitor.endVisit(*this);
}

void TypeName::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ElementaryTypeName::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void UserDefinedTypeName::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Mapping::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_keyType->accept(_visitor);
		m_valueType->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Statement::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Block::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		listAccept(m_statements, _visitor);
	_visitor.endVisit(*this);
}

void IfStatement::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_condition->accept(_visitor);
		m_trueBody->accept(_visitor);
		if (m_falseBody)
			m_falseBody->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void BreakableStatement::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void WhileStatement::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_condition->accept(_visitor);
		m_body->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Continue::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Break::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Return::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		if (m_expression)
			m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void VariableDefinition::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_variable->accept(_visitor);
		if (m_value)
			m_value->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Assignment::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_leftHandSide->accept(_visitor);
		m_rightHandSide->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void UnaryOperation::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		m_subExpression->accept(_visitor);
	_visitor.endVisit(*this);
}

void BinaryOperation::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_left->accept(_visitor);
		m_right->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void FunctionCall::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_expression->accept(_visitor);
		listAccept(m_arguments, _visitor);
	}
	_visitor.endVisit(*this);
}

void MemberAccess::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
		m_expression->accept(_visitor);
	_visitor.endVisit(*this);
}

void IndexAccess::accept(ASTVisitor& _visitor)
{
	if (_visitor.visit(*this))
	{
		m_base->accept(_visitor);
		m_index->accept(_visitor);
	}
	_visitor.endVisit(*this);
}

void Identifier::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void ElementaryTypeNameExpression::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

void Literal::accept(ASTVisitor& _visitor)
{
	_visitor.visit(*this);
	_visitor.endVisit(*this);
}

TypeError ASTNode::createTypeError(std::string const& _description)
{
	return TypeError() << errinfo_sourceLocation(getLocation()) << errinfo_comment(_description);
}

void Statement::expectType(Expression& _expression, const Type& _expectedType)
{
	_expression.checkTypeRequirements();
	if (!_expression.getType()->isImplicitlyConvertibleTo(_expectedType))
		BOOST_THROW_EXCEPTION(_expression.createTypeError("Type not implicitly convertible to expected type."));
	//@todo provide more information to the exception
}

void Block::checkTypeRequirements()
{
	for (std::shared_ptr<Statement> const& statement: m_statements)
		statement->checkTypeRequirements();
}

void IfStatement::checkTypeRequirements()
{
	expectType(*m_condition, BoolType());
	m_trueBody->checkTypeRequirements();
	if (m_falseBody)
		m_falseBody->checkTypeRequirements();
}

void WhileStatement::checkTypeRequirements()
{
	expectType(*m_condition, BoolType());
	m_body->checkTypeRequirements();
}

void Continue::checkTypeRequirements()
{
}

void Break::checkTypeRequirements()
{
}

void Return::checkTypeRequirements()
{
	BOOST_ASSERT(m_returnParameters != nullptr);
	if (m_returnParameters->getParameters().size() != 1)
		BOOST_THROW_EXCEPTION(createTypeError("Different number of arguments in return statement "
											  "than in returns declaration."));
	// this could later be changed such that the paramaters type is an anonymous struct type,
	// but for now, we only allow one return parameter
	expectType(*m_expression, *m_returnParameters->getParameters().front()->getType());
}

void VariableDefinition::checkTypeRequirements()
{
	// Variables can be declared without type (with "var"), in which case the first assignment
	// setsthe type.
	// Note that assignments before the first declaration are legal because of the special scoping
	// rules inherited from JavaScript.
	if (m_value)
	{
		if (m_variable->getType())
			expectType(*m_value, *m_variable->getType());
		else
		{
			// no type declared and no previous assignment, infer the type
			m_value->checkTypeRequirements();
			m_variable->setType(m_value->getType());
		}
	}
}

void Assignment::checkTypeRequirements()
{
	//@todo lefthandside actually has to be assignable
	// add a feature to the type system to check that
	m_leftHandSide->checkTypeRequirements();
	expectType(*m_rightHandSide, *m_leftHandSide->getType());
	m_type = m_leftHandSide->getType();
	if (m_assigmentOperator != Token::ASSIGN)
	{
		// complex assignment
		if (!m_type->acceptsBinaryOperator(Token::AssignmentToBinaryOp(m_assigmentOperator)))
			BOOST_THROW_EXCEPTION(createTypeError("Operator not compatible with type."));
	}
}

void UnaryOperation::checkTypeRequirements()
{
	// INC, DEC, NOT, BIT_NOT, DELETE
	m_subExpression->checkTypeRequirements();
	m_type = m_subExpression->getType();
	if (m_type->acceptsUnaryOperator(m_operator))
		BOOST_THROW_EXCEPTION(createTypeError("Unary operator not compatible with type."));
}

void BinaryOperation::checkTypeRequirements()
{
	m_right->checkTypeRequirements();
	m_left->checkTypeRequirements();
	if (m_right->getType()->isImplicitlyConvertibleTo(*m_left->getType()))
		m_commonType = m_left->getType();
	else if (m_left->getType()->isImplicitlyConvertibleTo(*m_right->getType()))
		m_commonType = m_right->getType();
	else
		BOOST_THROW_EXCEPTION(createTypeError("No common type found in binary operation."));
	if (Token::isCompareOp(m_operator))
		m_type = std::make_shared<BoolType>();
	else
	{
		BOOST_ASSERT(Token::isBinaryOp(m_operator));
		m_type = m_commonType;
		if (!m_commonType->acceptsBinaryOperator(m_operator))
			BOOST_THROW_EXCEPTION(createTypeError("Operator not compatible with type."));
	}
}

void FunctionCall::checkTypeRequirements()
{
	m_expression->checkTypeRequirements();
	for (ASTPointer<Expression> const& argument: m_arguments)
		argument->checkTypeRequirements();
	Type const& expressionType = *m_expression->getType();
	Type::Category const category = expressionType.getCategory();
	if (category == Type::Category::TYPE)
	{
		TypeType const* type = dynamic_cast<TypeType const*>(&expressionType);
		BOOST_ASSERT(type != nullptr);
		//@todo for structs, we have to check the number of arguments to be equal to the
		// number of non-mapping members
		if (m_arguments.size() != 1)
			BOOST_THROW_EXCEPTION(createTypeError("More than one argument for "
														   "explicit type conersion."));
		if (!m_arguments.front()->getType()->isExplicitlyConvertibleTo(*type->getActualType()))
			BOOST_THROW_EXCEPTION(createTypeError("Explicit type conversion not allowed."));
		m_type = type->getActualType();
	}
	else if (category == Type::Category::FUNCTION)
	{
		//@todo would be nice to create a struct type from the arguments
		// and then ask if that is implicitly convertible to the struct represented by the
		// function parameters
		FunctionType const* function = dynamic_cast<FunctionType const*>(&expressionType);
		BOOST_ASSERT(function != nullptr);
		FunctionDefinition const& fun = function->getFunction();
		std::vector<ASTPointer<VariableDeclaration>> const& parameters = fun.getParameters();
		if (parameters.size() != m_arguments.size())
			BOOST_THROW_EXCEPTION(createTypeError("Wrong argument count for function call."));
		for (size_t i = 0; i < m_arguments.size(); ++i)
			if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameters[i]->getType()))
				BOOST_THROW_EXCEPTION(createTypeError("Invalid type for argument in function call."));
		// @todo actually the return type should be an anonymous struct,
		// but we change it to the type of the first return value until we have structs
		if (fun.getReturnParameterList()->getParameters().empty())
			m_type = std::make_shared<VoidType>();
		else
			m_type = fun.getReturnParameterList()->getParameters().front()->getType();
	}
	else
		BOOST_THROW_EXCEPTION(createTypeError("Type does not support invocation."));
}

void MemberAccess::checkTypeRequirements()
{
	BOOST_ASSERT(false); // not yet implemented
	// m_type = ;
}

void IndexAccess::checkTypeRequirements()
{
	BOOST_ASSERT(false); // not yet implemented
	// m_type = ;
}

void Identifier::checkTypeRequirements()
{
	BOOST_ASSERT(m_referencedDeclaration != nullptr);
	//@todo these dynamic casts here are not really nice...
	// is i useful to have an AST visitor here?
	// or can this already be done in NameAndTypeResolver?
	// the only problem we get there is that in
	// var x;
	// x = 2;
	// var y = x;
	// the type of x is not yet determined.
	VariableDeclaration* variable = dynamic_cast<VariableDeclaration*>(m_referencedDeclaration);
	if (variable != nullptr)
	{
		if (!variable->getType())
			BOOST_THROW_EXCEPTION(createTypeError("Variable referenced before type "
														   "could be determined."));
		m_type = variable->getType();
		return;
	}
	//@todo can we unify these with TypeName::toType()?
	StructDefinition* structDef = dynamic_cast<StructDefinition*>(m_referencedDeclaration);
	if (structDef != nullptr)
	{
		// note that we do not have a struct type here
		m_type = std::make_shared<TypeType>(std::make_shared<StructType>(*structDef));
		return;
	}
	FunctionDefinition* functionDef = dynamic_cast<FunctionDefinition*>(m_referencedDeclaration);
	if (functionDef != nullptr)
	{
		// a function reference is not a TypeType, because calling a TypeType converts to the type.
		// Calling a function (e.g. function(12), otherContract.function(34)) does not do a type
		// conversion.
		m_type = std::make_shared<FunctionType>(*functionDef);
		return;
	}
	ContractDefinition* contractDef = dynamic_cast<ContractDefinition*>(m_referencedDeclaration);
	if (contractDef != nullptr)
	{
		m_type = std::make_shared<TypeType>(std::make_shared<ContractType>(*contractDef));
		return;
	}
	BOOST_ASSERT(false); // declaration reference of unknown/forbidden type
}

void ElementaryTypeNameExpression::checkTypeRequirements()
{
	m_type = std::make_shared<TypeType>(Type::fromElementaryTypeName(m_typeToken));
}

void Literal::checkTypeRequirements()
{
	m_type = Type::forLiteral(*this);
}

}
}
