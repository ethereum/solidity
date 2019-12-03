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

// Helper struct to do a search by name
struct MatchByName
{
	string const& m_name;
	bool operator()(CallableDeclaration const* _callable)
	{
		return _callable->name() == m_name;
	}
};

vector<ASTPointer<UserDefinedTypeName>> sortByContract(vector<ASTPointer<UserDefinedTypeName>> const& _list)
{
	auto sorted = _list;

	stable_sort(sorted.begin(), sorted.end(),
		[] (ASTPointer<UserDefinedTypeName> _a, ASTPointer<UserDefinedTypeName> _b) {
			if (!_a || !_b)
				return _a < _b;

			Declaration const* aDecl = _a->annotation().referencedDeclaration;
			Declaration const* bDecl = _b->annotation().referencedDeclaration;

			if (!aDecl || !bDecl)
				return aDecl < bDecl;

			return aDecl->id() < bDecl->id();
		}
	);

	return sorted;
}

template <class T, class B>
bool hasEqualNameAndParameters(T const& _a, B const& _b)
{
	return
		_a.name() == _b.name() &&
		FunctionType(_a).asCallableFunction(false)->hasEqualParameterTypes(
			*FunctionType(_b).asCallableFunction(false)
		);
}

vector<ContractDefinition const*> resolveDirectBaseContracts(ContractDefinition const& _contract)
{
	vector<ContractDefinition const*> resolvedContracts;

	for (ASTPointer<InheritanceSpecifier> const& specifier: _contract.baseContracts())
	{
		Declaration const* baseDecl =
			specifier->name().annotation().referencedDeclaration;
		auto contract = dynamic_cast<ContractDefinition const*>(baseDecl);
		solAssert(contract, "contract is null");
		resolvedContracts.emplace_back(contract);
	}

	return resolvedContracts;
}

}

bool ContractLevelChecker::LessFunction::operator()(ModifierDefinition const* _a, ModifierDefinition const* _b) const
{
	return _a->name() < _b->name();
}

bool ContractLevelChecker::LessFunction::operator()(FunctionDefinition const* _a, FunctionDefinition const* _b) const
{
	if (_a->name() != _b->name())
		return _a->name() < _b->name();

	if (_a->kind() != _b->kind())
		return _a->kind() < _b->kind();

	return boost::lexicographical_compare(
		FunctionType(*_a).asCallableFunction(false)->parameterTypes(),
		FunctionType(*_b).asCallableFunction(false)->parameterTypes(),
		[](auto const& _paramTypeA, auto const& _paramTypeB)
		{
			return _paramTypeA->richIdentifier() < _paramTypeB->richIdentifier();
		}
	);
}

bool ContractLevelChecker::LessFunction::operator()(ContractDefinition const* _a, ContractDefinition const* _b) const
{
	if (!_a || !_b)
		return _a < _b;

	return _a->id() < _b->id();
}

