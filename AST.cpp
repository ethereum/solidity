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
#include <libsolidity/Utils.h>
#include <libsolidity/AST.h>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/AST_accept.h>

#include <libdevcrypto/SHA3.h>

using namespace std;

namespace dev
{
namespace solidity
{

TypeError ASTNode::createTypeError(string const& _description) const
{
	return TypeError() << errinfo_sourceLocation(getLocation()) << errinfo_comment(_description);
}

TypePointer ContractDefinition::getType(ContractDefinition const* _currentContract) const
{
	return make_shared<TypeType>(make_shared<ContractType>(*this), _currentContract);
}

void ContractDefinition::checkTypeRequirements()
{
	for (ASTPointer<InheritanceSpecifier> const& baseSpecifier: getBaseContracts())
		baseSpecifier->checkTypeRequirements();

	checkIllegalOverrides();

	FunctionDefinition const* constructor = getConstructor();
	if (constructor && !constructor->getReturnParameters().empty())
		BOOST_THROW_EXCEPTION(constructor->getReturnParameterList()->createTypeError(
									"Non-empty \"returns\" directive for constructor."));

	FunctionDefinition const* fallbackFunction = nullptr;
	for (ASTPointer<FunctionDefinition> const& function: getDefinedFunctions())
		if (function->getName().empty())
		{
			if (fallbackFunction)
				BOOST_THROW_EXCEPTION(DeclarationError() << errinfo_comment("Only one fallback function is allowed."));
			else
			{
				fallbackFunction = function.get();
				if (!fallbackFunction->getParameters().empty())
					BOOST_THROW_EXCEPTION(fallbackFunction->getParameterList().createTypeError("Fallback function cannot take parameters."));
			}
		}
	for (ASTPointer<ModifierDefinition> const& modifier: getFunctionModifiers())
		modifier->checkTypeRequirements();

	for (ASTPointer<FunctionDefinition> const& function: getDefinedFunctions())
		function->checkTypeRequirements();

	// check for hash collisions in function signatures
	set<FixedHash<4>> hashes;
	for (auto const& it: getInterfaceFunctionList())
	{
		FixedHash<4> const& hash = it.first;
		if (hashes.count(hash))
			BOOST_THROW_EXCEPTION(createTypeError(
									  std::string("Function signature hash collision for ") +
									  it.second->getCanonicalSignature()));
		hashes.insert(hash);
	}
}

map<FixedHash<4>, FunctionTypePointer> ContractDefinition::getInterfaceFunctions() const
{
	auto exportedFunctionList = getInterfaceFunctionList();

	map<FixedHash<4>, FunctionTypePointer> exportedFunctions;
	for (auto const& it: exportedFunctionList)
		exportedFunctions.insert(it);

	solAssert(exportedFunctionList.size() == exportedFunctions.size(),
			  "Hash collision at Function Definition Hash calculation");

	return exportedFunctions;
}

FunctionDefinition const* ContractDefinition::getConstructor() const
{
	for (ASTPointer<FunctionDefinition> const& f: m_definedFunctions)
		if (f->isConstructor())
			return f.get();
	return nullptr;
}

FunctionDefinition const* ContractDefinition::getFallbackFunction() const
{
	for (ContractDefinition const* contract: getLinearizedBaseContracts())
		for (ASTPointer<FunctionDefinition> const& f: contract->getDefinedFunctions())
			if (f->getName().empty())
				return f.get();
	return nullptr;
}

void ContractDefinition::checkIllegalOverrides() const
{
	// TODO unify this at a later point. for this we need to put the constness and the access specifier
	// into the types
	map<string, FunctionDefinition const*> functions;
	map<string, ModifierDefinition const*> modifiers;

	// We search from derived to base, so the stored item causes the error.
	for (ContractDefinition const* contract: getLinearizedBaseContracts())
	{
		for (ASTPointer<FunctionDefinition> const& function: contract->getDefinedFunctions())
		{
			if (function->isConstructor())
				continue; // constructors can neither be overridden nor override anything
			string const& name = function->getName();
			if (modifiers.count(name))
				BOOST_THROW_EXCEPTION(modifiers[name]->createTypeError("Override changes function to modifier."));
			FunctionDefinition const*& override = functions[name];
			if (!override)
				override = function.get();
			else if (override->getVisibility() != function->getVisibility() ||
					 override->isDeclaredConst() != function->isDeclaredConst() ||
					 FunctionType(*override) != FunctionType(*function))
				BOOST_THROW_EXCEPTION(override->createTypeError("Override changes extended function signature."));
		}
		for (ASTPointer<ModifierDefinition> const& modifier: contract->getFunctionModifiers())
		{
			string const& name = modifier->getName();
			if (functions.count(name))
				BOOST_THROW_EXCEPTION(functions[name]->createTypeError("Override changes modifier to function."));
			ModifierDefinition const*& override = modifiers[name];
			if (!override)
				override = modifier.get();
			else if (ModifierType(*override) != ModifierType(*modifier))
				BOOST_THROW_EXCEPTION(override->createTypeError("Override changes modifier signature."));
		}
	}
}

std::vector<ASTPointer<EventDefinition>> const& ContractDefinition::getInterfaceEvents() const
{
	if (!m_interfaceEvents)
	{
		set<string> eventsSeen;
		m_interfaceEvents.reset(new std::vector<ASTPointer<EventDefinition>>());
		for (ContractDefinition const* contract: getLinearizedBaseContracts())
			for (ASTPointer<EventDefinition> const& e: contract->getEvents())
				if (eventsSeen.count(e->getName()) == 0)
				{
					eventsSeen.insert(e->getName());
					m_interfaceEvents->push_back(e);
				}
	}
	return *m_interfaceEvents;
}

vector<pair<FixedHash<4>, FunctionTypePointer>> const& ContractDefinition::getInterfaceFunctionList() const
{
	if (!m_interfaceFunctionList)
	{
		set<string> functionsSeen;
		m_interfaceFunctionList.reset(new vector<pair<FixedHash<4>, FunctionTypePointer>>());
		for (ContractDefinition const* contract: getLinearizedBaseContracts())
		{
			for (ASTPointer<FunctionDefinition> const& f: contract->getDefinedFunctions())
				if (f->isPublic() && !f->isConstructor() && !f->getName().empty() && functionsSeen.count(f->getName()) == 0)
				{
					functionsSeen.insert(f->getName());
					FixedHash<4> hash(dev::sha3(f->getCanonicalSignature()));
					m_interfaceFunctionList->push_back(make_pair(hash, make_shared<FunctionType>(*f, false)));
				}

			for (ASTPointer<VariableDeclaration> const& v: contract->getStateVariables())
				if (v->isPublic() && functionsSeen.count(v->getName()) == 0)
				{
					FunctionType ftype(*v);
					functionsSeen.insert(v->getName());
					FixedHash<4> hash(dev::sha3(ftype.getCanonicalSignature(v->getName())));
					m_interfaceFunctionList->push_back(make_pair(hash, make_shared<FunctionType>(*v)));
				}
		}
	}
	return *m_interfaceFunctionList;
}

void InheritanceSpecifier::checkTypeRequirements()
{
	m_baseName->checkTypeRequirements();
	for (ASTPointer<Expression> const& argument: m_arguments)
		argument->checkTypeRequirements();

	ContractDefinition const* base = dynamic_cast<ContractDefinition const*>(m_baseName->getReferencedDeclaration());
	solAssert(base, "Base contract not available.");
	TypePointers parameterTypes = ContractType(*base).getConstructorType()->getParameterTypes();
	if (parameterTypes.size() != m_arguments.size())
		BOOST_THROW_EXCEPTION(createTypeError("Wrong argument count for constructor call."));
	for (size_t i = 0; i < m_arguments.size(); ++i)
		if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameterTypes[i]))
			BOOST_THROW_EXCEPTION(createTypeError("Invalid type for argument in constructer call."));
}

