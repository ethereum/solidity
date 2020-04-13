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

#include <test/tools/ossfuzz/SolProtoAdaptor.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Whiskers.h>

#include <sstream>

using namespace solidity::test::solprotofuzzer::adaptor;
using namespace solidity::test::solprotofuzzer;
using namespace std;
using namespace solidity::util;

namespace
{
SolFunctionStateMutability mutabilityConverter(InterfaceFunction_StateMutability _mut)
{
	switch (_mut)
	{
	case InterfaceFunction_StateMutability_PURE:
		return SolFunctionStateMutability::PURE;
	case InterfaceFunction_StateMutability_VIEW:
		return SolFunctionStateMutability::VIEW;
	case InterfaceFunction_StateMutability_PAYABLE:
		return SolFunctionStateMutability::PAYABLE;
	}
}

SolFunctionStateMutability mutabilityConverter(ContractFunction_StateMutability _mut)
{
	switch (_mut)
	{
	case ContractFunction_StateMutability_PURE:
		return SolFunctionStateMutability::PURE;
	case ContractFunction_StateMutability_VIEW:
		return SolFunctionStateMutability::VIEW;
	case ContractFunction_StateMutability_PAYABLE:
		return SolFunctionStateMutability::PAYABLE;
	}
}

SolLibraryFunctionStateMutability mutabilityConverter(LibraryFunction_StateMutability _mut)
{
	switch (_mut)
	{
	case LibraryFunction_StateMutability_PURE:
		return SolLibraryFunctionStateMutability::PURE;
	case LibraryFunction_StateMutability_VIEW:
		return SolLibraryFunctionStateMutability::VIEW;
	}
}

SolFunctionVisibility visibilityConverter(ContractFunction_Visibility _vis)
{
	switch (_vis)
	{
	case ContractFunction_Visibility_PUBLIC:
		return SolFunctionVisibility::PUBLIC;
	case ContractFunction_Visibility_PRIVATE:
		return SolFunctionVisibility::PRIVATE;
	case ContractFunction_Visibility_EXTERNAL:
		return SolFunctionVisibility::EXTERNAL;
	case ContractFunction_Visibility_INTERNAL:
		return SolFunctionVisibility::INTERNAL;
	}
}

SolFunctionVisibility visibilityConverter(LibraryFunction_Visibility _vis)
{
	switch (_vis)
	{
	case LibraryFunction_Visibility_PUBLIC:
		return SolFunctionVisibility::PUBLIC;
	case LibraryFunction_Visibility_PRIVATE:
		return SolFunctionVisibility::PRIVATE;
	case LibraryFunction_Visibility_EXTERNAL:
		return SolFunctionVisibility::EXTERNAL;
	case LibraryFunction_Visibility_INTERNAL:
		return SolFunctionVisibility::INTERNAL;
	}
}

string functionVisibility(SolFunctionVisibility _vis)
{
	switch (_vis)
	{
	case SolFunctionVisibility::PUBLIC:
		return "public";
	case SolFunctionVisibility::PRIVATE:
		return "private";
	case SolFunctionVisibility::EXTERNAL:
		return "external";
	case SolFunctionVisibility::INTERNAL:
		return "internal";
	}
}

string functionMutability(SolFunctionStateMutability _mut)
{
	switch (_mut)
	{
	case SolFunctionStateMutability::PURE:
		return "pure";
	case SolFunctionStateMutability::VIEW:
		return "view";
	case SolFunctionStateMutability::PAYABLE:
		return "payable";
	}
}

string libraryFunctionMutability(SolLibraryFunctionStateMutability _mut)
{
	switch (_mut)
	{
	case SolLibraryFunctionStateMutability::PURE:
		return "pure";
	case SolLibraryFunctionStateMutability::VIEW:
		return "view";
	}
}
}

SolInterfaceFunction::SolInterfaceFunction(
	std::string _functionName,
	SolFunctionStateMutability _mutability
)
{
	m_functionName = _functionName;
	m_mutability = _mutability;
}

bool SolInterfaceFunction::operator==(SolInterfaceFunction const& _rhs) const
{
	// TODO: Change this once we permit arbitrary function parameter types
	return name() == _rhs.name();
}

bool SolInterfaceFunction::operator!=(SolInterfaceFunction const& _rhs) const
{
	// TODO: Change this once we permit arbitrary function parameter types
	return name() != _rhs.name();
}

string SolInterfaceFunction::str() const
{
	return Whiskers(R"(
	function <functionName>() external <stateMutability> returns (uint);)")
		("functionName", name())
		("stateMutability", functionMutability(mutability()))
		.render();
}

SolContractFunction::SolContractFunction(
	ContractFunction const& _function,
	std::string _contractName,
	std::string _functionName,
	bool _implemented,
	std::string _returnValue
)
{
	m_contractName = _contractName;
	m_functionName = _functionName;
	m_visibility = visibilityConverter(_function.vis());
	m_mutability = mutabilityConverter(_function.mut());
	m_returnValue = _returnValue;
	m_implemented = _implemented;
	// Unimplemented contract functions must be marked virtual
	m_virtual = _implemented ? _function.virtualfunc() : true;
}

