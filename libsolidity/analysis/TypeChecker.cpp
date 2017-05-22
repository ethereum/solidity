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
 * Type analyzer and checker.
 */

#include <libsolidity/analysis/TypeChecker.h>
#include <memory>
#include <boost/range/adaptor/reversed.hpp>
#include <libsolidity/ast/AST.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;


bool TypeChecker::checkTypeRequirements(ASTNode const& _contract)
{
	try
	{
		_contract.accept(*this);
	}
	catch (FatalError const&)
	{
		// We got a fatal error which required to stop further type checking, but we can
		// continue normally from here.
		if (m_errors.empty())
			throw; // Something is weird here, rather throw again.
	}
	return Error::containsOnlyWarnings(m_errors);
}

TypePointer const& TypeChecker::type(Expression const& _expression) const
{
	solAssert(!!_expression.annotation().type, "Type requested but not present.");
	return _expression.annotation().type;
}

TypePointer const& TypeChecker::type(VariableDeclaration const& _variable) const
{
	solAssert(!!_variable.annotation().type, "Type requested but not present.");
	return _variable.annotation().type;
}

bool TypeChecker::visit(ContractDefinition const& _contract)
{
	m_scope = &_contract;

	// We force our own visiting order here. The structs have to be excluded below.
	set<ASTNode const*> visited;
	for (auto const& s: _contract.definedStructs())
		visited.insert(s);
	ASTNode::listAccept(_contract.definedStructs(), *this);
	ASTNode::listAccept(_contract.baseContracts(), *this);

	checkContractDuplicateFunctions(_contract);
	checkContractIllegalOverrides(_contract);
	checkContractAbstractFunctions(_contract);
	checkContractAbstractConstructors(_contract);

	FunctionDefinition const* function = _contract.constructor();
	if (function)
	{
		if (!function->returnParameters().empty())
			typeError(function->returnParameterList()->location(), "Non-empty \"returns\" directive for constructor.");
		if (function->isDeclaredConst())
			typeError(function->location(), "Constructor cannot be defined as constant.");
		if (function->visibility() != FunctionDefinition::Visibility::Public && function->visibility() != FunctionDefinition::Visibility::Internal)
			typeError(function->location(), "Constructor must be public or internal.");
	}

	FunctionDefinition const* fallbackFunction = nullptr;
	for (FunctionDefinition const* function: _contract.definedFunctions())
	{
		if (function->name().empty())
		{
			if (fallbackFunction)
			{
				auto err = make_shared<Error>(Error::Type::DeclarationError);
				*err << errinfo_comment("Only one fallback function is allowed.");
				m_errors.push_back(err);
			}
			else
			{
				fallbackFunction = function;
				if (_contract.isLibrary())
					typeError(fallbackFunction->location(), "Libraries cannot have fallback functions.");
				if (fallbackFunction->isDeclaredConst())
					typeError(fallbackFunction->location(), "Fallback function cannot be declared constant.");
				if (!fallbackFunction->parameters().empty())
					typeError(fallbackFunction->parameterList().location(), "Fallback function cannot take parameters.");
				if (!fallbackFunction->returnParameters().empty())
					typeError(fallbackFunction->returnParameterList()->location(), "Fallback function cannot return values.");
			}
		}
		if (!function->isImplemented())
			_contract.annotation().isFullyImplemented = false;
	}

	for (auto const& n: _contract.subNodes())
		if (!visited.count(n.get()))
			n->accept(*this);

	checkContractExternalTypeClashes(_contract);
	// check for hash collisions in function signatures
	set<FixedHash<4>> hashes;
	for (auto const& it: _contract.interfaceFunctionList())
	{
		FixedHash<4> const& hash = it.first;
		if (hashes.count(hash))
			typeError(
				_contract.location(),
				string("Function signature hash collision for ") + it.second->externalSignature()
			);
		hashes.insert(hash);
	}

	if (_contract.isLibrary())
		checkLibraryRequirements(_contract);

	return false;
}

void TypeChecker::checkContractDuplicateFunctions(ContractDefinition const& _contract)
{
	/// Checks that two functions with the same name defined in this contract have different
	/// argument types and that there is at most one constructor.
	map<string, vector<FunctionDefinition const*>> functions;
	for (FunctionDefinition const* function: _contract.definedFunctions())
		functions[function->name()].push_back(function);

	// Constructor
	if (functions[_contract.name()].size() > 1)
	{
		SecondarySourceLocation ssl;
		auto it = ++functions[_contract.name()].begin();
		for (; it != functions[_contract.name()].end(); ++it)
			ssl.append("Another declaration is here:", (*it)->location());

		auto err = make_shared<Error>(Error(Error::Type::DeclarationError));
		*err <<
			errinfo_sourceLocation(functions[_contract.name()].front()->location()) <<
			errinfo_comment("More than one constructor defined.") <<
			errinfo_secondarySourceLocation(ssl);
		m_errors.push_back(err);
	}
	for (auto const& it: functions)
	{
		vector<FunctionDefinition const*> const& overloads = it.second;
		for (size_t i = 0; i < overloads.size(); ++i)
			for (size_t j = i + 1; j < overloads.size(); ++j)
				if (FunctionType(*overloads[i]).hasEqualArgumentTypes(FunctionType(*overloads[j])))
				{
					auto err = make_shared<Error>(Error(Error::Type::DeclarationError));
					*err <<
						errinfo_sourceLocation(overloads[j]->location()) <<
						errinfo_comment("Function with same name and arguments defined twice.") <<
						errinfo_secondarySourceLocation(SecondarySourceLocation().append(
							"Other declaration is here:", overloads[i]->location()));
					m_errors.push_back(err);
				}
	}
}

void TypeChecker::checkContractAbstractFunctions(ContractDefinition const& _contract)
{
	// Mapping from name to function definition (exactly one per argument type equality class) and
	// flag to indicate whether it is fully implemented.
	using FunTypeAndFlag = std::pair<FunctionTypePointer, bool>;
	map<string, vector<FunTypeAndFlag>> functions;

	bool allBaseConstructorsImplemented = true;
	// Search from base to derived
	for (ContractDefinition const* contract: boost::adaptors::reverse(_contract.annotation().linearizedBaseContracts))
		for (FunctionDefinition const* function: contract->definedFunctions())
		{
			// Take constructors out of overload hierarchy
			if (function->isConstructor())
			{
				if (!function->isImplemented())
					// Base contract's constructor is not fully implemented, no way to get
					// out of this.
					allBaseConstructorsImplemented = false;
				continue;
			}
			auto& overloads = functions[function->name()];
			FunctionTypePointer funType = make_shared<FunctionType>(*function);
			auto it = find_if(overloads.begin(), overloads.end(), [&](FunTypeAndFlag const& _funAndFlag)
			{
				return funType->hasEqualArgumentTypes(*_funAndFlag.first);
			});
			if (it == overloads.end())
				overloads.push_back(make_pair(funType, function->isImplemented()));
			else if (it->second)
			{
				if (!function->isImplemented())
					typeError(function->location(), "Redeclaring an already implemented function as abstract");
			}
			else if (function->isImplemented())
				it->second = true;
		}

	if (!allBaseConstructorsImplemented)
		_contract.annotation().isFullyImplemented = false;

	// Set to not fully implemented if at least one flag is false.
	for (auto const& it: functions)
		for (auto const& funAndFlag: it.second)
			if (!funAndFlag.second)
			{
				_contract.annotation().isFullyImplemented = false;
				return;
			}
}

