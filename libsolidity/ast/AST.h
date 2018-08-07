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
 * @date 2014
 * Solidity abstract syntax tree.
 */

#pragma once


#include <libsolidity/ast/ASTForward.h>
#include <libsolidity/parsing/Token.h>
#include <libsolidity/ast/Types.h>
#include <libsolidity/ast/ASTAnnotations.h>
#include <libsolidity/ast/ASTEnums.h>

#include <libevmasm/SourceLocation.h>
#include <libevmasm/Instruction.h>

#include <libdevcore/FixedHash.h>
#include <json/json.h>

#include <boost/noncopyable.hpp>

#include <string>
#include <vector>
#include <memory>

namespace dev
{
namespace solidity
{

class ASTVisitor;
class ASTConstVisitor;


/**
 * The root (abstract) class of the AST inheritance tree.
 * It is possible to traverse all direct and indirect children of an AST node by calling
 * accept, providing an ASTVisitor.
 */
class ASTNode: private boost::noncopyable
{
public:
	explicit ASTNode(SourceLocation const& _location);
	virtual ~ASTNode();

	/// @returns an identifier of this AST node that is unique for a single compilation run.
	size_t id() const { return m_id; }
	/// Resets the global ID counter. This invalidates all previous IDs.
	static void resetID();

	virtual void accept(ASTVisitor& _visitor) = 0;
	virtual void accept(ASTConstVisitor& _visitor) const = 0;
	template <class T>
	static void listAccept(std::vector<T> const& _list, ASTVisitor& _visitor)
	{
		for (T const& element: _list)
			element->accept(_visitor);
	}
	template <class T>
	static void listAccept(std::vector<T> const& _list, ASTConstVisitor& _visitor)
	{
		for (T const& element: _list)
			element->accept(_visitor);
	}

	/// @returns a copy of the vector containing only the nodes which derive from T.
	template <class _T>
	static std::vector<_T const*> filteredNodes(std::vector<ASTPointer<ASTNode>> const& _nodes);

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
	/// Annotation - is specialised in derived classes, is created upon request (because of polymorphism).
	mutable ASTAnnotation* m_annotation = nullptr;

private:
	SourceLocation m_location;
};

template <class _T>
std::vector<_T const*> ASTNode::filteredNodes(std::vector<ASTPointer<ASTNode>> const& _nodes)
{
	std::vector<_T const*> ret;
	for (auto const& n: _nodes)
		if (auto const* nt = dynamic_cast<_T const*>(n.get()))
			ret.push_back(nt);
	return ret;
}

/**
 * Source unit containing import directives and contract definitions.
 */
class SourceUnit: public ASTNode
{
public:
	SourceUnit(SourceLocation const& _location, std::vector<ASTPointer<ASTNode>> const& _nodes):
		ASTNode(_location), m_nodes(_nodes) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual SourceUnitAnnotation& annotation() const override;

	std::vector<ASTPointer<ASTNode>> nodes() const { return m_nodes; }

	/// @returns a set of referenced SourceUnits. Recursively if @a _recurse is true.
	std::set<SourceUnit const*> referencedSourceUnits(bool _recurse = false, std::set<SourceUnit const*> _skipList = std::set<SourceUnit const*>()) const;

private:
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
	ASTNode const* scope() const { return m_scope; }
	void setScope(ASTNode const* _scope) { m_scope = _scope; }

	/// @returns the source unit this scopable is present in.
	SourceUnit const& sourceUnit() const;

	/// @returns the source name this scopable is present in.
	/// Can be combined with annotation().canonicalName (if present) to form a globally unique name.
	std::string sourceUnitName() const;

protected:
	ASTNode const* m_scope = nullptr;
};

/**
 * Abstract AST class for a declaration (contract, function, struct, variable, import directive).
 */
class Declaration: public ASTNode, public Scopable
{
public:
	/// Visibility ordered from restricted to unrestricted.
	enum class Visibility { Default, Private, Internal, Public, External };

	static std::string visibilityToString(Declaration::Visibility _visibility)
	{
		switch(_visibility)
		{
		case Declaration::Visibility::Public:
			return "public";
		case Declaration::Visibility::Internal:
			return "internal";
		case Declaration::Visibility::Private:
			return "private";
		case Declaration::Visibility::External:
			return "external";
		default:
			solAssert(false, "Invalid visibility specifier.");
		}
		return std::string();
	}

	Declaration(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		Visibility _visibility = Visibility::Default
	):
		ASTNode(_location), m_name(_name), m_visibility(_visibility) {}

	/// @returns the declared name.
	ASTString const& name() const { return *m_name; }
	bool noVisibilitySpecified() const { return m_visibility == Visibility::Default; }
	Visibility visibility() const { return m_visibility == Visibility::Default ? defaultVisibility() : m_visibility; }
	bool isPublic() const { return visibility() >= Visibility::Public; }
	virtual bool isVisibleInContract() const { return visibility() != Visibility::External; }
	bool isVisibleInDerivedContracts() const { return isVisibleInContract() && visibility() >= Visibility::Internal; }
	bool isVisibleAsLibraryMember() const { return visibility() >= Visibility::Internal; }


