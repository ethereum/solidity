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

#include <boost/algorithm/string/case_conv.hpp>

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
	SolFunctionStateMutability _mutability,
	bool _override
)
{
	m_functionName = _functionName;
	m_mutability = _mutability;
	m_override = _override;
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
	string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
		("uint", "0")
		.render();

	return Whiskers(R"(
	function <functionName>() external <stateMutability><?isOverride> override</isOverride>
	<?isVirtual> virtual</isVirtual> returns (uint)<?isImplemented><body><!isImplemented>;</isImplemented>
	)")
		("functionName", name())
		("stateMutability", functionMutability(mutability()))
		("isOverride", override())
		("isVirtual", isVirtual())
		("isImplemented", implement())
		("body", bodyStr)
		.render();
}

SolContractFunction::SolContractFunction(
	ContractFunction const& _function,
	std::string _contractName,
	std::string _functionName,
	std::string _returnValue
)
{
	m_contractName = _contractName;
	m_functionName = _functionName;
	m_visibility = visibilityConverter(_function.vis());
	m_mutability = mutabilityConverter(_function.mut());
	m_virtual = _function.virtualfunc();
	m_returnValue = _returnValue;
}

bool SolContractFunction::operator==(SolContractFunction const& _rhs) const
{
	return this->m_visibility == _rhs.m_visibility && this->m_mutability == _rhs.m_mutability;
}

bool SolContractFunction::operator!=(SolContractFunction const& _rhs) const
{
	return this->m_visibility != _rhs.m_visibility || this->m_mutability != _rhs.m_mutability;
}

string SolContractFunction::str() const
{
	string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
		("uint", returnValue())
		.render();

	return Whiskers(R"(
	function <functionName>()<?isOverride> override</isOverride>
	<?isVirtual> virtual</isVirtual> <visibility> <stateMutability>
	returns (uint)<?isImplemented><body><!isImplemented>;</isImplemented>)")
		("functionName", name())
		("isVirtual", isVirtual())
		("isOverride", "_override")
		("visibility", functionVisibility(visibility()))
		("stateMutability", functionMutability(mutability()))
		("body", bodyStr)
		("isImplemented", "_implement")
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

SolBaseContract::SolBaseContract(ProtoBaseContract _base, string _name, shared_ptr<SolRandomNumGenerator> _prng)
{
	if (auto c = get<Contract const*>(_base))
		m_base.push_back(
			make_shared<SolContract>(SolContract(*c, _name, _prng))
		);
	else if (auto i = get<Interface const*>(_base))
		m_base.push_back(
			make_shared<SolInterface>(SolInterface(*i, _name, _prng))
		);
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
			if (m.first->mutability() != _function->mutability())
				assertThrow(
					false,
					langutil::FuzzerError,
					"Input specifies multiple function overrides with identical names"
					" and parameter types but different mutability."
				);
			// Add new base to list of overridden bases
			m_overrideMap[m.first].push_back(_base);
			multipleOverride = true;
			break;
		}
	}
	// If function has not been overridden, add new override
	if (!multipleOverride)
		m_overrideMap.insert(
			pair(
				make_shared<SolInterfaceFunction>(
					SolInterfaceFunction(
						functionName,
						mutability,
						true
					)
				),
				vector<shared_ptr<SolInterface>>{_base}
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
			overrideHelper(e.first, base);
	}
}

