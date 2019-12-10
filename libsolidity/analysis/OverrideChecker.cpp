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

#include <libsolidity/analysis/OverrideChecker.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/TypeProvider.h>
#include <libsolidity/analysis/TypeChecker.h>
#include <liblangutil/ErrorReporter.h>
#include <libdevcore/Visitor.h>
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


}

bool OverrideProxy::operator<(OverrideProxy const& _other) const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _function)
		{
			return std::visit(GenericVisitor{
				[&](FunctionDefinition const* _other)
				{
					if (_function->name() != _other->name())
						return _function->name() < _other->name();

					if (_function->kind() != _other->kind())
						return _function->kind() < _other->kind();

					return boost::lexicographical_compare(
						FunctionType(*_function).asCallableFunction(false)->parameterTypes(),
						FunctionType(*_other).asCallableFunction(false)->parameterTypes(),
						[](auto const& _paramTypeA, auto const& _paramTypeB)
						{
							return _paramTypeA->richIdentifier() < _paramTypeB->richIdentifier();
						}
					);
				},
				[&](VariableDeclaration const* _other)
				{
					if (_function->name() != _other->name())
						return _function->name() < _other->name();

					if (_function->kind() != Token::Function)
						return _function->kind() < Token::Function;

					return boost::lexicographical_compare(
						FunctionType(*_function).asCallableFunction(false)->parameterTypes(),
						FunctionType(*_other).asCallableFunction(false)->parameterTypes(),
						[](auto const& _paramTypeA, auto const& _paramTypeB)
						{
							return _paramTypeA->richIdentifier() < _paramTypeB->richIdentifier();
						}
					);
				},
				[&](ModifierDefinition const*)
				{
					solAssert(false, "Compared function to something else than function or state variable.");
					return false;
				}
			}, _other.item);
		},
		[&](ModifierDefinition const*)
		{
			solAssert(false, "todo");
			return false;
		},
		[&](VariableDeclaration const*)
		{
			solAssert(false, "todo");
			return false;
		}
	}, item);

	return false;
}


bool OverrideChecker::LessFunction::operator()(ModifierDefinition const* _a, ModifierDefinition const* _b) const
{
	return _a->name() < _b->name();
}

bool OverrideChecker::LessFunction::operator()(FunctionDefinition const* _a, FunctionDefinition const* _b) const
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

bool OverrideChecker::LessFunction::operator()(ContractDefinition const* _a, ContractDefinition const* _b) const
{
	if (!_a || !_b)
		return _a < _b;

	return _a->id() < _b->id();
}

void OverrideChecker::check(ContractDefinition const& _contract)
{
	checkIllegalOverrides(_contract);
	checkAmbiguousOverrides(_contract);
}

void OverrideChecker::checkIllegalOverrides(ContractDefinition const& _contract)
{
	FunctionMultiSet const& inheritedFuncs = inheritedFunctions(_contract);
	ModifierMultiSet const& inheritedMods = inheritedModifiers(_contract);

	for (auto const* stateVar: _contract.stateVariables())
	{
		if (!stateVar->isPublic())
			continue;

		bool found = false;
		for (
			auto it = find_if(inheritedFuncs.begin(), inheritedFuncs.end(), MatchByName{stateVar->name()});
			it != inheritedFuncs.end();
			it = find_if(++it, inheritedFuncs.end(), MatchByName{stateVar->name()})
		)
		{
			// -> compare equal
			if (!hasEqualNameAndParameters(*stateVar, **it))
				continue;

			if ((*it)->visibility() != Visibility::External)
				overrideError(*stateVar, **it, "Public state variables can only override functions with external visibility.");
			else
				checkOverride(*stateVar, **it);

			found = true;
		}

		if (!found && stateVar->overrides())
			m_errorReporter.typeError(
				stateVar->overrides()->location(),
				"Public state variable has override specified but does not override anything."
			);
	}

	for (ModifierDefinition const* modifier: _contract.functionModifiers())
	{
		if (contains_if(inheritedFuncs, MatchByName{modifier->name()}))
			m_errorReporter.typeError(
				modifier->location(),
				"Override changes function to modifier."
			);

		auto [begin, end] = inheritedMods.equal_range(modifier);

		if (begin == end && modifier->overrides())
			m_errorReporter.typeError(
				modifier->overrides()->location(),
				"Modifier has override specified but does not override anything."
			);

		for (; begin != end; begin++)
			if (ModifierType(**begin) != ModifierType(*modifier))
				m_errorReporter.typeError(
					modifier->location(),
					"Override changes modifier signature."
				);

		checkOverrideList(inheritedMods, *modifier);
	}

	for (FunctionDefinition const* function: _contract.definedFunctions())
	{
		if (function->isConstructor())
			continue;

		if (contains_if(inheritedMods, MatchByName{function->name()}))
			m_errorReporter.typeError(function->location(), "Override changes modifier to function.");

		// No inheriting functions found
		if (!inheritedFuncs.count(function) && function->overrides())
			m_errorReporter.typeError(
				function->overrides()->location(),
				"Function has override specified but does not override anything."
			);

		checkOverrideList(inheritedFuncs, *function);
	}
}