	virtual bool isLValue() const { return false; }
	virtual bool isPartOfExternalInterface() const { return false; }

	/// @returns the type of expressions referencing this declaration.
	/// The current contract has to be given since this context can change the type, especially of
	/// contract types.
	/// This can only be called once types of variable declarations have already been resolved.
	virtual TypePointer type() const = 0;

	/// @param _internal false indicates external interface is concerned, true indicates internal interface is concerned.
	/// @returns null when it is not accessible as a function.
	virtual FunctionTypePointer functionType(bool /*_internal*/) const { return {}; }

protected:
	virtual Visibility defaultVisibility() const { return Visibility::Public; }

private:
	ASTPointer<ASTString> m_name;
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
		SourceLocation const& _location,
		std::vector<Token::Value> const& _tokens,
		std::vector<ASTString> const& _literals
	): ASTNode(_location), m_tokens(_tokens), m_literals(_literals)
	{}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<Token::Value> const& tokens() const { return m_tokens; }
	std::vector<ASTString> const& literals() const { return m_literals; }

private:

	/// Sequence of tokens following the "pragma" keyword.
	std::vector<Token::Value> m_tokens;
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
	ImportDirective(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _path,
		ASTPointer<ASTString> const& _unitAlias,
		std::vector<std::pair<ASTPointer<Identifier>, ASTPointer<ASTString>>>&& _symbolAliases
	):
		Declaration(_location, _unitAlias),
		m_path(_path),
		m_symbolAliases(_symbolAliases)
	{ }

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ASTString const& path() const { return *m_path; }
	std::vector<std::pair<ASTPointer<Identifier>, ASTPointer<ASTString>>> const& symbolAliases() const
	{
		return m_symbolAliases;
	}
	virtual ImportAnnotation& annotation() const override;

	virtual TypePointer type() const override;

private:
	ASTPointer<ASTString> m_path;
	/// The aliases for the specific symbols to import. If non-empty import the specific symbols.
	/// If the second component is empty, import the identifier unchanged.
	/// If both m_unitAlias and m_symbolAlias are empty, import all symbols into the current scope.
	std::vector<std::pair<ASTPointer<Identifier>, ASTPointer<ASTString>>> m_symbolAliases;
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
 * Abstract class that is added to each AST node that can receive documentation.
 */
class Documented
{
public:
	virtual ~Documented() = default;
	explicit Documented(ASTPointer<ASTString> const& _documentation): m_documentation(_documentation) {}

	/// @return A shared pointer of an ASTString.
	/// Can contain a nullptr in which case indicates absence of documentation
	ASTPointer<ASTString> const& documentation() const { return m_documentation; }

protected:
	ASTPointer<ASTString> m_documentation;
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
class ContractDefinition: public Declaration, public Documented
{
public:
	enum class ContractKind { Interface, Contract, Library };

	ContractDefinition(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		ASTPointer<ASTString> const& _documentation,
		std::vector<ASTPointer<InheritanceSpecifier>> const& _baseContracts,
		std::vector<ASTPointer<ASTNode>> const& _subNodes,
		ContractKind _contractKind = ContractKind::Contract
	):
		Declaration(_location, _name),
		Documented(_documentation),
		m_baseContracts(_baseContracts),
		m_subNodes(_subNodes),
		m_contractKind(_contractKind)
	{}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<InheritanceSpecifier>> const& baseContracts() const { return m_baseContracts; }
	std::vector<ASTPointer<ASTNode>> const& subNodes() const { return m_subNodes; }
	std::vector<UsingForDirective const*> usingForDirectives() const { return filteredNodes<UsingForDirective>(m_subNodes); }
	std::vector<StructDefinition const*> definedStructs() const { return filteredNodes<StructDefinition>(m_subNodes); }
	std::vector<EnumDefinition const*> definedEnums() const { return filteredNodes<EnumDefinition>(m_subNodes); }
	std::vector<VariableDeclaration const*> stateVariables() const { return filteredNodes<VariableDeclaration>(m_subNodes); }
	std::vector<ModifierDefinition const*> functionModifiers() const { return filteredNodes<ModifierDefinition>(m_subNodes); }
	std::vector<FunctionDefinition const*> definedFunctions() const { return filteredNodes<FunctionDefinition>(m_subNodes); }
	std::vector<EventDefinition const*> events() const { return filteredNodes<EventDefinition>(m_subNodes); }
	std::vector<EventDefinition const*> const& interfaceEvents() const;
	bool isLibrary() const { return m_contractKind == ContractKind::Library; }

	/// @returns a map of canonical function signatures to FunctionDefinitions
	/// as intended for use by the ABI.
	std::map<FixedHash<4>, FunctionTypePointer> interfaceFunctions() const;
	std::vector<std::pair<FixedHash<4>, FunctionTypePointer>> const& interfaceFunctionList() const;

