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
 * @author Lefteris Karapetsas <lefteris@ethdev.com>
 * @date 2014
 * Unit tests for the solidity compiler JSON Interface output.
 */

#include <test/Common.h>
#include <string>
#include <libsolutil/JSON.h>
#include <libsolidity/interface/CompilerStack.h>
#include <liblangutil/Exceptions.h>
#include <libsolutil/Exceptions.h>
#include <libsolidity/interface/Natspec.h>

#include <boost/test/unit_test.hpp>

using namespace solidity::langutil;

namespace solidity::frontend::test
{

class DocumentationChecker
{
public:
	void checkNatspec(
		std::string const& _code,
		std::string const& _contractName,
		std::string const& _expectedDocumentationString,
		bool _userDocumentation
	)
	{
		m_compilerStack.reset();
		m_compilerStack.setSources({{"", "pragma solidity >=0.0;\n" + _code}});
		m_compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
		BOOST_REQUIRE_MESSAGE(m_compilerStack.parseAndAnalyze(), "Parsing contract failed");

		Json::Value generatedDocumentation;
		if (_userDocumentation)
			generatedDocumentation = m_compilerStack.natspecUser(_contractName);
		else
			generatedDocumentation = m_compilerStack.natspecDev(_contractName);
		Json::Value expectedDocumentation;
		std::string parseError;
		BOOST_REQUIRE_MESSAGE(util::jsonParseStrict(_expectedDocumentationString, expectedDocumentation, &parseError), parseError);

		expectedDocumentation["version"] = Json::Value(Natspec::c_natspecVersion);
		expectedDocumentation["kind"] = Json::Value(_userDocumentation ? "user" : "dev");

		BOOST_CHECK_MESSAGE(
			expectedDocumentation == generatedDocumentation,
			"Expected:\n" << util::jsonPrettyPrint(expectedDocumentation) <<
			"\n but got:\n" << util::jsonPrettyPrint(generatedDocumentation)
		);
	}

	void expectNatspecError(std::string const& _code)
	{
		m_compilerStack.reset();
		m_compilerStack.setSources({{"", "pragma solidity >=0.0;\n" + _code}});
		m_compilerStack.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
		BOOST_CHECK(!m_compilerStack.parseAndAnalyze());
		BOOST_REQUIRE(Error::containsErrorOfType(m_compilerStack.errors(), Error::Type::DocstringParsingError));
	}

protected:
	CompilerStack m_compilerStack;
};

BOOST_FIXTURE_TEST_SUITE(SolidityNatspecJSON, DocumentationChecker)

BOOST_AUTO_TEST_CASE(user_empty_natspec_test)
{
	char const* sourceCode = R"(
		contract test {
			///
			///
			function f() public {
			}
		}
	)";

	char const* natspec = R"(
	{
		"methods": {}
	}
	)";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_newline_break)
{
	char const* sourceCode = R"(
		contract test {
			///
			/// @notice hello

			/// @notice world
			function f() public {
			}
		}
	)";

	char const* natspec = R"ABCDEF(
	{
		"methods":
		{
			"f()":
			{
			"notice": "world"
			}
		}
	}
	)ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiline_empty_lines)
{
	char const* sourceCode = R"(
	contract test {
		/**
		 *
		 *
		 * @notice hello world
		 */
		function f() public {
		}
	}
	)";

	char const* natspec = R"ABCDEF(
	{
		"methods":
		{
			"f()":
			{
				"notice": "hello world"
			}
		}
	}
	)ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}


BOOST_AUTO_TEST_CASE(user_basic_test)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			function mul(uint a) public returns(uint d) { return a * 7; }
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256)\":{ \"notice\": \"Multiplies `a` by 7\"}"
	"}}";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_and_user_basic_test)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			/// @dev Multiplies a number by 7
			function mul(uint a) public returns (uint d) { return a * 7; }
		}
	)";

	char const* devNatspec = R"R(
	{
		"methods":
		{
			"mul(uint256)":
			{
				"details": "Multiplies a number by 7"
			}
		}
	})R";

	char const* userNatspec = R"R(
	{
		"methods":
		{
			"mul(uint256)":
			{
				"notice": "Multiplies `a` by 7"
			}
		}
	})R";

	checkNatspec(sourceCode, "test", devNatspec, false);
	checkNatspec(sourceCode, "test", userNatspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiline_comment)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7
			/// and then adds `b`
			function mul_and_add(uint a, uint256 b) public returns (uint256 d) {
				return (a * 7) + b;
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add(uint256,uint256)\":{ \"notice\": \"Multiplies `a` by 7 and then adds `b`\"}"
	"}}";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_multiple_functions)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice Multiplies `a` by 7 and then adds `b`
			function mul_and_add(uint a, uint256 b) public returns (uint256 d) {
				return (a * 7) + b;
			}

			/// @notice Divides `input` by `div`
			function divide(uint input, uint div) public returns (uint d) {
				return input / div;
			}

			/// @notice Subtracts 3 from `input`
			function sub(int input) public returns (int d) {
				return input - 3;
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul_and_add(uint256,uint256)\":{ \"notice\": \"Multiplies `a` by 7 and then adds `b`\"},"
	"    \"divide(uint256,uint256)\":{ \"notice\": \"Divides `input` by `div`\"},"
	"    \"sub(int256)\":{ \"notice\": \"Subtracts 3 from `input`\"}"
	"}}";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_empty_contract)
{
	char const* sourceCode = R"(
		contract test { }
	)";

	char const* natspec = "{\"methods\":{} }";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_and_user_no_doc)
{
	char const* sourceCode = R"(
		contract test {
			function mul(uint a) public returns (uint d) {
				return a * 7;
			}
			function sub(int input) public returns (int d) {
				return input - 3;
			}
		}
	)";

	char const* devNatspec = "{\"methods\":{}}";
	char const* userNatspec = "{\"methods\":{}}";

	checkNatspec(sourceCode, "test", devNatspec, false);
	checkNatspec(sourceCode, "test", userNatspec, true);
}

