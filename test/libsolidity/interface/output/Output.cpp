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
// SPDX-License-Identifier: GPL-3.0

#include <libsolidity/interface/output/Output.h>
#include <libsolidity/interface/output/JSONDeserialization.h>

#include <liblangutil/Exceptions.h>

#include <boost/test/unit_test.hpp>

using namespace solidity::util;

namespace solidity::frontend::test
{

static std::string const FullOutputJSON = R"json(
{
	"errors":
	[{
		"type": "TypeError",
		"message": "Return argument count is wrong.",
		"errorCode": "1874",
		"sourceLocation": {"file": "file.sol", "start": 0, "end": 11, "message": "Here."},
		"secondarySourceLocations": [{"file": "file.sol", "start": 30, "end": 40, "message": "Related variable."}]
	}],
	"sources": {"file.sol": {"id": 0}},
	"contracts":
	{
		"file.sol":
		{
			"C":
			{
				"abi":
				[
					{"type": "constructor", "inputs": [], "stateMutability": "nonpayable"},
					{
						"type": "function",
						"name": "f",
						"inputs":
						[
							{"name": "a", "type": "uint256", "internalType": "uint256"},
							{
								"name": "s",
								"type": "tuple",
								"internalType": "struct C.S",
								"components": [{"name": "", "type": "uint256", "internalType": "uint256"}]
							}
						],
						"outputs": [],
						"stateMutability": "view"
					},
					{"type": "fallback", "stateMutability": "payable"},
					{"type": "receive", "stateMutability": "nonpayable"},
					{"type": "event", "name": "E", "anonymous": false, "inputs": [{"name": "x", "type": "uint256", "internalType": "uint256", "indexed": true}]},
					{"type": "error", "name": "Err", "inputs": []}
				],
				"evm":
				{
					"bytecode":
					{
						"object": "0x60806081",
						"linkReferences": {"file.sol": {"MyLib": [{"start": 10, "length": 20}]}}
					},
					"methodIdentifiers": {"transfer(address,uint256)": "0xa9059cbb"}
				},
				"metadata": "{\"cbor\":{\"ipfs\":\"QmTest\"}}"
			}
		}
	}
}
)json";

BOOST_AUTO_TEST_SUITE(OutputTests)

BOOST_AUTO_TEST_CASE(default_deserialization)
{
	Json const document = Json::parse("{}");
	output::Output const output = document.get<output::Output>();

	BOOST_CHECK(output.errors.empty());
	BOOST_CHECK(output.sources.empty());
	BOOST_CHECK(output.contracts.empty());
}

