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

#pragma once

#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTEnums.h>
#include <libsolidity/parsing/Token.h>

#include <liblangutil/SourceLocation.h>
#include <libevmasm/Instruction.h>
#include <libsolutil/FixedHash.h>
#include <libsolutil/LazyInit.h>
#include <libsolutil/Visitor.h>

#include <json/json.h>

#include <range/v3/view/subrange.hpp>
#include <range/v3/view/map.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace solidity::yul
{
// Forward-declaration to <yul/AST.h>
struct Block;
struct Dialect;
}

namespace solidity::frontend
{

class ASTVisitor;
class ASTConstVisitor;


/**
 * The root (abstract) class of the AST inheritance tree.
 * It is possible to traverse all direct and indirect children of an AST node by calling
 * accept, providing an ASTVisitor.
 */
class ASTNode
{
public:
	/// Noncopyable.
	ASTNode(ASTNode const&) = delete;
	ASTNode& operator=(ASTNode const&) = delete;

	using CompareByID = frontend::ASTCompareByID<ASTNode>;
	using SourceLocation = langutil::SourceLocation;

	explicit ASTNode(int64_t _id, SourceLocation _location);
	virtual ~ASTNode() {}

	/// @returns an identifier of this AST node that is unique for a single compilation run.
	int64_t id() const { return int64_t(m_id); }

	virtual void accept(ASTVisitor& _visitor) = 0;
	virtual void accept(ASTConstVisitor& _visitor) const = 0;
	template <class T>
	static void listAccept(std::vector<T> const& _list, ASTVisitor& _visitor)
	{
		for (T const& element: _list)
			if (element)
				element->accept(_visitor);
	}
	template <class T>
	static void listAccept(std::vector<T> const& _list, ASTConstVisitor& _visitor)
	{
		for (T const& element: _list)
			if (element)
				element->accept(_visitor);
	}

	/// @returns a copy of the vector containing only the nodes which derive from T.
	template <class T>
	static std::vector<T const*> filteredNodes(std::vector<ASTPointer<ASTNode>> const& _nodes);

	/// Extracts the referenced declaration from all nodes whose annotations support
	/// `referencedDeclaration`.
	static Declaration const* referencedDeclaration(Expression const& _expression);
	/// Performs potential super or virtual lookup for a function call based on the most derived contract.
	static FunctionDefinition const* resolveFunctionCall(FunctionCall const& _functionCall, ContractDefinition const* _mostDerivedContract);

	/// Returns the source code location of this node.
	SourceLocation const& location() const { return m_location; }

	///@todo make this const-safe by providing a different way to access the annotation
	virtual ASTAnnotation& annotation() const;

	///@{
	///@name equality operators
	/// Equality relies on the fact that nodes cannot be copied.
	bool operator==(ASTNode const& _other) const { return this == &_other; }
	bool operator!=(ASTNode const& _other) const { return !operator==(_other); }
	///@}

protected:
	size_t const m_id = 0;

	template <class T>
	T& initAnnotation() const
	{
		if (!m_annotation)
			m_annotation = std::make_unique<T>();
		return dynamic_cast<T&>(*m_annotation);
	}

private:
	/// Annotation - is specialised in derived classes, is created upon request (because of polymorphism).
	mutable std::unique_ptr<ASTAnnotation> m_annotation;
	SourceLocation m_location;
};

template <class T>
std::vector<T const*> ASTNode::filteredNodes(std::vector<ASTPointer<ASTNode>> const& _nodes)
{
	std::vector<T const*> ret;
	for (auto const& n: _nodes)
		if (auto const* nt = dynamic_cast<T const*>(n.get()))
			ret.push_back(nt);
	return ret;
}

/**
 * Abstract marker class that specifies that this AST node opens a scope.
 */
class ScopeOpener
{
public:
	virtual ~ScopeOpener() = default;
};

/**
 * Source unit containing import directives and contract definitions.
 */
class SourceUnit: public ASTNode, public ScopeOpener
{
public:
	SourceUnit(
		int64_t _id,
		SourceLocation const& _location,
		std::optional<std::string> _licenseString,
		std::vector<ASTPointer<ASTNode>> _nodes
	):
		ASTNode(_id, _location), m_licenseString(std::move(_licenseString)), m_nodes(std::move(_nodes)) {}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;
	SourceUnitAnnotation& annotation() const override;

	std::optional<std::string> const& licenseString() const { return m_licenseString; }
	std::vector<ASTPointer<ASTNode>> nodes() const { return m_nodes; }

	/// @returns a set of referenced SourceUnits. Recursively if @a _recurse is true.
	std::set<SourceUnit const*> referencedSourceUnits(bool _recurse = false, std::set<SourceUnit const*> _skipList = std::set<SourceUnit const*>()) const;

private:
	std::optional<std::string> m_licenseString;
	std::vector<ASTPointer<ASTNode>> m_nodes;
};

/**
 * Abstract class that is added to each AST node that is stored inside a scope
 * (including scopes).
 */
class Scopable
{
public:
	virtual ~Scopable() = default;
	/// @returns the scope this declaration resides in. Can be nullptr if it is the global scope.
	/// Available only after name and type resolution step.
	ASTNode const* scope() const { return annotation().scope; }

	/// @returns the source unit this scopable is present in.
	SourceUnit const& sourceUnit() const;

	/// @returns the function or modifier definition this scopable is present in or nullptr.
	CallableDeclaration const* functionOrModifierDefinition() const;

	/// @returns the source name this scopable is present in.
	/// Can be combined with annotation().canonicalName (if present) to form a globally unique name.
	std::string sourceUnitName() const;

	virtual ScopableAnnotation& annotation() const = 0;
};

/**
 * Abstract AST class for a declaration (contract, function, struct, variable, import directive).
 */
class Declaration: public ASTNode, public Scopable
{
public:

	static std::string visibilityToString(Visibility _visibility)
	{
		switch (_visibility)
		{
		case Visibility::Public:
			return "public";
		case Visibility::Internal:
			return "internal";
		case Visibility::Private:
			return "private";
		case Visibility::External:
			return "external";
		default:
			solAssert(false, "Invalid visibility specifier.");
		}
		return std::string();
	}

	Declaration(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> _name,
		SourceLocation _nameLocation,
		Visibility _visibility = Visibility::Default
	):
		ASTNode(_id, _location), m_name(std::move(_name)), m_nameLocation(std::move(_nameLocation)), m_visibility(_visibility) {}

	/// @returns the declared name.
	ASTString const& name() const { return *m_name; }

	/// @returns the location of the declared name itself or empty location if not available or unknown.
	SourceLocation const& nameLocation() const noexcept { return m_nameLocation; }

	bool noVisibilitySpecified() const { return m_visibility == Visibility::Default; }
	Visibility visibility() const { return m_visibility == Visibility::Default ? defaultVisibility() : m_visibility; }
	bool isPublic() const { return visibility() >= Visibility::Public; }
	virtual bool isVisibleInContract() const { return visibility() != Visibility::External; }
	virtual bool isVisibleInDerivedContracts() const { return isVisibleInContract() && visibility() >= Visibility::Internal; }
	bool isVisibleAsLibraryMember() const { return visibility() >= Visibility::Internal; }
	virtual bool isVisibleViaContractTypeAccess() const { return false; }

	virtual bool isLValue() const { return false; }
	virtual bool isPartOfExternalInterface() const { return false; }

	/// @returns true if this is a declaration of an enum member.
	bool isEnumValue() const;
	/// @returns true if this is a declaration of a struct member.
	bool isStructMember() const;
	/// @returns true if this is a declaration of a parameter of an event.
	bool isEventOrErrorParameter() const;

	/// @returns false if the declaration can never be referenced without being qualified with a scope.
	/// Usually the name alone can be used to refer to the corresponding entity.
	/// But, for example, struct member names or enum member names always require a prefix.
	/// Another example is event parameter names, which do not participate in any proper scope.
	bool isVisibleAsUnqualifiedName() const;

	/// @returns the type of expressions referencing this declaration.
	/// This can only be called once types of variable declarations have already been resolved.
	virtual Type const* type() const = 0;

	/// @returns the type for members of the containing contract type that refer to this declaration.
	/// This can only be called once types of variable declarations have already been resolved.
	virtual Type const* typeViaContractName() const { return type(); }

	/// @param _internal false indicates external interface is concerned, true indicates internal interface is concerned.
	/// @returns null when it is not accessible as a function.
	virtual FunctionTypePointer functionType(bool /*_internal*/) const { return {}; }

