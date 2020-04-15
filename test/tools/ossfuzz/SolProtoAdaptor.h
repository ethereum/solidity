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
#pragma once

#include <test/tools/ossfuzz/protoToSol.h>

#include <iostream>
#include <string>
#include <utility>
#include <variant>
#include <boost/variant/static_visitor.hpp>

namespace solidity::test::solprotofuzzer::adaptor
{
/// Solidity contract abstraction class
struct SolContract;
/// Solidity interface abstraction class
struct SolInterface;
/// Solidity library abstraction class
struct SolLibrary;
/// Solidity interface function abstraction class
struct SolInterfaceFunction;
/// Solidity contract function abstraction class
struct SolContractFunction;
/// Solidity library function abstraction class
struct SolLibraryFunction;
/// Solidity contract function override abstraction class
struct CFunctionOverride;
/// Solidity interface function override abstraction class
struct IFunctionOverride;

/// Type that defines a contract function override as a variant of interface or
/// contract function types.
using OverrideFunction = std::variant<std::unique_ptr<SolInterfaceFunction const>, std::unique_ptr<SolContractFunction const>>;
/// Type that defines a list of base contracts as a list of variants of interface or
/// contract types.
using BaseContracts = std::vector<std::variant<std::shared_ptr<SolInterface const>, std::shared_ptr<SolContract const>>>;
/// Type that defines a contract function override as a pair of base contracts
/// and its (their) function.
using OverrideCFunction = std::pair<BaseContracts, OverrideFunction>;
/// Type that defines an interface function override as a pair of interface
/// and its function.
using OverrideIFunction = std::pair<std::vector<std::shared_ptr<SolInterface const>>, std::unique_ptr<SolInterfaceFunction const>>;
/// Type that defines a map of interface overrides
using InterfaceOverrideMap = std::map<std::shared_ptr<SolInterfaceFunction>, std::vector<std::shared_ptr<SolInterface>>>;
/// Type that defines an interface function as either a vanilla interface function
/// or an override function.
using IFunction = std::variant<std::unique_ptr<SolInterfaceFunction>, OverrideIFunction>;
/// Type that defines a contract function as either a vanilla contract function
/// or an override function.
using CFunction = std::variant<std::unique_ptr<SolContractFunction>, OverrideCFunction>;
/// Variant type that points to one of contract, interface protobuf messages
using ProtoBaseContract = std::variant<Contract const*, Interface const*>;

enum class SolFunctionVisibility
{
	PUBLIC,
	PRIVATE,
	INTERNAL,
	EXTERNAL
};

enum class SolFunctionStateMutability
{
	PURE,
	VIEW,
	PAYABLE
};

enum class SolLibraryFunctionStateMutability
{
	PURE,
	VIEW
};

struct SolInterfaceFunction
{
	enum class Type
	{
		MEMBERFUNCTION,
		IMPLICITOVERRIDE,
		EXPLICITOVERRIDE
	};

	SolInterfaceFunction(
		std::string _functionName,
		SolFunctionStateMutability _mutability,
		Type _type,
		std::string _contractName
	);
	bool operator==(SolInterfaceFunction const& _rhs) const;
	bool operator!=(SolInterfaceFunction const& _rhs) const;
	bool operator==(SolContractFunction const& _rhs) const;
	bool operator!=(SolContractFunction const& _rhs) const;
	void merge(SolInterfaceFunction const& _rhs);
	bool namesake(SolInterfaceFunction const& _rhs) const;
	bool namesake(SolContractFunction const& _rhs) const;
	void markExplicitOverride(std::string _contractName);

	std::string overriddenFromBaseNames() const;
	unsigned numOverriddenFromBases() const
	{
		return m_overriddenFrom.size();
	}
	void resetOverriddenBases()
	{
		m_overriddenFrom.clear();
		m_overriddenFrom.push_back(m_contractName);
	}

	std::string str() const;

	std::string name() const
	{
		return m_functionName;
	}
	SolFunctionStateMutability mutability() const
	{
		return m_mutability;
	}
	bool explicitOverride() const
	{
		return m_type == Type::EXPLICITOVERRIDE;
	}
	bool implicitOverride() const
	{
		return m_type == Type::IMPLICITOVERRIDE;
	}
	bool memberFunction() const
	{
		return m_type == Type::MEMBERFUNCTION;
	}
	void markImplicitOverride()
	{
		m_type = Type::IMPLICITOVERRIDE;
	}
	bool multipleBases() const
	{
		return m_overriddenFrom.size() > 1;
	}

