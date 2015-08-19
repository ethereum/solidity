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
#include <functional>
#include <boost/range/adaptor/reversed.hpp>
#include <libsolidity/Utils.h>
#include <libsolidity/AST.h>
#include <libsolidity/ASTVisitor.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/AST_accept.h>

#include <libdevcore/SHA3.h>

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

	checkDuplicateFunctions();
	checkIllegalOverrides();
	checkAbstractFunctions();
	checkAbstractConstructors();

	FunctionDefinition const* constructor = getConstructor();
	if (constructor && !constructor->getReturnParameters().empty())
		BOOST_THROW_EXCEPTION(constructor->getReturnParameterList()->createTypeError(
									"Non-empty \"returns\" directive for constructor."));

	FunctionDefinition const* fallbackFunction = nullptr;
	for (ASTPointer<FunctionDefinition> const& function: getDefinedFunctions())
	{
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
		if (!function->isFullyImplemented())
			setFullyImplemented(false);
	}
	for (ASTPointer<ModifierDefinition> const& modifier: getFunctionModifiers())
		modifier->checkTypeRequirements();

	for (ASTPointer<FunctionDefinition> const& function: getDefinedFunctions())
		function->checkTypeRequirements();

	for (ASTPointer<VariableDeclaration> const& variable: m_stateVariables)
		variable->checkTypeRequirements();

	checkExternalTypeClashes();
	// check for hash collisions in function signatures
	set<FixedHash<4>> hashes;
	for (auto const& it: getInterfaceFunctionList())
	{
		FixedHash<4> const& hash = it.first;
		if (hashes.count(hash))
			BOOST_THROW_EXCEPTION(createTypeError(
				string("Function signature hash collision for ") + it.second->externalSignature()
			));
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

void ContractDefinition::checkDuplicateFunctions() const
{
	/// Checks that two functions with the same name defined in this contract have different
	/// argument types and that there is at most one constructor.
	map<string, vector<FunctionDefinition const*>> functions;
	for (ASTPointer<FunctionDefinition> const& function: getDefinedFunctions())
		functions[function->getName()].push_back(function.get());

	if (functions[getName()].size() > 1)
	{
		SecondarySourceLocation ssl;
		auto it = functions[getName()].begin();
		++it;
		for (; it != functions[getName()].end(); ++it)
			ssl.append("Another declaration is here:", (*it)->getLocation());

		BOOST_THROW_EXCEPTION(
			DeclarationError() <<
			errinfo_sourceLocation(functions[getName()].front()->getLocation()) <<
			errinfo_comment("More than one constructor defined.") <<
			errinfo_secondarySourceLocation(ssl)
		);
	}
	for (auto const& it: functions)
	{
		vector<FunctionDefinition const*> const& overloads = it.second;
		for (size_t i = 0; i < overloads.size(); ++i)
			for (size_t j = i + 1; j < overloads.size(); ++j)
				if (FunctionType(*overloads[i]).hasEqualArgumentTypes(FunctionType(*overloads[j])))
					BOOST_THROW_EXCEPTION(
						DeclarationError() <<
						errinfo_sourceLocation(overloads[j]->getLocation()) <<
						errinfo_comment("Function with same name and arguments defined twice.") <<
						errinfo_secondarySourceLocation(SecondarySourceLocation().append(
							"Other declaration is here:", overloads[i]->getLocation()))
					);
	}
}

void ContractDefinition::checkAbstractFunctions()
{
	// Mapping from name to function definition (exactly one per argument type equality class) and
	// flag to indicate whether it is fully implemented.
	using FunTypeAndFlag = std::pair<FunctionTypePointer, bool>;
	map<string, vector<FunTypeAndFlag>> functions;

	// Search from base to derived
	for (ContractDefinition const* contract: boost::adaptors::reverse(getLinearizedBaseContracts()))
		for (ASTPointer<FunctionDefinition> const& function: contract->getDefinedFunctions())
		{
			auto& overloads = functions[function->getName()];
			FunctionTypePointer funType = make_shared<FunctionType>(*function);
			auto it = find_if(overloads.begin(), overloads.end(), [&](FunTypeAndFlag const& _funAndFlag)
			{
				return funType->hasEqualArgumentTypes(*_funAndFlag.first);
			});
			if (it == overloads.end())
				overloads.push_back(make_pair(funType, function->isFullyImplemented()));
			else if (it->second)
			{
				if (!function->isFullyImplemented())
					BOOST_THROW_EXCEPTION(function->createTypeError("Redeclaring an already implemented function as abstract"));
			}
			else if (function->isFullyImplemented())
				it->second = true;
		}

	// Set to not fully implemented if at least one flag is false.
	for (auto const& it: functions)
		for (auto const& funAndFlag: it.second)
			if (!funAndFlag.second)
			{
				setFullyImplemented(false);
				return;
			}
}

void ContractDefinition::checkAbstractConstructors()
{
	set<ContractDefinition const*> argumentsNeeded;
	// check that we get arguments for all base constructors that need it.
	// If not mark the contract as abstract (not fully implemented)

	vector<ContractDefinition const*> const& bases = getLinearizedBaseContracts();
	for (ContractDefinition const* contract: bases)
		if (FunctionDefinition const* constructor = contract->getConstructor())
			if (contract != this && !constructor->getParameters().empty())
				argumentsNeeded.insert(contract);

	for (ContractDefinition const* contract: bases)
	{
		if (FunctionDefinition const* constructor = contract->getConstructor())
			for (auto const& modifier: constructor->getModifiers())
			{
				auto baseContract = dynamic_cast<ContractDefinition const*>(
					&modifier->getName()->getReferencedDeclaration()
				);
				if (baseContract)
					argumentsNeeded.erase(baseContract);
			}


		for (ASTPointer<InheritanceSpecifier> const& base: contract->getBaseContracts())
		{
			auto baseContract = dynamic_cast<ContractDefinition const*>(
				&base->getName()->getReferencedDeclaration()
			);
			solAssert(baseContract, "");
			if (!base->getArguments().empty())
				argumentsNeeded.erase(baseContract);
		}
	}
	if (!argumentsNeeded.empty())
		setFullyImplemented(false);
}

void ContractDefinition::checkIllegalOverrides() const
{
	// TODO unify this at a later point. for this we need to put the constness and the access specifier
	// into the types
	map<string, vector<FunctionDefinition const*>> functions;
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
			FunctionType functionType(*function);
			// function should not change the return type
			for (FunctionDefinition const* overriding: functions[name])
			{
				FunctionType overridingType(*overriding);
				if (!overridingType.hasEqualArgumentTypes(functionType))
					continue;
				if (
					overriding->getVisibility() != function->getVisibility() ||
					overriding->isDeclaredConst() != function->isDeclaredConst() ||
					overridingType != functionType
				)
					BOOST_THROW_EXCEPTION(overriding->createTypeError("Override changes extended function signature."));
			}
			functions[name].push_back(function.get());
		}
		for (ASTPointer<ModifierDefinition> const& modifier: contract->getFunctionModifiers())
		{
			string const& name = modifier->getName();
			ModifierDefinition const*& override = modifiers[name];
			if (!override)
				override = modifier.get();
			else if (ModifierType(*override) != ModifierType(*modifier))
				BOOST_THROW_EXCEPTION(override->createTypeError("Override changes modifier signature."));
			if (!functions[name].empty())
				BOOST_THROW_EXCEPTION(override->createTypeError("Override changes modifier to function."));
		}
	}
}

