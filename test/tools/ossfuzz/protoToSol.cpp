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

#include <test/tools/ossfuzz/protoToSol.h>
#include <test/tools/ossfuzz/SolProtoAdaptor.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Whiskers.h>

#include <sstream>

using namespace solidity::test::solprotofuzzer;
using namespace solidity::test::solprotofuzzer::adaptor;
using namespace std;
using namespace solidity::util;

string ProtoConverter::protoToSolidity(Program const& _p)
{
	m_randomGen = make_shared<SolRandomNumGenerator>(_p.seed());
	return visit(_p);
}

string ProtoConverter::visit(TestContract const& _testContract)
{
	string testCode;
	string usingLibDecl;
	m_libraryTest = false;

	switch (_testContract.type())
	{
	case TestContract::LIBRARY:
	{
		if (emptyLibraryTests())
		{
			testCode = Whiskers(R"(
		return 0;)")
				.render();
		}
		else
		{
			m_libraryTest = true;
			auto testTuple = pseudoRandomLibraryTest(_testContract.programidx());
			m_libraryName = get<0>(testTuple);
			usingLibDecl = Whiskers(R"(
	using <libraryName> for uint;)")
				("libraryName", get<0>(testTuple))
				.render();
			testCode = Whiskers(R"(
		uint x;
		if (x.<testFunction>() != <expectedOutput>)
			return 1;
		return 0;)")
				("testFunction", get<1>(testTuple))
				("expectedOutput", get<2>(testTuple))
				.render();
		}
		break;
	}
	case TestContract::CONTRACT:
#if 0
		testCode = Whiskers(R"(
		<contractName> testContract = new <contractName>();
		if (testContract.<testFunction>() != <expectedOutput>)
			return 1;
		return 0;)")
			("contractName", "")
			("testFunction", "")
			("expectedOutput", "")
			.render();
#else
		testCode = Whiskers(R"(
		return 0;)")
				.render();
#endif
		break;
	}

	return Whiskers(R"(
contract C {<?isLibrary><usingDecl></isLibrary>
	function test() public returns (uint)
	{<testCode>
	}
}
)")
		("isLibrary", m_libraryTest)
		("usingDecl", usingLibDecl)
		("testCode", testCode)
		.render();
}

bool ProtoConverter::libraryTest() const
{
	return m_libraryTest;
}

string ProtoConverter::libraryName() const
{
	return m_libraryName;
}

string ProtoConverter::visit(Program const& _p)
{
	ostringstream program;
	ostringstream contracts;

	for (auto &contract: _p.contracts())
		contracts << visit(contract);

	program << Whiskers(R"(
pragma solidity >=0.0;

<contracts>

<testContract>
)")
	("contracts", contracts.str())
	("testContract", visit(_p.test()))
	.render();
	return program.str();
}

string ProtoConverter::visit(ContractType const& _contractType)
{
	m_mostDerivedAbstractContract = false;
	switch (_contractType.contract_type_oneof_case())
	{
	case ContractType::kC:
		m_mostDerivedAbstractContract = _contractType.c().abstract();
		m_mostDerivedProgram = MostDerivedProgram::CONTRACT;
//		return visit(_contractType.c());
		return "";
	case ContractType::kL:
		m_mostDerivedProgram = MostDerivedProgram::LIBRARY;
		return visit(_contractType.l());
	case ContractType::kI:
		m_mostDerivedProgram = MostDerivedProgram::INTERFACE;
		return visit(_contractType.i());
	case ContractType::CONTRACT_TYPE_ONEOF_NOT_SET:
		return "";
	}
}

string ProtoConverter::visit(ContractOrInterface const& _contractOrInterface)
{
	switch (_contractOrInterface.contract_or_interface_oneof_case())
	{
	case ContractOrInterface::kC:
//		return visit(_contractOrInterface.c());
		return "";
	case ContractOrInterface::kI:
//		return visit(_contractOrInterface.i());
		return "";
	case ContractOrInterface::CONTRACT_OR_INTERFACE_ONEOF_NOT_SET:
		return "";
	}
}