	std::string m_functionName;
	SolFunctionStateMutability m_mutability = SolFunctionStateMutability::PURE;
	std::vector<std::string> m_overriddenFrom;
	std::string m_contractName;
	Type m_type;
};

struct SolContractFunction
{
	enum class Type
	{
		MEMBERFUNCTION,
		IMPLICITOVERRIDECONTRACT,
		IMPLICITOVERRIDEINTERFACE,
		EXPLICITOVERRIDECONTRACT,
		EXPLICITOVERRIDEINTERFACE
	};

	SolContractFunction(
		ContractFunction const& _function,
		std::string _contractName,
		std::string _functionName,
		Type _type,
		bool _implement,
		std::string _returnValue
	);
	SolContractFunction(
		std::vector<std::string> _overriddenFrom,
		SolFunctionStateMutability _mutability,
		std::string _contractName,
		std::string _functionName,
		Type _type,
		bool _implement,
		bool _virtual,
		std::string _returnValue,
		SolFunctionVisibility _vis = SolFunctionVisibility::EXTERNAL
	);

	bool memberFunction() const
	{
		return m_type == Type::MEMBERFUNCTION;
	}
	bool explicitInterfaceOverride() const
	{
		return m_type == Type::EXPLICITOVERRIDEINTERFACE;
	}
	bool explicitContractOverride() const
	{
		return m_type == Type::EXPLICITOVERRIDECONTRACT;
	}
	bool implicitInterfaceOverride() const
	{
		return m_type == Type::IMPLICITOVERRIDEINTERFACE;
	}
	bool implicitContractOverride() const
	{
		return m_type == Type::IMPLICITOVERRIDECONTRACT;
	}
	bool explicitOverride() const
	{
		return explicitContractOverride() || explicitInterfaceOverride();
	}
	bool implicitOverride() const
	{
		return implicitContractOverride() || implicitInterfaceOverride();
	}
	void markImplicitContractOverride()
	{
		m_type = Type::IMPLICITOVERRIDECONTRACT;
	}
	void markImplicitInterfaceOverride()
	{
		m_type = Type::IMPLICITOVERRIDEINTERFACE;
	}
	void markExplicitContractOverride(std::string _contractName)
	{
		m_type = Type::EXPLICITOVERRIDECONTRACT;
		m_contractName = _contractName;
		m_overriddenFrom.clear();
		m_overriddenFrom.push_back(m_contractName);
	}
	void markExplicitInterfaceOverride(std::string _contractName)
	{
		m_type = Type::EXPLICITOVERRIDEINTERFACE;
		m_contractName = _contractName;
		m_overriddenFrom.clear();
		m_overriddenFrom.push_back(m_contractName);
	}
	std::string overriddenFromBaseNames() const;
	unsigned numOverriddenFromBases() const
	{
		return m_overriddenFrom.size();
	}
	void resetOverriddenBases()
	{
		m_overriddenFrom.clear();
		m_overriddenFrom.push_back(m_contractName);
	}

	bool namesake(SolContractFunction const& _rhs) const;
	bool namesake(SolInterfaceFunction const& _rhs) const;
	bool operator==(SolContractFunction const& _rhs) const;
	bool operator!=(SolContractFunction const& _rhs) const;
	bool operator==(SolInterfaceFunction const& _rhs) const;
	bool operator!=(SolInterfaceFunction const& _rhs) const;
	void merge(SolContractFunction const& _rhs);
	void merge(SolInterfaceFunction const& _rhs);
	bool disallowed() const;
	std::string str() const;

	std::string name() const
	{
		return m_functionName;
	}
	std::string contractName() const
	{
		return m_contractName;
	}
	bool isVirtual() const
	{
		return m_virtual;
	}
	bool implemented() const
	{
		return m_implemented;
	}
	std::string returnValue() const
	{
		return m_returnValue;
	}
	SolFunctionVisibility visibility() const
	{
		return m_visibility;
	}
	SolFunctionStateMutability mutability() const
	{
		return m_mutability;
	}

	std::string m_contractName;
	std::string m_functionName;
	SolFunctionVisibility m_visibility = SolFunctionVisibility::PUBLIC;
	SolFunctionStateMutability m_mutability = SolFunctionStateMutability::PURE;
	bool m_virtual = false;
	std::string m_returnValue;
	bool m_implemented = true;
	Type m_type;
	std::vector<std::string> m_overriddenFrom;
};

struct SolLibraryFunction
{
	SolLibraryFunction(
		LibraryFunction const& _function,
		std::string _libraryName,
		std::string _functionName,
		std::string _returnValue
	);
	std::string str() const;