void ContractDefinition::checkExternalTypeClashes() const
{
	map<string, vector<pair<Declaration const*, shared_ptr<FunctionType>>>> externalDeclarations;
	for (ContractDefinition const* contract: getLinearizedBaseContracts())
	{
		for (ASTPointer<FunctionDefinition> const& f: contract->getDefinedFunctions())
			if (f->isPartOfExternalInterface())
			{
				auto functionType = make_shared<FunctionType>(*f);
				externalDeclarations[functionType->externalSignature(f->getName())].push_back(
					make_pair(f.get(), functionType)
				);
			}
		for (ASTPointer<VariableDeclaration> const& v: contract->getStateVariables())
			if (v->isPartOfExternalInterface())
			{
				auto functionType = make_shared<FunctionType>(*v);
				externalDeclarations[functionType->externalSignature(v->getName())].push_back(
					make_pair(v.get(), functionType)
				);
			}
	}
	for (auto const& it: externalDeclarations)
		for (size_t i = 0; i < it.second.size(); ++i)
			for (size_t j = i + 1; j < it.second.size(); ++j)
				if (!it.second[i].second->hasEqualArgumentTypes(*it.second[j].second))
					BOOST_THROW_EXCEPTION(it.second[j].first->createTypeError(
						"Function overload clash during conversion to external types for arguments."
					));
}

