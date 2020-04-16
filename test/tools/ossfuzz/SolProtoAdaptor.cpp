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
using namespace solidity::util;
using namespace std;

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

void SolInterfaceFunction::markExplicitOverride(string _contractName)
{
	m_type = Type::EXPLICITOVERRIDE;
	m_overriddenFrom.clear();
	m_contractName = _contractName;
	m_overriddenFrom.push_back(m_contractName);
}

SolInterfaceFunction::SolInterfaceFunction(
	string _functionName,
	SolFunctionStateMutability _mutability,
	Type _type,
	string _contractName
)
{
	m_functionName = _functionName;
	m_mutability = _mutability;
	m_type = _type;
	m_overriddenFrom.push_back(_contractName);
	m_contractName = _contractName;
}

bool SolInterfaceFunction::namesake(SolContractFunction const& _rhs) const
{
	// TODO: Change this once we permit arbitrary function parameter types
	return name() == _rhs.name();
}

bool SolInterfaceFunction::namesake(SolInterfaceFunction const& _rhs) const
{
	// TODO: Change this once we permit arbitrary function parameter types
	return name() == _rhs.name();
}

bool SolInterfaceFunction::operator==(SolInterfaceFunction const& _rhs) const
{
	solAssert(namesake(_rhs), "Sol proto adaptor: Cannot compare two interface functions with different names");
	return m_mutability == _rhs.m_mutability;
}

bool SolInterfaceFunction::operator!=(SolInterfaceFunction const& _rhs) const
{
	solAssert(namesake(_rhs), "Sol proto adaptor: Comparing two interface functions with different names is redundant");
	return m_mutability != _rhs.m_mutability;
}

void SolInterfaceFunction::merge(SolInterfaceFunction const& _rhs)
{
	assertThrow(
		this->operator==(_rhs),
		langutil::FuzzerError,
		"Sol proto adaptor: Invalid inheritance hierarchy"
	);
	m_type = Type::EXPLICITOVERRIDE;
	for (auto &b: _rhs.m_overriddenFrom)
		m_overriddenFrom.push_back(b);
#if 0
	std::cout << "overridden base names " << overriddenFromBaseNames() << std::endl;
#endif
}

bool SolInterfaceFunction::operator==(SolContractFunction const& _rhs) const
{
	// TODO: Change this once we permit arbitrary function parameter types
	return name() == _rhs.name();
}

bool SolInterfaceFunction::operator!=(SolContractFunction const& _rhs) const
{
	// TODO: Change this once we permit arbitrary function parameter types
	return name() != _rhs.name();
}

string SolInterfaceFunction::overriddenFromBaseNames() const
{
	ostringstream nameStr;
	string separator{};
	for (auto &b: m_overriddenFrom)
	{
		nameStr << separator << b;
		if (separator.empty())
			separator = ", ";
	}
	return nameStr.str();
}

string SolInterfaceFunction::str() const
{
	if (explicitOverride())
		return Whiskers(R"(
	function <functionName>() override<?isMultiple>(<baseNames>)</isMultiple>
	external <stateMutability> returns (uint);)")
		("functionName", name())
		("isMultiple", multipleBases())
		("baseNames", overriddenFromBaseNames())
		("stateMutability", functionMutability(mutability()))
		.render();
	else if (memberFunction())
		return Whiskers(R"(
	function <functionName>() external <stateMutability> returns (uint);)")
			("functionName", name())
			("stateMutability", functionMutability(mutability()))
			.render();
	else
		return "";
}

string SolContractFunction::overriddenFromBaseNames() const
{
	ostringstream nameStr;
	string separator{};
	for (auto &b: m_overriddenFrom)
	{
		nameStr << separator << b;
		if (separator.empty())
			separator = ", ";
	}
	return nameStr.str();
}

void SolContractFunction::merge(SolContractFunction const& _rhs)
{
	assertThrow(
		this->operator==(_rhs),
		langutil::FuzzerError,
		"Sol proto adaptor: Invalid inheritance hierarchy"
	);
	m_type = Type::EXPLICITOVERRIDECONTRACT;
	for (auto &b: _rhs.m_overriddenFrom)
		m_overriddenFrom.push_back(b);
}

void SolContractFunction::merge(SolInterfaceFunction const& _rhs)
{
	assertThrow(
		this->operator==(_rhs),
		langutil::FuzzerError,
		"Sol proto adaptor: Invalid inheritance hierarchy"
	);
	m_type = Type::EXPLICITOVERRIDEINTERFACE;
	for (auto &b: _rhs.m_overriddenFrom)
		m_overriddenFrom.push_back(b);
}

