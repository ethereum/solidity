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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity abstract syntax tree.
 */

#include <libsolidity/ast/AST.h>

#include <libsolidity/ast/CallGraph.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/ast/AST_accept.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolutil/Keccak256.h>

#include <boost/algorithm/string.hpp>

#include <functional>
#include <utility>

using namespace std;
using namespace solidity;
using namespace solidity::frontend;

ASTNode::ASTNode(int64_t _id, SourceLocation _location):
	m_id(static_cast<size_t>(_id)),
	m_location(std::move(_location))
{
}

Declaration const* ASTNode::referencedDeclaration(Expression const& _expression)
{
	if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_expression))
		return memberAccess->annotation().referencedDeclaration;
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(&_expression))
		return identifierPath->annotation().referencedDeclaration;
	else if (auto const* identifier = dynamic_cast<Identifier const*>(&_expression))
		return identifier->annotation().referencedDeclaration;
	else
		return nullptr;
}

FunctionDefinition const* ASTNode::resolveFunctionCall(FunctionCall const& _functionCall, ContractDefinition const* _mostDerivedContract)
{
	auto const* functionDef = dynamic_cast<FunctionDefinition const*>(
		ASTNode::referencedDeclaration(_functionCall.expression())
	);

	if (!functionDef)
		return nullptr;

	if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(&_functionCall.expression()))
	{
		if (*memberAccess->annotation().requiredLookup == VirtualLookup::Super)
		{
			if (auto const typeType = dynamic_cast<TypeType const*>(memberAccess->expression().annotation().type))
				if (auto const contractType = dynamic_cast<ContractType const*>(typeType->actualType()))
				{
					solAssert(_mostDerivedContract, "");
					solAssert(contractType->isSuper(), "");
					ContractDefinition const* superContract = contractType->contractDefinition().superContract(*_mostDerivedContract);

					return &functionDef->resolveVirtual(
						*_mostDerivedContract,
						superContract
					);
				}
		}
		else
			solAssert(*memberAccess->annotation().requiredLookup == VirtualLookup::Static, "");
	}
	else if (auto const* identifier = dynamic_cast<Identifier const*>(&_functionCall.expression()))
	{
		solAssert(*identifier->annotation().requiredLookup == VirtualLookup::Virtual, "");
		if (functionDef->virtualSemantics())
		{
			solAssert(_mostDerivedContract, "");
			return &functionDef->resolveVirtual(*_mostDerivedContract);
		}
	}
	else
		solAssert(false, "");

	return functionDef;
}

ASTAnnotation& ASTNode::annotation() const
{
	if (!m_annotation)
		m_annotation = make_unique<ASTAnnotation>();
	return *m_annotation;
}

SourceUnitAnnotation& SourceUnit::annotation() const
{
	return initAnnotation<SourceUnitAnnotation>();
}

set<SourceUnit const*> SourceUnit::referencedSourceUnits(bool _recurse, set<SourceUnit const*> _skipList) const
{
	set<SourceUnit const*> sourceUnits;
	for (ImportDirective const* importDirective: filteredNodes<ImportDirective>(nodes()))
	{
		auto const& sourceUnit = importDirective->annotation().sourceUnit;
		if (!_skipList.count(sourceUnit))
		{
			_skipList.insert(sourceUnit);
			sourceUnits.insert(sourceUnit);
			if (_recurse)
				sourceUnits += sourceUnit->referencedSourceUnits(true, _skipList);
		}
	}
	return sourceUnits;
}

ImportAnnotation& ImportDirective::annotation() const
{
	return initAnnotation<ImportAnnotation>();
}

Type const* ImportDirective::type() const
{
	solAssert(!!annotation().sourceUnit, "");
	return TypeProvider::module(*annotation().sourceUnit);
}

bool ContractDefinition::derivesFrom(ContractDefinition const& _base) const
{
	return util::contains(annotation().linearizedBaseContracts, &_base);
}