vector<ASTPointer<EventDefinition>> const& ContractDefinition::getInterfaceEvents() const
{
	if (!m_interfaceEvents)
	{
		set<string> eventsSeen;
		m_interfaceEvents.reset(new vector<ASTPointer<EventDefinition>>());
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
		set<string> signaturesSeen;
		m_interfaceFunctionList.reset(new vector<pair<FixedHash<4>, FunctionTypePointer>>());
		for (ContractDefinition const* contract: getLinearizedBaseContracts())
		{
			for (ASTPointer<FunctionDefinition> const& f: contract->getDefinedFunctions())
			{
				if (!f->isPartOfExternalInterface())
					continue;
				string functionSignature = f->externalSignature();
				if (signaturesSeen.count(functionSignature) == 0)
				{
					functionsSeen.insert(f->getName());
					signaturesSeen.insert(functionSignature);
					FixedHash<4> hash(dev::sha3(functionSignature));
					m_interfaceFunctionList->push_back(make_pair(hash, make_shared<FunctionType>(*f, false)));
				}
			}

			for (ASTPointer<VariableDeclaration> const& v: contract->getStateVariables())
				if (functionsSeen.count(v->getName()) == 0 && v->isPartOfExternalInterface())
				{
					FunctionType ftype(*v);
					solAssert(v->getType().get(), "");
					functionsSeen.insert(v->getName());
					FixedHash<4> hash(dev::sha3(ftype.externalSignature(v->getName())));
					m_interfaceFunctionList->push_back(make_pair(hash, make_shared<FunctionType>(*v)));
				}
		}
	}
	return *m_interfaceFunctionList;
}

string const& ContractDefinition::devDocumentation() const
{
	return m_devDocumentation;
}

string const& ContractDefinition::userDocumentation() const
{
	return m_userDocumentation;
}

void ContractDefinition::setDevDocumentation(string const& _devDocumentation)
{
	m_devDocumentation = _devDocumentation;
}

void ContractDefinition::setUserDocumentation(string const& _userDocumentation)
{
	m_userDocumentation = _userDocumentation;
}


vector<Declaration const*> const& ContractDefinition::getInheritableMembers() const
{
	if (!m_inheritableMembers)
	{
		set<string> memberSeen;
		m_inheritableMembers.reset(new vector<Declaration const*>());
		auto addInheritableMember = [&](Declaration const* _decl)
		{
			if (memberSeen.count(_decl->getName()) == 0 && _decl->isVisibleInDerivedContracts())
			{
				memberSeen.insert(_decl->getName());
				m_inheritableMembers->push_back(_decl);
			}
		};

		for (ASTPointer<FunctionDefinition> const& f: getDefinedFunctions())
			addInheritableMember(f.get());

		for (ASTPointer<VariableDeclaration> const& v: getStateVariables())
			addInheritableMember(v.get());

		for (ASTPointer<StructDefinition> const& s: getDefinedStructs())
			addInheritableMember(s.get());
	}
	return *m_inheritableMembers;
}

TypePointer EnumValue::getType(ContractDefinition const*) const
{
	EnumDefinition const* parentDef = dynamic_cast<EnumDefinition const*>(getScope());
	solAssert(parentDef, "Enclosing Scope of EnumValue was not set");
	return make_shared<EnumType>(*parentDef);
}

void InheritanceSpecifier::checkTypeRequirements()
{
	m_baseName->checkTypeRequirements(nullptr);
	for (ASTPointer<Expression> const& argument: m_arguments)
		argument->checkTypeRequirements(nullptr);

	ContractDefinition const* base = dynamic_cast<ContractDefinition const*>(&m_baseName->getReferencedDeclaration());
	solAssert(base, "Base contract not available.");
	TypePointers parameterTypes = ContractType(*base).getConstructorType()->getParameterTypes();
	if (!m_arguments.empty() && parameterTypes.size() != m_arguments.size())
		BOOST_THROW_EXCEPTION(createTypeError(
			"Wrong argument count for constructor call: " +
			toString(m_arguments.size()) +
			" arguments given but expected " +
			toString(parameterTypes.size()) +
			"."
		));

	for (size_t i = 0; i < m_arguments.size(); ++i)
		if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameterTypes[i]))
			BOOST_THROW_EXCEPTION(m_arguments[i]->createTypeError(
				"Invalid type for argument in constructor call. "
				"Invalid implicit conversion from " +
				m_arguments[i]->getType()->toString() +
				" to " +
				parameterTypes[i]->toString() +
				" requested."
			));
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
	using StructPointer = StructDefinition const*;
	using StructPointersSet = set<StructPointer>;
	function<void(StructPointer,StructPointersSet const&)> check = [&](StructPointer _struct, StructPointersSet const& _parents)
	{
		if (_parents.count(_struct))
			BOOST_THROW_EXCEPTION(
				ParserError() <<
				errinfo_sourceLocation(_struct->getLocation()) <<
				errinfo_comment("Recursive struct definition.")
			);
		set<StructDefinition const*> parents = _parents;
		parents.insert(_struct);
		for (ASTPointer<VariableDeclaration> const& member: _struct->getMembers())
			if (member->getType()->getCategory() == Type::Category::Struct)
			{
				auto const& typeName = dynamic_cast<UserDefinedTypeName const&>(*member->getTypeName());
				check(
					&dynamic_cast<StructDefinition const&>(*typeName.getReferencedDeclaration()),
					parents
				);
			}
	};
	check(this, StructPointersSet{});
}