void TypeChecker::checkContractAbstractConstructors(ContractDefinition const& _contract)
{
	set<ContractDefinition const*> argumentsNeeded;
	// check that we get arguments for all base constructors that need it.
	// If not mark the contract as abstract (not fully implemented)

	vector<ContractDefinition const*> const& bases = _contract.annotation().linearizedBaseContracts;
	for (ContractDefinition const* contract: bases)
		if (FunctionDefinition const* constructor = contract->constructor())
			if (contract != &_contract && !constructor->parameters().empty())
				argumentsNeeded.insert(contract);

	for (ContractDefinition const* contract: bases)
	{
		if (FunctionDefinition const* constructor = contract->constructor())
			for (auto const& modifier: constructor->modifiers())
			{
				auto baseContract = dynamic_cast<ContractDefinition const*>(
					&dereference(*modifier->name())
				);
				if (baseContract)
					argumentsNeeded.erase(baseContract);
			}


		for (ASTPointer<InheritanceSpecifier> const& base: contract->baseContracts())
		{
			auto baseContract = dynamic_cast<ContractDefinition const*>(&dereference(base->name()));
			solAssert(baseContract, "");
			if (!base->arguments().empty())
				argumentsNeeded.erase(baseContract);
		}
	}
	if (!argumentsNeeded.empty())
		_contract.annotation().isFullyImplemented = false;
}

void TypeChecker::checkContractIllegalOverrides(ContractDefinition const& _contract)
{
	// TODO unify this at a later point. for this we need to put the constness and the access specifier
	// into the types
	map<string, vector<FunctionDefinition const*>> functions;
	map<string, ModifierDefinition const*> modifiers;

	// We search from derived to base, so the stored item causes the error.
	for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts)
	{
		for (FunctionDefinition const* function: contract->definedFunctions())
		{
			if (function->isConstructor())
				continue; // constructors can neither be overridden nor override anything
			string const& name = function->name();
			if (modifiers.count(name))
				typeError(modifiers[name]->location(), "Override changes function to modifier.");
			FunctionType functionType(*function);
			// function should not change the return type
			for (FunctionDefinition const* overriding: functions[name])
			{
				FunctionType overridingType(*overriding);
				if (!overridingType.hasEqualArgumentTypes(functionType))
					continue;
				if (
					overriding->visibility() != function->visibility() ||
					overriding->isDeclaredConst() != function->isDeclaredConst() ||
					overriding->isPayable() != function->isPayable() ||
					overridingType != functionType
				)
					typeError(overriding->location(), "Override changes extended function signature.");
			}
			functions[name].push_back(function);
		}
		for (ModifierDefinition const* modifier: contract->functionModifiers())
		{
			string const& name = modifier->name();
			ModifierDefinition const*& override = modifiers[name];
			if (!override)
				override = modifier;
			else if (ModifierType(*override) != ModifierType(*modifier))
				typeError(override->location(), "Override changes modifier signature.");
			if (!functions[name].empty())
				typeError(override->location(), "Override changes modifier to function.");
		}
	}
}

void TypeChecker::checkContractExternalTypeClashes(ContractDefinition const& _contract)
{
	map<string, vector<pair<Declaration const*, FunctionTypePointer>>> externalDeclarations;
	for (ContractDefinition const* contract: _contract.annotation().linearizedBaseContracts)
	{
		for (FunctionDefinition const* f: contract->definedFunctions())
			if (f->isPartOfExternalInterface())
			{
				auto functionType = make_shared<FunctionType>(*f);
				// under non error circumstances this should be true
				if (functionType->interfaceFunctionType())
					externalDeclarations[functionType->externalSignature()].push_back(
						make_pair(f, functionType)
					);
			}
		for (VariableDeclaration const* v: contract->stateVariables())
			if (v->isPartOfExternalInterface())
			{
				auto functionType = make_shared<FunctionType>(*v);
				// under non error circumstances this should be true
				if (functionType->interfaceFunctionType())
					externalDeclarations[functionType->externalSignature()].push_back(
						make_pair(v, functionType)
					);
			}
	}
	for (auto const& it: externalDeclarations)
		for (size_t i = 0; i < it.second.size(); ++i)
			for (size_t j = i + 1; j < it.second.size(); ++j)
				if (!it.second[i].second->hasEqualArgumentTypes(*it.second[j].second))
					typeError(
						it.second[j].first->location(),
						"Function overload clash during conversion to external types for arguments."
					);
}

void TypeChecker::checkLibraryRequirements(ContractDefinition const& _contract)
{
	solAssert(_contract.isLibrary(), "");
	if (!_contract.baseContracts().empty())
		typeError(_contract.location(), "Library is not allowed to inherit.");

	for (auto const& var: _contract.stateVariables())
		if (!var->isConstant())
			typeError(var->location(), "Library cannot have non-constant state variables");
}

void TypeChecker::endVisit(InheritanceSpecifier const& _inheritance)
{
	auto base = dynamic_cast<ContractDefinition const*>(&dereference(_inheritance.name()));
	solAssert(base, "Base contract not available.");

	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
		typeError(_inheritance.location(), "Interfaces cannot inherit.");

	if (base->isLibrary())
		typeError(_inheritance.location(), "Libraries cannot be inherited from.");

	auto const& arguments = _inheritance.arguments();
	TypePointers parameterTypes = ContractType(*base).newExpressionType()->parameterTypes();
	if (!arguments.empty() && parameterTypes.size() != arguments.size())
	{
		typeError(
			_inheritance.location(),
			"Wrong argument count for constructor call: " +
			toString(arguments.size()) +
			" arguments given but expected " +
			toString(parameterTypes.size()) +
			"."
		);
		return;
	}

	for (size_t i = 0; i < arguments.size(); ++i)
		if (!type(*arguments[i])->isImplicitlyConvertibleTo(*parameterTypes[i]))
			typeError(
				arguments[i]->location(),
				"Invalid type for argument in constructor call. "
				"Invalid implicit conversion from " +
				type(*arguments[i])->toString() +
				" to " +
				parameterTypes[i]->toString() +
				" requested."
						);
}

void TypeChecker::endVisit(UsingForDirective const& _usingFor)
{
	ContractDefinition const* library = dynamic_cast<ContractDefinition const*>(
		_usingFor.libraryName().annotation().referencedDeclaration
	);
	if (!library || !library->isLibrary())
		typeError(_usingFor.libraryName().location(), "Library name expected.");
}

bool TypeChecker::visit(StructDefinition const& _struct)
{
	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
		typeError(_struct.location(), "Structs cannot be defined in interfaces.");

	for (ASTPointer<VariableDeclaration> const& member: _struct.members())
		if (!type(*member)->canBeStored())
			typeError(member->location(), "Type cannot be used in struct.");

	// Check recursion, fatal error if detected.
	using StructPointer = StructDefinition const*;
	using StructPointersSet = set<StructPointer>;
	function<void(StructPointer,StructPointersSet const&)> check = [&](StructPointer _struct, StructPointersSet const& _parents)
	{
		if (_parents.count(_struct))
			fatalTypeError(_struct->location(), "Recursive struct definition.");
		StructPointersSet parents = _parents;
		parents.insert(_struct);
		for (ASTPointer<VariableDeclaration> const& member: _struct->members())
			if (type(*member)->category() == Type::Category::Struct)
			{
				auto const& typeName = dynamic_cast<UserDefinedTypeName const&>(*member->typeName());
				check(&dynamic_cast<StructDefinition const&>(*typeName.annotation().referencedDeclaration), parents);
			}
	};
	check(&_struct, StructPointersSet{});

	ASTNode::listAccept(_struct.members(), *this);

	return false;
}

