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
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <libsolidity/ast/AST.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/inlineasm/AsmAnalysisInfo.h>
#include <libsolidity/inlineasm/AsmData.h>
#include <libsolidity/interface/ErrorReporter.h>
#include <libdevcore/Algorithms.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

namespace
{

bool typeSupportedByOldABIEncoder(Type const& _type)
{
	if (_type.dataStoredIn(DataLocation::Storage))
		return true;
	if (_type.category() == Type::Category::Struct)
		return false;
	if (_type.category() == Type::Category::Array)
	{
		auto const& arrayType = dynamic_cast<ArrayType const&>(_type);
		auto base = arrayType.baseType();
		if (!typeSupportedByOldABIEncoder(*base) || (base->category() == Type::Category::Array && base->isDynamicallySized()))
			return false;
	}
	return true;
}

}


bool TypeChecker::checkTypeRequirements(ASTNode const& _contract)
{
	_contract.accept(*this);
	return Error::containsOnlyWarnings(m_errorReporter.errors());
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
	checkContractDuplicateEvents(_contract);
	checkContractIllegalOverrides(_contract);
	checkContractAbstractFunctions(_contract);
	checkContractBaseConstructorArguments(_contract);

	FunctionDefinition const* function = _contract.constructor();
	if (function)
	{
		if (!function->returnParameters().empty())
			m_errorReporter.typeError(function->returnParameterList()->location(), "Non-empty \"returns\" directive for constructor.");
		if (function->stateMutability() != StateMutability::NonPayable && function->stateMutability() != StateMutability::Payable)
			m_errorReporter.typeError(
				function->location(),
				"Constructor must be payable or non-payable, but is \"" +
				stateMutabilityToString(function->stateMutability()) +
				"\"."
			);
		if (function->visibility() != FunctionDefinition::Visibility::Public && function->visibility() != FunctionDefinition::Visibility::Internal)
			m_errorReporter.typeError(function->location(), "Constructor must be public or internal.");
	}

	for (FunctionDefinition const* function: _contract.definedFunctions())
		if (function->isFallback())
		{
			if (_contract.isLibrary())
				m_errorReporter.typeError(function->location(), "Libraries cannot have fallback functions.");
			if (function->stateMutability() != StateMutability::NonPayable && function->stateMutability() != StateMutability::Payable)
				m_errorReporter.typeError(
					function->location(),
					"Fallback function must be payable or non-payable, but is \"" +
					stateMutabilityToString(function->stateMutability()) +
					"\"."
			);
			if (!function->parameters().empty())
				m_errorReporter.typeError(function->parameterList().location(), "Fallback function cannot take parameters.");
			if (!function->returnParameters().empty())
				m_errorReporter.typeError(function->returnParameterList()->location(), "Fallback function cannot return values.");
			if (function->visibility() != FunctionDefinition::Visibility::External)
				m_errorReporter.typeError(function->location(), "Fallback function must be defined as \"external\".");
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
			m_errorReporter.typeError(
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
	FunctionDefinition const* constructor = nullptr;
	FunctionDefinition const* fallback = nullptr;
	for (FunctionDefinition const* function: _contract.definedFunctions())
		if (function->isConstructor())
		{
			if (constructor)
				m_errorReporter.declarationError(
					function->location(),
					SecondarySourceLocation().append("Another declaration is here:", constructor->location()),
					"More than one constructor defined."
				);
			constructor = function;
		}
		else if (function->isFallback())
		{
			if (fallback)
				m_errorReporter.declarationError(
					function->location(),
					SecondarySourceLocation().append("Another declaration is here:", fallback->location()),
					"Only one fallback function is allowed."
				);
			fallback = function;
		}
		else
		{
			solAssert(!function->name().empty(), "");
			functions[function->name()].push_back(function);
		}

	findDuplicateDefinitions(functions, "Function with same name and arguments defined twice.");
}

void TypeChecker::checkContractDuplicateEvents(ContractDefinition const& _contract)
{
	/// Checks that two events with the same name defined in this contract have different
	/// argument types
	map<string, vector<EventDefinition const*>> events;
	for (EventDefinition const* event: _contract.events())
		events[event->name()].push_back(event);

	findDuplicateDefinitions(events, "Event with same name and arguments defined twice.");
}

template <class T>
void TypeChecker::findDuplicateDefinitions(map<string, vector<T>> const& _definitions, string _message)
{
	for (auto const& it: _definitions)
	{
		vector<T> const& overloads = it.second;
		set<size_t> reported;
		for (size_t i = 0; i < overloads.size() && !reported.count(i); ++i)
		{
			SecondarySourceLocation ssl;

			for (size_t j = i + 1; j < overloads.size(); ++j)
				if (FunctionType(*overloads[i]).hasEqualArgumentTypes(FunctionType(*overloads[j])))
				{
					ssl.append("Other declaration is here:", overloads[j]->location());
					reported.insert(j);
				}

			if (ssl.infos.size() > 0)
			{
				ssl.limitSize(_message);

				m_errorReporter.declarationError(
					overloads[i]->location(),
					ssl,
					_message
				);
			}
		}
	}
}

void TypeChecker::checkContractAbstractFunctions(ContractDefinition const& _contract)
{
	// Mapping from name to function definition (exactly one per argument type equality class) and
	// flag to indicate whether it is fully implemented.
	using FunTypeAndFlag = std::pair<FunctionTypePointer, bool>;
	map<string, vector<FunTypeAndFlag>> functions;

	// Search from base to derived
	for (ContractDefinition const* contract: boost::adaptors::reverse(_contract.annotation().linearizedBaseContracts))
		for (FunctionDefinition const* function: contract->definedFunctions())
		{
			// Take constructors out of overload hierarchy
			if (function->isConstructor())
				continue;
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
					m_errorReporter.typeError(function->location(), "Redeclaring an already implemented function as abstract");
			}
			else if (function->isImplemented())
				it->second = true;
		}

	// Set to not fully implemented if at least one flag is false.
	for (auto const& it: functions)
		for (auto const& funAndFlag: it.second)
			if (!funAndFlag.second)
			{
				FunctionDefinition const* function = dynamic_cast<FunctionDefinition const*>(&funAndFlag.first->declaration());
				solAssert(function, "");
				_contract.annotation().unimplementedFunctions.push_back(function);
				break;
			}
}

void TypeChecker::checkContractBaseConstructorArguments(ContractDefinition const& _contract)
{
	bool const v050 = _contract.sourceUnit().annotation().experimentalFeatures.count(ExperimentalFeature::V050);

	vector<ContractDefinition const*> const& bases = _contract.annotation().linearizedBaseContracts;

	// Determine the arguments that are used for the base constructors.
	for (ContractDefinition const* contract: bases)
	{
		if (FunctionDefinition const* constructor = contract->constructor())
			for (auto const& modifier: constructor->modifiers())
			{
				auto baseContract = dynamic_cast<ContractDefinition const*>(&dereference(*modifier->name()));
				if (modifier->arguments())
				{
					if (baseContract && baseContract->constructor())
						annotateBaseConstructorArguments(_contract, baseContract->constructor(), modifier.get());
				}
				else
				{
					if (v050)
						m_errorReporter.declarationError(
							modifier->location(),
							"Modifier-style base constructor call without arguments."
						);
					else
						m_errorReporter.warning(
							modifier->location(),
							"Modifier-style base constructor call without arguments."
						);
				}
			}

		for (ASTPointer<InheritanceSpecifier> const& base: contract->baseContracts())
		{
			auto baseContract = dynamic_cast<ContractDefinition const*>(&dereference(base->name()));
			solAssert(baseContract, "");

			if (baseContract->constructor() && base->arguments() && !base->arguments()->empty())
				annotateBaseConstructorArguments(_contract, baseContract->constructor(), base.get());
		}
	}

	// check that we get arguments for all base constructors that need it.
	// If not mark the contract as abstract (not fully implemented)
	for (ContractDefinition const* contract: bases)
		if (FunctionDefinition const* constructor = contract->constructor())
			if (contract != &_contract && !constructor->parameters().empty())
				if (!_contract.annotation().baseConstructorArguments.count(constructor))
					_contract.annotation().unimplementedFunctions.push_back(constructor);
}

void TypeChecker::annotateBaseConstructorArguments(
	ContractDefinition const& _currentContract,
	FunctionDefinition const* _baseConstructor,
	ASTNode const* _argumentNode
)
{
	solAssert(_baseConstructor, "");
	solAssert(_argumentNode, "");

	auto insertionResult = _currentContract.annotation().baseConstructorArguments.insert(
		std::make_pair(_baseConstructor, _argumentNode)
	);
	if (!insertionResult.second)
	{
		ASTNode const* previousNode = insertionResult.first->second;

		SourceLocation const* mainLocation = nullptr;
		SecondarySourceLocation ssl;

		if (
			_currentContract.location().contains(previousNode->location()) ||
			_currentContract.location().contains(_argumentNode->location())
		)
		{
			mainLocation = &previousNode->location();
			ssl.append("Second constructor call is here:", _argumentNode->location());
		}
		else
		{
			mainLocation = &_currentContract.location();
			ssl.append("First constructor call is here: ", _argumentNode->location());
			ssl.append("Second constructor call is here: ", previousNode->location());
		}

		m_errorReporter.declarationError(
			*mainLocation,
			ssl,
			"Base constructor arguments given twice."
		);
	}

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
				m_errorReporter.typeError(modifiers[name]->location(), "Override changes function to modifier.");

			for (FunctionDefinition const* overriding: functions[name])
				checkFunctionOverride(*overriding, *function);

			functions[name].push_back(function);
		}
		for (ModifierDefinition const* modifier: contract->functionModifiers())
		{
			string const& name = modifier->name();
			ModifierDefinition const*& override = modifiers[name];
			if (!override)
				override = modifier;
			else if (ModifierType(*override) != ModifierType(*modifier))
				m_errorReporter.typeError(override->location(), "Override changes modifier signature.");
			if (!functions[name].empty())
				m_errorReporter.typeError(override->location(), "Override changes modifier to function.");
		}
	}
}