	std::string name() const
	{
		return m_functionName;
	}
	std::string returnValue() const
	{
		return m_returnValue;
	}
	SolFunctionVisibility visibility() const
	{
		return m_visibility;
	}
	SolLibraryFunctionStateMutability mutability() const
	{
		return m_mutability;
	}

	std::string m_libraryName;
	std::string m_functionName;
	SolFunctionVisibility m_visibility = SolFunctionVisibility::PUBLIC;
	SolLibraryFunctionStateMutability m_mutability = SolLibraryFunctionStateMutability::PURE;
	std::string m_returnValue;
};

struct SolLibrary
{
	SolLibrary(Library const& _library, std::string _name, std::shared_ptr<SolRandomNumGenerator> _prng);
	std::vector<std::unique_ptr<SolLibraryFunction>> m_functions;
	/// Maps publicly exposed function name to expected output
	std::map<std::string, std::string> m_publicFunctionMap;

	void addFunction(LibraryFunction const& _function);

	bool validTest() const;
	unsigned randomNumber() const
	{
		return m_prng->operator()();
	}

	/// Returns a pair of function name and expected output
	/// that is pseudo randomly chosen from the list of all
	/// library functions.
	std::pair<std::string, std::string> pseudoRandomTest();

	std::string str() const;

	std::string name() const
	{
		return m_libraryName;
	}
	std::string newFunctionName()
	{
		return "f" + std::to_string(m_functionIndex++);
	}
	std::string newReturnValue()
	{
		return std::to_string(m_returnValue++);
	}

	std::string m_libraryName;
	unsigned m_functionIndex = 0;
	unsigned m_returnValue = 0;
	std::shared_ptr<SolRandomNumGenerator> m_prng;
};

struct SolBaseContract
{
	enum BaseType
	{
		INTERFACE,
		CONTRACT
	};

	SolBaseContract(
		ProtoBaseContract _base,
		std::string _name,
		std::shared_ptr<SolRandomNumGenerator> _prng
	);

	std::variant<std::vector<std::shared_ptr<SolContractFunction>>, std::vector<std::shared_ptr<SolInterfaceFunction>>>
	baseFunctions();
	BaseType type() const;
	std::string name();
	std::string str();
	std::shared_ptr<SolInterface> interface()
	{
		return std::get<std::shared_ptr<SolInterface>>(m_base);
	}
	std::shared_ptr<SolContract> contract()
	{
		return std::get<std::shared_ptr<SolContract>>(m_base);
	}
	unsigned functionIndex();
	std::string lastBaseName();

	std::variant<std::shared_ptr<SolInterface>, std::shared_ptr<SolContract>> m_base;
	std::string m_baseName;
	std::shared_ptr<SolRandomNumGenerator> m_prng;
};

struct SolFunction
{
	enum Type
	{
		INTERFACE,
		CONTRACT
	};
	using FunctionVariant = std::variant<std::shared_ptr<SolInterfaceFunction>, std::shared_ptr<SolContractFunction>>;

	Type operator()(FunctionVariant _var) const;
};

struct ContractFunctionAttributes
{
	ContractFunctionAttributes(
		std::variant<std::shared_ptr<SolInterfaceFunction>, std::shared_ptr<SolContractFunction>> _solFunction
	)
	{
		if (SolFunction{}(_solFunction) == SolFunction::INTERFACE)
		{
			auto f = std::get<std::shared_ptr<SolInterfaceFunction>>(_solFunction);
			m_name = f->name();
			m_mutability = f->mutability();
			m_visibility = SolFunctionVisibility::EXTERNAL;
		}
		else
		{
			auto f = std::get<std::shared_ptr<SolContractFunction>>(_solFunction);
			m_name = f->name();
			m_mutability = f->mutability();
			m_visibility = f->visibility();
		}
	}
	bool namesake(ContractFunctionAttributes const& _rhs) const;
	bool operator==(ContractFunctionAttributes const& _rhs) const;
	bool operator!=(ContractFunctionAttributes const& _rhs) const;
	void merge(ContractFunctionAttributes const& _rhs);

	std::string m_name;
	SolFunctionVisibility m_visibility;
	SolFunctionStateMutability m_mutability;
	std::string m_returnValue;
	bool m_virtual;
	bool m_override;
	bool m_implemeted;
	std::vector<std::string> m_bases;
};

struct SolContract
{
	using FunctionList = std::vector<std::variant<std::shared_ptr<SolContractFunction>, std::shared_ptr<SolInterfaceFunction>>>;

