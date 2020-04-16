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
	ostringstream testCode;
	string usingLibDecl;
	m_libraryTest = false;

	switch (_testContract.type())
	{
	case TestContract::LIBRARY:
	{
		if (emptyLibraryTests())
		{
			testCode << Whiskers(R"(
		return 0;)")
				.render();
		}
		else
		{
			m_libraryTest = true;
			auto testTuple = pseudoRandomLibraryTest();
			m_libraryName = get<0>(testTuple);
			usingLibDecl = Whiskers(R"(
	using <libraryName> for uint;)")
				("libraryName", get<0>(testTuple))
				.render();
			testCode << Whiskers(R"(
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
		if (emptyContractTests())
			testCode << Whiskers(R"(
		return 0;)")
				.render();
		else
		{
			unsigned errorCode = 1;
			unsigned contractVarIndex = 0;
			for (auto &testTuple: m_contractTests)
			{
				// Do this to avoid stack too deep errors
				// We require uint as a return var, so we
				// cannot have more than 16 variables without
				// running into stack too deep errors
				if (contractVarIndex >= s_maxVars)
					break;
				string contractName = testTuple.first;
				string contractVarName = "tc" + to_string(contractVarIndex);
				testCode << Whiskers(R"(
			<contractName> <contractVarName> = new <contractName>();)")
					("contractName", contractName)
					("contractVarName", contractVarName)
					.render();
				for (auto &t: testTuple.second)
				{
					testCode << Whiskers(R"(
			if (<contractVarName>.<testFunction>() != <expectedOutput>)
				return <errorCode>;)")
						("contractVarName", contractVarName)
						("testFunction", t.first)
						("expectedOutput", t.second)
						("errorCode", to_string(errorCode))
						.render();
					errorCode++;
				}
				contractVarIndex++;
			}
			// Expected return value
			testCode << Whiskers(R"(
			return 0;)").render();
		}
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
		("testCode", testCode.str())
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
	switch (_contractType.contract_type_oneof_case())
	{
	case ContractType::kC:
		return visit(_contractType.c());
	case ContractType::kL:
		return visit(_contractType.l());
	case ContractType::kI:
		return visit(_contractType.i());
	case ContractType::CONTRACT_TYPE_ONEOF_NOT_SET:
		return "";
	}
}

string ProtoConverter::visit(Contract const& _contract)
{
	if (_contract.funcdef_size() == 0 && _contract.bases_size() == 0)
		return "";

	openProgramScope(&_contract);
	try {
		auto contract = SolContract(_contract, programName(&_contract), m_randomGen);
		if (contract.validTest())
		{
			map<string, map<string, string>> testSet;
			contract.validContractTests(testSet);
			for (auto &contractTestSet: testSet)
			{
				m_contractTests.insert(pair(contractTestSet.first, map<string, string>{}));
				for (auto &contractTest: contractTestSet.second)
					m_contractTests[contractTestSet.first].insert(
						make_pair(contractTest.first, contractTest.second)
					);
			}
			return contract.str();
		}
		// There is no point in generating a contract that can not provide
		// a valid test case, so we simply bail.
		else
		{
			std::cout << contract.str() << std::endl;
			return "";
		}
	}
	catch (langutil::FuzzerError const& error)
	{
		std::cout << error.what() << std::endl;
		// Return empty string if input specification is invalid.
		return "";
	}
}

string ProtoConverter::visit(Interface const& _interface)
{
	if (_interface.funcdef_size() == 0 && _interface.bases_size() == 0)
		return "";

	openProgramScope(&_interface);
	try {
		auto interface = SolInterface(_interface, programName(&_interface), m_randomGen);
		return interface.str();
	}
	catch (langutil::FuzzerError const& error)
	{
		std::cout << error.what() << std::endl;
		// Return empty string if input specification is invalid.
		return "";
	}
}

string ProtoConverter::visit(Library const& _library)
{
	if (emptyLibrary(_library))
		return "";

	openProgramScope(&_library);
	auto lib = SolLibrary(_library, programName(&_library), m_randomGen);
	if (lib.validTest())
	{
		auto libTestPair = lib.pseudoRandomTest();
		m_libraryTests.push_back({lib.name(), libTestPair.first, libTestPair.second});
	}
	return lib.str();
}

tuple<string, string, string> ProtoConverter::pseudoRandomLibraryTest()
{
	solAssert(m_libraryTests.size() > 0, "Sol proto fuzzer: No library tests found");
	unsigned index = randomNumber() % m_libraryTests.size();
	return m_libraryTests[index];
}

//tuple<string, string, string> ProtoConverter::pseudoRandomContractTest()
//{
//	solAssert(m_contractTests.size() > 0, "Sol proto fuzzer: No contract tests found");
//	unsigned index = randomNumber() % m_contractTests.size();
//	return m_contractTests[index];
//}

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