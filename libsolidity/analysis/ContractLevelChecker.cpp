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
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <liblangutil/ErrorReporter.h>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string/predicate.hpp>


using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;

namespace
{

template <class T>
bool hasEqualNameAndParameters(T const& _a, T const& _b)
{
	return
		_a.name() == _b.name() &&
		FunctionType(_a).asCallableFunction(false)->hasEqualParameterTypes(
			*FunctionType(_b).asCallableFunction(false)
		);
}

vector<ContractDefinition const*> resolveBaseContracts(ContractDefinition const& _contract)
{
	vector<ContractDefinition const*> resolvedContracts;

	for (ASTPointer<InheritanceSpecifier> const& specifier: _contract.baseContracts())
	{
		Declaration const* baseDecl =
			specifier->name().annotation().referencedDeclaration;
		resolvedContracts.emplace_back(dynamic_cast<ContractDefinition const*>(baseDecl));
	}

	return resolvedContracts;
}

}

bool LessFunction::operator()(ModifierDefinition const* _a, ModifierDefinition const* _b) const
{
	return _a->name() < _b->name();
}

bool LessFunction::operator()(FunctionDefinition const* _a, FunctionDefinition const* _b) const
{
	if (_a->name() != _b->name())
		return _a->name() < _b->name();

	return boost::lexicographical_compare(
		FunctionType(*_a).asCallableFunction(false)->parameterTypes(),
		FunctionType(*_b).asCallableFunction(false)->parameterTypes(),
		[](auto const& _paramTypeA, auto const& _paramTypeB)
		{
			return _paramTypeA->richIdentifier() < _paramTypeB->richIdentifier();
		}
	);
}

bool ContractLevelChecker::check(ContractDefinition const& _contract)
{
	checkDuplicateFunctions(_contract);
	checkDuplicateEvents(_contract);
	checkIllegalOverrides(_contract);
	checkAmbiguousOverrides(_contract);
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
				if (hasEqualNameAndParameters(*overloads[i], *overloads[j]))
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
	FunctionSet const& funcSet = getBaseFunctions(&_contract);
	ModifierSet const& modSet = getBaseModifiers(&_contract);

	struct MatchByName
	{
		string const& m_name;
		bool operator()(CallableDeclaration const* _callable)
		{
			return _callable->name() == m_name;
		}
	};

	for (ModifierDefinition const* modifier: _contract.functionModifiers())
	{
		if (contains_if(funcSet, MatchByName{modifier->name()}))
			m_errorReporter.typeError(
				modifier->location(),
				"Override changes function to modifier."
			);

		auto [begin,end] = modSet.equal_range(modifier);

		// Skip if no modifiers found in bases
		if (begin == end)
			continue;

		if (!modifier->overrides())
			overrideError(*modifier, **begin, "Overriding modifier is missing 'override' specifier.");

		for (; begin != end; begin++)
			if (ModifierType(**begin) != ModifierType(*modifier))
				m_errorReporter.typeError(
					modifier->location(),
					"Override changes modifier signature."
			);
	}

	for (FunctionDefinition const* function: _contract.definedFunctions())
	{
		if (contains_if(modSet, MatchByName{function->name()}))
			m_errorReporter.typeError(function->location(), "Override changes modifier to function.");

		// Skip if not overridable
		if (!function->isOverridable())
			continue;

		// No overriding functions found
		if (funcSet.find(function) == funcSet.cend())
		{
			// Skip if no override
			if (!function->overrides())
				continue;
			else
				m_errorReporter.typeError(
					function->overrides()->location(),
					"Function has override specified but doesn't override anything."
				);
		}

		set<ContractDefinition const*> specifiedContracts =
			function->overrides() ?
			resolveOverrideList(*function->overrides()) :
			set<ContractDefinition const*>{};
			);

		decltype(specifiedContracts) missingContracts;

		// Iterate over the overrides
		for (auto [begin, end] = funcSet.equal_range(function); begin != end; begin++)
		{
			// Validate the override
			if (!checkFunctionOverride(*function, **begin))
				break;

			auto const result = find(
				specifiedContracts.cbegin(),
				specifiedContracts.cend(),
				(*begin)->annotation().contract
			);

			if (result == specifiedContracts.cend())
				missingContracts.insert((*begin)->annotation().contract);
			else
				specifiedContracts.erase(result);
		}

		if (missingContracts.size() > 1)
			overrideListError(
				*function,
				missingContracts,
				"Function needs to specify overridden ",
				""
			);

		if (!specifiedContracts.empty())
			overrideListError(
				*function,
				specifiedContracts,
				"Invalid ",
				"specified in override list: "
			);
	}
}