BOOST_AUTO_TEST_CASE(full_deserialization)
{
	Json const document = Json::parse(FullOutputJSON);
	output::Output const output = document.get<output::Output>();

	BOOST_CHECK_EQUAL(output.errors.size(), 1u);
	output::Error const& error = output.errors.at(0);
	BOOST_CHECK(error.errorCode == langutil::ErrorId{1874});
	BOOST_CHECK(error.type == langutil::Error::Type::TypeError);
	BOOST_CHECK(error.message.has_value());
	BOOST_CHECK_EQUAL(error.message.value_or(""), "Return argument count is wrong.");

	BOOST_CHECK(error.sourceLocation.has_value());
	BOOST_CHECK_EQUAL(error.sourceLocation->file, "file.sol");
	BOOST_CHECK_EQUAL(error.sourceLocation->start, 0);
	BOOST_CHECK_EQUAL(error.sourceLocation->end, 11);
	BOOST_CHECK_EQUAL(error.sourceLocation->message.value_or(""), "Here.");

	BOOST_CHECK(error.secondarySourceLocations.has_value());
	BOOST_CHECK_EQUAL(error.secondarySourceLocations->size(), 1u);
	BOOST_CHECK_EQUAL(error.secondarySourceLocations->at(0).message.value_or(""), "Related variable.");
	BOOST_CHECK_EQUAL(error.secondarySourceLocations->at(0).start, 30);

	BOOST_CHECK(output.sources.contains("file.sol"));
	BOOST_CHECK_EQUAL(output.sources.at("file.sol").id, 0u);

	BOOST_CHECK(output.contracts.contains("file.sol"));
	BOOST_CHECK(output.contracts.at("file.sol").contains("C"));
	BOOST_CHECK_EQUAL(output.contracts.at("file.sol").size(), 1u);
	output::Contract const& contract = output.contracts.at("file.sol").at("C");

	BOOST_CHECK_EQUAL(contract.name, "C");
	BOOST_CHECK_EQUAL(contract.abi.size(), 6u);
	BOOST_CHECK(std::holds_alternative<output::ABIConstructor>(contract.abi.at(0)));
	BOOST_CHECK(std::holds_alternative<output::ABIFunction>(contract.abi.at(1)));
	BOOST_CHECK(std::holds_alternative<output::ABIFallback>(contract.abi.at(2)));
	BOOST_CHECK(std::holds_alternative<output::ABIReceive>(contract.abi.at(3)));
	BOOST_CHECK(std::holds_alternative<output::ABIEvent>(contract.abi.at(4)));
	BOOST_CHECK(std::holds_alternative<output::ABIError>(contract.abi.at(5)));

	output::ABIConstructor const& constructor = std::get<output::ABIConstructor>(contract.abi.at(0));
	BOOST_CHECK(constructor.stateMutability == StateMutability::NonPayable);
	BOOST_CHECK(constructor.inputs.empty());

	output::ABIFunction const& function = std::get<output::ABIFunction>(contract.abi.at(1));
	BOOST_CHECK_EQUAL(function.name, "f");
	BOOST_CHECK(function.stateMutability == StateMutability::View);
	BOOST_CHECK_EQUAL(function.inputs.size(), 2u);
	BOOST_CHECK_EQUAL(function.inputs.at(0).name, "a");
	BOOST_CHECK_EQUAL(function.inputs.at(0).type, "uint256");
	BOOST_CHECK_EQUAL(function.inputs.at(0).internalType.value_or(""), "uint256");
	BOOST_CHECK(!function.inputs.at(0).indexed.has_value());
	BOOST_CHECK(!function.inputs.at(0).components.has_value());
	BOOST_CHECK_EQUAL(function.inputs.at(1).internalType.value_or(""), "struct C.S");
	BOOST_CHECK(function.inputs.at(1).components.has_value());
	BOOST_CHECK_EQUAL(function.inputs.at(1).components->size(), 1u);
	BOOST_CHECK_EQUAL(function.inputs.at(1).components->at(0).type, "uint256");

	output::ABIFallback const& fallback = std::get<output::ABIFallback>(contract.abi.at(2));
	BOOST_CHECK(fallback.stateMutability == StateMutability::Payable);

	output::ABIReceive const& receive = std::get<output::ABIReceive>(contract.abi.at(3));
	BOOST_CHECK(receive.stateMutability == StateMutability::NonPayable);

	output::ABIEvent const& event = std::get<output::ABIEvent>(contract.abi.at(4));
	BOOST_CHECK_EQUAL(event.name, "E");
	BOOST_CHECK(!event.isAnonymous);
	BOOST_CHECK_EQUAL(event.inputs.size(), 1u);
	BOOST_CHECK(event.inputs.at(0).indexed.has_value());
	BOOST_CHECK_EQUAL(event.inputs.at(0).indexed.value(), true);

	output::ABIError const& abiError = std::get<output::ABIError>(contract.abi.at(5));
	BOOST_CHECK_EQUAL(abiError.name, "Err");

	BOOST_CHECK(contract.evm.bytecode.object == util::fromHex("60806081"));
	BOOST_CHECK(contract.evm.bytecode.linkReferences.contains("file.sol"));
	BOOST_CHECK(contract.evm.bytecode.linkReferences.at("file.sol").contains("MyLib"));
	BOOST_CHECK_EQUAL(contract.evm.bytecode.linkReferences.at("file.sol").at("MyLib").size(), 1u);
	BOOST_CHECK_EQUAL(contract.evm.bytecode.linkReferences.at("file.sol").at("MyLib").at(0).start, 10u);
	BOOST_CHECK_EQUAL(contract.evm.bytecode.linkReferences.at("file.sol").at("MyLib").at(0).length, 20u);

	BOOST_CHECK_EQUAL(contract.evm.methodIdentifiers.size(), 1u);
	BOOST_CHECK_EQUAL(contract.evm.methodIdentifiers.at("transfer(address,uint256)"), "0xa9059cbb");

	BOOST_CHECK_EQUAL(contract.metadata, "{\"cbor\":{\"ipfs\":\"QmTest\"}}");
}

BOOST_AUTO_TEST_CASE(optional_fields_omitted)
{
	Json const document = Json::parse(R"({"errors": [{"type": "Warning", "message": "unused"}]})");
	output::Output const output = document.get<output::Output>();

	BOOST_CHECK_EQUAL(output.errors.size(), 1u);
	BOOST_CHECK(!output.errors.at(0).sourceLocation.has_value());
	BOOST_CHECK(!output.errors.at(0).secondarySourceLocations.has_value());
	BOOST_CHECK(!output.errors.at(0).errorCode.has_value());
	BOOST_CHECK(output.sources.empty());
	BOOST_CHECK(output.contracts.empty());

	output::Errors const errorsOnly = document.at("errors").get<output::Errors>();
	BOOST_CHECK_EQUAL(errorsOnly.size(), 1u);
}

BOOST_AUTO_TEST_CASE(invalid_documents_throw)
{
	Json const invalidMutability = Json::parse(R"(
		{"contracts":
			{"file.sol":
				{"C":
					{"abi": [{"type": "constructor", "inputs": [], "stateMutability": "invalid"}]}
				}
			}
		})"
	);
	BOOST_CHECK_THROW(invalidMutability.get<output::Output>(), langutil::InternalCompilerError);

	Json const missingMessage = Json::parse(R"({"errors": [{"type": "TypeError"}]})");
	BOOST_CHECK_THROW(missingMessage.get<output::Output>(), nlohmann::json::out_of_range);
}

BOOST_AUTO_TEST_CASE(parser_abstraction)
{
	output::NlohmannParser parser;
	output::Output const output = output::extract<output::Output>(parser, FullOutputJSON);

	BOOST_CHECK_EQUAL(output.contracts.at("file.sol").at("C").name, "C");
	BOOST_CHECK_EQUAL(output.contracts.at("file.sol").at("C").abi.size(), 6u);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace solidity::frontend::test