map<util::FixedHash<4>, FunctionTypePointer> ContractDefinition::interfaceFunctions(bool _includeInheritedFunctions) const
{
	auto exportedFunctionList = interfaceFunctionList(_includeInheritedFunctions);

	map<util::FixedHash<4>, FunctionTypePointer> exportedFunctions;
	for (auto const& it: exportedFunctionList)
		exportedFunctions.insert(it);

	solAssert(
		exportedFunctionList.size() == exportedFunctions.size(),
		"Hash collision at Function Definition Hash calculation"
	);

	return exportedFunctions;
}

FunctionDefinition const* ContractDefinition::constructor() const
{
	for (FunctionDefinition const* f: definedFunctions())
		if (f->isConstructor())
			return f;
	return nullptr;
}

bool ContractDefinition::canBeDeployed() const
{
	return !abstract() && !isInterface();
}

FunctionDefinition const* ContractDefinition::fallbackFunction() const
{
	for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		for (FunctionDefinition const* f: contract->definedFunctions())
			if (f->isFallback())
				return f;
	return nullptr;
}

FunctionDefinition const* ContractDefinition::receiveFunction() const
{
	for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		for (FunctionDefinition const* f: contract->definedFunctions())
			if (f->isReceive())
				return f;
	return nullptr;
}

vector<EventDefinition const*> const& ContractDefinition::interfaceEvents() const
{
	return m_interfaceEvents.init([&]{
		set<string> eventsSeen;
		vector<EventDefinition const*> interfaceEvents;

		for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
			for (EventDefinition const* e: contract->events())
			{
				/// NOTE: this requires the "internal" version of an Event,
				///       though here internal strictly refers to visibility,
				///       and not to function encoding (jump vs. call)
				auto const& function = e->functionType(true);
				solAssert(function, "");
				string eventSignature = function->externalSignature();
				if (eventsSeen.count(eventSignature) == 0)
				{
					eventsSeen.insert(eventSignature);
					interfaceEvents.push_back(e);
				}
			}

		return interfaceEvents;
	});
}

vector<ErrorDefinition const*> ContractDefinition::interfaceErrors(bool _requireCallGraph) const
{
	set<ErrorDefinition const*, CompareByID> result;
	for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		result += filteredNodes<ErrorDefinition>(contract->m_subNodes);
	solAssert(annotation().creationCallGraph.set() == annotation().deployedCallGraph.set(), "");
	if (_requireCallGraph)
		solAssert(annotation().creationCallGraph.set(), "");
	if (annotation().creationCallGraph.set())
	{
		result += (*annotation().creationCallGraph)->usedErrors;
		result += (*annotation().deployedCallGraph)->usedErrors;
	}
	return convertContainer<vector<ErrorDefinition const*>>(move(result));
}

vector<pair<util::FixedHash<4>, FunctionTypePointer>> const& ContractDefinition::interfaceFunctionList(bool _includeInheritedFunctions) const
{
	return m_interfaceFunctionList[_includeInheritedFunctions].init([&]{
		set<string> signaturesSeen;
		vector<pair<util::FixedHash<4>, FunctionTypePointer>> interfaceFunctionList;

		for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		{
			if (_includeInheritedFunctions == false && contract != this)
				continue;
			vector<FunctionTypePointer> functions;
			for (FunctionDefinition const* f: contract->definedFunctions())
				if (f->isPartOfExternalInterface())
					functions.push_back(TypeProvider::function(*f, FunctionType::Kind::External));
			for (VariableDeclaration const* v: contract->stateVariables())
				if (v->isPartOfExternalInterface())
					functions.push_back(TypeProvider::function(*v));
			for (FunctionTypePointer const& fun: functions)
			{
				if (!fun->interfaceFunctionType())
					// Fails hopefully because we already registered the error
					continue;
				string functionSignature = fun->externalSignature();
				if (signaturesSeen.count(functionSignature) == 0)
				{
					signaturesSeen.insert(functionSignature);
					util::FixedHash<4> hash(util::keccak256(functionSignature));
					interfaceFunctionList.emplace_back(hash, fun);
				}
			}
		}

		return interfaceFunctionList;
	});
}

uint32_t ContractDefinition::interfaceId() const
{
	uint32_t result{0};
	for (auto const& function: interfaceFunctionList(false))
		result ^= fromBigEndian<uint32_t>(function.first.ref());
	return result;
}

Type const* ContractDefinition::type() const
{
	return TypeProvider::typeType(TypeProvider::contract(*this));
}