bool ContractLevelChecker::checkFunctionOverride(FunctionDefinition const& _function, FunctionDefinition const& _super)
{
	FunctionTypePointer functionType = FunctionType(_function).asCallableFunction(false);
	FunctionTypePointer superType = FunctionType(_super).asCallableFunction(false);

	bool success = true;

	if (!functionType->hasEqualParameterTypes(*superType))
		return true;

	if (!_function.overrides())
	{
		overrideError(_function, _super, "Overriding function is missing 'override' specifier.");
		success = false;
	}

	if (!functionType->hasEqualReturnTypes(*superType))
	{
		overrideError(_function, _super, "Overriding function return types differ.");
		success = false;
	}

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
		{
			overrideError(_function, _super, "Overriding function visibility differs.");
			success = false;
		}
	}
	if (_function.stateMutability() != _super.stateMutability())
	{
		overrideError(
			_function,
			_super,
			"Overriding function changes state mutability from \"" +
			stateMutabilityToString(_super.stateMutability()) +
			"\" to \"" +
			stateMutabilityToString(_function.stateMutability()) +
			"\"."
		);
		success = false;
	}

	return success;
}

void ContractLevelChecker::overrideListError(FunctionDefinition const& function, set<ContractDefinition const*> _secondary, string const& _message1, string const& _message2)
{
	// Using a set rather than a vector so the order is always the same
	set<string> names;
	SecondarySourceLocation ssl;
	for (Declaration const* c: _secondary)
	{
		ssl.append("This contract: ", c->location());
		names.insert(c->name());
	}
	string contractSingularPlural = "contract ";
	if (_secondary.size() > 1)
		contractSingularPlural = "contracts ";

	m_errorReporter.typeError(
		function.overrides() ? function.overrides()->location() : function.location(),
		ssl,
		_message1 +
		contractSingularPlural +
		_message2 +
		joinHumanReadable(names, ", ", " and ") +
		"."
	);
}