BOOST_AUTO_TEST_CASE(public_state_variable)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice example of notice
			/// @dev example of dev
			/// @return returns state
			uint public state;
		}
	)";

	char const* devDoc = R"R(
	{
		"methods": {},
		"stateVariables":
		{
			"state":
			{
				"details": "example of dev",
				"return": "returns state",
				"returns":
				{
					"_0": "returns state"
				}
			}
		}
	}
	)R";
	checkNatspec(sourceCode, "test", devDoc, false);

	char const* userDoc = R"R(
	{
		"methods":
		{
			"state()":
			{
				"notice": "example of notice"
			}
		}
	}
	)R";
	checkNatspec(sourceCode, "test", userDoc, true);
}

BOOST_AUTO_TEST_CASE(public_state_variable_struct)
{
	char const* sourceCode = R"(
		contract Bank {
			struct Coin {
				string observeGraphicURL;
				string reverseGraphicURL;
			}

			/// @notice Get the n-th coin I own
			/// @return observeGraphicURL Front pic
			/// @return reverseGraphicURL Back pic
			Coin[] public coinStack;
		}
	)";

	char const* devDoc = R"R(
	{
		"methods": {},
		"stateVariables":
		{
			"coinStack":
			{
				"returns":
				{
					"observeGraphicURL": "Front pic",
					"reverseGraphicURL": "Back pic"
				}
			}
		}
	}
	)R";
	checkNatspec(sourceCode, "Bank", devDoc, false);

	char const* userDoc = R"R(
	{
		"methods":
		{
			"coinStack(uint256)":
			{
				"notice": "Get the n-th coin I own"
			}
		}
	}
	)R";
	checkNatspec(sourceCode, "Bank", userDoc, true);
}

BOOST_AUTO_TEST_CASE(public_state_variable_struct_repeated)
{
	char const* sourceCode = R"(
		contract Bank {
			struct Coin {
				string obverseGraphicURL;
				string reverseGraphicURL;
			}

			/// @notice Get the n-th coin I own
			/// @return obverseGraphicURL Front pic
			/// @return obverseGraphicURL Front pic
			Coin[] public coinStack;
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(private_state_variable)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev example of dev
			uint private state;
		}
	)";

	char const* devDoc = R"(
	{
		"methods": {},
		"stateVariables":
		{
			"state":
			{
				"details": "example of dev"
			}
		}
	}
	)";
	checkNatspec(sourceCode, "test", devDoc, false);

	char const* userDoc = R"(
	{
		"methods":{}
	}
	)";
	checkNatspec(sourceCode, "test", userDoc, true);
}

BOOST_AUTO_TEST_CASE(event)
{
	char const* sourceCode = R"(
		contract ERC20 {
			/// @notice This event is emitted when a transfer occurs.
			/// @param from The source account.
			/// @param to The destination account.
			/// @param amount The amount.
			/// @dev A test case!
			event Transfer(address indexed from, address indexed to, uint amount);
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"events":
		{
			"Transfer(address,address,uint256)":
			{
				"details": "A test case!",
				"params":
				{
					"amount": "The amount.", "from": "The source account.", "to": "The destination account."
				}
			}
		},
		"methods": {}
	}
	)ABCDEF";
	checkNatspec(sourceCode, "ERC20", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"events":
		{
			"Transfer(address,address,uint256)":
			{
				"notice": "This event is emitted when a transfer occurs."
			}
		},
		"methods": {}
	}
	)ABCDEF";
	checkNatspec(sourceCode, "ERC20", userDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_event_from_foreign_contract)
{
	char const* sourceCode = R"(
		contract X {
			/// @notice Userdoc for event E.
			/// @dev Devdoc for event E.
			event E();
		}

		contract C {
			function g() public {
				emit X.E();
			}
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"events":
		{
			"E()":
			{
				"details": "Devdoc for event E."
			}
		},
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"events":
		{
			"E()":
			{
				"notice": "Userdoc for event E."
			}
		},
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_event_from_foreign_contract_with_same_signature)
{
	char const* sourceCode = R"(
		contract C {
			/// @notice C.E event
			/// @dev C.E event
			event E(uint256 value);
		}

		contract D {
			/// @notice D.E event
			/// @dev D.E event
			event E(uint256 value);

			function test() public {
				emit C.E(1);
				emit E(2);
			}
		}
	)";

	char const* devDocC = R"ABCDEF(
	{
		"events":
		{
			"E(uint256)":
			{
				"details": "C.E event"
			}
		},
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDocC, false);

	char const* devDocD = R"ABCDEF(
	{
		"events":
		{
			"E(uint256)":
			{
				"details": "D.E event"
			}
		},
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "D", devDocD, false);

	char const* userDocC = R"ABCDEF(
	{
		"events":
		{
			"E(uint256)":
			{
				"notice": "C.E event"
			}
		},
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDocC, true);

	char const* userDocD = R"ABCDEF(
	{
		"events":
		{
			"E(uint256)":
			{
				"notice": "D.E event"
			}
		},
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "D", userDocD, true);
}