bool ProtoConverter::contractFunctionImplemented(
	Contract const* _contract,
	CIFunc _function
)
{
	auto v = m_contractFunctionMap[_contract];
	for (auto &e: v)
		if (get<0>(e) == _function)
			return get<1>(e);
	return false;
}

bool ProtoConverter::contractFunctionVirtual(
	Contract const* _contract,
	CIFunc _function
)
{
	auto v = m_contractFunctionMap[_contract];
	for (auto &e: v)
		if (get<0>(e) == _function)
			return get<2>(e);
	return false;
}

#if 0
string ProtoConverter::mostDerivedInterfaceOverrides(Interface const& _interface)
{
	ostringstream funcs;

	for (auto base = _interface.bases().rbegin(); base != _interface.bases().rend(); base++)
	{
		for (auto& f: base->funcdef())
		{
			// An interface may override base interface's function
			bool override = pseudoRandomCoinFlip();
			if (!override)
				continue;
			funcs << overrideFunction(&f, false, false);
		}
		funcs << mostDerivedInterfaceOverrides(*base);
	}
	return funcs.str();
}

string ProtoConverter::mostDerivedContractOverrides(Interface const& _interface)
{
	ostringstream funcs;

	for (auto base = _interface.bases().rbegin(); base != _interface.bases().rend(); base++)
	{
		for (auto& f: base->funcdef())
		{
			bool override = pseudoRandomCoinFlip() || mostDerivedProgramAbstractContract();

			/// We can arrive here from contract through other contracts and interfaces.
			/// We define most derived contract (MDC) as the most derived contract in
			/// the inheritence chain to which the visited interface belongs to.

			/// When MDC is not abstract we must implement (with override) all
			/// interface functions that MDC inherits unless it has been implemented
			/// by some other contract in the inheritence chain that MDC also derives
			/// from.
			/// In other words, if an interface function has never been implemented
			/// thus far in the inheritence chain, it must be implemented.
			/// If it has already been implemented, it may be reimplemented provided
			/// it is virtual.
			/// When reimplementing, it may be revirtualized.

			/// When MDC is abstract, we may or may not redeclare interface function.
			/// If interface function has been implemented in the inheritence chain,
			/// and we redeclare it, we must reimplement it.
			/// If inheritence function has not been implemented and we redeclare it,
			/// we may implement it.
			/// If we implement it, it may be marked virtual.
			/// If we don't implement it, it must be marked virtual.

			/// . We may virtualize.
			/// We create a pair <interfaceFunction, virtualized> and add it to list of
			/// implemented interface functions.

			/// When IAC is abstract:
			/// - we may redeclare
			/// - if redeclared, we may implement
			/// - if we do not implement redeclared function then:
			///  - we must override and virtualize
			///  - add to list of unimplemented interface functions
			/// - if we implement then:
			///  - we must override
			///  - we may virtualize
			///  - create a pair <interfaceFunction, virtualized> and add it to list of
			/// implemented interface functions.


			///   - ancestor contract is not abstract and this interface function
			/// has been implemented by some other contract in the traversal path
			///	that also virtualizes the function.
			/// When we override, we may mark it as virtual.

			/// We may override when ancestor contract is abstract
			/// If ancestor contract is overriding, we may or may not implement it.
			/// If we do not implement it, we must mark it virtual since otherwise
			/// we are left with unimplementable function.
			/// If we implement it, we may mark it as virtual.
			string funcStr = visit(
				f,
				index++,
				override,
				programName(&*base)
			);

			if (override)
				funcs << funcStr;
		}
		funcs << mostDerivedContractOverrides(*base);
	}
	return funcs.str();
}

pair<bool, bool> ProtoConverter::contractFunctionParams(
	Contract const* _contract,
	ContractFunction const* _function
)
{
	// If contract is abstract, we may implement this function. If contract
	// is not abstract, we must implement this function.
	bool implement = !_contract->abstract() || pseudoRandomCoinFlip();
	// We may mark a non-overridden function as virtual.
	// We must mark an unimplemented abstract contract function as
	// virtual.
	bool virtualFunc = _function->virtualfunc() || (_contract->abstract() && !implement);
	return pair(implement, virtualFunc);
}

