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
#include <libsolutil/Visitor.h>

#include <boost/algorithm/string/predicate.hpp>


using namespace std;
using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::langutil;

using solidity::util::GenericVisitor;
using solidity::util::contains_if;
using solidity::util::joinHumanReadable;

namespace
{

// Helper struct to do a search by name
struct MatchByName
{
	string const& m_name;
	bool operator()(OverrideProxy const& _item)
	{
		return _item.name() == m_name;
	}
};

/**
 * Construct the override graph for this signature.
 * Reserve node 0 for the current contract and node
 * 1 for an artificial top node to which all override paths
 * connect at the end.
 */
struct OverrideGraph
{
	OverrideGraph(set<OverrideProxy> const& _baseCallables)
	{
		for (auto const& baseFunction: _baseCallables)
			addEdge(0, visit(baseFunction));
	}
	std::map<OverrideProxy, int> nodes;
	std::map<int, OverrideProxy> nodeInv;
	std::map<int, std::set<int>> edges;
	size_t numNodes = 2;
	void addEdge(int _a, int _b)
	{
		edges[_a].insert(_b);
		edges[_b].insert(_a);
	}
private:
	/// Completes the graph starting from @a _function and
	/// @returns the node ID.
	int visit(OverrideProxy const& _function)
	{
		auto it = nodes.find(_function);
		if (it != nodes.end())
			return it->second;
		int currentNode = static_cast<int>(numNodes++);
		nodes[_function] = currentNode;
		nodeInv[currentNode] = _function;
		if (_function.overrides())
			for (auto const& baseFunction: _function.baseFunctions())
				addEdge(currentNode, visit(baseFunction));
		else
			addEdge(currentNode, 1);

		return currentNode;
	}
};

/**
 * Detect cut vertices following https://en.wikipedia.org/wiki/Biconnected_component#Pseudocode
 * Can ignore the root node, since it is never a cut vertex in our case.
 */
struct CutVertexFinder
{
	CutVertexFinder(OverrideGraph const& _graph): m_graph(_graph)
	{
		run();
	}
	std::set<OverrideProxy> const& cutVertices() const { return m_cutVertices; }

private:
	OverrideGraph const& m_graph;

	std::vector<bool> m_visited = std::vector<bool>(m_graph.numNodes, false);
	std::vector<int> m_depths = std::vector<int>(m_graph.numNodes, -1);
	std::vector<int> m_low = std::vector<int>(m_graph.numNodes, -1);
	std::vector<int> m_parent = std::vector<int>(m_graph.numNodes, -1);
	std::set<OverrideProxy> m_cutVertices{};