	DeclarationAnnotation& annotation() const override;

protected:
	virtual Visibility defaultVisibility() const { return Visibility::Public; }

private:
	ASTPointer<ASTString> m_name;
	SourceLocation m_nameLocation;
	Visibility m_visibility;
};

/**
 * Pragma directive, only version requirements in the form `pragma solidity "^0.4.0";` are
 * supported for now.
 */
class PragmaDirective: public ASTNode
{
public:
	PragmaDirective(
		int64_t _id,
		SourceLocation const& _location,
		std::vector<Token> _tokens,
		std::vector<ASTString> _literals
	): ASTNode(_id, _location), m_tokens(std::move(_tokens)), m_literals(std::move(_literals))
	{}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<Token> const& tokens() const { return m_tokens; }
	std::vector<ASTString> const& literals() const { return m_literals; }

private:

	/// Sequence of tokens following the "pragma" keyword.
	std::vector<Token> m_tokens;
	/// Sequence of literals following the "pragma" keyword.
	std::vector<ASTString> m_literals;
};

/**
 * Import directive for referencing other files / source objects.
 * Example: import "abc.sol" // imports all symbols of "abc.sol" into current scope
 * Source objects are identified by a string which can be a file name but does not have to be.
 * Other ways to use it:
 * import "abc" as x; // creates symbol "x" that contains all symbols in "abc"
 * import * as x from "abc"; // same as above
 * import {a as b, c} from "abc"; // creates new symbols "b" and "c" referencing "a" and "c" in "abc", respectively.
 */
class ImportDirective: public Declaration
{
public:
	struct SymbolAlias
	{
		ASTPointer<Identifier> symbol;
		ASTPointer<ASTString> alias;
		SourceLocation location;
	};

	using SymbolAliasList = std::vector<SymbolAlias>;

	ImportDirective(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> _path,
		ASTPointer<ASTString> const& _unitAlias,
		SourceLocation _unitAliasLocation,
		SymbolAliasList _symbolAliases
	):
		Declaration(_id, _location, _unitAlias, std::move(_unitAliasLocation)),
		m_path(std::move(_path)),
		m_symbolAliases(std::move(_symbolAliases))
	{ }

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	ASTString const& path() const { return *m_path; }
	SymbolAliasList const& symbolAliases() const
	{
		return m_symbolAliases;
	}
	ImportAnnotation& annotation() const override;

	Type const* type() const override;

private:
	ASTPointer<ASTString> m_path;
	/// The aliases for the specific symbols to import. If non-empty import the specific symbols.
	/// If the `alias` component is empty, import the identifier unchanged.
	/// If both m_unitAlias and m_symbolAlias are empty, import all symbols into the current scope.
	SymbolAliasList m_symbolAliases;
};

/**
 * Abstract class that is added to each AST node that can store local variables.
 * Local variables in functions are always added to functions, even though they are not
 * in scope for the whole function.
 */
class VariableScope
{
public:
	virtual ~VariableScope() = default;
	void addLocalVariable(VariableDeclaration const& _localVariable) { m_localVariables.push_back(&_localVariable); }
	std::vector<VariableDeclaration const*> const& localVariables() const { return m_localVariables; }

private:
	std::vector<VariableDeclaration const*> m_localVariables;
};

/**
 * The doxygen-style, structured documentation class that represents an AST node.
 */
class StructuredDocumentation: public ASTNode
{
public:
	StructuredDocumentation(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> _text
	): ASTNode(_id, _location), m_text(std::move(_text))
	{}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	/// @return A shared pointer of an ASTString.
	/// Contains doxygen-style, structured documentation that is parsed later on.
	ASTPointer<ASTString> const& text() const { return m_text; }

private:
	ASTPointer<ASTString> m_text;
};

/**
 * Abstract class that is added to each AST node that can receive documentation.
 */
class Documented
{
public:
	virtual ~Documented() = default;
	explicit Documented(ASTPointer<ASTString> _documentation): m_documentation(std::move(_documentation)) {}

	/// @return A shared pointer of an ASTString.
	/// Can contain a nullptr in which case indicates absence of documentation
	ASTPointer<ASTString> const& documentation() const { return m_documentation; }

protected:
	ASTPointer<ASTString> m_documentation;
};

/**
 * Abstract class that is added to each AST node that can receive a structured documentation.
 */
class StructurallyDocumented
{
public:
	virtual ~StructurallyDocumented() = default;
	explicit StructurallyDocumented(ASTPointer<StructuredDocumentation> _documentation): m_documentation(std::move(_documentation)) {}

	/// @return A shared pointer of a FormalDocumentation.
	/// Can contain a nullptr in which case indicates absence of documentation
	ASTPointer<StructuredDocumentation> const& documentation() const { return m_documentation; }

protected:
	ASTPointer<StructuredDocumentation> m_documentation;
};


/**
 * Abstract class that is added to AST nodes that can be marked as not being fully implemented
 */
class ImplementationOptional
{
public:
	virtual ~ImplementationOptional() = default;
	explicit ImplementationOptional(bool _implemented): m_implemented(_implemented) {}

	/// @return whether this node is fully implemented or not
	bool isImplemented() const { return m_implemented; }

protected:
	bool m_implemented;
};

/// @}

/**
 * Definition of a contract or library. This is the only AST nodes where child nodes are not visited in
 * document order. It first visits all struct declarations, then all variable declarations and
 * finally all function declarations.
 */
class ContractDefinition: public Declaration, public StructurallyDocumented, public ScopeOpener
{
public:
	ContractDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		ASTPointer<StructuredDocumentation> const& _documentation,
		std::vector<ASTPointer<InheritanceSpecifier>> _baseContracts,
		std::vector<ASTPointer<ASTNode>> _subNodes,
		ContractKind _contractKind = ContractKind::Contract,
		bool _abstract = false
	):
		Declaration(_id, _location, _name, std::move(_nameLocation)),
		StructurallyDocumented(_documentation),
		m_baseContracts(std::move(_baseContracts)),
		m_subNodes(std::move(_subNodes)),
		m_contractKind(_contractKind),
		m_abstract(_abstract)
	{}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<InheritanceSpecifier>> const& baseContracts() const { return m_baseContracts; }
	std::vector<ASTPointer<ASTNode>> const& subNodes() const { return m_subNodes; }
	std::vector<UsingForDirective const*> usingForDirectives() const { return filteredNodes<UsingForDirective>(m_subNodes); }
	std::vector<StructDefinition const*> definedStructs() const { return filteredNodes<StructDefinition>(m_subNodes); }
	std::vector<EnumDefinition const*> definedEnums() const { return filteredNodes<EnumDefinition>(m_subNodes); }
	std::vector<VariableDeclaration const*> stateVariables() const { return filteredNodes<VariableDeclaration>(m_subNodes); }
	std::vector<ModifierDefinition const*> functionModifiers() const { return filteredNodes<ModifierDefinition>(m_subNodes); }
	std::vector<FunctionDefinition const*> definedFunctions() const { return filteredNodes<FunctionDefinition>(m_subNodes); }
	/// @returns a view<FunctionDefinition const*> of all functions
	/// defined in this contract of the given name (excluding inherited functions).
	auto definedFunctions(std::string const& _name) const
	{
		auto&& [b, e] = definedFunctionsByName().equal_range(_name);
		return ranges::subrange<decltype(b)>(b, e) | ranges::views::values;
	}
	std::vector<EventDefinition const*> events() const { return filteredNodes<EventDefinition>(m_subNodes); }
	std::vector<EventDefinition const*> const& definedInterfaceEvents() const;
	std::vector<EventDefinition const*> const usedInterfaceEvents() const;
	/// @returns all errors defined in this contract or any base contract
	/// and all errors referenced during execution.
	/// @param _requireCallGraph if false, do not fail if the call graph has not been computed yet.
	std::vector<ErrorDefinition const*> interfaceErrors(bool _requireCallGraph = true) const;
	bool isInterface() const { return m_contractKind == ContractKind::Interface; }
	bool isLibrary() const { return m_contractKind == ContractKind::Library; }

	/// @returns true, if the contract derives from @arg _base.
	bool derivesFrom(ContractDefinition const& _base) const;

	/// @returns a map of canonical function signatures to FunctionDefinitions
	/// as intended for use by the ABI.
	std::map<util::FixedHash<4>, FunctionTypePointer> interfaceFunctions(bool _includeInheritedFunctions = true) const;
	std::vector<std::pair<util::FixedHash<4>, FunctionTypePointer>> const& interfaceFunctionList(bool _includeInheritedFunctions = true) const;
	/// @returns the EIP-165 compatible interface identifier. This will exclude inherited functions.
	uint32_t interfaceId() const;

	/// @returns a list of all declarations in this contract
	std::vector<Declaration const*> declarations() const { return filteredNodes<Declaration>(m_subNodes); }

