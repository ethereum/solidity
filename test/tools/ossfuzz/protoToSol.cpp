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

#include <liblangutil/Exceptions.h>

#include <libsolutil/Whiskers.h>

#include <sstream>

using namespace solidity::test::solprotofuzzer;
using namespace solidity::util;
using namespace std;

string ProtoConverter::protoToSolidity(Program const& _p)
{
	// Create random number generator with fuzzer supplied
	// seed.
	m_randomGen = make_shared<SolRandomNumGenerator>(_p.seed());
	return visit(_p);
}

pair<string, string> ProtoConverter::generateTestCase(TestContract const& _testContract)
{
	ostringstream testCode;
	string usingLibDecl;
	switch (_testContract.type())
	{
	case TestContract::LIBRARY:
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
			return 1;)")
			("testFunction", get<1>(testTuple))
			("expectedOutput", get<2>(testTuple))
			.render();
		break;
	}
	case TestContract::CONTRACT:
	{
		unsigned errorCode = 1;
		unsigned contractVarIndex = 0;
		for (auto& testTuple: m_contractTests)
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
			for (auto& t: testTuple.second)
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
		break;
	}
	}
	// Expected return value when all tests pass
	testCode << Whiskers(R"(
		return 0;)")
		.render();
	return pair(usingLibDecl, testCode.str());
}

string ProtoConverter::visit(TestContract const& _testContract)
{
	string testCode;
	string usingLibDecl;
	m_libraryTest = false;

	// Simply return valid uint (zero) if there are
	// no tests.
	if (emptyLibraryTests() || emptyContractTests())
		testCode = Whiskers(R"(
		return 0;)")
			.render();
	else
		tie(usingLibDecl, testCode) = generateTestCase(_testContract);

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
	openProgramScope(&_contract);
	return "";
}

string ProtoConverter::visit(Interface const& _interface)
{
	openProgramScope(&_interface);
	return "";
}

string ProtoConverter::visit(Library const& _library)
{
	openProgramScope(&_library);
	return "";
}

tuple<string, string, string> ProtoConverter::pseudoRandomLibraryTest()
{
	solAssert(m_libraryTests.size() > 0, "Sol proto fuzzer: No library tests found");
	unsigned index = randomNumber() % m_libraryTests.size();
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
	string programName = programNamePrefix + to_string(m_programNumericSuffix++);
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