void TypeChecker::checkFunctionOverride(FunctionDefinition const& function, FunctionDefinition const& super)
{
	FunctionType functionType(function);
	FunctionType superType(super);

	if (!functionType.hasEqualArgumentTypes(superType))
		return;

	if (!function.annotation().superFunction)
		function.annotation().superFunction = &super;

	if (function.visibility() != super.visibility())
	{
		// visibility is enforced to be external in interfaces, but a contract can override that with public
		if (
			super.inContractKind() == ContractDefinition::ContractKind::Interface &&
			function.inContractKind() != ContractDefinition::ContractKind::Interface &&
			function.visibility() == FunctionDefinition::Visibility::Public
		)
			return;
		overrideError(function, super, "Overriding function visibility differs.");
	}

	else if (function.stateMutability() != super.stateMutability())
		overrideError(
			function,
			super,
			"Overriding function changes state mutability from \"" +
			stateMutabilityToString(super.stateMutability()) +
			"\" to \"" +
			stateMutabilityToString(function.stateMutability()) +
			"\"."
		);

	else if (functionType != superType)
		overrideError(function, super, "Overriding function return types differ.");
}

void TypeChecker::overrideError(FunctionDefinition const& function, FunctionDefinition const& super, string message)
{
	m_errorReporter.typeError(
		function.location(),
		SecondarySourceLocation().append("Overridden function is here:", super.location()),
		message
	);
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
					m_errorReporter.typeError(
						it.second[j].first->location(),
						"Function overload clash during conversion to external types for arguments."
					);
}

void TypeChecker::checkLibraryRequirements(ContractDefinition const& _contract)
{
	solAssert(_contract.isLibrary(), "");
	if (!_contract.baseContracts().empty())
		m_errorReporter.typeError(_contract.location(), "Library is not allowed to inherit.");

	for (auto const& var: _contract.stateVariables())
		if (!var->isConstant())
			m_errorReporter.typeError(var->location(), "Library cannot have non-constant state variables");
}

void TypeChecker::checkDoubleStorageAssignment(Assignment const& _assignment)
{
	TupleType const& lhs = dynamic_cast<TupleType const&>(*type(_assignment.leftHandSide()));
	TupleType const& rhs = dynamic_cast<TupleType const&>(*type(_assignment.rightHandSide()));

	bool fillRight = !lhs.components().empty() && (!lhs.components().back() || lhs.components().front());
	size_t storageToStorageCopies = 0;
	size_t toStorageCopies = 0;
	for (size_t i = 0; i < lhs.components().size(); ++i)
	{
		ReferenceType const* ref = dynamic_cast<ReferenceType const*>(lhs.components()[i].get());
		if (!ref || !ref->dataStoredIn(DataLocation::Storage) || ref->isPointer())
			continue;
		size_t rhsPos = fillRight ? i : rhs.components().size() - (lhs.components().size() - i);
		solAssert(rhsPos < rhs.components().size(), "");
		toStorageCopies++;
		if (rhs.components()[rhsPos]->dataStoredIn(DataLocation::Storage))
			storageToStorageCopies++;
	}
	if (storageToStorageCopies >= 1 && toStorageCopies >= 2)
		m_errorReporter.warning(
			_assignment.location(),
			"This assignment performs two copies to storage. Since storage copies do not first "
			"copy to a temporary location, one of them might be overwritten before the second "
			"is executed and thus may have unexpected effects. It is safer to perform the copies "
			"separately or assign to storage pointers first."
		);
}

void TypeChecker::endVisit(InheritanceSpecifier const& _inheritance)
{
	auto base = dynamic_cast<ContractDefinition const*>(&dereference(_inheritance.name()));
	solAssert(base, "Base contract not available.");

	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
		m_errorReporter.typeError(_inheritance.location(), "Interfaces cannot inherit.");

	if (base->isLibrary())
		m_errorReporter.typeError(_inheritance.location(), "Libraries cannot be inherited from.");

	auto const& arguments = _inheritance.arguments();
	TypePointers parameterTypes;
	if (base->contractKind() != ContractDefinition::ContractKind::Interface)
		// Interfaces do not have constructors, so there are zero parameters.
		parameterTypes = ContractType(*base).newExpressionType()->parameterTypes();

	if (arguments)
	{
		if (parameterTypes.size() != arguments->size())
		{
			m_errorReporter.typeError(
				_inheritance.location(),
				"Wrong argument count for constructor call: " +
				toString(arguments->size()) +
				" arguments given but expected " +
				toString(parameterTypes.size()) +
				". Remove parentheses if you do not want to provide arguments here."
			);
		}
		for (size_t i = 0; i < std::min(arguments->size(), parameterTypes.size()); ++i)
			if (!type(*(*arguments)[i])->isImplicitlyConvertibleTo(*parameterTypes[i]))
				m_errorReporter.typeError(
					(*arguments)[i]->location(),
					"Invalid type for argument in constructor call. "
					"Invalid implicit conversion from " +
					type(*(*arguments)[i])->toString() +
					" to " +
					parameterTypes[i]->toString() +
					" requested."
				);
	}
}