bool TypeChecker::visit(FunctionDefinition const& _function)
{
	bool isLibraryFunction =
		dynamic_cast<ContractDefinition const*>(_function.scope()) &&
		dynamic_cast<ContractDefinition const*>(_function.scope())->isLibrary();
	if (_function.isPayable())
	{
		if (isLibraryFunction)
			typeError(_function.location(), "Library functions cannot be payable.");
		if (!_function.isConstructor() && !_function.name().empty() && !_function.isPartOfExternalInterface())
			typeError(_function.location(), "Internal functions cannot be payable.");
		if (_function.isDeclaredConst())
			typeError(_function.location(), "Functions cannot be constant and payable at the same time.");
	}
	for (ASTPointer<VariableDeclaration> const& var: _function.parameters() + _function.returnParameters())
	{
		if (!type(*var)->canLiveOutsideStorage())
			typeError(var->location(), "Type is required to live outside storage.");
		if (_function.visibility() >= FunctionDefinition::Visibility::Public && !(type(*var)->interfaceType(isLibraryFunction)))
			fatalTypeError(var->location(), "Internal type is not allowed for public or external functions.");
	}
	for (ASTPointer<ModifierInvocation> const& modifier: _function.modifiers())
		visitManually(
			*modifier,
			_function.isConstructor() ?
			dynamic_cast<ContractDefinition const&>(*_function.scope()).annotation().linearizedBaseContracts :
			vector<ContractDefinition const*>()
		);
	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
	{
		if (_function.isImplemented())
			typeError(_function.location(), "Functions in interfaces cannot have an implementation.");
		if (_function.visibility() < FunctionDefinition::Visibility::Public)
			typeError(_function.location(), "Functions in interfaces cannot be internal or private.");
		if (_function.isConstructor())
			typeError(_function.location(), "Constructor cannot be defined in interfaces.");
	}
	if (_function.isImplemented())
		_function.body().accept(*this);
	return false;
}

bool TypeChecker::visit(VariableDeclaration const& _variable)
{
	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
		typeError(_variable.location(), "Variables cannot be declared in interfaces.");

	// Variables can be declared without type (with "var"), in which case the first assignment
	// sets the type.
	// Note that assignments before the first declaration are legal because of the special scoping
	// rules inherited from JavaScript.

	// type is filled either by ReferencesResolver directly from the type name or by
	// TypeChecker at the VariableDeclarationStatement level.
	TypePointer varType = _variable.annotation().type;
	solAssert(!!varType, "Failed to infer variable type.");
	if (_variable.value())
		expectType(*_variable.value(), *varType);
	if (_variable.isConstant())
	{
		if (!_variable.isStateVariable())
			typeError(_variable.location(), "Illegal use of \"constant\" specifier.");
		if (!_variable.type()->isValueType())
		{
			bool allowed = false;
			if (auto arrayType = dynamic_cast<ArrayType const*>(_variable.type().get()))
				allowed = arrayType->isString();
			if (!allowed)
				typeError(_variable.location(), "Constants of non-value type not yet implemented.");
		}
		if (!_variable.value())
			typeError(_variable.location(), "Uninitialized \"constant\" variable.");
		else if (!_variable.value()->annotation().isPure)
			warning(
				_variable.value()->location(),
				"Initial value for constant variable has to be compile-time constant. "
				"This will fail to compile with the next breaking version change."
			);
	}
	if (!_variable.isStateVariable())
	{
		if (varType->dataStoredIn(DataLocation::Memory) || varType->dataStoredIn(DataLocation::CallData))
			if (!varType->canLiveOutsideStorage())
				typeError(_variable.location(), "Type " + varType->toString() + " is only valid in storage.");
	}
	else if (
		_variable.visibility() >= VariableDeclaration::Visibility::Public &&
		!FunctionType(_variable).interfaceFunctionType()
	)
		typeError(_variable.location(), "Internal type is not allowed for public state variables.");
	return false;
}

bool TypeChecker::visit(EnumDefinition const& _enum)
{
	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
		typeError(_enum.location(), "Enumerable cannot be declared in interfaces.");
	return false;
}

void TypeChecker::visitManually(
	ModifierInvocation const& _modifier,
	vector<ContractDefinition const*> const& _bases
)
{
	std::vector<ASTPointer<Expression>> const& arguments = _modifier.arguments();
	for (ASTPointer<Expression> const& argument: arguments)
		argument->accept(*this);
	_modifier.name()->accept(*this);

	auto const* declaration = &dereference(*_modifier.name());
	vector<ASTPointer<VariableDeclaration>> emptyParameterList;
	vector<ASTPointer<VariableDeclaration>> const* parameters = nullptr;
	if (auto modifierDecl = dynamic_cast<ModifierDefinition const*>(declaration))
		parameters = &modifierDecl->parameters();
	else
		// check parameters for Base constructors
		for (ContractDefinition const* base: _bases)
			if (declaration == base)
			{
				if (auto referencedConstructor = base->constructor())
					parameters = &referencedConstructor->parameters();
				else
					parameters = &emptyParameterList;
				break;
			}
	if (!parameters)
	{
		typeError(_modifier.location(), "Referenced declaration is neither modifier nor base class.");
		return;
	}
	if (parameters->size() != arguments.size())
	{
		typeError(
			_modifier.location(),
			"Wrong argument count for modifier invocation: " +
			toString(arguments.size()) +
			" arguments given but expected " +
			toString(parameters->size()) +
			"."
		);
		return;
	}
	for (size_t i = 0; i < _modifier.arguments().size(); ++i)
		if (!type(*arguments[i])->isImplicitlyConvertibleTo(*type(*(*parameters)[i])))
			typeError(
				arguments[i]->location(),
				"Invalid type for argument in modifier invocation. "
				"Invalid implicit conversion from " +
				type(*arguments[i])->toString() +
				" to " +
				type(*(*parameters)[i])->toString() +
				" requested."
			);
}

bool TypeChecker::visit(EventDefinition const& _eventDef)
{
	unsigned numIndexed = 0;
	for (ASTPointer<VariableDeclaration> const& var: _eventDef.parameters())
	{
		if (var->isIndexed())
			numIndexed++;
		if (_eventDef.isAnonymous() && numIndexed > 4)
			typeError(_eventDef.location(), "More than 4 indexed arguments for anonymous event.");
		else if (!_eventDef.isAnonymous() && numIndexed > 3)
			typeError(_eventDef.location(), "More than 3 indexed arguments for event.");
		if (!type(*var)->canLiveOutsideStorage())
			typeError(var->location(), "Type is required to live outside storage.");
		if (!type(*var)->interfaceType(false))
			typeError(var->location(), "Internal type is not allowed as event parameter type.");
	}
	return false;
}

void TypeChecker::endVisit(FunctionTypeName const& _funType)
{
	FunctionType const& fun = dynamic_cast<FunctionType const&>(*_funType.annotation().type);
	if (fun.kind() == FunctionType::Kind::External)
		if (!fun.canBeUsedExternally(false))
			typeError(_funType.location(), "External function type uses internal types.");
}

bool TypeChecker::visit(InlineAssembly const& _inlineAssembly)
{
	// External references have already been resolved in a prior stage and stored in the annotation.
	// We run the resolve step again regardless.
	assembly::ExternalIdentifierAccess::Resolver identifierAccess = [&](
		assembly::Identifier const& _identifier,
		assembly::IdentifierContext _context
	)
	{
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return size_t(-1);
		Declaration const* declaration = ref->second.declaration;
		solAssert(!!declaration, "");
		if (auto var = dynamic_cast<VariableDeclaration const*>(declaration))
		{
			if (ref->second.isSlot || ref->second.isOffset)
			{
				if (!var->isStateVariable() && !var->type()->dataStoredIn(DataLocation::Storage))
				{
					typeError(_identifier.location, "The suffixes _offset and _slot can only be used on storage variables.");
					return size_t(-1);
				}
				else if (_context != assembly::IdentifierContext::RValue)
				{
					typeError(_identifier.location, "Storage variables cannot be assigned to.");
					return size_t(-1);
				}
			}
			else if (var->isConstant())
			{
				typeError(_identifier.location, "Constant variables not supported by inline assembly.");
				return size_t(-1);
			}
			else if (!var->isLocalVariable())
			{
				typeError(_identifier.location, "Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.");
				return size_t(-1);
			}
			else if (var->type()->dataStoredIn(DataLocation::Storage))
			{
				typeError(_identifier.location, "You have to use the _slot or _offset prefix to access storage reference variables.");
				return size_t(-1);
			}
			else if (var->type()->sizeOnStack() != 1)
			{
				typeError(_identifier.location, "Only types that use one stack slot are supported.");
				return size_t(-1);
			}
		}
		else if (_context == assembly::IdentifierContext::LValue)
		{
			typeError(_identifier.location, "Only local variables can be assigned to in inline assembly.");
			return size_t(-1);
		}

		if (_context == assembly::IdentifierContext::RValue)
		{
			solAssert(!!declaration->type(), "Type of declaration required but not yet determined.");
			if (dynamic_cast<FunctionDefinition const*>(declaration))
			{
			}
			else if (dynamic_cast<VariableDeclaration const*>(declaration))
			{
			}
			else if (auto contract = dynamic_cast<ContractDefinition const*>(declaration))
			{
				if (!contract->isLibrary())
				{
					typeError(_identifier.location, "Expected a library.");
					return size_t(-1);
				}
			}
			else
				return size_t(-1);
		}
		ref->second.valueSize = 1;
		return size_t(1);
	};
	solAssert(!_inlineAssembly.annotation().analysisInfo, "");
	_inlineAssembly.annotation().analysisInfo = make_shared<assembly::AsmAnalysisInfo>();
	assembly::AsmAnalyzer analyzer(
		*_inlineAssembly.annotation().analysisInfo,
		m_errors,
		identifierAccess
	);
	if (!analyzer.analyze(_inlineAssembly.operations()))
		return false;
	return true;
}