template<class T, class U>
void OverrideChecker::checkOverride(T const& _overriding, U const& _super)
{
	static_assert(
		std::is_same<VariableDeclaration, T>::value ||
		std::is_same<FunctionDefinition, T>::value ||
		std::is_same<ModifierDefinition, T>::value,
		"Invalid call to checkOverride."
	);

	static_assert(
		std::is_same<FunctionDefinition, U>::value ||
		std::is_same<ModifierDefinition, U>::value,
		"Invalid call to checkOverride."
	);
	static_assert(
		!std::is_same<ModifierDefinition, U>::value ||
		std::is_same<ModifierDefinition, T>::value,
		"Invalid call to checkOverride."
	);

	string overridingName;
	if constexpr(std::is_same<FunctionDefinition, T>::value)
		overridingName = "function";
	else if constexpr(std::is_same<ModifierDefinition, T>::value)
		overridingName = "modifier";
	else
		overridingName = "public state variable";

	string superName;
	if constexpr(std::is_same<FunctionDefinition, U>::value)
		superName = "function";
	else
		superName = "modifier";

	if (!_overriding.overrides())
		overrideError(_overriding, _super, "Overriding " + overridingName + " is missing 'override' specifier.");

	if (!_super.virtualSemantics())
		overrideError(
			_super,
			_overriding,
			"Trying to override non-virtual " + superName + ". Did you forget to add \"virtual\"?",
			"Overriding " + overridingName + " is here:"
		);

	if (_overriding.visibility() != _super.visibility())
	{
		// Visibility change from external to public is fine.
		// Any other change is disallowed.
		if (!(
			_super.visibility() == Visibility::External &&
			_overriding.visibility() == Visibility::Public
		))
			overrideError(_overriding, _super, "Overriding " + overridingName + " visibility differs.");
	}

	// This is only relevant for overriding functions by functions or state variables,
	// it is skipped for modifiers.
	if constexpr(std::is_same<FunctionDefinition, U>::value)
	{
		FunctionTypePointer functionType = FunctionType(_overriding).asCallableFunction(false);
		FunctionTypePointer superType = FunctionType(_super).asCallableFunction(false);

		solAssert(functionType->hasEqualParameterTypes(*superType), "Override doesn't have equal parameters!");

		if (!functionType->hasEqualReturnTypes(*superType))
			overrideError(_overriding, _super, "Overriding " + overridingName + " return types differ.");

		// This is only relevant for a function overriding a function.
		if constexpr(std::is_same<T, FunctionDefinition>::value)
		{
			_overriding.annotation().baseFunctions.emplace(&_super);

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
}

void OverrideChecker::overrideListError(
	CallableDeclaration const& _callable,
	set<ContractDefinition const*, LessFunction> _secondary,
	string const& _message1,
	string const& _message2
)
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
		_callable.overrides() ? _callable.overrides()->location() : _callable.location(),
		ssl,
		_message1 +
		contractSingularPlural +
		_message2 +
		joinHumanReadable(names, ", ", " and ") +
		"."
	);
}

void OverrideChecker::overrideError(Declaration const& _overriding, Declaration const& _super, string _message, string _secondaryMsg)
{
	m_errorReporter.typeError(
		_overriding.location(),
		SecondarySourceLocation().append(_secondaryMsg, _super.location()),
		_message
	);
}