	/// Returns the constructor or nullptr if no constructor was specified.
	FunctionDefinition const* constructor() const;
	/// @returns true iff the contract can be deployed, i.e. is not abstract and has a
	/// public constructor.
	/// Should only be called after the type checker has run.
	bool canBeDeployed() const;
	/// Returns the fallback function or nullptr if no fallback function was specified.
	FunctionDefinition const* fallbackFunction() const;

	/// Returns the ether receiver function or nullptr if no receive function was specified.
	FunctionDefinition const* receiveFunction() const;

	std::string fullyQualifiedName() const { return sourceUnitName() + ":" + name(); }

	Type const* type() const override;

	ContractDefinitionAnnotation& annotation() const override;

	ContractKind contractKind() const { return m_contractKind; }

	bool abstract() const { return m_abstract; }

	ContractDefinition const* superContract(ContractDefinition const& _mostDerivedContract) const;
	/// @returns the next constructor in the inheritance hierarchy.
	FunctionDefinition const* nextConstructor(ContractDefinition const& _mostDerivedContract) const;

private:
	std::multimap<std::string, FunctionDefinition const*> const& definedFunctionsByName() const;

	std::vector<ASTPointer<InheritanceSpecifier>> m_baseContracts;
	std::vector<ASTPointer<ASTNode>> m_subNodes;
	ContractKind m_contractKind;
	bool m_abstract{false};

	util::LazyInit<std::vector<std::pair<util::FixedHash<4>, FunctionTypePointer>>> m_interfaceFunctionList[2];
	util::LazyInit<std::vector<EventDefinition const*>> m_interfaceEvents;
	util::LazyInit<std::multimap<std::string, FunctionDefinition const*>> m_definedFunctionsByName;
};

/**
 * A sequence of identifiers separated by dots used outside the expression context. Inside the expression context, this is a sequence of Identifier and MemberAccess.
 */
class IdentifierPath: public ASTNode
{
public:
	IdentifierPath(
		int64_t _id,
		SourceLocation const& _location,
		std::vector<ASTString> _path,
		std::vector<SourceLocation> _pathLocations
	):
		ASTNode(_id, _location), m_path(std::move(_path)), m_pathLocations(std::move(_pathLocations))
	{
		solAssert(m_pathLocations.size() == m_path.size());
	}

	std::vector<ASTString> const& path() const { return m_path; }
	std::vector<SourceLocation > const& pathLocations() const { return m_pathLocations; }
	IdentifierPathAnnotation& annotation() const override
	{
		return initAnnotation<IdentifierPathAnnotation>();
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;
private:
	std::vector<ASTString> m_path;
	// Corresponding locations for m_path. Array has same length and indices as m_path.
	std::vector<SourceLocation> m_pathLocations;
};

class InheritanceSpecifier: public ASTNode
{
public:
	InheritanceSpecifier(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<IdentifierPath> _baseName,
		std::unique_ptr<std::vector<ASTPointer<Expression>>> _arguments
	):
		ASTNode(_id, _location), m_baseName(std::move(_baseName)), m_arguments(std::move(_arguments))
	{
		solAssert(m_baseName != nullptr, "Name cannot be null.");
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	IdentifierPath const& name() const { return *m_baseName; }
	// Returns nullptr if no argument list was given (``C``).
	// If an argument list is given (``C(...)``), the arguments are returned
	// as a vector of expressions. Note that this vector can be empty (``C()``).
	std::vector<ASTPointer<Expression>> const* arguments() const { return m_arguments.get(); }

private:
	ASTPointer<IdentifierPath> m_baseName;
	std::unique_ptr<std::vector<ASTPointer<Expression>>> m_arguments;
};

/**
 * Using for directive:
 *
 * 1. `using LibraryName for T` attaches all functions from the library `LibraryName` to the type `T`
 * 2. `using LibraryName for *` attaches to all types.
 * 3. `using {f1, f2, ..., fn} for T` attaches the functions `f1`, `f2`, ...,
 *     `fn`, respectively to `T`.
 *
 * For version 3, T has to be implicitly convertible to the first parameter type of
 * all functions, and this is checked at the point of the using statement. For versions 1 and
 * 2, this check is only done when a function is called.
 *
 * Finally, `using {f1, f2, ..., fn} for T global` is also valid at file level, as long as T is
 * a user-defined type defined in the same file at file level. In this case, the methods are
 * attached to all objects of that type regardless of scope.
 */
class UsingForDirective: public ASTNode
{
public:
	UsingForDirective(
		int64_t _id,
		SourceLocation const& _location,
		std::vector<ASTPointer<IdentifierPath>> _functions,
		bool _usesBraces,
		ASTPointer<TypeName> _typeName,
		bool _global
	):
		ASTNode(_id, _location),
		m_functions(_functions),
		m_usesBraces(_usesBraces),
		m_typeName(std::move(_typeName)),
		m_global{_global}
	{
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	/// @returns the type name the library is attached to, null for `*`.
	TypeName const* typeName() const { return m_typeName.get(); }

	/// @returns a list of functions or the single library.
	std::vector<ASTPointer<IdentifierPath>> const& functionsOrLibrary() const { return m_functions; }
	bool usesBraces() const { return m_usesBraces; }
	bool global() const { return m_global; }

private:
	/// Either the single library or a list of functions.
	std::vector<ASTPointer<IdentifierPath>> m_functions;
	bool m_usesBraces;
	ASTPointer<TypeName> m_typeName;
	bool m_global = false;
};

class StructDefinition: public Declaration, public ScopeOpener
{
public:
	StructDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		std::vector<ASTPointer<VariableDeclaration>> _members
	):
		Declaration(_id, _location, _name, std::move(_nameLocation)), m_members(std::move(_members)) {}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& members() const { return m_members; }

	Type const* type() const override;

	bool isVisibleInDerivedContracts() const override { return true; }
	bool isVisibleViaContractTypeAccess() const override { return true; }

	StructDeclarationAnnotation& annotation() const override;

private:
	std::vector<ASTPointer<VariableDeclaration>> m_members;
};

class EnumDefinition: public Declaration, public ScopeOpener
{
public:
	EnumDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		std::vector<ASTPointer<EnumValue>> _members
	):
		Declaration(_id, _location, _name, std::move(_nameLocation)), m_members(std::move(_members)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	bool isVisibleInDerivedContracts() const override { return true; }
	bool isVisibleViaContractTypeAccess() const override { return true; }

	std::vector<ASTPointer<EnumValue>> const& members() const { return m_members; }

	Type const* type() const override;

	TypeDeclarationAnnotation& annotation() const override;

private:
	std::vector<ASTPointer<EnumValue>> m_members;
};

/**
 * Declaration of an Enum Value
 */
class EnumValue: public Declaration
{
public:
	EnumValue(int64_t _id, SourceLocation const& _location, ASTPointer<ASTString> const& _name):
		Declaration(_id, _location, _name, _location) {}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Type const* type() const override;
};

/**
 * User defined value types, i.e., custom types, for example, `type MyInt is int`. Allows creating a
 * zero cost abstraction over value type with stricter type requirements.
 */
class UserDefinedValueTypeDefinition: public Declaration
{
public:
	UserDefinedValueTypeDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> _name,
		SourceLocation _nameLocation,
		ASTPointer<TypeName> _underlyingType
	):
		Declaration(_id, _location, _name, std::move(_nameLocation), Visibility::Default),
		m_underlyingType(std::move(_underlyingType))
	{
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Type const* type() const override;

	TypeDeclarationAnnotation& annotation() const override;

	TypeName const* underlyingType() const { return m_underlyingType.get(); }
	bool isVisibleViaContractTypeAccess() const override { return true; }

private:
	/// The name of the underlying type
	ASTPointer<TypeName> m_underlyingType;
};

/**
 * Parameter list, used as function parameter list, return list and for try and catch.
 * None of the parameters is allowed to contain mappings (not even recursively
 * inside structs).
 */
class ParameterList: public ASTNode
{
public:
	ParameterList(
		int64_t _id,
		SourceLocation const& _location,
		std::vector<ASTPointer<VariableDeclaration>> _parameters
	):
		ASTNode(_id, _location), m_parameters(std::move(_parameters)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& parameters() const { return m_parameters; }

private:
	std::vector<ASTPointer<VariableDeclaration>> m_parameters;
};

/**
 * Base class for all nodes that define function-like objects, i.e. FunctionDefinition,
 * EventDefinition, ErrorDefinition and ModifierDefinition.
 */
class CallableDeclaration: public Declaration, public VariableScope
{
public:
	CallableDeclaration(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		Visibility _visibility,
		ASTPointer<ParameterList> _parameters,
		bool _isVirtual = false,
		ASTPointer<OverrideSpecifier> _overrides = nullptr,
		ASTPointer<ParameterList> _returnParameters = ASTPointer<ParameterList>()
	):
		Declaration(_id, _location, _name, std::move(_nameLocation), _visibility),
		m_parameters(std::move(_parameters)),
		m_overrides(std::move(_overrides)),
		m_returnParameters(std::move(_returnParameters)),
		m_isVirtual(_isVirtual)
	{
	}

