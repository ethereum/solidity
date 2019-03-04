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
 * Component that verifies overloads, abstract contracts, function clashes and others
 * checks at contract or function level.
 */

#include <libsolidity/analysis/ContractLevelChecker.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <liblangutil/ErrorReporter.h>
#include <boost/range/adaptor/reversed.hpp>


using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;


bool ContractLevelChecker::check(ContractDefinition const& _contract)
{
	checkDuplicateFunctions(_contract);
	checkDuplicateEvents(_contract);
	checkIllegalOverrides(_contract);
	checkAbstractFunctions(_contract);
	checkBaseConstructorArguments(_contract);
	checkConstructor(_contract);
	checkFallbackFunction(_contract);
	checkExternalTypeClashes(_contract);
	checkHashCollisions(_contract);
	checkLibraryRequirements(_contract);
	checkBaseABICompatibility(_contract);

	return Error::containsOnlyWarnings(m_errorReporter.errors());
}

void ContractLevelChecker::checkDuplicateFunctions(ContractDefinition const& _contract)
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

void ContractLevelChecker::checkDuplicateEvents(ContractDefinition const& _contract)
{
	/// Checks that two events with the same name defined in this contract have different
	/// argument types
	map<string, vector<EventDefinition const*>> events;
	for (EventDefinition const* event: _contract.events())
		events[event->name()].push_back(event);

	findDuplicateDefinitions(events, "Event with same name and arguments defined twice.");
}

