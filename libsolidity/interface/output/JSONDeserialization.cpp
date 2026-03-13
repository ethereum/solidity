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
#include <libsolidity/interface/output/JSONDeserialization.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/JSON.h>

#include <libsolidity/ast/ASTEnums.h>

#include <utility>
#include <vector>

using namespace solidity::frontend;
using namespace solidity::langutil;
using namespace solidity::util;

namespace {

	template <typename T>
	std::optional<T> getOptional(solidity::Json const& _json, std::string_view _key)
	{
		auto it = _json.find(_key);
		if (it == _json.end())
			return std::nullopt;
		return it->get<T>();
	}
}

void output::from_json(Json const& _json, output::SourceLocation& _sourceLocation)
{
	_sourceLocation = output::SourceLocation{
		_json["file"].get<std::string>(),
		_json["start"].get<int>(),
		_json["end"].get<int>(),
		getOptional<std::string>(_json, "message")
	};
}

void output::from_json(Json const& _json, output::Error& _error)
{
	_error.sourceLocation = getOptional<SourceLocation>(_json, "sourceLocation");
	_error.secondarySourceLocations = getOptional<std::vector<SourceLocation>>(_json, "secondarySourceLocations");

	if (auto errorCode = getOptional<std::string>(_json, "errorCode"))
		_error.errorCode = langutil::ErrorId{std::stoull(*errorCode)};

	auto type = langutil::Error::parseErrorType(_json.at("type").get<std::string>());
	solAssert(type);
	_error.type = type.value();

	_error.message = _json.at("message").get<std::string>();
}

void output::from_json(Json const& _json, Source& _source)
{
	_source.id = _json.at("id").get<size_t>();
}

void output::from_json(Json const& _json, ABIParameter& _param)
{
    _param.name = _json.at("name").get<std::string>();
    _param.type = _json.at("type").get<std::string>();
    _param.internalType = getOptional<std::string>(_json, "internalType");
    _param.indexed = getOptional<bool>(_json, "indexed");
	_param.components = getOptional<std::vector<ABIParameter>>(_json, "components");
}

void output::from_json(Json const& _json, ABIConstructor& _constructor)
{
	auto mutability = stateMutabilityFromString(_json.at("stateMutability").get<std::string>());
	solAssert(mutability, "Invalid state mutability");
	_constructor.stateMutability = *mutability;
    _constructor.inputs = _json.at("inputs").get<std::vector<ABIParameter>>();
}

void output::from_json(Json const& _json, ABIFunction& _function)
{
    _function.name = _json.at("name").get<std::string>();
    auto mutability = stateMutabilityFromString(_json.at("stateMutability").get<std::string>());
    solAssert(mutability, "Invalid state mutability");
    _function.stateMutability = *mutability;
    _function.inputs = _json.at("inputs").get<std::vector<ABIParameter>>();
    _function.outputs = _json.at("outputs").get<std::vector<ABIParameter>>();
}

void output::from_json(Json const& _json, ABIFallback& _fallback)
{
	auto mutability = stateMutabilityFromString(_json.at("stateMutability").get<std::string>());
	solAssert(mutability, "Invalid state mutability");
	_fallback.stateMutability = *mutability;
}

void output::from_json(Json const& _json, ABIReceive& _receive)
{
	auto mutability = stateMutabilityFromString(_json.at("stateMutability").get<std::string>());
	solAssert(mutability, "Invalid state mutability");
	_receive.stateMutability = *mutability;
}

void output::from_json(Json const& _json, ABIEvent& _event)
{
    _event.name = _json.at("name").get<std::string>();
    _event.isAnonymous = _json.at("anonymous").get<bool>();
    _event.inputs = _json.at("inputs").get<std::vector<ABIParameter>>();
}

void output::from_json(Json const& _json, ABIError& _event)
{
    _event.name = _json.at("name").get<std::string>();
    _event.inputs = _json.at("inputs").get<std::vector<ABIParameter>>();
}

void output::from_json(Json const& _json, ABIEntry& _entry)
{
    auto const type = _json.at("type").get<std::string>();
	if (type == "constructor")
		_entry = _json.get<ABIConstructor>();
	else if (type == "function")
		_entry = _json.get<ABIFunction>();
	else if (type == "fallback")
		_entry = _json.get<ABIFallback>();
	else if (type == "receive")
		_entry = _json.get<ABIReceive>();
	else if (type == "event")
		_entry = _json.get<ABIEvent>();
	else if (type == "error")
		_entry = _json.get<ABIError>();
}

void output::from_json(Json const& _json, ByteOffset& _byteOffset)
{
	_byteOffset.start = _json.at("start").get<size_t>();
	_byteOffset.length = _json.at("length").get<size_t>();
}

void output::from_json(Json const& _json, Bytecode& _bytecode)
{
	_bytecode.object = util::fromHex(_json.at("object").get<std::string>());
	_bytecode.linkReferences = _json.at("linkReferences").get<std::map<std::string, LinkReferences>>();
}

void output::from_json(Json const& _json, EVM& _evm)
{
	_evm.bytecode = _json.at("bytecode").get<Bytecode>();
	_evm.methodIdentifiers = _json.at("methodIdentifiers").get<std::map<std::string, std::string>>();
}

void output::from_json(Json const& _json, Contract& _contract)
{
	_contract.abi = _json.at("abi").get<ABI>();
	_contract.evm = _json.at("evm").get<EVM>();
	_contract.metadata = _json.at("metadata").get<std::string>();
}

void output::from_json(Json const& _json, Contracts& _contracts)
{
    for (auto const& [source, contractsJson] : _json.items())
        for (auto const& [name, contractJson] : contractsJson.items())
		{
            auto contract = contractJson.get<Contract>();
			contract.name = name;
			_contracts[source].insert({name, std::move(contract)});
		}
}

void output::from_json(Json const& _json, output::Output& _output)
{
	if (_json.contains("errors"))
		_output.errors = _json.at("errors").get<Errors>();
	if (_json.contains("sources"))
		_output.sources = _json.at("sources").get<Sources>();
	if (_json.contains("contracts"))
		_output.contracts = _json.at("contracts").get<Contracts>();
}