pair<bool, bool> ProtoConverter::contractFunctionOverrideParams(
	Contract const* _base,
	Contract const* _derived,
	CIFunc _f
)
{
	bool baseAbstract = _base->abstract();
	bool derivedAbstract = _derived->abstract();

	bool implement = false;
	bool revirtualize = false;

	// There are four possibilities here:
	// 1. both base and derived are abstract,
	// 2. base abstract, derived not
	// 3. base not abstract, derived is
	// 4. both base and derived are not abstract
	if (baseAbstract && derivedAbstract)
	{
		// Case 1: Both base and derived are abstract
		// virtual base functions may be redeclared (with override)
		// if redeclared virtual base function has been implemented, it must be reimplemented but
		// may be revirtualized
		// if revirtualized, there is nothing to be changed in list of <ContractFunction, bool>
		// if not revirtualized, we changed the boolean to false in the list of implemented contract
		// functions.
		// if redeclared virtual base function has not been implemented, it may be implemented.
		// if it is implemented (with override), it may be revirtualized.
		// if it is not implemented, it must be marked virtual.
		// if virtual base function not redeclared, there is no status change
		bool virtualImplemented = contractFunctionImplemented(_base, _f);
		implement = virtualImplemented || pseudoRandomCoinFlip();
		revirtualize = (!implement && !virtualImplemented) || pseudoRandomCoinFlip();
	}
	else if (baseAbstract && !derivedAbstract)
	{
		// Case 2: Base abstract, derived not
		// If base function appears in list of unimplemented virtual contract functions, we
		// must implement it (with override). Remove from unimplemented virtual contract functions. We may
		// revirtualize. Add to list of implemented contract functions.

		// Unimplemented virtual functions must be implemented (with override)

		// Implemented virtual functions may be implemented (with override)

		//
	}
	else if (!baseAbstract && derivedAbstract)
	{
		// Case 3: Base not abstract, derived is
		// All base functions are implemented. Base functions marked virtual may be overridden.
		// If they are overridden they must be reimplemented. They may be revirtualized.
		// Base functions that are not virtual may not be redeclared.
	}
	else
	{
		// Case 4: Neither base nor derived are abstract
		// All base functions are implemented. Base functions marked virtual may be overridden.
		// If they are overridden they must be reimplemented. They may be revirtualized.
		// Base functions that are not virtual may not be redeclared.
	}
	return pair(implement, revirtualize);
}

string ProtoConverter::traverseOverrides(Contract const& _contract)
{
	ostringstream funcs;

	for (auto base = _contract.bases().rbegin(); base != _contract.bases().rend(); base++)
	{
		if (base->contract_or_interface_oneof_case() == ContractOrInterface::CONTRACT_OR_INTERFACE_ONEOF_NOT_SET)
			continue;

		if (base->has_c())
		{
			for (auto& f: base->c().funcdef())
			{
				// We may redeclare virtual functions in base contract
				bool redeclareVirtual = f.virtualfunc() && pseudoRandomCoinFlip();
				// If base function is not virtual or if we choose to not
				// redeclare the virtual function, we skip to the next function
				// after incrementing the function index.
				if (!f.virtualfunc() || !redeclareVirtual)
					continue;

				// Check if overridden function may be implemented/revirtualized
				// by the derived contract.
				auto [implement, revirtualize] = contractFunctionOverrideParams(
					&base->c(),
                    &_contract,
                    &f
                );
				funcs << overrideFunction(&f, revirtualize, implement);
				// Update contract function map
				m_contractFunctionMap[&_contract].push_back(CITuple(&f, implement, revirtualize));
			}
			// Override revirtualized functions.
			// TODO: Function that returns all revirtualized functions and their implementation
			// status.
			for (auto &tuple: m_contractFunctionMap[&base->c()])
			{
				auto function = get<0>(tuple);
				bool overrideRevirtualized = true;
				if (holds_alternative<ContractFunction const*>(function))
				{
					auto contractFunction = get<ContractFunction const*>(function);

					auto [implementRevirtualized, revirtualizeRevirtualized] = contractFunctionOverrideParams(
						&base->c(),
						&_contract,
						get<0>(tuple)
					);
					funcs << visit(
						*contractFunction,
						index++,
						overrideRevirtualized,
						revirtualizeRevirtualized,
						implementRevirtualized,
						programName(&base->c())
					);
				}
			}
			// Traverse base
			funcs << traverseOverrides(base->c());
		}
		else if (base->has_i())
		{
			for (auto& f: base->i().funcdef())
			{
				bool implement = pseudoRandomCoinFlip();
				m_counter += base->i().funcdef_size();
				bool override = pseudoRandomCoinFlip();

				if (_contract.abstract() && !implement && !override)
					continue;

				funcs << overrideFunction(
					&f,
					!_contract.abstract() || implement,
					pseudoRandomCoinFlip() || (override && _contract.abstract())
				);
			}
			funcs << mostDerivedContractOverrides(base->i(), !_contract.abstract());
		}
	}
	return funcs.str();
}