	std::vector<ASTPointer<VariableDeclaration>> const& parameters() const { return m_parameters->parameters(); }
	ASTPointer<OverrideSpecifier> const& overrides() const { return m_overrides; }
	std::vector<ASTPointer<VariableDeclaration>> const& returnParameters() const { return m_returnParameters->parameters(); }
	ParameterList const& parameterList() const { return *m_parameters; }
	ASTPointer<ParameterList> const& returnParameterList() const { return m_returnParameters; }
	bool markedVirtual() const { return m_isVirtual; }
	virtual bool virtualSemantics() const { return markedVirtual(); }

	CallableDeclarationAnnotation& annotation() const override = 0;

	/// Performs virtual or super function/modifier lookup:
	/// If @a _searchStart is nullptr, performs virtual function lookup, i.e.
	/// searches the inheritance hierarchy of @a _mostDerivedContract towards the base
	/// and returns the first function/modifier definition that
	/// is overwritten by this callable.
	/// If @a _searchStart is non-null, starts searching only from that contract, but
	/// still in the hierarchy of @a _mostDerivedContract.
	virtual CallableDeclaration const& resolveVirtual(
		ContractDefinition const& _mostDerivedContract,
		ContractDefinition const* _searchStart = nullptr
	) const = 0;

protected:
	ASTPointer<ParameterList> m_parameters;
	ASTPointer<OverrideSpecifier> m_overrides;
	ASTPointer<ParameterList> m_returnParameters;
	bool m_isVirtual = false;
};

/**
 * Function override specifier. Consists of a single override keyword
 * potentially followed by a parenthesized list of base contract names.
 */
class OverrideSpecifier: public ASTNode
{
public:
	OverrideSpecifier(
		int64_t _id,
		SourceLocation const& _location,
		std::vector<ASTPointer<IdentifierPath>> _overrides
	):
		ASTNode(_id, _location),
		m_overrides(std::move(_overrides))
	{
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	/// @returns the list of specific overrides, if any
	std::vector<ASTPointer<IdentifierPath>> const& overrides() const { return m_overrides; }

protected:
	std::vector<ASTPointer<IdentifierPath>> m_overrides;
};

class FunctionDefinition: public CallableDeclaration, public StructurallyDocumented, public ImplementationOptional, public ScopeOpener
{
public:
	FunctionDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation const& _nameLocation,
		Visibility _visibility,
		StateMutability _stateMutability,
		bool _free,
		Token _kind,
		bool _isVirtual,
		ASTPointer<OverrideSpecifier> const& _overrides,
		ASTPointer<StructuredDocumentation> const& _documentation,
		ASTPointer<ParameterList> const& _parameters,
		std::vector<ASTPointer<ModifierInvocation>> _modifiers,
		ASTPointer<ParameterList> const& _returnParameters,
		ASTPointer<Block> const& _body
	):
		CallableDeclaration(_id, _location, _name, std::move(_nameLocation), _visibility, _parameters, _isVirtual, _overrides, _returnParameters),
		StructurallyDocumented(_documentation),
		ImplementationOptional(_body != nullptr),
		m_stateMutability(_stateMutability),
		m_free(_free),
		m_kind(_kind),
		m_functionModifiers(std::move(_modifiers)),
		m_body(_body)
	{
		solAssert(_kind == Token::Constructor || _kind == Token::Function || _kind == Token::Fallback || _kind == Token::Receive, "");
		solAssert(isOrdinary() == !name().empty(), "");
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	StateMutability stateMutability() const { return m_stateMutability; }
	bool libraryFunction() const;
	bool isOrdinary() const { return m_kind == Token::Function; }
	bool isConstructor() const { return m_kind == Token::Constructor; }
	bool isFallback() const { return m_kind == Token::Fallback; }
	bool isReceive() const { return m_kind == Token::Receive; }
	bool isFree() const { return m_free; }
	Token kind() const { return m_kind; }
	bool isPayable() const { return m_stateMutability == StateMutability::Payable; }
	std::vector<ASTPointer<ModifierInvocation>> const& modifiers() const { return m_functionModifiers; }
	Block const& body() const { solAssert(m_body, ""); return *m_body; }
	Visibility defaultVisibility() const override;
	bool isVisibleInContract() const override
	{
		return isOrdinary() && Declaration::isVisibleInContract();
	}
	bool isVisibleViaContractTypeAccess() const override
	{
		solAssert(!isFree(), "");
		return isOrdinary() && visibility() >= Visibility::Public;
	}
	bool isPartOfExternalInterface() const override { return isOrdinary() && isPublic(); }

	/// @returns the external signature of the function
	/// That consists of the name of the function followed by the types of the
	/// arguments separated by commas all enclosed in parentheses without any spaces.
	std::string externalSignature() const;

	/// @returns the external identifier of this function (the hash of the signature) as a hex string.
	std::string externalIdentifierHex() const;

	Type const* type() const override;
	Type const* typeViaContractName() const override;

	/// @param _internal false indicates external interface is concerned, true indicates internal interface is concerned.
	/// @returns null when it is not accessible as a function.
	FunctionTypePointer functionType(bool /*_internal*/) const override;

	FunctionDefinitionAnnotation& annotation() const override;

	bool virtualSemantics() const override
	{
		return
			CallableDeclaration::virtualSemantics() ||
			(annotation().contract && annotation().contract->isInterface());
	}

	FunctionDefinition const& resolveVirtual(
		ContractDefinition const& _mostDerivedContract,
		ContractDefinition const* _searchStart = nullptr
	) const override;

private:
	StateMutability m_stateMutability;
	bool m_free;
	Token const m_kind;
	std::vector<ASTPointer<ModifierInvocation>> m_functionModifiers;
	ASTPointer<Block> m_body;
};

/**
 * Declaration of a variable. This can be used in various places, e.g. in function parameter
 * lists, struct definitions and even function bodies.
 */
class VariableDeclaration: public Declaration, public StructurallyDocumented
{
public:
	enum Location { Unspecified, Storage, Memory, CallData, Code};
	enum class Mutability { Mutable, Immutable, Constant };
	static std::string mutabilityToString(Mutability _mutability)
	{
		switch (_mutability)
		{
		case Mutability::Mutable: return "mutable";
		case Mutability::Immutable: return "immutable";
		case Mutability::Constant: return "constant";
		}
		return {};
	}

	VariableDeclaration(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<TypeName> _type,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		ASTPointer<Expression> _value,
		Visibility _visibility,
		ASTPointer<StructuredDocumentation> const _documentation = nullptr,
		bool _isIndexed = false,
		Mutability _mutability = Mutability::Mutable,
		ASTPointer<OverrideSpecifier> _overrides = nullptr,
		Location _referenceLocation = Location::Unspecified
	):
		Declaration(_id, _location, _name, std::move(_nameLocation), _visibility),
		StructurallyDocumented(std::move(_documentation)),
		m_typeName(std::move(_type)),
		m_value(std::move(_value)),
		m_isIndexed(_isIndexed),
		m_mutability(_mutability),
		m_overrides(std::move(_overrides)),
		m_location(_referenceLocation)
	{
		solAssert(m_typeName, "");
	}


	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	TypeName const& typeName() const { return *m_typeName; }
	ASTPointer<Expression> const& value() const { return m_value; }

	bool isLValue() const override;
	bool isPartOfExternalInterface() const override { return isPublic(); }