	SolContract(
		Contract const& _contract,
		std::string _name,
		std::shared_ptr<SolRandomNumGenerator> _prng
	);

	void merge();
	std::string str();
	std::string interfaceOverrideStr();
	std::string contractOverrideStr();
	void disallowedBase(std::shared_ptr<SolBaseContract> _base1, std::shared_ptr<SolBaseContract> _base2);
	void addFunctions(Contract const& _contract);
	void addBases(Contract const& _contract);
	void addOverrides();
	void interfaceFunctionOverride(
		std::shared_ptr<SolInterface> _base,
		std::shared_ptr<SolInterfaceFunction> _function
	);
	void contractFunctionOverride(
		std::shared_ptr<SolContract> _base,
		std::shared_ptr<SolContractFunction> _function
	);

	bool validTest();
	std::string baseNames() const;
	std::tuple<std::string, std::string, std::string> validContractTest();
	std::tuple<std::string, std::string, std::string> pseudoRandomTest();

	unsigned randomNumber() const
	{
		return m_prng->operator()();
	}
	bool coinToss() const
	{
		return randomNumber() % 2 == 0;
	}

	std::string name() const
	{
		return m_contractName;
	}
	bool abstract() const
	{
		return m_abstract;
	}
	std::string newFunctionName()
	{
		return "f" + std::to_string(m_functionIndex++);
	}
	unsigned functionIndex() const
	{
		return m_functionIndex;
	}
	std::string newBaseName()
	{
		m_lastBaseName += "B";
		return m_lastBaseName;
	}
	std::string lastBaseName() const
	{
		return m_lastBaseName;
	}
	std::string newReturnValue()
	{
		return std::to_string(randomNumber());
	}

	std::string m_contractName;
	bool m_abstract = false;
	unsigned m_functionIndex = 0;
	unsigned m_returnValue = 0;
	std::string m_lastBaseName;
	FunctionList m_functions;
	std::vector<std::shared_ptr<SolBaseContract>> m_baseContracts;
	/// Maps non abstract contract name to list of publicly exposed function name
	/// and their expected output
	std::map<std::string, std::map<std::string, std::string>> m_contractFunctionMap;
	std::shared_ptr<SolRandomNumGenerator> m_prng;
};

struct SolInterface
{
	SolInterface(
		Interface const& _interface,
		std::string _interfaceName,
		std::shared_ptr<SolRandomNumGenerator> _prng
	);

	void merge();

	std::string name() const
	{
		return m_interfaceName;
	}

	unsigned randomNumber() const
	{
		return m_prng->operator()();
	}

	bool coinToss() const
	{
		return randomNumber() % 2 == 0;
	}

	std::string newFunctionName()
	{
		return "f" + std::to_string(m_functionIndex++);
	}

	unsigned functionIndex() const
	{
		return m_functionIndex;
	}

	std::string newBaseName()
	{
		m_lastBaseName += "B";
		return m_lastBaseName;
	}

	std::string lastBaseName()
	{
		return m_lastBaseName;
	}

	std::string str() const;


	/// Returns the Solidity code for all base interfaces
	/// inherited by this interface.
	std::string baseInterfaceStr() const;
	std::string functionStr() const;
	bool bases() const
	{
		return m_baseInterfaces.size() > 0;
	}

	/// Returns comma-space separated names of base interfaces inherited by
	/// this interface.
	std::string baseNames() const;

	/// Add base contracts in a depth-first manner
	void addBases(Interface const& _interface);
	/// Add functions
	void addFunctions(Interface const& _interface);

	unsigned m_functionIndex = 0;
	std::string m_lastBaseName;
	std::string m_interfaceName;
	std::vector<std::shared_ptr<SolInterfaceFunction>> m_functions;
	std::vector<std::shared_ptr<SolInterface>> m_baseInterfaces;
	std::shared_ptr<SolRandomNumGenerator> m_prng;
};

/* Contract functions may be overridden by other contracts. Base and derived contracts
 * may either be abstract or non-abstract. That gives us four possibilities:
 * - both abstract
 * - both non abstract
 * - one of them abstract, the other non abstract
 */

struct CFunctionOverride
{
	enum class DerivedType
	{
		ABSTRACTCONTRACT,
		CONTRACT
	};