// Tests that emitting an event from contract C in contract D does not inherit natspec from C.E
BOOST_AUTO_TEST_CASE(emit_event_from_foreign_contract_no_inheritance)
{
	char const* sourceCode = R"(
		contract C {
			/// @notice C.E event
			/// @dev C.E event
			event E();
		}

		contract D {
			event E();

			function test() public {
				emit C.E();
			}
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "D", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "D", userDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_same_signature_event_library_contract)
{
	char const* sourceCode = R"(
		library L {
		    /// @notice This event is defined in Library L
		    /// @dev This should not appear in Contract C dev doc
		    event SameSignatureEvent(uint16);
		    /// @notice This event is defined in Library L
		    /// @dev This should appear in Contract C dev doc
		    event LibraryEvent(uint32);
		}
		contract C {
		    /// @notice This event is defined in Contract C
			/// @dev This should appear in Contract C dev doc
		    event SameSignatureEvent(uint16);
		    /// @notice This event is defined in Contract C
			/// @dev This should appear in contract C dev doc
		    event ContractEvent(uint32);
		    function f() public {
		        emit L.SameSignatureEvent(0);
		        emit SameSignatureEvent(1);
		        emit L.LibraryEvent(2);
		        emit ContractEvent(3);
		    }
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
	    "events":
	    {
	        "ContractEvent(uint32)":
	        {
	            "details": "This should appear in contract C dev doc"
	        },
			"LibraryEvent(uint32)":
			{
				"details": "This should appear in Contract C dev doc"
			},
	        "SameSignatureEvent(uint16)":
	        {
	            "details": "This should appear in Contract C dev doc"
	        }
	    },
	    "kind": "dev",
	    "methods": {},
	    "version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"events":
		{
			"ContractEvent(uint32)":
			{
				"notice": "This event is defined in Contract C"
			},
			"LibraryEvent(uint32)":
			{
				"notice": "This event is defined in Library L"
			},
			"SameSignatureEvent(uint16)":
			{
				"notice": "This event is defined in Contract C"
			}
		},
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_same_signature_event_different_libraries)
{
	char const* sourceCode = R"(
		library L1 {
		    /// @notice This event is defined in Library L1
		    /// @dev This should not appear in Contract C dev doc
		    event SameSignatureEvent(uint16);
		}
		library L2 {
		    /// @notice This event is defined in Library L2
		    /// @dev This should not appear in Contract C dev doc
		    event SameSignatureEvent(uint16);
		}
		library L3 {
		    /// @notice This event is defined in Library L3
		    /// @dev This should not appear in Contract C dev doc
		    event SameSignatureEvent(uint16);
		}
		contract C {
		    function f() public {
		        emit L1.SameSignatureEvent(0);
		        emit L2.SameSignatureEvent(1);
		        emit L3.SameSignatureEvent(2);
		    }
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDoc, true);

	char const* libraryDevDoc = R"ABCDEF(
	{
		"events":
		{
			"SameSignatureEvent(uint16)":
			{
				"details": "This should not appear in Contract C dev doc"
			}
		},
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "L1", libraryDevDoc, false);

	char const* libraryUserDoc = R"ABCDEF(
	{
		"events":
		{
			"SameSignatureEvent(uint16)":
			{
				"notice": "This event is defined in Library L1"
			}
		},
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "L1", libraryUserDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_same_signature_event_library_inherited)
{
	char const* sourceCode = R"(
		contract D {
			/// @notice This event is defined in contract D
			/// @dev This should appear in Contract C dev doc
			event SameSignatureEvent(uint16);
		}
		library L {
		    /// @notice This event is defined in Library L
		    /// @dev This should not appear in Contract C
		    event SameSignatureEvent(uint16);
		}
		contract C is D {
		    function f() public {
		        emit L.SameSignatureEvent(0);
		        emit D.SameSignatureEvent(1);
		    }
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"events":
		{
			"SameSignatureEvent(uint16)":
			{
				"details": "This should appear in Contract C dev doc"
			}
		},
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"events":
		{
			"SameSignatureEvent(uint16)":
			{
				"notice": "This event is defined in contract D"
			}
		},
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_same_signature_event_library_contract_missing_natspec)
{
	char const* sourceCode = R"(
		library L {
		    /// @notice This event is defined in library L
		    /// @dev This should not appear in contract C devdoc
		    event SameSignatureEvent(uint16);
		    /// @notice This event is defined in library L
			/// @dev This should appear in contract C devdoc
		    event LibraryEvent(uint32);
		}
		contract C {
			event SameSignatureEvent(uint16);
		    /// @notice This event is defined in contract C
		    event ContractEvent(uint32);
		    function f() public {
		        emit L.SameSignatureEvent(0);
		        emit SameSignatureEvent(1);
		        emit L.LibraryEvent(2);
		        emit ContractEvent(3);
		    }
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
	    "events":
	    {
	        "LibraryEvent(uint32)":
	        {
	            "details": "This should appear in contract C devdoc"
	        }
	    },
	    "kind": "dev",
	    "methods": {},
	    "version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"events":
		{
			"ContractEvent(uint32)":
			{
				"notice": "This event is defined in contract C"
			},
			"LibraryEvent(uint32)":
			{
				"notice": "This event is defined in library L"
			}
		},
		"kind": "user",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_same_signature_event_different_libraries_missing_natspec)
{
	char const* sourceCode = R"(
		library L1 {
		    event SameSignatureEvent(uint16);
		}
		library L2 {
		    /// @notice This event is defined in library L2
		    /// @dev This should not appear in Contract C devdoc
		    event SameSignatureEvent(uint16);
		}
		library L3 {
		    event SameSignatureEvent(uint16);
		}
		contract C {
		    function f() public {
		        emit L1.SameSignatureEvent(0);
		        emit L2.SameSignatureEvent(1);
		        emit L3.SameSignatureEvent(2);
		    }
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
	    "kind": "user",
	    "methods": {},
	    "version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(emit_same_signature_event_library_inherited_missing_natspec)
{
	char const* sourceCode = R"(
		contract D {
			event SameSignatureEvent(uint16);
		}
		library L {
		    /// @notice This event is defined in library L
		    /// @dev This should not appear in contract C devdoc
		    event SameSignatureEvent(uint16);
		}
		contract C is D {
		    function f() public {
		        emit L.SameSignatureEvent(0);
		        emit D.SameSignatureEvent(1);
		    }
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"kind": "dev",
		"methods": {},
		"version": 1
	}
	)ABCDEF";
	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
   {
     "kind": "user",
     "methods": {},
     "version": 1
   }
	)ABCDEF";
	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(event_inheritance_interface)
{
	char const* sourceCode = R"(
		interface ERC20 {
			/// @notice This event is emitted when a transfer occurs.
			/// @param from The source account.
			/// @param to The destination account.
			/// @param amount The amount.
			/// @dev A test case!
			event Transfer(address indexed from, address indexed to, uint amount);
		}
		contract A is ERC20 {
		}
		contract B is A {
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"events":
		{
			"Transfer(address,address,uint256)":
			{
				"details": "A test case!",
				"params":
				{
					"amount": "The amount.",
					"from": "The source account.",
					"to": "The destination account."
				}
			}
		},
		"methods": {}
	}
	)ABCDEF";
	checkNatspec(sourceCode, "A", devDoc, false);
	checkNatspec(sourceCode, "B", devDoc, false);
	checkNatspec(sourceCode, "ERC20", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"events":
		{
			"Transfer(address,address,uint256)":
			{
				"notice": "This event is emitted when a transfer occurs."
			}
		},
		"methods": {}
	}
	)ABCDEF";
	checkNatspec(sourceCode, "A", userDoc, true);
	checkNatspec(sourceCode, "B", userDoc, true);
	checkNatspec(sourceCode, "ERC20", userDoc, true);
}

BOOST_AUTO_TEST_CASE(event_inheritance)
{
	char const* sourceCode = R"(
		contract ERC20 {
			/// @notice This event is emitted when a transfer occurs.
			/// @param from The source account.
			/// @param to The destination account.
			/// @param amount The amount.
			/// @dev A test case!
			event Transfer(address indexed from, address indexed to, uint amount);
		}
		contract A is ERC20 {
		}
		contract B is A {
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"events":
		{
			"Transfer(address,address,uint256)":
			{
				"details": "A test case!",
				"params":
				{
					"amount": "The amount.",
					"from": "The source account.",
					"to": "The destination account."
				}
			}
		},
		"methods": {}
	}
	)ABCDEF";
	checkNatspec(sourceCode, "A", devDoc, false);
	checkNatspec(sourceCode, "B", devDoc, false);
	checkNatspec(sourceCode, "ERC20", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"events":
		{
			"Transfer(address,address,uint256)":
			{
				"notice": "This event is emitted when a transfer occurs."
			}
		},
		"methods": {}
	}
	)ABCDEF";
	checkNatspec(sourceCode, "A", userDoc, true);
	checkNatspec(sourceCode, "B", userDoc, true);
	checkNatspec(sourceCode, "ERC20", userDoc, true);
}

BOOST_AUTO_TEST_CASE(dev_desc_after_nl)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev
			/// Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiple_params)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_mutiline_param_description)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiple_functions)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
			/// @dev Divides 2 numbers
			/// @param input Documentation for the input parameter
			/// @param div Documentation for the div parameter
			function divide(uint input, uint div) public returns (uint d) {
				return input / div;
			}
			/// @dev Subtracts 3 from `input`
			/// @param input Documentation for the input parameter
			function sub(int input) public returns (int d) {
				return input - 3;
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        }\n"
	"    },\n"
	"    \"divide(uint256,uint256)\":{ \n"
	"        \"details\": \"Divides 2 numbers\",\n"
	"        \"params\": {\n"
	"            \"input\": \"Documentation for the input parameter\",\n"
	"            \"div\": \"Documentation for the div parameter\"\n"
	"        }\n"
	"    },\n"
	"    \"sub(int256)\":{ \n"
	"        \"details\": \"Subtracts 3 from `input`\",\n"
	"        \"params\": {\n"
	"            \"input\": \"Documentation for the input parameter\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return_no_params)
{
	char const* sourceCode = R"(
		contract test {
			/// @return d The result of the multiplication
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = R"ABCDEF(
	{
		"methods":
		{
			"mul(uint256,uint256)":
			{
				"returns": { "d": "The result of the multiplication" }
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return d The result of the multiplication
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"returns\": {\n"
	"            \"d\": \"The result of the multiplication\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return_desc_after_nl)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return
			/// d The result of the multiplication
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"returns\": {\n"
	"            \"d\": \"The result of the multiplication\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return_desc_multiple_unamed_mixed)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return The result of the multiplication
			/// @return _cookies And cookies with nutella
			function mul(uint a, uint second) public returns (uint, uint _cookies) {
				uint mul = a * 7;
				return (mul, second);
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"returns\": {\n"
	"            \"_0\": \"The result of the multiplication\",\n"
	"            \"_cookies\": \"And cookies with nutella\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return_desc_multiple_unamed_mixed_2)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return _cookies And cookies with nutella
			/// @return The result of the multiplication
			/// @return _milk And milk with nutella
			function mul(uint a, uint second) public returns (uint _cookies, uint, uint _milk) {
				uint mul = a * 7;
				uint milk = 4;
				return (mul, second, milk);
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"returns\": {\n"
	"            \"_cookies\": \"And cookies with nutella\",\n"
	"            \"_1\": \"The result of the multiplication\",\n"
	"            \"_milk\": \"And milk with nutella\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return_desc_multiple_unamed)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return The result of the multiplication
			/// @return And cookies with nutella
			function mul(uint a, uint second) public returns (uint, uint) {
				uint mul = a * 7;
				return (mul, second);
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"returns\": {\n"
	"            \"_0\": \"The result of the multiplication\",\n"
	"            \"_1\": \"And cookies with nutella\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_return_desc_multiple)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return d The result of the multiplication
			/// @return f And cookies with nutella
			function mul(uint a, uint second) public returns (uint d, uint f) {
				uint mul = a * 7;
				return (mul, second);
			}
		}
	)";

	char const* natspec = "{"
	"\"methods\":{"
	"    \"mul(uint256,uint256)\":{ \n"
	"        \"details\": \"Multiplies a number by 7 and adds second parameter\",\n"
	"        \"params\": {\n"
	"            \"a\": \"Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines\",\n"
	"            \"second\": \"Documentation for the second parameter\"\n"
	"        },\n"
	"        \"returns\": {\n"
	"            \"d\": \"The result of the multiplication\",\n"
	"            \"f\": \"And cookies with nutella\"\n"
	"        }\n"
	"    }\n"
	"}}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiline_return)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return d The result of the multiplication
			/// and cookies with nutella
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
		}
	)";

	char const* natspec = R"R({
		"methods":
		{
			"mul(uint256,uint256)":
			{
				"details": "Multiplies a number by 7 and adds second parameter",
				"params":
				{
					"a": "Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines",
					"second": "Documentation for the second parameter"
				},
				"returns":
				{
					"d": "The result of the multiplication and cookies with nutella"
				}
			}
		}
	})R";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_multiline_comment)
{
	char const* sourceCode = R"(
		contract test {
			/**
			 * @dev Multiplies a number by 7 and adds second parameter
			 * @param a Documentation for the first parameter starts here.
			 * Since it's a really complicated parameter we need 2 lines
			 * @param second Documentation for the second parameter
			 * @return d The result of the multiplication
			 * and cookies with nutella
			 */
			function mul(uint a, uint second) public returns (uint d) {
				return a * 7 + second;
			}
		}
	)";

	char const* natspec = R"R(
	{
		"methods":
		{
			"mul(uint256,uint256)":
			{
				"details": "Multiplies a number by 7 and adds second parameter",
				"params":
				{
					"a": "Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines",
					"second": "Documentation for the second parameter"
				},
				"returns":
				{
					"d": "The result of the multiplication and cookies with nutella"
				}
			}
		}
	})R";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_documenting_no_return_param_name)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second Documentation for the second parameter
			/// @return
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_contract_no_doc)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Mul function
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\"\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_contract_doc)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = "{"
	"    \"author\": \"Lefteris\","
	"    \"title\": \"Just a test contract\","
	"    \"methods\":{"
	"        \"mul(uint256,uint256)\":{ \n"
	"            \"details\": \"Mul function\"\n"
	"        }\n"
	"    }\n"
	"}";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_author_at_function)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			/// @author John Doe
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(struct_no_docs)
{
	char const* sourceCode = R"(
		contract C {
			/// @title example of title
			/// @author example of author
			/// @notice example of notice
			/// @dev example of dev
			struct Example {
				string text;
				bool valid;
				uint256 value;
			}
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"kind": "dev",
		"methods": {},
		"version": 1
	})ABCDEF";

	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
		"kind": "user",
		"methods": {},
		"version": 1
	})ABCDEF";

	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(enum_no_docs)
{
	char const* sourceCode = R"(
		contract C {
			/// @title example of title
			/// @author example of author
			/// @notice example of notice
			/// @dev example of dev
			enum Color {
				Red,
				Green
			}
		}
	)";

	char const* devDoc = R"ABCDEF(
	{
		"kind": "dev",
		"methods": {},
		"version": 1
	})ABCDEF";

	checkNatspec(sourceCode, "C", devDoc, false);

	char const* userDoc = R"ABCDEF(
	{
	  "kind": "user",
	  "methods": {},
	  "version": 1
	})ABCDEF";

	checkNatspec(sourceCode, "C", userDoc, true);
}