TypePointer EnumDefinition::getType(ContractDefinition const*) const
{
	return make_shared<TypeType>(make_shared<EnumType>(*this));
}

TypePointer FunctionDefinition::getType(ContractDefinition const*) const
{
	return make_shared<FunctionType>(*this);
}

void FunctionDefinition::checkTypeRequirements()
{
	for (ASTPointer<VariableDeclaration> const& var: getParameters() + getReturnParameters())
	{
		if (!var->getType()->canLiveOutsideStorage())
			BOOST_THROW_EXCEPTION(var->createTypeError("Type is required to live outside storage."));
		if (getVisibility() >= Visibility::Public && !(var->getType()->externalType()))
			BOOST_THROW_EXCEPTION(var->createTypeError("Internal type is not allowed for public and external functions."));
	}
	for (ASTPointer<ModifierInvocation> const& modifier: m_functionModifiers)
		modifier->checkTypeRequirements(isConstructor() ?
			dynamic_cast<ContractDefinition const&>(*getScope()).getLinearizedBaseContracts() :
			vector<ContractDefinition const*>());
	if (m_body)
		m_body->checkTypeRequirements();
}

string FunctionDefinition::externalSignature() const
{
	return FunctionType(*this).externalSignature(getName());
}

bool VariableDeclaration::isLValue() const
{
	// External function parameters and constant declared variables are Read-Only
	return !isExternalCallableParameter() && !m_isConstant;
}

void VariableDeclaration::checkTypeRequirements()
{
	// Variables can be declared without type (with "var"), in which case the first assignment
	// sets the type.
	// Note that assignments before the first declaration are legal because of the special scoping
	// rules inherited from JavaScript.
	if (m_isConstant)
	{
		if (!dynamic_cast<ContractDefinition const*>(getScope()))
			BOOST_THROW_EXCEPTION(createTypeError("Illegal use of \"constant\" specifier."));
		if ((m_type && !m_type->isValueType()) || !m_value)
			BOOST_THROW_EXCEPTION(createTypeError("Unitialized \"constant\" variable."));
	}
	if (m_type)
	{
		if (m_value)
			m_value->expectType(*m_type);
	}
	else
	{
		if (!m_value)
			// This feature might be extended in the future.
			BOOST_THROW_EXCEPTION(createTypeError("Assignment necessary for type detection."));
		m_value->checkTypeRequirements(nullptr);

		TypePointer const& type = m_value->getType();
		if (
			type->getCategory() == Type::Category::IntegerConstant &&
			!dynamic_pointer_cast<IntegerConstantType const>(type)->getIntegerType()
		)
			BOOST_THROW_EXCEPTION(m_value->createTypeError("Invalid integer constant " + type->toString() + "."));
		else if (type->getCategory() == Type::Category::Void)
			BOOST_THROW_EXCEPTION(createTypeError("Variable cannot have void type."));
		m_type = type->mobileType();
	}
	solAssert(!!m_type, "");
	if (!m_isStateVariable)
	{
		if (m_type->dataStoredIn(DataLocation::Memory) || m_type->dataStoredIn(DataLocation::CallData))
			if (!m_type->canLiveOutsideStorage())
				BOOST_THROW_EXCEPTION(createTypeError(
					"Type " + m_type->toString() + " is only valid in storage."
				));
	}
	else if (getVisibility() >= Visibility::Public && !FunctionType(*this).externalType())
		BOOST_THROW_EXCEPTION(createTypeError("Internal type is not allowed for public state variables."));
}