void TypeChecker::endVisit(UsingForDirective const& _usingFor)
{
	ContractDefinition const* library = dynamic_cast<ContractDefinition const*>(
		_usingFor.libraryName().annotation().referencedDeclaration
	);
	if (!library || !library->isLibrary())
		m_errorReporter.fatalTypeError(_usingFor.libraryName().location(), "Library name expected.");
}

bool TypeChecker::visit(StructDefinition const& _struct)
{
	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
		m_errorReporter.typeError(_struct.location(), "Structs cannot be defined in interfaces.");

	for (ASTPointer<VariableDeclaration> const& member: _struct.members())
		if (!type(*member)->canBeStored())
			m_errorReporter.typeError(member->location(), "Type cannot be used in struct.");

	// Check recursion, fatal error if detected.
	auto visitor = [&](StructDefinition const& _struct, CycleDetector<StructDefinition>& _cycleDetector)
	{
		for (ASTPointer<VariableDeclaration> const& member: _struct.members())
		{
			Type const* memberType = type(*member).get();
			while (auto arrayType = dynamic_cast<ArrayType const*>(memberType))
			{
				if (arrayType->isDynamicallySized())
					break;
				memberType = arrayType->baseType().get();
			}
			if (auto structType = dynamic_cast<StructType const*>(memberType))
				if (_cycleDetector.run(structType->structDefinition()))
					return;
		}
	};
	if (CycleDetector<StructDefinition>(visitor).run(_struct) != nullptr)
		m_errorReporter.fatalTypeError(_struct.location(), "Recursive struct definition.");

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
			m_errorReporter.typeError(_function.location(), "Library functions cannot be payable.");
		if (!_function.isConstructor() && !_function.isFallback() && !_function.isPartOfExternalInterface())
			m_errorReporter.typeError(_function.location(), "Internal functions cannot be payable.");
	}
	for (ASTPointer<VariableDeclaration> const& var: _function.parameters() + _function.returnParameters())
	{
		if (!type(*var)->canLiveOutsideStorage())
			m_errorReporter.typeError(var->location(), "Type is required to live outside storage.");
		if (_function.visibility() >= FunctionDefinition::Visibility::Public && !(type(*var)->interfaceType(isLibraryFunction)))
			m_errorReporter.fatalTypeError(var->location(), "Internal or recursive type is not allowed for public or external functions.");
		if (
			_function.visibility() > FunctionDefinition::Visibility::Internal &&
			!_function.sourceUnit().annotation().experimentalFeatures.count(ExperimentalFeature::ABIEncoderV2) &&
			!typeSupportedByOldABIEncoder(*type(*var))
		)
			m_errorReporter.typeError(
				var->location(),
				"This type is only supported in the new experimental ABI encoder. "
				"Use \"pragma experimental ABIEncoderV2;\" to enable the feature."
			);

		var->accept(*this);
	}
	set<Declaration const*> modifiers;
	for (ASTPointer<ModifierInvocation> const& modifier: _function.modifiers())
	{
		visitManually(
			*modifier,
			_function.isConstructor() ?
			dynamic_cast<ContractDefinition const&>(*_function.scope()).annotation().linearizedBaseContracts :
			vector<ContractDefinition const*>()
		);
		Declaration const* decl = &dereference(*modifier->name());
		if (modifiers.count(decl))
		{
			if (dynamic_cast<ContractDefinition const*>(decl))
				m_errorReporter.declarationError(modifier->location(), "Base constructor already provided.");
		}
		else
			modifiers.insert(decl);
	}
	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
	{
		if (_function.isImplemented())
			m_errorReporter.typeError(_function.location(), "Functions in interfaces cannot have an implementation.");

		if (_function.visibility() != FunctionDefinition::Visibility::External)
			m_errorReporter.typeError(_function.location(), "Functions in interfaces must be declared external.");

		if (_function.isConstructor())
			m_errorReporter.typeError(_function.location(), "Constructor cannot be defined in interfaces.");
	}
	else if (m_scope->contractKind() == ContractDefinition::ContractKind::Library)
		if (_function.isConstructor())
			m_errorReporter.typeError(_function.location(), "Constructor cannot be defined in libraries.");
	if (_function.isImplemented())
		_function.body().accept(*this);
	else if (_function.isConstructor())
		m_errorReporter.typeError(_function.location(), "Constructor must be implemented if declared.");
	else if (isLibraryFunction && _function.visibility() <= FunctionDefinition::Visibility::Internal)
		m_errorReporter.typeError(_function.location(), "Internal library function must be implemented if declared.");
	return false;
}

bool TypeChecker::visit(VariableDeclaration const& _variable)
{
	// Forbid any variable declarations inside interfaces unless they are part of
	// a function's input/output parameters.
	if (
		m_scope->contractKind() == ContractDefinition::ContractKind::Interface
		&& !_variable.isCallableParameter()
	)
		m_errorReporter.typeError(_variable.location(), "Variables cannot be declared in interfaces.");

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
			m_errorReporter.typeError(_variable.location(), "Illegal use of \"constant\" specifier.");
		if (!_variable.type()->isValueType())
		{
			bool allowed = false;
			if (auto arrayType = dynamic_cast<ArrayType const*>(_variable.type().get()))
				allowed = arrayType->isByteArray();
			if (!allowed)
				m_errorReporter.typeError(_variable.location(), "Constants of non-value type not yet implemented.");
		}

		if (!_variable.value())
			m_errorReporter.typeError(_variable.location(), "Uninitialized \"constant\" variable.");
		else if (!_variable.value()->annotation().isPure)
			m_errorReporter.typeError(
				_variable.value()->location(),
				"Initial value for constant variable has to be compile-time constant."
			);
	}
	if (!_variable.isStateVariable())
	{
		if (varType->dataStoredIn(DataLocation::Memory) || varType->dataStoredIn(DataLocation::CallData))
			if (!varType->canLiveOutsideStorage())
				m_errorReporter.typeError(_variable.location(), "Type " + varType->toString() + " is only valid in storage.");
	}
	else if (
		_variable.visibility() >= VariableDeclaration::Visibility::Public &&
		!FunctionType(_variable).interfaceFunctionType()
	)
		m_errorReporter.typeError(_variable.location(), "Internal or recursive type is not allowed for public state variables.");

	if (varType->category() == Type::Category::Array)
		if (auto arrayType = dynamic_cast<ArrayType const*>(varType.get()))
			if (
				((arrayType->location() == DataLocation::Memory) ||
				(arrayType->location() == DataLocation::CallData)) &&
				!arrayType->validForCalldata()
			)
				m_errorReporter.typeError(_variable.location(), "Array is too large to be encoded.");

	return false;
}

bool TypeChecker::visit(EnumDefinition const& _enum)
{
	if (m_scope->contractKind() == ContractDefinition::ContractKind::Interface)
		m_errorReporter.typeError(_enum.location(), "Enumerable cannot be declared in interfaces.");
	return false;
}

