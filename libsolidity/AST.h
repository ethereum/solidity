/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity abstract syntax tree.
 */

#pragma once


#include <string>
#include <vector>
#include <memory>
#include <boost/noncopyable.hpp>
#include <libevmasm/SourceLocation.h>
#include <libsolidity/Utils.h>
#include <libsolidity/ASTForward.h>
#include <libsolidity/Token.h>
#include <libsolidity/Types.h>
#include <libsolidity/Exceptions.h>

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
	explicit ASTNode(SourceLocation const& _location): m_location(_location) {}

	virtual ~ASTNode() {}

	virtual void accept(ASTVisitor& _visitor) = 0;
	virtual void accept(ASTConstVisitor& _visitor) const = 0;
	template <class T>
	static void listAccept(std::vector<ASTPointer<T>>& _list, ASTVisitor& _visitor)
	{
		for (ASTPointer<T>& element: _list)
			element->accept(_visitor);
	}
	template <class T>
	static void listAccept(std::vector<ASTPointer<T>> const& _list, ASTConstVisitor& _visitor)
	{
		for (ASTPointer<T> const& element: _list)
			element->accept(_visitor);
	}

	/// Returns the source code location of this node.
	SourceLocation const& getLocation() const { return m_location; }

	/// Creates a @ref TypeError exception and decorates it with the location of the node and
	/// the given description
	TypeError createTypeError(std::string const& _description) const;

	///@{
	///@name equality operators
	/// Equality relies on the fact that nodes cannot be copied.
	bool operator==(ASTNode const& _other) const { return this == &_other; }
	bool operator!=(ASTNode const& _other) const { return !operator==(_other); }
	///@}

private:
	SourceLocation m_location;
};

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

	std::vector<ASTPointer<ASTNode>> getNodes() const { return m_nodes; }

private:
	std::vector<ASTPointer<ASTNode>> m_nodes;
};

/**
 * Import directive for referencing other files / source objects.
 * Example: import "abc.sol"
 * Source objects are identified by a string which can be a file name but does not have to be.
 */
class ImportDirective: public ASTNode
{
public:
	ImportDirective(SourceLocation const& _location, ASTPointer<ASTString> const& _identifier):
		ASTNode(_location), m_identifier(_identifier) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ASTString const& getIdentifier() const { return *m_identifier; }

private:
	ASTPointer<ASTString> m_identifier;
};

/**
 * Abstract AST class for a declaration (contract, function, struct, variable).
 */
class Declaration: public ASTNode
{
public:
	/// Visibility ordered from restricted to unrestricted.
	enum class Visibility { Default, Private, Internal, Public, External };

	Declaration(SourceLocation const& _location, ASTPointer<ASTString> const& _name,
				Visibility _visibility = Visibility::Default):
		ASTNode(_location), m_name(_name), m_visibility(_visibility), m_scope(nullptr) {}

	/// @returns the declared name.
	ASTString const& getName() const { return *m_name; }
	Visibility getVisibility() const { return m_visibility == Visibility::Default ? getDefaultVisibility() : m_visibility; }
	bool isPublic() const { return getVisibility() >= Visibility::Public; }
	virtual bool isVisibleInContract() const { return getVisibility() != Visibility::External; }
	bool isVisibleInDerivedContracts() const { return isVisibleInContract() && getVisibility() >= Visibility::Internal; }

	/// @returns the scope this declaration resides in. Can be nullptr if it is the global scope.
	/// Available only after name and type resolution step.
	Declaration const* getScope() const { return m_scope; }
	void setScope(Declaration const* _scope) { m_scope = _scope; }

	/// @returns the type of expressions referencing this declaration.
	/// The current contract has to be given since this context can change the type, especially of
	/// contract types.
	virtual TypePointer getType(ContractDefinition const* m_currentContract = nullptr) const = 0;
	virtual bool isLValue() const { return false; }
	virtual bool isPartOfExternalInterface() const { return false; }

protected:
	virtual Visibility getDefaultVisibility() const { return Visibility::Public; }

private:
	ASTPointer<ASTString> m_name;
	Visibility m_visibility;
	Declaration const* m_scope;
};

/**
 * Abstract class that is added to each AST node that can store local variables.
 */
class VariableScope
{
public:
	void addLocalVariable(VariableDeclaration const& _localVariable) { m_localVariables.push_back(&_localVariable); }
	std::vector<VariableDeclaration const*> const& getLocalVariables() const { return m_localVariables; }

private:
	std::vector<VariableDeclaration const*> m_localVariables;
};

/**
 * Abstract class that is added to each AST node that can receive documentation.
 */
class Documented
{
public:
	explicit Documented(ASTPointer<ASTString> const& _documentation): m_documentation(_documentation) {}