	/// @returns a list of the inheritable members of this contract
	std::vector<Declaration const*> const& inheritableMembers() const;

	/// Returns the constructor or nullptr if no constructor was specified.
	FunctionDefinition const* constructor() const;
	/// @returns true iff the constructor of this contract is public (or non-existing).
	bool constructorIsPublic() const;
	/// Returns the fallback function or nullptr if no fallback function was specified.
	FunctionDefinition const* fallbackFunction() const;

	std::string fullyQualifiedName() const { return sourceUnitName() + ":" + name(); }

	virtual TypePointer type() const override;

	virtual ContractDefinitionAnnotation& annotation() const override;

	ContractKind contractKind() const { return m_contractKind; }

private:
	std::vector<ASTPointer<InheritanceSpecifier>> m_baseContracts;
	std::vector<ASTPointer<ASTNode>> m_subNodes;
	ContractKind m_contractKind;

	std::vector<ContractDefinition const*> m_linearizedBaseContracts;
	mutable std::unique_ptr<std::vector<std::pair<FixedHash<4>, FunctionTypePointer>>> m_interfaceFunctionList;
	mutable std::unique_ptr<std::vector<EventDefinition const*>> m_interfaceEvents;
	mutable std::unique_ptr<std::vector<Declaration const*>> m_inheritableMembers;
};

class InheritanceSpecifier: public ASTNode
{
public:
	InheritanceSpecifier(
		SourceLocation const& _location,
		ASTPointer<UserDefinedTypeName> const& _baseName,
		std::unique_ptr<std::vector<ASTPointer<Expression>>> _arguments
	):
		ASTNode(_location), m_baseName(_baseName), m_arguments(std::move(_arguments)) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	UserDefinedTypeName const& name() const { return *m_baseName; }
	// Returns nullptr if no argument list was given (``C``).
	// If an argument list is given (``C(...)``), the arguments are returned
	// as a vector of expressions. Note that this vector can be empty (``C()``).
	std::vector<ASTPointer<Expression>> const* arguments() const { return m_arguments.get(); }

private:
	ASTPointer<UserDefinedTypeName> m_baseName;
	std::unique_ptr<std::vector<ASTPointer<Expression>>> m_arguments;
};

/**
 * `using LibraryName for uint` will attach all functions from the library LibraryName
 * to `uint` if the first parameter matches the type. `using LibraryName for *` attaches
 * the function to any matching type.
 */
class UsingForDirective: public ASTNode
{
public:
	UsingForDirective(
		SourceLocation const& _location,
		ASTPointer<UserDefinedTypeName> const& _libraryName,
		ASTPointer<TypeName> const& _typeName
	):
		ASTNode(_location), m_libraryName(_libraryName), m_typeName(_typeName) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	UserDefinedTypeName const& libraryName() const { return *m_libraryName; }
	/// @returns the type name the library is attached to, null for `*`.
	TypeName const* typeName() const { return m_typeName.get(); }

private:
	ASTPointer<UserDefinedTypeName> m_libraryName;
	ASTPointer<TypeName> m_typeName;
};

class StructDefinition: public Declaration
{
public:
	StructDefinition(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		std::vector<ASTPointer<VariableDeclaration>> const& _members
	):
		Declaration(_location, _name), m_members(_members) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& members() const { return m_members; }

	virtual TypePointer type() const override;

	virtual TypeDeclarationAnnotation& annotation() const override;

private:
	std::vector<ASTPointer<VariableDeclaration>> m_members;
};

class EnumDefinition: public Declaration
{
public:
	EnumDefinition(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		std::vector<ASTPointer<EnumValue>> const& _members
	):
		Declaration(_location, _name), m_members(_members) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<EnumValue>> const& members() const { return m_members; }

	virtual TypePointer type() const override;

	virtual TypeDeclarationAnnotation& annotation() const override;

private:
	std::vector<ASTPointer<EnumValue>> m_members;
};

/**
 * Declaration of an Enum Value
 */
class EnumValue: public Declaration
{
public:
	EnumValue(SourceLocation const& _location, ASTPointer<ASTString> const& _name):
		Declaration(_location, _name) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	virtual TypePointer type() const override;
};

/**
 * Parameter list, used as function parameter list and return list.
 * None of the parameters is allowed to contain mappings (not even recursively
 * inside structs).
 */
class ParameterList: public ASTNode
{
public:
	ParameterList(
		SourceLocation const& _location,
		std::vector<ASTPointer<VariableDeclaration>> const& _parameters
	):
		ASTNode(_location), m_parameters(_parameters) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& parameters() const { return m_parameters; }

private:
	std::vector<ASTPointer<VariableDeclaration>> m_parameters;
};

/**
 * Base class for all nodes that define function-like objects, i.e. FunctionDefinition,
 * EventDefinition and ModifierDefinition.
 */
class CallableDeclaration: public Declaration, public VariableScope
{
public:
	CallableDeclaration(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		Declaration::Visibility _visibility,
		ASTPointer<ParameterList> const& _parameters,
		ASTPointer<ParameterList> const& _returnParameters = ASTPointer<ParameterList>()
	):
		Declaration(_location, _name, _visibility),
		m_parameters(_parameters),
		m_returnParameters(_returnParameters)
	{
	}