TypePointer StructDefinition::getType(ContractDefinition const*) const
{
	return make_shared<TypeType>(make_shared<StructType>(*this));
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
			if (member->getType()->getCategory() == Type::Category::Struct)
			{
				UserDefinedTypeName const& typeName = dynamic_cast<UserDefinedTypeName const&>(*member->getTypeName());
				queue.push_back(&dynamic_cast<StructDefinition const&>(*typeName.getReferencedDeclaration()));
			}
	}
}

TypePointer FunctionDefinition::getType(ContractDefinition const*) const
{
	return make_shared<FunctionType>(*this);
}

void FunctionDefinition::checkTypeRequirements()
{
	for (ASTPointer<VariableDeclaration> const& var: getParameters() + getReturnParameters())
		if (!var->getType()->canLiveOutsideStorage())
			BOOST_THROW_EXCEPTION(var->createTypeError("Type is required to live outside storage."));
	for (ASTPointer<ModifierInvocation> const& modifier: m_functionModifiers)
		modifier->checkTypeRequirements();

	m_body->checkTypeRequirements();
}

string FunctionDefinition::getCanonicalSignature() const
{
	return FunctionType(*this).getCanonicalSignature(getName());
}

Declaration::LValueType VariableDeclaration::getLValueType() const
{
	if (dynamic_cast<FunctionDefinition const*>(getScope()) || dynamic_cast<ModifierDefinition const*>(getScope()))
		return Declaration::LValueType::Local;
	else
		return Declaration::LValueType::Storage;
}