void OverrideChecker::checkAmbiguousOverrides(ContractDefinition const& _contract) const
{
	std::function<bool(CallableDeclaration const*, CallableDeclaration const*)> compareById =
		[](auto const* _a, auto const* _b) { return _a->id() < _b->id(); };

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
			std::set<CallableDeclaration const*, decltype(compareById)> baseFunctions(compareById);
			for (auto nextSignature = nonOverriddenFunctions.upper_bound(*it); it != nextSignature; ++it)
				baseFunctions.insert(*it);

			checkAmbiguousOverridesInternal(std::move(baseFunctions), _contract.location());
		}
	}

	{
		ModifierMultiSet modifiers = inheritedModifiers(_contract);
		modifiers -= _contract.functionModifiers();
		for (auto it = modifiers.cbegin(); it != modifiers.cend();)
		{
			std::set<CallableDeclaration const*, decltype(compareById)> baseModifiers(compareById);
			for (auto next = modifiers.upper_bound(*it); it != next; ++it)
				baseModifiers.insert(*it);

			checkAmbiguousOverridesInternal(std::move(baseModifiers), _contract.location());
		}

	}
}

void OverrideChecker::checkAmbiguousOverridesInternal(set<
	CallableDeclaration const*,
	std::function<bool(CallableDeclaration const*, CallableDeclaration const*)>
> _baseCallables, SourceLocation const& _location) const
{
	if (_baseCallables.size() <= 1)
		return;

	// Construct the override graph for this signature.
	// Reserve node 0 for the current contract and node
	// 1 for an artificial top node to which all override paths
	// connect at the end.
	struct OverrideGraph
	{
		OverrideGraph(decltype(_baseCallables) const& __baseCallables)
		{
			for (auto const* baseFunction: __baseCallables)
				addEdge(0, visit(baseFunction));
		}
		std::map<CallableDeclaration const*, int> nodes;
		std::map<int, CallableDeclaration const*> nodeInv;
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
		int visit(CallableDeclaration const* _function)
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
	} overrideGraph(_baseCallables);

	// Detect cut vertices following https://en.wikipedia.org/wiki/Biconnected_component#Pseudocode
	// Can ignore the root node, since it is never a cut vertex in our case.
	struct CutVertexFinder
	{
		CutVertexFinder(OverrideGraph const& _graph): m_graph(_graph)
		{
			run();
		}
		std::set<CallableDeclaration const*> const& cutVertices() const { return m_cutVertices; }

	private:
		OverrideGraph const& m_graph;

		std::vector<bool> m_visited = std::vector<bool>(m_graph.numNodes, false);
		std::vector<int> m_depths = std::vector<int>(m_graph.numNodes, -1);
		std::vector<int> m_low = std::vector<int>(m_graph.numNodes, -1);
		std::vector<int> m_parent = std::vector<int>(m_graph.numNodes, -1);
		std::set<CallableDeclaration const*> m_cutVertices{};

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
		std::set<CallableDeclaration const*> toTraverse = function->annotation().baseFunctions;
		while (!toTraverse.empty())
		{
			auto const *base = *toTraverse.begin();
			toTraverse.erase(toTraverse.begin());
			_baseCallables.erase(base);
			for (CallableDeclaration const* f: base->annotation().baseFunctions)
				toTraverse.insert(f);
		}
		// Remove unimplemented base functions at the cut vertices itself as well.
		if (auto opt = dynamic_cast<ImplementationOptional const*>(function))
			if (!opt->isImplemented())
				_baseCallables.erase(function);
	}

	// If more than one function is left, they have to be overridden.
	if (_baseCallables.size() <= 1)
		return;

	SecondarySourceLocation ssl;
	for (auto const* baseFunction: _baseCallables)
	{
		string contractName = dynamic_cast<ContractDefinition const&>(*baseFunction->scope()).name();
		ssl.append("Definition in \"" + contractName + "\": ", baseFunction->location());
	}

	string callableName;
	string distinguishigProperty;
	if (dynamic_cast<FunctionDefinition const*>(*_baseCallables.begin()))
	{
		callableName = "function";
		distinguishigProperty = "name and parameter types";
	}
	else if (dynamic_cast<ModifierDefinition const*>(*_baseCallables.begin()))
	{
		callableName = "modifier";
		distinguishigProperty = "name";
	}
	else
		solAssert(false, "Invalid type for ambiguous override.");

	m_errorReporter.typeError(
		_location,
		ssl,
		"Derived contract must override " + callableName + " \"" +
		(*_baseCallables.begin())->name() +
		"\". Two or more base classes define " + callableName + " with same " + distinguishigProperty + "."
	);
}

