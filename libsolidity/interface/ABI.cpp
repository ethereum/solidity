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
 * Utilities to handle the Contract ABI (https://solidity.readthedocs.io/en/develop/abi-spec.html)
 */

#include <libsolidity/interface/ABI.h>

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

namespace
{
bool anyDataStoredInStorage(TypePointers const& _pointers)
{
	for (TypePointer const& pointer: _pointers)
		if (pointer->dataStoredIn(DataLocation::Storage))
			return true;

	return false;
}
}

Json::Value ABI::generate(ContractDefinition const& _contractDef)
{
	auto compare = [](Json::Value const& _a, Json::Value const& _b) -> bool {
		return make_tuple(_a["type"], _a["name"]) < make_tuple(_b["type"], _b["name"]);
	};
	multiset<Json::Value, decltype(compare)> abi(compare);

	for (auto it: _contractDef.interfaceFunctions())
	{
		if (_contractDef.isLibrary() && (
			it.second->stateMutability() > StateMutability::View ||
			anyDataStoredInStorage(it.second->parameterTypes() + it.second->returnParameterTypes())
		))
			continue;

		FunctionType const* externalFunctionType = it.second->interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json::Value method;
		method["type"] = "function";
		method["name"] = it.second->declaration().name();
		// TODO: deprecate constant in a future release
		method["constant"] = externalFunctionType->stateMutability() == StateMutability::Pure || it.second->stateMutability() == StateMutability::View;
		method["payable"] = externalFunctionType->isPayable();
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
	if (_contractDef.constructor())
	{
		FunctionType constrType(*_contractDef.constructor(), false);
		FunctionType const* externalFunctionType = constrType.interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json::Value method;
		method["type"] = "constructor";
		method["payable"] = externalFunctionType->isPayable();
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		method["inputs"] = formatTypeList(
			externalFunctionType->parameterNames(),
			externalFunctionType->parameterTypes(),
			constrType.parameterTypes(),
			_contractDef.isLibrary()
		);
		abi.emplace(std::move(method));
	}
	if (_contractDef.fallbackFunction())
	{
		FunctionType const* externalFunctionType = FunctionType(*_contractDef.fallbackFunction(), false).interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json::Value method;
		method["type"] = "fallback";
		method["payable"] = externalFunctionType->isPayable();
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		abi.emplace(std::move(method));
	}
	for (auto const& it: _contractDef.interfaceEvents())
	{
		Json::Value event;
		event["type"] = "event";
		event["name"] = it->name();
		event["anonymous"] = it->isAnonymous();
		Json::Value params(Json::arrayValue);
		for (auto const& p: it->parameters())
		{
			Type const* type = p->annotation().type->interfaceType(false);
			solAssert(type, "");
			Json::Value input;
			auto param = formatType(p->name(), *type, *p->annotation().type, false);
			param["indexed"] = p->isIndexed();
			params.append(std::move(param));
		}
		event["inputs"] = std::move(params);
		abi.emplace(std::move(event));
	}

	Json::Value abiJson{Json::arrayValue};
	for (auto& f: abi)
		abiJson.append(std::move(f));
	return abiJson;
}

Json::Value ABI::formatTypeList(
	vector<string> const& _names,
	vector<TypePointer> const& _encodingTypes,
	vector<TypePointer> const& _solidityTypes,
	bool _forLibrary
)
{
	Json::Value params(Json::arrayValue);
	solAssert(_names.size() == _encodingTypes.size(), "Names and types vector size does not match");
	solAssert(_names.size() == _solidityTypes.size(), "");
	for (unsigned i = 0; i < _names.size(); ++i)
	{
		solAssert(_encodingTypes[i], "");
		params.append(formatType(_names[i], *_encodingTypes[i], *_solidityTypes[i], _forLibrary));
	}
	return params;
}

Json::Value ABI::formatType(
	string const& _name,
	Type const& _encodingType,
	Type const& _solidityType,
	bool _forLibrary
)
{
	Json::Value ret;
	ret["name"] = _name;
	ret["internalType"] = _solidityType.toString(true);
	string suffix = (_forLibrary && _encodingType.dataStoredIn(DataLocation::Storage)) ? " storage" : "";
	if (_encodingType.isValueType() || (_forLibrary && _encodingType.dataStoredIn(DataLocation::Storage)))
		ret["type"] = _encodingType.canonicalName() + suffix;
	else if (ArrayType const* arrayType = dynamic_cast<ArrayType const*>(&_encodingType))
	{
		if (arrayType->isByteArray())
			ret["type"] = _encodingType.canonicalName() + suffix;
		else
		{
			string suffix;
			if (arrayType->isDynamicallySized())
				suffix = "[]";
			else
				suffix = string("[") + arrayType->length().str() + "]";
			solAssert(arrayType->baseType(), "");
			Json::Value subtype = formatType(
				"",
				*arrayType->baseType(),
				*dynamic_cast<ArrayType const&>(_solidityType).baseType(),
				_forLibrary
			);
			if (subtype.isMember("components"))
			{
				ret["type"] = subtype["type"].asString() + suffix;
				ret["components"] = subtype["components"];
			}
			else
				ret["type"] = subtype["type"].asString() + suffix;
		}
	}
	else if (StructType const* structType = dynamic_cast<StructType const*>(&_encodingType))
	{
		ret["type"] = "tuple";
		ret["components"] = Json::arrayValue;
		for (auto const& member: structType->members(nullptr))
		{
			solAssert(member.type, "");
			Type const* t = member.type->interfaceType(_forLibrary);
			solAssert(t, "");
			ret["components"].append(formatType(member.name, *t, *member.type, _forLibrary));
		}
	}
	else
		solAssert(false, "Invalid type.");
	return ret;
}