bool SolContractFunction::operator==(SolContractFunction const& _rhs) const
{
	// TODO: Consider function parameters in addition to name once they are
	// implemented.
	return name() == _rhs.name();
}

bool SolContractFunction::operator!=(SolContractFunction const& _rhs) const
{
	// TODO: Consider function parameters in addition to name once they are
	// implemented.
	return name() != _rhs.name();
}

bool SolContractFunction::disallowed() const
{
	// Private virtual functions are disallowed
	if (visibility() == SolFunctionVisibility::PRIVATE && isVirtual())
		return true;
		// Private payable functions are disallowed
	else if (visibility() == SolFunctionVisibility::PRIVATE && mutability() == SolFunctionStateMutability::PAYABLE)
		return true;
		// Internal payable functions are disallowed
	else if (visibility() == SolFunctionVisibility::INTERNAL && mutability() == SolFunctionStateMutability::PAYABLE)
		return true;
	return false;
}

string SolContractFunction::str() const
{
	if (disallowed())
		return "";

	string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
		("uint", returnValue())
		.render();

	return Whiskers(R"(
	function <functionName>()<?isVirtual> virtual</isVirtual> <visibility> <stateMutability>
	returns (uint)<?isImplemented><body><!isImplemented>;</isImplemented>)")
		("functionName", name())
		("isVirtual", isVirtual())
		("visibility", functionVisibility(visibility()))
		("stateMutability", functionMutability(mutability()))
		("body", bodyStr)
		("isImplemented", implemented())
		.render();
}

SolLibraryFunction::SolLibraryFunction(
	LibraryFunction const& _function,
	std::string _libraryName,
	std::string _functionName,
	std::string _returnValue
)
{
	m_libraryName = _libraryName;
	m_functionName = _functionName;
	m_visibility = visibilityConverter(_function.vis());
	m_mutability = mutabilityConverter(_function.mut());
	m_returnValue = _returnValue;
}

string SolLibraryFunction::str() const
{
	string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
		("uint", returnValue())
		.render();

	return Whiskers(R"(
	function <functionName>(uint) <visibility> <stateMutability> returns (uint)<body>)")
		("functionName", name())
		("visibility", functionVisibility(visibility()))
		("stateMutability", libraryFunctionMutability(mutability()))
		("body", bodyStr)
		.render();
}

unsigned SolBaseContract::functionIndex()
{
	if (type() == BaseType::INTERFACE)
		return interface()->functionIndex();
	else
	{
		solAssert(type() == BaseType::CONTRACT, "Sol proto adaptor: Invalid base contract");
		return contract()->functionIndex();
	}
}

string SolBaseContract::lastBaseName()
{
	if (type() == BaseType::INTERFACE)
		return interface()->lastBaseName();
	else
	{
		solAssert(type() == BaseType::CONTRACT, "Sol proto adaptor: Invalid base contract");
		return contract()->lastBaseName();
	}
}

SolBaseContract::BaseType SolBaseContract::type() const
{
	if (holds_alternative<shared_ptr<SolInterface>>(m_base))
		return BaseType::INTERFACE;
	else
	{
		solAssert(holds_alternative<shared_ptr<SolContract>>(m_base), "Sol proto fuzzer: Invalid base contract");
		return BaseType::CONTRACT;
	}
}

string SolBaseContract::str()
{
	switch (type())
	{
	case BaseType::INTERFACE:
		return interface()->str();
	case BaseType::CONTRACT:
		return contract()->str();
	}
}

string SolBaseContract::name()
{
	if (type() == BaseType::INTERFACE)
		return interface()->name();
	else
	{
		solAssert(type() == BaseType::CONTRACT, "Sol proto adaptor: Invalid base contract");
		return contract()->name();
	}
}

SolBaseContract::SolBaseContract(ProtoBaseContract _base, string _name, shared_ptr<SolRandomNumGenerator> _prng)
{
	if (holds_alternative<Contract const*>(_base))
		m_base = make_shared<SolContract>(SolContract(*get<Contract const*>(_base), _name, _prng));
	else
	{
		solAssert(holds_alternative<Interface const*>(_base), "Sol proto adaptor: Invalid base contract");
		m_base = make_shared<SolInterface>(SolInterface(*get<Interface const*>(_base), _name, _prng));
	}
}