	/// @return A shared pointer of an ASTString.
	/// Can contain a nullptr in which case indicates absence of documentation
	ASTPointer<ASTString> const& getDocumentation() const { return m_documentation; }

protected:
	ASTPointer<ASTString> m_documentation;
};

/**
 * Abstract class that is added to AST nodes that can be marked as not being fully implemented
 */
class ImplementationOptional
{
public:
	explicit ImplementationOptional(bool _implemented): m_implemented(_implemented) {}

	/// @return whether this node is fully implemented or not
	bool isFullyImplemented() const { return m_implemented; }
	void setFullyImplemented(bool _implemented) {  m_implemented = _implemented; }

protected:
	bool m_implemented;
};

/// @}

/**
 * Definition of a contract. This is the only AST nodes where child nodes are not visited in
 * document order. It first visits all struct declarations, then all variable declarations and
 * finally all function declarations.
 */
class ContractDefinition: public Declaration, public Documented, public ImplementationOptional
{
public:
	ContractDefinition(
		SourceLocation const& _location,
		ASTPointer<ASTString> const& _name,
		ASTPointer<ASTString> const& _documentation,
		std::vector<ASTPointer<InheritanceSpecifier>> const& _baseContracts,
		std::vector<ASTPointer<StructDefinition>> const& _definedStructs,
		std::vector<ASTPointer<EnumDefinition>> const& _definedEnums,
		std::vector<ASTPointer<VariableDeclaration>> const& _stateVariables,
		std::vector<ASTPointer<FunctionDefinition>> const& _definedFunctions,
		std::vector<ASTPointer<ModifierDefinition>> const& _functionModifiers,
		std::vector<ASTPointer<EventDefinition>> const& _events
	):
		Declaration(_location, _name),
		Documented(_documentation),
		ImplementationOptional(true),
		m_baseContracts(_baseContracts),
		m_definedStructs(_definedStructs),
		m_definedEnums(_definedEnums),
		m_stateVariables(_stateVariables),
		m_definedFunctions(_definedFunctions),
		m_functionModifiers(_functionModifiers),
		m_events(_events)
	{}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<InheritanceSpecifier>> const& getBaseContracts() const { return m_baseContracts; }
	std::vector<ASTPointer<StructDefinition>> const& getDefinedStructs() const { return m_definedStructs; }
	std::vector<ASTPointer<EnumDefinition>> const& getDefinedEnums() const { return m_definedEnums; }
	std::vector<ASTPointer<VariableDeclaration>> const& getStateVariables() const { return m_stateVariables; }
	std::vector<ASTPointer<ModifierDefinition>> const& getFunctionModifiers() const { return m_functionModifiers; }
	std::vector<ASTPointer<FunctionDefinition>> const& getDefinedFunctions() const { return m_definedFunctions; }
	std::vector<ASTPointer<EventDefinition>> const& getEvents() const { return m_events; }
	std::vector<ASTPointer<EventDefinition>> const& getInterfaceEvents() const;

	virtual TypePointer getType(ContractDefinition const* m_currentContract) const override;

	/// Checks that there are no illegal overrides, that the constructor does not have a "returns"
	/// and calls checkTypeRequirements on all its functions.
	void checkTypeRequirements();

	/// @returns a map of canonical function signatures to FunctionDefinitions
	/// as intended for use by the ABI.
	std::map<FixedHash<4>, FunctionTypePointer> getInterfaceFunctions() const;

	/// @returns a list of the inheritable members of this contract
	std::vector<Declaration const*> const& getInheritableMembers() const;

	/// List of all (direct and indirect) base contracts in order from derived to base, including
	/// the contract itself. Available after name resolution
	std::vector<ContractDefinition const*> const& getLinearizedBaseContracts() const { return m_linearizedBaseContracts; }
	void setLinearizedBaseContracts(std::vector<ContractDefinition const*> const& _bases) { m_linearizedBaseContracts = _bases; }

	/// Returns the constructor or nullptr if no constructor was specified.
	FunctionDefinition const* getConstructor() const;
	/// Returns the fallback function or nullptr if no fallback function was specified.
	FunctionDefinition const* getFallbackFunction() const;

	std::string const& userDocumentation() const;
	void setUserDocumentation(std::string const& _userDocumentation);

	std::string const& devDocumentation() const;
	void setDevDocumentation(std::string const& _devDocumentation);

private:
	/// Checks that two functions defined in this contract with the same name have different
	/// arguments and that there is at most one constructor.
	void checkDuplicateFunctions() const;
	void checkIllegalOverrides() const;
	void checkAbstractFunctions();
	void checkAbstractConstructors();
	/// Checks that different functions with external visibility end up having different
	/// external argument types (i.e. different signature).
	void checkExternalTypeClashes() const;