set<ContractDefinition const*, OverrideChecker::LessFunction> OverrideChecker::resolveOverrideList(OverrideSpecifier const& _overrides) const
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

template <class T>
void OverrideChecker::checkOverrideList(
	std::multiset<T const*, LessFunction> const& _inheritedCallables,
	T const& _callable
)
{
	set<ContractDefinition const*, LessFunction> specifiedContracts =
		_callable.overrides() ?
		resolveOverrideList(*_callable.overrides()) :
		decltype(specifiedContracts){};

	// Check for duplicates in override list
	if (_callable.overrides() && specifiedContracts.size() != _callable.overrides()->overrides().size())
	{
		// Sort by contract id to find duplicate for error reporting
		vector<ASTPointer<UserDefinedTypeName>> list =
			sortByContract(_callable.overrides()->overrides());

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
						_callable.name() +
						"\"."
				);
			}
		}
	}

	decltype(specifiedContracts) expectedContracts;

	// Build list of expected contracts
	for (auto [begin, end] = _inheritedCallables.equal_range(&_callable); begin != end; begin++)
	{
		// Validate the override
		checkOverride(_callable, **begin);

		expectedContracts.insert(&dynamic_cast<ContractDefinition const&>(*(*begin)->scope()));
	}

	decltype(specifiedContracts) missingContracts;
	decltype(specifiedContracts) surplusContracts;

	// If we expect only one contract, no contract needs to be specified
	if (expectedContracts.size() > 1)
		missingContracts = expectedContracts - specifiedContracts;

	surplusContracts = specifiedContracts - expectedContracts;

	if (!missingContracts.empty())
		overrideListError(
			_callable,
			missingContracts,
			"Function needs to specify overridden ",
			""
		);

	if (!surplusContracts.empty())
		overrideListError(
			_callable,
			surplusContracts,
			"Invalid ",
			"specified in override list: "
		);
}

OverrideChecker::FunctionMultiSet const& OverrideChecker::inheritedFunctions(ContractDefinition const& _contract) const
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

OverrideChecker::ModifierMultiSet const& OverrideChecker::inheritedModifiers(ContractDefinition const& _contract) const
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


multiset<OverrideProxy> const& OverrideChecker::inheritedFunctionsByProxy(ContractDefinition const& _contract) const
{
	if (!m_inheritedFunctionsByProxy.count(&_contract))
	{
		multiset<OverrideProxy> result;

		for (auto const* base: resolveDirectBaseContracts(_contract))
		{
			std::set<OverrideProxy> functionsInBase;
			for (FunctionDefinition const* fun: base->definedFunctions())
				if (!fun->isConstructor())
					functionsInBase.emplace(OverrideProxy{fun});

			for (OverrideProxy const& func: inheritedFunctionsByProxy(*base))
				functionsInBase.insert(func);

			result += functionsInBase;
		}

		m_inheritedFunctionsByProxy[&_contract] = result;
	}

	return m_inheritedFunctionsByProxy[&_contract];
}

multiset<OverrideProxy> const& OverrideChecker::inheritedModifiersByProxy(ContractDefinition const& _contract) const
{
	if (!m_inheritedModifiersByProxy.count(&_contract))
	{
		multiset<OverrideProxy> result;

		for (auto const* base: resolveDirectBaseContracts(_contract))
		{
			std::set<OverrideProxy> modifiersInBase;
			for (ModifierDefinition const* mod: base->functionModifiers())
				modifiersInBase.emplace(OverrideProxy{mod});

			for (OverrideProxy const& mod: inheritedModifiersByProxy(*base))
				modifiersInBase.insert(mod);

			result += modifiersInBase;
		}

		m_inheritedModifiersByProxy[&_contract] = result;
	}

	return m_inheritedModifiersByProxy[&_contract];
}