BOOST_AUTO_TEST_CASE(notice_without_tag)
{
	char const* sourceCode = R"(
		contract test {
			/// I do something awesome
			function mul(uint a) public returns (uint d) { return a * 7; }
		}
	)";


	char const* natspec = R"ABCDEF(
	{
		"methods":
		{
			"mul(uint256)":
			{
				"notice": "I do something awesome"
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(multiline_notice_without_tag)
{
	char const* sourceCode = R"(
		contract test {
			/// I do something awesome
			/// which requires two lines to explain
			function mul(uint a) public returns (uint d) { return a * 7; }
		}
	)";

	char const* natspec = R"ABCDEF(
	{
		"methods":
		{
			"mul(uint256)":
			{
			"notice": "I do something awesome which requires two lines to explain"
		}
	}
	}
	)ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(empty_comment)
{
	char const* sourceCode = R"(
		//
		contract test
		{}
	)";
	char const* natspec = R"ABCDEF(
	{
		"methods": {}
	}
	)ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_title_at_function_error)
{
	char const* sourceCode = R"(
		/// @author Lefteris
		/// @title Just a test contract
		contract test {
			/// @dev Mul function
			/// @title I really should not be here
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_nonexistent_param)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param not_existing Documentation for the second parameter
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_no_param_name)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_no_param_name_end)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param se
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_documenting_no_param_description)
{
	char const* sourceCode = R"(
		contract test {
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter
			/// @param second
			function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(user_constructor)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice this is a really nice constructor
			constructor(uint a, uint second) { }
		}
	)";

	char const* natspec = R"ABCDEF({
	"methods":
	{
		"constructor":
		{
			"notice": "this is a really nice constructor"
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(user_constructor_and_function)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice this is a really nice constructor
			constructor(uint a, uint second) { }
			/// another multiplier
			function mul(uint a, uint second) public returns(uint d) { return a * 7 + second; }
		}
	)";

	char const* natspec = R"ABCDEF({
	"methods":
	{
		"mul(uint256,uint256)":
		{
			"notice": "another multiplier"
		},
		"constructor":
		{
			"notice": "this is a really nice constructor"
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_constructor)
{
	char const *sourceCode = R"(
		contract test {
			/// @param a the parameter a is really nice and very useful
			/// @param second the second parameter is not very useful, it just provides additional confusion
			constructor(uint a, uint second) { }
		}
	)";

	char const *natspec = R"ABCDEF({
	"methods":
	{
		"constructor":
		{
			"params":
			{
				"a": "the parameter a is really nice and very useful",
				"second": "the second parameter is not very useful, it just provides additional confusion"
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(dev_constructor_return)
{
	char const* sourceCode = R"(
		contract test {
			/// @param a the parameter a is really nice and very useful
			/// @param second the second parameter is not very useful, it just provides additional confusion
			/// @return return should not work within constructors
			constructor(uint a, uint second) { }
		}
	)";

	expectNatspecError(sourceCode);
}

BOOST_AUTO_TEST_CASE(dev_constructor_and_function)
{
	char const *sourceCode = R"(
		contract test {
			/// @param a the parameter a is really nice and very useful
			/// @param second the second parameter is not very useful, it just provides additional confusion
			constructor(uint a, uint second) { }
			/// @dev Multiplies a number by 7 and adds second parameter
			/// @param a Documentation for the first parameter starts here.
			/// Since it's a really complicated parameter we need 2 lines
			/// @param second Documentation for the second parameter
			/// @return d The result of the multiplication
			/// and cookies with nutella
			function mul(uint a, uint second) public returns(uint d) {
				return a * 7 + second;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
	"methods":
	{
		"mul(uint256,uint256)":
		{
			"details": "Multiplies a number by 7 and adds second parameter",
			"params":
			{
				"a": "Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines",
				"second": "Documentation for the second parameter"
			},
			"returns":
			{
				"d": "The result of the multiplication and cookies with nutella"
			}
		},
		"constructor":
		{
			"params":
			{
				"a": "the parameter a is really nice and very useful",
				"second": "the second parameter is not very useful, it just provides additional confusion"
			}
		}
	}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, false);
}

BOOST_AUTO_TEST_CASE(slash4)
{
	char const* sourceCode = R"(
		contract test {
			//// @notice lorem ipsum
			function f() public { }
		}
	)";

	char const* natspec = R"( { "methods": {} } )";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(star3)
{
	char const* sourceCode = R"(
		contract test {
			/***
			 * @notice lorem ipsum
			 */
			function f() public { }
		}
	)";

	char const* natspec = R"( { "methods": {} } )";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(slash3_slash3)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice lorem
			/// ipsum
			function f() public { }
		}
	)";

	char const* natspec = R"ABCDEF({
		"methods":
		{
			"f()": { "notice": "lorem ipsum" }
		}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(slash3_slash4)
{
	char const* sourceCode = R"(
		contract test {
			/// @notice lorem
			//// ipsum
			function f() public { }
		}
	)";

	char const* natspec = R"ABCDEF({
		"methods":
		{
			"f()": { "notice": "lorem" }
		}
	})ABCDEF";

	checkNatspec(sourceCode, "test", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_default_inherit_variable)
{
	char const *sourceCode = R"(
		contract C {
			/// @notice Hello world
			/// @dev test
			function x() virtual external returns (uint) {
				return 1;
			}
		}

		contract D is C {
			uint public override x;
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods": { "x()": { "details": "test" } }
	})ABCDEF";

	char const *natspec1 = R"ABCDEF({
		"methods": {},
		"stateVariables":
		{
			"x":
			{
				"details": "test"
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "C", natspec, false);
	checkNatspec(sourceCode, "D", natspec1, false);
}