ContractDefinitionAnnotation& ContractDefinition::annotation() const
{
	return initAnnotation<ContractDefinitionAnnotation>();
}

ContractDefinition const* ContractDefinition::superContract(ContractDefinition const& _mostDerivedContract) const
{
	auto const& hierarchy = _mostDerivedContract.annotation().linearizedBaseContracts;
	auto it = find(hierarchy.begin(), hierarchy.end(), this);
	solAssert(it != hierarchy.end(), "Base not found in inheritance hierarchy.");
	++it;
	if (it == hierarchy.end())
		return nullptr;
	else
	{
		solAssert(*it != this, "");
		return *it;
	}
}

FunctionDefinition const* ContractDefinition::nextConstructor(ContractDefinition const& _mostDerivedContract) const
{
	ContractDefinition const* next = superContract(_mostDerivedContract);
	if (next == nullptr)
		return nullptr;
	for (ContractDefinition const* c: _mostDerivedContract.annotation().linearizedBaseContracts)
		if (c == next || next == nullptr)
		{
			if (c->constructor())
				return c->constructor();
			next = nullptr;
		}

	return nullptr;
}

multimap<std::string, FunctionDefinition const*> const& ContractDefinition::definedFunctionsByName() const
{
	return m_definedFunctionsByName.init([&]{
		std::multimap<std::string, FunctionDefinition const*> result;
		for (FunctionDefinition const* fun: filteredNodes<FunctionDefinition>(m_subNodes))
			result.insert({fun->name(), fun});
		return result;
	});
}


TypeNameAnnotation& TypeName::annotation() const
{
	return initAnnotation<TypeNameAnnotation>();
}

Type const* UserDefinedValueTypeDefinition::type() const
{
	solAssert(m_underlyingType->annotation().type, "");
	return TypeProvider::typeType(TypeProvider::userDefinedValueType(*this));
}

TypeDeclarationAnnotation& UserDefinedValueTypeDefinition::annotation() const
{
	return initAnnotation<TypeDeclarationAnnotation>();
}

Type const* StructDefinition::type() const
{
	solAssert(annotation().recursive.has_value(), "Requested struct type before DeclarationTypeChecker.");
	return TypeProvider::typeType(TypeProvider::structType(*this, DataLocation::Storage));
}

StructDeclarationAnnotation& StructDefinition::annotation() const
{
	return initAnnotation<StructDeclarationAnnotation>();
}

Type const* EnumValue::type() const
{
	auto parentDef = dynamic_cast<EnumDefinition const*>(scope());
	solAssert(parentDef, "Enclosing Scope of EnumValue was not set");
	return TypeProvider::enumType(*parentDef);
}

Type const* EnumDefinition::type() const
{
	return TypeProvider::typeType(TypeProvider::enumType(*this));
}

TypeDeclarationAnnotation& EnumDefinition::annotation() const
{
	return initAnnotation<TypeDeclarationAnnotation>();
}

bool FunctionDefinition::libraryFunction() const
{
	if (auto const* contractDef = dynamic_cast<ContractDefinition const*>(scope()))
		return contractDef->isLibrary();
	return false;
}

Visibility FunctionDefinition::defaultVisibility() const
{
	solAssert(!isConstructor(), "");
	return isFree() ? Visibility::Internal : Declaration::defaultVisibility();
}

FunctionTypePointer FunctionDefinition::functionType(bool _internal) const
{
	if (_internal)
	{
		switch (visibility())
		{
		case Visibility::Default:
			solAssert(false, "visibility() should not return Default");
		case Visibility::Private:
		case Visibility::Internal:
		case Visibility::Public:
			return TypeProvider::function(*this, FunctionType::Kind::Internal);
		case Visibility::External:
			return {};
		}
	}
	else
	{
		switch (visibility())
		{
		case Visibility::Default:
			solAssert(false, "visibility() should not return Default");
		case Visibility::Private:
		case Visibility::Internal:
			return {};
		case Visibility::Public:
		case Visibility::External:
			return TypeProvider::function(*this, FunctionType::Kind::External);
		}
	}

	// To make the compiler happy
	return {};
}