void ContractLevelChecker::overrideError(CallableDeclaration const& function, CallableDeclaration const& super, string message)
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
				registerFunction(*v, TypeProvider::function(*v), true);

		for (FunctionDefinition const* function: contract->definedFunctions())
			if (!function->isConstructor())
				registerFunction(
					*function,
					TypeProvider::function(*function)->asCallableFunction(false),
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
			ssl.append("First constructor call is here:", _argumentNode->location());
			ssl.append("Second constructor call is here:", previousNode->location());
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
				auto functionType = TypeProvider::function(*f);
				// under non error circumstances this should be true
				if (functionType->interfaceFunctionType())
					externalDeclarations[functionType->externalSignature()].emplace_back(
						f, functionType->asCallableFunction(false)
					);
			}
		for (VariableDeclaration const* v: contract->stateVariables())
			if (v->isPartOfExternalInterface())
			{
				auto functionType = TypeProvider::function(*v);
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

void ContractLevelChecker::checkAmbiguousOverrides(ContractDefinition const& _contract) const
{
	vector<FunctionDefinition const*> contractFuncs = _contract.definedFunctions();

	auto const resolvedBases = resolveBaseContracts(_contract);

	FunctionSet collectedFunctions;

	// Collect the most inherited functions of the direct base contracts
	for (ContractDefinition const* contract: resolveBaseContracts(_contract))
	{
		FunctionSet baseSet = convertContainer<FunctionSet>(contract->definedFunctions());

		for (FunctionDefinition const* function: getBaseFunctions(contract))
		{
			if (baseSet.cend() == baseSet.find(function))
				baseSet.insert(function);
		}

		if (baseSet.empty())
			continue;

		// Collect the first of every override-set of functions (they are sorted
		// by most-inherited function)
		for (auto it = baseSet.cbegin(); it != baseSet.cend(); it = baseSet.upper_bound(*it))
		{
			if (!(*it)->isOverridable())
				continue;

			collectedFunctions.insert(*it);
		}
	}

	// Check the sets of the most-inherited functions
	for (auto it = collectedFunctions.cbegin(); it != collectedFunctions.cend(); it = collectedFunctions.upper_bound(*it))
	{
		auto [begin,end] = collectedFunctions.equal_range(*it);

		// Only one function
		if (next(begin) == end)
			continue;

		// Not an overridable function
		if (!(*it)->isOverridable())
			continue;

		// Function has been explicitly overridden
		if (contains_if(
				contractFuncs,
				[&] (FunctionDefinition const* _f) {
					return hasEqualNameAndParameters(*_f, **it);
				}
			)
		)
			continue;

		set<FunctionDefinition const*> ambiguousFunctions;
		SecondarySourceLocation ssl;

		for (;begin != end; begin++)
		{
			ambiguousFunctions.insert(*begin);
			ssl.append("Definition here: ", (*begin)->location());
		}

		// Make sure the functions are not from the same base contract
		if (ambiguousFunctions.size() == 1)
			continue;

		m_errorReporter.typeError(
			_contract.location(),
			ssl,
			"Functions of the same name " +
			(*it)->name() +
			" and parameter types defined in two or more base contracts must be overridden in the derived contract."
		);
	}
}

set<ContractDefinition const*> ContractLevelChecker::resolveOverrideList(OverrideSpecifier const& _overrides) const
{
	set<ContractDefinition const*> resolved;

	for (ASTPointer<UserDefinedTypeName> const& override: _overrides.overrides())
	{
		Declaration const* decl  = override->annotation().referencedDeclaration;
		solAssert(decl, "Expected declaration to be resolved.");

		// If it's not a contract it will be caught
		// in the reference resolver
		if (ContractDefinition const* contract = dynamic_cast<decltype(contract)>(decl))
			resolved.insert(contract);
	}

	return resolved;
}

ContractLevelChecker::FunctionSet const& ContractLevelChecker::getBaseFunctions(ContractDefinition const* _contract) const
{
	auto const& result = m_contractBaseFunctions.find(_contract);

	if (result != m_contractBaseFunctions.cend())
		return result->second;

	FunctionSet set;

	for (auto const* base: resolveBaseContracts(*_contract))
	{
		FunctionSet tmpSet = convertContainer<FunctionSet>(base->definedFunctions());

		for (auto const& func: getBaseFunctions(base))
			if (tmpSet.cend() == tmpSet.find(func))
				tmpSet.insert(func);

		set += tmpSet;
	}

	return m_contractBaseFunctions[_contract] = set;
}

ContractLevelChecker::ModifierSet const& ContractLevelChecker::getBaseModifiers(ContractDefinition const* _contract) const
{
	auto const& result = m_contractBaseModifiers.find(_contract);

	if (result != m_contractBaseModifiers.cend())
		return result->second;

	ModifierSet set;

	for (auto const* base: resolveBaseContracts(*_contract))
	{
		ModifierSet tmpSet = convertContainer<ModifierSet>(base->functionModifiers());

		for (auto const& mod: getBaseModifiers(base))
			if (tmpSet.cend() == tmpSet.find(mod))
				tmpSet.insert(mod);

		set += tmpSet;
	}

	return m_contractBaseModifiers[_contract] = set;
}
