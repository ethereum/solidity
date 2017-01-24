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
 * Utilities to handle the Contract ABI (https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI)
 */

#include <libsolidity/interface/ABI.h>
#include <boost/range/irange.hpp>
#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Json::Value ABI::generate(ContractDefinition const& _contractDef)
{
	Json::Value abi(Json::arrayValue);

	for (auto it: _contractDef.interfaceFunctions())
	{
		auto externalFunctionType = it.second->interfaceFunctionType();
		Json::Value method;
		method["type"] = "function";
		method["name"] = it.second->declaration().name();
		method["constant"] = it.second->isConstant();
		method["payable"] = it.second->isPayable();
		method["inputs"] = formatTypeList(
			externalFunctionType->parameterNames(),
			externalFunctionType->parameterTypes(),
			_contractDef.isLibrary()
		);
		method["outputs"] = formatTypeList(
			externalFunctionType->returnParameterNames(),
			externalFunctionType->returnParameterTypes(),
			_contractDef.isLibrary()
		);
		abi.append(method);
	}
	if (_contractDef.constructor())
	{
		Json::Value method;
		method["type"] = "constructor";
		auto externalFunction = FunctionType(*_contractDef.constructor(), false).interfaceFunctionType();
		solAssert(!!externalFunction, "");
		method["payable"] = externalFunction->isPayable();
		method["inputs"] = formatTypeList(
			externalFunction->parameterNames(),
			externalFunction->parameterTypes(),
			_contractDef.isLibrary()
		);
		abi.append(method);
	}
	if (_contractDef.fallbackFunction())
	{
		auto externalFunctionType = FunctionType(*_contractDef.fallbackFunction(), false).interfaceFunctionType();
		solAssert(!!externalFunctionType, "");
		Json::Value method;
		method["type"] = "fallback";
		method["payable"] = externalFunctionType->isPayable();
		abi.append(method);
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
			auto type = p->annotation().type->interfaceType(false);
			solAssert(type, "");
			Json::Value input;
			auto param = formatType(p->name(), *type, false);
			param["indexed"] = p->isIndexed();
			params.append(param);
		}
		event["inputs"] = params;
		abi.append(event);
	}

	return abi;
}

Json::Value ABI::formatTypeList(
	vector<string> const& _names,
	vector<TypePointer> const& _types,
	bool _forLibrary
)
{
	Json::Value params(Json::arrayValue);
	solAssert(_names.size() == _types.size(), "Names and types vector size does not match");
	for (unsigned i = 0; i < _names.size(); ++i)
	{
		solAssert(_types[i], "");
		params.append(formatType(_names[i], *_types[i], _forLibrary));
	}
	return params;
}

Json::Value ABI::formatType(string const& _name, Type const& _type, bool _forLibrary)
{
	Json::Value ret;
	ret["name"] = _name;
	if (_type.isValueType() || (_forLibrary && _type.dataStoredIn(DataLocation::Storage)))
		ret["type"] = _type.canonicalName(_forLibrary);
	else if (ArrayType const* arrayType = dynamic_cast<ArrayType const*>(&_type))
	{
		if (arrayType->isByteArray())
			ret["type"] = _type.canonicalName(_forLibrary);
		else
		{
			string suffix;
			if (arrayType->isDynamicallySized())
				suffix = "[]";
			else
				suffix = string("[") + arrayType->length().str() + "]";
			solAssert(arrayType->baseType(), "");
			Json::Value subtype = formatType("", *arrayType->baseType(), _forLibrary);
			if (subtype["type"].isString() && !subtype.isMember("subtype"))
				ret["type"] = subtype["type"].asString() + suffix;
			else
			{
				ret["type"] = suffix;
				solAssert(!subtype.isMember("subtype"), "");
				ret["subtype"] = subtype["type"];
			}
		}
	}
	else if (StructType const* structType = dynamic_cast<StructType const*>(&_type))
	{
		ret["type"] = Json::arrayValue;
		for (auto const& member: structType->members(nullptr))
		{
			solAssert(member.type, "");
			ret["type"].append(formatType(member.name, *member.type, _forLibrary));
		}
	}
	else
		solAssert(false, "Invalid type.");
	return ret;
}