bool TypeChecker::visit(IfStatement const& _ifStatement)
{
	expectType(_ifStatement.condition(), BoolType());
	_ifStatement.trueStatement().accept(*this);
	if (_ifStatement.falseStatement())
		_ifStatement.falseStatement()->accept(*this);
	return false;
}

bool TypeChecker::visit(WhileStatement const& _whileStatement)
{
	expectType(_whileStatement.condition(), BoolType());
	_whileStatement.body().accept(*this);
	return false;
}

bool TypeChecker::visit(ForStatement const& _forStatement)
{
	if (_forStatement.initializationExpression())
		_forStatement.initializationExpression()->accept(*this);
	if (_forStatement.condition())
		expectType(*_forStatement.condition(), BoolType());
	if (_forStatement.loopExpression())
		_forStatement.loopExpression()->accept(*this);
	_forStatement.body().accept(*this);
	return false;
}

void TypeChecker::endVisit(Return const& _return)
{
	if (!_return.expression())
		return;
	ParameterList const* params = _return.annotation().functionReturnParameters;
	if (!params)
	{
		typeError(_return.location(), "Return arguments not allowed.");
		return;
	}
	TypePointers returnTypes;
	for (auto const& var: params->parameters())
		returnTypes.push_back(type(*var));
	if (auto tupleType = dynamic_cast<TupleType const*>(type(*_return.expression()).get()))
	{
		if (tupleType->components().size() != params->parameters().size())
			typeError(_return.location(), "Different number of arguments in return statement than in returns declaration.");
		else if (!tupleType->isImplicitlyConvertibleTo(TupleType(returnTypes)))
			typeError(
				_return.expression()->location(),
				"Return argument type " +
				type(*_return.expression())->toString() +
				" is not implicitly convertible to expected type " +
				TupleType(returnTypes).toString(false) +
				"."
			);
	}
	else if (params->parameters().size() != 1)
		typeError(_return.location(), "Different number of arguments in return statement than in returns declaration.");
	else
	{
		TypePointer const& expected = type(*params->parameters().front());
		if (!type(*_return.expression())->isImplicitlyConvertibleTo(*expected))
			typeError(
				_return.expression()->location(),
				"Return argument type " +
				type(*_return.expression())->toString() +
				" is not implicitly convertible to expected type (type of first return variable) " +
				expected->toString() +
				"."
			);
	}
}

bool TypeChecker::visit(VariableDeclarationStatement const& _statement)
{
	if (!_statement.initialValue())
	{
		// No initial value is only permitted for single variables with specified type.
		if (_statement.declarations().size() != 1 || !_statement.declarations().front())
			fatalTypeError(_statement.location(), "Assignment necessary for type detection.");
		VariableDeclaration const& varDecl = *_statement.declarations().front();
		if (!varDecl.annotation().type)
			fatalTypeError(_statement.location(), "Assignment necessary for type detection.");
		if (auto ref = dynamic_cast<ReferenceType const*>(type(varDecl).get()))
		{
			if (ref->dataStoredIn(DataLocation::Storage))
				warning(
					varDecl.location(),
					"Uninitialized storage pointer. Did you mean '<type> memory " + varDecl.name() + "'?"
				);
		}
		else if (dynamic_cast<MappingType const*>(type(varDecl).get()))
			typeError(
				varDecl.location(),
				"Uninitialized mapping. Mappings cannot be created dynamically, you have to assign them from a state variable."
			);
		varDecl.accept(*this);
		return false;
	}

	// Here we have an initial value and might have to derive some types before we can visit
	// the variable declaration(s).

	_statement.initialValue()->accept(*this);
	TypePointers valueTypes;
	if (auto tupleType = dynamic_cast<TupleType const*>(type(*_statement.initialValue()).get()))
		valueTypes = tupleType->components();
	else
		valueTypes = TypePointers{type(*_statement.initialValue())};

	// Determine which component is assigned to which variable.
	// If numbers do not match, fill up if variables begin or end empty (not both).
	vector<VariableDeclaration const*>& assignments = _statement.annotation().assignments;
	assignments.resize(valueTypes.size(), nullptr);
	vector<ASTPointer<VariableDeclaration>> const& variables = _statement.declarations();
	if (variables.empty())
	{
		if (!valueTypes.empty())
			fatalTypeError(
				_statement.location(),
				"Too many components (" +
				toString(valueTypes.size()) +
				") in value for variable assignment (0) needed"
			);
	}
	else if (valueTypes.size() != variables.size() && !variables.front() && !variables.back())
		fatalTypeError(
			_statement.location(),
			"Wildcard both at beginning and end of variable declaration list is only allowed "
			"if the number of components is equal."
		);
	size_t minNumValues = variables.size();
	if (!variables.empty() && (!variables.back() || !variables.front()))
		--minNumValues;
	if (valueTypes.size() < minNumValues)
		fatalTypeError(
			_statement.location(),
			"Not enough components (" +
			toString(valueTypes.size()) +
			") in value to assign all variables (" +
			toString(minNumValues) + ")."
		);
	if (valueTypes.size() > variables.size() && variables.front() && variables.back())
		fatalTypeError(
			_statement.location(),
			"Too many components (" +
			toString(valueTypes.size()) +
			") in value for variable assignment (" +
			toString(minNumValues) +
			" needed)."
		);
	bool fillRight = !variables.empty() && (!variables.back() || variables.front());
	for (size_t i = 0; i < min(variables.size(), valueTypes.size()); ++i)
		if (fillRight)
			assignments[i] = variables[i].get();
		else
			assignments[assignments.size() - i - 1] = variables[variables.size() - i - 1].get();

	for (size_t i = 0; i < assignments.size(); ++i)
	{
		if (!assignments[i])
			continue;
		VariableDeclaration const& var = *assignments[i];
		solAssert(!var.value(), "Value has to be tied to statement.");
		TypePointer const& valueComponentType = valueTypes[i];
		solAssert(!!valueComponentType, "");
		if (!var.annotation().type)
		{
			// Infer type from value.
			solAssert(!var.typeName(), "");
			var.annotation().type = valueComponentType->mobileType();
			if (!var.annotation().type)
			{
				if (valueComponentType->category() == Type::Category::RationalNumber)
					fatalTypeError(
						_statement.initialValue()->location(),
						"Invalid rational " +
						valueComponentType->toString() +
						" (absolute value too large or divison by zero)."
					);
				else
					solAssert(false, "");
			}
			else if (*var.annotation().type == TupleType())
				typeError(
					var.location(),
					"Cannot declare variable with void (empty tuple) type."
				);
			var.accept(*this);
		}
		else
		{
			var.accept(*this);
			if (!valueComponentType->isImplicitlyConvertibleTo(*var.annotation().type))
			{
				if (
					valueComponentType->category() == Type::Category::RationalNumber &&
					dynamic_cast<RationalNumberType const&>(*valueComponentType).isFractional() &&
					valueComponentType->mobileType()
				)
					typeError(
						_statement.location(),
						"Type " +
						valueComponentType->toString() +
						" is not implicitly convertible to expected type " +
						var.annotation().type->toString() +
						". Try converting to type " +
						valueComponentType->mobileType()->toString() +
						" or use an explicit conversion." 
					);
				else
					typeError(
						_statement.location(),
						"Type " +
						valueComponentType->toString() +
						" is not implicitly convertible to expected type " +
						var.annotation().type->toString() +
						"."
					);
			}
		}
	}
	return false;
}