void SolInterface::overrideHelper(
	shared_ptr<SolInterfaceFunction> _function,
	shared_ptr<SolInterface> _base
)
{
	auto functionName = _function->name();
	auto mutability = _function->mutability();
	// Check if two or more bases define this function
	bool multipleOverride = false;
	// If function has already been overridden, add
	// new base to list of overridden bases
	for (auto &m: m_overrideMap)
	{
		// Must override if two or more bases define the
		// same function
		if (m.first->operator==(*_function))
		{
			// Report error if state mutability of identically
			// named functions differ
			if (m.first->mutability() != mutability)
				assertThrow(
					false,
					langutil::FuzzerError,
					"Input specifies multiple function overrides with identical names"
					" and parameter types but different mutability."
				);
#if 0
			cout << "Overriding function " <<
				_function->name() <<
				" explicitly inherited from " <<
				_base->name() <<
				" by " <<
				name() <<
				endl;
#endif

			// Add new base to list of overridden bases
			m_overrideMap[m.first].push_back(
				shared_ptr<IFunctionOverride>(
					make_shared<IFunctionOverride>(
						IFunctionOverride(
							_base,
							_function,
							this,
							false,
							false,
							true,
							""
						)
					)
				)
			);
			multipleOverride = true;
			break;
		}
	}
	// Use a pseudo-random coin flip to decide whether to override explicitly
	// or not. Implicit override means that the overridden function is not
	// redeclared with the override keyword.
	bool explicitOverride = coinToss();
#if 0
	if (explicitOverride)
		cout << "Overriding function " <<
			_function->name() <<
			" explicitly inherited from " <<
			_base->name() <<
			" by " <<
			name() <<
			endl;
#endif

	// If function has not been overridden, add new override pseudo-randomly
	if (!multipleOverride)
		m_overrideMap.insert(
			pair(
				_function,
				vector<shared_ptr<IFunctionOverride>>{
					make_shared<IFunctionOverride>(
						IFunctionOverride(
								_base,
								_function,
								this,
								false,
								false,
								explicitOverride,
								""
						)
					)
				}
			)
		);
}

void SolInterface::addOverrides()
{
	for (auto &base: m_baseInterfaces)
	{
		// Override base interface functions
		for (auto &f: base->m_interfaceFunctions)
			overrideHelper(f, base);
		// Override base interface functions that are themselves overrides
		for (auto &e: base->m_overrideMap)
		{
			solAssert(e.second.size() >= 1, "Sol proto fuzzer: Inconsistent interface override map");
			if (e.second.size() == 1)
			{
				if (e.second[0]->explicitlyInherited())
					overrideHelper(e.first, base);
				else
					overrideHelper(e.first, e.second[0]->m_baseInterface);
			}
			else
			{
				overrideHelper(e.first, base);
			}
		}
	}
}

void SolInterface::addBases(Interface const& _interface)
{
	for (auto &b: _interface.bases())
	{
		auto base = make_shared<SolInterface>(SolInterface(b, newBaseName(), m_prng));
		m_baseInterfaces.push_back(base);
		// Worst case, we override all base functions so we
		// increment derived contract's function index by
		// this amount.
		m_functionIndex += base->functionIndex();
		m_lastBaseName = base->lastBaseName();
	}
}

void SolInterface::addFunctions(Interface const& _interface)
{
	for (auto &f: _interface.funcdef())
		m_interfaceFunctions.push_back(
			make_shared<SolInterfaceFunction>(
				SolInterfaceFunction(
					newFunctionName(),
					mutabilityConverter(f.mut())
				)
			)
		);
}

SolInterface::SolInterface(Interface const& _interface, string _name, shared_ptr<SolRandomNumGenerator> _prng)
{
	m_prng = _prng;
	m_interfaceName = _name;
	m_lastBaseName = m_interfaceName;
	addBases(_interface);
	addOverrides();
	addFunctions(_interface);
}

string SolInterface::baseNames() const
{
	ostringstream bases;
	string separator{};
	for (auto &b: m_baseInterfaces)
	{
		bases << separator << b->name();
		if (separator.empty())
			separator = ", ";
	}
	return bases.str();
}

string SolInterface::baseInterfaceStr() const
{
	ostringstream baseInterfaces;
	for (auto &b: m_baseInterfaces)
		baseInterfaces << b->str();

	return baseInterfaces.str();
}

string SolInterface::overrideStr() const
{
	ostringstream overriddenFunctions;
	for (auto &f: m_overrideMap)
	{
		ostringstream overriddenBaseNames;
		if (f.second.size() > 1)
		{
			string sep{};
			for (auto &b: f.second)
			{
				overriddenBaseNames << Whiskers(R"(<sep><name>)")
					("sep", sep)
					("name", b->baseName())
					.render();
				if (sep.empty())
					sep = ", ";
			}
		}
		else
		{
			solAssert(f.second.size() == 1, "Inconsistent override map");
			if (!f.second[0]->explicitlyInherited())
				continue;
		}
		overriddenFunctions << Whiskers(R"(
	function <functionName>() external <stateMutability> override<?multiple>(<baseNames>)</multiple> returns (uint);)")
			("functionName", f.first->name())
			("stateMutability", functionMutability(f.first->mutability()))
			("multiple", f.second.size() > 1)
			("baseNames", overriddenBaseNames.str())
			.render();
	}
	return overriddenFunctions.str();
}

