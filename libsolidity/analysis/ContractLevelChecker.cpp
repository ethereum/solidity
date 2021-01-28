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
 * Component that verifies overloads, abstract contracts, function clashes and others
 * checks at contract or function level.
 */

#include <libsolidity/analysis/ContractLevelChecker.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <libsolutil/FunctionSelector.h>
#include <liblangutil/ErrorReporter.h>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace
{

template <class T, class B>
bool hasEqualParameters(T const& _a, B const& _b)
{
	return FunctionType(_a).asExternallyCallableFunction(false)->hasEqualParameterTypes(
		*FunctionType(_b).asExternallyCallableFunction(false)
	);
}

template<typename T>
map<ASTString, vector<T const*>> filterDeclarations(
	map<ASTString, vector<Declaration const*>> const& _declarations)
{
	map<ASTString, vector<T const*>> filteredDeclarations;
	for (auto const& [name, overloads]: _declarations)
		for (auto const* declaration: overloads)
			if (auto typedDeclaration = dynamic_cast<T const*>(declaration))
				filteredDeclarations[name].push_back(typedDeclaration);
	return filteredDeclarations;
}

}

bool ContractLevelChecker::check(SourceUnit const& _sourceUnit)
{
	bool noErrors = true;
	findDuplicateDefinitions(
		filterDeclarations<FunctionDefinition>(*_sourceUnit.annotation().exportedSymbols)
	);
	// This check flags duplicate free events when free events become
	// a Solidity feature
	findDuplicateDefinitions(
		filterDeclarations<EventDefinition>(*_sourceUnit.annotation().exportedSymbols)
	);
	if (!Error::containsOnlyWarnings(m_errorReporter.errors()))
		noErrors = false;
	for (ASTPointer<ASTNode> const& node: _sourceUnit.nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
			if (!check(*contract))
				noErrors = false;
	return noErrors;
}

bool ContractLevelChecker::check(ContractDefinition const& _contract)
{
	_contract.annotation().unimplementedDeclarations = std::vector<Declaration const*>();

	checkDuplicateFunctions(_contract);
	checkDuplicateEvents(_contract);
	m_overrideChecker.check(_contract);
	checkBaseConstructorArguments(_contract);
	checkAbstractDefinitions(_contract);
	checkExternalTypeClashes(_contract);
	checkHashCollisions(_contract);
	checkLibraryRequirements(_contract);
	checkBaseABICompatibility(_contract);
	checkPayableFallbackWithoutReceive(_contract);
	checkStorageSize(_contract);

	return Error::containsOnlyWarnings(m_errorReporter.errors());
}

void ContractLevelChecker::checkDuplicateFunctions(ContractDefinition const& _contract)
{
	/// Checks that two functions with the same name defined in this contract have different
	/// argument types and that there is at most one constructor.
	map<string, vector<FunctionDefinition const*>> functions;
	FunctionDefinition const* constructor = nullptr;
	FunctionDefinition const* fallback = nullptr;
	FunctionDefinition const* receive = nullptr;
	for (FunctionDefinition const* function: _contract.definedFunctions())
		if (function->isConstructor())
		{
			if (constructor)
				m_errorReporter.declarationError(
					7997_error,
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
					7301_error,
					function->location(),
					SecondarySourceLocation().append("Another declaration is here:", fallback->location()),
					"Only one fallback function is allowed."
				);
			fallback = function;
		}
		else if (function->isReceive())
		{
			if (receive)
				m_errorReporter.declarationError(
					4046_error,
					function->location(),
					SecondarySourceLocation().append("Another declaration is here:", receive->location()),
					"Only one receive function is allowed."
				);
			receive = function;
		}
		else
		{
			solAssert(!function->name().empty(), "");
			functions[function->name()].push_back(function);
		}

	findDuplicateDefinitions(functions);
}

void ContractLevelChecker::checkDuplicateEvents(ContractDefinition const& _contract)
{
	/// Checks that two events with the same name defined in this contract have different
	/// argument types
	map<string, vector<EventDefinition const*>> events;
	for (auto const* contract: _contract.annotation().linearizedBaseContracts)
		for (EventDefinition const* event: contract->events())
			events[event->name()].push_back(event);

	findDuplicateDefinitions(events);
}

template <class T>
void ContractLevelChecker::findDuplicateDefinitions(map<string, vector<T>> const& _definitions)
{
	for (auto const& it: _definitions)
	{
		vector<T> const& overloads = it.second;
		set<size_t> reported;
		for (size_t i = 0; i < overloads.size() && !reported.count(i); ++i)
		{
			SecondarySourceLocation ssl;

			for (size_t j = i + 1; j < overloads.size(); ++j)
				if (hasEqualParameters(*overloads[i], *overloads[j]))
				{
					solAssert(
						(
							dynamic_cast<ContractDefinition const*>(overloads[i]->scope()) &&
							dynamic_cast<ContractDefinition const*>(overloads[j]->scope()) &&
							overloads[i]->name() == overloads[j]->name()
						) ||
						(
							dynamic_cast<SourceUnit const*>(overloads[i]->scope()) &&
							dynamic_cast<SourceUnit const*>(overloads[j]->scope())
						),
						"Override is neither a namesake function/event in contract scope nor "
						"a free function/event (alias)."
					);
					ssl.append("Other declaration is here:", overloads[j]->location());
					reported.insert(j);
				}

			if (ssl.infos.size() > 0)
			{
				ErrorId error;
				string message;
				if constexpr (is_same_v<T, FunctionDefinition const*>)
				{
					error = 1686_error;
					message = "Function with same name and parameter types defined twice.";
				}
				else
				{
					static_assert(is_same_v<T, EventDefinition const*>, "Expected \"FunctionDefinition const*\" or \"EventDefinition const*\"");
					error = 5883_error;
					message = "Event with same name and parameter types defined twice.";
				}

				ssl.limitSize(message);

				m_errorReporter.declarationError(
					error,
					overloads[i]->location(),
					ssl,
					message
				);
			}
		}
	}
}

void ContractLevelChecker::checkAbstractDefinitions(ContractDefinition const& _contract)
{
	// Collects functions, static variable getters and modifiers. If they
	// override (unimplemented) base class ones, they are replaced.
	set<OverrideProxy, OverrideProxy::CompareBySignature> proxies;

	auto registerProxy = [&proxies](OverrideProxy const& _overrideProxy)
	{
		// Overwrite an existing proxy, if it exists.
		if (!_overrideProxy.unimplemented())
			proxies.erase(_overrideProxy);

		proxies.insert(_overrideProxy);
	};

	// Search from base to derived, collect all functions and modifiers and
	// update proxies.
	for (ContractDefinition const* contract: boost::adaptors::reverse(_contract.annotation().linearizedBaseContracts))
	{
		for (VariableDeclaration const* v: contract->stateVariables())
			if (v->isPartOfExternalInterface())
				registerProxy(OverrideProxy(v));

		for (FunctionDefinition const* function: contract->definedFunctions())
			if (!function->isConstructor())
				registerProxy(OverrideProxy(function));

		for (ModifierDefinition const* modifier: contract->functionModifiers())
			registerProxy(OverrideProxy(modifier));
	}

	// Set to not fully implemented if at least one flag is false.
	// Note that `_contract.annotation().unimplementedDeclarations` has already been
	// pre-filled by `checkBaseConstructorArguments`.
	//
	for (auto const& proxy: proxies)
		if (proxy.unimplemented())
			_contract.annotation().unimplementedDeclarations->push_back(proxy.declaration());

	if (_contract.abstract())
	{
		if (_contract.contractKind() == ContractKind::Interface)
			m_errorReporter.typeError(9348_error, _contract.location(), "Interfaces do not need the \"abstract\" keyword, they are abstract implicitly.");
		else if (_contract.contractKind() == ContractKind::Library)
			m_errorReporter.typeError(9571_error, _contract.location(), "Libraries cannot be abstract.");
		else
			solAssert(_contract.contractKind() == ContractKind::Contract, "");
	}

	// For libraries, we emit errors on function-level, so this is fine as long as we do
	// not have inheritance for libraries.
	if (
		_contract.contractKind() == ContractKind::Contract &&
		!_contract.abstract() &&
		!_contract.annotation().unimplementedDeclarations->empty()
	)
	{
		SecondarySourceLocation ssl;
		for (auto declaration: *_contract.annotation().unimplementedDeclarations)
			ssl.append("Missing implementation: ", declaration->location());
		m_errorReporter.typeError(
			3656_error,
			_contract.location(),
			ssl,
			"Contract \"" + *_contract.annotation().canonicalName + "\" should be marked as abstract."
		);
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
					modifier->name().annotation().referencedDeclaration
				))
				{
					if (modifier->arguments())
					{
						if (baseContract->constructor())
							annotateBaseConstructorArguments(_contract, baseContract->constructor(), modifier.get());
					}
					else
						m_errorReporter.declarationError(
							1563_error,
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
					_contract.annotation().unimplementedDeclarations->push_back(constructor);
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
			3364_error,
			*mainLocation,
			ssl,
			"Base constructor arguments given twice."
		);
	}

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
						f, functionType->asExternallyCallableFunction(false)
					);
			}
		for (VariableDeclaration const* v: contract->stateVariables())
			if (v->isPartOfExternalInterface())
			{
				auto functionType = TypeProvider::function(*v);
				// under non error circumstances this should be true
				if (functionType->interfaceFunctionType())
					externalDeclarations[functionType->externalSignature()].emplace_back(
						v, functionType->asExternallyCallableFunction(false)
					);
			}
	}
	for (auto const& it: externalDeclarations)
		for (size_t i = 0; i < it.second.size(); ++i)
			for (size_t j = i + 1; j < it.second.size(); ++j)
				if (!it.second[i].second->hasEqualParameterTypes(*it.second[j].second))
					m_errorReporter.typeError(
						9914_error,
						it.second[j].first->location(),
						"Function overload clash during conversion to external types for arguments."
					);
}