void TypeChecker::endVisit(ExpressionStatement const& _statement)
{
	if (type(_statement.expression())->category() == Type::Category::RationalNumber)
		if (!dynamic_cast<RationalNumberType const&>(*type(_statement.expression())).mobileType())
			typeError(_statement.expression().location(), "Invalid rational number.");

	if (auto call = dynamic_cast<FunctionCall const*>(&_statement.expression()))
	{
		if (auto callType = dynamic_cast<FunctionType const*>(type(call->expression()).get()))
		{
			auto kind = callType->kind();
			if (
				kind == FunctionType::Kind::Bare ||
				kind == FunctionType::Kind::BareCallCode ||
				kind == FunctionType::Kind::BareDelegateCall
			)
				warning(_statement.location(), "Return value of low-level calls not used.");
			else if (kind == FunctionType::Kind::Send)
				warning(_statement.location(), "Failure condition of 'send' ignored. Consider using 'transfer' instead.");
		}
	}
}

bool TypeChecker::visit(Conditional const& _conditional)
{
	expectType(_conditional.condition(), BoolType());

	_conditional.trueExpression().accept(*this);
	_conditional.falseExpression().accept(*this);

	TypePointer trueType = type(_conditional.trueExpression())->mobileType();
	TypePointer falseType = type(_conditional.falseExpression())->mobileType();
	if (!trueType)
		fatalTypeError(_conditional.trueExpression().location(), "Invalid mobile type.");
	if (!falseType)
		fatalTypeError(_conditional.falseExpression().location(), "Invalid mobile type.");

	TypePointer commonType = Type::commonType(trueType, falseType);
	if (!commonType)
	{
		typeError(
				_conditional.location(),
				"True expression's type " +
				trueType->toString() +
				" doesn't match false expression's type " +
				falseType->toString() +
				"."
		);
		// even we can't find a common type, we have to set a type here,
		// otherwise the upper statement will not be able to check the type.
		commonType = trueType;
	}

	_conditional.annotation().type = commonType;
	_conditional.annotation().isPure =
		_conditional.condition().annotation().isPure &&
		_conditional.trueExpression().annotation().isPure &&
		_conditional.falseExpression().annotation().isPure;

	if (_conditional.annotation().lValueRequested)
		typeError(
				_conditional.location(),
				"Conditional expression as left value is not supported yet."
		);

	return false;
}

bool TypeChecker::visit(Assignment const& _assignment)
{
	requireLValue(_assignment.leftHandSide());
	TypePointer t = type(_assignment.leftHandSide());
	_assignment.annotation().type = t;
	if (TupleType const* tupleType = dynamic_cast<TupleType const*>(t.get()))
	{
		if (_assignment.assignmentOperator() != Token::Assign)
			typeError(
				_assignment.location(),
				"Compound assignment is not allowed for tuple types."
			);
		// Sequenced assignments of tuples is not valid, make the result a "void" type.
		_assignment.annotation().type = make_shared<TupleType>();
		expectType(_assignment.rightHandSide(), *tupleType);
	}
	else if (t->category() == Type::Category::Mapping)
	{
		typeError(_assignment.location(), "Mappings cannot be assigned to.");
		_assignment.rightHandSide().accept(*this);
	}
	else if (_assignment.assignmentOperator() == Token::Assign)
		expectType(_assignment.rightHandSide(), *t);
	else
	{
		// compound assignment
		_assignment.rightHandSide().accept(*this);
		TypePointer resultType = t->binaryOperatorResult(
			Token::AssignmentToBinaryOp(_assignment.assignmentOperator()),
			type(_assignment.rightHandSide())
		);
		if (!resultType || *resultType != *t)
			typeError(
				_assignment.location(),
				"Operator " +
				string(Token::toString(_assignment.assignmentOperator())) +
				" not compatible with types " +
				t->toString() +
				" and " +
				type(_assignment.rightHandSide())->toString()
			);
	}
	return false;
}

bool TypeChecker::visit(TupleExpression const& _tuple)
{
	vector<ASTPointer<Expression>> const& components = _tuple.components();
	TypePointers types;

	if (_tuple.annotation().lValueRequested)
	{
		if (_tuple.isInlineArray())
			fatalTypeError(_tuple.location(), "Inline array type cannot be declared as LValue.");
		for (auto const& component: components)
			if (component)
			{
				requireLValue(*component);
				types.push_back(type(*component));
			}
			else
				types.push_back(TypePointer());
		if (components.size() == 1)
			_tuple.annotation().type = type(*components[0]);
		else
			_tuple.annotation().type = make_shared<TupleType>(types);
		// If some of the components are not LValues, the error is reported above.
		_tuple.annotation().isLValue = true;
	}
	else
	{
		bool isPure = true;
		TypePointer inlineArrayType;
		for (size_t i = 0; i < components.size(); ++i)
		{
			// Outside of an lvalue-context, the only situation where a component can be empty is (x,).
			if (!components[i] && !(i == 1 && components.size() == 2))
				fatalTypeError(_tuple.location(), "Tuple component cannot be empty.");
			else if (components[i])
			{
				components[i]->accept(*this);
				types.push_back(type(*components[i]));
				if (_tuple.isInlineArray())
					solAssert(!!types[i], "Inline array cannot have empty components");
				if (_tuple.isInlineArray())
				{
					if ((i == 0 || inlineArrayType) && !types[i]->mobileType())
						fatalTypeError(components[i]->location(), "Invalid mobile type.");

					if (i == 0)
						inlineArrayType = types[i]->mobileType();
					else if (inlineArrayType)
						inlineArrayType = Type::commonType(inlineArrayType, types[i]);
				}
				if (!components[i]->annotation().isPure)
					isPure = false;
			}
			else
				types.push_back(TypePointer());
		}
		_tuple.annotation().isPure = isPure;
		if (_tuple.isInlineArray())
		{
			if (!inlineArrayType) 
				fatalTypeError(_tuple.location(), "Unable to deduce common type for array elements.");
			_tuple.annotation().type = make_shared<ArrayType>(DataLocation::Memory, inlineArrayType, types.size());
		}
		else
		{
			if (components.size() == 1)
				_tuple.annotation().type = type(*components[0]);
			else
			{
				if (components.size() == 2 && !components[1])
					types.pop_back();
				_tuple.annotation().type = make_shared<TupleType>(types);
			}
		}

	}
	return false;
}

bool TypeChecker::visit(UnaryOperation const& _operation)
{
	// Inc, Dec, Add, Sub, Not, BitNot, Delete
	Token::Value op = _operation.getOperator();
	bool const modifying = (op == Token::Value::Inc || op == Token::Value::Dec || op == Token::Value::Delete);
	if (modifying)
		requireLValue(_operation.subExpression());
	else
		_operation.subExpression().accept(*this);
	TypePointer const& subExprType = type(_operation.subExpression());
	TypePointer t = type(_operation.subExpression())->unaryOperatorResult(op);
	if (!t)
	{
		typeError(
			_operation.location(),
			"Unary operator " +
			string(Token::toString(op)) +
			" cannot be applied to type " +
			subExprType->toString()
		);
		t = subExprType;
	}
	_operation.annotation().type = t;
	_operation.annotation().isPure = !modifying && _operation.subExpression().annotation().isPure;
	return false;
}