	std::vector<std::pair<FixedHash<4>, FunctionTypePointer>> const& getInterfaceFunctionList() const;

	std::vector<ASTPointer<InheritanceSpecifier>> m_baseContracts;
	std::vector<ASTPointer<StructDefinition>> m_definedStructs;
	std::vector<ASTPointer<EnumDefinition>> m_definedEnums;
	std::vector<ASTPointer<VariableDeclaration>> m_stateVariables;
	std::vector<ASTPointer<FunctionDefinition>> m_definedFunctions;
	std::vector<ASTPointer<ModifierDefinition>> m_functionModifiers;
	std::vector<ASTPointer<EventDefinition>> m_events;

	// parsed Natspec documentation of the contract.
	std::string m_userDocumentation;
	std::string m_devDocumentation;

	std::vector<ContractDefinition const*> m_linearizedBaseContracts;
	mutable std::unique_ptr<std::vector<std::pair<FixedHash<4>, FunctionTypePointer>>> m_interfaceFunctionList;
	mutable std::unique_ptr<std::vector<ASTPointer<EventDefinition>>> m_interfaceEvents;
	mutable std::unique_ptr<std::vector<Declaration const*>> m_inheritableMembers;
};

class InheritanceSpecifier: public ASTNode
{
public:
	InheritanceSpecifier(SourceLocation const& _location, ASTPointer<Identifier> const& _baseName,
						 std::vector<ASTPointer<Expression>> _arguments):
		ASTNode(_location), m_baseName(_baseName), m_arguments(_arguments) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ASTPointer<Identifier> const& getName() const { return m_baseName; }
	std::vector<ASTPointer<Expression>> const& getArguments() const { return m_arguments; }

	void checkTypeRequirements();

private:
	ASTPointer<Identifier> m_baseName;
	std::vector<ASTPointer<Expression>> m_arguments;
};

class StructDefinition: public Declaration
{
public:
	StructDefinition(SourceLocation const& _location,
					 ASTPointer<ASTString> const& _name,
					 std::vector<ASTPointer<VariableDeclaration>> const& _members):
		Declaration(_location, _name), m_members(_members) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& getMembers() const { return m_members; }

	virtual TypePointer getType(ContractDefinition const*) const override;

	/// Checks that the members do not include any recursive structs and have valid types
	/// (e.g. no functions).
	void checkValidityOfMembers() const;

private:
	void checkMemberTypes() const;
	void checkRecursion() const;

	std::vector<ASTPointer<VariableDeclaration>> m_members;
};

class EnumDefinition: public Declaration
{
public:
	EnumDefinition(SourceLocation const& _location,
				   ASTPointer<ASTString> const& _name,
				   std::vector<ASTPointer<EnumValue>> const& _members):
		Declaration(_location, _name), m_members(_members) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<EnumValue>> const& getMembers() const { return m_members; }

	virtual TypePointer getType(ContractDefinition const*) const override;

private:
	std::vector<ASTPointer<EnumValue>> m_members;
};

/**
 * Declaration of an Enum Value
 */
class EnumValue: public Declaration
{
  public:
	EnumValue(SourceLocation const& _location,
			  ASTPointer<ASTString> const& _name):
		Declaration(_location, _name) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual TypePointer getType(ContractDefinition const* = nullptr) const override;
};

/**
 * Parameter list, used as function parameter list and return list.
 * None of the parameters is allowed to contain mappings (not even recursively
 * inside structs).
 */
class ParameterList: public ASTNode
{
public:
	ParameterList(SourceLocation const& _location,
				  std::vector<ASTPointer<VariableDeclaration>> const& _parameters):
		ASTNode(_location), m_parameters(_parameters) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	std::vector<ASTPointer<VariableDeclaration>> const& getParameters() const { return m_parameters; }

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