	CFunctionOverride(
		std::variant<std::shared_ptr<SolContract>, std::shared_ptr<SolInterface>> _base,
		std::shared_ptr<SolContractFunction> _function,
		SolContract* _derived,
		bool _implemented,
		bool _virtualized,
		bool _explicitInheritance,
		std::string _returnValue
	)
	{
		m_baseContract = _base;
		m_baseFunction = _function;
		m_derivedProgram = _derived;
		m_implemented = _implemented;
		m_virtualized = _virtualized;
		m_explicitlyInherited = _explicitInheritance;
		m_returnValue = _returnValue;
		m_derivedType = _derived->abstract() ? DerivedType::ABSTRACTCONTRACT : DerivedType::CONTRACT;
	}

	std::string str() const;

	std::string name() const;

	bool contractFunction() const;

	SolFunctionVisibility visibility() const;

	SolFunctionStateMutability mutability() const;

	std::string commaSeparatedBaseNames() const;

	std::string baseName() const;

//	std::shared_ptr<SolContract> baseContract() const
//	{
//		return m_baseContract;
//	}

	std::shared_ptr<SolContractFunction> baseFunction() const
	{
		return m_baseFunction;
	}

	std::variant<std::shared_ptr<SolContract>, std::shared_ptr<SolInterface>> m_baseContract;
	std::shared_ptr<SolContractFunction> m_baseFunction;
	SolContract* m_derivedProgram;

	/// Flag that is true if overridden function is implemented in derived contract
	bool m_implemented = false;
	/// Flag that is true if overridden function implemented in derived contract is
	/// marked virtual
	bool m_virtualized = false;
	/// Flag that is true if overridden function is redeclared but not implemented
	bool m_explicitlyInherited = false;
	/// The uint value to be returned if the overridden interface function is implemented
	std::string m_returnValue;
	DerivedType m_derivedType;

	bool implemented() const
	{
		return m_implemented;
	}

	bool virtualized() const
	{
		return m_virtualized;
	}

	bool explicitlyInherited() const
	{
		return m_explicitlyInherited;
	}

	std::string returnValue() const
	{
		return m_returnValue;
	}
};

/* Difference between interface function declaration and interface
 * function override is that the former can not be implemented and
 * should not be marked virtual i.e., virtual is implicit.
 *
 * Interface function declarations may be implicitly or explicitly
 * inherited by derived interfaces. To explicitly inherit base
 * interface's function declaration, derived base must redeclare
 * the said function and mark it override. If base interface function
 * does not redeclare base interface function, it implicitly inherits
 * it from base and exposes it to its derived interfaces.
 *
 * Interface functions inherited by contracts may be implicitly or
 * explicitly inherited. Derived non abstract contracts must explicitly
 * override and implement inherited interface functions unless they have
 * already been implemented by one of its bases. Abstract contracts
 * may implicitly or explicitly inherit base interface functions. If
 * explicitly inherited, they must be redeclared and marked override.
 * When a base interface function is explicitly inherited by a contract
 * it may be marked virtual.
 */
struct IFunctionOverride
{
	enum class DerivedType
	{
		INTERFACE,
		ABSTRACTCONTRACT,
		CONTRACT
	};

	IFunctionOverride(
		std::shared_ptr<SolInterface> _baseInterface,
		std::shared_ptr<SolInterfaceFunction const> _baseFunction,
		std::variant<SolInterface*, SolContract*> _derivedProgram,
		bool _implement,
		bool _virtual,
		bool _explicitInherit,
		std::string _returnValue
	);

	std::string str() const;
	std::string interfaceStr() const;
	std::string contractStr() const;

	void setImplement()
	{
		m_implemented = true;
	}

	void setVirtual()
	{
		m_virtualized = true;
	}

	void setExplicitInherit()
	{
		m_explicitlyInherited = true;
	}

	bool implemented() const
	{
		return m_implemented;
	}

	bool virtualized() const
	{
		return m_virtualized;
	}

	bool explicitlyInherited() const
	{
		return m_explicitlyInherited;
	}

	std::string returnValue() const
	{
		return m_returnValue;
	}

	std::string baseName() const
	{
		return m_baseInterface->name();
	}

	std::shared_ptr<SolInterface> m_baseInterface;
	std::shared_ptr<SolInterfaceFunction const> m_baseFunction;
	std::variant<SolInterface*, SolContract*> m_derivedProgram;

	/// Flag that is true if overridden function is implemented in derived contract
	bool m_implemented = false;
	/// Flag that is true if overridden function implemented in derived contract is
	/// marked virtual
	bool m_virtualized = false;
	/// Flag that is true if overridden function is redeclared but not implemented
	bool m_explicitlyInherited = false;
	/// The uint value to be returned if the overridden interface function is implemented
	std::string m_returnValue;
	DerivedType m_derivedType;
};
}