/// This function is called when root is interface.
tuple<string, string, string> ProtoConverter::visitMostDerivedInterface(Interface const& _interface)
{
	ostringstream bases;
	ostringstream baseNames;
	ostringstream funcs;

	string separator{};
	for (auto &base: _interface.bases())
	{
		string baseStr = visit(base);
		if (baseStr.empty())
			continue;
		bases << baseStr;
		baseNames << separator
		          << programName(&base);
		if (separator.empty())
			separator = ", ";
	}

	// First define overridden functions
	bool overrides = _interface.bases_size() > 0 && !baseNames.str().empty();
	if (overrides)
		funcs << mostDerivedInterfaceOverrides(_interface);

	unsigned index = 0;
	// Define non-overridden functions
	for (auto &f: _interface.funcdef())
		funcs << registerAndVisitFunction(
			&_interface,
			&f,
			index++,
			false,
			false,
			false
		);

	return make_tuple(bases.str(), baseNames.str(), funcs.str());
}

void ProtoConverter::registerFunctionName(CIL _program, CILFunc _function, unsigned _index)
{
	string pName = programName(_program);
	string fName = createFunctionName(_program, _index);
	solAssert(!m_functionNameMap.count(_function), "Sol proto fuzzer: Duplicate function registration");
	m_functionNameMap.insert(pair(_function, fName));
}

string ProtoConverter::registerAndVisitFunction(
	CIL _program,
	CILFunc _function,
	unsigned _index,
	bool _override,
	bool _virtual,
	bool _implement
)
{
	registerFunctionName(_program, _function, _index);
	return visit(_function, _override, _virtual, _implement);
}

tuple<string, string, string> ProtoConverter::visitProgramHelper(CIL _program)
{
	ostringstream bases;
	ostringstream funcs;
	ostringstream baseNames;

	string pName = programName(_program);

	string separator{};
	if (holds_alternative<Contract const*>(_program))
	{
		Contract const* contract = get<Contract const*>(_program);

		for (auto &base: contract->bases())
		{
			string baseStr = visit(base);
			if (baseStr.empty())
				continue;
			bases << baseStr;
			baseNames << separator
			            << (base.has_c() ? programName(&base.c()) : programName(&base.i()));
			if (separator.empty())
				separator = ", ";
		}

		// First define overridden functions
		bool overrides = contract->bases_size() > 0 && !baseNames.str().empty();
		if (overrides)
		{
			funcs << traverseOverrides(*contract);
		}

		// Declare/define non-overridden functions
		unsigned index = 0;
		for (auto &f: contract->funcdef())
		{
			auto [implement, virtualize] = contractFunctionParams(contract, &f);

			funcs << registerAndVisitFunction(
				_program,
				&f,
				index++,
				false,
				virtualize,
				implement
			);

			// Update contract function map
			m_contractFunctionMap[contract].push_back(CITuple(&f, implement, virtualize));
		}
	}
	else if (holds_alternative<Interface const*>(_program))
	{
		// If we are here, it means most derived program is a contract.

		auto interface = get<Interface const*>(_program);
		for (auto &base: interface->bases())
		{
			string baseStr = visit(base);
			if (baseStr.empty())
				continue;
			bases << baseStr;
			baseNames << separator
			          << programName(&base);
			if (separator.empty())
				separator = ", ";
		}

		// First define overridden functions
		bool overrides = interface->bases_size() > 0 && !baseNames.str().empty();
		if (overrides)
		{
			funcs << mostDerivedContractOverrides(*interface, false);
		}

		unsigned index = 0;
		// Declare non-overridden functions
		for (auto &f: interface->funcdef())
			funcs << registerAndVisitFunction(
				_program,
				&f,
				index++,
				false,
				false,
				false
			);
	}
	else
	{
		auto library = get<Library const*>(_program);
		unsigned index = 0;
		for (auto &f: library->funcdef())
			funcs << registerAndVisitFunction(
				_program,
				&f,
				index++,
				false,
				false,
				false
			);
	}
	return make_tuple(bases.str(), baseNames.str(), funcs.str());
}