BOOST_AUTO_TEST_CASE(user_default_inherit_variable)
{
	char const *sourceCode = R"(
		contract C {
			/// @notice Hello world
			/// @dev test
			function x() virtual external returns (uint) {
				return 1;
			}
		}

		contract D is C {
			uint public override x;
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods": { "x()": { "notice": "Hello world" } }
	})ABCDEF";

	checkNatspec(sourceCode, "C", natspec, true);
	checkNatspec(sourceCode, "D", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_explicit_inherit_variable)
{
	char const *sourceCode = R"(
		contract B {
			function x() virtual external returns (uint) {
				return 1;
			}
		}

		contract C {
			/// @notice Hello world
			/// @dev test
			function x() virtual external returns (uint) {
				return 1;
			}
		}

		contract D is C, B {
			/// @inheritdoc C
			uint public override(C, B) x;
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods": { "x()": { "details": "test" } }
	})ABCDEF";

	char const *natspec1 = R"ABCDEF({
		"methods": {},
		"stateVariables":
		{
			"x":
			{
				"details": "test"
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "C", natspec, false);
	checkNatspec(sourceCode, "D", natspec1, false);
}

BOOST_AUTO_TEST_CASE(user_explicit_inherit_variable)
{
	char const *sourceCode = R"(
		contract B {
			function x() virtual external returns (uint) {
				return 1;
			}
		}

		contract C {
			/// @notice Hello world
			/// @dev test
			function x() virtual external returns (uint) {
				return 1;
			}
		}

		contract D is C, B {
			/// @inheritdoc C
			uint public override(C, B) x;
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods": { "x()": { "notice": "Hello world" } }
	})ABCDEF";

	checkNatspec(sourceCode, "C", natspec, true);
	checkNatspec(sourceCode, "D", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_default_inherit)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// Second line.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract Middle is ERC20 {
			function transfer(address to, uint amount) virtual override external returns (bool)
			{
			return false;
			}
		}

		contract Token is Middle {
			function transfer(address to, uint amount) override external returns (bool)
			{
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "test",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, false);
	checkNatspec(sourceCode, "Middle", natspec, false);
	checkNatspec(sourceCode, "Token", natspec, false);
}

