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
#include <libsolidity/AST_accept.h>

using namespace std;

namespace dev
{
namespace solidity
{

TypeError ASTNode::createTypeError(string const& _description) const
{
	return TypeError() << errinfo_sourceLocation(getLocation()) << errinfo_comment(_description);
}

vector<FunctionDefinition const*> ContractDefinition::getInterfaceFunctions() const
{
	vector<FunctionDefinition const*> exportedFunctions;
	for (ASTPointer<FunctionDefinition> const& f: m_definedFunctions)
		if (f->isPublic() && f->getName() != getName())
			exportedFunctions.push_back(f.get());
	auto compareNames = [](FunctionDefinition const* _a, FunctionDefinition const* _b)
	{
		return _a->getName().compare(_b->getName()) < 0;
	};

	sort(exportedFunctions.begin(), exportedFunctions.end(), compareNames);
	return exportedFunctions;
}

FunctionDefinition const* ContractDefinition::getConstructor() const
{
	for (ASTPointer<FunctionDefinition> const& f: m_definedFunctions)
		if (f->getName() == getName())
			return f.get();
	return nullptr;
}

void StructDefinition::checkMemberTypes() const
{
	for (ASTPointer<VariableDeclaration> const& member: getMembers())
		if (!member->getType()->canBeStored())
			BOOST_THROW_EXCEPTION(member->createTypeError("Type cannot be used in struct."));
}

void StructDefinition::checkRecursion() const
{
	set<StructDefinition const*> definitionsSeen;
	vector<StructDefinition const*> queue = {this};
	while (!queue.empty())
	{
		StructDefinition const* def = queue.back();
		queue.pop_back();
		if (definitionsSeen.count(def))
			BOOST_THROW_EXCEPTION(ParserError() << errinfo_sourceLocation(def->getLocation())
												<< errinfo_comment("Recursive struct definition."));
		definitionsSeen.insert(def);
		for (ASTPointer<VariableDeclaration> const& member: def->getMembers())
			if (member->getType()->getCategory() == Type::Category::STRUCT)
			{
				UserDefinedTypeName const& typeName = dynamic_cast<UserDefinedTypeName const&>(*member->getTypeName());
				queue.push_back(&dynamic_cast<StructDefinition const&>(*typeName.getReferencedDeclaration()));
			}
	}
}

void FunctionDefinition::checkTypeRequirements()
{
	for (ASTPointer<VariableDeclaration> const& var: getParameters() + getReturnParameters())
		if (!var->getType()->canLiveOutsideStorage())
			BOOST_THROW_EXCEPTION(var->createTypeError("Type is required to live outside storage."));

	m_body->checkTypeRequirements();
}

void Block::checkTypeRequirements()
{
	for (shared_ptr<Statement> const& statement: m_statements)
		statement->checkTypeRequirements();
}

void IfStatement::checkTypeRequirements()
{
	m_condition->expectType(BoolType());
	m_trueBody->checkTypeRequirements();
	if (m_falseBody)
		m_falseBody->checkTypeRequirements();
}

void WhileStatement::checkTypeRequirements()
{
	m_condition->expectType(BoolType());
	m_body->checkTypeRequirements();
}

void Return::checkTypeRequirements()
{
	if (!m_expression)
		return;
	if (asserts(m_returnParameters))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Return parameters not assigned."));
	if (m_returnParameters->getParameters().size() != 1)
		BOOST_THROW_EXCEPTION(createTypeError("Different number of arguments in return statement "
											  "than in returns declaration."));
	// this could later be changed such that the paramaters type is an anonymous struct type,
	// but for now, we only allow one return parameter
	m_expression->expectType(*m_returnParameters->getParameters().front()->getType());
}

void VariableDefinition::checkTypeRequirements()
{
	// Variables can be declared without type (with "var"), in which case the first assignment
	// sets the type.
	// Note that assignments before the first declaration are legal because of the special scoping
	// rules inherited from JavaScript.
	if (m_value)
	{
		if (m_variable->getType())
			m_value->expectType(*m_variable->getType());
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
	m_leftHandSide->checkTypeRequirements();
	m_leftHandSide->requireLValue();
	//@todo later, assignments to structs might be possible, but not to mappings
	if (!m_leftHandSide->getType()->isValueType() && !m_leftHandSide->isLocalLValue())
		BOOST_THROW_EXCEPTION(createTypeError("Assignment to non-local non-value lvalue."));
	m_rightHandSide->expectType(*m_leftHandSide->getType());
	m_type = m_leftHandSide->getType();
	if (m_assigmentOperator != Token::ASSIGN)
		// compound assignment
		if (!m_type->acceptsBinaryOperator(Token::AssignmentToBinaryOp(m_assigmentOperator)))
			BOOST_THROW_EXCEPTION(createTypeError("Operator not compatible with type."));
}

void ExpressionStatement::checkTypeRequirements()
{
	m_expression->checkTypeRequirements();
}

void Expression::expectType(Type const& _expectedType)
{
	checkTypeRequirements();
	Type const& type = *getType();
	if (!type.isImplicitlyConvertibleTo(_expectedType))
		BOOST_THROW_EXCEPTION(createTypeError("Type " + type.toString() +
											  " not implicitly convertible to expected type "
											  + _expectedType.toString() + "."));
}

void Expression::requireLValue()
{
	if (!isLValue())
		BOOST_THROW_EXCEPTION(createTypeError("Expression has to be an lvalue."));
	m_lvalueRequested = true;
}

void UnaryOperation::checkTypeRequirements()
{
	// INC, DEC, ADD, SUB, NOT, BIT_NOT, DELETE
	m_subExpression->checkTypeRequirements();
	if (m_operator == Token::Value::INC || m_operator == Token::Value::DEC || m_operator == Token::Value::DELETE)
		m_subExpression->requireLValue();
	m_type = m_subExpression->getType();
	if (!m_type->acceptsUnaryOperator(m_operator))
		BOOST_THROW_EXCEPTION(createTypeError("Unary operator not compatible with type."));
}

void BinaryOperation::checkTypeRequirements()
{
	m_left->checkTypeRequirements();
	m_right->checkTypeRequirements();
	if (m_right->getType()->isImplicitlyConvertibleTo(*m_left->getType()))
		m_commonType = m_left->getType();
	else if (m_left->getType()->isImplicitlyConvertibleTo(*m_right->getType()))
		m_commonType = m_right->getType();
	else
		BOOST_THROW_EXCEPTION(createTypeError("No common type found in binary operation: " +
											  m_left->getType()->toString() + " vs. " +
											  m_right->getType()->toString()));
	if (Token::isCompareOp(m_operator))
		m_type = make_shared<BoolType>();
	else
	{
		m_type = m_commonType;
		if (!m_commonType->acceptsBinaryOperator(m_operator))
			BOOST_THROW_EXCEPTION(createTypeError("Operator " + string(Token::toString(m_operator)) +
												  " not compatible with type " +
												  m_commonType->toString()));
	}
}

void FunctionCall::checkTypeRequirements()
{
	m_expression->checkTypeRequirements();
	for (ASTPointer<Expression> const& argument: m_arguments)
		argument->checkTypeRequirements();

	Type const* expressionType = m_expression->getType().get();
	if (isTypeConversion())
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*expressionType);
		//@todo for structs, we have to check the number of arguments to be equal to the
		// number of non-mapping members
		if (m_arguments.size() != 1)
			BOOST_THROW_EXCEPTION(createTypeError("More than one argument for "
														   "explicit type conersion."));
		if (!m_arguments.front()->getType()->isExplicitlyConvertibleTo(*type.getActualType()))
			BOOST_THROW_EXCEPTION(createTypeError("Explicit type conversion not allowed."));
		m_type = type.getActualType();
	}
	else if (FunctionType const* functionType = dynamic_cast<FunctionType const*>(expressionType))
	{
		//@todo would be nice to create a struct type from the arguments
		// and then ask if that is implicitly convertible to the struct represented by the
		// function parameters
		TypePointers const& parameterTypes = functionType->getParameterTypes();
		if (parameterTypes.size() != m_arguments.size())
			BOOST_THROW_EXCEPTION(createTypeError("Wrong argument count for function call."));
		for (size_t i = 0; i < m_arguments.size(); ++i)
			if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameterTypes[i]))
				BOOST_THROW_EXCEPTION(createTypeError("Invalid type for argument in function call."));
		// @todo actually the return type should be an anonymous struct,
		// but we change it to the type of the first return value until we have structs
		if (functionType->getReturnParameterTypes().empty())
			m_type = make_shared<VoidType>();
		else
			m_type = functionType->getReturnParameterTypes().front();
	}
	else
		BOOST_THROW_EXCEPTION(createTypeError("Type is not callable."));
}