void ContractLevelChecker::checkHashCollisions(ContractDefinition const& _contract)
{
	set<util::FixedHash<4>> hashes;
	for (auto const& it: _contract.interfaceFunctionList())
	{
		util::FixedHash<4> const& hash = it.first;
		if (hashes.count(hash))
			m_errorReporter.typeError(
				1860_error,
				_contract.location(),
				string("Function signature hash collision for ") + it.second->externalSignature()
			);
		hashes.insert(hash);
	}

	map<uint32_t, SourceLocation> errorHashes;
	// TODO all used errors?
	for (ErrorDefinition const* error: _contract.errors())
	{
		if (!error->functionType(true)->interfaceFunctionType())
			// Will create an error later on.
			continue;
		uint32_t hash = selectorFromSignature32(error->functionType(true)->externalSignature());
		if (errorHashes.count(hash))
			m_errorReporter.typeError(
				4883_error,
				_contract.location(),
				SecondarySourceLocation{}.append("This error has the same selector: ", errorHashes[hash]),
				string("Error signature hash collision for ") + error->functionType(true)->externalSignature()
			);
		else
			errorHashes[hash] = error->location();
	}
}

void ContractLevelChecker::checkLibraryRequirements(ContractDefinition const& _contract)
{
	if (!_contract.isLibrary())
		return;

	if (!_contract.baseContracts().empty())
		m_errorReporter.typeError(9469_error, _contract.location(), "Library is not allowed to inherit.");

	for (auto const& var: _contract.stateVariables())
		if (!var->isConstant())
			m_errorReporter.typeError(9957_error, var->location(), "Library cannot have non-constant state variables");
}