string SolInterface::str() const
{
	ostringstream functions;
	ostringstream bases;

	// Print overridden functions
	functions << overrideStr();
	// Print non-overridden functions
	for (auto &f: m_interfaceFunctions)
		functions << f->str();

	for (auto &b: m_baseInterfaces)
		bases << b->str();

	return Whiskers(R"(
<bases>
interface <programName><?inheritance> is <baseNames></inheritance> {
<functionDefs>
})")
		("bases", bases.str())
		("programName", name())
		("inheritance", m_baseInterfaces.size() > 0)
		("baseNames", baseNames())
		("functionDefs", functions.str())
		.render();
}

string SolContract::baseNames() const
{
	ostringstream bases;
	string separator{};
	for (auto &b: m_baseContracts)
	{
		bases << separator << b->name();
		if (separator.empty())
			separator = ", ";
	}
	return bases.str();
}

bool SolContract::validTest() const
{
	// Check if at least one contract has one valid test function
	for (auto &c: m_contractFunctionMap)
		if (c.second.size() > 1)
			return true;
	return false;
}

tuple<string, string, string> SolContract::validContractTest()
{
	string chosenContractName{};
	string chosenFunctionName{};
	string expectedOutput{};
	unsigned numContracts = m_contractFunctionMap.size();
	unsigned contractIdx = random() % numContracts;
	unsigned functionIdx = 0;
	unsigned mapIdx = 0;
	for (auto &e: m_contractFunctionMap)
	{
		if (contractIdx == mapIdx)
		{
			// Recurse if chosen contract has no valid test cases
			// We can be sure there is at least one contract with
			// a valid test case because validTest() has been
			// asserted by caller of this function.
			if (e.second.size() == 0)
				return validContractTest();
			else
			{
				chosenContractName = e.first;
				functionIdx = random() % e.second.size();
				unsigned functionMapIdx = 0;
				for (auto &f: e.second)
				{
					if (functionIdx == functionMapIdx)
					{
						chosenFunctionName = f.first;
						expectedOutput = f.second;
						break;
					}
					functionMapIdx++;
				}
				break;
			}
		}
		mapIdx++;
	}
	solAssert(m_contractFunctionMap.count(chosenContractName), "Sol proto adaptor: Invalid contract chosen");
	solAssert(m_contractFunctionMap[chosenContractName].count(chosenFunctionName), "Sol proto adaptor: Invalid contract function chosen");
	return tuple(chosenContractName, chosenFunctionName, expectedOutput);
}

tuple<string, string, string> SolContract::pseudoRandomTest()
{
	solAssert(validTest(), "Sol proto adaptor: No valid contract test cases");
	return validContractTest();
}

void SolContract::interfaceFunctionOverride(
	std::shared_ptr<SolInterface> _base,
	std::shared_ptr<SolInterfaceFunction> _function
)
{
	string functionName = _function->name();
	auto mutability = _function->mutability();

	// Check if two or more bases define this function
	bool multipleOverride = false;
	// If function has already been overridden, add
	// new base to list of overridden bases
	for (auto &m: m_overriddenInterfaceFunctions)
	{
		// Must override if two or more bases define the
		// same function
		if (m.first->operator==(*_function))
		{
			// Report error if state mutability of identically
			// named functions differ
			if (m.first->mutability() != mutability)
				assertThrow(
					false,
					langutil::FuzzerError,
					"Input specifies multiple function overrides with identical names"
					" and parameter types but different mutability."
				);
			// Should interface function be implemented: May be but if not it must be marked virtual
			// Should it be explicitly overridden: Yes
			// Should it be marked virtual: May be
			bool implement = abstract() ? coinToss() : true;
			bool virtualize = coinToss();
			if (abstract() && !implement)
				virtualize = true;
			// Add new base to list of overridden bases
			m_overriddenInterfaceFunctions[m.first].push_back(
				shared_ptr<IFunctionOverride>(
					make_shared<IFunctionOverride>(
						IFunctionOverride(
							_base,
							_function,
							this,
							implement,
							virtualize,
							true,
							newReturnValue()
						)
					)
				)
			);
			multipleOverride = true;
			break;
		}
	}
	// Use a pseudo-random coin toss to decide whether to override explicitly
	// or not. Implicit override means that the overridden function is not
	// redeclared with the override keyword.
	bool explicitOverride = abstract() ? coinToss() : true;
	// If function has not been overridden, add new override pseudo-randomly
	// Should it be virtual: May be but only matters for explicit overrides
	// Should it be implemented: If non abstract, otherwise may be
	bool virtualize = explicitOverride ? coinToss() : false;
	bool implement = abstract() ? coinToss() : true;
	if (abstract() && explicitOverride && !implement)
		virtualize = true;
	if (!multipleOverride)
		m_overriddenInterfaceFunctions.insert(
			pair(
				_function,
				vector<shared_ptr<IFunctionOverride>>{
					make_shared<IFunctionOverride>(
						IFunctionOverride(
							_base,
							_function,
							this,
							implement,
							virtualize,
							explicitOverride,
							newReturnValue()
						)
					)
				}
			)
		);
}