bool VariableDeclaration::isCallableParameter() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(getScope());
	if (!callable)
		return false;
	for (auto const& variable: callable->getParameters())
		if (variable.get() == this)
			return true;
	if (callable->getReturnParameterList())
		for (auto const& variable: callable->getReturnParameterList()->getParameters())
			if (variable.get() == this)
				return true;
	return false;
}

bool VariableDeclaration::isExternalCallableParameter() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(getScope());
	if (!callable || callable->getVisibility() != Declaration::Visibility::External)
		return false;
	for (auto const& variable: callable->getParameters())
		if (variable.get() == this)
			return true;
	return false;
}

TypePointer ModifierDefinition::getType(ContractDefinition const*) const
{
	return make_shared<ModifierType>(*this);
}

void ModifierDefinition::checkTypeRequirements()
{
	m_body->checkTypeRequirements();
}

void ModifierInvocation::checkTypeRequirements(vector<ContractDefinition const*> const& _bases)
{
	TypePointers argumentTypes;
	for (ASTPointer<Expression> const& argument: m_arguments)
	{
		argument->checkTypeRequirements(nullptr);
		argumentTypes.push_back(argument->getType());
	}
	m_modifierName->checkTypeRequirements(&argumentTypes);

	auto const* declaration = &m_modifierName->getReferencedDeclaration();
	vector<ASTPointer<VariableDeclaration>> emptyParameterList;
	vector<ASTPointer<VariableDeclaration>> const* parameters = nullptr;
	if (auto modifier = dynamic_cast<ModifierDefinition const*>(declaration))
		parameters = &modifier->getParameters();
	else
		// check parameters for Base constructors
		for (ContractDefinition const* base: _bases)
			if (declaration == base)
			{
				if (auto referencedConstructor = base->getConstructor())
					parameters = &referencedConstructor->getParameters();
				else
					parameters = &emptyParameterList;
				break;
			}
	if (!parameters)
		BOOST_THROW_EXCEPTION(createTypeError("Referenced declaration is neither modifier nor base class."));
	if (parameters->size() != m_arguments.size())
		BOOST_THROW_EXCEPTION(createTypeError(
			"Wrong argument count for modifier invocation: " +
			toString(m_arguments.size()) +
			" arguments given but expected " +
			toString(parameters->size()) +
			"."
		));
	for (size_t i = 0; i < m_arguments.size(); ++i)
		if (!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*(*parameters)[i]->getType()))
			BOOST_THROW_EXCEPTION(m_arguments[i]->createTypeError(
				"Invalid type for argument in modifier invocation. "
				"Invalid implicit conversion from " +
				m_arguments[i]->getType()->toString() +
				" to " +
				(*parameters)[i]->getType()->toString() +
				" requested."
			));
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
		if (!var->getType()->externalType())
			BOOST_THROW_EXCEPTION(var->createTypeError("Internal type is not allowed as event parameter type."));
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

void VariableDeclarationStatement::checkTypeRequirements()
{
	m_variable->checkTypeRequirements();
}

