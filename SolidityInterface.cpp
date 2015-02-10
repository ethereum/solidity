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
 * @date 2015
 * Unit tests for generating source interfaces for Solidity contracts.
 */

#include <boost/test/unit_test.hpp>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/AST.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

class SolidityInterfaceChecker
{
public:
	SolidityInterfaceChecker(): m_compilerStack(false) {}

	/// Compiles the given code, generates the interface and parses that again.
	ContractDefinition const& checkInterface(string const& _code, string const& _contractName = "")
	{
		m_code = _code;
		BOOST_REQUIRE_NO_THROW(m_compilerStack.parse(_code));
		m_interface = m_compilerStack.getMetadata("", DocumentationType::ABISolidityInterface);
		BOOST_REQUIRE_NO_THROW(m_reCompiler.parse(m_interface));
		return m_reCompiler.getContractDefinition(_contractName);
	}

	string getSourcePart(ASTNode const& _node) const
	{
		Location location = _node.getLocation();
		BOOST_REQUIRE(!location.isEmpty());
		return m_interface.substr(location.start, location.end - location.start);
	}

protected:
	string m_code;
	string m_interface;
	CompilerStack m_compilerStack;
	CompilerStack m_reCompiler;
};

BOOST_FIXTURE_TEST_SUITE(SolidityInterface, SolidityInterfaceChecker)

BOOST_AUTO_TEST_CASE(empty_contract)
{
	ContractDefinition const& contract = checkInterface("contract test {}");
	BOOST_CHECK_EQUAL(getSourcePart(contract), "contract test{}");
}

BOOST_AUTO_TEST_CASE(single_function)
{
	ContractDefinition const& contract = checkInterface(
		"contract test {\n"
		"  function f(uint a) returns(uint d) { return a * 7; }\n"
		"}\n");
	BOOST_REQUIRE_EQUAL(1, contract.getDefinedFunctions().size());
	BOOST_CHECK_EQUAL(getSourcePart(*contract.getDefinedFunctions().front()),
					  "function f(uint256 a)returns(uint256 d){}");
}

BOOST_AUTO_TEST_CASE(single_constant_function)
{
	ContractDefinition const& contract = checkInterface(
			"contract test { function f(uint a) constant returns(hash8 x) { 1==2; } }");
	BOOST_REQUIRE_EQUAL(1, contract.getDefinedFunctions().size());
	BOOST_CHECK_EQUAL(getSourcePart(*contract.getDefinedFunctions().front()),
					  "function f(uint256 a)constant returns(hash8 x){}");
}

BOOST_AUTO_TEST_CASE(multiple_functions)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"  function g(uint b) returns(uint e) { return b * 8; }\n"
	"}\n";
	ContractDefinition const& contract = checkInterface(sourceCode);
	set<string> expectation({"function f(uint256 a)returns(uint256 d){}",
							 "function g(uint256 b)returns(uint256 e){}"});
	BOOST_REQUIRE_EQUAL(2, contract.getDefinedFunctions().size());
	BOOST_CHECK(expectation == set<string>({getSourcePart(*contract.getDefinedFunctions().at(0)),
											getSourcePart(*contract.getDefinedFunctions().at(1))}));
}

BOOST_AUTO_TEST_CASE(exclude_fallback_function)
{
	char const* sourceCode = "contract test { function() {} }";
	ContractDefinition const& contract = checkInterface(sourceCode);
	BOOST_CHECK_EQUAL(getSourcePart(contract), "contract test{}");
}

BOOST_AUTO_TEST_CASE(event)
{
	ContractDefinition const& contract = checkInterface(
			"contract test { event Event; }");
	BOOST_REQUIRE_EQUAL(1, contract.getEvents().size());
	BOOST_CHECK_EQUAL(getSourcePart(*contract.getEvents().front()),
					  "event Event;");
}

BOOST_AUTO_TEST_CASE(event_arguments)
{
	ContractDefinition const& contract = checkInterface(
			"contract test { event Event(uint a, uint indexed b); }");
	BOOST_REQUIRE_EQUAL(1, contract.getEvents().size());
	BOOST_CHECK_EQUAL(getSourcePart(*contract.getEvents().front()),
					  "event Event(uint256 a,uint256 indexed b);");
}

BOOST_AUTO_TEST_CASE(events)
{
	char const* sourceCode = "contract test {\n"
	"  function f(uint a) returns(uint d) { return a * 7; }\n"
	"  event e1(uint b, address indexed c); \n"
	"  event e2(); \n"
	"}\n";
	ContractDefinition const& contract = checkInterface(sourceCode);
	set<string> expectation({"event e1(uint256 b,address indexed c);", "event e2;"});
	BOOST_REQUIRE_EQUAL(2, contract.getEvents().size());
	BOOST_CHECK(expectation == set<string>({getSourcePart(*contract.getEvents().at(0)),
											getSourcePart(*contract.getEvents().at(1))}));
}

BOOST_AUTO_TEST_CASE(inheritance)
{
	char const* sourceCode =
	"	contract Base { \n"
	"		function baseFunction(uint p) returns (uint i) { return p; } \n"
	"		event baseEvent(string32 indexed evtArgBase); \n"
	"	} \n"
	"	contract Derived is Base { \n"
	"		function derivedFunction(string32 p) returns (string32 i) { return p; } \n"
	"		event derivedEvent(uint indexed evtArgDerived); \n"
	"	}";
	ContractDefinition const& contract = checkInterface(sourceCode);
	set<string> expectedEvents({"event derivedEvent(uint256 indexed evtArgDerived);",
								"event baseEvent(string32 indexed evtArgBase);"});
	set<string> expectedFunctions({"function baseFunction(uint256 p)returns(uint256 i){}",
								   "function derivedFunction(string32 p)returns(string32 i){}"});
	BOOST_CHECK(expectedEvents == set<string>({getSourcePart(*contract.getEvents().at(0)),
											   getSourcePart(*contract.getEvents().at(1))}));
	BOOST_REQUIRE_EQUAL(2, contract.getDefinedFunctions().size());
	BOOST_CHECK(expectedFunctions == set<string>({getSourcePart(*contract.getDefinedFunctions().at(0)),
												  getSourcePart(*contract.getDefinedFunctions().at(1))}));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