	void run(size_t _u = 0, size_t _depth = 0)
	{
		m_visited.at(_u) = true;
		m_depths.at(_u) = m_low.at(_u) = static_cast<int>(_depth);
		for (int const v: m_graph.edges.at(static_cast<int>(_u)))
		{
			auto const vInd = static_cast<size_t>(v);
			if (!m_visited.at(vInd))
			{
				m_parent[vInd] = static_cast<int>(_u);
				run(vInd, _depth + 1);
				if (m_low[vInd] >= m_depths[_u] && m_parent[_u] != -1)
					m_cutVertices.insert(m_graph.nodeInv.at(static_cast<int>(_u)));
				m_low[_u] = min(m_low[_u], m_low[vInd]);
			}
			else if (v != m_parent[_u])
				m_low[_u] = min(m_low[_u], m_depths[vInd]);
		}
	}
};

vector<ContractDefinition const*> resolveDirectBaseContracts(ContractDefinition const& _contract)
{
	vector<ContractDefinition const*> resolvedContracts;

	for (ASTPointer<InheritanceSpecifier> const& specifier: _contract.baseContracts())
	{
		Declaration const* baseDecl =
			specifier->name().annotation().referencedDeclaration;
		auto contract = dynamic_cast<ContractDefinition const*>(baseDecl);
		if (contract)
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

OverrideProxy makeOverrideProxy(CallableDeclaration const& _callable)
{
	if (auto const* fun = dynamic_cast<FunctionDefinition const*>(&_callable))
		return OverrideProxy{fun};
	else if (auto const* mod = dynamic_cast<ModifierDefinition const*>(&_callable))
		return OverrideProxy{mod};
	else
		solAssert(false, "Invalid call to makeOverrideProxy.");
	return {};
}

}

bool OverrideProxy::operator<(OverrideProxy const& _other) const
{
	return id() < _other.id();
}

bool OverrideProxy::isVariable() const
{
	return holds_alternative<VariableDeclaration const*>(m_item);
}

bool OverrideProxy::isFunction() const
{
	return holds_alternative<FunctionDefinition const*>(m_item);
}

bool OverrideProxy::isModifier() const
{
	return holds_alternative<ModifierDefinition const*>(m_item);
}

bool OverrideProxy::CompareBySignature::operator()(OverrideProxy const& _a, OverrideProxy const& _b) const
{
	return _a.overrideComparator() < _b.overrideComparator();
}

size_t OverrideProxy::id() const
{
	return std::visit(GenericVisitor{
		[&](auto const* _item) -> size_t { return static_cast<size_t>(_item->id()); }
	}, m_item);
}

shared_ptr<OverrideSpecifier> OverrideProxy::overrides() const
{
	return std::visit(GenericVisitor{
		[&](auto const* _item) { return _item->overrides(); }
	}, m_item);
}

set<OverrideProxy> OverrideProxy::baseFunctions() const
{
	return std::visit(GenericVisitor{
		[&](auto const* _item) -> set<OverrideProxy> {
			set<OverrideProxy> ret;
			for (auto const* f: _item->annotation().baseFunctions)
				ret.insert(makeOverrideProxy(*f));
			return ret;
		}
	}, m_item);
}

void OverrideProxy::storeBaseFunction(OverrideProxy const& _base) const
{
	std::visit(GenericVisitor{
		[&](FunctionDefinition const* _item) {
			_item->annotation().baseFunctions.emplace(std::get<FunctionDefinition const*>(_base.m_item));
		},
		[&](ModifierDefinition const* _item) {
			_item->annotation().baseFunctions.emplace(std::get<ModifierDefinition const*>(_base.m_item));
		},
		[&](VariableDeclaration const* _item) {
			_item->annotation().baseFunctions.emplace(std::get<FunctionDefinition const*>(_base.m_item));
		}
	}, m_item);
}

string const& OverrideProxy::name() const
{
	return std::visit(GenericVisitor{
		[&](auto const* _item) -> string const& { return _item->name(); }
	}, m_item);
}

ContractDefinition const& OverrideProxy::contract() const
{
	return std::visit(GenericVisitor{
		[&](auto const* _item) -> ContractDefinition const& {
			return dynamic_cast<ContractDefinition const&>(*_item->scope());
		}
	}, m_item);
}

string const& OverrideProxy::contractName() const
{
	return contract().name();
}

Visibility OverrideProxy::visibility() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _item) { return _item->visibility(); },
		[&](ModifierDefinition const* _item) { return _item->visibility(); },
		[&](VariableDeclaration const*) { return Visibility::External; }
	}, m_item);
}

StateMutability OverrideProxy::stateMutability() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _item) { return _item->stateMutability(); },
		[&](ModifierDefinition const*) { solAssert(false, "Requested state mutability from modifier."); return StateMutability{}; },
		[&](VariableDeclaration const*) { return StateMutability::View; }
	}, m_item);
}

bool OverrideProxy::virtualSemantics() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _item) { return _item->virtualSemantics(); },
		[&](ModifierDefinition const* _item) { return _item->virtualSemantics(); },
		[&](VariableDeclaration const*) { return false; }
	}, m_item);
}

Token OverrideProxy::functionKind() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _item) { return _item->kind(); },
		[&](ModifierDefinition const*) { return Token::Function; },
		[&](VariableDeclaration const*) { return Token::Function; }
	}, m_item);
}

FunctionType const* OverrideProxy::functionType() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _item) { return FunctionType(*_item).asExternallyCallableFunction(false); },
		[&](VariableDeclaration const* _item) { return FunctionType(*_item).asExternallyCallableFunction(false); },
		[&](ModifierDefinition const*) -> FunctionType const* { solAssert(false, "Requested function type of modifier."); return nullptr; }
	}, m_item);
}