void SolInterface::addBases(Interface const& _interface)
{
	for (auto &b: _interface.bases())
	{
		auto base = make_shared<SolInterface>(SolInterface(b, newBaseName(), m_prng));
		m_baseInterfaces.push_back(base);
		cout << "Added " << base->name() << " as base" << endl;
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
	cout << "Constructing " << _name << endl;
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
					("name", b->name())
					.render();
				if (sep.empty())
					sep = ", ";
			}
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

SolContract::SolContract(Contract const& _contract, std::string _name, shared_ptr<SolRandomNumGenerator> _prng)
{
	m_prng = _prng;
	m_contractName = _name;
	m_abstract = _contract.abstract();
	for (auto &f: _contract.funcdef())
		m_contractFunctions.push_back(
			make_unique<SolContractFunction>(
				SolContractFunction(
					f,
					m_contractName,
					newFunctionName(),
					newReturnValue()
				)
			)
		);
	for (auto &b: _contract.bases())
	{
		switch (b.contract_or_interface_oneof_case())
		{
		case ContractOrInterface::kC:
			m_baseContracts.push_back(
				make_unique<SolBaseContract>(
					SolBaseContract(&b.c(), newContractBaseName(), m_prng)
				)
			);
			break;
		case ContractOrInterface::kI:
			m_baseContracts.push_back(
				make_unique<SolBaseContract>(
					SolBaseContract(&b.i(), newInterfaceBaseName(), m_prng)
				)
			);
			break;
		case ContractOrInterface::CONTRACT_OR_INTERFACE_ONEOF_NOT_SET:
			break;
		}
	}
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

SolLibrary::SolLibrary(Library const& _library, string _name)
{
	m_libraryName = _name;
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

pair<string, string> SolLibrary::pseudoRandomTest(unsigned _randomIdx)
{
	solAssert(m_publicFunctionMap.size() > 0, "Sol proto adaptor: Empty library map");
	string chosenFunction;
	unsigned numFunctions = m_publicFunctionMap.size();
	unsigned functionIndex = _randomIdx % numFunctions;
	unsigned mapIdx = 0;
	for (auto &e: m_publicFunctionMap)
	{
		if (functionIndex == mapIdx)
			chosenFunction = e.first;
		mapIdx++;
	}
	solAssert(m_publicFunctionMap.count(chosenFunction), "Sol proto adaptor: Invalid library function chosen");
	return pair(chosenFunction, m_publicFunctionMap[chosenFunction]);
}

CFunctionOverride::CFunctionOverrideType CFunctionOverride::functionType() const
{
	if (holds_alternative<unique_ptr<SolContractFunction const>>(m_function.second))
		return CFunctionOverrideType::CONTRACT;
	solAssert(interfaceFunction(), "Sol proto fuzzer: Invalid override function type");
	return CFunctionOverrideType::INTERFACE;
}

string CFunctionOverride::name() const
{
	if (holds_alternative<unique_ptr<SolContractFunction const>>(m_function.second))
		return get<unique_ptr<SolContractFunction const>>(m_function.second)->name();
	solAssert(interfaceFunction(), "Sol proto fuzzer: Invalid override function type");
	return get<unique_ptr<SolInterfaceFunction const>>(m_function.second)->name();
}

bool CFunctionOverride::interfaceFunction() const
{
	return functionType() == CFunctionOverrideType::INTERFACE;
}

bool CFunctionOverride::contractFunction() const
{
	return functionType() == CFunctionOverrideType::CONTRACT;
}

SolFunctionVisibility CFunctionOverride::visibility() const
{
	if (contractFunction())
		return get<unique_ptr<SolContractFunction const>>(m_function.second)->visibility();
	solAssert(interfaceFunction(), "Sol proto fuzzer: Invalid override function type");
	return SolFunctionVisibility::EXTERNAL;
}

SolFunctionStateMutability CFunctionOverride::mutability() const
{
	if (contractFunction())
		return get<unique_ptr<SolContractFunction const>>(m_function.second)->mutability();
	solAssert(interfaceFunction(), "Sol proto fuzzer: Invalid override function type");
	return get<unique_ptr<SolInterfaceFunction const>>(m_function.second)->mutability();
}

string CFunctionOverride::str() const
{
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
		("isVirtual", virtualized() || !implemented())
		("visibility", functionVisibility(visibility()))
		("stateMutability", functionMutability(mutability()))
		("body", bodyStr)
		("isImplemented", implemented())
		.render();
}

//string CFunctionOverride::commaSeparatedBaseNames()
//{
//	ostringstream baseNames;
//	string separator{};
//	for (auto &override: m_function)
//	{
//		auto base = override.first;
//		string baseName;
//		if (auto b = get<SolInterface const*>(base))
//			baseName = b->name();
//		else
//			baseName = get<SolContract const*>(base)->name();
//
//		baseNames << Whiskers(R"(<sep><base>)")
//			("sep", separator)
//			("base", baseName)
//			.render();
//		if (separator.empty())
//			separator = ", ";
//	}
//	return baseNames.str();
//}

//string CFunctionOverride::baseName() const
//{
//	auto base = m_function.first;
//	if (contractFunction())
//		return get<shared_ptr<SolContract const>>(base)->name();
//	solAssert(interfaceFunction(), "Sol proto fuzzer: Invalid override function type");
//	return get<shared_ptr<SolInterface const>>(base)->name();
//}