	/// @returns true iff this variable is the parameter (or return parameter) of a function
	/// (or function type name or event) or declared inside a function body.
	bool isLocalVariable() const;
	/// @returns true if this variable is a parameter or return parameter of a function.
	bool isCallableOrCatchParameter() const;
	/// @returns true if this variable is a return parameter of a function.
	bool isReturnParameter() const;
	/// @returns true if this variable is a parameter of the success or failure clausse
	/// of a try/catch statement.
	bool isTryCatchParameter() const;
	/// @returns true if this variable is a local variable or return parameter.
	bool isLocalOrReturn() const;
	/// @returns true if this variable is a parameter (not return parameter) of an external function.
	/// This excludes parameters of external function type names.
	bool isExternalCallableParameter() const;
	/// @returns true if this variable is a parameter (not return parameter) of a public function.
	bool isPublicCallableParameter() const;
	/// @returns true if this variable is a parameter or return parameter of an internal function
	/// or a function type of internal visibility.
	bool isInternalCallableParameter() const;
	/// @returns true if this variable is the parameter of a constructor.
	bool isConstructorParameter() const;
	/// @returns true iff this variable is a parameter(or return parameter of a library function
	bool isLibraryFunctionParameter() const;
	/// @returns true if the type of the variable is a reference or mapping type, i.e.
	/// array, struct or mapping. These types can take a data location (and often require it).
	/// Can only be called after reference resolution.
	bool hasReferenceOrMappingType() const;
	bool isStateVariable() const;
	bool isFileLevelVariable() const;
	bool isIndexed() const { return m_isIndexed; }
	Mutability mutability() const { return m_mutability; }
	bool isConstant() const { return m_mutability == Mutability::Constant; }
	bool immutable() const { return m_mutability == Mutability::Immutable; }
	ASTPointer<OverrideSpecifier> const& overrides() const { return m_overrides; }
	Location referenceLocation() const { return m_location; }
	/// @returns a set of allowed storage locations for the variable.
	std::set<Location> allowedDataLocations() const;

	/// @returns the external identifier of this variable (the hash of the signature) as a hex string (works only for public state variables).
	std::string externalIdentifierHex() const;

	Type const* type() const override;

	/// @param _internal false indicates external interface is concerned, true indicates internal interface is concerned.
	/// @returns null when it is not accessible as a function.
	FunctionTypePointer functionType(bool /*_internal*/) const override;

	VariableDeclarationAnnotation& annotation() const override;

protected:
	Visibility defaultVisibility() const override { return Visibility::Internal; }

private:
	ASTPointer<TypeName> m_typeName;
	/// Initially assigned value, can be missing. For local variables, this is stored inside
	/// VariableDeclarationStatement and not here.
	ASTPointer<Expression> m_value;
	bool m_isIndexed = false; ///< Whether this is an indexed variable (used by events).
	/// Whether the variable is "constant", "immutable" or non-marked (mutable).
	Mutability m_mutability = Mutability::Mutable;
	ASTPointer<OverrideSpecifier> m_overrides; ///< Contains the override specifier node
	Location m_location = Location::Unspecified; ///< Location of the variable if it is of reference type.
};

/**
 * Definition of a function modifier.
 */
class ModifierDefinition: public CallableDeclaration, public StructurallyDocumented, public ImplementationOptional, public ScopeOpener
{
public:
	ModifierDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		ASTPointer<StructuredDocumentation> const& _documentation,
		ASTPointer<ParameterList> const& _parameters,
		bool _isVirtual,
		ASTPointer<OverrideSpecifier> const& _overrides,
		ASTPointer<Block> const& _body
	):
		CallableDeclaration(_id, _location, _name, std::move(_nameLocation), Visibility::Internal, _parameters, _isVirtual, _overrides),
		StructurallyDocumented(_documentation),
		ImplementationOptional(_body != nullptr),
		m_body(_body)
	{
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Block const& body() const { solAssert(m_body, ""); return *m_body; }

	Type const* type() const override;

	Visibility defaultVisibility() const override { return Visibility::Internal; }

	ModifierDefinitionAnnotation& annotation() const override;

	ModifierDefinition const& resolveVirtual(
		ContractDefinition const& _mostDerivedContract,
		ContractDefinition const* _searchStart = nullptr
	) const override;


private:
	ASTPointer<Block> m_body;
};

/**
 * Invocation/usage of a modifier in a function header or a base constructor call.
 */
class ModifierInvocation: public ASTNode
{
public:
	ModifierInvocation(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<IdentifierPath> _name,
		std::unique_ptr<std::vector<ASTPointer<Expression>>> _arguments
	):
		ASTNode(_id, _location), m_modifierName(std::move(_name)), m_arguments(std::move(_arguments))
	{
		solAssert(m_modifierName != nullptr, "Name cannot be null.");
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	IdentifierPath& name() const { return *m_modifierName; }
	// Returns nullptr if no argument list was given (``mod``).
	// If an argument list is given (``mod(...)``), the arguments are returned
	// as a vector of expressions. Note that this vector can be empty (``mod()``).
	std::vector<ASTPointer<Expression>> const* arguments() const { return m_arguments.get(); }

private:
	ASTPointer<IdentifierPath> m_modifierName;
	std::unique_ptr<std::vector<ASTPointer<Expression>>> m_arguments;
};

/**
 * Definition of a (loggable) event.
 */
class EventDefinition: public CallableDeclaration, public StructurallyDocumented, public ScopeOpener
{
public:
	EventDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		ASTPointer<StructuredDocumentation> const& _documentation,
		ASTPointer<ParameterList> const& _parameters,
		bool _anonymous = false
	):
		CallableDeclaration(_id, _location, _name, std::move(_nameLocation), Visibility::Default, _parameters),
		StructurallyDocumented(_documentation),
		m_anonymous(_anonymous)
	{
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	bool isAnonymous() const { return m_anonymous; }

	Type const* type() const override;
	FunctionTypePointer functionType(bool /*_internal*/) const override;

	bool isVisibleInDerivedContracts() const override { return true; }
	bool isVisibleViaContractTypeAccess() const override { return false; /* TODO */ }

	EventDefinitionAnnotation& annotation() const override;

	CallableDeclaration const& resolveVirtual(
		ContractDefinition const&,
		ContractDefinition const*
	) const override
	{
		return *this;
	}

private:
	bool m_anonymous = false;
};

/**
 * Definition of an error type usable in ``revert(MyError(x))``, ``require(condition, MyError(x))``
 * and ``catch MyError(_x)``.
 */
class ErrorDefinition: public CallableDeclaration, public StructurallyDocumented, public ScopeOpener
{
public:
	ErrorDefinition(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		SourceLocation _nameLocation,
		ASTPointer<StructuredDocumentation> const& _documentation,
		ASTPointer<ParameterList> const& _parameters
	):
		CallableDeclaration(_id, _location, _name, std::move(_nameLocation), Visibility::Default, _parameters),
		StructurallyDocumented(_documentation)
	{
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Type const* type() const override;

	FunctionTypePointer functionType(bool _internal) const override;

	bool isVisibleInDerivedContracts() const override { return true; }
	bool isVisibleViaContractTypeAccess() const override { return true; }

	ErrorDefinitionAnnotation& annotation() const override;

	CallableDeclaration const& resolveVirtual(
		ContractDefinition const&,
		ContractDefinition const*
	) const override
	{
		return *this;
	}
};

/**
 * Pseudo AST node that is used as declaration for "this", "msg", "tx", "block" and the global
 * functions when such an identifier is encountered. Will never have a valid location in the source code
 */
class MagicVariableDeclaration: public Declaration
{
public:
	MagicVariableDeclaration(int _id, ASTString const& _name, Type const* _type):
		Declaration(_id, SourceLocation(), std::make_shared<ASTString>(_name), {}), m_type(_type) { }

	void accept(ASTVisitor&) override
	{
		solAssert(false, "MagicVariableDeclaration used inside real AST.");
	}
	void accept(ASTConstVisitor&) const override
	{
		solAssert(false, "MagicVariableDeclaration used inside real AST.");
	}