TypePointer ModifierDefinition::getType(ContractDefinition const*) const
{
	return make_shared<ModifierType>(*this);
}

void ModifierDefinition::checkTypeRequirements()
{
	m_body->checkTypeRequirements();
}

void ModifierInvocation::checkTypeRequirements()
{
	m_modifierName->checkTypeRequirements();
	for (ASTPointer<Expression> const& argument: m_arguments)
		argument->checkTypeRequirements();

	ModifierDefinition const* modifier = dynamic_cast<ModifierDefinition const*>(m_modifierName->getReferencedDeclaration());
	solAssert(modifier, "Function modifier not found.");
	vector<ASTPointer<VariableDeclaration>> const& parameters = modifier->getParameters();
	if (parameters.size() != m_arguments.size())
		BOOST_THROW_EXCEPTION(createTypeError("Wrong argument count for modifier invocation."));
	for (size_t i = 0; i < m_arguments.size(); ++i)
		if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameters[i]->getType()))
			BOOST_THROW_EXCEPTION(createTypeError("Invalid type for argument in modifier invocation."));
}

void EventDefinition::checkTypeRequirements()
{
	int numIndexed = 0;
	for (ASTPointer<VariableDeclaration> const& var: getParameters())
	{
		if (var->isIndexed())
			numIndexed++;
		if (!var->getType()->canLiveOutsideStorage())
			BOOST_THROW_EXCEPTION(var->createTypeError("Type is required to live outside storage."));
	}
	if (numIndexed > 3)
		BOOST_THROW_EXCEPTION(createTypeError("More than 3 indexed arguments for event."));
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

void ForStatement::checkTypeRequirements()
{
	if (m_initExpression)
		m_initExpression->checkTypeRequirements();
	if (m_condExpression)
		m_condExpression->expectType(BoolType());
	if (m_loopExpression)
		m_loopExpression->checkTypeRequirements();
	m_body->checkTypeRequirements();
}

void Return::checkTypeRequirements()
{
	if (!m_expression)
		return;
	if (!m_returnParameters)
		BOOST_THROW_EXCEPTION(createTypeError("Return arguments not allowed."));
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
			TypePointer type = m_value->getType();
			if (type->getCategory() == Type::Category::IntegerConstant)
			{
				auto intType = dynamic_pointer_cast<IntegerConstantType const>(type)->getIntegerType();
				if (!intType)
					BOOST_THROW_EXCEPTION(m_value->createTypeError("Invalid integer constant " + type->toString()));
				type = intType;
			}
			else if (type->getCategory() == Type::Category::Void)
				BOOST_THROW_EXCEPTION(m_variable->createTypeError("var cannot be void type"));
			m_variable->setType(type);
		}
	}
}

void Assignment::checkTypeRequirements()
{
	m_leftHandSide->checkTypeRequirements();
	m_leftHandSide->requireLValue();
	if (m_leftHandSide->getType()->getCategory() == Type::Category::Mapping)
		BOOST_THROW_EXCEPTION(createTypeError("Mappings cannot be assigned to."));
	m_type = m_leftHandSide->getType();
	if (m_assigmentOperator == Token::Assign)
		m_rightHandSide->expectType(*m_type);
	else
	{
		// compound assignment
		m_rightHandSide->checkTypeRequirements();
		TypePointer resultType = m_type->binaryOperatorResult(Token::AssignmentToBinaryOp(m_assigmentOperator),
															  m_rightHandSide->getType());
		if (!resultType || *resultType != *m_type)
			BOOST_THROW_EXCEPTION(createTypeError("Operator " + string(Token::toString(m_assigmentOperator)) +
												  " not compatible with types " +
												  m_type->toString() + " and " +
												  m_rightHandSide->getType()->toString()));
	}
}

void ExpressionStatement::checkTypeRequirements()
{
	m_expression->checkTypeRequirements();
	if (m_expression->getType()->getCategory() == Type::Category::IntegerConstant)
		if (!dynamic_pointer_cast<IntegerConstantType const>(m_expression->getType())->getIntegerType())
			BOOST_THROW_EXCEPTION(m_expression->createTypeError("Invalid integer constant."));
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
	// Inc, Dec, Add, Sub, Not, BitNot, Delete
	m_subExpression->checkTypeRequirements();
	if (m_operator == Token::Value::Inc || m_operator == Token::Value::Dec || m_operator == Token::Value::Delete)
		m_subExpression->requireLValue();
	m_type = m_subExpression->getType()->unaryOperatorResult(m_operator);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Unary operator not compatible with type."));
}