Type const* FunctionDefinition::type() const
{
	solAssert(visibility() != Visibility::External, "");
	return TypeProvider::function(*this, FunctionType::Kind::Internal);
}

Type const* FunctionDefinition::typeViaContractName() const
{
	if (libraryFunction())
	{
		if (isPublic())
			return FunctionType(*this).asExternallyCallableFunction(true);
		else
			return TypeProvider::function(*this, FunctionType::Kind::Internal);
	}
	else
		return TypeProvider::function(*this, FunctionType::Kind::Declaration);
}

string FunctionDefinition::externalSignature() const
{
	return TypeProvider::function(*this)->externalSignature();
}

string FunctionDefinition::externalIdentifierHex() const
{
	return TypeProvider::function(*this)->externalIdentifierHex();
}

FunctionDefinitionAnnotation& FunctionDefinition::annotation() const
{
	return initAnnotation<FunctionDefinitionAnnotation>();
}

FunctionDefinition const& FunctionDefinition::resolveVirtual(
	ContractDefinition const& _mostDerivedContract,
	ContractDefinition const* _searchStart
) const
{
	solAssert(!isConstructor(), "");
	solAssert(!name().empty(), "");

	// If we are not doing super-lookup and the function is not virtual, we can stop here.
	if (_searchStart == nullptr && !virtualSemantics())
		return *this;

	solAssert(!isFree(), "");
	solAssert(isOrdinary(), "");
	solAssert(!libraryFunction(), "");

	FunctionType const* functionType = TypeProvider::function(*this)->asExternallyCallableFunction(false);

	bool foundSearchStart = (_searchStart == nullptr);
	for (ContractDefinition const* c: _mostDerivedContract.annotation().linearizedBaseContracts)
	{
		if (!foundSearchStart && c != _searchStart)
			continue;
		else
			foundSearchStart = true;

		for (FunctionDefinition const* function: c->definedFunctions(name()))
			if (
				// With super lookup analysis guarantees that there is an implemented function in the chain.
				// With virtual lookup there are valid cases where returning an unimplemented one is fine.
				(function->isImplemented() || _searchStart == nullptr) &&
				FunctionType(*function).asExternallyCallableFunction(false)->hasEqualParameterTypes(*functionType)
			)
				return *function;
	}

	solAssert(false, "Virtual function " + name() + " not found.");
	return *this; // not reached
}

Type const* ModifierDefinition::type() const
{
	return TypeProvider::modifier(*this);
}

ModifierDefinitionAnnotation& ModifierDefinition::annotation() const
{
	return initAnnotation<ModifierDefinitionAnnotation>();
}

ModifierDefinition const& ModifierDefinition::resolveVirtual(
	ContractDefinition const& _mostDerivedContract,
	ContractDefinition const* _searchStart
) const
{
	solAssert(_searchStart == nullptr, "Used super in connection with modifiers.");

	// If we are not doing super-lookup and the modifier is not virtual, we can stop here.
	if (_searchStart == nullptr && !virtualSemantics())
		return *this;

	solAssert(!dynamic_cast<ContractDefinition const&>(*scope()).isLibrary(), "");

	for (ContractDefinition const* c: _mostDerivedContract.annotation().linearizedBaseContracts)
	{
		if (_searchStart != nullptr && c != _searchStart)
			continue;
		_searchStart = nullptr;
		for (ModifierDefinition const* modifier: c->functionModifiers())
			if (modifier->name() == name())
				return *modifier;
	}
	solAssert(false, "Virtual modifier " + name() + " not found.");
	return *this; // not reached
}


Type const* EventDefinition::type() const
{
	return TypeProvider::function(*this);
}

FunctionTypePointer EventDefinition::functionType(bool _internal) const
{
	if (_internal)
		return TypeProvider::function(*this);
	else
		return nullptr;
}

EventDefinitionAnnotation& EventDefinition::annotation() const
{
	return initAnnotation<EventDefinitionAnnotation>();
}

Type const* ErrorDefinition::type() const
{
	return TypeProvider::function(*this);
}

FunctionTypePointer ErrorDefinition::functionType(bool _internal) const
{
	if (_internal)
		return TypeProvider::function(*this);
	else
		return nullptr;
}