void TypeChecker::endVisit(BinaryOperation const& _operation)
{
	TypePointer const& leftType = type(_operation.leftExpression());
	TypePointer const& rightType = type(_operation.rightExpression());
	TypePointer commonType = leftType->binaryOperatorResult(_operation.getOperator(), rightType);
	if (!commonType)
	{
		typeError(
			_operation.location(),
			"Operator " +
			string(Token::toString(_operation.getOperator())) +
			" not compatible with types " +
			leftType->toString() +
			" and " +
			rightType->toString()
		);
		commonType = leftType;
	}
	_operation.annotation().commonType = commonType;
	_operation.annotation().type =
		Token::isCompareOp(_operation.getOperator()) ?
		make_shared<BoolType>() :
		commonType;
	_operation.annotation().isPure =
		_operation.leftExpression().annotation().isPure &&
		_operation.rightExpression().annotation().isPure;

	if (_operation.getOperator() == Token::Exp)
	{
		if (
			leftType->category() == Type::Category::RationalNumber &&
			rightType->category() != Type::Category::RationalNumber
		)
			if ((
				commonType->category() == Type::Category::Integer &&
				dynamic_cast<IntegerType const&>(*commonType).numBits() != 256
			) || (
				commonType->category() == Type::Category::FixedPoint &&
				dynamic_cast<FixedPointType const&>(*commonType).numBits() != 256
			))
				warning(
					_operation.location(),
					"Result of exponentiation has type " + commonType->toString() + " and thus "
					"might overflow. Silence this warning by converting the literal to the "
					"expected type."
				);
	}
}

bool TypeChecker::visit(FunctionCall const& _functionCall)
{
	bool isPositionalCall = _functionCall.names().empty();
	vector<ASTPointer<Expression const>> arguments = _functionCall.arguments();
	vector<ASTPointer<ASTString>> const& argumentNames = _functionCall.names();

	bool isPure = true;

	// We need to check arguments' type first as they will be needed for overload resolution.
	shared_ptr<TypePointers> argumentTypes;
	if (isPositionalCall)
		argumentTypes = make_shared<TypePointers>();
	for (ASTPointer<Expression const> const& argument: arguments)
	{
		argument->accept(*this);
		if (!argument->annotation().isPure)
			isPure = false;
		// only store them for positional calls
		if (isPositionalCall)
			argumentTypes->push_back(type(*argument));
	}
	if (isPositionalCall)
		_functionCall.expression().annotation().argumentTypes = move(argumentTypes);

	_functionCall.expression().accept(*this);
	TypePointer expressionType = type(_functionCall.expression());

	if (auto const* typeType = dynamic_cast<TypeType const*>(expressionType.get()))
	{
		if (typeType->actualType()->category() == Type::Category::Struct)
			_functionCall.annotation().kind = FunctionCallKind::StructConstructorCall;
		else
			_functionCall.annotation().kind = FunctionCallKind::TypeConversion;

	}
	else
		_functionCall.annotation().kind = FunctionCallKind::FunctionCall;
	solAssert(_functionCall.annotation().kind != FunctionCallKind::Unset, "");

	if (_functionCall.annotation().kind == FunctionCallKind::TypeConversion)
	{
		TypeType const& t = dynamic_cast<TypeType const&>(*expressionType);
		TypePointer resultType = t.actualType();
		if (arguments.size() != 1)
			typeError(_functionCall.location(), "Exactly one argument expected for explicit type conversion.");
		else if (!isPositionalCall)
			typeError(_functionCall.location(), "Type conversion cannot allow named arguments.");
		else
		{
			TypePointer const& argType = type(*arguments.front());
			if (auto argRefType = dynamic_cast<ReferenceType const*>(argType.get()))
				// do not change the data location when converting
				// (data location cannot yet be specified for type conversions)
				resultType = ReferenceType::copyForLocationIfReference(argRefType->location(), resultType);
			if (!argType->isExplicitlyConvertibleTo(*resultType))
				typeError(_functionCall.location(), "Explicit type conversion not allowed.");
		}
		_functionCall.annotation().type = resultType;
		_functionCall.annotation().isPure = isPure;

		return false;
	}

	// Actual function call or struct constructor call.

	FunctionTypePointer functionType;

	/// For error message: Struct members that were removed during conversion to memory.
	set<string> membersRemovedForStructConstructor;
	if (_functionCall.annotation().kind == FunctionCallKind::StructConstructorCall)
	{
		TypeType const& t = dynamic_cast<TypeType const&>(*expressionType);
		auto const& structType = dynamic_cast<StructType const&>(*t.actualType());
		functionType = structType.constructorType();
		membersRemovedForStructConstructor = structType.membersMissingInMemory();
		_functionCall.annotation().isPure = isPure;
	}
	else
	{
		functionType = dynamic_pointer_cast<FunctionType const>(expressionType);
		_functionCall.annotation().isPure =
			isPure &&
			_functionCall.expression().annotation().isPure &&
			functionType->isPure();
	}

	if (!functionType)
	{
		typeError(_functionCall.location(), "Type is not callable");
		_functionCall.annotation().type = make_shared<TupleType>();
		return false;
	}
	else if (functionType->returnParameterTypes().size() == 1)
		_functionCall.annotation().type = functionType->returnParameterTypes().front();
	else
		_functionCall.annotation().type = make_shared<TupleType>(functionType->returnParameterTypes());

	TypePointers parameterTypes = functionType->parameterTypes();
	if (!functionType->takesArbitraryParameters() && parameterTypes.size() != arguments.size())
	{
		string msg =
			"Wrong argument count for function call: " +
			toString(arguments.size()) +
			" arguments given but expected " +
			toString(parameterTypes.size()) +
			".";
		// Extend error message in case we try to construct a struct with mapping member.
		if (_functionCall.annotation().kind == FunctionCallKind::StructConstructorCall && !membersRemovedForStructConstructor.empty())
		{
			msg += " Members that have to be skipped in memory:";
			for (auto const& member: membersRemovedForStructConstructor)
				msg += " " + member;
		}
		typeError(_functionCall.location(), msg);
	}
	else if (isPositionalCall)
	{
		// call by positional arguments
		for (size_t i = 0; i < arguments.size(); ++i)
		{
			auto const& argType = type(*arguments[i]);
			if (functionType->takesArbitraryParameters())
			{
				if (auto t = dynamic_cast<RationalNumberType const*>(argType.get()))
					if (!t->mobileType())
						typeError(arguments[i]->location(), "Invalid rational number (too large or division by zero).");
			}
			else if (!type(*arguments[i])->isImplicitlyConvertibleTo(*parameterTypes[i]))
				typeError(
					arguments[i]->location(),
					"Invalid type for argument in function call. "
					"Invalid implicit conversion from " +
					type(*arguments[i])->toString() +
					" to " +
					parameterTypes[i]->toString() +
					" requested."
				);
		}
	}
	else
	{
		// call by named arguments
		auto const& parameterNames = functionType->parameterNames();
		if (functionType->takesArbitraryParameters())
			typeError(
				_functionCall.location(),
				"Named arguments cannnot be used for functions that take arbitrary parameters."
			);
		else if (parameterNames.size() > argumentNames.size())
			typeError(_functionCall.location(), "Some argument names are missing.");
		else if (parameterNames.size() < argumentNames.size())
			typeError(_functionCall.location(), "Too many arguments.");
		else
		{
			// check duplicate names
			bool duplication = false;
			for (size_t i = 0; i < argumentNames.size(); i++)
				for (size_t j = i + 1; j < argumentNames.size(); j++)
					if (*argumentNames[i] == *argumentNames[j])
					{
						duplication = true;
						typeError(arguments[i]->location(), "Duplicate named argument.");
					}

			// check actual types
			if (!duplication)
				for (size_t i = 0; i < argumentNames.size(); i++)
				{
					bool found = false;
					for (size_t j = 0; j < parameterNames.size(); j++)
						if (parameterNames[j] == *argumentNames[i])
						{
							found = true;
							// check type convertible
							if (!type(*arguments[i])->isImplicitlyConvertibleTo(*parameterTypes[j]))
								typeError(
									arguments[i]->location(),
									"Invalid type for argument in function call. "
									"Invalid implicit conversion from " +
									type(*arguments[i])->toString() +
									" to " +
									parameterTypes[i]->toString() +
									" requested."
								);
							break;
						}

					if (!found)
						typeError(
							_functionCall.location(),
							"Named argument does not match function declaration."
						);
				}
		}
	}

	return false;
}