	std::vector<ASTPointer<VariableDeclaration>> const& parameters() const { return m_parameters->parameters(); }
	ParameterList const& parameterList() const { return *m_parameters; }
	ASTPointer<ParameterList> const& returnParameterList() const { return m_returnParameters; }

protected:
	ASTPointer<ParameterList> m_parameters;
	ASTPointer<ParameterList> m_returnParameters;
};

class FunctionDefinition: public CallableDeclaration, public Documented, public ImplementationOptional
{
public:
	FunctionDefinition(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		Declaration::Visibility _visibility,
		StateMutability _stateMutability,
		bool _isConstructor,
		ASTPointer<ASTString> const& _documentation,
		ASTPointer<ParameterList> const& _parameters,
		std::vector<ASTPointer<ModifierInvocation>> const& _modifiers,
		ASTPointer<ParameterList> const& _returnParameters,
		ASTPointer<Block> const& _body
	):
		CallableDeclaration(_location, _name, _visibility, _parameters, _returnParameters),
		Documented(_documentation),
		ImplementationOptional(_body != nullptr),
		m_stateMutability(_stateMutability),
		m_isConstructor(_isConstructor),
		m_functionModifiers(_modifiers),
		m_body(_body)
	{}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	StateMutability stateMutability() const { return m_stateMutability; }
	bool isConstructor() const { return m_isConstructor; }
	bool isFallback() const { return !m_isConstructor && name().empty(); }
	bool isPayable() const { return m_stateMutability == StateMutability::Payable; }
	std::vector<ASTPointer<ModifierInvocation>> const& modifiers() const { return m_functionModifiers; }
	std::vector<ASTPointer<VariableDeclaration>> const& returnParameters() const { return m_returnParameters->parameters(); }
	Block const& body() const { solAssert(m_body, ""); return *m_body; }
	virtual bool isVisibleInContract() const override
	{
		return Declaration::isVisibleInContract() && !isConstructor() && !isFallback();
	}
	virtual bool isPartOfExternalInterface() const override { return isPublic() && !isConstructor() && !isFallback(); }

	/// @returns the external signature of the function
	/// That consists of the name of the function followed by the types of the
	/// arguments separated by commas all enclosed in parentheses without any spaces.
	std::string externalSignature() const;

	ContractDefinition::ContractKind inContractKind() const;

	virtual TypePointer type() const override;

	/// @param _internal false indicates external interface is concerned, true indicates internal interface is concerned.
	/// @returns null when it is not accessible as a function.
	virtual FunctionTypePointer functionType(bool /*_internal*/) const override;

	virtual FunctionDefinitionAnnotation& annotation() const override;

private:
	StateMutability m_stateMutability;
	bool m_isConstructor;
	std::vector<ASTPointer<ModifierInvocation>> m_functionModifiers;
	ASTPointer<Block> m_body;
};

/**
 * Declaration of a variable. This can be used in various places, e.g. in function parameter
 * lists, struct definitions and even function bodies.
 */
class VariableDeclaration: public Declaration
{
public:
	enum Location { Default, Storage, Memory, CallData };

	VariableDeclaration(
		SourceLocation const& _sourceLocation,
		ASTPointer<TypeName> const& _type,
		ASTPointer<ASTString> const& _name,
		ASTPointer<Expression> _value,
		Visibility _visibility,
		bool _isStateVar = false,
		bool _isIndexed = false,
		bool _isConstant = false,
		Location _referenceLocation = Location::Default
	):
		Declaration(_sourceLocation, _name, _visibility),
		m_typeName(_type),
		m_value(_value),
		m_isStateVariable(_isStateVar),
		m_isIndexed(_isIndexed),
		m_isConstant(_isConstant),
		m_location(_referenceLocation) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	TypeName* typeName() const { return m_typeName.get(); }
	ASTPointer<Expression> const& value() const { return m_value; }

	virtual bool isLValue() const override;
	virtual bool isPartOfExternalInterface() const override { return isPublic(); }

	bool isLocalVariable() const;
	/// @returns true if this variable is a parameter or return parameter of a function.
	bool isCallableParameter() const;
	/// @returns true if this variable is a return parameter of a function.
	bool isReturnParameter() const;
	/// @returns true if this variable is a local variable or return parameter.
	bool isLocalOrReturn() const;
	/// @returns true if this variable is a parameter (not return parameter) of an external function.
	bool isExternalCallableParameter() const;
	/// @returns true if the type of the variable does not need to be specified, i.e. it is declared
	/// in the body of a function or modifier.
	bool isStateVariable() const { return m_isStateVariable; }
	bool isIndexed() const { return m_isIndexed; }
	bool isConstant() const { return m_isConstant; }
	Location referenceLocation() const { return m_location; }