BOOST_AUTO_TEST_CASE(user_default_inherit)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// Second line.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract Middle is ERC20 {
			function transfer(address to, uint amount) virtual override external returns (bool)
			{
				return false;
			}
		}

		contract Token is Middle {
			function transfer(address to, uint amount) override external returns (bool)
			{
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "Transfer ``amount`` from ``msg.sender`` to ``to``. Second line."
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, true);
	checkNatspec(sourceCode, "Middle", natspec, true);
	checkNatspec(sourceCode, "Token", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_explicit_inherit)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 {
			function transfer(address to, uint amount) virtual external returns (bool) {
				return false;
			}
		}

		contract Token is ERC21, ERC20 {
			/// @inheritdoc ERC20
			function transfer(address to, uint amount) override(ERC21, ERC20) external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "test",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, false);
	checkNatspec(sourceCode, "Token", natspec, false);
}

BOOST_AUTO_TEST_CASE(user_explicit_inherit)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 {
			function transfer(address to, uint amount) virtual external returns (bool) {
				return false;
			}
		}

		contract Token is ERC21, ERC20 {
			/// @inheritdoc ERC20
			function transfer(address to, uint amount) override(ERC21, ERC20) external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, true);
	checkNatspec(sourceCode, "Token", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_explicit_inherit2)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 is ERC20 {
			function transfer(address to, uint amount) virtual override external returns (bool) {
				return false;
			}
		}

		contract Token is ERC20 {
			/// @inheritdoc ERC20
			function transfer(address to, uint amount) override external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "test",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, false);
	checkNatspec(sourceCode, "ERC21", natspec, false);
	checkNatspec(sourceCode, "Token", natspec, false);
}