	FunctionType const* functionType(bool) const override
	{
		solAssert(m_type->category() == Type::Category::Function, "");
		return dynamic_cast<FunctionType const*>(m_type);
	}
	Type const* type() const override { return m_type; }

private:
	Type const* m_type;
};

/// Types
/// @{

/**
 * Abstract base class of a type name, can be any built-in or user-defined type.
 */
class TypeName: public ASTNode
{
protected:
	explicit TypeName(int64_t _id, SourceLocation const& _location): ASTNode(_id, _location) {}

public:
	TypeNameAnnotation& annotation() const override;
};

/**
 * Any pre-defined type name represented by a single keyword (and possibly a state mutability for address types),
 * i.e. it excludes mappings, contracts, functions, etc.
 */
class ElementaryTypeName: public TypeName
{
public:
	ElementaryTypeName(
		int64_t _id,
		SourceLocation const& _location,
		ElementaryTypeNameToken const& _elem,
		std::optional<StateMutability> _stateMutability = {}
	): TypeName(_id, _location), m_type(_elem), m_stateMutability(_stateMutability)
	{
		solAssert(!_stateMutability.has_value() || _elem.token() == Token::Address, "");
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	ElementaryTypeNameToken const& typeName() const { return m_type; }

	std::optional<StateMutability> const& stateMutability() const { return m_stateMutability; }

private:
	ElementaryTypeNameToken m_type;
	std::optional<StateMutability> m_stateMutability; ///< state mutability for address type
};

/**
 * Name referring to a user-defined type (i.e. a struct, contract, etc.).
 */
class UserDefinedTypeName: public TypeName
{
public:
	UserDefinedTypeName(int64_t _id, SourceLocation const& _location, ASTPointer<IdentifierPath> _namePath):
		TypeName(_id, _location), m_namePath(std::move(_namePath))
	{
		solAssert(m_namePath != nullptr, "Name cannot be null.");
	}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTString> const& namePath() const { return m_namePath->path(); }
	IdentifierPath& pathNode() const { return *m_namePath; }

private:
	ASTPointer<IdentifierPath> m_namePath;
};

/**
 * A literal function type. Its source form is "function (paramType1, paramType2) internal / external returns (retType1, retType2)"
 */
class FunctionTypeName: public TypeName, public ScopeOpener
{
public:
	FunctionTypeName(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ParameterList> _parameterTypes,
		ASTPointer<ParameterList> _returnTypes,
		Visibility _visibility,
		StateMutability _stateMutability
	):
		TypeName(_id, _location), m_parameterTypes(std::move(_parameterTypes)), m_returnTypes(std::move(_returnTypes)),
		m_visibility(_visibility), m_stateMutability(_stateMutability)
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& parameterTypes() const { return m_parameterTypes->parameters(); }
	std::vector<ASTPointer<VariableDeclaration>> const& returnParameterTypes() const { return m_returnTypes->parameters(); }
	ASTPointer<ParameterList> const& parameterTypeList() const { return m_parameterTypes; }
	ASTPointer<ParameterList> const& returnParameterTypeList() const { return m_returnTypes; }

	Visibility visibility() const
	{
		return m_visibility == Visibility::Default ? Visibility::Internal : m_visibility;
	}
	StateMutability stateMutability() const { return m_stateMutability; }
	bool isPayable() const { return m_stateMutability == StateMutability::Payable; }

private:
	ASTPointer<ParameterList> m_parameterTypes;
	ASTPointer<ParameterList> m_returnTypes;
	Visibility m_visibility;
	StateMutability m_stateMutability;
};

/**
 * A mapping type. Its source form is "mapping('keyType' => 'valueType')"
 */
class Mapping: public TypeName
{
public:
	Mapping(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<TypeName> _keyType,
		ASTPointer<TypeName> _valueType
	):
		TypeName(_id, _location), m_keyType(std::move(_keyType)), m_valueType(std::move(_valueType)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	TypeName const& keyType() const { return *m_keyType; }
	TypeName const& valueType() const { return *m_valueType; }

private:
	ASTPointer<TypeName> m_keyType;
	ASTPointer<TypeName> m_valueType;
};

/**
 * An array type, can be "typename[]" or "typename[<expression>]".
 */
class ArrayTypeName: public TypeName
{
public:
	ArrayTypeName(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<TypeName> _baseType,
		ASTPointer<Expression> _length
	):
		TypeName(_id, _location), m_baseType(std::move(_baseType)), m_length(std::move(_length)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	TypeName const& baseType() const { return *m_baseType; }
	Expression const* length() const { return m_length.get(); }

private:
	ASTPointer<TypeName> m_baseType;
	ASTPointer<Expression> m_length; ///< Length of the array, might be empty.
};

/// @}

/// Statements
/// @{


/**
 * Abstract base class for statements.
 */
class Statement: public ASTNode, public Documented
{
public:
	explicit Statement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString
	): ASTNode(_id, _location), Documented(_docString) {}

	StatementAnnotation& annotation() const override;
};

/**
 * Inline assembly.
 */
class InlineAssembly: public Statement
{
public:
	InlineAssembly(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		yul::Dialect const& _dialect,
		ASTPointer<std::vector<ASTPointer<ASTString>>> _flags,
		std::shared_ptr<yul::Block> _operations
	):
		Statement(_id, _location, _docString),
		m_dialect(_dialect),
		m_flags(std::move(_flags)),
		m_operations(std::move(_operations))
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	yul::Dialect const& dialect() const { return m_dialect; }
	yul::Block const& operations() const { return *m_operations; }
	ASTPointer<std::vector<ASTPointer<ASTString>>> const& flags() const { return m_flags; }

	InlineAssemblyAnnotation& annotation() const override;

private:
	yul::Dialect const& m_dialect;
	ASTPointer<std::vector<ASTPointer<ASTString>>> m_flags;
	std::shared_ptr<yul::Block> m_operations;
};

/**
 * Brace-enclosed block containing zero or more statements.
 */
class Block: public Statement, public Scopable, public ScopeOpener
{
public:
	Block(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		bool _unchecked,
		std::vector<ASTPointer<Statement>> _statements
	):
		Statement(_id, _location, _docString),
		m_statements(std::move(_statements)),
		m_unchecked(_unchecked)
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<Statement>> const& statements() const { return m_statements; }
	bool unchecked() const { return m_unchecked; }

	BlockAnnotation& annotation() const override;

private:
	std::vector<ASTPointer<Statement>> m_statements;
	bool m_unchecked;
};

/**
 * Special placeholder statement denoted by "_" used in function modifiers. This is replaced by
 * the original function when the modifier is applied.
 */
class PlaceholderStatement: public Statement
{
public:
	explicit PlaceholderStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString
	): Statement(_id, _location, _docString) {}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;
};

/**
 * If-statement with an optional "else" part. Note that "else if" is modeled by having a new
 * if-statement as the false (else) body.
 */
class IfStatement: public Statement
{
public:
	IfStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> _condition,
		ASTPointer<Statement> _trueBody,
		ASTPointer<Statement> _falseBody
	):
		Statement(_id, _location, _docString),
		m_condition(std::move(_condition)),
		m_trueBody(std::move(_trueBody)),
		m_falseBody(std::move(_falseBody))
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& condition() const { return *m_condition; }
	Statement const& trueStatement() const { return *m_trueBody; }
	/// @returns the "else" part of the if statement or nullptr if there is no "else" part.
	Statement const* falseStatement() const { return m_falseBody.get(); }

private:
	ASTPointer<Expression> m_condition;
	ASTPointer<Statement> m_trueBody;
	ASTPointer<Statement> m_falseBody; ///< "else" part, optional
};

/**
 * Clause of a try-catch block. Includes both the successful case and the
 * unsuccessful cases.
 * Names are only allowed for the unsuccessful cases.
 */
class TryCatchClause: public ASTNode, public Scopable, public ScopeOpener
{
public:
	TryCatchClause(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> _errorName,
		ASTPointer<ParameterList> _parameters,
		ASTPointer<Block> _block
	):
		ASTNode(_id, _location),
		m_errorName(std::move(_errorName)),
		m_parameters(std::move(_parameters)),
		m_block(std::move(_block))
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	ASTString const& errorName() const { return *m_errorName; }
	ParameterList const* parameters() const { return m_parameters.get(); }
	Block const& block() const { return *m_block; }

	TryCatchClauseAnnotation& annotation() const override;

private:
	ASTPointer<ASTString> m_errorName;
	ASTPointer<ParameterList> m_parameters;
	ASTPointer<Block> m_block;
};

/**
 * Try-statement with a variable number of catch statements.
 * Syntax:
 * try <call> returns (uint x, uint y) {
 *   // success code
 * } catch Panic(uint errorCode) {
 *   // panic
 * } catch Error(string memory cause) {
 *   // error code, reason provided
 * } catch (bytes memory lowLevelData) {
 *   // error code, no reason provided or non-matching error signature.
 * }
 *
 * The last statement given above can also be specified as
 * } catch () {
 */
class TryStatement: public Statement
{
public:
	TryStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> _externalCall,
		std::vector<ASTPointer<TryCatchClause>> _clauses
	):
		Statement(_id, _location, _docString),
		m_externalCall(std::move(_externalCall)),
		m_clauses(std::move(_clauses))
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& externalCall() const { return *m_externalCall; }
	std::vector<ASTPointer<TryCatchClause>> const& clauses() const { return m_clauses; }