void Assignment::checkTypeRequirements(TypePointers const*)
{
	m_leftHandSide->checkTypeRequirements(nullptr);
	m_leftHandSide->requireLValue();
	if (m_leftHandSide->getType()->getCategory() == Type::Category::Mapping)
		BOOST_THROW_EXCEPTION(createTypeError("Mappings cannot be assigned to."));
	m_type = m_leftHandSide->getType();
	if (m_assigmentOperator == Token::Assign)
		m_rightHandSide->expectType(*m_type);
	else
	{
		// compound assignment
		m_rightHandSide->checkTypeRequirements(nullptr);
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
	m_expression->checkTypeRequirements(nullptr);
	if (m_expression->getType()->getCategory() == Type::Category::IntegerConstant)
		if (!dynamic_pointer_cast<IntegerConstantType const>(m_expression->getType())->getIntegerType())
			BOOST_THROW_EXCEPTION(m_expression->createTypeError("Invalid integer constant."));
}

void Expression::expectType(Type const& _expectedType)
{
	checkTypeRequirements(nullptr);
	Type const& type = *getType();
	if (!type.isImplicitlyConvertibleTo(_expectedType))
		BOOST_THROW_EXCEPTION(createTypeError(
			"Type " +
			type.toString() +
			" is not implicitly convertible to expected type " +
			_expectedType.toString() +
			"."
		));
}

void Expression::requireLValue()
{
	if (!isLValue())
		BOOST_THROW_EXCEPTION(createTypeError("Expression has to be an lvalue."));
	m_lvalueRequested = true;
}

void UnaryOperation::checkTypeRequirements(TypePointers const*)
{
	// Inc, Dec, Add, Sub, Not, BitNot, Delete
	m_subExpression->checkTypeRequirements(nullptr);
	if (m_operator == Token::Value::Inc || m_operator == Token::Value::Dec || m_operator == Token::Value::Delete)
		m_subExpression->requireLValue();
	m_type = m_subExpression->getType()->unaryOperatorResult(m_operator);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Unary operator not compatible with type."));
}

void BinaryOperation::checkTypeRequirements(TypePointers const*)
{
	m_left->checkTypeRequirements(nullptr);
	m_right->checkTypeRequirements(nullptr);
	m_commonType = m_left->getType()->binaryOperatorResult(m_operator, m_right->getType());
	if (!m_commonType)
		BOOST_THROW_EXCEPTION(createTypeError("Operator " + string(Token::toString(m_operator)) +
											  " not compatible with types " +
											  m_left->getType()->toString() + " and " +
											  m_right->getType()->toString()));
	m_type = Token::isCompareOp(m_operator) ? make_shared<BoolType>() : m_commonType;
}

void FunctionCall::checkTypeRequirements(TypePointers const*)
{
	bool isPositionalCall = m_names.empty();

	// we need to check arguments' type first as they will be forwarded to
	// m_expression->checkTypeRequirements
	TypePointers argumentTypes;
	for (ASTPointer<Expression> const& argument: m_arguments)
	{
		argument->checkTypeRequirements(nullptr);
		// only store them for positional calls
		if (isPositionalCall)
			argumentTypes.push_back(argument->getType());
	}

	m_expression->checkTypeRequirements(isPositionalCall ? &argumentTypes : nullptr);

	TypePointer const& expressionType = m_expression->getType();
	FunctionTypePointer functionType;
	if (isTypeConversion())
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*expressionType);
		if (m_arguments.size() != 1)
			BOOST_THROW_EXCEPTION(createTypeError("Exactly one argument expected for explicit type conversion."));
		if (!isPositionalCall)
			BOOST_THROW_EXCEPTION(createTypeError("Type conversion cannot allow named arguments."));
		m_type = type.getActualType();
		auto argType = m_arguments.front()->getType();
		if (auto argRefType = dynamic_cast<ReferenceType const*>(argType.get()))
			// do not change the data location when converting
			// (data location cannot yet be specified for type conversions)
			m_type = ReferenceType::copyForLocationIfReference(argRefType->location(), m_type);
		if (!argType->isExplicitlyConvertibleTo(*m_type))
			BOOST_THROW_EXCEPTION(createTypeError("Explicit type conversion not allowed."));

		return;
	}

	/// For error message: Struct members that were removed during conversion to memory.
	set<string> membersRemovedForStructConstructor;
	if (isStructConstructorCall())
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*expressionType);
		auto const& structType = dynamic_cast<StructType const&>(*type.getActualType());
		functionType = structType.constructorType();
		membersRemovedForStructConstructor = structType.membersMissingInMemory();
	}
	else
		functionType = dynamic_pointer_cast<FunctionType const>(expressionType);

	if (!functionType)
		BOOST_THROW_EXCEPTION(createTypeError("Type is not callable."));

	//@todo would be nice to create a struct type from the arguments
	// and then ask if that is implicitly convertible to the struct represented by the
	// function parameters
	TypePointers const& parameterTypes = functionType->getParameterTypes();
	if (!functionType->takesArbitraryParameters() && parameterTypes.size() != m_arguments.size())
	{
		string msg =
			"Wrong argument count for function call: " +
			toString(m_arguments.size()) +
			" arguments given but expected " +
			toString(parameterTypes.size()) +
			".";
		// Extend error message in case we try to construct a struct with mapping member.
		if (isStructConstructorCall() && !membersRemovedForStructConstructor.empty())
		{
			msg += " Members that have to be skipped in memory:";
			for (auto const& member: membersRemovedForStructConstructor)
				msg += " " + member;
		}
		BOOST_THROW_EXCEPTION(createTypeError(msg));
	}

	if (isPositionalCall)
	{
		// call by positional arguments
		for (size_t i = 0; i < m_arguments.size(); ++i)
			if (
				!functionType->takesArbitraryParameters() &&
				!m_arguments[i]->getType()->isImplicitlyConvertibleTo(*parameterTypes[i])
			)
				BOOST_THROW_EXCEPTION(m_arguments[i]->createTypeError(
					"Invalid type for argument in function call. "
					"Invalid implicit conversion from " +
					m_arguments[i]->getType()->toString() +
					" to " +
					parameterTypes[i]->toString() +
					" requested."
				));
	}
	else
	{
		// call by named arguments
		if (functionType->takesArbitraryParameters())
			BOOST_THROW_EXCEPTION(createTypeError(
				"Named arguments cannnot be used for functions that take arbitrary parameters."
			));
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
						BOOST_THROW_EXCEPTION(m_arguments[i]->createTypeError(
							"Invalid type for argument in function call. "
							"Invalid implicit conversion from " +
							m_arguments[i]->getType()->toString() +
							" to " +
							parameterTypes[i]->toString() +
							" requested."
						));

					found = true;
					break;
				}
			}
			if (!found)
				BOOST_THROW_EXCEPTION(createTypeError("Named argument does not match function declaration."));
		}
	}

	// @todo actually the return type should be an anonymous struct,
	// but we change it to the type of the first return value until we have anonymous
	// structs and tuples
	if (functionType->getReturnParameterTypes().empty())
		m_type = make_shared<VoidType>();
	else
		m_type = functionType->getReturnParameterTypes().front();
}