string ProtoConverter::visit(Contract const& _contract)
{
	openProgramScope(&_contract);
	auto [bases, baseNames, funcs] = visitProgramHelper(&_contract);
	return Whiskers(R"(
<bases>
<?isAbstract>abstract </isAbstract>contract <programName><?inheritance> is <baseNames></inheritance> {
<functionDefs>
})")
		("bases", bases)
		("isAbstract", _contract.abstract())
		("programName", programName(&_contract))
		("inheritance", _contract.bases_size() > 0 && !baseNames.empty())
		("baseNames", baseNames)
		("functionDefs", funcs)
		.render();
}
#endif

string ProtoConverter::visit(Interface const& _interface)
{
	if (_interface.funcdef_size() == 0 && _interface.bases_size() == 0)
		return "";

	openProgramScope(&_interface);
	try {
		auto interface = SolInterface(_interface, programName(&_interface), m_randomGen);
		return interface.str();
	}
	catch (langutil::FuzzerError const&)
	{
		// Return empty string if input specification is invalid.
		return "";
	}
}

string ProtoConverter::visit(Library const& _library)
{
	if (emptyLibrary(_library))
		return "";

	openProgramScope(&_library);
	auto lib = SolLibrary(_library, programName(&_library));
	if (lib.validTest())
	{
		auto libTestPair = lib.pseudoRandomTest(_library.random());
		m_libraryTests.push_back({lib.name(), libTestPair.first, libTestPair.second});
	}
	return lib.str();
}

tuple<string, string, string> ProtoConverter::pseudoRandomLibraryTest(unsigned _randomIdx)
{
	solAssert(m_libraryTests.size() > 0, "Sol proto fuzzer: No library tests found");
	unsigned index = _randomIdx % m_libraryTests.size();
	return m_libraryTests[index];
}

void ProtoConverter::openProgramScope(CIL _program)
{
	string programNamePrefix;
	if (holds_alternative<Contract const*>(_program))
		programNamePrefix = "C";
	else if (holds_alternative<Interface const*>(_program))
		programNamePrefix = "I";
	else
		programNamePrefix = "L";
	string programName = programNamePrefix + to_string(m_numPrograms++);
	m_programNameMap.insert(pair(_program, programName));

	if (holds_alternative<Contract const*>(_program))
		m_contractFunctionMap.insert(pair(get<Contract const*>(_program), vector<CITuple>{}));
}

string ProtoConverter::programName(CIL _program)
{
	solAssert(m_programNameMap.count(_program), "Sol proto fuzzer: Unregistered program");
	return m_programNameMap[_program];
}

unsigned ProtoConverter::randomNumber()
{
	solAssert(m_randomGen, "Sol proto fuzzer: Uninitialized random number generator");
	return m_randomGen->operator()();
}

#if 0
bool ProtoConverter::disallowedContractFunction(SolContractFunction const& _contractFunction, bool _isVirtual)
{
	string visibility = functionVisibility(_contractFunction.m_visibility);
	string mutability = functionMutability(_contractFunction.m_mutability);

	// Private virtual functions are disallowed
	if (visibility == "private" && _isVirtual)
		return true;
	// Private payable functions are disallowed
	else if (visibility == "private" && mutability == "payable")
		return true;
	// Internal payable functions are disallowed
	else if (visibility == "internal" && mutability == "payable")
		return true;
	return false;
}

string ProtoConverter::functionName(CILFunc _function)
{
	solAssert(m_functionNameMap.count(_function), "Sol proto fuzzer: Unregistered function");
	return m_functionNameMap[_function];
}
#endif