ModifierType const* OverrideProxy::modifierType() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const*) -> ModifierType const* { solAssert(false, "Requested modifier type of function."); return nullptr; },
		[&](VariableDeclaration const*) -> ModifierType const* { solAssert(false, "Requested modifier type of variable."); return nullptr; },
		[&](ModifierDefinition const* _modifier) -> ModifierType const* { return TypeProvider::modifier(*_modifier); }
	}, m_item);
}


Declaration const* OverrideProxy::declaration() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _function) -> Declaration const* { return _function; },
		[&](VariableDeclaration const* _variable) -> Declaration const* { return _variable; },
		[&](ModifierDefinition const* _modifier) -> Declaration const* { return _modifier; }
	}, m_item);
}

SourceLocation const& OverrideProxy::location() const
{
	return std::visit(GenericVisitor{
		[&](auto const* _item) -> SourceLocation const& { return _item->location(); }
	}, m_item);
}

string OverrideProxy::astNodeName() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const*) { return "function"; },
		[&](ModifierDefinition const*) { return "modifier"; },
		[&](VariableDeclaration const*) { return "public state variable"; },
	}, m_item);
}

string OverrideProxy::astNodeNameCapitalized() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const*) { return "Function"; },
		[&](ModifierDefinition const*) { return "Modifier"; },
		[&](VariableDeclaration const*) { return "Public state variable"; },
	}, m_item);
}

string OverrideProxy::distinguishingProperty() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const*) { return "name and parameter types"; },
		[&](ModifierDefinition const*) { return "name"; },
		[&](VariableDeclaration const*) { return "name and parameter types"; },
	}, m_item);
}

bool OverrideProxy::unimplemented() const
{
	return std::visit(GenericVisitor{
		[&](FunctionDefinition const* _item) { return !_item->isImplemented(); },
		[&](ModifierDefinition const* _item) { return !_item->isImplemented(); },
		[&](VariableDeclaration const*) { return false; }
	}, m_item);
}

bool OverrideProxy::OverrideComparator::operator<(OverrideComparator const& _other) const
{
	if (name != _other.name)
		return name < _other.name;

	if (!functionKind || !_other.functionKind)
		return false;

	if (functionKind != _other.functionKind)
		return *functionKind < *_other.functionKind;

	if (!parameterTypes || !_other.parameterTypes)
		return false;

	return boost::lexicographical_compare(*parameterTypes, *_other.parameterTypes);
}

OverrideProxy::OverrideComparator const& OverrideProxy::overrideComparator() const
{
	if (!m_comparator)
	{
		m_comparator = make_shared<OverrideComparator>(std::visit(GenericVisitor{
			[&](FunctionDefinition const* _function)
			{
				vector<string> paramTypes;
				for (Type const* t: functionType()->parameterTypes())
					paramTypes.emplace_back(t->richIdentifier());
				return OverrideComparator{
					_function->name(),
					_function->kind(),
					std::move(paramTypes)
				};
			},
			[&](VariableDeclaration const* _var)
			{
				vector<string> paramTypes;
				for (Type const* t: functionType()->parameterTypes())
					paramTypes.emplace_back(t->richIdentifier());
				return OverrideComparator{
					_var->name(),
					Token::Function,
					std::move(paramTypes)
				};
			},
			[&](ModifierDefinition const* _mod)
			{
				return OverrideComparator{
					_mod->name(),
					{},
					{}
				};
			}
		}, m_item));
	}

	return *m_comparator;
}

bool OverrideChecker::CompareByID::operator()(ContractDefinition const* _a, ContractDefinition const* _b) const
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
	OverrideProxyBySignatureMultiSet const& inheritedFuncs = inheritedFunctions(_contract);
	OverrideProxyBySignatureMultiSet const& inheritedMods = inheritedModifiers(_contract);

	for (ModifierDefinition const* modifier: _contract.functionModifiers())
	{
		if (contains_if(inheritedFuncs, MatchByName{modifier->name()}))
			m_errorReporter.typeError(
				5631_error,
				modifier->location(),
				"Override changes function or public state variable to modifier."
			);

		checkOverrideList(OverrideProxy{modifier}, inheritedMods);
	}

	for (FunctionDefinition const* function: _contract.definedFunctions())
	{
		if (function->isConstructor())
			continue;

		if (contains_if(inheritedMods, MatchByName{function->name()}))
			m_errorReporter.typeError(1469_error, function->location(), "Override changes modifier to function.");

		checkOverrideList(OverrideProxy{function}, inheritedFuncs);
	}
	for (auto const* stateVar: _contract.stateVariables())
	{
		if (!stateVar->isPublic())
		{
			if (stateVar->overrides())
				m_errorReporter.typeError(8022_error, stateVar->location(), "Override can only be used with public state variables.");

			continue;
		}

		if (contains_if(inheritedMods, MatchByName{stateVar->name()}))
			m_errorReporter.typeError(1456_error, stateVar->location(), "Override changes modifier to public state variable.");

		checkOverrideList(OverrideProxy{stateVar}, inheritedFuncs);
	}

}

