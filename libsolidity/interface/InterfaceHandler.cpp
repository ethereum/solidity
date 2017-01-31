
#include <libsolidity/interface/InterfaceHandler.h>
#include <boost/range/irange.hpp>
#include <libsolidity/ast/AST.h>
#include <libsolidity/interface/CompilerStack.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Json::Value InterfaceHandler::documentation(
	ContractDefinition const& _contractDef,
	DocumentationType _type
)
{
	switch(_type)
	{
	case DocumentationType::NatspecUser:
		return userDocumentation(_contractDef);
	case DocumentationType::NatspecDev:
		return devDocumentation(_contractDef);
	case DocumentationType::ABIInterface:
		return abiInterface(_contractDef);
	}

	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown documentation type"));
}

Json::Value InterfaceHandler::abiInterface(ContractDefinition const& _contractDef)
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
			solAssert(!!p->annotation().type->interfaceType(false), "");
			Json::Value input;
			input["name"] = p->name();
			input["type"] = p->annotation().type->interfaceType(false)->canonicalName(false);
			input["indexed"] = p->isIndexed();
			params.append(input);
		}
		event["inputs"] = params;
		abi.append(event);
	}

	return abi;
}

Json::Value InterfaceHandler::userDocumentation(ContractDefinition const& _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	for (auto const& it: _contractDef.interfaceFunctions())
		if (it.second->hasDeclaration())
			if (auto const* f = dynamic_cast<FunctionDefinition const*>(&it.second->declaration()))
			{
				string value = extractDoc(f->annotation().docTags, "notice");
				if (!value.empty())
				{
					Json::Value user;
					// since @notice is the only user tag if missing function should not appear
					user["notice"] = Json::Value(value);
					methods[it.second->externalSignature()] = user;
				}
			}
	doc["methods"] = methods;

	return doc;
}

Json::Value InterfaceHandler::devDocumentation(ContractDefinition const& _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	auto author = extractDoc(_contractDef.annotation().docTags, "author");
	if (!author.empty())
		doc["author"] = author;
	auto title = extractDoc(_contractDef.annotation().docTags, "title");
	if (!title.empty())
		doc["title"] = title;

	for (auto const& it: _contractDef.interfaceFunctions())
	{
		if (!it.second->hasDeclaration())
			continue;
		Json::Value method;
		if (auto fun = dynamic_cast<FunctionDefinition const*>(&it.second->declaration()))
		{
			auto dev = extractDoc(fun->annotation().docTags, "dev");
			if (!dev.empty())
				method["details"] = Json::Value(dev);

			auto author = extractDoc(fun->annotation().docTags, "author");
			if (!author.empty())
				method["author"] = author;

			auto ret = extractDoc(fun->annotation().docTags, "return");
			if (!ret.empty())
				method["return"] = ret;

			Json::Value params(Json::objectValue);
			auto paramRange = fun->annotation().docTags.equal_range("param");
			for (auto i = paramRange.first; i != paramRange.second; ++i)
				params[i->second.paramName] = Json::Value(i->second.content);

			if (!params.empty())
				method["params"] = params;

			if (!method.empty())
				// add the function, only if we have any documentation to add
				methods[it.second->externalSignature()] = method;
		}
	}
	doc["methods"] = methods;

	return doc;
}

Json::Value InterfaceHandler::formatTypeList(
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
		Json::Value param;
		param["name"] = _names[i];
		param["type"] = _types[i]->canonicalName(_forLibrary);
		params.append(param);
	}
	return params;
}

string InterfaceHandler::extractDoc(multimap<string, DocTag> const& _tags, string const& _name)
{
	string value;
	auto range = _tags.equal_range(_name);
	for (auto i = range.first; i != range.second; i++)
		value += i->second.content;
	return value;
}