void TypeChecker::endVisit(NewExpression const& _newExpression)
{
	TypePointer type = _newExpression.typeName().annotation().type;
	solAssert(!!type, "Type name not resolved.");

	if (auto contractName = dynamic_cast<UserDefinedTypeName const*>(&_newExpression.typeName()))
	{
		auto contract = dynamic_cast<ContractDefinition const*>(&dereference(*contractName));

		if (!contract)
			fatalTypeError(_newExpression.location(), "Identifier is not a contract.");
		if (!contract->annotation().isFullyImplemented)
			typeError(_newExpression.location(), "Trying to create an instance of an abstract contract.");
		if (!contract->constructorIsPublic())
			typeError(_newExpression.location(), "Contract with internal constructor cannot be created directly.");

		solAssert(!!m_scope, "");
		m_scope->annotation().contractDependencies.insert(contract);
		solAssert(
			!contract->annotation().linearizedBaseContracts.empty(),
			"Linearized base contracts not yet available."
		);
		if (contractDependenciesAreCyclic(*m_scope))
			typeError(
				_newExpression.location(),
				"Circular reference for contract creation (cannot create instance of derived or same contract)."
			);

		_newExpression.annotation().type = FunctionType::newExpressionType(*contract);
	}
	else if (type->category() == Type::Category::Array)
	{
		if (!type->canLiveOutsideStorage())
			fatalTypeError(
				_newExpression.typeName().location(),
				"Type cannot live outside storage."
			);
		if (!type->isDynamicallySized())
			typeError(
				_newExpression.typeName().location(),
				"Length has to be placed in parentheses after the array type for new expression."
			);
		type = ReferenceType::copyForLocationIfReference(DataLocation::Memory, type);
		_newExpression.annotation().type = make_shared<FunctionType>(
			TypePointers{make_shared<IntegerType>(256)},
			TypePointers{type},
			strings(),
			strings(),
			FunctionType::Kind::ObjectCreation
		);
		_newExpression.annotation().isPure = true;
	}
	else
		fatalTypeError(_newExpression.location(), "Contract or array type expected.");
}

bool TypeChecker::visit(MemberAccess const& _memberAccess)
{
	_memberAccess.expression().accept(*this);
	TypePointer exprType = type(_memberAccess.expression());
	ASTString const& memberName = _memberAccess.memberName();

	// Retrieve the types of the arguments if this is used to call a function.
	auto const& argumentTypes = _memberAccess.annotation().argumentTypes;
	MemberList::MemberMap possibleMembers = exprType->members(m_scope).membersByName(memberName);
	if (possibleMembers.size() > 1 && argumentTypes)
	{
		// do overload resolution
		for (auto it = possibleMembers.begin(); it != possibleMembers.end();)
			if (
				it->type->category() == Type::Category::Function &&
				!dynamic_cast<FunctionType const&>(*it->type).canTakeArguments(*argumentTypes, exprType)
			)
				it = possibleMembers.erase(it);
			else
				++it;
	}
	if (possibleMembers.size() == 0)
	{
		auto storageType = ReferenceType::copyForLocationIfReference(
			DataLocation::Storage,
			exprType
		);
		if (!storageType->members(m_scope).membersByName(memberName).empty())
			fatalTypeError(
				_memberAccess.location(),
				"Member \"" + memberName + "\" is not available in " +
				exprType->toString() +
				" outside of storage."
			);
		fatalTypeError(
			_memberAccess.location(),
			"Member \"" + memberName + "\" not found or not visible "
			"after argument-dependent lookup in " + exprType->toString() +
			(memberName == "value" ? " - did you forget the \"payable\" modifier?" : "")
		);
	}
	else if (possibleMembers.size() > 1)
		fatalTypeError(
			_memberAccess.location(),
			"Member \"" + memberName + "\" not unique "
			"after argument-dependent lookup in " + exprType->toString() +
			(memberName == "value" ? " - did you forget the \"payable\" modifier?" : "")
		);

	auto& annotation = _memberAccess.annotation();
	annotation.referencedDeclaration = possibleMembers.front().declaration;
	annotation.type = possibleMembers.front().type;

	if (auto funType = dynamic_cast<FunctionType const*>(annotation.type.get()))
		if (funType->bound() && !exprType->isImplicitlyConvertibleTo(*funType->selfType()))
			typeError(
				_memberAccess.location(),
				"Function \"" + memberName + "\" cannot be called on an object of type " +
				exprType->toString() + " (expected " + funType->selfType()->toString() + ")"
			);

	if (exprType->category() == Type::Category::Struct)
		annotation.isLValue = true;
	else if (exprType->category() == Type::Category::Array)
	{
		auto const& arrayType(dynamic_cast<ArrayType const&>(*exprType));
		annotation.isLValue = (
			memberName == "length" &&
			arrayType.location() == DataLocation::Storage &&
			arrayType.isDynamicallySized()
		);
	}
	else if (exprType->category() == Type::Category::FixedBytes)
		annotation.isLValue = false;
	else if (TypeType const* typeType = dynamic_cast<decltype(typeType)>(exprType.get()))
	{
		if (ContractType const* contractType = dynamic_cast<decltype(contractType)>(typeType->actualType().get()))
			annotation.isLValue = annotation.referencedDeclaration->isLValue();
	}

	// TODO some members might be pure, but for example `address(0x123).balance` is not pure
	// although every subexpression is, so leaving this limited for now.
	if (auto tt = dynamic_cast<TypeType const*>(exprType.get()))
		if (tt->actualType()->category() == Type::Category::Enum)
			annotation.isPure = true;

	return false;
}

bool TypeChecker::visit(IndexAccess const& _access)
{
	_access.baseExpression().accept(*this);
	TypePointer baseType = type(_access.baseExpression());
	TypePointer resultType;
	bool isLValue = false;
	bool isPure = _access.baseExpression().annotation().isPure;
	Expression const* index = _access.indexExpression();
	switch (baseType->category())
	{
	case Type::Category::Array:
	{
		ArrayType const& actualType = dynamic_cast<ArrayType const&>(*baseType);
		if (!index)
			typeError(_access.location(), "Index expression cannot be omitted.");
		else if (actualType.isString())
		{
			typeError(_access.location(), "Index access for string is not possible.");
			index->accept(*this);
		}
		else
		{
			expectType(*index, IntegerType(256));
			if (auto numberType = dynamic_cast<RationalNumberType const*>(type(*index).get()))
			{
				if (!numberType->isFractional()) // error is reported above
					if (!actualType.isDynamicallySized() && actualType.length() <= numberType->literalValue(nullptr))
						typeError(_access.location(), "Out of bounds array access.");
			}
		}
		resultType = actualType.baseType();
		isLValue = actualType.location() != DataLocation::CallData;
		break;
	}
	case Type::Category::Mapping:
	{
		MappingType const& actualType = dynamic_cast<MappingType const&>(*baseType);
		if (!index)
			typeError(_access.location(), "Index expression cannot be omitted.");
		else
			expectType(*index, *actualType.keyType());
		resultType = actualType.valueType();
		isLValue = true;
		break;
	}
	case Type::Category::TypeType:
	{
		TypeType const& typeType = dynamic_cast<TypeType const&>(*baseType);
		if (!index)
			resultType = make_shared<TypeType>(make_shared<ArrayType>(DataLocation::Memory, typeType.actualType()));
		else
		{
			expectType(*index, IntegerType(256));
			if (auto length = dynamic_cast<RationalNumberType const*>(type(*index).get()))
				resultType = make_shared<TypeType>(make_shared<ArrayType>(
					DataLocation::Memory,
					typeType.actualType(),
					length->literalValue(nullptr)
				));
			else
				fatalTypeError(index->location(), "Integer constant expected.");
		}
		break;
	}
	case Type::Category::FixedBytes:
	{
		FixedBytesType const& bytesType = dynamic_cast<FixedBytesType const&>(*baseType);
		if (!index)
			typeError(_access.location(), "Index expression cannot be omitted.");
		else
		{
			expectType(*index, IntegerType(256));
			if (auto integerType = dynamic_cast<RationalNumberType const*>(type(*index).get()))
				if (bytesType.numBytes() <= integerType->literalValue(nullptr))
					typeError(_access.location(), "Out of bounds array access.");
		}
		resultType = make_shared<FixedBytesType>(1);
		isLValue = false; // @todo this heavily depends on how it is embedded
		break;
	}
	default:
		fatalTypeError(
			_access.baseExpression().location(),
			"Indexed expression has to be a type, mapping or array (is " + baseType->toString() + ")"
		);
	}
	_access.annotation().type = move(resultType);
	_access.annotation().isLValue = isLValue;
	if (index && !index->annotation().isPure)
		isPure = false;
	_access.annotation().isPure = isPure;

	return false;
}