BOOST_AUTO_TEST_CASE(user_explicit_inherit2)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 is ERC20 {
			function transfer(address to, uint amount) virtual override external returns (bool) {
				return false;
			}
		}

		contract Token is ERC20 {
			/// @inheritdoc ERC20
			function transfer(address to, uint amount) override external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, true);
	checkNatspec(sourceCode, "ERC21", natspec, true);
	checkNatspec(sourceCode, "Token", natspec, true);
}

BOOST_AUTO_TEST_CASE(dev_explicit_inherit_partial2)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 is ERC20 {
			/// @inheritdoc ERC20
			/// @dev override dev comment
			/// @notice override notice
			function transfer(address to, uint amount) virtual override external returns (bool) {
				return false;
			}
		}

		contract Token is ERC21 {
			function transfer(address to, uint amount) override external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "test",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "override dev comment",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, false);
	checkNatspec(sourceCode, "Token", natspec2, false);
}

BOOST_AUTO_TEST_CASE(user_explicit_inherit_partial2)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 is ERC20 {
			/// @inheritdoc ERC20
			/// @dev override dev comment
			/// @notice override notice
			function transfer(address to, uint amount) virtual override external returns (bool) {
				return false;
			}
		}

		contract Token is ERC21 {
			function transfer(address to, uint amount) override external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "override notice"
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, true);
	checkNatspec(sourceCode, "Token", natspec2, true);
}

BOOST_AUTO_TEST_CASE(dev_explicit_inherit_partial)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 {
			function transfer(address to, uint amount) virtual external returns (bool) {
				return false;
			}
		}

		contract Token is ERC21, ERC20 {
			/// @inheritdoc ERC20
			/// @dev override dev comment
			/// @notice override notice
			function transfer(address to, uint amount) override(ERC21, ERC20) external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "test",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "override dev comment",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, false);
	checkNatspec(sourceCode, "Token", natspec2, false);
}

BOOST_AUTO_TEST_CASE(user_explicit_inherit_partial)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract ERC21 {
			function transfer(address to, uint amount) virtual external returns (bool) {
				return false;
			}
		}

		contract Token is ERC21, ERC20 {
			/// @inheritdoc ERC20
			/// @dev override dev comment
			/// @notice override notice
			function transfer(address to, uint amount) override(ERC21, ERC20) external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "override notice"
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, true);
	checkNatspec(sourceCode, "Token", natspec2, true);
}

BOOST_AUTO_TEST_CASE(dev_inherit_parameter_mismatch)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract Middle is ERC20 {
			function transfer(address to, uint amount) override virtual external returns (bool) {
				return false;
			}
		}

		contract Token is Middle {
			function transfer(address too, uint amount) override external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"details": "test",
				"params":
				{
					"amount": "amount to transfer",
					"to": "address to transfer to"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods": { }
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, false);
	checkNatspec(sourceCode, "Middle", natspec, false);
	checkNatspec(sourceCode, "Token", natspec2, false);
}

BOOST_AUTO_TEST_CASE(user_inherit_parameter_mismatch)
{
	char const *sourceCode = R"(
		interface ERC20 {
			/// Transfer ``amount`` from ``msg.sender`` to ``to``.
			/// @dev test
			/// @param to address to transfer to
			/// @param amount amount to transfer
			function transfer(address to, uint amount) external returns (bool);
		}

		contract Middle is ERC20 {
			function transfer(address to, uint amount) override virtual external returns (bool) {
				return false;
			}
		}

		contract Token is Middle {
			function transfer(address too, uint amount) override external returns (bool) {
				return false;
			}
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"transfer(address,uint256)":
			{
				"notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods": { }
	})ABCDEF";

	checkNatspec(sourceCode, "ERC20", natspec, true);
	checkNatspec(sourceCode, "Middle", natspec, true);
	checkNatspec(sourceCode, "Token", natspec2, true);
}

BOOST_AUTO_TEST_CASE(dev_different_return_name)
{
	char const *sourceCode = R"(
		contract A {
			/// @return y value
			function g(int x) public pure virtual returns (int y) { return x; }
		}

		contract B is A {
			function g(int x) public pure override returns (int z) { return x; }
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"y": "value"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"z": "value"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "A", natspec, false);
	checkNatspec(sourceCode, "B", natspec2, false);
}

BOOST_AUTO_TEST_CASE(dev_different_return_name_multiple)
{
	char const *sourceCode = R"(
		contract A {
			/// @return a value A
			/// @return b value B
			function g(int x) public pure virtual returns (int a, int b) { return (1, 2); }
		}

		contract B is A {
			function g(int x) public pure override returns (int z, int y) { return (1, 2); }
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"a": "value A",
					"b": "value B"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"z": "value A",
					"y": "value B"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "A", natspec, false);
	checkNatspec(sourceCode, "B", natspec2, false);
}

BOOST_AUTO_TEST_CASE(dev_different_return_name_multiple_partly_unnamed)
{
	char const *sourceCode = R"(
		contract A {
			/// @return value A
			/// @return b value B
			function g(int x) public pure virtual returns (int, int b) { return (1, 2); }
		}

		contract B is A {
			function g(int x) public pure override returns (int z, int) { return (1, 2); }
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"_0": "value A",
					"b": "value B"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"z": "value A",
					"_1": "value B"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "A", natspec, false);
	checkNatspec(sourceCode, "B", natspec2, false);
}

BOOST_AUTO_TEST_CASE(dev_different_return_name_multiple_unnamed)
{
	char const *sourceCode = R"(
		contract A {
			/// @return value A
			/// @return value B
			function g(int x) public pure virtual returns (int, int) { return (1, 2); }
		}

		contract B is A {
			function g(int x) public pure override returns (int z, int y) { return (1, 2); }
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"_0": "value A",
					"_1": "value B"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"z": "value A",
					"y": "value B"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "A", natspec, false);
	checkNatspec(sourceCode, "B", natspec2, false);
}