bool FunctionCall::isTypeConversion() const
{
	return m_expression->getType()->getCategory() == Type::Category::TYPE;
}

void NewExpression::checkTypeRequirements()
{
	m_contractName->checkTypeRequirements();
	for (ASTPointer<Expression> const& argument: m_arguments)
		argument->checkTypeRequirements();

	m_contract = dynamic_cast<ContractDefinition const*>(m_contractName->getReferencedDeclaration());
	if (!m_contract)
		BOOST_THROW_EXCEPTION(createTypeError("Identifier is not a contract."));
	shared_ptr<ContractType const> type = make_shared<ContractType const>(*m_contract);
	m_type = type;
	TypePointers const& parameterTypes = type->getConstructorType()->getParameterTypes();
	if (parameterTypes.size() != m_arguments.size())
		BOOST_THROW_EXCEPTION(createTypeError("Wrong argument count for constructor call."));
	for (size_t i = 0; i < m_arguments.size(); ++i)
		if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameterTypes[i]))
			BOOST_THROW_EXCEPTION(createTypeError("Invalid type for argument in constructor call."));
}

void MemberAccess::checkTypeRequirements()
{
	m_expression->checkTypeRequirements();
	Type const& type = *m_expression->getType();
	m_type = type.getMemberType(*m_memberName);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Member \"" + *m_memberName + "\" not found in " + type.toString()));
	//@todo later, this will not always be STORAGE
	m_lvalue = type.getCategory() == Type::Category::STRUCT ? LValueType::STORAGE : LValueType::NONE;
}