bool TypeChecker::visit(Identifier const& _identifier)
{
	IdentifierAnnotation& annotation = _identifier.annotation();
	if (!annotation.referencedDeclaration)
	{
		if (!annotation.argumentTypes)
		{
			// The identifier should be a public state variable shadowing other functions
			vector<Declaration const*> candidates;

			for (Declaration const* declaration: annotation.overloadedDeclarations)
			{
				if (VariableDeclaration const* variableDeclaration = dynamic_cast<decltype(variableDeclaration)>(declaration))
					candidates.push_back(declaration);
			}
			if (candidates.empty())
				fatalTypeError(_identifier.location(), "No matching declaration found after variable lookup.");
			else if (candidates.size() == 1)
				annotation.referencedDeclaration = candidates.front();
			else
				fatalTypeError(_identifier.location(), "No unique declaration found after variable lookup.");
		}
		else if (annotation.overloadedDeclarations.empty())
			fatalTypeError(_identifier.location(), "No candidates for overload resolution found.");
		else if (annotation.overloadedDeclarations.size() == 1)
			annotation.referencedDeclaration = *annotation.overloadedDeclarations.begin();
		else
		{
			vector<Declaration const*> candidates;

			for (Declaration const* declaration: annotation.overloadedDeclarations)
			{
				TypePointer function = declaration->type();
				solAssert(!!function, "Requested type not present.");
				auto const* functionType = dynamic_cast<FunctionType const*>(function.get());
				if (functionType && functionType->canTakeArguments(*annotation.argumentTypes))
					candidates.push_back(declaration);
			}
			if (candidates.empty())
				fatalTypeError(_identifier.location(), "No matching declaration found after argument-dependent lookup.");
			else if (candidates.size() == 1)
				annotation.referencedDeclaration = candidates.front();
			else
				fatalTypeError(_identifier.location(), "No unique declaration found after argument-dependent lookup.");
		}
	}
	solAssert(
		!!annotation.referencedDeclaration,
		"Referenced declaration is null after overload resolution."
	);
	annotation.isLValue = annotation.referencedDeclaration->isLValue();
	annotation.type = annotation.referencedDeclaration->type();
	if (!annotation.type)
		fatalTypeError(_identifier.location(), "Declaration referenced before type could be determined.");
	if (auto variableDeclaration = dynamic_cast<VariableDeclaration const*>(annotation.referencedDeclaration))
		annotation.isPure = annotation.isConstant = variableDeclaration->isConstant();
	else if (dynamic_cast<MagicVariableDeclaration const*>(annotation.referencedDeclaration))
		if (dynamic_cast<FunctionType const*>(annotation.type.get()))
			annotation.isPure = true;
	return false;
}

void TypeChecker::endVisit(ElementaryTypeNameExpression const& _expr)
{
	_expr.annotation().type = make_shared<TypeType>(Type::fromElementaryTypeName(_expr.typeName()));
	_expr.annotation().isPure = true;
}

void TypeChecker::endVisit(Literal const& _literal)
{
	if (_literal.looksLikeAddress())
	{
		if (_literal.passesAddressChecksum())
		{
			_literal.annotation().type = make_shared<IntegerType>(0, IntegerType::Modifier::Address);
			return;
		}
		else
			warning(
				_literal.location(),
				"This looks like an address but has an invalid checksum. "
				"If this is not used as an address, please prepend '00'."
			);
	}
	_literal.annotation().type = Type::forLiteral(_literal);
	_literal.annotation().isPure = true;
	if (!_literal.annotation().type)
		fatalTypeError(_literal.location(), "Invalid literal value.");
}

bool TypeChecker::contractDependenciesAreCyclic(
	ContractDefinition const& _contract,
	std::set<ContractDefinition const*> const& _seenContracts
) const
{
	// Naive depth-first search that remembers nodes already seen.
	if (_seenContracts.count(&_contract))
		return true;
	set<ContractDefinition const*> seen(_seenContracts);
	seen.insert(&_contract);
	for (auto const* c: _contract.annotation().contractDependencies)
		if (contractDependenciesAreCyclic(*c, seen))
			return true;
	return false;
}

Declaration const& TypeChecker::dereference(Identifier const& _identifier) const
{
	solAssert(!!_identifier.annotation().referencedDeclaration, "Declaration not stored.");
	return *_identifier.annotation().referencedDeclaration;
}

Declaration const& TypeChecker::dereference(UserDefinedTypeName const& _typeName) const
{
	solAssert(!!_typeName.annotation().referencedDeclaration, "Declaration not stored.");
	return *_typeName.annotation().referencedDeclaration;
}

void TypeChecker::expectType(Expression const& _expression, Type const& _expectedType)
{
	_expression.accept(*this);
	if (!type(_expression)->isImplicitlyConvertibleTo(_expectedType))
	{
		if (
			type(_expression)->category() == Type::Category::RationalNumber &&
			dynamic_pointer_cast<RationalNumberType const>(type(_expression))->isFractional() &&
			type(_expression)->mobileType()
		)
			typeError(
				_expression.location(),
				"Type " +
				type(_expression)->toString() +
				" is not implicitly convertible to expected type " +
				_expectedType.toString() +
				". Try converting to type " +
				type(_expression)->mobileType()->toString() +
				" or use an explicit conversion."
			);
		else
			typeError(
				_expression.location(),
				"Type " +
				type(_expression)->toString() +
				" is not implicitly convertible to expected type " +
				_expectedType.toString() +
				"."
			);
	}		
}

void TypeChecker::requireLValue(Expression const& _expression)
{
	_expression.annotation().lValueRequested = true;
	_expression.accept(*this);

	if (_expression.annotation().isConstant)
		typeError(_expression.location(), "Cannot assign to a constant variable.");
	else if (!_expression.annotation().isLValue)
		typeError(_expression.location(), "Expression has to be an lvalue.");
}

void TypeChecker::typeError(SourceLocation const& _location, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::TypeError);
	*err <<
		errinfo_sourceLocation(_location) <<
		errinfo_comment(_description);

	m_errors.push_back(err);
}

void TypeChecker::warning(SourceLocation const& _location, string const& _description)
{
	auto err = make_shared<Error>(Error::Type::Warning);
	*err <<
		errinfo_sourceLocation(_location) <<
		errinfo_comment(_description);

	m_errors.push_back(err);
}

void TypeChecker::fatalTypeError(SourceLocation const& _location, string const& _description)
{
	typeError(_location, _description);
	BOOST_THROW_EXCEPTION(FatalError());
}