template <class T>
void ContractLevelChecker::findDuplicateDefinitions(map<string, vector<T>> const& _definitions, string _message)
{
	for (auto const& it: _definitions)
	{
		vector<T> const& overloads = it.second;
		set<size_t> reported;
		for (size_t i = 0; i < overloads.size() && !reported.count(i); ++i)
		{
			SecondarySourceLocation ssl;

			for (size_t j = i + 1; j < overloads.size(); ++j)
				if (FunctionType(*overloads[i]).asCallableFunction(false)->hasEqualParameterTypes(
					*FunctionType(*overloads[j]).asCallableFunction(false))
				)
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

void ContractLevelChecker::checkIllegalOverrides(ContractDefinition const& _contract)
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

void ContractLevelChecker::checkFunctionOverride(FunctionDefinition const& _function, FunctionDefinition const& _super)
{
	FunctionTypePointer functionType = FunctionType(_function).asCallableFunction(false);
	FunctionTypePointer superType = FunctionType(_super).asCallableFunction(false);

	if (!functionType->hasEqualParameterTypes(*superType))
		return;
	if (!functionType->hasEqualReturnTypes(*superType))
		overrideError(_function, _super, "Overriding function return types differ.");

	if (!_function.annotation().superFunction)
		_function.annotation().superFunction = &_super;

	if (_function.visibility() != _super.visibility())
	{
		// Visibility change from external to public is fine.
		// Any other change is disallowed.
		if (!(
			_super.visibility() == FunctionDefinition::Visibility::External &&
			_function.visibility() == FunctionDefinition::Visibility::Public
		))
			overrideError(_function, _super, "Overriding function visibility differs.");
	}
	if (_function.stateMutability() != _super.stateMutability())
		overrideError(
			_function,
			_super,
			"Overriding function changes state mutability from \"" +
			stateMutabilityToString(_super.stateMutability()) +
			"\" to \"" +
			stateMutabilityToString(_function.stateMutability()) +
			"\"."
		);
}

void ContractLevelChecker::overrideError(FunctionDefinition const& function, FunctionDefinition const& super, string message)
{
	m_errorReporter.typeError(
		function.location(),
		SecondarySourceLocation().append("Overridden function is here:", super.location()),
		message
	);
}

void ContractLevelChecker::checkAbstractFunctions(ContractDefinition const& _contract)
{
	// Mapping from name to function definition (exactly one per argument type equality class) and
	// flag to indicate whether it is fully implemented.
	using FunTypeAndFlag = std::pair<FunctionTypePointer, bool>;
	map<string, vector<FunTypeAndFlag>> functions;

	auto registerFunction = [&](Declaration const& _declaration, FunctionTypePointer const& _type, bool _implemented)
	{
		auto& overloads = functions[_declaration.name()];
		auto it = find_if(overloads.begin(), overloads.end(), [&](FunTypeAndFlag const& _funAndFlag)
		{
			return _type->hasEqualParameterTypes(*_funAndFlag.first);
		});
		if (it == overloads.end())
			overloads.emplace_back(_type, _implemented);
		else if (it->second)
		{
			if (!_implemented)
				m_errorReporter.typeError(_declaration.location(), "Redeclaring an already implemented function as abstract");
		}
		else if (_implemented)
			it->second = true;
	};

	// Search from base to derived, collect all functions and update
	// the 'implemented' flag.
	for (ContractDefinition const* contract: boost::adaptors::reverse(_contract.annotation().linearizedBaseContracts))
	{
		for (VariableDeclaration const* v: contract->stateVariables())
			if (v->isPartOfExternalInterface())
				registerFunction(*v, make_shared<FunctionType>(*v), true);

		for (FunctionDefinition const* function: contract->definedFunctions())
			if (!function->isConstructor())
				registerFunction(
					*function,
					make_shared<FunctionType>(*function)->asCallableFunction(false),
					function->isImplemented()
				);
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


void ContractLevelChecker::checkBaseConstructorArguments(ContractDefinition const& _contract)
{
	vector<ContractDefinition const*> const& bases = _contract.annotation().linearizedBaseContracts;

	// Determine the arguments that are used for the base constructors.
	for (ContractDefinition const* contract: bases)
	{
		if (FunctionDefinition const* constructor = contract->constructor())
			for (auto const& modifier: constructor->modifiers())
				if (auto baseContract = dynamic_cast<ContractDefinition const*>(
					modifier->name()->annotation().referencedDeclaration
				))
				{
					if (modifier->arguments())
					{
						if (baseContract->constructor())
							annotateBaseConstructorArguments(_contract, baseContract->constructor(), modifier.get());
					}
					else
						m_errorReporter.declarationError(
							modifier->location(),
							"Modifier-style base constructor call without arguments."
						);
				}

		for (ASTPointer<InheritanceSpecifier> const& base: contract->baseContracts())
		{
			ContractDefinition const* baseContract = dynamic_cast<ContractDefinition const*>(
				base->name().annotation().referencedDeclaration
			);
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

void ContractLevelChecker::annotateBaseConstructorArguments(
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

void ContractLevelChecker::checkConstructor(ContractDefinition const& _contract)
{
	FunctionDefinition const* constructor = _contract.constructor();
	if (!constructor)
		return;

	if (!constructor->returnParameters().empty())
		m_errorReporter.typeError(constructor->returnParameterList()->location(), "Non-empty \"returns\" directive for constructor.");
	if (constructor->stateMutability() != StateMutability::NonPayable && constructor->stateMutability() != StateMutability::Payable)
		m_errorReporter.typeError(
			constructor->location(),
			"Constructor must be payable or non-payable, but is \"" +
			stateMutabilityToString(constructor->stateMutability()) +
			"\"."
		);
	if (constructor->visibility() != FunctionDefinition::Visibility::Public && constructor->visibility() != FunctionDefinition::Visibility::Internal)
		m_errorReporter.typeError(constructor->location(), "Constructor must be public or internal.");
}

void ContractLevelChecker::checkFallbackFunction(ContractDefinition const& _contract)
{
	FunctionDefinition const* fallback = _contract.fallbackFunction();
	if (!fallback)
		return;

	if (_contract.isLibrary())
		m_errorReporter.typeError(fallback->location(), "Libraries cannot have fallback functions.");
	if (fallback->stateMutability() != StateMutability::NonPayable && fallback->stateMutability() != StateMutability::Payable)
		m_errorReporter.typeError(
			fallback->location(),
			"Fallback function must be payable or non-payable, but is \"" +
			stateMutabilityToString(fallback->stateMutability()) +
			"\"."
	);
	if (!fallback->parameters().empty())
		m_errorReporter.typeError(fallback->parameterList().location(), "Fallback function cannot take parameters.");
	if (!fallback->returnParameters().empty())
		m_errorReporter.typeError(fallback->returnParameterList()->location(), "Fallback function cannot return values.");
	if (fallback->visibility() != FunctionDefinition::Visibility::External)
		m_errorReporter.typeError(fallback->location(), "Fallback function must be defined as \"external\".");
}

void ContractLevelChecker::checkExternalTypeClashes(ContractDefinition const& _contract)
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
					externalDeclarations[functionType->externalSignature()].emplace_back(
						f, functionType->asCallableFunction(false)
					);
			}
		for (VariableDeclaration const* v: contract->stateVariables())
			if (v->isPartOfExternalInterface())
			{
				auto functionType = make_shared<FunctionType>(*v);
				// under non error circumstances this should be true
				if (functionType->interfaceFunctionType())
					externalDeclarations[functionType->externalSignature()].emplace_back(
						v, functionType->asCallableFunction(false)
					);
			}
	}
	for (auto const& it: externalDeclarations)
		for (size_t i = 0; i < it.second.size(); ++i)
			for (size_t j = i + 1; j < it.second.size(); ++j)
				if (!it.second[i].second->hasEqualParameterTypes(*it.second[j].second))
					m_errorReporter.typeError(
						it.second[j].first->location(),
						"Function overload clash during conversion to external types for arguments."
					);
}

void ContractLevelChecker::checkHashCollisions(ContractDefinition const& _contract)
{
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
}

void ContractLevelChecker::checkLibraryRequirements(ContractDefinition const& _contract)
{
	if (!_contract.isLibrary())
		return;

	if (!_contract.baseContracts().empty())
		m_errorReporter.typeError(_contract.location(), "Library is not allowed to inherit.");

	for (auto const& var: _contract.stateVariables())
		if (!var->isConstant())
			m_errorReporter.typeError(var->location(), "Library cannot have non-constant state variables");
}

void ContractLevelChecker::checkBaseABICompatibility(ContractDefinition const& _contract)
{
	if (_contract.sourceUnit().annotation().experimentalFeatures.count(ExperimentalFeature::ABIEncoderV2))
		return;

	if (_contract.isLibrary())
	{
		solAssert(
			_contract.baseContracts().empty() || m_errorReporter.hasErrors(),
			"Library is not allowed to inherit"
		);
		return;
	}

	SecondarySourceLocation errors;

	// interfaceFunctionList contains all inherited functions as well
	for (auto const& func: _contract.interfaceFunctionList())
	{
		solAssert(func.second->hasDeclaration(), "Function has no declaration?!");

		if (!func.second->declaration().sourceUnit().annotation().experimentalFeatures.count(ExperimentalFeature::ABIEncoderV2))
			continue;

		auto const& currentLoc = func.second->declaration().location();

		for (TypePointer const& paramType: func.second->parameterTypes() + func.second->parameterTypes())
			if (!TypeChecker::typeSupportedByOldABIEncoder(*paramType, false))
			{
				errors.append("Type only supported by the new experimental ABI encoder", currentLoc);
				break;
			}
	}

	if (!errors.infos.empty())
		m_errorReporter.fatalTypeError(
			_contract.location(),
			errors,
			std::string("Contract \"") +
			_contract.name() +
			"\" does not use the new experimental ABI encoder but wants to inherit from a contract " +
			"which uses types that require it. " +
			"Use \"pragma experimental ABIEncoderV2;\" for the inheriting contract as well to enable the feature."
		);

}