void TypeChecker::visitManually(
	ModifierInvocation const& _modifier,
	vector<ContractDefinition const*> const& _bases
)
{
	std::vector<ASTPointer<Expression>> const& arguments =
		_modifier.arguments() ? *_modifier.arguments() : std::vector<ASTPointer<Expression>>();
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
		m_errorReporter.typeError(_modifier.location(), "Referenced declaration is neither modifier nor base class.");
		return;
	}
	if (parameters->size() != arguments.size())
	{
		m_errorReporter.typeError(
			_modifier.location(),
			"Wrong argument count for modifier invocation: " +
			toString(arguments.size()) +
			" arguments given but expected " +
			toString(parameters->size()) +
			"."
		);
		return;
	}
	for (size_t i = 0; i < arguments.size(); ++i)
		if (!type(*arguments[i])->isImplicitlyConvertibleTo(*type(*(*parameters)[i])))
			m_errorReporter.typeError(
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
		if (!type(*var)->canLiveOutsideStorage())
			m_errorReporter.typeError(var->location(), "Type is required to live outside storage.");
		if (!type(*var)->interfaceType(false))
			m_errorReporter.typeError(var->location(), "Internal or recursive type is not allowed as event parameter type.");
	}
	if (_eventDef.isAnonymous() && numIndexed > 4)
		m_errorReporter.typeError(_eventDef.location(), "More than 4 indexed arguments for anonymous event.");
	else if (!_eventDef.isAnonymous() && numIndexed > 3)
		m_errorReporter.typeError(_eventDef.location(), "More than 3 indexed arguments for event.");
	return false;
}

void TypeChecker::endVisit(FunctionTypeName const& _funType)
{
	FunctionType const& fun = dynamic_cast<FunctionType const&>(*_funType.annotation().type);
	if (fun.kind() == FunctionType::Kind::External)
		if (!fun.canBeUsedExternally(false))
			m_errorReporter.typeError(_funType.location(), "External function type uses internal types.");
}