SolContractFunction::SolContractFunction(
	vector<string> _overriddenFrom,
	SolFunctionStateMutability _mutability,
	std::string _contractName,
	std::string _functionName,
	Type _type,
	bool _implemented,
	bool _virtual,
	std::string _returnValue,
	SolFunctionVisibility _vis
)
{
	for (auto &s: _overriddenFrom)
		m_overriddenFrom.push_back(s);
	m_contractName = _contractName;
	m_functionName = _functionName;
	m_type = _type;
	m_implemented = _implemented;
	m_returnValue = _returnValue;
	m_visibility = _vis;
	m_mutability = _mutability;
	// Unimplemented contract functions must be marked virtual
	if (explicitOverride() || memberFunction())
	{
		if (_implemented)
			m_virtual = _virtual;
		else
			m_virtual = true;
	}
}

SolContractFunction::SolContractFunction(
	ContractFunction const& _function,
	std::string _contractName,
	std::string _functionName,
	Type _type,
	bool _implemented,
	std::string _returnValue
)
{
	m_contractName = _contractName;
	m_overriddenFrom.push_back(m_contractName);
	m_functionName = _functionName;
	m_visibility = visibilityConverter(_function.vis());
	m_mutability = mutabilityConverter(_function.mut());
	m_type = _type;
	m_returnValue = _returnValue;
	m_implemented = _implemented;
	// Unimplemented contract functions must be marked virtual
	if (explicitOverride() || memberFunction())
	{
		if (_implemented)
			m_virtual = _function.virtualfunc();
		else
			m_virtual = true;
	}
	// TODO: Perhaps not hard fail here. Silently ignoring instead.
	assertThrow(
		!disallowed(),
		langutil::FuzzerError,
		"Sol proto adaptor: Invalid parameters for a contract function e.g., private+virtual"
	);
}

bool SolContractFunction::namesake(SolContractFunction const& _rhs) const
{
	return name() == _rhs.name();
}

bool SolContractFunction::namesake(SolInterfaceFunction const& _rhs) const
{
	return name() == _rhs.name();
}

bool SolContractFunction::operator==(SolContractFunction const& _rhs) const
{
	solAssert(namesake(_rhs), "Sol proto adaptor: Cannot compare two contract functions with different names");
	return m_mutability == _rhs.m_mutability && m_visibility == _rhs.m_visibility;
}

bool SolContractFunction::operator!=(SolContractFunction const& _rhs) const
{
	solAssert(namesake(_rhs), "Sol proto adaptor: Comparing two contract functions with different names is redundant");
	return m_mutability != _rhs.m_mutability || m_visibility != _rhs.m_visibility;
}

bool SolContractFunction::operator==(SolInterfaceFunction const& _rhs) const
{
	solAssert(namesake(_rhs), "Sol proto adaptor: Cannot compare a contract function with a differently named interface function");
	return m_mutability == _rhs.m_mutability && m_visibility == SolFunctionVisibility::EXTERNAL;
}