void ContractLevelChecker::checkBaseABICompatibility(ContractDefinition const& _contract)
{
	if (*_contract.sourceUnit().annotation().useABICoderV2)
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

		if (!*func.second->declaration().sourceUnit().annotation().useABICoderV2)
			continue;

		auto const& currentLoc = func.second->declaration().location();

		for (TypePointer const& paramType: func.second->parameterTypes() + func.second->returnParameterTypes())
			if (!TypeChecker::typeSupportedByOldABIEncoder(*paramType, false))
			{
				errors.append("Type only supported by ABIEncoderV2", currentLoc);
				break;
			}
	}

	if (!errors.infos.empty())
		m_errorReporter.fatalTypeError(
			6594_error,
			_contract.location(),
			errors,
			std::string("Contract \"") +
			_contract.name() +
			"\" does not use ABI coder v2 but wants to inherit from a contract " +
			"which uses types that require it. " +
			"Use \"pragma abicoder v2;\" for the inheriting contract as well to enable the feature."
		);

}

void ContractLevelChecker::checkPayableFallbackWithoutReceive(ContractDefinition const& _contract)
{
	if (auto const* fallback = _contract.fallbackFunction())
		if (fallback->isPayable() && !_contract.interfaceFunctionList().empty() && !_contract.receiveFunction())
			m_errorReporter.warning(
				3628_error,
				_contract.location(),
				"This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.",
				SecondarySourceLocation{}.append("The payable fallback function is defined here.", fallback->location())
			);
}

void ContractLevelChecker::checkStorageSize(ContractDefinition const& _contract)
{
	bigint size = 0;
	for (ContractDefinition const* contract: boost::adaptors::reverse(_contract.annotation().linearizedBaseContracts))
		for (VariableDeclaration const* variable: contract->stateVariables())
			if (!(variable->isConstant() || variable->immutable()))
			{
				size += variable->annotation().type->storageSizeUpperBound();
				if (size >= bigint(1) << 256)
				{
					m_errorReporter.typeError(7676_error, _contract.location(), "Contract requires too much storage.");
					break;
				}
			}
}