void IndexAccess::checkTypeRequirements()
{
	m_base->checkTypeRequirements();
	if (m_base->getType()->getCategory() != Type::Category::MAPPING)
		BOOST_THROW_EXCEPTION(m_base->createTypeError("Indexed expression has to be a mapping (is " +
													  m_base->getType()->toString() + ")"));
	MappingType const& type = dynamic_cast<MappingType const&>(*m_base->getType());
	m_index->expectType(*type.getKeyType());
	m_type = type.getValueType();
	m_lvalue = LValueType::STORAGE;
}

void Identifier::checkTypeRequirements()
{
	if (asserts(m_referencedDeclaration))
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Identifier not resolved."));

	VariableDeclaration const* variable = dynamic_cast<VariableDeclaration const*>(m_referencedDeclaration);
	if (variable)
	{
		if (!variable->getType())
			BOOST_THROW_EXCEPTION(createTypeError("Variable referenced before type could be determined."));
		m_type = variable->getType();
		m_lvalue = variable->isLocalVariable() ? LValueType::LOCAL : LValueType::STORAGE;
		return;
	}
	//@todo can we unify these with TypeName::toType()?
	StructDefinition const* structDef = dynamic_cast<StructDefinition const*>(m_referencedDeclaration);
	if (structDef)
	{
		// note that we do not have a struct type here
		m_type = make_shared<TypeType const>(make_shared<StructType const>(*structDef));
		return;
	}
	FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(m_referencedDeclaration);
	if (functionDef)
	{
		// a function reference is not a TypeType, because calling a TypeType converts to the type.
		// Calling a function (e.g. function(12), otherContract.function(34)) does not do a type
		// conversion.
		m_type = make_shared<FunctionType const>(*functionDef);
		return;
	}
	ContractDefinition const* contractDef = dynamic_cast<ContractDefinition const*>(m_referencedDeclaration);
	if (contractDef)
	{
		m_type = make_shared<TypeType const>(make_shared<ContractType>(*contractDef));
		return;
	}
	MagicVariableDeclaration const* magicVariable = dynamic_cast<MagicVariableDeclaration const*>(m_referencedDeclaration);
	if (magicVariable)
	{
		m_type = magicVariable->getType();
		return;
	}
	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Declaration reference of unknown/forbidden type."));
}

void ElementaryTypeNameExpression::checkTypeRequirements()
{
	m_type = make_shared<TypeType const>(Type::fromElementaryTypeName(m_typeToken));
}

void Literal::checkTypeRequirements()
{
	m_type = Type::forLiteral(*this);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Literal value too large."));
}

}
}