void BinaryOperation::checkTypeRequirements()
{
	m_left->checkTypeRequirements();
	m_right->checkTypeRequirements();
	m_commonType = m_left->getType()->binaryOperatorResult(m_operator, m_right->getType());
	if (!m_commonType)
		BOOST_THROW_EXCEPTION(createTypeError("Operator " + string(Token::toString(m_operator)) +
											  " not compatible with types " +
											  m_left->getType()->toString() + " and " +
											  m_right->getType()->toString()));
	m_type = Token::isCompareOp(m_operator) ? make_shared<BoolType>() : m_commonType;
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
			BOOST_THROW_EXCEPTION(createTypeError("More than one argument for explicit type conversion."));
		if (!m_names.empty())
			BOOST_THROW_EXCEPTION(createTypeError("Type conversion cannot allow named arguments."));
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
		if (!functionType->takesArbitraryParameters() && parameterTypes.size() != m_arguments.size())
			BOOST_THROW_EXCEPTION(createTypeError("Wrong argument count for function call."));

		if (m_names.empty())
		{
			for (size_t i = 0; i < m_arguments.size(); ++i)
				if (!functionType->takesArbitraryParameters() &&
						!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameterTypes[i]))
					BOOST_THROW_EXCEPTION(m_arguments[i]->createTypeError("Invalid type for argument in function call."));
		}
		else
		{
			if (functionType->takesArbitraryParameters())
				BOOST_THROW_EXCEPTION(createTypeError("Named arguments cannnot be used for functions "
													  "that take arbitrary parameters."));
			auto const& parameterNames = functionType->getParameterNames();
			if (parameterNames.size() != m_names.size())
				BOOST_THROW_EXCEPTION(createTypeError("Some argument names are missing."));

			// check duplicate names
			for (size_t i = 0; i < m_names.size(); i++)
				for (size_t j = i + 1; j < m_names.size(); j++)
					if (*m_names[i] == *m_names[j])
						BOOST_THROW_EXCEPTION(m_arguments[i]->createTypeError("Duplicate named argument."));

			for (size_t i = 0; i < m_names.size(); i++) {
				bool found = false;
				for (size_t j = 0; j < parameterNames.size(); j++) {
					if (parameterNames[j] == *m_names[i]) {
						// check type convertible
						if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameterTypes[j]))
							BOOST_THROW_EXCEPTION(createTypeError("Invalid type for argument in function call."));

						found = true;
						break;
					}
				}
				if (!found)
					BOOST_THROW_EXCEPTION(createTypeError("Named argument does not match function declaration."));
			}
		}

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
	return m_expression->getType()->getCategory() == Type::Category::TypeType;
}

void NewExpression::checkTypeRequirements()
{
	m_contractName->checkTypeRequirements();
	m_contract = dynamic_cast<ContractDefinition const*>(m_contractName->getReferencedDeclaration());
	if (!m_contract)
		BOOST_THROW_EXCEPTION(createTypeError("Identifier is not a contract."));
	shared_ptr<ContractType const> contractType = make_shared<ContractType>(*m_contract);
	TypePointers const& parameterTypes = contractType->getConstructorType()->getParameterTypes();
	m_type = make_shared<FunctionType>(parameterTypes, TypePointers{contractType},
									   FunctionType::Location::Creation);
}

void MemberAccess::checkTypeRequirements()
{
	m_expression->checkTypeRequirements();
	Type const& type = *m_expression->getType();
	m_type = type.getMemberType(*m_memberName);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Member \"" + *m_memberName + "\" not found or not "
											  "visible in " + type.toString()));
	//@todo later, this will not always be STORAGE
	m_lvalue = type.getCategory() == Type::Category::Struct ? Declaration::LValueType::Storage : Declaration::LValueType::None;
}

void IndexAccess::checkTypeRequirements()
{
	m_base->checkTypeRequirements();
	if (m_base->getType()->getCategory() != Type::Category::Mapping)
		BOOST_THROW_EXCEPTION(m_base->createTypeError("Indexed expression has to be a mapping (is " +
													  m_base->getType()->toString() + ")"));
	MappingType const& type = dynamic_cast<MappingType const&>(*m_base->getType());
	m_index->expectType(*type.getKeyType());
	m_type = type.getValueType();
	m_lvalue = Declaration::LValueType::Storage;
}

void Identifier::checkTypeRequirements()
{
	solAssert(m_referencedDeclaration, "Identifier not resolved.");

	m_lvalue = m_referencedDeclaration->getLValueType();
	m_type = m_referencedDeclaration->getType(m_currentContract);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Declaration referenced before type could be determined."));
}

void ElementaryTypeNameExpression::checkTypeRequirements()
{
	m_type = make_shared<TypeType>(Type::fromElementaryTypeName(m_typeToken));
}

void Literal::checkTypeRequirements()
{
	m_type = Type::forLiteral(*this);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Invalid literal value."));
}

}
}