bool ContractLevelChecker::check(ContractDefinition const& _contract)
{
	checkDuplicateFunctions(_contract);
	checkDuplicateEvents(_contract);
	checkIllegalOverrides(_contract);
	checkAmbiguousOverrides(_contract);
	checkBaseConstructorArguments(_contract);
	checkAbstractFunctions(_contract);
	checkExternalTypeClashes(_contract);
	checkHashCollisions(_contract);
	checkLibraryRequirements(_contract);
	checkBaseABICompatibility(_contract);
	checkPayableFallbackWithoutReceive(_contract);

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
		else if (function->isReceive())
		{
			if (receive)
				m_errorReporter.declarationError(
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
	FunctionMultiSet const& funcSet = inheritedFunctions(_contract);
	ModifierMultiSet const& modSet = inheritedModifiers(_contract);

	checkModifierOverrides(funcSet, modSet, _contract.functionModifiers());

	for (auto const* stateVar: _contract.stateVariables())
	{
		if (!stateVar->isPublic())
			continue;

		bool found = false;
		for (
			auto it = find_if(funcSet.begin(), funcSet.end(), MatchByName{stateVar->name()});
			it != funcSet.end();
			it = find_if(++it, funcSet.end(), MatchByName{stateVar->name()})
		)
		{
			if (!hasEqualNameAndParameters(*stateVar, **it))
				continue;

			if ((*it)->visibility() != Declaration::Visibility::External)
				overrideError(*stateVar, **it, "Public state variables can only override functions with external visibility.");
			else
				checkFunctionOverride(*stateVar, **it);

			found = true;
		}

		if (!found && stateVar->overrides())
			m_errorReporter.typeError(
				stateVar->overrides()->location(),
				"Public state variable has override specified but does not override anything."
			);
	}

	for (FunctionDefinition const* function: _contract.definedFunctions())
	{
		if (function->isConstructor())
			continue;

		if (contains_if(modSet, MatchByName{function->name()}))
			m_errorReporter.typeError(function->location(), "Override changes modifier to function.");

		// No inheriting functions found
		if (!funcSet.count(function) && function->overrides())
			m_errorReporter.typeError(
				function->overrides()->location(),
				"Function has override specified but does not override anything."
			);

		checkOverrideList(funcSet, *function);
	}
}

template<class T>
void ContractLevelChecker::checkFunctionOverride(T const& _overriding, FunctionDefinition const& _super)
{
	string overridingName;

	if constexpr(std::is_same<FunctionDefinition, T>::value)
		overridingName = "function";
	else
		overridingName = "public state variable";

	FunctionTypePointer functionType = FunctionType(_overriding).asCallableFunction(false);
	FunctionTypePointer superType = FunctionType(_super).asCallableFunction(false);

	solAssert(functionType->hasEqualParameterTypes(*superType), "Override doesn't have equal parameters!");

	if (!_overriding.overrides())
		overrideError(_overriding, _super, "Overriding " + overridingName + " is missing 'override' specifier.");

	if (!_super.virtualSemantics())
		overrideError( _super, _overriding, "Trying to override non-virtual function. Did you forget to add \"virtual\"?", "Overriding " + overridingName + " is here:");

	if (!functionType->hasEqualReturnTypes(*superType))
		overrideError(_overriding, _super, "Overriding " + overridingName + " return types differ.");

	if constexpr(std::is_same<T, FunctionDefinition>::value)
		_overriding.annotation().baseFunctions.emplace(&_super);

	if (_overriding.visibility() != _super.visibility())
	{
		// Visibility change from external to public is fine.
		// Any other change is disallowed.
		if (!(
			_super.visibility() == FunctionDefinition::Visibility::External &&
			_overriding.visibility() == FunctionDefinition::Visibility::Public
		))
			overrideError(_overriding, _super, "Overriding " + overridingName + " visibility differs.");
	}

	if constexpr(std::is_same<T, FunctionDefinition>::value)
	{
		if (_overriding.stateMutability() != _super.stateMutability())
			overrideError(
				_overriding,
				_super,
				"Overriding function changes state mutability from \"" +
				stateMutabilityToString(_super.stateMutability()) +
				"\" to \"" +
				stateMutabilityToString(_overriding.stateMutability()) +
				"\"."
			);

		if (!_overriding.isImplemented() && _super.isImplemented())
			overrideError(
				_overriding,
				_super,
				"Overriding an implemented function with an unimplemented function is not allowed."
			);
	}
}

void ContractLevelChecker::overrideListError(FunctionDefinition const& function, set<ContractDefinition const*, LessFunction> _secondary, string const& _message1, string const& _message2)
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

void ContractLevelChecker::overrideError(Declaration const& _overriding, Declaration const& _super, string _message, string _secondaryMsg)
{
	m_errorReporter.typeError(
		_overriding.location(),
		SecondarySourceLocation().append(_secondaryMsg, _super.location()),
		_message
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
	// Note that `_contract.annotation().unimplementedFunctions` has already been
	// pre-filled by `checkBaseConstructorArguments`.
	for (auto const& it: functions)
		for (auto const& funAndFlag: it.second)
			if (!funAndFlag.second)
			{
				FunctionDefinition const* function = dynamic_cast<FunctionDefinition const*>(&funAndFlag.first->declaration());
				solAssert(function, "");
				_contract.annotation().unimplementedFunctions.push_back(function);
				break;
			}

	if (_contract.abstract())
	{
		if (_contract.contractKind() == ContractDefinition::ContractKind::Interface)
			m_errorReporter.typeError(_contract.location(), "Interfaces do not need the \"abstract\" keyword, they are abstract implicitly.");
		else if (_contract.contractKind() == ContractDefinition::ContractKind::Library)
			m_errorReporter.typeError(_contract.location(), "Libraries cannot be abstract.");
		else
			solAssert(_contract.contractKind() == ContractDefinition::ContractKind::Contract, "");
	}

	// For libraries, we emit errors on function-level, so this is fine as long as we do
	// not have inheritance for libraries.
	if (
		_contract.contractKind() == ContractDefinition::ContractKind::Contract &&
		!_contract.abstract() &&
		!_contract.annotation().unimplementedFunctions.empty()
	)
	{
		SecondarySourceLocation ssl;
		for (auto function: _contract.annotation().unimplementedFunctions)
			ssl.append("Missing implementation:", function->location());
		m_errorReporter.typeError(_contract.location(), ssl,
			"Contract \"" + _contract.annotation().canonicalName
			+ "\" should be marked as abstract.");
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
	// Fetch inherited functions and sort them by signature.
	// We get at least one function per signature and direct base contract, which is
	// enough because we re-construct the inheritance graph later.
	FunctionMultiSet nonOverriddenFunctions = inheritedFunctions(_contract);
	// Remove all functions that match the signature of a function in the current contract.
	nonOverriddenFunctions -= _contract.definedFunctions();

	// Walk through the set of functions signature by signature.
	for (auto it = nonOverriddenFunctions.cbegin(); it != nonOverriddenFunctions.cend();)
	{
		static constexpr auto compareById = [](auto const* a, auto const* b) { return a->id() < b->id(); };
		std::set<FunctionDefinition const*, decltype(compareById)> baseFunctions(compareById);
		for (auto nextSignature = nonOverriddenFunctions.upper_bound(*it); it != nextSignature; ++it)
			baseFunctions.insert(*it);

		if (baseFunctions.size() <= 1)
			continue;

		// Construct the override graph for this signature.
		// Reserve node 0 for the current contract and node
		// 1 for an artificial top node to which all override paths
		// connect at the end.
		struct OverrideGraph
		{
			OverrideGraph(decltype(baseFunctions) const& _baseFunctions)
			{
				for (auto const* baseFunction: _baseFunctions)
					addEdge(0, visit(baseFunction));
			}
			std::map<FunctionDefinition const*, int> nodes;
			std::map<int, FunctionDefinition const*> nodeInv;
			std::map<int, std::set<int>> edges;
			int numNodes = 2;
			void addEdge(int _a, int _b)
			{
				edges[_a].insert(_b);
				edges[_b].insert(_a);
			}
		private:
			/// Completes the graph starting from @a _function and
			/// @returns the node ID.
			int visit(FunctionDefinition const* _function)
			{
				auto it = nodes.find(_function);
				if (it != nodes.end())
					return it->second;
				int currentNode = numNodes++;
				nodes[_function] = currentNode;
				nodeInv[currentNode] = _function;
				if (_function->overrides())
					for (auto const* baseFunction: _function->annotation().baseFunctions)
						addEdge(currentNode, visit(baseFunction));
				else
					addEdge(currentNode, 1);

				return currentNode;
			}
		} overrideGraph(baseFunctions);

		// Detect cut vertices following https://en.wikipedia.org/wiki/Biconnected_component#Pseudocode
		// Can ignore the root node, since it is never a cut vertex in our case.
		struct CutVertexFinder
		{
			CutVertexFinder(OverrideGraph const& _graph): m_graph(_graph)
			{
				run();
			}
			std::set<FunctionDefinition const*> const& cutVertices() const { return m_cutVertices; }

		private:
			OverrideGraph const& m_graph;

			std::vector<bool> m_visited = std::vector<bool>(m_graph.numNodes, false);
			std::vector<int> m_depths = std::vector<int>(m_graph.numNodes, -1);
			std::vector<int> m_low = std::vector<int>(m_graph.numNodes, -1);
			std::vector<int> m_parent = std::vector<int>(m_graph.numNodes, -1);
			std::set<FunctionDefinition const*> m_cutVertices{};

			void run(int _u = 0, int _depth = 0)
			{
				m_visited.at(_u) = true;
				m_depths.at(_u) = m_low.at(_u) = _depth;
				for (int v: m_graph.edges.at(_u))
					if (!m_visited.at(v))
					{
						m_parent[v] = _u;
						run(v, _depth + 1);
						if (m_low[v] >= m_depths[_u] && m_parent[_u] != -1)
							m_cutVertices.insert(m_graph.nodeInv.at(_u));
						m_low[_u] = min(m_low[_u], m_low[v]);
					}
					else if (v != m_parent[_u])
						m_low[_u] = min(m_low[_u], m_depths[v]);
			}
		} cutVertexFinder{overrideGraph};

		// Remove all base functions overridden by cut vertices (they don't need to be overridden).
		for (auto const* function: cutVertexFinder.cutVertices())
		{
			std::set<FunctionDefinition const*> toTraverse = function->annotation().baseFunctions;
			while (!toTraverse.empty())
			{
				auto const* base = *toTraverse.begin();
				toTraverse.erase(toTraverse.begin());
				baseFunctions.erase(base);
				for (auto const* f: base->annotation().baseFunctions)
					toTraverse.insert(f);
			}
			// Remove unimplemented base functions at the cut vertices themselves as well.
			if (!function->isImplemented())
				baseFunctions.erase(function);
		}

		// If more than one function is left, they have to be overridden.
		if (baseFunctions.size() <= 1)
			continue;

		SecondarySourceLocation ssl;
		for (auto const* baseFunction: baseFunctions)
			ssl.append("Definition here: ", baseFunction->location());

		m_errorReporter.typeError(
			_contract.location(),
			ssl,
			"Derived contract must override function \"" +
			(*baseFunctions.begin())->name() +
			"\". Function with the same name and parameter types defined in two or more base classes."
		);
	}
}

set<ContractDefinition const*, ContractLevelChecker::LessFunction> ContractLevelChecker::resolveOverrideList(OverrideSpecifier const& _overrides) const
{
	set<ContractDefinition const*, LessFunction> resolved;

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


void ContractLevelChecker::checkModifierOverrides(FunctionMultiSet const& _funcSet, ModifierMultiSet const& _modSet, std::vector<ModifierDefinition const*> _modifiers)
{
	for (ModifierDefinition const* modifier: _modifiers)
	{
		if (contains_if(_funcSet, MatchByName{modifier->name()}))
			m_errorReporter.typeError(
				modifier->location(),
				"Override changes function to modifier."
			);

		auto [begin,end] = _modSet.equal_range(modifier);

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

}

void ContractLevelChecker::checkOverrideList(FunctionMultiSet const& _funcSet, FunctionDefinition const& _function)
{
	set<ContractDefinition const*, LessFunction> specifiedContracts =
		_function.overrides() ?
		resolveOverrideList(*_function.overrides()) :
		decltype(specifiedContracts){};

	// Check for duplicates in override list
	if (_function.overrides() && specifiedContracts.size() != _function.overrides()->overrides().size())
	{
		// Sort by contract id to find duplicate for error reporting
		vector<ASTPointer<UserDefinedTypeName>> list =
			sortByContract(_function.overrides()->overrides());

		// Find duplicates and output error
		for (size_t i = 1; i < list.size(); i++)
		{
			Declaration const* aDecl = list[i]->annotation().referencedDeclaration;
			Declaration const* bDecl = list[i-1]->annotation().referencedDeclaration;
			if (!aDecl || !bDecl)
				continue;

			if (aDecl->id() == bDecl->id())
			{
				SecondarySourceLocation ssl;
				ssl.append("First occurrence here: ", list[i-1]->location());
				m_errorReporter.typeError(
					list[i]->location(),
					ssl,
						"Duplicate contract \"" +
						joinHumanReadable(list[i]->namePath(), ".") +
						"\" found in override list of \"" +
						_function.name() +
						"\"."
				);
			}
		}
	}

	decltype(specifiedContracts) expectedContracts;

	// Build list of expected contracts
	for (auto [begin, end] = _funcSet.equal_range(&_function); begin != end; begin++)
	{
		// Validate the override
		checkFunctionOverride(_function, **begin);

		expectedContracts.insert((*begin)->annotation().contract);
	}

	decltype(specifiedContracts) missingContracts;
	decltype(specifiedContracts) surplusContracts;

	// If we expect only one contract, no contract needs to be specified
	if (expectedContracts.size() > 1)
		missingContracts = expectedContracts - specifiedContracts;

	surplusContracts = specifiedContracts - expectedContracts;

	if (!missingContracts.empty())
		overrideListError(
			_function,
			missingContracts,
			"Function needs to specify overridden ",
			""
		);

	if (!surplusContracts.empty())
		overrideListError(
			_function,
			surplusContracts,
			"Invalid ",
			"specified in override list: "
		);
}

ContractLevelChecker::FunctionMultiSet const& ContractLevelChecker::inheritedFunctions(ContractDefinition const& _contract) const
{
	if (!m_inheritedFunctions.count(&_contract))
	{
		FunctionMultiSet set;

		for (auto const* base: resolveDirectBaseContracts(_contract))
		{
			std::set<FunctionDefinition const*, LessFunction> functionsInBase;
			for (FunctionDefinition const* fun: base->definedFunctions())
				if (!fun->isConstructor())
					functionsInBase.emplace(fun);

			for (auto const& func: inheritedFunctions(*base))
				functionsInBase.insert(func);

			set += functionsInBase;
		}

		m_inheritedFunctions[&_contract] = set;
	}

	return m_inheritedFunctions[&_contract];
}

ContractLevelChecker::ModifierMultiSet const& ContractLevelChecker::inheritedModifiers(ContractDefinition const& _contract) const
{
	auto const& result = m_contractBaseModifiers.find(&_contract);

	if (result != m_contractBaseModifiers.cend())
		return result->second;

	ModifierMultiSet set;

	for (auto const* base: resolveDirectBaseContracts(_contract))
	{
		std::set<ModifierDefinition const*, LessFunction> tmpSet =
			convertContainer<decltype(tmpSet)>(base->functionModifiers());

		for (auto const& mod: inheritedModifiers(*base))
			tmpSet.insert(mod);

		set += tmpSet;
	}

	return m_contractBaseModifiers[&_contract] = set;
}

void ContractLevelChecker::checkPayableFallbackWithoutReceive(ContractDefinition const& _contract)
{
	if (auto const* fallback = _contract.fallbackFunction())
		if (fallback->isPayable() && !_contract.interfaceFunctionList().empty() && !_contract.receiveFunction())
			m_errorReporter.warning(
				_contract.location(),
				"This contract has a payable fallback function, but no receive ether function. Consider adding a receive ether function.",
				SecondarySourceLocation{}.append("The payable fallback function is defined here.", fallback->location())
			);
}