void SolContract::contractFunctionOverride(
	std::shared_ptr<SolContract> _base,
	std::shared_ptr<SolContractFunction> _function
)
{
	string functionName = _function->name();
	auto mutability = _function->mutability();
	auto visibility = _function->visibility();

	// Check if two or more bases define this function
	bool multipleOverride = false;
	// If function has already been overridden, add
	// new base to list of overridden bases
	for (auto &m: m_overriddenContractFunctions)
	{
		// Must override if two or more bases define the
		// same function
		if (m.first->operator==(*_function))
		{
			// Report error if state mutability of identically
			// named functions differ
			if (m.first->mutability() != mutability || m.first->visibility() != visibility)
				assertThrow(
					false,
					langutil::FuzzerError,
					"Input specifies multiple contract function overrides with identical names"
					" and parameter types but different mutability and/or visibility."
				);
			/* Case 1: Base and derived are abstract
			 * Case 2: Base and derived not abstract
			 * Case 3: Derived abstract, base not
			 * Case 4: Derived non abstract, base abstract
			 */
			bool implement = abstract() ? coinToss() : true;
			bool virtualize = coinToss();
			if (abstract() && !implement)
				virtualize = true;
			// Add new base to list of overridden bases
			m_overriddenContractFunctions[m.first].push_back(
				shared_ptr<CFunctionOverride>(
					make_shared<CFunctionOverride>(
						CFunctionOverride(
							_base,
							_function,
							this,
							implement,
							virtualize,
							true,
							newReturnValue()
						)
					)
				)
			);
			multipleOverride = true;
			break;
		}
	}
	bool implement;
	if (_function->implemented())
		implement = true;
	else
		implement = abstract() ? coinToss() : true;
	bool virtualize = coinToss();
	if (!implement && abstract())
		virtualize = true;
	bool explicitOverride = true;
	if (_base->abstract() && !implement && abstract())
		explicitOverride = coinToss();

	if (!multipleOverride)
		m_overriddenContractFunctions.insert(
			pair(
				_function,
				vector<shared_ptr<CFunctionOverride>>{
					make_shared<CFunctionOverride>(
						CFunctionOverride(
							_base,
							_function,
							this,
							implement,
							virtualize,
							explicitOverride,
							newReturnValue()
						)
					)
				}
			)
		);
}

void SolContract::addOverrides()
{
	for (auto &base: m_baseContracts)
	{
		// Check if base is contract or interface
		if (base->type() == SolBaseContract::BaseType::INTERFACE)
		{
			// Override interface functions
			for (auto &f: base->interface()->m_interfaceFunctions)
			{
				interfaceFunctionOverride(base->interface(), f);
			}
			// Override interface implicit and explicit overrides
			for (auto &m: base->interface()->m_overrideMap)
			{
				solAssert(m.second.size() >= 1, "Sol proto adaptor: Inconsistent interface override map");
				if (m.second.size() == 1)
				{
					if (m.second[0]->explicitlyInherited())
						interfaceFunctionOverride(base->interface(), m.first);
					else
						interfaceFunctionOverride(m.second[0]->m_baseInterface, m.first);
				}
				else
				{
					interfaceFunctionOverride(base->interface(), m.first);
				}
			}
		}
		else
		{
			solAssert(base->type() == SolBaseContract::BaseType::CONTRACT, "Sol proto fuzzer: Base contract neither interface nor contract");
			// Override virtual contract functions
			for (auto &f: base->contract()->m_contractFunctions)
			{
				// Override well-defined virtual functions only
				// Well defined means that they are not explicitly disallowed
				// See SolContractFunction::disallowed() for what is disallowed.
				if (!f->disallowed() && f->isVirtual())
					contractFunctionOverride(base->contract(), f);
			}
			// Override contract overrides only if they are virtual.
			for (auto &m: base->contract()->m_overriddenContractFunctions)
			{
				if (!m.first->disallowed())
					for (auto &b: m.second)
						if (b->virtualized())
							contractFunctionOverride(b->m_baseContract, m.first);
			}
			// Override implicit contract overrides from base interface only if
			// - They have been implicitly overridden by base contract OR
			// - They have been explicitly overridden and virtualized but unimplemented
			for (auto &m: base->contract()->m_overriddenInterfaceFunctions)
			{
				solAssert(m.second.size() == 1, "Sol proto fuzzer: Cannot have left multiple override interface function unoverridden");
				if (!m.second[0]->explicitlyInherited() || (m.second[0]->virtualized() && !m.second[0]->implemented()))
					interfaceFunctionOverride(m.second[0]->m_baseInterface, m.first);
			}
		}
	}
	// If any of the interface overrides has same name as one of the contract overrides
	// but they differ in state mutability and/or visibility, we throw to signal invalid
	// contract.
//	for (auto &c: m_overriddenContractFunctions)
//		for (auto &i: m_overriddenInterfaceFunctions)
//			if (i.first->name() == c.first->name() && ((i.first->mutability() != c.first->mutability()) ||
//				c.first->visibility() != SolFunctionVisibility::EXTERNAL))
//				assertThrow(
//					false,
//					langutil::FuzzerError,
//					"Sol proto fuzzer: Interface and contract overrides have same name but "
//								"different visibility and/or state mutability."
//				);
}