	virtual TypePointer type() const override;

	/// @param _internal false indicates external interface is concerned, true indicates internal interface is concerned.
	/// @returns null when it is not accessible as a function.
	virtual FunctionTypePointer functionType(bool /*_internal*/) const override;

	virtual VariableDeclarationAnnotation& annotation() const override;

protected:
	Visibility defaultVisibility() const override { return Visibility::Internal; }

private:
	ASTPointer<TypeName> m_typeName; ///< can be empty ("var")
	/// Initially assigned value, can be missing. For local variables, this is stored inside
	/// VariableDeclarationStatement and not here.
	ASTPointer<Expression> m_value;
	bool m_isStateVariable; ///< Whether or not this is a contract state variable
	bool m_isIndexed; ///< Whether this is an indexed variable (used by events).
	bool m_isConstant; ///< Whether the variable is a compile-time constant.
	Location m_location; ///< Location of the variable if it is of reference type.
};

/**
 * Definition of a function modifier.
 */
class ModifierDefinition: public CallableDeclaration, public Documented
{
public:
	ModifierDefinition(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		ASTPointer<ASTString> const& _documentation,
		ASTPointer<ParameterList> const& _parameters,
		ASTPointer<Block> const& _body
	):
		CallableDeclaration(_location, _name, Visibility::Internal, _parameters),
		Documented(_documentation),
		m_body(_body)
	{
	}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Block const& body() const { return *m_body; }

	virtual TypePointer type() const override;

	virtual ModifierDefinitionAnnotation& annotation() const override;

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
		SourceLocation const& _location,
		ASTPointer<Identifier> const& _name,
		std::unique_ptr<std::vector<ASTPointer<Expression>>> _arguments
	):
		ASTNode(_location), m_modifierName(_name), m_arguments(std::move(_arguments)) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ASTPointer<Identifier> const& name() const { return m_modifierName; }
	// Returns nullptr if no argument list was given (``mod``).
	// If an argument list is given (``mod(...)``), the arguments are returned
	// as a vector of expressions. Note that this vector can be empty (``mod()``).
	std::vector<ASTPointer<Expression>> const* arguments() const { return m_arguments.get(); }

private:
	ASTPointer<Identifier> m_modifierName;
	std::unique_ptr<std::vector<ASTPointer<Expression>>> m_arguments;
};

/**
 * Definition of a (loggable) event.
 */
class EventDefinition: public CallableDeclaration, public Documented
{
public:
	EventDefinition(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		ASTPointer<ASTString> const& _documentation,
		ASTPointer<ParameterList> const& _parameters,
		bool _anonymous = false
	):
		CallableDeclaration(_location, _name, Visibility::Default, _parameters),
		Documented(_documentation),
		m_anonymous(_anonymous)
	{
	}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	bool isAnonymous() const { return m_anonymous; }

	virtual TypePointer type() const override;
	virtual FunctionTypePointer functionType(bool /*_internal*/) const override;

	virtual EventDefinitionAnnotation& annotation() const override;

private:
	bool m_anonymous = false;
};

/**
 * Pseudo AST node that is used as declaration for "this", "msg", "tx", "block" and the global
 * functions when such an identifier is encountered. Will never have a valid location in the source code.
 */
class MagicVariableDeclaration: public Declaration
{
public:
	MagicVariableDeclaration(ASTString const& _name, std::shared_ptr<Type const> const& _type):
		Declaration(SourceLocation(), std::make_shared<ASTString>(_name)), m_type(_type) {}
	virtual void accept(ASTVisitor&) override
	{
		solAssert(false, "MagicVariableDeclaration used inside real AST.");
	}
	virtual void accept(ASTConstVisitor&) const override
	{
		solAssert(false, "MagicVariableDeclaration used inside real AST.");
	}

	virtual FunctionTypePointer functionType(bool) const override
	{
		solAssert(m_type->category() == Type::Category::Function, "");
		return std::dynamic_pointer_cast<FunctionType const>(m_type);
	}
	virtual TypePointer type() const override { return m_type; }

private:
	std::shared_ptr<Type const> m_type;
};

/// Types
/// @{

/**
 * Abstract base class of a type name, can be any built-in or user-defined type.
 */
class TypeName: public ASTNode
{
protected:
	explicit TypeName(SourceLocation const& _location): ASTNode(_location) {}

public:
	virtual TypeNameAnnotation& annotation() const override;
};

/**
 * Any pre-defined type name represented by a single keyword, i.e. it excludes mappings,
 * contracts, functions, etc.
 */
class ElementaryTypeName: public TypeName
{
public:
	ElementaryTypeName(SourceLocation const& _location, ElementaryTypeNameToken const& _elem):
		TypeName(_location), m_type(_elem)
	{}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ElementaryTypeNameToken const& typeName() const { return m_type; }

private:
	ElementaryTypeNameToken m_type;
};

/**
 * Name referring to a user-defined type (i.e. a struct, contract, etc.).
 */
class UserDefinedTypeName: public TypeName
{
public:
	UserDefinedTypeName(SourceLocation const& _location, std::vector<ASTString> const& _namePath):
		TypeName(_location), m_namePath(_namePath) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTString> const& namePath() const { return m_namePath; }

