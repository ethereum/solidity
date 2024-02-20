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
/**
 * Utilities to handle the Contract ABI (https://docs.soliditylang.org/en/develop/abi-spec.html)
 */

#include <libsolidity/interface/ABI.h>

#include <libsolidity/ast/AST.h>

using namespace solidity;
using namespace solidity::frontend;

namespace
{
bool anyDataStoredInStorage(TypePointers const& _pointers)
{
	for (Type const* pointer: _pointers)
		if (pointer->dataStoredIn(DataLocation::Storage))
			return true;

	return false;
}
}

Json ABI::generate(ContractDefinition const& _contractDef)
{
	auto compare = [](Json const& _a, Json const& _b) -> bool
	{
		return std::make_tuple(_a.value("type", Json()), _a.value("name", Json())) <
					std::make_tuple(_b.value("type", Json()), _b.value("name", Json()));
	};
	std::multiset<Json, decltype(compare)> abi(compare);

	for (auto it: _contractDef.interfaceFunctions())
	{
		if (_contractDef.isLibrary() && (
			it.second->stateMutability() > StateMutability::View ||
			anyDataStoredInStorage(it.second->parameterTypes() + it.second->returnParameterTypes())
		))
			continue;

		FunctionType const* externalFunctionType = it.second->interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json method;
		method["type"] = "function";
		method["name"] = it.second->declaration().name();
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		method["inputs"] = formatTypeList(
			externalFunctionType->parameterNames(),
			externalFunctionType->parameterTypes(),
			it.second->parameterTypes(),
			_contractDef.isLibrary()
		);
		method["outputs"] = formatTypeList(
			externalFunctionType->returnParameterNames(),
			externalFunctionType->returnParameterTypes(),
			it.second->returnParameterTypes(),
			_contractDef.isLibrary()
		);
		abi.emplace(std::move(method));
	}
	FunctionDefinition const* constructor = _contractDef.constructor();
	if (constructor && !_contractDef.abstract())
	{
		FunctionType constrType(*constructor);
		FunctionType const* externalFunctionType = constrType.interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json method;
		method["type"] = "constructor";
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		method["inputs"] = formatTypeList(
			externalFunctionType->parameterNames(),
			externalFunctionType->parameterTypes(),
			constrType.parameterTypes(),
			_contractDef.isLibrary()
		);
		abi.emplace(std::move(method));
	}
	for (auto const* fallbackOrReceive: {_contractDef.fallbackFunction(), _contractDef.receiveFunction()})
		if (fallbackOrReceive)
		{
			auto const* externalFunctionType = FunctionType(*fallbackOrReceive).interfaceFunctionType();
			solAssert(!!externalFunctionType, "");
			Json method;
			method["type"] = TokenTraits::toString(fallbackOrReceive->kind());
			method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
			abi.emplace(std::move(method));
		}
	for (auto const& it: _contractDef.interfaceEvents())
	{
		Json event;
		event["type"] = "event";
		event["name"] = it->name();
		event["anonymous"] = it->isAnonymous();
		Json params = Json::array();
		for (auto const& p: it->parameters())
		{
			Type const* type = p->annotation().type->interfaceType(false);
			solAssert(type, "");
			auto param = formatType(p->name(), *type, *p->annotation().type, false);
			param["indexed"] = p->isIndexed();
			params.emplace_back(param);
		}
		event["inputs"] = std::move(params);
		abi.emplace(std::move(event));
	}

	for (ErrorDefinition const* error: _contractDef.interfaceErrors())
	{
		Json errorJson;
		errorJson["type"] = "error";
		errorJson["name"] = error->name();
		errorJson["inputs"] = Json::array();
		for (auto const& p: error->parameters())
		{
			Type const* type = p->annotation().type->interfaceType(false);
			solAssert(type, "");
			errorJson["inputs"].emplace_back(
				formatType(p->name(), *type, *p->annotation().type, false)
			);
		}
		abi.emplace(std::move(errorJson));
	}

	Json abiJson = Json::array();
	for (auto& f: abi)
		abiJson.emplace_back(std::move(f));
	return abiJson;
}

Json ABI::formatTypeList(
	std::vector<std::string> const& _names,
	std::vector<Type const*> const& _encodingTypes,
	std::vector<Type const*> const& _solidityTypes,
	bool _forLibrary
)
{
	Json params = Json::array();
	solAssert(_names.size() == _encodingTypes.size(), "Names and types vector size does not match");
	solAssert(_names.size() == _solidityTypes.size(), "");
	for (unsigned i = 0; i < _names.size(); ++i)
	{
		solAssert(_encodingTypes[i], "");
		params.emplace_back(formatType(_names[i], *_encodingTypes[i], *_solidityTypes[i], _forLibrary));
	}
	return params;
}

Json ABI::formatType(
	std::string const& _name,
	Type const& _encodingType,
	Type const& _solidityType,
	bool _forLibrary
)
{
	Json ret;
	ret["name"] = _name;
	ret["internalType"] = _solidityType.toString(true);
	std::string suffix = (_forLibrary && _encodingType.dataStoredIn(DataLocation::Storage)) ? " storage" : "";
	if (_encodingType.isValueType() || (_forLibrary && _encodingType.dataStoredIn(DataLocation::Storage)))
		ret["type"] = _encodingType.canonicalName() + suffix;
	else if (ArrayType const* arrayType = dynamic_cast<ArrayType const*>(&_encodingType))
	{
		if (arrayType->isByteArrayOrString())
			ret["type"] = _encodingType.canonicalName() + suffix;
		else
		{
			std::string suffix;
			if (arrayType->isDynamicallySized())
				suffix = "[]";
			else
				suffix = std::string("[") + arrayType->length().str() + "]";
			solAssert(arrayType->baseType(), "");
			Json subtype = formatType(
				"",
				*arrayType->baseType(),
				*dynamic_cast<ArrayType const&>(_solidityType).baseType(),
				_forLibrary
			);
			if (subtype.contains("components"))
			{
				ret["type"] = subtype["type"].get<std::string>() + suffix;
				ret["components"] = subtype["components"];
			}
			else
				ret["type"] = subtype["type"].get<std::string>() + suffix;
		}
	}
	else if (StructType const* structType = dynamic_cast<StructType const*>(&_encodingType))
	{
		ret["type"] = "tuple";
		ret["components"] = Json::array();
		for (auto const& member: structType->members(nullptr))
		{
			solAssert(member.type, "");
			Type const* t = member.type->interfaceType(_forLibrary);
			solAssert(t, "");
			ret["components"].emplace_back(formatType(member.name, *t, *member.type, _forLibrary));
		}
	}
	else
		solAssert(false, "Invalid type.");
	return ret;
}