void OverrideChecker::checkOverride(OverrideProxy const& _overriding, OverrideProxy const& _super)
{
	solAssert(_super.isModifier() == _overriding.isModifier(), "");

	if (_super.isFunction() || _super.isModifier())
		_overriding.storeBaseFunction(_super);

	if (_overriding.isModifier() && *_overriding.modifierType() != *_super.modifierType())
		m_errorReporter.typeError(
			1078_error,
			_overriding.location(),
			"Override changes modifier signature."
		);

	if (!_overriding.overrides())
		overrideError(_overriding, _super, 9456_error, "Overriding " + _overriding.astNodeName() + " is missing \"override\" specifier.");

	if (_super.isVariable())
		overrideError(
			_super,
			_overriding,
			1452_error,
			"Cannot override public state variable.",
			"Overriding " + _overriding.astNodeName() + " is here:"
		);
	else if (!_super.virtualSemantics())
		overrideError(
			_super,
			_overriding,
			4334_error,
			"Trying to override non-virtual " + _super.astNodeName() + ". Did you forget to add \"virtual\"?",
			"Overriding " + _overriding.astNodeName() + " is here:"
		);

	if (_overriding.isVariable())
	{
		if (_super.visibility() != Visibility::External)
			overrideError(_overriding, _super, 5225_error, "Public state variables can only override functions with external visibility.");
		solAssert(_overriding.visibility() == Visibility::External, "");
	}
	else if (_overriding.visibility() != _super.visibility())
	{
		// Visibility change from external to public is fine.
		// Any other change is disallowed.
		if (!(
			_super.visibility() == Visibility::External &&
			_overriding.visibility() == Visibility::Public
		))
			overrideError(_overriding, _super, 9098_error, "Overriding " + _overriding.astNodeName() + " visibility differs.");
	}

	if (_super.isFunction())
	{
		FunctionType const* functionType = _overriding.functionType();
		FunctionType const* superType = _super.functionType();

		solAssert(functionType->hasEqualParameterTypes(*superType), "Override doesn't have equal parameters!");

		if (!functionType->hasEqualReturnTypes(*superType))
			overrideError(_overriding, _super, 4822_error, "Overriding " + _overriding.astNodeName() + " return types differ.");

		// This is only relevant for a function overriding a function.
		if (_overriding.isFunction())
		{
			if (_overriding.stateMutability() != _super.stateMutability())
				overrideError(
					_overriding,
					_super,
					6959_error,
					"Overriding function changes state mutability from \"" +
					stateMutabilityToString(_super.stateMutability()) +
					"\" to \"" +
					stateMutabilityToString(_overriding.stateMutability()) +
					"\"."
				);

			if (_overriding.unimplemented() && !_super.unimplemented())
				overrideError(
					_overriding,
					_super,
					4593_error,
					"Overriding an implemented function with an unimplemented function is not allowed."
				);
		}
	}
}

void OverrideChecker::overrideListError(
	OverrideProxy const& _item,
	set<ContractDefinition const*, CompareByID> _secondary,
	ErrorId _error,
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
		names.insert("\"" + c->name() + "\"");
	}
	string contractSingularPlural = "contract ";
	if (_secondary.size() > 1)
		contractSingularPlural = "contracts ";

	m_errorReporter.typeError(
		_error,
		_item.overrides() ? _item.overrides()->location() : _item.location(),
		ssl,
		_message1 +
		contractSingularPlural +
		_message2 +
		joinHumanReadable(names, ", ", " and ") +
		"."
	);
}