	virtual UserDefinedTypeNameAnnotation& annotation() const override;

private:
	std::vector<ASTString> m_namePath;
};

/**
 * A literal function type. Its source form is "function (paramType1, paramType2) internal / external returns (retType1, retType2)"
 */
class FunctionTypeName: public TypeName
{
public:
	FunctionTypeName(
		SourceLocation const& _location,
		ASTPointer<ParameterList> const& _parameterTypes,
		ASTPointer<ParameterList> const& _returnTypes,
		Declaration::Visibility _visibility,
		StateMutability _stateMutability
	):
		TypeName(_location), m_parameterTypes(_parameterTypes), m_returnTypes(_returnTypes),
		m_visibility(_visibility), m_stateMutability(_stateMutability)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& parameterTypes() const { return m_parameterTypes->parameters(); }
	std::vector<ASTPointer<VariableDeclaration>> const& returnParameterTypes() const { return m_returnTypes->parameters(); }
	ASTPointer<ParameterList> const& parameterTypeList() const { return m_parameterTypes; }
	ASTPointer<ParameterList> const& returnParameterTypeList() const { return m_returnTypes; }

	Declaration::Visibility visibility() const
	{
		return m_visibility == Declaration::Visibility::Default ? Declaration::Visibility::Internal : m_visibility;
	}
	StateMutability stateMutability() const { return m_stateMutability; }
	bool isPayable() const { return m_stateMutability == StateMutability::Payable; }

private:
	ASTPointer<ParameterList> m_parameterTypes;
	ASTPointer<ParameterList> m_returnTypes;
	Declaration::Visibility m_visibility;
	StateMutability m_stateMutability;
};

/**
 * A mapping type. Its source form is "mapping('keyType' => 'valueType')"
 */
class Mapping: public TypeName
{
public:
	Mapping(
		SourceLocation const& _location,
		ASTPointer<ElementaryTypeName> const& _keyType,
		ASTPointer<TypeName> const& _valueType
	):
		TypeName(_location), m_keyType(_keyType), m_valueType(_valueType) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ElementaryTypeName const& keyType() const { return *m_keyType; }
	TypeName const& valueType() const { return *m_valueType; }

private:
	ASTPointer<ElementaryTypeName> m_keyType;
	ASTPointer<TypeName> m_valueType;
};

/**
 * An array type, can be "typename[]" or "typename[<expression>]".
 */
class ArrayTypeName: public TypeName
{
public:
	ArrayTypeName(
		SourceLocation const& _location,
		ASTPointer<TypeName> const& _baseType,
		ASTPointer<Expression> const& _length
	):
		TypeName(_location), m_baseType(_baseType), m_length(_length) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString
	): ASTNode(_location), Documented(_docString) {}

	virtual StatementAnnotation& annotation() const override;
};

namespace assembly
{
// Forward-declaration to AsmData.h
struct Block;
}

/**
 * Inline assembly.
 */
class InlineAssembly: public Statement
{
public:
	InlineAssembly(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		std::shared_ptr<assembly::Block> const& _operations
	):
		Statement(_location, _docString), m_operations(_operations) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	assembly::Block const& operations() const { return *m_operations; }