	std::vector<ASTPointer<VariableDeclaration>> const& getParameters() const { return m_parameters->getParameters(); }
	ParameterList const& getParameterList() const { return *m_parameters; }
	ASTPointer<ParameterList> const& getReturnParameterList() const { return m_returnParameters; }

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
		bool _isConstructor,
		ASTPointer<ASTString> const& _documentation,
		ASTPointer<ParameterList> const& _parameters,
		bool _isDeclaredConst,
		std::vector<ASTPointer<ModifierInvocation>> const& _modifiers,
		ASTPointer<ParameterList> const& _returnParameters,
		ASTPointer<Block> const& _body
	):
		CallableDeclaration(_location, _name, _visibility, _parameters, _returnParameters),
		Documented(_documentation),
		ImplementationOptional(_body != nullptr),
		m_isConstructor(_isConstructor),
		m_isDeclaredConst(_isDeclaredConst),
		m_functionModifiers(_modifiers),
		m_body(_body)
	{}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	bool isConstructor() const { return m_isConstructor; }
	bool isDeclaredConst() const { return m_isDeclaredConst; }
	std::vector<ASTPointer<ModifierInvocation>> const& getModifiers() const { return m_functionModifiers; }
	std::vector<ASTPointer<VariableDeclaration>> const& getReturnParameters() const { return m_returnParameters->getParameters(); }
	Block const& getBody() const { return *m_body; }

	virtual bool isVisibleInContract() const override
	{
		return Declaration::isVisibleInContract() && !isConstructor() && !getName().empty();
	}
	virtual TypePointer getType(ContractDefinition const*) const override;
	virtual bool isPartOfExternalInterface() const override { return isPublic() && !m_isConstructor && !getName().empty(); }

	/// Checks that all parameters have allowed types and calls checkTypeRequirements on the body.
	void checkTypeRequirements();

	/// @returns the external signature of the function
	/// That consists of the name of the function followed by the types of the
	/// arguments separated by commas all enclosed in parentheses without any spaces.
	std::string externalSignature() const;

private:
	bool m_isConstructor;
	bool m_isDeclaredConst;
	std::vector<ASTPointer<ModifierInvocation>> m_functionModifiers;
	ASTPointer<Block> m_body;
};

/**
 * Declaration of a variable. This can be used in various places, e.g. in function parameter
 * lists, struct definitions and even function bodys.
 */
class VariableDeclaration: public Declaration
{
public:
	enum Location { Default, Storage, Memory };

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

	TypeName* getTypeName() { return m_typeName.get(); }
	ASTPointer<Expression> const& getValue() const { return m_value; }

	/// Returns the declared or inferred type. Can be an empty pointer if no type was explicitly
	/// declared and there is no assignment to the variable that fixes the type.
	TypePointer getType(ContractDefinition const* = nullptr) const override { return m_type; }
	void setType(std::shared_ptr<Type const> const& _type) { m_type = _type; }

	virtual bool isLValue() const override;
	virtual bool isPartOfExternalInterface() const override { return isPublic() && !m_isConstant; }

	void checkTypeRequirements();
	bool isLocalVariable() const { return !!dynamic_cast<FunctionDefinition const*>(getScope()); }
	/// @returns true if this variable is a parameter or return parameter of a function.
	bool isCallableParameter() const;
	/// @returns true if this variable is a parameter (not return parameter) of an external function.
	bool isExternalCallableParameter() const;
	bool isStateVariable() const { return m_isStateVariable; }
	bool isIndexed() const { return m_isIndexed; }
	bool isConstant() const { return m_isConstant; }
	Location referenceLocation() const { return m_location; }

protected:
	Visibility getDefaultVisibility() const override { return Visibility::Internal; }

private:
	ASTPointer<TypeName> m_typeName; ///< can be empty ("var")
	ASTPointer<Expression> m_value; ///< the assigned value, can be missing
	bool m_isStateVariable; ///< Whether or not this is a contract state variable
	bool m_isIndexed; ///< Whether this is an indexed variable (used by events).
	bool m_isConstant; ///< Whether the variable is a compile-time constant.
	Location m_location; ///< Location of the variable if it is of reference type.

	std::shared_ptr<Type const> m_type; ///< derived type, initially empty
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
		CallableDeclaration(_location, _name, Visibility::Default, _parameters),
		Documented(_documentation),
		m_body(_body)
	{
	}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	Block const& getBody() const { return *m_body; }

	virtual TypePointer getType(ContractDefinition const* = nullptr) const override;

	void checkTypeRequirements();

private:
	ASTPointer<Block> m_body;
};

/**
 * Invocation/usage of a modifier in a function header or a base constructor call.
 */
class ModifierInvocation: public ASTNode
{
public:
	ModifierInvocation(SourceLocation const& _location, ASTPointer<Identifier> const& _name,
					   std::vector<ASTPointer<Expression>> _arguments):
		ASTNode(_location), m_modifierName(_name), m_arguments(_arguments) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	ASTPointer<Identifier> const& getName() const { return m_modifierName; }
	std::vector<ASTPointer<Expression>> const& getArguments() const { return m_arguments; }

	/// @param _bases is the list of base contracts for base constructor calls. For modifiers an empty vector should be passed.
	void checkTypeRequirements(std::vector<ContractDefinition const*> const& _bases);

private:
	ASTPointer<Identifier> m_modifierName;
	std::vector<ASTPointer<Expression>> m_arguments;
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