void OverrideChecker::overrideError(Declaration const& _overriding, Declaration const& _super, ErrorId _error, string const& _message, string const& _secondaryMsg)
{
	m_errorReporter.typeError(
		_error,
		_overriding.location(),
		SecondarySourceLocation().append(_secondaryMsg, _super.location()),
		_message
	);
}


void OverrideChecker::overrideError(OverrideProxy const& _overriding, OverrideProxy const& _super, ErrorId _error, string const& _message, string const& _secondaryMsg)
{
	m_errorReporter.typeError(
		_error,
		_overriding.location(),
		SecondarySourceLocation().append(_secondaryMsg, _super.location()),
		_message
	);
}

void OverrideChecker::checkAmbiguousOverrides(ContractDefinition const& _contract) const
{
	{
		// Fetch inherited functions and sort them by signature.
		// We get at least one function per signature and direct base contract, which is
		// enough because we re-construct the inheritance graph later.
		OverrideProxyBySignatureMultiSet nonOverriddenFunctions = inheritedFunctions(_contract);

		// Remove all functions that match the signature of a function in the current contract.
		for (FunctionDefinition const* f: _contract.definedFunctions())
			nonOverriddenFunctions.erase(OverrideProxy{f});
		for (VariableDeclaration const* v: _contract.stateVariables())
			if (v->isPublic())
				nonOverriddenFunctions.erase(OverrideProxy{v});

		// Walk through the set of functions signature by signature.
		for (auto it = nonOverriddenFunctions.cbegin(); it != nonOverriddenFunctions.cend();)
		{
			std::set<OverrideProxy> baseFunctions;
			for (auto nextSignature = nonOverriddenFunctions.upper_bound(*it); it != nextSignature; ++it)
				baseFunctions.insert(*it);

			checkAmbiguousOverridesInternal(std::move(baseFunctions), _contract.location());
		}
	}

	{
		OverrideProxyBySignatureMultiSet modifiers = inheritedModifiers(_contract);
		for (ModifierDefinition const* mod: _contract.functionModifiers())
			modifiers.erase(OverrideProxy{mod});

		for (auto it = modifiers.cbegin(); it != modifiers.cend();)
		{
			std::set<OverrideProxy> baseModifiers;
			for (auto next = modifiers.upper_bound(*it); it != next; ++it)
				baseModifiers.insert(*it);

			checkAmbiguousOverridesInternal(std::move(baseModifiers), _contract.location());
		}

	}
}

void OverrideChecker::checkAmbiguousOverridesInternal(set<OverrideProxy> _baseCallables, SourceLocation const& _location) const
{
	if (_baseCallables.size() <= 1)
		return;

	OverrideGraph overrideGraph(_baseCallables);
	CutVertexFinder cutVertexFinder{overrideGraph};

	// Remove all base functions overridden by cut vertices (they don't need to be overridden).
	for (OverrideProxy const& function: cutVertexFinder.cutVertices())
	{
		std::set<OverrideProxy> toTraverse = function.baseFunctions();
		while (!toTraverse.empty())
		{
			OverrideProxy base = *toTraverse.begin();
			toTraverse.erase(toTraverse.begin());
			_baseCallables.erase(base);
			for (OverrideProxy const& f: base.baseFunctions())
				toTraverse.insert(f);
		}
		// Remove unimplemented base functions at the cut vertices itself as well.
		if (function.unimplemented())
			_baseCallables.erase(function);
	}

	// If more than one function is left, they have to be overridden.
	if (_baseCallables.size() <= 1)
		return;

	SecondarySourceLocation ssl;
	for (OverrideProxy const& baseFunction: _baseCallables)
		ssl.append("Definition in \"" + baseFunction.contractName() + "\": ", baseFunction.location());

	string callableName = _baseCallables.begin()->astNodeName();
	if (_baseCallables.begin()->isVariable())
		callableName = "function";
	string distinguishigProperty = _baseCallables.begin()->distinguishingProperty();

	bool foundVariable = false;
	for (auto const& base: _baseCallables)
		if (base.isVariable())
			foundVariable = true;

	string message =
		"Derived contract must override " + callableName + " \"" +
		_baseCallables.begin()->name() +
		"\". Two or more base classes define " + callableName + " with same " + distinguishigProperty + ".";

	if (foundVariable)
		message +=
			" Since one of the bases defines a public state variable which cannot be overridden, "
			"you have to change the inheritance layout or the names of the functions.";

	m_errorReporter.typeError(6480_error, _location, ssl, message);
}