bool FunctionCall::isTypeConversion() const
{
	return m_expression->getType()->getCategory() == Type::Category::TypeType && !isStructConstructorCall();
}

bool FunctionCall::isStructConstructorCall() const
{
	if (auto const* type = dynamic_cast<TypeType const*>(m_expression->getType().get()))
		return type->getActualType()->getCategory() == Type::Category::Struct;
	else
		return false;
}

void NewExpression::checkTypeRequirements(TypePointers const*)
{
	m_contractName->checkTypeRequirements(nullptr);
	m_contract = dynamic_cast<ContractDefinition const*>(&m_contractName->getReferencedDeclaration());
	if (!m_contract)
		BOOST_THROW_EXCEPTION(createTypeError("Identifier is not a contract."));
	if (!m_contract->isFullyImplemented())
		BOOST_THROW_EXCEPTION(createTypeError("Trying to create an instance of an abstract contract."));
	shared_ptr<ContractType const> contractType = make_shared<ContractType>(*m_contract);
	TypePointers const& parameterTypes = contractType->getConstructorType()->getParameterTypes();
	m_type = make_shared<FunctionType>(
		parameterTypes,
		TypePointers{contractType},
		strings(),
		strings(),
		FunctionType::Location::Creation);
}

void MemberAccess::checkTypeRequirements(TypePointers const* _argumentTypes)
{
	m_expression->checkTypeRequirements(nullptr);
	Type const& type = *m_expression->getType();

	MemberList::MemberMap possibleMembers = type.getMembers().membersByName(*m_memberName);
	if (possibleMembers.size() > 1 && _argumentTypes)
	{
		// do override resolution
		for (auto it = possibleMembers.begin(); it != possibleMembers.end();)
			if (
				it->type->getCategory() == Type::Category::Function &&
				!dynamic_cast<FunctionType const&>(*it->type).canTakeArguments(*_argumentTypes)
			)
				it = possibleMembers.erase(it);
			else
				++it;
	}
	if (possibleMembers.size() == 0)
	{
		auto storageType = ReferenceType::copyForLocationIfReference(
			DataLocation::Storage,
			m_expression->getType()
		);
		if (!storageType->getMembers().membersByName(*m_memberName).empty())
			BOOST_THROW_EXCEPTION(createTypeError(
				"Member \"" + *m_memberName + "\" is not available in " +
				type.toString() +
				" outside of storage."
			));
		BOOST_THROW_EXCEPTION(createTypeError(
			"Member \"" + *m_memberName + "\" not found or not visible "
			"after argument-dependent lookup in " + type.toString()
		));
	}
	else if (possibleMembers.size() > 1)
		BOOST_THROW_EXCEPTION(createTypeError(
			"Member \"" + *m_memberName + "\" not unique "
			"after argument-dependent lookup in " + type.toString()
		));

	m_referencedDeclaration = possibleMembers.front().declaration;
	m_type = possibleMembers.front().type;
	if (type.getCategory() == Type::Category::Struct)
		m_isLValue = true;
	else if (type.getCategory() == Type::Category::Array)
	{
		auto const& arrayType(dynamic_cast<ArrayType const&>(type));
		m_isLValue = (
			*m_memberName == "length" &&
			arrayType.location() == DataLocation::Storage &&
			arrayType.isDynamicallySized()
		);
	}
	else
		m_isLValue = false;
}