	virtual TypePointer getType(ContractDefinition const* = nullptr) const override
	{
		return std::make_shared<FunctionType>(*this);
	}

	void checkTypeRequirements();

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
	virtual void accept(ASTVisitor&) override { BOOST_THROW_EXCEPTION(InternalCompilerError()
							<< errinfo_comment("MagicVariableDeclaration used inside real AST.")); }
	virtual void accept(ASTConstVisitor&) const override { BOOST_THROW_EXCEPTION(InternalCompilerError()
							<< errinfo_comment("MagicVariableDeclaration used inside real AST.")); }

	virtual TypePointer getType(ContractDefinition const* = nullptr) const override { return m_type; }

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
public:
	explicit TypeName(SourceLocation const& _location): ASTNode(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	/// Retrieve the element of the type hierarchy this node refers to. Can return an empty shared
	/// pointer until the types have been resolved using the @ref NameAndTypeResolver.
	/// If it returns an empty shared pointer after that, this indicates that the type was not found.
	virtual std::shared_ptr<Type const> toType() = 0;
};

/**
 * Any pre-defined type name represented by a single keyword, i.e. it excludes mappings,
 * contracts, functions, etc.
 */
class ElementaryTypeName: public TypeName
{
public:
	explicit ElementaryTypeName(SourceLocation const& _location, Token::Value _type):
		TypeName(_location), m_type(_type)
	{
		solAssert(Token::isElementaryTypeName(_type), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual std::shared_ptr<Type const> toType() override { return Type::fromElementaryTypeName(m_type); }

	Token::Value getTypeName() const { return m_type; }

private:
	Token::Value m_type;
};

/**
 * Name referring to a user-defined type (i.e. a struct, contract, etc.).
 */
class UserDefinedTypeName: public TypeName
{
public:
	UserDefinedTypeName(SourceLocation const& _location, ASTPointer<ASTString> const& _name):
		TypeName(_location), m_name(_name), m_referencedDeclaration(nullptr) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual std::shared_ptr<Type const> toType() override { return Type::fromUserDefinedTypeName(*this); }

	ASTString const& getName() const { return *m_name; }
	void setReferencedDeclaration(Declaration const& _referencedDeclaration) { m_referencedDeclaration = &_referencedDeclaration; }
	Declaration const* getReferencedDeclaration() const { return m_referencedDeclaration; }

private:
	ASTPointer<ASTString> m_name;

	Declaration const* m_referencedDeclaration;
};

/**
 * A mapping type. Its source form is "mapping('keyType' => 'valueType')"
 */
class Mapping: public TypeName
{
public:
	Mapping(SourceLocation const& _location, ASTPointer<ElementaryTypeName> const& _keyType,
			ASTPointer<TypeName> const& _valueType):
		TypeName(_location), m_keyType(_keyType), m_valueType(_valueType) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual TypePointer toType() override { return Type::fromMapping(*m_keyType, *m_valueType); }

	ElementaryTypeName const& getKeyType() const { return *m_keyType; }
	TypeName const& getValueType() const { return *m_valueType; }

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
	ArrayTypeName(SourceLocation const& _location, ASTPointer<TypeName> const& _baseType,
			ASTPointer<Expression> const& _length):
		TypeName(_location), m_baseType(_baseType), m_length(_length) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual std::shared_ptr<Type const> toType() override { return Type::fromArrayTypeName(*m_baseType, m_length.get()); }

	TypeName const& getBaseType() const { return *m_baseType; }
	Expression const* getLength() const { return m_length.get(); }

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
class Statement: public ASTNode
{
public:
	explicit Statement(SourceLocation const& _location): ASTNode(_location) {}

	/// Check all type requirements, throws exception if some requirement is not met.
	/// This includes checking that operators are applicable to their arguments but also that
	/// the number of function call arguments matches the number of formal parameters and so forth.
	virtual void checkTypeRequirements() = 0;
};

/**
 * Brace-enclosed block containing zero or more statements.
 */
class Block: public Statement
{
public:
	Block(SourceLocation const& _location, std::vector<ASTPointer<Statement>> const& _statements):
		Statement(_location), m_statements(_statements) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	virtual void checkTypeRequirements() override;

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
	PlaceholderStatement(SourceLocation const& _location): Statement(_location) {}

	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;

	virtual void checkTypeRequirements() override { }
};

/**
 * If-statement with an optional "else" part. Note that "else if" is modeled by having a new
 * if-statement as the false (else) body.
 */
class IfStatement: public Statement
{
public:
	IfStatement(SourceLocation const& _location, ASTPointer<Expression> const& _condition,
				ASTPointer<Statement> const& _trueBody, ASTPointer<Statement> const& _falseBody):
		Statement(_location),
		m_condition(_condition), m_trueBody(_trueBody), m_falseBody(_falseBody) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override;

	Expression const& getCondition() const { return *m_condition; }
	Statement const& getTrueStatement() const { return *m_trueBody; }
	/// @returns the "else" part of the if statement or nullptr if there is no "else" part.
	Statement const* getFalseStatement() const { return m_falseBody.get(); }

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
	BreakableStatement(SourceLocation const& _location): Statement(_location) {}
};

class WhileStatement: public BreakableStatement
{
public:
	WhileStatement(SourceLocation const& _location, ASTPointer<Expression> const& _condition,
				   ASTPointer<Statement> const& _body):
		BreakableStatement(_location), m_condition(_condition), m_body(_body) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override;

	Expression const& getCondition() const { return *m_condition; }
	Statement const& getBody() const { return *m_body; }

private:
	ASTPointer<Expression> m_condition;
	ASTPointer<Statement> m_body;
};

/**
 * For loop statement
 */
class ForStatement: public BreakableStatement
{
public:
	ForStatement(SourceLocation const& _location,
				 ASTPointer<Statement> const& _initExpression,
				 ASTPointer<Expression> const& _conditionExpression,
				 ASTPointer<ExpressionStatement> const& _loopExpression,
				 ASTPointer<Statement> const& _body):
		BreakableStatement(_location),
		m_initExpression(_initExpression),
		m_condExpression(_conditionExpression),
		m_loopExpression(_loopExpression),
		m_body(_body) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override;

	Statement const* getInitializationExpression() const { return m_initExpression.get(); }
	Expression const* getCondition() const { return m_condExpression.get(); }
	ExpressionStatement const* getLoopExpression() const { return m_loopExpression.get(); }
	Statement const& getBody() const { return *m_body; }

private:
	/// For statement's initialization expresion. for(XXX; ; ). Can be empty
	ASTPointer<Statement> m_initExpression;
	/// For statement's condition expresion. for(; XXX ; ). Can be empty
	ASTPointer<Expression> m_condExpression;
	/// For statement's loop expresion. for(;;XXX). Can be empty
	ASTPointer<ExpressionStatement> m_loopExpression;
	/// The body of the loop
	ASTPointer<Statement> m_body;
};

class Continue: public Statement
{
public:
	Continue(SourceLocation const& _location): Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override {}
};

class Break: public Statement
{
public:
	Break(SourceLocation const& _location): Statement(_location) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override {}
};

class Return: public Statement
{
public:
	Return(SourceLocation const& _location, ASTPointer<Expression> _expression):
		Statement(_location), m_expression(_expression), m_returnParameters(nullptr) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override;

	void setFunctionReturnParameters(ParameterList const* _parameters) { m_returnParameters = _parameters; }
	ParameterList const* getFunctionReturnParameters() const { return m_returnParameters; }
	Expression const* getExpression() const { return m_expression.get(); }

private:
	ASTPointer<Expression> m_expression; ///< value to return, optional

	/// Pointer to the parameter list of the function, filled by the @ref NameAndTypeResolver.
	ParameterList const* m_returnParameters;
};

/**
 * Definition of a variable as a statement inside a function. It requires a type name (which can
 * also be "var") but the actual assignment can be missing.
 * Examples: var a = 2; uint256 a;
 */
class VariableDeclarationStatement: public Statement
{
public:
	VariableDeclarationStatement(SourceLocation const& _location, ASTPointer<VariableDeclaration> _variable):
		Statement(_location), m_variable(_variable) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override;

	VariableDeclaration const& getDeclaration() const { return *m_variable; }
	Expression const* getExpression() const { return m_variable->getValue().get(); }

private:
	ASTPointer<VariableDeclaration> m_variable;
};

/**
 * A statement that contains only an expression (i.e. an assignment, function call, ...).
 */
class ExpressionStatement: public Statement
{
public:
	ExpressionStatement(SourceLocation const& _location, ASTPointer<Expression> _expression):
		Statement(_location), m_expression(_expression) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements() override;

	Expression const& getExpression() const { return *m_expression; }

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
	Expression(SourceLocation const& _location): ASTNode(_location) {}
	/// Performs type checking after which m_type should be set.
	/// @arg _argumentTypes if set, provides the argument types for the case that this expression
	/// is used in the context of a call, used for function overload resolution.
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) = 0;

	std::shared_ptr<Type const> const& getType() const { return m_type; }
	bool isLValue() const { return m_isLValue; }

	/// Helper function, infer the type via @ref checkTypeRequirements and then check that it
	/// is implicitly convertible to @a _expectedType. If not, throw exception.
	void expectType(Type const& _expectedType);
	/// Checks that this expression is an lvalue and also registers that an address and
	/// not a value is generated during compilation. Can be called after checkTypeRequirements()
	/// by an enclosing expression.
	void requireLValue();
	/// Returns true if @a requireLValue was previously called on this expression.
	bool lvalueRequested() const { return m_lvalueRequested; }

protected:
	//! Inferred type of the expression, only filled after a call to checkTypeRequirements().
	std::shared_ptr<Type const> m_type;
	//! If this expression is an lvalue (i.e. something that can be assigned to).
	//! This is set during calls to @a checkTypeRequirements()
	bool m_isLValue = false;
	//! Whether the outer expression requested the address (true) or the value (false) of this expression.
	bool m_lvalueRequested = false;
};

/// Assignment, can also be a compound assignment.
/// Examples: (a = 7 + 8) or (a *= 2)
class Assignment: public Expression
{
public:
	Assignment(SourceLocation const& _location, ASTPointer<Expression> const& _leftHandSide,
			   Token::Value _assignmentOperator, ASTPointer<Expression> const& _rightHandSide):
		Expression(_location), m_leftHandSide(_leftHandSide),
		m_assigmentOperator(_assignmentOperator), m_rightHandSide(_rightHandSide)
	{
		solAssert(Token::isAssignmentOp(_assignmentOperator), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	Expression const& getLeftHandSide() const { return *m_leftHandSide; }
	Token::Value getAssignmentOperator() const { return m_assigmentOperator; }
	Expression const& getRightHandSide() const { return *m_rightHandSide; }

private:
	ASTPointer<Expression> m_leftHandSide;
	Token::Value m_assigmentOperator;
	ASTPointer<Expression> m_rightHandSide;
};

/**
 * Operation involving a unary operator, pre- or postfix.
 * Examples: ++i, delete x or !true
 */
class UnaryOperation: public Expression
{
public:
	UnaryOperation(SourceLocation const& _location, Token::Value _operator,
				   ASTPointer<Expression> const& _subExpression, bool _isPrefix):
		Expression(_location), m_operator(_operator),
		m_subExpression(_subExpression), m_isPrefix(_isPrefix)
	{
		solAssert(Token::isUnaryOp(_operator), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	Token::Value getOperator() const { return m_operator; }
	bool isPrefixOperation() const { return m_isPrefix; }
	Expression const& getSubExpression() const { return *m_subExpression; }

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
	BinaryOperation(SourceLocation const& _location, ASTPointer<Expression> const& _left,
					Token::Value _operator, ASTPointer<Expression> const& _right):
		Expression(_location), m_left(_left), m_operator(_operator), m_right(_right)
	{
		solAssert(Token::isBinaryOp(_operator) || Token::isCompareOp(_operator), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	Expression const& getLeftExpression() const { return *m_left; }
	Expression const& getRightExpression() const { return *m_right; }
	Token::Value getOperator() const { return m_operator; }
	Type const& getCommonType() const { return *m_commonType; }

private:
	ASTPointer<Expression> m_left;
	Token::Value m_operator;
	ASTPointer<Expression> m_right;

	/// The common type that is used for the operation, not necessarily the result type (e.g. for
	/// comparisons, this is always bool).
	std::shared_ptr<Type const> m_commonType;
};

/**
 * Can be ordinary function call, type cast or struct construction.
 */
class FunctionCall: public Expression
{
public:
	FunctionCall(SourceLocation const& _location, ASTPointer<Expression> const& _expression,
				 std::vector<ASTPointer<Expression>> const& _arguments, std::vector<ASTPointer<ASTString>> const& _names):
		Expression(_location), m_expression(_expression), m_arguments(_arguments), m_names(_names) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	Expression const& getExpression() const { return *m_expression; }
	std::vector<ASTPointer<Expression const>> getArguments() const { return {m_arguments.begin(), m_arguments.end()}; }
	std::vector<ASTPointer<ASTString>> const& getNames() const { return m_names; }

	/// @returns true if this is not an actual function call, but an explicit type conversion.
	/// Returns false for struct constructor calls.
	bool isTypeConversion() const;
	/// @return true if this is a constructor call for a struct, i.e. StructName(...).
	bool isStructConstructorCall() const;

private:
	ASTPointer<Expression> m_expression;
	std::vector<ASTPointer<Expression>> m_arguments;
	std::vector<ASTPointer<ASTString>> m_names;
};

/**
 * Expression that creates a new contract, e.g. the "new SomeContract" part in "new SomeContract(1, 2)".
 */
class NewExpression: public Expression
{
public:
	NewExpression(SourceLocation const& _location, ASTPointer<Identifier> const& _contractName):
		Expression(_location), m_contractName(_contractName) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	/// Returns the referenced contract. Can only be called after type checking.
	ContractDefinition const* getContract() const { solAssert(m_contract, ""); return m_contract; }

private:
	ASTPointer<Identifier> m_contractName;

	ContractDefinition const* m_contract = nullptr;
};

/**
 * Access to a member of an object. Example: x.name
 */
class MemberAccess: public Expression
{
public:
	MemberAccess(SourceLocation const& _location, ASTPointer<Expression> _expression,
				 ASTPointer<ASTString> const& _memberName):
		Expression(_location), m_expression(_expression), m_memberName(_memberName) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	Expression const& getExpression() const { return *m_expression; }
	ASTString const& getMemberName() const { return *m_memberName; }
	/// @returns the declaration referenced by this expression. Might return nullptr even if the
	/// expression is valid, e.g. if the member does not correspond to an AST node.
	Declaration const* referencedDeclaration() const { return m_referencedDeclaration; }
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

private:
	ASTPointer<Expression> m_expression;
	ASTPointer<ASTString> m_memberName;

	/// Pointer to the referenced declaration, this is sometimes needed to resolve function over
	/// loads in the type-checking phase.
	Declaration const* m_referencedDeclaration = nullptr;
};

/**
 * Index access to an array. Example: a[2]
 */
class IndexAccess: public Expression
{
public:
	IndexAccess(SourceLocation const& _location, ASTPointer<Expression> const& _base,
				ASTPointer<Expression> const& _index):
		Expression(_location), m_base(_base), m_index(_index) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	Expression const& getBaseExpression() const { return *m_base; }
	Expression const* getIndexExpression() const { return m_index.get(); }

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
	Identifier(SourceLocation const& _location, ASTPointer<ASTString> const& _name):
		PrimaryExpression(_location), m_name(_name) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	ASTString const& getName() const { return *m_name; }

	void setReferencedDeclaration(
		Declaration const& _referencedDeclaration,
		ContractDefinition const* _currentContract = nullptr
	)
	{
		m_referencedDeclaration = &_referencedDeclaration;
		m_currentContract = _currentContract;
	}
	Declaration const& getReferencedDeclaration() const;

	/// Stores a set of possible declarations referenced by this identifier. Has to be resolved
	/// providing argument types using overloadResolution before the referenced declaration
	/// is accessed.
	void setOverloadedDeclarations(std::vector<Declaration const*> const& _declarations)
	{
		m_overloadedDeclarations = _declarations;
	}

	/// Tries to find exactly one of the possible referenced declarations provided the given
	/// argument types in a call context.
	void overloadResolution(TypePointers const& _argumentTypes);

private:
	ASTPointer<ASTString> m_name;

	/// Declaration the name refers to.
	Declaration const* m_referencedDeclaration = nullptr;
	/// Stores a reference to the current contract. This is needed because types of base contracts
	/// change depending on the context.
	ContractDefinition const* m_currentContract = nullptr;
	/// A vector of overloaded declarations, right now only FunctionDefinition has overloaded declarations.
	std::vector<Declaration const*> m_overloadedDeclarations;
};

/**
 * An elementary type name expression is used in expressions like "a = uint32(2)" to change the
 * type of an expression explicitly. Here, "uint32" is the elementary type name expression and
 * "uint32(2)" is a @ref FunctionCall.
 */
class ElementaryTypeNameExpression: public PrimaryExpression
{
public:
	ElementaryTypeNameExpression(SourceLocation const& _location, Token::Value _typeToken):
		PrimaryExpression(_location), m_typeToken(_typeToken)
	{
		solAssert(Token::isElementaryTypeName(_typeToken), "");
	}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	Token::Value getTypeToken() const { return m_typeToken; }

private:
	Token::Value m_typeToken;
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
	Literal(SourceLocation const& _location, Token::Value _token,
			ASTPointer<ASTString> const& _value,
			SubDenomination _sub = SubDenomination::None):
		PrimaryExpression(_location), m_token(_token), m_value(_value), m_subDenomination(_sub) {}
	virtual void accept(ASTVisitor& _visitor) override;
	virtual void accept(ASTConstVisitor& _visitor) const override;
	virtual void checkTypeRequirements(TypePointers const* _argumentTypes) override;

	Token::Value getToken() const { return m_token; }
	/// @returns the non-parsed value of the literal
	ASTString const& getValue() const { return *m_value; }

	SubDenomination getSubDenomination() const { return m_subDenomination; }

private:
	Token::Value m_token;
	ASTPointer<ASTString> m_value;
	SubDenomination m_subDenomination;
};

/// @}


}
}