void SolContract::addFunctions(Contract const& _contract)
{
	bool abs = abstract();
	string contractName = name();
	// Add contract to contract function map only if the contract
	// is not abstract.
	if (!abs)
		m_contractFunctionMap.insert(pair(contractName, map<string, string>{}));
	// Add functions
	for (auto &f: _contract.funcdef())
	{
		auto function = make_shared<SolContractFunction>(
			SolContractFunction(
				f,
				contractName,
				newFunctionName(),
				(abs ? coinToss() : true),
				newReturnValue()
			)
		);
		m_contractFunctions.push_back(function);
		// If contract is not abstract, add its public and external
		// functions to contract function map.
		if (!abs)
		{
			auto visibility = function->visibility();
			string functionName = function->name();
			string expectedOutput = function->returnValue();
			// Register only public and external contract functions because only they can
			// be called from a different contract.
			if (visibility == SolFunctionVisibility::PUBLIC || visibility == SolFunctionVisibility::EXTERNAL)
			{
				solAssert(!m_contractFunctionMap[contractName].count(functionName), "Sol proto adaptor: Duplicate contract function");
				m_contractFunctionMap[contractName].insert(pair(functionName, expectedOutput));
			}
		}
	}
}

void SolContract::disallowedBase(shared_ptr<SolBaseContract> _base)
{
	if (m_baseContracts.size() == 0)
		return;

	shared_ptr<SolBaseContract> lastBase = m_baseContracts[m_baseContracts.size() - 1];
	auto baseType = _base->type();
	auto lastBaseType = lastBase->type();
	if (baseType == SolBaseContract::BaseType::INTERFACE && lastBaseType == SolBaseContract::BaseType::INTERFACE)
	{
		for (auto &bf: _base->interface()->m_interfaceFunctions)
			for (auto &lbf: lastBase->interface()->m_interfaceFunctions)
				assertThrow(
					bf != lbf,
					langutil::FuzzerError,
					"Sol proto adaptor: New base defines namesake function with different "
						"visibility and/or state mutability"
				);
	}
	else if (baseType == SolBaseContract::BaseType::INTERFACE && lastBaseType == SolBaseContract::BaseType::CONTRACT)
	{
		for (auto &bf: _base->interface()->m_interfaceFunctions)
			for (auto &lbf: lastBase->contract()->m_contractFunctions)
				if (bf->name() == lbf->name() &&
				((bf->mutability() != lbf->mutability()) || (lbf->visibility() != SolFunctionVisibility::EXTERNAL)))
					assertThrow(
						false,
						langutil::FuzzerError,
						"Sol proto adaptor: New base defines namesake function with different "
						"visibility and/or state mutability"
					);
	}
	else if (baseType == SolBaseContract::BaseType::CONTRACT && lastBaseType == SolBaseContract::BaseType::INTERFACE)
	{
		for (auto &bf: _base->contract()->m_contractFunctions)
			for (auto &lbf: lastBase->interface()->m_interfaceFunctions)
				if (bf->name() == lbf->name() &&
				    ((bf->mutability() != lbf->mutability()) || (bf->visibility() != SolFunctionVisibility::EXTERNAL)))
					assertThrow(
						false,
						langutil::FuzzerError,
						"Sol proto adaptor: New base defines namesake function with different "
						"visibility and/or state mutability"
					);
	}
	else
	{
		for (auto &bf: _base->contract()->m_contractFunctions)
			for (auto &lbf: lastBase->contract()->m_contractFunctions)
				assertThrow(
					bf != lbf,
					langutil::FuzzerError,
					"Sol proto adaptor: New base defines namesake function with different "
					"visibility and/or state mutability"
				);
	}
}

void SolContract::addBases(Contract const& _contract)
{
	shared_ptr<SolBaseContract> base;
	for (auto &b: _contract.bases())
	{
		switch (b.contract_or_interface_oneof_case())
		{
		case ContractOrInterface::kC:
			base = make_shared<SolBaseContract>(SolBaseContract(&b.c(), newBaseName(), m_prng));
			break;
		case ContractOrInterface::kI:
			base = make_shared<SolBaseContract>(SolBaseContract(&b.i(), newBaseName(), m_prng));
			break;
		case ContractOrInterface::CONTRACT_OR_INTERFACE_ONEOF_NOT_SET:
			continue;
		}
		// Check if new base defines a namesake function with different
		// visibility and/or state mutability in relation to previous
		// base
		disallowedBase(base);
		m_baseContracts.push_back(base);
		// Worst case, we override all base functions so we
		// increment derived contract's function index by
		// this amount.
		m_functionIndex += base->functionIndex();
		m_lastBaseName = base->lastBaseName();
	}
}