bool SolContractFunction::operator!=(SolInterfaceFunction const& _rhs) const
{
	solAssert(namesake(_rhs), "Sol proto adaptor: Cannot compare a contract function with a differently named interface function");
	return m_mutability != _rhs.m_mutability || m_visibility != SolFunctionVisibility::EXTERNAL;
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
	if (implicitOverride())
		return "";

	bool override = false;
	bool multiOverride = false;
	if (explicitOverride())
	{
		override = true;
		multiOverride = m_overriddenFrom.size() > 1;
	}
	string bodyStr = Whiskers(R"(
	{
		return <uint>;
	})")
		("uint", returnValue())
		.render();

	return Whiskers(R"(
	function <functionName>()<?isVirtual> virtual</isVirtual> <visibility> <stateMutability>
	<?isOverride>override<?isMultiple>(<baseNames>)</isMultiple></isOverride>
	returns (uint)<?isImplemented><body><!isImplemented>;</isImplemented>)")
		("functionName", name())
		("isVirtual", isVirtual())
		("visibility", functionVisibility(visibility()))
		("stateMutability", functionMutability(mutability()))
		("isOverride", override)
		("isMultiple", multiOverride)
		("baseNames", overriddenFromBaseNames())
		("isImplemented", implemented())
		("body", bodyStr)
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

void SolInterface::merge()
{
	/* Merge algorithm:
	 * 1. Deep copy all base interface functions (local) into a single list (global)
	 * 2. Mark all of these as implicit overrides
	 * 3. Iterate list of implicit overrides
	 *   3a. If n-way merge is necessary, do so and mark a two-base explicit override and add to contract
	 *   3b. If n-way merge is not possible, add as implicit override to contract
	 * 4. Update ownership of n-way merges
	 * 5. Iterate list of contract implicit and explicit (2-way) overrides
	 *   5a. If implicit, pseudo randomly mark it explicit
	 */

	// Step 1-2
	vector<shared_ptr<SolInterfaceFunction>> global{};
	for (auto &base: m_baseInterfaces)
	{
		vector<shared_ptr<SolInterfaceFunction>> local{};
		for (auto &bf: base->m_functions)
			local.push_back(make_shared<SolInterfaceFunction>(*bf));
		for (auto &l: local)
		{
			// Reset override history for past n-way merge
			if (l->explicitOverride() && l->numOverriddenFromBases() > 1)
				l->resetOverriddenBases();
			// Mark all as implicit overrides
			l->markImplicitOverride();
			global.push_back(l);
		}
	}
	// Step 3
	vector<shared_ptr<SolInterfaceFunction>> updateList;
	for (auto &f: global)
	{
#if 0
		std::cout << "Processing " << f->name() << " from " << f->m_contractName << std::endl;
#endif
		bool merged = false;
		for (auto &e: m_functions)
		{
			if (e->namesake(*f))
			{
#if 0
				std::cout << "n-way merge of " << f->name() << " from " << f->m_contractName << std::endl;
#endif
				e->merge(*f);
				updateList.push_back(e);
				merged = true;
				break;
			}
		}
		if (!merged)
			m_functions.push_back(f);
	}
	// Step 4
	for (auto &u: updateList)
		u->m_contractName = name();
	// Step 5
	for (auto &e: m_functions)
		if (e->implicitOverride() && coinToss())
			e->markExplicitOverride(name());
}

void SolInterface::addBases(Interface const& _interface)
{
	for (auto &b: _interface.bases())
	{
		auto base = make_shared<SolInterface>(SolInterface(b, newBaseName(), m_prng));
		m_baseInterfaces.push_back(base);
		// Do bookkeeping to keep function and base numbering
		// consistent
		m_functionIndex += base->functionIndex();
		m_lastBaseName = base->lastBaseName();
	}
	merge();
}

void SolInterface::addFunctions(Interface const& _interface)
{
	for (auto &f: _interface.funcdef())
		m_functions.push_back(
			make_shared<SolInterfaceFunction>(
				SolInterfaceFunction(
					newFunctionName(),
					mutabilityConverter(f.mut()),
					SolInterfaceFunction::Type::MEMBERFUNCTION,
					name()
				)
			)
		);
}

SolInterface::SolInterface(
	Interface const& _interface,
	string _name,
	shared_ptr<SolRandomNumGenerator> _prng
)
{
	m_prng = _prng;
	m_interfaceName = _name;
	m_lastBaseName = m_interfaceName;
	addBases(_interface);
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

string SolInterface::functionStr() const
{
	ostringstream functions;

	for (auto &f: m_functions)
		functions << f->str();
	return functions.str();
}

string SolInterface::str() const
{
	return Whiskers(R"(
<bases>
interface <programName><?inheritance> is <baseNames></inheritance> {
<functionDefs>
})")
		("bases", baseInterfaceStr())
		("programName", name())
		("inheritance", bases())
		("baseNames", baseNames())
		("functionDefs", functionStr())
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

SolBaseContract::SolBaseContract(
	ProtoBaseContract _base,
	string _name,
	shared_ptr<SolRandomNumGenerator> _prng
)
{
	if (holds_alternative<Contract const*>(_base))
		m_base = make_shared<SolContract>(
			SolContract(*get<Contract const*>(_base), _name, _prng)
		);
	else
	{
		solAssert(holds_alternative<Interface const*>(_base), "Sol proto adaptor: Invalid base contract");
		m_base = make_shared<SolInterface>(
			SolInterface(*get<Interface const*>(_base), _name, _prng)
		);
	}
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

bool SolContract::validTest()
{
	// If we have at least one function-expectation pair in this contract
	// we have a valid test
	if (!m_contractFunctionMap.empty())
		return true;
	else
	{
		// If this contract does not offer a valid test, we check if its
		// base contracts offer one.
		for (auto &b: m_baseContracts)
		{
			if (b->type() == SolBaseContract::BaseType::CONTRACT)
			{
				if (b->contract()->validTest())
					return true;
			}
		}
		return false;
	}
}

/// Must be called only when validTest() returns true.
void SolContract::validContractTests(map<string, map<string, string>>& _testSet)
{
	string chosenContractName;
	string chosenFunctionName{};
	string expectedOutput{};
	// Add test cases in current contract
	if (!m_contractFunctionMap.empty())
	{
		chosenContractName = name();
		_testSet[chosenContractName] = map<string, string>{};
		for (auto &e: m_contractFunctionMap)
			_testSet[chosenContractName].insert(make_pair(e.first, e.second));
	}
	// Continue search in base contracts
	for (auto &b: m_baseContracts)
		if (b->type() == SolBaseContract::BaseType::CONTRACT)
			if (b->contract()->validTest())
				b->contract()->validContractTests(_testSet);
}

//tuple<string, string, string> SolContract::pseudoRandomTest()
//{
//	solAssert(validTest(), "Sol proto adaptor: Please call validTest()");
//	return validContractTests();
//}

void SolContract::addFunctions(Contract const& _contract)
{
	bool abs = abstract();
	string contractName = name();
	// Add functions
	for (auto &f: _contract.funcdef())
	{
		auto function = make_shared<SolContractFunction>(
			SolContractFunction(
				f,
				contractName,
				newFunctionName(),
				SolContractFunction::Type::MEMBERFUNCTION,
				(abs ? coinToss() : true),
				newReturnValue()
			)
		);
		m_functions.push_back(function);
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
				solAssert(!m_contractFunctionMap.count(functionName), "Sol proto adaptor: Duplicate contract function");
				m_contractFunctionMap.insert(pair(functionName, expectedOutput));
			}
		}
	}
}

void SolContract::merge()
{
	/* Merge algorithm:
	 * 1. Deep copy all base interface functions (local) into a single list (global)
	 * 2. Mark all of these as implicit overrides
	 * 3. Iterate list of implicit overrides
	 *   3a. If n-way merge is necessary, do so and mark a two-base explicit override and add to contract
	 *   3b. If n-way merge is not possible, add as implicit override to contract
	 * 4. Update ownership of n-way merges
	 * 5. Iterate list of contract implicit and explicit (2-way) overrides
	 *   5a. If implicit, pseudo randomly mark it explicit
	 */
	FunctionList global{};
	// Step 1-2
	for (auto& base: m_baseContracts) {
		auto baseType = base->type();
		solAssert(
			baseType == SolBaseContract::BaseType::INTERFACE || baseType == SolBaseContract::BaseType::CONTRACT,
			"Sol proto adaptor: Invalid base contract"
		);
		FunctionList local{};
		if (baseType == SolBaseContract::BaseType::INTERFACE)
			for (auto& f: base->interface()->m_functions)
				local.push_back(make_shared<SolInterfaceFunction>(*f));
		else
			// Contract functions may be implicitly inherited
			// interface function or contract function.
			for (auto& f: base->contract()->m_functions) {
				if (holds_alternative<shared_ptr<SolInterfaceFunction>>(f)) {
					local.push_back(
						make_shared<SolInterfaceFunction>(
							*get<shared_ptr<SolInterfaceFunction>>(f)
						)
					);
				} else {
					local.push_back(
						make_shared<SolContractFunction>(
							*get<shared_ptr<SolContractFunction>>(f)
						)
					);
				}
			}

		for (auto& item: local) {
			if (holds_alternative<shared_ptr<SolInterfaceFunction>>(item)) {
				auto function = get<shared_ptr<SolInterfaceFunction>>(item);
				// Reset override history for past n-way merge
				if (function->explicitOverride() && function->numOverriddenFromBases() > 1)
					function->resetOverriddenBases();
				// Mark interface function as an implicit override
				function->markImplicitOverride();
				global.push_back(item);
			} else {
				auto function = get<shared_ptr<SolContractFunction>>(item);
				// Reset override history for past n-way merge
				if (function->explicitOverride() && function->numOverriddenFromBases() > 1)
					function->resetOverriddenBases();
				if (!function->implicitOverride())
					function->markImplicitContractOverride();
				global.push_back(item);
			}
		}
	}
	// Step 3
	FunctionList updateList;
	for (auto& functionInBaseContract: global) {
#if 0
		std::cout << "Processing " << f->name() << " from " << f->m_contractName << std::endl;
#endif
		bool merged = false;
		shared_ptr<SolContractFunction> mergedContract;
		for (auto& functionInContract: m_functions) {
			if (holds_alternative<shared_ptr<SolInterfaceFunction>>(functionInContract)) {
				auto function = get<shared_ptr<SolInterfaceFunction>>(functionInContract);
				// Merge interface into another interface and create a new contract function
				if (holds_alternative<shared_ptr<SolInterfaceFunction>>(functionInBaseContract)) {
					auto g = get<shared_ptr<SolInterfaceFunction>>(functionInBaseContract);
					if (function->namesake(*g)) {
#if 0
						std::cout << "Interface function merged into interface function" << std::endl;
						std::cout << "n-way merge of " << g->name() << " from " << g->m_contractName << std::endl;
#endif
						function->merge(*g);
						// Create new contract function
						mergedContract = make_shared<SolContractFunction>(
							SolContractFunction(
								function->m_overriddenFrom,
								function->mutability(),
								name(),
								function->name(),
								SolContractFunction::Type::EXPLICITOVERRIDEINTERFACE,
								abstract() ? coinToss() : true,
								coinToss(),
								newReturnValue()
							)
						);
						// Erase merged interface function
						auto pos = find(m_functions.begin(), m_functions.end(), functionInContract);
						m_functions.erase(pos);
						m_functions.insert(pos, mergedContract);
						merged = true;
						break;
					}
				}
				// Merge contract function into interface function
				else {
					auto g = get<shared_ptr<SolContractFunction>>(functionInBaseContract);
					if (function->namesake(*g)) {
#if 0
						std::cout << "Contract function merged into interface function" << std::endl;
						std::cout << "n-way merge of " << g->name() << " from " << g->m_contractName << std::endl;
#endif
						// Assert contract function is virtual
						assertThrow(
							g->isVirtual(),
							langutil::FuzzerError,
							"Sol proto fuzzer: n-way merge of non-virtual contract function is not possible"
						);
						// Merge interface into contract instead of the other way round
						g->merge(*function);
						// If abstract contract, we may implement
						bool implement = abstract() ? coinToss() : true;
						// If merged function has already been implemented
						// we must implement.
						if (g->implemented() && !implement)
							implement = true;
						// Create new contract function
						mergedContract = make_shared<SolContractFunction>(
							SolContractFunction(
								g->m_overriddenFrom,
								g->mutability(),
								name(),
								g->name(),
								SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
								implement,
								coinToss(),
								newReturnValue(),
								g->visibility()
							)
						);
						// Erase merged interface function
						auto pos = find(m_functions.begin(), m_functions.end(), functionInContract);
						m_functions.erase(pos);
						m_functions.insert(pos, mergedContract);
						merged = true;
						break;
					}
				}
			}
			// Merge interface/contract into contract
			else {
				auto function = get<shared_ptr<SolContractFunction>>(functionInContract);
				// Merge interface into contract function
				if (holds_alternative<shared_ptr<SolInterfaceFunction>>(functionInBaseContract)) {
					auto g = get<shared_ptr<SolInterfaceFunction>>(functionInBaseContract);
					if (function->namesake(*g)) {
#if 0
						std::cout << "Interface function merged into contract function" << std::endl;
						std::cout << "n-way merge of " << g->name() << " from " << g->m_contractName << std::endl;
#endif
						// Assert contract function is virtual
						assertThrow(
							function->isVirtual(),
							langutil::FuzzerError,
							"Sol proto fuzzer: n-way merge of non-virtual contract function is not possible"
						);
						function->merge(*g);
#if 0
						std::cout << "Overridden from " << function->overriddenFromBaseNames() << std::endl;
#endif
						// If abstract contract, we may implement
						bool implement = abstract() ? coinToss() : true;
						// If merged function has already been implemented
						// we must implement.
						if (function->implemented() && !implement)
							implement = true;

						// Create new contract function
						mergedContract = make_shared<SolContractFunction>(
							SolContractFunction(
								function->m_overriddenFrom,
								function->mutability(),
								name(),
								function->name(),
								SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
								implement,
								coinToss(),
								newReturnValue(),
								function->visibility()
							)
						);
						// Erase merged interface function
						auto pos = find(m_functions.begin(), m_functions.end(), functionInContract);
						m_functions.erase(pos);
						m_functions.insert(pos, mergedContract);
						merged = true;
						break;
					}
				}
				// Merge contract function into contract function
				else {
					auto g = get<shared_ptr<SolContractFunction>>(functionInBaseContract);
					if (function->namesake(*g)) {
#if 0
						std::cout << "Contract function merged into contract function" << std::endl;
						std::cout << "n-way merge of " << g->name() << " from " << g->m_contractName << std::endl;
#endif
						// Assert contract functions are virtual
						assertThrow(
							g->isVirtual() && function->isVirtual(),
							langutil::FuzzerError,
							"Sol proto fuzzer: n-way merge of non-virtual contract function is not possible"
						);
						// Check if at least one base implements function
						bool atleastOneImplements = g->implemented() || function->implemented();
						function->merge(*g);
						// If abstract contract, we may implement
						bool implement = abstract() ? coinToss() : true;
						// If merged function has already been implemented by at
						// least one of the bases, we must implement.
						if (atleastOneImplements && !implement)
							implement = true;

						// Create new contract function
						mergedContract = make_shared<SolContractFunction>(
							SolContractFunction(
								function->m_overriddenFrom,
								function->mutability(),
								name(),
								function->name(),
								SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
								implement,
								coinToss(),
								newReturnValue(),
								function->visibility()
							)
						);
						// Erase merged interface function
						auto pos = find(m_functions.begin(), m_functions.end(), functionInContract);
						m_functions.erase(pos);
						m_functions.insert(pos, mergedContract);
						merged = true;
						break;
					}
				}
			}
		}
		if (!merged)
			m_functions.push_back(functionInBaseContract);
	}
	// Step 4
	for (auto& u: updateList) {
		if (holds_alternative<shared_ptr<SolInterfaceFunction>>(u))
			get<shared_ptr<SolInterfaceFunction>>(u)->m_contractName = name();
		else
			get<shared_ptr<SolContractFunction>>(u)->m_contractName = name();
	}
	// Step 5:
	for (auto& e: m_functions)
	{
		// All implicitly inherited interface functions must be explicitly inherited
		// and implemented if not abstract.
		if (holds_alternative<shared_ptr<SolInterfaceFunction>>(e))
		{
			auto t = get<shared_ptr<SolInterfaceFunction>>(e);
			if (t->implicitOverride())
			{
#if 0
				std::cout << "Explicitly overriding " << t->name() << " from " << t->m_contractName << std::endl;
#endif
				auto pos = find(m_functions.begin(), m_functions.end(), e);
				auto contractOverride = make_shared<SolContractFunction>(
					SolContractFunction(
						{name()},
						t->mutability(),
						name(),
						t->name(),
						SolContractFunction::Type::EXPLICITOVERRIDEINTERFACE,
						abstract() ? coinToss() : true,
						coinToss(),
						newReturnValue()
					)
				);
#if 0
				std::cout << "New function basenames are " << contractOverride->overriddenFromBaseNames() << std::endl;
#endif
				m_functions.erase(pos);
#if 0
				std::cout << "Size of function list after erase " << m_functions.size() << std::endl;
#endif
				m_functions.insert(pos, contractOverride);
#if 0
				std::cout << "Size of function list after insert " << m_functions.size() << std::endl;
#endif
			}
		}
		else
		{
			auto t = get<shared_ptr<SolContractFunction>>(e);
			if (t->implicitOverride() && t->isVirtual())
			{
				// If not abstract, all unimplemented functions must be implemented
				if (!abstract())
				{
					if (!t->implemented())
					{
						// This function must be from an abstract contract
						// Create a new contract function with an implementation
						auto contractOverride = make_shared<SolContractFunction>(
							SolContractFunction(
								{name()},
								t->mutability(),
								name(),
								t->name(),
								SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
								true,
								coinToss(),
								newReturnValue(),
								t->visibility()
							)
						);
						auto pos = find(m_functions.begin(), m_functions.end(), e);
						m_functions.erase(pos);
						m_functions.insert(pos, contractOverride);
					}
					// Implemented virtual base contract function
					else
					{
						if (coinToss())
						{
							// Reimplement
							auto contractOverride = make_shared<SolContractFunction>(
								SolContractFunction(
									{name()},
									t->mutability(),
									name(),
									t->name(),
									SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
									true,
									coinToss(),
									newReturnValue(),
									t->visibility()
								)
							);
							auto pos = find(m_functions.begin(), m_functions.end(), e);
							m_functions.erase(pos);
							m_functions.insert(pos, contractOverride);
						}
					}
				}
				// Abstract contract
				else
				{
					// Virtual implemented base
					if (t->implemented())
					{
						// If we are explicitly overriding an implemented contract function
						// we must implement it.
						if (coinToss())
						{
							auto contractOverride = make_shared<SolContractFunction>(
								SolContractFunction(
									{name()},
									t->mutability(),
									name(),
									t->name(),
									SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
									true,
									coinToss(),
									newReturnValue(),
									t->visibility()
								)
							);
							auto pos = find(m_functions.begin(), m_functions.end(), e);
							m_functions.erase(pos);
							m_functions.insert(pos, contractOverride);
						}
					}
					// Unimplemented base contract function virtual implicit
					else
					{
						// Option 1: do nothing
						// Option 2: override and virtualize and not implement
						// Option 3: override and implement and optionally virtualize
						switch (randomNumber() % 3)
						{
							case 0:
								break;
							case 1:
							{
								auto contractOverride = make_shared<SolContractFunction>(
									SolContractFunction(
										{name()},
										t->mutability(),
										name(),
										t->name(),
										SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
										false,
										true,
										"",
										t->visibility()
									)
								);
								auto pos = find(m_functions.begin(), m_functions.end(), e);
								m_functions.erase(pos);
								m_functions.insert(pos, contractOverride);
								break;
							}
							case 2:
							{
								auto contractOverride = make_shared<SolContractFunction>(
									SolContractFunction(
										{name()},
										t->mutability(),
										name(),
										t->name(),
										SolContractFunction::Type::EXPLICITOVERRIDECONTRACT,
										true,
										coinToss(),
										newReturnValue(),
										t->visibility()
									)
								);
								auto pos = find(m_functions.begin(), m_functions.end(), e);
								m_functions.erase(pos);
								m_functions.insert(pos, contractOverride);
								break;
							}
						}
					}
				}
			}
		}
	}
	// Step 6
	if (!abstract())
		for (auto &f: m_functions)
			if (holds_alternative<shared_ptr<SolContractFunction>>(f))
			{
				auto function = get<shared_ptr<SolContractFunction>>(f);
				if (function->implemented() && (function->visibility() == SolFunctionVisibility::EXTERNAL || function->visibility() == SolFunctionVisibility::PUBLIC))
					m_contractFunctionMap.insert(pair(function->name(), function->returnValue()));
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
			base = make_shared<SolBaseContract>(
				SolBaseContract(&b.c(), newBaseName(), m_prng)
			);
			break;
		case ContractOrInterface::kI:
			base = make_shared<SolBaseContract>(
				SolBaseContract(&b.i(), newBaseName(), m_prng)
			);
			break;
		case ContractOrInterface::CONTRACT_OR_INTERFACE_ONEOF_NOT_SET:
			continue;
		}
		m_baseContracts.push_back(base);
		// Worst case, we override all base functions so we
		// increment derived contract's function index by
		// this amount.
		m_functionIndex += base->functionIndex();
		m_lastBaseName = base->lastBaseName();
	}
	merge();
}

string SolContract::str()
{
	ostringstream bases;
	for (auto &b: m_baseContracts)
		bases << b->str();

	ostringstream functions;

	// Print functions
	for (auto &f: m_functions)
		if (holds_alternative<shared_ptr<SolContractFunction>>(f))
			functions << get<shared_ptr<SolContractFunction>>(f)->str();

	return Whiskers(R"(
<bases>
<?isAbstract>abstract </isAbstract>contract <contractName><?inheritance> is <baseNames></inheritance> {
<functions>
})")
		("bases", bases.str())
		("isAbstract", abstract())
		("contractName", name())
		("inheritance", !m_baseContracts.empty())
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
	// Initialize function map.
	m_contractFunctionMap = map<string, string>{};
	addBases(_contract);
	addFunctions(_contract);
}

// Library implementation
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
	return !m_publicFunctionMap.empty();
}

pair<string, string> SolLibrary::pseudoRandomTest()
{
	solAssert(!m_publicFunctionMap.empty(), "Sol proto adaptor: Empty library map");
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