ErrorDefinitionAnnotation& ErrorDefinition::annotation() const
{
	return initAnnotation<ErrorDefinitionAnnotation>();
}

SourceUnit const& Scopable::sourceUnit() const
{
	ASTNode const* s = scope();
	solAssert(s, "");
	// will not always be a declaration
	while (dynamic_cast<Scopable const*>(s) && dynamic_cast<Scopable const*>(s)->scope())
		s = dynamic_cast<Scopable const*>(s)->scope();
	return dynamic_cast<SourceUnit const&>(*s);
}

CallableDeclaration const* Scopable::functionOrModifierDefinition() const
{
	ASTNode const* s = scope();
	solAssert(s, "");
	while (dynamic_cast<Scopable const*>(s))
	{
		if (auto funDef = dynamic_cast<FunctionDefinition const*>(s))
			return funDef;
		if (auto modDef = dynamic_cast<ModifierDefinition const*>(s))
			return modDef;
		s = dynamic_cast<Scopable const*>(s)->scope();
	}
	return nullptr;
}

string Scopable::sourceUnitName() const
{
	return *sourceUnit().annotation().path;
}

bool Declaration::isEnumValue() const
{
	solAssert(scope(), "");
	return dynamic_cast<EnumDefinition const*>(scope());
}

bool Declaration::isStructMember() const
{
	solAssert(scope(), "");
	return dynamic_cast<StructDefinition const*>(scope());
}

bool Declaration::isEventOrErrorParameter() const
{
	solAssert(scope(), "");
	return dynamic_cast<EventDefinition const*>(scope()) || dynamic_cast<ErrorDefinition const*>(scope());
}

bool Declaration::isVisibleAsUnqualifiedName() const
{
	if (!scope())
		return true;
	if (isStructMember() || isEnumValue() || isEventOrErrorParameter())
		return false;
	if (auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(scope()))
		if (!functionDefinition->isImplemented())
			return false; // parameter of a function without body
	return true;
}

DeclarationAnnotation& Declaration::annotation() const
{
	return initAnnotation<DeclarationAnnotation>();
}

bool VariableDeclaration::isLValue() const
{
	// Constant declared variables are Read-Only
	return !isConstant();
}

bool VariableDeclaration::isLocalVariable() const
{
	auto s = scope();
	return
		dynamic_cast<FunctionTypeName const*>(s) ||
		dynamic_cast<CallableDeclaration const*>(s) ||
		dynamic_cast<Block const*>(s) ||
		dynamic_cast<TryCatchClause const*>(s) ||
		dynamic_cast<ForStatement const*>(s);
}

bool VariableDeclaration::isCallableOrCatchParameter() const
{
	if (isReturnParameter() || isTryCatchParameter())
		return true;

	vector<ASTPointer<VariableDeclaration>> const* parameters = nullptr;

	if (auto const* funTypeName = dynamic_cast<FunctionTypeName const*>(scope()))
		parameters = &funTypeName->parameterTypes();
	else if (auto const* callable = dynamic_cast<CallableDeclaration const*>(scope()))
		parameters = &callable->parameters();

	if (parameters)
		for (auto const& variable: *parameters)
			if (variable.get() == this)
				return true;
	return false;
}

bool VariableDeclaration::isLocalOrReturn() const
{
	return isReturnParameter() || (isLocalVariable() && !isCallableOrCatchParameter());
}

bool VariableDeclaration::isReturnParameter() const
{
	vector<ASTPointer<VariableDeclaration>> const* returnParameters = nullptr;

	if (auto const* funTypeName = dynamic_cast<FunctionTypeName const*>(scope()))
		returnParameters = &funTypeName->returnParameterTypes();
	else if (auto const* callable = dynamic_cast<CallableDeclaration const*>(scope()))
		if (callable->returnParameterList())
			returnParameters = &callable->returnParameterList()->parameters();

	if (returnParameters)
		for (auto const& variable: *returnParameters)
			if (variable.get() == this)
				return true;
	return false;
}

bool VariableDeclaration::isTryCatchParameter() const
{
	return dynamic_cast<TryCatchClause const*>(scope());
}