string SolContract::contractOverrideStr() const
{
	ostringstream overriddenFunctions;
	for (auto &f: m_overriddenContractFunctions)
	{
		string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
			("uint", f.second[0]->returnValue())
			.render();

		bool implemented = f.second[0]->implemented();
		bool virtualized = f.second[0]->virtualized();

		ostringstream overriddenBaseNames;
		if (f.second.size() > 1)
		{
			string sep{};
			for (auto &b: f.second)
			{
				overriddenBaseNames << Whiskers(R"(<sep><name>)")
					("sep", sep)
					("name", b->baseName())
					.render();
				if (sep.empty())
					sep = ", ";
			}
		}
		else
		{
			solAssert(f.second.size() == 1, "Inconsistent override map");
			if (!f.second[0]->explicitlyInherited())
				continue;
		}
		overriddenFunctions << Whiskers(R"(
	function <functionName>() <visibility> <stateMutability><?isVirtual> virtual</isVirtual>
	override<?multiple>(<baseNames>)</multiple> returns (uint)<?isImplemented><body><!isImplemented>;</isImplemented>)")
			("functionName", f.first->name())
			("visibility", functionVisibility(f.first->visibility()))
			("stateMutability", functionMutability(f.first->mutability()))
			("isVirtual", virtualized)
			("multiple", f.second.size() > 1)
			("baseNames", overriddenBaseNames.str())
			("isImplemented", implemented)
			("body", bodyStr)
			.render();
	}
	return overriddenFunctions.str();
}

string SolContract::interfaceOverrideStr() const
{
	ostringstream overriddenFunctions;
	for (auto &f: m_overriddenInterfaceFunctions)
	{
		string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
			("uint", f.second[0]->returnValue())
			.render();

		// Check override class for implementation and virtualized
		// flags and print appropriately.
		bool implemented = f.second[0]->implemented();
		bool virtualized = f.second[0]->virtualized();

		ostringstream overriddenBaseNames;
		if (f.second.size() > 1)
		{
			string sep{};
			for (auto &b: f.second)
			{
				overriddenBaseNames << Whiskers(R"(<sep><name>)")
					("sep", sep)
					("name", b->baseName())
					.render();
				if (sep.empty())
					sep = ", ";
			}
		}
		else
		{
			solAssert(f.second.size() == 1, "Inconsistent override map");
			if (!f.second[0]->explicitlyInherited())
				continue;
		}
		overriddenFunctions << Whiskers(R"(
	function <functionName>() external <stateMutability><?isVirtual> virtual</isVirtual>
	override<?multiple>(<baseNames>)</multiple> returns (uint)<?isImplemented><body><!isImplemented>;</isImplemented>)")
			("functionName", f.first->name())
			("stateMutability", functionMutability(f.first->mutability()))
			("isVirtual", virtualized)
			("multiple", f.second.size() > 1)
			("baseNames", overriddenBaseNames.str())
			("isImplemented", implemented)
			("body", bodyStr)
			.render();
	}
	return overriddenFunctions.str();
}

string SolContract::str() const
{
	ostringstream bases;
	for (auto &b: m_baseContracts)
		bases << b->str();

	ostringstream functions;

	// Print overridden functions
	functions << interfaceOverrideStr() << contractOverrideStr();

	// Print non-overridden functions
	for (auto &f: m_contractFunctions)
		functions << f->str();

	return Whiskers(R"(
<bases>
<?isAbstract>abstract </isAbstract>contract <contractName><?inheritance> is <baseNames></inheritance> {
<functions>
})")
		("bases", bases.str())
		("isAbstract", abstract())
		("contractName", name())
		("inheritance", m_baseContracts.size() > 0)
		("baseNames", baseNames())
		("functions", functions.str())
		.render();
}

SolContract::SolContract(
	Contract const& _contract,
	std::string _name,
	shared_ptr<SolRandomNumGenerator> _prng
)
{
	m_prng = _prng;
	m_contractName = _name;
	m_lastBaseName = m_contractName;
	m_abstract = _contract.abstract();
	addBases(_contract);
	addOverrides();
	addFunctions(_contract);
}

void SolLibrary::addFunction(LibraryFunction const& _function)
{
	// Register function name and return value
	string functionName = newFunctionName();
	string outputStr = newReturnValue();

	SolFunctionVisibility visibility = visibilityConverter(_function.vis());

	if (visibility == SolFunctionVisibility::PUBLIC || visibility == SolFunctionVisibility::EXTERNAL)
	{
		solAssert(!m_publicFunctionMap.count(functionName), "Sol proto adapter: Duplicate library function");
		m_publicFunctionMap.insert(pair(functionName, outputStr));
	}

	// Create and add function to library
	m_functions.push_back(
		make_unique<SolLibraryFunction>(
			SolLibraryFunction(_function,
			                   name(),
			                   functionName,
			                   outputStr
			)
		)
	);
}

SolLibrary::SolLibrary(Library const& _library, string _name, shared_ptr<SolRandomNumGenerator> _prng)
{
	m_libraryName = _name;
	m_prng = _prng;
	for (LibraryFunction const& f: _library.funcdef())
		addFunction(f);
}