bool TypeChecker::visit(InlineAssembly const& _inlineAssembly)
{
	// External references have already been resolved in a prior stage and stored in the annotation.
	// We run the resolve step again regardless.
	julia::ExternalIdentifierAccess::Resolver identifierAccess = [&](
		assembly::Identifier const& _identifier,
		julia::IdentifierContext _context,
		bool
	)
	{
		auto ref = _inlineAssembly.annotation().externalReferences.find(&_identifier);
		if (ref == _inlineAssembly.annotation().externalReferences.end())
			return size_t(-1);
		Declaration const* declaration = ref->second.declaration;
		solAssert(!!declaration, "");
		if (auto var = dynamic_cast<VariableDeclaration const*>(declaration))
		{
			if (var->isConstant())
			{
				m_errorReporter.typeError(_identifier.location, "Constant variables not supported by inline assembly.");
				return size_t(-1);
			}
			else if (ref->second.isSlot || ref->second.isOffset)
			{
				if (!var->isStateVariable() && !var->type()->dataStoredIn(DataLocation::Storage))
				{
					m_errorReporter.typeError(_identifier.location, "The suffixes _offset and _slot can only be used on storage variables.");
					return size_t(-1);
				}
				else if (_context != julia::IdentifierContext::RValue)
				{
					m_errorReporter.typeError(_identifier.location, "Storage variables cannot be assigned to.");
					return size_t(-1);
				}
			}
			else if (!var->isLocalVariable())
			{
				m_errorReporter.typeError(_identifier.location, "Only local variables are supported. To access storage variables, use the _slot and _offset suffixes.");
				return size_t(-1);
			}
			else if (var->type()->dataStoredIn(DataLocation::Storage))
			{
				m_errorReporter.typeError(_identifier.location, "You have to use the _slot or _offset suffix to access storage reference variables.");
				return size_t(-1);
			}
			else if (var->type()->sizeOnStack() != 1)
			{
				if (var->type()->dataStoredIn(DataLocation::CallData))
					m_errorReporter.typeError(_identifier.location, "Call data elements cannot be accessed directly. Copy to a local variable first or use \"calldataload\" or \"calldatacopy\" with manually determined offsets and sizes.");
				else
					m_errorReporter.typeError(_identifier.location, "Only types that use one stack slot are supported.");
				return size_t(-1);
			}
		}
		else if (_context == julia::IdentifierContext::LValue)
		{
			m_errorReporter.typeError(_identifier.location, "Only local variables can be assigned to in inline assembly.");
			return size_t(-1);
		}

		if (_context == julia::IdentifierContext::RValue)
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
					m_errorReporter.typeError(_identifier.location, "Expected a library.");
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
	boost::optional<Error::Type> errorTypeForLoose =
		m_scope->sourceUnit().annotation().experimentalFeatures.count(ExperimentalFeature::V050) ?
		Error::Type::SyntaxError :
		Error::Type::Warning;
	assembly::AsmAnalyzer analyzer(
		*_inlineAssembly.annotation().analysisInfo,
		m_errorReporter,
		m_evmVersion,
		errorTypeForLoose,
		assembly::AsmFlavour::Loose,
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
		m_errorReporter.typeError(_return.location(), "Return arguments not allowed.");
		return;
	}
	TypePointers returnTypes;
	for (auto const& var: params->parameters())
		returnTypes.push_back(type(*var));
	if (auto tupleType = dynamic_cast<TupleType const*>(type(*_return.expression()).get()))
	{
		if (tupleType->components().size() != params->parameters().size())
			m_errorReporter.typeError(_return.location(), "Different number of arguments in return statement than in returns declaration.");
		else if (!tupleType->isImplicitlyConvertibleTo(TupleType(returnTypes)))
			m_errorReporter.typeError(
				_return.expression()->location(),
				"Return argument type " +
				type(*_return.expression())->toString() +
				" is not implicitly convertible to expected type " +
				TupleType(returnTypes).toString(false) +
				"."
			);
	}
	else if (params->parameters().size() != 1)
		m_errorReporter.typeError(_return.location(), "Different number of arguments in return statement than in returns declaration.");
	else
	{
		TypePointer const& expected = type(*params->parameters().front());
		if (!type(*_return.expression())->isImplicitlyConvertibleTo(*expected))
			m_errorReporter.typeError(
				_return.expression()->location(),
				"Return argument type " +
				type(*_return.expression())->toString() +
				" is not implicitly convertible to expected type (type of first return variable) " +
				expected->toString() +
				"."
			);
	}
}

void TypeChecker::endVisit(EmitStatement const& _emit)
{
	if (
		_emit.eventCall().annotation().kind != FunctionCallKind::FunctionCall ||
		type(_emit.eventCall().expression())->category() != Type::Category::Function ||
		dynamic_cast<FunctionType const&>(*type(_emit.eventCall().expression())).kind() != FunctionType::Kind::Event
	)
		m_errorReporter.typeError(_emit.eventCall().expression().location(), "Expression has to be an event invocation.");
	m_insideEmitStatement = false;
}

namespace
{
/**
 * @returns a suggested left-hand-side of a multi-variable declaration contairing
 * the variable declarations given in @a _decls.
 */
string createTupleDecl(vector<ASTPointer<VariableDeclaration>> const& _decls)
{
	vector<string> components;
	for (ASTPointer<VariableDeclaration> const& decl: _decls)
		if (decl)
			components.emplace_back(decl->annotation().type->toString(false) + " " + decl->name());
		else
			components.emplace_back();

	if (_decls.size() == 1)
		return components.front();
	else
		return "(" + boost::algorithm::join(components, ", ") + ")";
}

bool typeCanBeExpressed(vector<ASTPointer<VariableDeclaration>> const& decls)
{
	for (ASTPointer<VariableDeclaration> const& decl: decls)
	{
		// skip empty tuples (they can be expressed of course)
		if (!decl)
			continue;

		if (auto functionType = dynamic_cast<FunctionType const*>(decl->annotation().type.get()))
			if (
				functionType->kind() != FunctionType::Kind::Internal &&
				functionType->kind() != FunctionType::Kind::External
			)
				return false;
	}

	return true;
}
}

bool TypeChecker::visit(VariableDeclarationStatement const& _statement)
{
	if (!_statement.initialValue())
	{
		// No initial value is only permitted for single variables with specified type.
		if (_statement.declarations().size() != 1 || !_statement.declarations().front())
		{
			if (boost::algorithm::all_of_equal(_statement.declarations(), nullptr))
			{
				// The syntax checker has already generated an error for this case (empty LHS tuple).
				solAssert(m_errorReporter.hasErrors(), "");

				// It is okay to return here, as there are no named components on the
				// left-hand-side that could cause any damage later.
				return false;
			}
			else
				// Bailing out *fatal* here, as those (untyped) vars may be used later, and diagnostics wouldn't be helpful then.
				m_errorReporter.fatalTypeError(_statement.location(), "Use of the \"var\" keyword is disallowed.");
		}

		VariableDeclaration const& varDecl = *_statement.declarations().front();
		if (!varDecl.annotation().type)
			m_errorReporter.fatalTypeError(_statement.location(), "Use of the \"var\" keyword is disallowed.");

		if (auto ref = dynamic_cast<ReferenceType const*>(type(varDecl).get()))
		{
			if (ref->dataStoredIn(DataLocation::Storage))
			{
				string errorText{"Uninitialized storage pointer."};
				if (varDecl.referenceLocation() == VariableDeclaration::Location::Default)
					errorText += " Did you mean '<type> memory " + varDecl.name() + "'?";
				solAssert(m_scope, "");
				m_errorReporter.declarationError(varDecl.location(), errorText);
			}
		}
		else if (dynamic_cast<MappingType const*>(type(varDecl).get()))
			m_errorReporter.typeError(
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

	vector<ASTPointer<VariableDeclaration>> const& variables = _statement.declarations();
	if (variables.empty())
		// We already have an error for this in the SyntaxChecker.
		solAssert(m_errorReporter.hasErrors(), "");
	else if (valueTypes.size() != variables.size())
		m_errorReporter.typeError(
			_statement.location(),
			"Different number of components on the left hand side (" +
			toString(variables.size()) +
			") than on the right hand side (" +
			toString(valueTypes.size()) +
			")."
		);

	bool autoTypeDeductionNeeded = false;

	for (size_t i = 0; i < min(variables.size(), valueTypes.size()); ++i)
	{
		if (!variables[i])
			continue;
		VariableDeclaration const& var = *variables[i];
		solAssert(!var.value(), "Value has to be tied to statement.");
		TypePointer const& valueComponentType = valueTypes[i];
		solAssert(!!valueComponentType, "");
		if (!var.annotation().type)
		{
			autoTypeDeductionNeeded = true;

			// Infer type from value.
			solAssert(!var.typeName(), "");
			var.annotation().type = valueComponentType->mobileType();
			if (!var.annotation().type)
			{
				if (valueComponentType->category() == Type::Category::RationalNumber)
					m_errorReporter.fatalTypeError(
						_statement.initialValue()->location(),
						"Invalid rational " +
						valueComponentType->toString() +
						" (absolute value too large or division by zero)."
					);
				else
					solAssert(false, "");
			}
			else if (*var.annotation().type == TupleType())
				m_errorReporter.typeError(
					var.location(),
					"Cannot declare variable with void (empty tuple) type."
				);
			else if (valueComponentType->category() == Type::Category::RationalNumber)
			{
				string typeName = var.annotation().type->toString(true);
				string extension;
				if (auto type = dynamic_cast<IntegerType const*>(var.annotation().type.get()))
				{
					unsigned numBits = type->numBits();
					bool isSigned = type->isSigned();
					solAssert(numBits > 0, "");
					string minValue;
					string maxValue;
					if (isSigned)
					{
						numBits--;
						minValue = "-" + bigint(bigint(1) << numBits).str();
					}
					else
						minValue = "0";
					maxValue = bigint((bigint(1) << numBits) - 1).str();
					extension = ", which can hold values between " + minValue + " and " + maxValue;
				}
				else
					solAssert(dynamic_cast<FixedPointType const*>(var.annotation().type.get()), "Unknown type.");
			}

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
					m_errorReporter.typeError(
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
					m_errorReporter.typeError(
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

	if (autoTypeDeductionNeeded)
	{
		if (!typeCanBeExpressed(variables))
			m_errorReporter.syntaxError(
				_statement.location(),
				"Use of the \"var\" keyword is disallowed. "
				"Type cannot be expressed in syntax."
			);
		else
			m_errorReporter.syntaxError(
				_statement.location(),
				"Use of the \"var\" keyword is disallowed. "
				"Use explicit declaration `" + createTupleDecl(variables) + " = ...´ instead."
			);
	}

	return false;
}

void TypeChecker::endVisit(ExpressionStatement const& _statement)
{
	if (type(_statement.expression())->category() == Type::Category::RationalNumber)
		if (!dynamic_cast<RationalNumberType const&>(*type(_statement.expression())).mobileType())
			m_errorReporter.typeError(_statement.expression().location(), "Invalid rational number.");

	if (auto call = dynamic_cast<FunctionCall const*>(&_statement.expression()))
	{
		if (auto callType = dynamic_cast<FunctionType const*>(type(call->expression()).get()))
		{
			auto kind = callType->kind();
			if (
				kind == FunctionType::Kind::BareCall ||
				kind == FunctionType::Kind::BareCallCode ||
				kind == FunctionType::Kind::BareDelegateCall
			)
				m_errorReporter.warning(_statement.location(), "Return value of low-level calls not used.");
			else if (kind == FunctionType::Kind::Send)
				m_errorReporter.warning(_statement.location(), "Failure condition of 'send' ignored. Consider using 'transfer' instead.");
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
		m_errorReporter.fatalTypeError(_conditional.trueExpression().location(), "Invalid mobile type.");
	if (!falseType)
		m_errorReporter.fatalTypeError(_conditional.falseExpression().location(), "Invalid mobile type.");

	TypePointer commonType = Type::commonType(trueType, falseType);
	if (!commonType)
	{
		m_errorReporter.typeError(
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
		m_errorReporter.typeError(
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
			m_errorReporter.typeError(
				_assignment.location(),
				"Compound assignment is not allowed for tuple types."
			);
		// Sequenced assignments of tuples is not valid, make the result a "void" type.
		_assignment.annotation().type = make_shared<TupleType>();

		expectType(_assignment.rightHandSide(), *tupleType);

		// expectType does not cause fatal errors, so we have to check again here.
		if (dynamic_cast<TupleType const*>(type(_assignment.rightHandSide()).get()))
			checkDoubleStorageAssignment(_assignment);
	}
	else if (t->category() == Type::Category::Mapping)
	{
		m_errorReporter.typeError(_assignment.location(), "Mappings cannot be assigned to.");
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
			m_errorReporter.typeError(
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
			m_errorReporter.fatalTypeError(_tuple.location(), "Inline array type cannot be declared as LValue.");
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
			if (!components[i])
				m_errorReporter.fatalTypeError(_tuple.location(), "Tuple component cannot be empty.");
			else if (components[i])
			{
				components[i]->accept(*this);
				types.push_back(type(*components[i]));

				if (types[i]->category() == Type::Category::Tuple)
					if (dynamic_cast<TupleType const&>(*types[i]).components().empty())
					{
						if (_tuple.isInlineArray())
							m_errorReporter.fatalTypeError(components[i]->location(), "Array component cannot be empty.");
						m_errorReporter.typeError(components[i]->location(), "Tuple component cannot be empty.");
					}

				// Note: code generation will visit each of the expression even if they are not assigned from.
				if (types[i]->category() == Type::Category::RationalNumber && components.size() > 1)
					if (!dynamic_cast<RationalNumberType const&>(*types[i]).mobileType())
						m_errorReporter.fatalTypeError(components[i]->location(), "Invalid rational number.");

				if (_tuple.isInlineArray())
					solAssert(!!types[i], "Inline array cannot have empty components");
				if (_tuple.isInlineArray())
				{
					if ((i == 0 || inlineArrayType) && !types[i]->mobileType())
						m_errorReporter.fatalTypeError(components[i]->location(), "Invalid mobile type.");

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
				m_errorReporter.fatalTypeError(_tuple.location(), "Unable to deduce common type for array elements.");
			_tuple.annotation().type = make_shared<ArrayType>(DataLocation::Memory, inlineArrayType, types.size());
		}
		else
		{
			if (components.size() == 1)
				_tuple.annotation().type = type(*components[0]);
			else
				_tuple.annotation().type = make_shared<TupleType>(types);
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
		m_errorReporter.typeError(
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
		m_errorReporter.typeError(
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

	if (_operation.getOperator() == Token::Exp || _operation.getOperator() == Token::SHL)
	{
		string operation = _operation.getOperator() == Token::Exp ? "exponentiation" : "shift";
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
				m_errorReporter.warning(
					_operation.location(),
					"Result of " + operation + " has type " + commonType->toString() + " and thus "
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
			m_errorReporter.typeError(_functionCall.location(), "Exactly one argument expected for explicit type conversion.");
		else if (!isPositionalCall)
			m_errorReporter.typeError(_functionCall.location(), "Type conversion cannot allow named arguments.");
		else
		{
			TypePointer const& argType = type(*arguments.front());
			if (auto argRefType = dynamic_cast<ReferenceType const*>(argType.get()))
				// do not change the data location when converting
				// (data location cannot yet be specified for type conversions)
				resultType = ReferenceType::copyForLocationIfReference(argRefType->location(), resultType);
			if (!argType->isExplicitlyConvertibleTo(*resultType))
				m_errorReporter.typeError(
					_functionCall.location(),
					"Explicit type conversion not allowed from \"" +
					argType->toString() +
					"\" to \"" +
					resultType->toString() +
					"\"."
				);
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
	else if ((functionType = dynamic_pointer_cast<FunctionType const>(expressionType)))
		_functionCall.annotation().isPure =
			isPure &&
			_functionCall.expression().annotation().isPure &&
			functionType->isPure();

	bool allowDynamicTypes = m_evmVersion.supportsReturndata();
	if (!functionType)
	{
		m_errorReporter.typeError(_functionCall.location(), "Type is not callable");
		_functionCall.annotation().type = make_shared<TupleType>();
		return false;
	}

	auto returnTypes =
		allowDynamicTypes ?
		functionType->returnParameterTypes() :
		functionType->returnParameterTypesWithoutDynamicTypes();
	if (returnTypes.size() == 1)
		_functionCall.annotation().type = returnTypes.front();
	else
		_functionCall.annotation().type = make_shared<TupleType>(returnTypes);

	if (auto functionName = dynamic_cast<Identifier const*>(&_functionCall.expression()))
	{
		if (functionName->name() == "sha3" && functionType->kind() == FunctionType::Kind::SHA3)
			m_errorReporter.typeError(_functionCall.location(), "\"sha3\" has been deprecated in favour of \"keccak256\"");
		else if (functionName->name() == "suicide" && functionType->kind() == FunctionType::Kind::Selfdestruct)
			m_errorReporter.typeError(_functionCall.location(), "\"suicide\" has been deprecated in favour of \"selfdestruct\"");
	}
	if (!m_insideEmitStatement && functionType->kind() == FunctionType::Kind::Event)
		m_errorReporter.typeError(_functionCall.location(), "Event invocations have to be prefixed by \"emit\".");

	TypePointers parameterTypes = functionType->parameterTypes();

	if (!functionType->padArguments())
	{
		for (size_t i = 0; i < arguments.size(); ++i)
		{
			auto const& argType = type(*arguments[i]);
			if (auto literal = dynamic_cast<RationalNumberType const*>(argType.get()))
			{
				if (literal->mobileType())
					m_errorReporter.typeError(
						arguments[i]->location(),
						"Cannot perform packed encoding for a literal. Please convert it to an explicit type first."
					);
				else
				{
					/* If no mobile type is available an error will be raised elsewhere. */
					solAssert(m_errorReporter.hasErrors(), "");
				}
			}
		}
	}

	if (functionType->takesArbitraryParameters() && arguments.size() < parameterTypes.size())
	{
		solAssert(_functionCall.annotation().kind == FunctionCallKind::FunctionCall, "");
		m_errorReporter.typeError(
			_functionCall.location(),
			"Need at least " +
			toString(parameterTypes.size()) +
			" arguments for function call, but provided only " +
			toString(arguments.size()) +
			"."
		);
	}
	else if (!functionType->takesArbitraryParameters() && parameterTypes.size() != arguments.size())
	{
		bool isStructConstructorCall = _functionCall.annotation().kind == FunctionCallKind::StructConstructorCall;

		string msg =
			"Wrong argument count for " +
			string(isStructConstructorCall ? "struct constructor" : "function call") +
			": " +
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
		else if (
			functionType->kind() == FunctionType::Kind::BareCall ||
			functionType->kind() == FunctionType::Kind::BareCallCode ||
			functionType->kind() == FunctionType::Kind::BareDelegateCall
		)
		{
			if (arguments.empty())
				msg += " This function requires a single bytes argument. Use \"\" as argument to provide empty calldata.";
			else
				msg += " This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.";
		}
		else if (
			functionType->kind() == FunctionType::Kind::SHA3 ||
			functionType->kind() == FunctionType::Kind::SHA256 ||
			functionType->kind() == FunctionType::Kind::RIPEMD160
		)
			msg +=
				" This function requires a single bytes argument."
				" Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour"
				" or abi.encode(...) to use ABI encoding.";
		m_errorReporter.typeError(_functionCall.location(), msg);
	}
	else if (isPositionalCall)
	{
		bool const abiEncodeV2 = m_scope->sourceUnit().annotation().experimentalFeatures.count(ExperimentalFeature::ABIEncoderV2);

		for (size_t i = 0; i < arguments.size(); ++i)
		{
			auto const& argType = type(*arguments[i]);
			if (functionType->takesArbitraryParameters() && i >= parameterTypes.size())
			{
				bool errored = false;
				if (auto t = dynamic_cast<RationalNumberType const*>(argType.get()))
					if (!t->mobileType())
					{
						m_errorReporter.typeError(arguments[i]->location(), "Invalid rational number (too large or division by zero).");
						errored = true;
					}
				if (!errored)
				{
					TypePointer encodingType;
					if (
						argType->mobileType() &&
						argType->mobileType()->interfaceType(false) &&
						argType->mobileType()->interfaceType(false)->encodingType()
					)
						encodingType = argType->mobileType()->interfaceType(false)->encodingType();
					// Structs are fine as long as ABIV2 is activated and we do not do packed encoding.
					if (!encodingType || (
						dynamic_cast<StructType const*>(encodingType.get()) &&
						!(abiEncodeV2 && functionType->padArguments())
					))
						m_errorReporter.typeError(arguments[i]->location(), "This type cannot be encoded.");
				}
			}
			else if (!type(*arguments[i])->isImplicitlyConvertibleTo(*parameterTypes[i]))
			{
				string msg =
					"Invalid type for argument in function call. "
					"Invalid implicit conversion from " +
					type(*arguments[i])->toString() +
					" to " +
					parameterTypes[i]->toString() +
					" requested.";
				if (
					functionType->kind() == FunctionType::Kind::BareCall ||
					functionType->kind() == FunctionType::Kind::BareCallCode ||
					functionType->kind() == FunctionType::Kind::BareDelegateCall
				)
					msg += " This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.";
				else if (
					functionType->kind() == FunctionType::Kind::SHA3 ||
					functionType->kind() == FunctionType::Kind::SHA256 ||
					functionType->kind() == FunctionType::Kind::RIPEMD160
				)
					msg +=
						" This function requires a single bytes argument."
						" Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour"
						" or abi.encode(...) to use ABI encoding.";
				m_errorReporter.typeError(arguments[i]->location(), msg);
			}
		}
	}
	else
	{
		// call by named arguments
		auto const& parameterNames = functionType->parameterNames();
		if (functionType->takesArbitraryParameters())
			m_errorReporter.typeError(
				_functionCall.location(),
				"Named arguments cannot be used for functions that take arbitrary parameters."
			);
		else if (parameterNames.size() > argumentNames.size())
			m_errorReporter.typeError(_functionCall.location(), "Some argument names are missing.");
		else if (parameterNames.size() < argumentNames.size())
			m_errorReporter.typeError(_functionCall.location(), "Too many arguments.");
		else
		{
			// check duplicate names
			bool duplication = false;
			for (size_t i = 0; i < argumentNames.size(); i++)
				for (size_t j = i + 1; j < argumentNames.size(); j++)
					if (*argumentNames[i] == *argumentNames[j])
					{
						duplication = true;
						m_errorReporter.typeError(arguments[i]->location(), "Duplicate named argument.");
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
								m_errorReporter.typeError(
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
						m_errorReporter.typeError(
							_functionCall.location(),
							"Named argument \"" + *argumentNames[i] +  "\" does not match function declaration."
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
			m_errorReporter.fatalTypeError(_newExpression.location(), "Identifier is not a contract.");
		if (contract->contractKind() == ContractDefinition::ContractKind::Interface)
				m_errorReporter.fatalTypeError(_newExpression.location(), "Cannot instantiate an interface.");
		if (!contract->annotation().unimplementedFunctions.empty())
		{
			SecondarySourceLocation ssl;
			for (auto function: contract->annotation().unimplementedFunctions)
				ssl.append("Missing implementation:", function->location());
			string msg = "Trying to create an instance of an abstract contract.";
			ssl.limitSize(msg);
			m_errorReporter.typeError(
				_newExpression.location(),
				ssl,
				msg
			);
		}
		if (!contract->constructorIsPublic())
			m_errorReporter.typeError(_newExpression.location(), "Contract with internal constructor cannot be created directly.");

		solAssert(!!m_scope, "");
		m_scope->annotation().contractDependencies.insert(contract);
		solAssert(
			!contract->annotation().linearizedBaseContracts.empty(),
			"Linearized base contracts not yet available."
		);
		if (contractDependenciesAreCyclic(*m_scope))
			m_errorReporter.typeError(
				_newExpression.location(),
				"Circular reference for contract creation (cannot create instance of derived or same contract)."
			);

		_newExpression.annotation().type = FunctionType::newExpressionType(*contract);
	}
	else if (type->category() == Type::Category::Array)
	{
		if (!type->canLiveOutsideStorage())
			m_errorReporter.fatalTypeError(
				_newExpression.typeName().location(),
				"Type cannot live outside storage."
			);
		if (!type->isDynamicallySized())
			m_errorReporter.typeError(
				_newExpression.typeName().location(),
				"Length has to be placed in parentheses after the array type for new expression."
			);
		type = ReferenceType::copyForLocationIfReference(DataLocation::Memory, type);
		_newExpression.annotation().type = make_shared<FunctionType>(
			TypePointers{make_shared<IntegerType>(256)},
			TypePointers{type},
			strings(),
			strings(),
			FunctionType::Kind::ObjectCreation,
			false,
			StateMutability::Pure
		);
		_newExpression.annotation().isPure = true;
	}
	else
		m_errorReporter.fatalTypeError(_newExpression.location(), "Contract or array type expected.");
}

bool TypeChecker::visit(MemberAccess const& _memberAccess)
{
	_memberAccess.expression().accept(*this);
	TypePointer exprType = type(_memberAccess.expression());
	ASTString const& memberName = _memberAccess.memberName();

	// Retrieve the types of the arguments if this is used to call a function.
	auto const& argumentTypes = _memberAccess.annotation().argumentTypes;
	MemberList::MemberMap possibleMembers = exprType->members(m_scope).membersByName(memberName);
	size_t const initialMemberCount = possibleMembers.size();
	if (initialMemberCount > 1 && argumentTypes)
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

	auto& annotation = _memberAccess.annotation();

	if (possibleMembers.size() == 0)
	{
		if (initialMemberCount == 0)
		{
			// Try to see if the member was removed because it is only available for storage types.
			auto storageType = ReferenceType::copyForLocationIfReference(
				DataLocation::Storage,
				exprType
			);
			if (!storageType->members(m_scope).membersByName(memberName).empty())
				m_errorReporter.fatalTypeError(
					_memberAccess.location(),
					"Member \"" + memberName + "\" is not available in " +
					exprType->toString() +
					" outside of storage."
				);
		}
		string errorMsg = "Member \"" + memberName + "\" not found or not visible "
				"after argument-dependent lookup in " + exprType->toString() +
				(memberName == "value" ? " - did you forget the \"payable\" modifier?" : ".");
		if (exprType->category() == Type::Category::Contract)
			for (auto const& addressMember: IntegerType(160, IntegerType::Modifier::Address).nativeMembers(nullptr))
				if (addressMember.name == memberName)
				{
					Identifier const* var = dynamic_cast<Identifier const*>(&_memberAccess.expression());
					string varName = var ? var->name() : "...";
					errorMsg += " Use \"address(" + varName + ")." + memberName + "\" to access this address member.";
					break;
				}
		m_errorReporter.fatalTypeError(
			_memberAccess.location(),
			errorMsg
		);
	}
	else if (possibleMembers.size() > 1)
		m_errorReporter.fatalTypeError(
			_memberAccess.location(),
			"Member \"" + memberName + "\" not unique "
			"after argument-dependent lookup in " + exprType->toString() +
			(memberName == "value" ? " - did you forget the \"payable\" modifier?" : ".")
		);

	annotation.referencedDeclaration = possibleMembers.front().declaration;
	annotation.type = possibleMembers.front().type;

	if (auto funType = dynamic_cast<FunctionType const*>(annotation.type.get()))
		if (funType->bound() && !exprType->isImplicitlyConvertibleTo(*funType->selfType()))
			m_errorReporter.typeError(
				_memberAccess.location(),
				"Function \"" + memberName + "\" cannot be called on an object of type " +
				exprType->toString() + " (expected " + funType->selfType()->toString() + ")."
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

	if (exprType->category() == Type::Category::Contract)
	{
		// Warn about using send or transfer with a non-payable fallback function.
		if (auto callType = dynamic_cast<FunctionType const*>(type(_memberAccess).get()))
		{
			auto kind = callType->kind();
			auto contractType = dynamic_cast<ContractType const*>(exprType.get());
			solAssert(!!contractType, "Should be contract type.");

			if (
				(kind == FunctionType::Kind::Send || kind == FunctionType::Kind::Transfer) &&
				!contractType->isPayable()
			)
				m_errorReporter.typeError(
					_memberAccess.location(),
					"Value transfer to a contract without a payable fallback function."
				);
		}
	}

	// TODO some members might be pure, but for example `address(0x123).balance` is not pure
	// although every subexpression is, so leaving this limited for now.
	if (auto tt = dynamic_cast<TypeType const*>(exprType.get()))
		if (tt->actualType()->category() == Type::Category::Enum)
			annotation.isPure = true;
	if (auto magicType = dynamic_cast<MagicType const*>(exprType.get()))
		if (magicType->kind() == MagicType::Kind::ABI)
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
			m_errorReporter.typeError(_access.location(), "Index expression cannot be omitted.");
		else if (actualType.isString())
		{
			m_errorReporter.typeError(_access.location(), "Index access for string is not possible.");
			index->accept(*this);
		}
		else
		{
			expectType(*index, IntegerType(256));
			if (auto numberType = dynamic_cast<RationalNumberType const*>(type(*index).get()))
			{
				if (!numberType->isFractional()) // error is reported above
					if (!actualType.isDynamicallySized() && actualType.length() <= numberType->literalValue(nullptr))
						m_errorReporter.typeError(_access.location(), "Out of bounds array access.");
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
			m_errorReporter.typeError(_access.location(), "Index expression cannot be omitted.");
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
				m_errorReporter.fatalTypeError(index->location(), "Integer constant expected.");
		}
		break;
	}
	case Type::Category::FixedBytes:
	{
		FixedBytesType const& bytesType = dynamic_cast<FixedBytesType const&>(*baseType);
		if (!index)
			m_errorReporter.typeError(_access.location(), "Index expression cannot be omitted.");
		else
		{
			expectType(*index, IntegerType(256));
			if (auto integerType = dynamic_cast<RationalNumberType const*>(type(*index).get()))
				if (bytesType.numBytes() <= integerType->literalValue(nullptr))
					m_errorReporter.typeError(_access.location(), "Out of bounds array access.");
		}
		resultType = make_shared<FixedBytesType>(1);
		isLValue = false; // @todo this heavily depends on how it is embedded
		break;
	}
	default:
		m_errorReporter.fatalTypeError(
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
				m_errorReporter.fatalTypeError(_identifier.location(), "No matching declaration found after variable lookup.");
			else if (candidates.size() == 1)
				annotation.referencedDeclaration = candidates.front();
			else
				m_errorReporter.fatalTypeError(_identifier.location(), "No unique declaration found after variable lookup.");
		}
		else if (annotation.overloadedDeclarations.empty())
			m_errorReporter.fatalTypeError(_identifier.location(), "No candidates for overload resolution found.");
		else if (annotation.overloadedDeclarations.size() == 1)
			annotation.referencedDeclaration = *annotation.overloadedDeclarations.begin();
		else
		{
			vector<Declaration const*> candidates;

			for (Declaration const* declaration: annotation.overloadedDeclarations)
			{
				FunctionTypePointer functionType = declaration->functionType(true);
				solAssert(!!functionType, "Requested type not present.");
				if (functionType->canTakeArguments(*annotation.argumentTypes))
					candidates.push_back(declaration);
			}
			if (candidates.empty())
				m_errorReporter.fatalTypeError(_identifier.location(), "No matching declaration found after argument-dependent lookup.");
			else if (candidates.size() == 1)
				annotation.referencedDeclaration = candidates.front();
			else
				m_errorReporter.fatalTypeError(_identifier.location(), "No unique declaration found after argument-dependent lookup.");
		}
	}
	solAssert(
		!!annotation.referencedDeclaration,
		"Referenced declaration is null after overload resolution."
	);
	annotation.isLValue = annotation.referencedDeclaration->isLValue();
	annotation.type = annotation.referencedDeclaration->type();
	if (!annotation.type)
		m_errorReporter.fatalTypeError(_identifier.location(), "Declaration referenced before type could be determined.");
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
		// Assign type here if it even looks like an address. This prevents double errors for invalid addresses
		_literal.annotation().type = make_shared<IntegerType>(160, IntegerType::Modifier::Address);

		string msg;
		if (_literal.value().length() != 42) // "0x" + 40 hex digits
			// looksLikeAddress enforces that it is a hex literal starting with "0x"
			msg =
				"This looks like an address but is not exactly 40 hex digits. It is " +
				to_string(_literal.value().length() - 2) +
				" hex digits.";
		else if (!_literal.passesAddressChecksum())
		{
			msg = "This looks like an address but has an invalid checksum.";
			if (!_literal.getChecksummedAddress().empty())
				msg += " Correct checksummed address: \"" + _literal.getChecksummedAddress() + "\".";
		}

		if (!msg.empty())
			m_errorReporter.syntaxError(
				_literal.location(),
				msg +
				" If this is not used as an address, please prepend '00'. " +
				"For more information please see https://solidity.readthedocs.io/en/develop/types.html#address-literals"
			);
	}

	if (_literal.isHexNumber() && _literal.subDenomination() != Literal::SubDenomination::None)
		m_errorReporter.fatalTypeError(
			_literal.location(),
			"Hexadecimal numbers cannot be used with unit denominations. "
			"You can use an expression of the form \"0x1234 * 1 day\" instead."
		);

	if (_literal.subDenomination() == Literal::SubDenomination::Year)
		m_errorReporter.typeError(
			_literal.location(),
			"Using \"years\" as a unit denomination is deprecated."
		);

	if (!_literal.annotation().type)
		_literal.annotation().type = Type::forLiteral(_literal);

	if (!_literal.annotation().type)
		m_errorReporter.fatalTypeError(_literal.location(), "Invalid literal value.");

	_literal.annotation().isPure = true;
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
			m_errorReporter.typeError(
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
			m_errorReporter.typeError(
				_expression.location(),
				"Type " +
				type(_expression)->toString() +
				" is not implicitly convertible to expected type " +
				_expectedType.toString() +
				"."
			);
	}

	if (
		type(_expression)->category() == Type::Category::RationalNumber &&
		_expectedType.category() == Type::Category::FixedBytes
	)
	{
		auto literal = dynamic_cast<Literal const*>(&_expression);

		if (literal && !literal->isHexNumber())
			m_errorReporter.warning(
				_expression.location(),
				"Decimal literal assigned to bytesXX variable will be left-aligned. "
				"Use an explicit conversion to silence this warning."
			);
	}

}

void TypeChecker::requireLValue(Expression const& _expression)
{
	_expression.annotation().lValueRequested = true;
	_expression.accept(*this);

	if (_expression.annotation().isConstant)
		m_errorReporter.typeError(_expression.location(), "Cannot assign to a constant variable.");
	else if (!_expression.annotation().isLValue)
		m_errorReporter.typeError(_expression.location(), "Expression has to be an lvalue.");
}