bool VariableDeclaration::isExternalCallableParameter() const
{
	if (!isCallableOrCatchParameter())
		return false;

	if (auto const* callable = dynamic_cast<CallableDeclaration const*>(scope()))
		if (callable->visibility() == Visibility::External)
			return !isReturnParameter();

	return false;
}

bool VariableDeclaration::isPublicCallableParameter() const
{
	if (!isCallableOrCatchParameter())
		return false;

	if (auto const* callable = dynamic_cast<CallableDeclaration const*>(scope()))
		if (callable->visibility() == Visibility::Public)
			return !isReturnParameter();

	return false;
}

bool VariableDeclaration::isInternalCallableParameter() const
{
	if (!isCallableOrCatchParameter())
		return false;

	if (auto const* funTypeName = dynamic_cast<FunctionTypeName const*>(scope()))
		return funTypeName->visibility() == Visibility::Internal;
	else if (auto const* callable = dynamic_cast<CallableDeclaration const*>(scope()))
		return callable->visibility() <= Visibility::Internal;
	return false;
}

bool VariableDeclaration::isConstructorParameter() const
{
	if (!isCallableOrCatchParameter())
		return false;
	if (auto const* function = dynamic_cast<FunctionDefinition const*>(scope()))
		return function->isConstructor();
	return false;
}

bool VariableDeclaration::isLibraryFunctionParameter() const
{
	if (!isCallableOrCatchParameter())
		return false;
	if (auto const* funDef = dynamic_cast<FunctionDefinition const*>(scope()))
		return funDef->libraryFunction();
	return false;
}

bool VariableDeclaration::hasReferenceOrMappingType() const
{
	solAssert(typeName().annotation().type, "Can only be called after reference resolution");
	Type const* type = typeName().annotation().type;
	return type->category() == Type::Category::Mapping || dynamic_cast<ReferenceType const*>(type);
}

bool VariableDeclaration::isStateVariable() const
{
	return dynamic_cast<ContractDefinition const*>(scope());
}

bool VariableDeclaration::isFileLevelVariable() const
{
	return dynamic_cast<SourceUnit const*>(scope());
}

set<VariableDeclaration::Location> VariableDeclaration::allowedDataLocations() const
{
	using Location = VariableDeclaration::Location;

	if (!hasReferenceOrMappingType() || isStateVariable() || isEventOrErrorParameter())
		return set<Location>{ Location::Unspecified };
	else if (isCallableOrCatchParameter())
	{
		set<Location> locations{ Location::Memory };
		if (
			isConstructorParameter() ||
			isInternalCallableParameter() ||
			isLibraryFunctionParameter()
		)
			locations.insert(Location::Storage);
		if (!isTryCatchParameter() && !isConstructorParameter())
			locations.insert(Location::CallData);

		return locations;
	}
	else if (isLocalVariable())
		// Further restrictions will be imposed later on.
		return set<Location>{ Location::Memory, Location::Storage, Location::CallData };
	else
		// Struct members etc.
		return set<Location>{ Location::Unspecified };
}

string VariableDeclaration::externalIdentifierHex() const
{
	solAssert(isStateVariable() && isPublic(), "Can only be called for public state variables");
	return TypeProvider::function(*this)->externalIdentifierHex();
}

Type const* VariableDeclaration::type() const
{
	return annotation().type;
}

FunctionTypePointer VariableDeclaration::functionType(bool _internal) const
{
	if (_internal)
		return nullptr;
	switch (visibility())
	{
	case Visibility::Default:
		solAssert(false, "visibility() should not return Default");
	case Visibility::Private:
	case Visibility::Internal:
		return nullptr;
	case Visibility::Public:
	case Visibility::External:
		return TypeProvider::function(*this);
	}

	// To make the compiler happy
	return nullptr;
}

VariableDeclarationAnnotation& VariableDeclaration::annotation() const
{
	return initAnnotation<VariableDeclarationAnnotation>();
}

StatementAnnotation& Statement::annotation() const
{
	return initAnnotation<StatementAnnotation>();
}

InlineAssemblyAnnotation& InlineAssembly::annotation() const
{
	return initAnnotation<InlineAssemblyAnnotation>();
}