	virtual InlineAssemblyAnnotation& annotation() const override;

private:
	std::shared_ptr<assembly::Block> m_operations;
};

/**
 * Brace-enclosed block containing zero or more statements.
 */
class Block: public Statement, public Scopable
{
public:
	Block(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		std::vector<ASTPointer<Statement>> const& _statements
	):
		Statement(_location, _docString), m_statements(_statements) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<Statement>> const& statements() const { return m_statements; }

private:
	std::vector<ASTPointer<Statement>> m_statements;
};

/**
 * Special placeholder statement denoted by "_" used in function modifiers. This is replaced by
 * the original function when the modifier is applied.
 */
class PlaceholderStatement: public Statement
{
public:
	explicit PlaceholderStatement(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString
	): Statement(_location, _docString) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
};

/**
 * If-statement with an optional "else" part. Note that "else if" is modeled by having a new
 * if-statement as the false (else) body.
 */
class IfStatement: public Statement
{
public:
	IfStatement(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> const& _condition,
		ASTPointer<Statement> const& _trueBody,
		ASTPointer<Statement> const& _falseBody
	):
		Statement(_location, _docString),
		m_condition(_condition),
		m_trueBody(_trueBody),
		m_falseBody(_falseBody)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
 * Statement in which a break statement is legal (abstract class).
 */
class BreakableStatement: public Statement
{
public:
	explicit BreakableStatement(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString
	): Statement(_location, _docString) {}
};

class WhileStatement: public BreakableStatement
{
public:
	WhileStatement(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> const& _condition,
		ASTPointer<Statement> const& _body,
		bool _isDoWhile
	):
		BreakableStatement(_location, _docString), m_condition(_condition), m_body(_body),
		m_isDoWhile(_isDoWhile) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
class ForStatement: public BreakableStatement, public Scopable
{
public:
	ForStatement(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Statement> const& _initExpression,
		ASTPointer<Expression> const& _conditionExpression,
		ASTPointer<ExpressionStatement> const& _loopExpression,
		ASTPointer<Statement> const& _body
	):
		BreakableStatement(_location, _docString),
		m_initExpression(_initExpression),
		m_condExpression(_conditionExpression),
		m_loopExpression(_loopExpression),
		m_body(_body)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Statement const* initializationExpression() const { return m_initExpression.get(); }
	Expression const* condition() const { return m_condExpression.get(); }
	ExpressionStatement const* loopExpression() const { return m_loopExpression.get(); }
	Statement const& body() const { return *m_body; }

private:
	/// For statement's initialization expression. for(XXX; ; ). Can be empty
	ASTPointer<Statement> m_initExpression;
	/// For statement's condition expression. for(; XXX ; ). Can be empty
	ASTPointer<Expression> m_condExpression;
	/// For statement's loop expression. for(;;XXX). Can be empty
	ASTPointer<ExpressionStatement> m_loopExpression;
	/// The body of the loop
	ASTPointer<Statement> m_body;
};

class Continue: public Statement
{
public:
	explicit Continue(SourceLocation const& _location, 	ASTPointer<ASTString> const& _docString):
		Statement(_location, _docString) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
};

class Break: public Statement
{
public:
	explicit Break(SourceLocation const& _location, ASTPointer<ASTString> const& _docString):
		Statement(_location, _docString) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
};

class Return: public Statement
{
public:
	Return(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> _expression
	): Statement(_location, _docString), m_expression(_expression) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Expression const* expression() const { return m_expression.get(); }

	virtual ReturnAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_expression; ///< value to return, optional
};

/**
 * @brief The Throw statement to throw that triggers a solidity exception(jump to ErrorTag)
 */
class Throw: public Statement
{
public:
	explicit Throw(SourceLocation const& _location, ASTPointer<ASTString> const& _docString):
		Statement(_location, _docString) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
};

/**
 * The emit statement is used to emit events: emit EventName(arg1, ..., argn)
 */
class EmitStatement: public Statement
{
public:
	explicit EmitStatement(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<FunctionCall> const& _functionCall
	):
		Statement(_location, _docString), m_eventCall(_functionCall) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		std::vector<ASTPointer<VariableDeclaration>> const& _variables,
		ASTPointer<Expression> const& _initialValue
	):
		Statement(_location, _docString), m_variables(_variables), m_initialValue(_initialValue) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _docString,
		ASTPointer<Expression> _expression
	):
		Statement(_location, _docString), m_expression(_expression) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
	explicit Expression(SourceLocation const& _location): ASTNode(_location) {}