BOOST_AUTO_TEST_CASE(dev_return_name_no_description)
{
	char const *sourceCode = R"(
		contract A {
			/// @return a
			function g(int x) public pure virtual returns (int a) { return 2; }
		}

		contract B is A {
			function g(int x) public pure override returns (int b) { return 2; }
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"a": "a"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods":
		{
			"g(int256)":
			{
				"returns":
				{
					"b": "a"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "A", natspec, false);
	checkNatspec(sourceCode, "B", natspec2, false);
}

BOOST_AUTO_TEST_CASE(error)
{
	char const* sourceCode = R"(
		contract test {
			/// Something failed.
			/// @dev an error.
			/// @param a first parameter
			/// @param b second parameter
			error E(uint a, uint b);
		}
	)";

	char const* devdoc = R"X({
		"errors":{
			"E(uint256,uint256)": [{
				"details": "an error.",
				"params":
				{
					"a": "first parameter",
					"b": "second parameter"
				}
			}]
		},
		"methods": {}
	})X";

	checkNatspec(sourceCode, "test", devdoc, false);

	char const* userdoc = R"X({
		"errors":{
			"E(uint256,uint256)": [{
				"notice": "Something failed."
			}]
		},
		"methods": {}
	})X";
	checkNatspec(sourceCode, "test", userdoc, true);
}

BOOST_AUTO_TEST_CASE(error_multiple)
{
	char const* sourceCode = R"(
		contract A {
			/// Something failed.
			/// @dev an error.
			/// @param x first parameter
			/// @param y second parameter
			error E(uint x, uint y);
		}
		contract test {
			/// X Something failed.
			/// @dev X an error.
			/// @param a X first parameter
			/// @param b X second parameter
			error E(uint a, uint b);
			function f(bool a) public pure {
				if (a)
					revert E(1, 2);
				else
					revert A.E(5, 6);
			}
		}
	)";

	char const* devdoc = R"X({
		"methods": {},
		"errors":
		{
			"E(uint256,uint256)": [
				{
					"details": "an error.",
					"params":
					{
						"x": "first parameter",
						"y": "second parameter"
					}
				},
				{
					"details": "X an error.",
					"params":
					{
						"a": "X first parameter",
						"b": "X second parameter"
					}
				}
			]
		}
	})X";

	checkNatspec(sourceCode, "test", devdoc, false);

	char const* userdoc = R"X({
		"errors":{
			"E(uint256,uint256)": [
				{ "notice": "Something failed." },
				{ "notice": "X Something failed." }
			]
		},
		"methods": {}
	})X";
	checkNatspec(sourceCode, "test", userdoc, true);
}

BOOST_AUTO_TEST_CASE(custom)
{
	char const* sourceCode = R"(
		/// @custom:x one two three
		/// @custom:y line
		/// break
		/// @custom:t one
		/// @custom:t two
		contract A {
			/// @custom:note statevar
			uint x;
			/// @custom:since 2014
			function g(int x) public pure virtual returns (int, int) { return (1, 2); }
		}
	)";

	char const* natspec = R"ABCDEF({
		"custom:t": "onetwo",
		"custom:x": "one two three",
		"custom:y": "line break",
		"methods":
		{
			"g(int256)":
			{
				"custom:since": "2014"
			}
		},
		"stateVariables": { "x": { "custom:note": "statevar" } }
	})ABCDEF";

	checkNatspec(sourceCode, "A", natspec, false);
}

BOOST_AUTO_TEST_CASE(custom_inheritance)
{
	char const *sourceCode = R"(
		contract A {
			/// @custom:since 2014
			function g(uint x) public pure virtual {}
		}
		contract B is A {
			function g(uint x) public pure override {}
		}
	)";

	char const* natspecA = R"ABCDEF(
	{
		"methods":
		{
			"g(uint256)":
			{
				"custom:since": "2014"
			}
		}
	})ABCDEF";
	char const* natspecB = R"ABCDEF(
	{
		"methods": {}
	})ABCDEF";

	checkNatspec(sourceCode, "A", natspecA, false);
	checkNatspec(sourceCode, "B", natspecB, false);
}

BOOST_AUTO_TEST_CASE(dev_struct_getter_override)
{
	char const *sourceCode = R"(
		interface IThing {
			/// @return x a number
			/// @return y another number
			function value() external view returns (uint128 x, uint128 y);
		}

		contract Thing is IThing {
			struct Value {
				uint128 x;
				uint128 y;
			}

			Value public override value;
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"value()":
			{
				"returns":
				{
					"x": "a number",
					"y": "another number"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods": {},
		"stateVariables":
		{
			"value":
			{
				"returns":
				{
					"x": "a number",
					"y": "another number"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "IThing", natspec, false);
	checkNatspec(sourceCode, "Thing", natspec2, false);
}

BOOST_AUTO_TEST_CASE(dev_struct_getter_override_no_return_name)
{
	char const *sourceCode = R"(
		interface IThing {
			///@return
			function value(uint) external returns (uint128,uint128);
		}

		contract Thing is IThing {
			struct Value {
				uint128 x;
				uint128 A;
			}
			mapping(uint=>Value) public override value;
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"value(uint256)":
			{
				"returns":
				{
					"_0": ""
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods": {},
		"stateVariables":
		{
			"value":
			{
				"return": "x ",
				"returns":
				{
					"x": ""
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "IThing", natspec, false);
	checkNatspec(sourceCode, "Thing", natspec2, false);
}

BOOST_AUTO_TEST_CASE(dev_struct_getter_override_different_return_parameter_names)
{
	char const *sourceCode = R"(
		interface IThing {
			/// @return x a number
			/// @return y another number
			function value() external view returns (uint128 x, uint128 y);
		}

		contract Thing is IThing {
			struct Value {
				uint128 a;
				uint128 b;
			}

			Value public override value;
		}
	)";

	char const *natspec = R"ABCDEF({
		"methods":
		{
			"value()":
			{
				"returns":
				{
					"x": "a number",
					"y": "another number"
				}
			}
		}
	})ABCDEF";

	char const *natspec2 = R"ABCDEF({
		"methods": {},
		"stateVariables":
		{
			"value":
			{
				"returns":
				{
					"a": "a number",
					"b": "another number"
				}
			}
		}
	})ABCDEF";

	checkNatspec(sourceCode, "IThing", natspec, false);
	checkNatspec(sourceCode, "Thing", natspec2, false);
}

}

BOOST_AUTO_TEST_SUITE_END()