BlockAnnotation& Block::annotation() const
{
	return initAnnotation<BlockAnnotation>();
}

TryCatchClauseAnnotation& TryCatchClause::annotation() const
{
	return initAnnotation<TryCatchClauseAnnotation>();
}

ForStatementAnnotation& ForStatement::annotation() const
{
	return initAnnotation<ForStatementAnnotation>();
}

ReturnAnnotation& Return::annotation() const
{
	return initAnnotation<ReturnAnnotation>();
}

ExpressionAnnotation& Expression::annotation() const
{
	return initAnnotation<ExpressionAnnotation>();
}

MemberAccessAnnotation& MemberAccess::annotation() const
{
	return initAnnotation<MemberAccessAnnotation>();
}

BinaryOperationAnnotation& BinaryOperation::annotation() const
{
	return initAnnotation<BinaryOperationAnnotation>();
}

FunctionCallAnnotation& FunctionCall::annotation() const
{
	return initAnnotation<FunctionCallAnnotation>();
}

vector<ASTPointer<Expression const>> FunctionCall::sortedArguments() const
{
	// normal arguments
	if (m_names.empty())
		return arguments();

	// named arguments
	FunctionTypePointer functionType;
	if (*annotation().kind == FunctionCallKind::StructConstructorCall)
	{
		auto const& type = dynamic_cast<TypeType const&>(*m_expression->annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());
		functionType = structType.constructorType();
	}
	else
		functionType = dynamic_cast<FunctionType const*>(m_expression->annotation().type);

	vector<ASTPointer<Expression const>> sorted;
	for (auto const& parameterName: functionType->parameterNames())
	{
		bool found = false;
		for (size_t j = 0; j < m_names.size() && !found; j++)
			if ((found = (parameterName == *m_names.at(j))))
				// we found the actual parameter position
				sorted.push_back(m_arguments.at(j));
		solAssert(found, "");
	}

	if (!functionType->takesArbitraryParameters())
	{
		solAssert(m_arguments.size() == functionType->parameterTypes().size(), "");
		solAssert(m_arguments.size() == m_names.size(), "");
		solAssert(m_arguments.size() == sorted.size(), "");
	}

	return sorted;
}

IdentifierAnnotation& Identifier::annotation() const
{
	return initAnnotation<IdentifierAnnotation>();
}

ASTString Literal::valueWithoutUnderscores() const
{
	return boost::erase_all_copy(value(), "_");
}

bool Literal::isHexNumber() const
{
	if (token() != Token::Number)
		return false;
	return boost::starts_with(value(), "0x");
}

bool Literal::looksLikeAddress() const
{
	if (subDenomination() != SubDenomination::None)
		return false;

	if (!isHexNumber())
		return false;

	return abs(int(valueWithoutUnderscores().length()) - 42) <= 1;
}

bool Literal::passesAddressChecksum() const
{
	solAssert(isHexNumber(), "Expected hex number");
	return util::passesAddressChecksum(valueWithoutUnderscores(), true);
}

string Literal::getChecksummedAddress() const
{
	solAssert(isHexNumber(), "Expected hex number");
	/// Pad literal to be a proper hex address.
	string address = valueWithoutUnderscores().substr(2);
	if (address.length() > 40)
		return string();
	address.insert(address.begin(), 40 - address.size(), '0');
	return util::getChecksummedAddress(address);
}

TryCatchClause const* TryStatement::successClause() const
{
	solAssert(m_clauses.size() > 0, "");
	return m_clauses[0].get();
}

TryCatchClause const* TryStatement::panicClause() const
{
	for (size_t i = 1; i < m_clauses.size(); ++i)
		if (m_clauses[i]->errorName() == "Panic")
			return m_clauses[i].get();
	return nullptr;
}

TryCatchClause const* TryStatement::errorClause() const
{
	for (size_t i = 1; i < m_clauses.size(); ++i)
		if (m_clauses[i]->errorName() == "Error")
			return m_clauses[i].get();
	return nullptr;
}

TryCatchClause const* TryStatement::fallbackClause() const
{
	for (size_t i = 1; i < m_clauses.size(); ++i)
		if (m_clauses[i]->errorName().empty())
			return m_clauses[i].get();
	return nullptr;
}