set<ContractDefinition const*, OverrideChecker::CompareByID> OverrideChecker::resolveOverrideList(OverrideSpecifier const& _overrides) const
{
	set<ContractDefinition const*, CompareByID> resolved;

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

void OverrideChecker::checkOverrideList(OverrideProxy _item, OverrideProxyBySignatureMultiSet const& _inherited)
{
	set<ContractDefinition const*, CompareByID> specifiedContracts =
		_item.overrides() ?
		resolveOverrideList(*_item.overrides()) :
		decltype(specifiedContracts){};

	// Check for duplicates in override list
	if (_item.overrides() && specifiedContracts.size() != _item.overrides()->overrides().size())
	{
		// Sort by contract id to find duplicate for error reporting
		vector<ASTPointer<UserDefinedTypeName>> list =
			sortByContract(_item.overrides()->overrides());

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
					4520_error,
					list[i]->location(),
					ssl,
					"Duplicate contract \"" +
					joinHumanReadable(list[i]->namePath(), ".") +
					"\" found in override list of \"" +
					_item.name() +
					"\"."
				);
			}
		}
	}

	set<ContractDefinition const*, CompareByID> expectedContracts;

	// Build list of expected contracts
	for (auto [begin, end] = _inherited.equal_range(_item); begin != end; begin++)
	{
		// Validate the override
		checkOverride(_item, *begin);

		expectedContracts.insert(&begin->contract());
	}

	if (_item.overrides() && expectedContracts.empty())
		m_errorReporter.typeError(
			7792_error,
			_item.overrides()->location(),
			_item.astNodeNameCapitalized() + " has override specified but does not override anything."
		);

	set<ContractDefinition const*, CompareByID> missingContracts;
	// If we expect only one contract, no contract needs to be specified
	if (expectedContracts.size() > 1)
		missingContracts = expectedContracts - specifiedContracts;

	if (!missingContracts.empty())
		overrideListError(
			_item,
			missingContracts,
			4327_error,
			_item.astNodeNameCapitalized() + " needs to specify overridden ",
			""
		);

	auto surplusContracts = specifiedContracts - expectedContracts;
	if (!surplusContracts.empty())
		overrideListError(
			_item,
			surplusContracts,
			2353_error,
			"Invalid ",
			"specified in override list: "
		);
}

OverrideChecker::OverrideProxyBySignatureMultiSet const& OverrideChecker::inheritedFunctions(ContractDefinition const& _contract) const
{
	if (!m_inheritedFunctions.count(&_contract))
	{
		OverrideProxyBySignatureMultiSet result;

		for (auto const* base: resolveDirectBaseContracts(_contract))
		{
			set<OverrideProxy, OverrideProxy::CompareBySignature> functionsInBase;
			for (FunctionDefinition const* fun: base->definedFunctions())
				if (!fun->isConstructor())
					functionsInBase.emplace(OverrideProxy{fun});
			for (VariableDeclaration const* var: base->stateVariables())
				if (var->isPublic())
					functionsInBase.emplace(OverrideProxy{var});

			for (OverrideProxy const& func: inheritedFunctions(*base))
				functionsInBase.insert(func);

			result += functionsInBase;
		}

		m_inheritedFunctions[&_contract] = result;
	}

	return m_inheritedFunctions[&_contract];
}

OverrideChecker::OverrideProxyBySignatureMultiSet const& OverrideChecker::inheritedModifiers(ContractDefinition const& _contract) const
{
	if (!m_inheritedModifiers.count(&_contract))
	{
		OverrideProxyBySignatureMultiSet result;

		for (auto const* base: resolveDirectBaseContracts(_contract))
		{
			set<OverrideProxy, OverrideProxy::CompareBySignature> modifiersInBase;
			for (ModifierDefinition const* mod: base->functionModifiers())
				modifiersInBase.emplace(OverrideProxy{mod});

			for (OverrideProxy const& mod: inheritedModifiers(*base))
				modifiersInBase.insert(mod);

			result += modifiersInBase;
		}

		m_inheritedModifiers[&_contract] = result;
	}

	return m_inheritedModifiers[&_contract];
}