void IndexAccess::checkTypeRequirements(TypePointers const*)
{
	m_base->checkTypeRequirements(nullptr);
	switch (m_base->getType()->getCategory())
	{
	case Type::Category::Array:
	{
		ArrayType const& type = dynamic_cast<ArrayType const&>(*m_base->getType());
		if (!m_index)
			BOOST_THROW_EXCEPTION(createTypeError("Index expression cannot be omitted."));
		if (type.isString())
			BOOST_THROW_EXCEPTION(createTypeError("Index access for string is not possible."));
		m_index->expectType(IntegerType(256));
		if (type.isByteArray())
			m_type = make_shared<FixedBytesType>(1);
		else
			m_type = type.getBaseType();
		m_isLValue = type.location() != DataLocation::CallData;
		break;
	}
	case Type::Category::Mapping:
	{
		MappingType const& type = dynamic_cast<MappingType const&>(*m_base->getType());
		if (!m_index)
			BOOST_THROW_EXCEPTION(createTypeError("Index expression cannot be omitted."));
		m_index->expectType(*type.getKeyType());
		m_type = type.getValueType();
		m_isLValue = true;
		break;
	}
	case Type::Category::TypeType:
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*m_base->getType());
		if (!m_index)
			m_type = make_shared<TypeType>(make_shared<ArrayType>(DataLocation::Memory, type.getActualType()));
		else
		{
			m_index->checkTypeRequirements(nullptr);
			auto length = dynamic_cast<IntegerConstantType const*>(m_index->getType().get());
			if (!length)
				BOOST_THROW_EXCEPTION(m_index->createTypeError("Integer constant expected."));
			m_type = make_shared<TypeType>(make_shared<ArrayType>(
				DataLocation::Memory, type.getActualType(),
				length->literalValue(nullptr)
			));
		}
		break;
	}
	default:
		BOOST_THROW_EXCEPTION(m_base->createTypeError(
			"Indexed expression has to be a type, mapping or array (is " + m_base->getType()->toString() + ")"));
	}
}

void Identifier::checkTypeRequirements(TypePointers const* _argumentTypes)
{
	if (!m_referencedDeclaration)
	{
		if (!_argumentTypes)
			BOOST_THROW_EXCEPTION(createTypeError("Unable to determine overloaded type."));
		overloadResolution(*_argumentTypes);
	}
	solAssert(!!m_referencedDeclaration, "Referenced declaration is null after overload resolution.");
	m_isLValue = m_referencedDeclaration->isLValue();
	m_type = m_referencedDeclaration->getType(m_currentContract);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Declaration referenced before type could be determined."));
}

Declaration const& Identifier::getReferencedDeclaration() const
{
	solAssert(!!m_referencedDeclaration, "Identifier not resolved.");
	return *m_referencedDeclaration;
}

void Identifier::overloadResolution(TypePointers const& _argumentTypes)
{
	solAssert(!m_referencedDeclaration, "Referenced declaration should be null before overload resolution.");
	solAssert(!m_overloadedDeclarations.empty(), "No candidates for overload resolution found.");

	vector<Declaration const*> possibles;
	if (m_overloadedDeclarations.size() == 1)
		m_referencedDeclaration = *m_overloadedDeclarations.begin();

	for (Declaration const* declaration: m_overloadedDeclarations)
	{
		TypePointer const& function = declaration->getType();
		auto const* functionType = dynamic_cast<FunctionType const*>(function.get());
		if (functionType && functionType->canTakeArguments(_argumentTypes))
			possibles.push_back(declaration);
	}
	if (possibles.size() == 1)
		m_referencedDeclaration = possibles.front();
	else if (possibles.empty())
		BOOST_THROW_EXCEPTION(createTypeError("No matching declaration found after argument-dependent lookup."));
	else
		BOOST_THROW_EXCEPTION(createTypeError("No unique declaration found after argument-dependent lookup."));
}

void ElementaryTypeNameExpression::checkTypeRequirements(TypePointers const*)
{
	m_type = make_shared<TypeType>(Type::fromElementaryTypeName(m_typeToken));
}

void Literal::checkTypeRequirements(TypePointers const*)
{
	m_type = Type::forLiteral(*this);
	if (!m_type)
		BOOST_THROW_EXCEPTION(createTypeError("Invalid literal value."));
}

}
}