string SolLibrary::str() const
{
	ostringstream functions;

	for (auto &f: m_functions)
		functions << f->str();

	return Whiskers(R"(
library <name> {
<functions>
})")
		("name", name())
		("functions", functions.str())
		.render();
}

bool SolLibrary::validTest() const
{
	return m_publicFunctionMap.size() > 0;
}

pair<string, string> SolLibrary::pseudoRandomTest()
{
	solAssert(m_publicFunctionMap.size() > 0, "Sol proto adaptor: Empty library map");
	string chosenFunction;
	unsigned numFunctions = m_publicFunctionMap.size();
	unsigned functionIndex = randomNumber() % numFunctions;
	unsigned mapIdx = 0;
	for (auto &e: m_publicFunctionMap)
	{
		if (functionIndex == mapIdx)
		{
			chosenFunction = e.first;
			break;
		}
		mapIdx++;
	}
	solAssert(m_publicFunctionMap.count(chosenFunction), "Sol proto adaptor: Invalid library function chosen");
	return pair(chosenFunction, m_publicFunctionMap[chosenFunction]);
}

string CFunctionOverride::name() const
{
	return m_baseFunction->name();
}

SolFunctionVisibility CFunctionOverride::visibility() const
{
	return m_baseFunction->visibility();
}

SolFunctionStateMutability CFunctionOverride::mutability() const
{
	return m_baseFunction->mutability();
}

string CFunctionOverride::str() const
{
	solAssert(virtualized() || !implemented(), "Sol proto fuzzer: Invalid virtualization of contract function override");

	string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
		("uint", returnValue())
		.render();

	return Whiskers(R"(
	function <functionName>() override <?isVirtual> virtual</isVirtual> <visibility> <stateMutability>
		returns (uint)<?isImplemented><body><!isImplemented>;</isImplemented>)")
		("functionName", name())
		("isVirtual", virtualized())
		("visibility", functionVisibility(visibility()))
		("stateMutability", functionMutability(mutability()))
		("body", bodyStr)
		("isImplemented", implemented())
		.render();
}

string CFunctionOverride::baseName() const
{
	return m_baseContract->name();
}

IFunctionOverride::IFunctionOverride(
	std::shared_ptr<SolInterface> _baseInterface,
	std::shared_ptr<SolInterfaceFunction const> _baseFunction,
	std::variant<SolInterface*, SolContract*> _derivedProgram,
	bool _implement,
	bool _virtual,
	bool _explicitInherit,
	std::string _returnValue
)
{
	if (std::holds_alternative<SolContract*>(_derivedProgram))
	{
		auto derived = std::get<SolContract*>(_derivedProgram);
		m_derivedType = derived->abstract() ? DerivedType::ABSTRACTCONTRACT : DerivedType::CONTRACT;
		if (!derived->abstract())
			solAssert(
				_explicitInherit && !_returnValue.empty() && _implement,
				"Contract overrides base interface function either without"
				" implementing it or implictly overrides."
			);
		else
			if (_explicitInherit)
				solAssert(
					_virtual || (_implement && !_returnValue.empty()),
					"Abstract contract overrides base interface function either"
					" without implementing it or without marking it virtual."
				);
	}
	else
	{
		solAssert(
			holds_alternative<SolInterface*>(_derivedProgram),
			"Derived program neither an interface nor a contract"
		);
		m_derivedType = DerivedType::INTERFACE;
		if (_explicitInherit)
			solAssert(
				!_virtual && !_implement && _returnValue.empty(),
				"Interface overrides base interface function with invalid parameters."
			);
	}

	m_baseInterface = _baseInterface;
	m_baseFunction = _baseFunction;
	m_derivedProgram = _derivedProgram;
	m_implemented = _implement;
	m_virtualized = _virtual;
	m_explicitlyInherited = _explicitInherit;
	m_returnValue = _returnValue;
}

std::string IFunctionOverride::str() const
{
	switch (m_derivedType)
	{
	case DerivedType::INTERFACE:
		return interfaceStr();
	case DerivedType::ABSTRACTCONTRACT:
	case DerivedType::CONTRACT:
		return contractStr();
	}
}

string IFunctionOverride::interfaceStr() const
{
	return Whiskers(R"(
	function <functionName>() external <mutability> override returns(uint);
)")
		("functionName", m_baseFunction->name())
		("mutability", functionMutability(m_baseFunction->mutability()))
		.render();
}

string IFunctionOverride::contractStr() const
{
	string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
		("uint", returnValue())
		.render();

	return Whiskers(R"(
	function <functionName>() external <mutability> override
		<?isVirtual> virtual</isVirtual> returns(uint)<?isImplemented><body><!isImplemented>;</isImplemented>
)")
		("functionName", m_baseFunction->name())
		("mutability", functionMutability(m_baseFunction->mutability()))
		("isVirtual", virtualized())
		("isImplemented", implemented())
		("body", bodyStr)
		.render();
}