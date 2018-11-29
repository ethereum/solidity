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

#include <liblangutil/ErrorReporter.h>


using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;


bool ContractLevelChecker::check(ContractDefinition const& _contract)
{
	checkContractDuplicateFunctions(_contract);
	checkContractDuplicateEvents(_contract);
	checkContractIllegalOverrides(_contract);

	return Error::containsOnlyWarnings(m_errorReporter.errors());
}

void ContractLevelChecker::checkContractDuplicateFunctions(ContractDefinition const& _contract)
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

void ContractLevelChecker::checkContractDuplicateEvents(ContractDefinition const& _contract)
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

void ContractLevelChecker::checkContractIllegalOverrides(ContractDefinition const& _contract)
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