	TryCatchClause const* successClause() const;
	TryCatchClause const* panicClause() const;
	TryCatchClause const* errorClause() const;
	TryCatchClause const* fallbackClause() const;

private:
	ASTPointer<Expression> m_externalCall;
	std::vector<ASTPointer<TryCatchClause>> m_clauses;
};

/**
 * Statement in which a break statement is legal (abstract class).
 */
class BreakableStatement: public Statement
{
public:
	explicit BreakableStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString
	): Statement(_id, _location, _docString) {}
};

class WhileStatement: public BreakableStatement
{
public:
	WhileStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> _condition,
		ASTPointer<Statement> _body,
		bool _isDoWhile
	):
		BreakableStatement(_id, _location, _docString), m_condition(std::move(_condition)), m_body(std::move(_body)),
		m_isDoWhile(_isDoWhile) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& condition() const { return *m_condition; }
	Statement const& body() const { return *m_body; }
	bool isDoWhile() const { return m_isDoWhile; }

private:
	ASTPointer<Expression> m_condition;
	ASTPointer<Statement> m_body;
	bool m_isDoWhile;
};

/**
 * For loop statement
 */
class ForStatement: public BreakableStatement, public Scopable, public ScopeOpener
{
public:
	ForStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Statement> _initExpression,
		ASTPointer<Expression> _conditionExpression,
		ASTPointer<ExpressionStatement> _loopExpression,
		ASTPointer<Statement> _body
	):
		BreakableStatement(_id, _location, _docString),
		m_initExpression(std::move(_initExpression)),
		m_condExpression(std::move(_conditionExpression)),
		m_loopExpression(std::move(_loopExpression)),
		m_body(std::move(_body))
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Statement const* initializationExpression() const { return m_initExpression.get(); }
	Expression const* condition() const { return m_condExpression.get(); }
	ExpressionStatement const* loopExpression() const { return m_loopExpression.get(); }
	Statement const& body() const { return *m_body; }

	ForStatementAnnotation& annotation() const override;

private:
	/// For statement's initialization expression. for (XXX; ; ). Can be empty
	ASTPointer<Statement> m_initExpression;
	/// For statement's condition expression. for (; XXX ; ). Can be empty
	ASTPointer<Expression> m_condExpression;
	/// For statement's loop expression. for (;;XXX). Can be empty
	ASTPointer<ExpressionStatement> m_loopExpression;
	/// The body of the loop
	ASTPointer<Statement> m_body;
};

class Continue: public Statement
{
public:
	explicit Continue(int64_t _id, SourceLocation const& _location, ASTPointer<ASTString> const& _docString):
		Statement(_id, _location, _docString) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;
};

class Break: public Statement
{
public:
	explicit Break(int64_t _id, SourceLocation const& _location, ASTPointer<ASTString> const& _docString):
		Statement(_id, _location, _docString) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;
};

class Return: public Statement
{
public:
	Return(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> _expression
	): Statement(_id, _location, _docString), m_expression(std::move(_expression)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const* expression() const { return m_expression.get(); }

	ReturnAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_expression; ///< value to return, optional
};

/**
 * @brief The Throw statement to throw that triggers a solidity exception(jump to ErrorTag)
 */
class Throw: public Statement
{
public:
	explicit Throw(int64_t _id, SourceLocation const& _location, ASTPointer<ASTString> const& _docString):
		Statement(_id, _location, _docString) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;
};

/**
 * The revert statement is used to revert state changes and return error data.
 */
class RevertStatement: public Statement
{
public:
	explicit RevertStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<FunctionCall> _functionCall
	):
		Statement(_id, _location, _docString), m_errorCall(std::move(_functionCall))
	{
		solAssert(m_errorCall, "");
	}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	FunctionCall const& errorCall() const { return *m_errorCall; }

private:
	ASTPointer<FunctionCall> m_errorCall;
};

/**
 * The emit statement is used to emit events: emit EventName(arg1, ..., argn)
 */
class EmitStatement: public Statement
{
public:
	explicit EmitStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<FunctionCall> _functionCall
	):
		Statement(_id, _location, _docString), m_eventCall(std::move(_functionCall)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	FunctionCall const& eventCall() const { return *m_eventCall; }

private:
	ASTPointer<FunctionCall> m_eventCall;
};

/**
 * Definition of one or more variables as a statement inside a function.
 * If multiple variables are declared, a value has to be assigned directly.
 * If only a single variable is declared, the value can be missing.
 * Examples:
 * uint[] memory a; uint a = 2;
 * (uint a, bytes32 b, ) = f(); (, uint a, , StructName storage x) = g();
 */
class VariableDeclarationStatement: public Statement
{
public:
	VariableDeclarationStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		std::vector<ASTPointer<VariableDeclaration>> _variables,
		ASTPointer<Expression> _initialValue
	):
		Statement(_id, _location, _docString), m_variables(std::move(_variables)), m_initialValue(std::move(_initialValue)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& declarations() const { return m_variables; }
	Expression const* initialValue() const { return m_initialValue.get(); }

private:
	/// List of variables, some of which can be empty pointers (unnamed components).
	/// Note that the ``m_value`` member of these is unused. Instead, ``m_initialValue``
	/// below is used, because the initial value can be a single expression assigned
	/// to all variables.
	std::vector<ASTPointer<VariableDeclaration>> m_variables;
	/// The assigned expression / initial value.
	ASTPointer<Expression> m_initialValue;
};

/**
 * A statement that contains only an expression (i.e. an assignment, function call, ...).
 */
class ExpressionStatement: public Statement
{
public:
	ExpressionStatement(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> _expression
	):
		Statement(_id, _location, _docString), m_expression(std::move(_expression)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& expression() const { return *m_expression; }

private:
	ASTPointer<Expression> m_expression;
};

/// @}

/// Expressions
/// @{

/**
 * An expression, i.e. something that has a value (which can also be of type "void" in case
 * of some function calls).
 * @abstract
 */
class Expression: public ASTNode
{
public:
	explicit Expression(int64_t _id, SourceLocation const& _location): ASTNode(_id, _location) {}

	ExpressionAnnotation& annotation() const override;
};

class Conditional: public Expression
{
public:
	Conditional(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _condition,
		ASTPointer<Expression> _trueExpression,
		ASTPointer<Expression> _falseExpression
	):
		Expression(_id, _location),
		m_condition(std::move(_condition)),
		m_trueExpression(std::move(_trueExpression)),
		m_falseExpression(std::move(_falseExpression))
	{}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& condition() const { return *m_condition; }
	Expression const& trueExpression() const { return *m_trueExpression; }
	Expression const& falseExpression() const { return *m_falseExpression; }

private:
	ASTPointer<Expression> m_condition;
	ASTPointer<Expression> m_trueExpression;
	ASTPointer<Expression> m_falseExpression;
};

/// Assignment, can also be a compound assignment.
/// Examples: (a = 7 + 8) or (a *= 2)
class Assignment: public Expression
{
public:
	Assignment(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _leftHandSide,
		Token _assignmentOperator,
		ASTPointer<Expression> _rightHandSide
	):
		Expression(_id, _location),
		m_leftHandSide(std::move(_leftHandSide)),
		m_assigmentOperator(_assignmentOperator),
		m_rightHandSide(std::move(_rightHandSide))
	{
		solAssert(TokenTraits::isAssignmentOp(_assignmentOperator), "");
	}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& leftHandSide() const { return *m_leftHandSide; }
	Token assignmentOperator() const { return m_assigmentOperator; }
	Expression const& rightHandSide() const { return *m_rightHandSide; }

private:
	ASTPointer<Expression> m_leftHandSide;
	Token m_assigmentOperator;
	ASTPointer<Expression> m_rightHandSide;
};


/**
 * Tuple, parenthesized expression, or bracketed expression.
 * Examples: (1, 2), (x,), (x), (), [1, 2],
 * Individual components might be empty shared pointers (as in the second example).
 * The respective types in lvalue context are: 2-tuple, 2-tuple (with wildcard), type of x, 0-tuple
 * Not in lvalue context: 2-tuple, _1_-tuple, type of x, 0-tuple.
 */
class TupleExpression: public Expression
{
public:
	TupleExpression(
		int64_t _id,
		SourceLocation const& _location,
		std::vector<ASTPointer<Expression>> _components,
		bool _isArray
	):
		Expression(_id, _location),
		m_components(std::move(_components)),
		m_isArray(_isArray) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<Expression>> const& components() const { return m_components; }
	bool isInlineArray() const { return m_isArray; }

private:
	std::vector<ASTPointer<Expression>> m_components;
	bool m_isArray;
};

/**
 * Operation involving a unary operator, pre- or postfix.
 * Examples: ++i, delete x or !true
 */
class UnaryOperation: public Expression
{
public:
	UnaryOperation(
		int64_t _id,
		SourceLocation const& _location,
		Token _operator,
		ASTPointer<Expression> _subExpression,
		bool _isPrefix
	):
		Expression(_id, _location),
		m_operator(_operator),
		m_subExpression(std::move(_subExpression)),
		m_isPrefix(_isPrefix)
	{
		solAssert(TokenTraits::isUnaryOp(_operator), "");
	}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Token getOperator() const { return m_operator; }
	bool isPrefixOperation() const { return m_isPrefix; }
	Expression const& subExpression() const { return *m_subExpression; }

private:
	Token m_operator;
	ASTPointer<Expression> m_subExpression;
	bool m_isPrefix;
};

/**
 * Operation involving a binary operator.
 * Examples: 1 + 2, true && false or 1 <= 4
 */
class BinaryOperation: public Expression
{
public:
	BinaryOperation(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _left,
		Token _operator,
		ASTPointer<Expression> _right
	):
		Expression(_id, _location), m_left(std::move(_left)), m_operator(_operator), m_right(std::move(_right))
	{
		solAssert(TokenTraits::isBinaryOp(_operator) || TokenTraits::isCompareOp(_operator), "");
	}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& leftExpression() const { return *m_left; }
	Expression const& rightExpression() const { return *m_right; }
	Token getOperator() const { return m_operator; }

	BinaryOperationAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_left;
	Token m_operator;
	ASTPointer<Expression> m_right;
};

/**
 * Can be ordinary function call, type cast or struct construction.
 */
class FunctionCall: public Expression
{
public:
	FunctionCall(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _expression,
		std::vector<ASTPointer<Expression>> _arguments,
		std::vector<ASTPointer<ASTString>> _names,
		std::vector<SourceLocation> _nameLocations
	):
		Expression(_id, _location), m_expression(std::move(_expression)), m_arguments(std::move(_arguments)), m_names(std::move(_names)), m_nameLocations(std::move(_nameLocations))
	{
		solAssert(m_nameLocations.size() == m_names.size());
	}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& expression() const { return *m_expression; }
	/// @returns the given arguments in the order they were written.
	std::vector<ASTPointer<Expression const>> arguments() const { return {m_arguments.begin(), m_arguments.end()}; }
	/// @returns the given arguments sorted by how the called function takes them.
	std::vector<ASTPointer<Expression const>> sortedArguments() const;
	/// @returns the list of given argument names if this is a named call,
	/// in the order they were written.
	/// If this is not a named call, this is empty.
	std::vector<ASTPointer<ASTString>> const& names() const { return m_names; }
	std::vector<SourceLocation> const& nameLocations() const { return m_nameLocations; }

	FunctionCallAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_expression;
	std::vector<ASTPointer<Expression>> m_arguments;
	std::vector<ASTPointer<ASTString>> m_names;
	std::vector<SourceLocation> m_nameLocations;
};

/**
 * Expression that annotates a function call / a new expression with extra
 * options like gas, value, salt: new SomeContract{salt=123}(params)
 */
class FunctionCallOptions: public Expression
{
public:
	FunctionCallOptions(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _expression,
		std::vector<ASTPointer<Expression>> _options,
		std::vector<ASTPointer<ASTString>> _names
	):
		Expression(_id, _location), m_expression(std::move(_expression)), m_options(std::move(_options)), m_names(std::move(_names)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& expression() const { return *m_expression; }
	std::vector<ASTPointer<Expression const>> options() const { return {m_options.begin(), m_options.end()}; }
	std::vector<ASTPointer<ASTString>> const& names() const { return m_names; }

private:
	ASTPointer<Expression> m_expression;
	std::vector<ASTPointer<Expression>> m_options;
	std::vector<ASTPointer<ASTString>> m_names;

};

/**
 * Expression that creates a new contract or memory-array,
 * e.g. the "new SomeContract" part in "new SomeContract(1, 2)".
 */
class NewExpression: public Expression
{
public:
	NewExpression(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<TypeName> _typeName
	):
		Expression(_id, _location), m_typeName(std::move(_typeName)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	TypeName const& typeName() const { return *m_typeName; }

private:
	ASTPointer<TypeName> m_typeName;
};

/**
 * Access to a member of an object. Example: x.name
 */
class MemberAccess: public Expression
{
public:
	MemberAccess(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _expression,
		ASTPointer<ASTString> _memberName,
		SourceLocation _memberLocation
	):
		Expression(_id, _location),
		m_expression(std::move(_expression)),
		m_memberName(std::move(_memberName)),
		m_memberLocation(std::move(_memberLocation))
	{}

	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;
	Expression const& expression() const { return *m_expression; }
	ASTString const& memberName() const { return *m_memberName; }
	SourceLocation const& memberLocation() const { return m_memberLocation; }

	MemberAccessAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_expression;
	ASTPointer<ASTString> m_memberName;
	SourceLocation m_memberLocation;
};

/**
 * Index access to an array or mapping. Example: a[2]
 */
class IndexAccess: public Expression
{
public:
	IndexAccess(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _base,
		ASTPointer<Expression> _index
	):
		Expression(_id, _location), m_base(std::move(_base)), m_index(std::move(_index)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& baseExpression() const { return *m_base; }
	Expression const* indexExpression() const { return m_index.get(); }

private:
	ASTPointer<Expression> m_base;
	ASTPointer<Expression> m_index;
};

/**
 * Index range access to an array. Example: a[2:3]
 */
class IndexRangeAccess: public Expression
{
public:
	IndexRangeAccess(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<Expression> _base,
		ASTPointer<Expression> _start,
		ASTPointer<Expression> _end
	):
		Expression(_id, _location), m_base(std::move(_base)), m_start(std::move(_start)), m_end(std::move(_end)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Expression const& baseExpression() const { return *m_base; }
	Expression const* startExpression() const { return m_start.get(); }
	Expression const* endExpression() const { return m_end.get(); }

private:
	ASTPointer<Expression> m_base;
	ASTPointer<Expression> m_start;
	ASTPointer<Expression> m_end;
};

/**
 * Primary expression, i.e. an expression that cannot be divided any further. Examples are literals
 * or variable references.
 */
class PrimaryExpression: public Expression
{
public:
	PrimaryExpression(int64_t _id, SourceLocation const& _location): Expression(_id, _location) {}
};

/**
 * An identifier, i.e. a reference to a declaration by name like a variable or function.
 */
class Identifier: public PrimaryExpression
{
public:
	Identifier(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ASTString> _name
	):
		PrimaryExpression(_id, _location), m_name(std::move(_name)) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	ASTString const& name() const { return *m_name; }

	IdentifierAnnotation& annotation() const override;

private:
	ASTPointer<ASTString> m_name;
};

/**
 * An elementary type name expression is used in expressions like "a = uint32(2)" to change the
 * type of an expression explicitly. Here, "uint32" is the elementary type name expression and
 * "uint32(2)" is a @ref FunctionCall.
 */
class ElementaryTypeNameExpression: public PrimaryExpression
{
public:
	ElementaryTypeNameExpression(
		int64_t _id,
		SourceLocation const& _location,
		ASTPointer<ElementaryTypeName> _type
	):
		PrimaryExpression(_id, _location),
		m_type(std::move(_type))
	{
	}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	ElementaryTypeName const& type() const { return *m_type; }

private:
	ASTPointer<ElementaryTypeName> m_type;
};

/**
 * A literal string or number. @see ExpressionCompiler::endVisit() is used to actually parse its value.
 */
class Literal: public PrimaryExpression
{
public:
	enum class SubDenomination
	{
		None = static_cast<int>(Token::Illegal),
		Wei = static_cast<int>(Token::SubWei),
		Gwei = static_cast<int>(Token::SubGwei),
		Ether = static_cast<int>(Token::SubEther),
		Second = static_cast<int>(Token::SubSecond),
		Minute = static_cast<int>(Token::SubMinute),
		Hour = static_cast<int>(Token::SubHour),
		Day = static_cast<int>(Token::SubDay),
		Week = static_cast<int>(Token::SubWeek),
		Year = static_cast<int>(Token::SubYear)
	};
	Literal(
		int64_t _id,
		SourceLocation const& _location,
		Token _token,
		ASTPointer<ASTString> _value,
		SubDenomination _sub = SubDenomination::None
	):
		PrimaryExpression(_id, _location), m_token(_token), m_value(std::move(_value)), m_subDenomination(_sub) {}
	void accept(ASTVisitor& _visitor) override;
	void accept(ASTConstVisitor& _visitor) const override;

	Token token() const { return m_token; }
	/// @returns the non-parsed value of the literal
	ASTString const& value() const { return *m_value; }

	ASTString valueWithoutUnderscores() const;

	SubDenomination subDenomination() const { return m_subDenomination; }

	/// @returns true if this is a number with a hex prefix.
	bool isHexNumber() const;

	/// @returns true if this looks like a checksummed address.
	bool looksLikeAddress() const;
	/// @returns true if it passes the address checksum test.
	bool passesAddressChecksum() const;
	/// @returns the checksummed version of an address (or empty string if not valid)
	std::string getChecksummedAddress() const;

private:
	Token m_token;
	ASTPointer<ASTString> m_value;
	SubDenomination m_subDenomination;
};

/// @}

}