	ExpressionAnnotation& annotation() const override;
};

class Conditional: public Expression
{
public:
	Conditional(
		SourceLocation const& _location,
		ASTPointer<Expression> const& _condition,
		ASTPointer<Expression> const& _trueExpression,
		ASTPointer<Expression> const& _falseExpression
	):
		Expression(_location),
		m_condition(_condition),
		m_trueExpression(_trueExpression),
		m_falseExpression(_falseExpression)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
		SourceLocation const& _location,
		ASTPointer<Expression> const& _leftHandSide,
		Token::Value _assignmentOperator,
		ASTPointer<Expression> const& _rightHandSide
	):
		Expression(_location),
		m_leftHandSide(_leftHandSide),
		m_assigmentOperator(_assignmentOperator),
		m_rightHandSide(_rightHandSide)
	{
		solAssert(Token::isAssignmentOp(_assignmentOperator), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Expression const& leftHandSide() const { return *m_leftHandSide; }
	Token::Value assignmentOperator() const { return m_assigmentOperator; }
	Expression const& rightHandSide() const { return *m_rightHandSide; }

private:
	ASTPointer<Expression> m_leftHandSide;
	Token::Value m_assigmentOperator;
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
		SourceLocation const& _location,
		std::vector<ASTPointer<Expression>> const& _components,
		bool _isArray
	):
		Expression(_location),
		m_components(_components),
		m_isArray(_isArray) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
		SourceLocation const& _location,
		Token::Value _operator,
		ASTPointer<Expression> const& _subExpression,
		bool _isPrefix
	):
		Expression(_location),
		m_operator(_operator),
		m_subExpression(_subExpression),
		m_isPrefix(_isPrefix)
	{
		solAssert(Token::isUnaryOp(_operator), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Token::Value getOperator() const { return m_operator; }
	bool isPrefixOperation() const { return m_isPrefix; }
	Expression const& subExpression() const { return *m_subExpression; }

private:
	Token::Value m_operator;
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
		SourceLocation const& _location,
		ASTPointer<Expression> const& _left,
		Token::Value _operator,
		ASTPointer<Expression> const& _right
	):
		Expression(_location), m_left(_left), m_operator(_operator), m_right(_right)
	{
		solAssert(Token::isBinaryOp(_operator) || Token::isCompareOp(_operator), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Expression const& leftExpression() const { return *m_left; }
	Expression const& rightExpression() const { return *m_right; }
	Token::Value getOperator() const { return m_operator; }

	BinaryOperationAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_left;
	Token::Value m_operator;
	ASTPointer<Expression> m_right;
};

/**
 * Can be ordinary function call, type cast or struct construction.
 */
class FunctionCall: public Expression
{
public:
	FunctionCall(
		SourceLocation const& _location,
		ASTPointer<Expression> const& _expression,
		std::vector<ASTPointer<Expression>> const& _arguments,
		std::vector<ASTPointer<ASTString>> const& _names
	):
		Expression(_location), m_expression(_expression), m_arguments(_arguments), m_names(_names) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Expression const& expression() const { return *m_expression; }
	std::vector<ASTPointer<Expression const>> arguments() const { return {m_arguments.begin(), m_arguments.end()}; }
	std::vector<ASTPointer<ASTString>> const& names() const { return m_names; }

	virtual FunctionCallAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_expression;
	std::vector<ASTPointer<Expression>> m_arguments;
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
		SourceLocation const& _location,
		ASTPointer<TypeName> const& _typeName
	):
		Expression(_location), m_typeName(_typeName) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

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
		SourceLocation const& _location,
		ASTPointer<Expression> _expression,
		ASTPointer<ASTString> const& _memberName
	):
		Expression(_location), m_expression(_expression), m_memberName(_memberName) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	Expression const& expression() const { return *m_expression; }
	ASTString const& memberName() const { return *m_memberName; }

	virtual MemberAccessAnnotation& annotation() const override;

private:
	ASTPointer<Expression> m_expression;
	ASTPointer<ASTString> m_memberName;
};

/**
 * Index access to an array. Example: a[2]
 */
class IndexAccess: public Expression
{
public:
	IndexAccess(
		SourceLocation const& _location,
		ASTPointer<Expression> const& _base,
		ASTPointer<Expression> const& _index
	):
		Expression(_location), m_base(_base), m_index(_index) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Expression const& baseExpression() const { return *m_base; }
	Expression const* indexExpression() const { return m_index.get(); }

private:
	ASTPointer<Expression> m_base;
	ASTPointer<Expression> m_index;
};

/**
 * Primary expression, i.e. an expression that cannot be divided any further. Examples are literals
 * or variable references.
 */
class PrimaryExpression: public Expression
{
public:
	PrimaryExpression(SourceLocation const& _location): Expression(_location) {}
};

/**
 * An identifier, i.e. a reference to a declaration by name like a variable or function.
 */
class Identifier: public PrimaryExpression
{
public:
	Identifier(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name
	):
		PrimaryExpression(_location), m_name(_name) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ASTString const& name() const { return *m_name; }

	virtual IdentifierAnnotation& annotation() const override;

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
	ElementaryTypeNameExpression(SourceLocation const& _location, ElementaryTypeNameToken const& _type):
		PrimaryExpression(_location), m_typeToken(_type)
	{}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ElementaryTypeNameToken const& typeName() const { return m_typeToken; }

private:
	ElementaryTypeNameToken m_typeToken;
};

/**
 * A literal string or number. @see ExpressionCompiler::endVisit() is used to actually parse its value.
 */
class Literal: public PrimaryExpression
{
public:
	enum class SubDenomination
	{
		None = Token::Illegal,
		Wei = Token::SubWei,
		Szabo = Token::SubSzabo,
		Finney = Token::SubFinney,
		Ether = Token::SubEther,
		Second = Token::SubSecond,
		Minute = Token::SubMinute,
		Hour = Token::SubHour,
		Day = Token::SubDay,
		Week = Token::SubWeek,
		Year = Token::SubYear
	};
	Literal(
		SourceLocation const& _location,
		Token::Value _token,
		ASTPointer<ASTString> const& _value,
		SubDenomination _sub = SubDenomination::None
	):
		PrimaryExpression(_location), m_token(_token), m_value(_value), m_subDenomination(_sub) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Token::Value token() const { return m_token; }
	/// @returns the non-parsed value of the literal
	ASTString const& value() const { return *m_value; }

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
	Token::Value m_token;
	ASTPointer<ASTString> m_value;
	SubDenomination m_subDenomination;
};

/// @}


}
}
