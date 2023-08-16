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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Takes the parsed AST and produces the Natspec documentation:
 * https://github.com/ethereum/wiki/wiki/Ethereum-Natural-Specification-Format
 *
 * Can generally deal with JSON files
 */

#include <libsolidity/interface/Natspec.h>

#include <libsolidity/ast/AST.h>

#include <boost/algorithm/string.hpp>

using namespace solidity;
using namespace solidity::frontend;

Json::Value Natspec::userDocumentation(ContractDefinition const& _contractDef)
{
	Json::Value doc{Json::objectValue};

	doc["version"] = Json::Value(c_natspecVersion);
	doc["kind"]    = Json::Value("user");
	doc["methods"] = Json::objectValue;

	auto constructorDefinition(_contractDef.constructor());
	if (constructorDefinition)
	{
		std::string const value = extractDoc(constructorDefinition->annotation().docTags, "notice");
		if (!value.empty())
		{
			// add the constructor, only if we have any documentation to add
			Json::Value user{Json::objectValue};
			user["notice"] = Json::Value(value);
			doc["methods"]["constructor"] = user;
		}
	}

	std::string notice = extractDoc(_contractDef.annotation().docTags, "notice");
	if (!notice.empty())
		doc["notice"] = Json::Value(notice);

	for (auto const& it: _contractDef.interfaceFunctions())
		if (it.second->hasDeclaration())
		{
			std::string value;

			if (auto const* f = dynamic_cast<FunctionDefinition const*>(&it.second->declaration()))
				value = extractDoc(f->annotation().docTags, "notice");
			else if (auto var = dynamic_cast<VariableDeclaration const*>(&it.second->declaration()))
			{
				solAssert(var->isStateVariable() && var->isPublic(), "");
				value = extractDoc(var->annotation().docTags, "notice");
			}

			if (!value.empty())
				// since @notice is the only user tag if missing function should not appear
				doc["methods"][it.second->externalSignature()]["notice"] = value;
		}

	for (auto const& event: uniqueInterfaceEvents(_contractDef))
	{
		std::string value = extractDoc(event->annotation().docTags, "notice");
		if (!value.empty())
			doc["events"][event->functionType(true)->externalSignature()]["notice"] = value;
	}

	for (auto const& error: _contractDef.interfaceErrors())
	{
		std::string value = extractDoc(error->annotation().docTags, "notice");
		if (!value.empty())
		{
			Json::Value errorDoc{Json::objectValue};
			errorDoc["notice"] = value;
			doc["errors"][error->functionType(true)->externalSignature()].append(std::move(errorDoc));
		}
	}

	return doc;
}

Json::Value Natspec::devDocumentation(ContractDefinition const& _contractDef)
{
	Json::Value doc = extractCustomDoc(_contractDef.annotation().docTags);

	doc["version"] = Json::Value(c_natspecVersion);
	doc["kind"] = Json::Value("dev");

	auto author = extractDoc(_contractDef.annotation().docTags, "author");
	if (!author.empty())
		doc["author"] = author;
	auto title = extractDoc(_contractDef.annotation().docTags, "title");
	if (!title.empty())
		doc["title"] = title;
	auto dev = extractDoc(_contractDef.annotation().docTags, "dev");
	if (!dev.empty())
		doc["details"] = Json::Value(dev);

	doc["methods"] = Json::objectValue;
	auto constructorDefinition(_contractDef.constructor());
	if (constructorDefinition)
	{
		Json::Value constructor(devDocumentation(constructorDefinition->annotation().docTags));
		if (!constructor.empty())
			// add the constructor, only if we have any documentation to add
			doc["methods"]["constructor"] = constructor;
	}

	for (auto const& it: _contractDef.interfaceFunctions())
	{
		if (!it.second->hasDeclaration())
			continue;
		if (auto fun = dynamic_cast<FunctionDefinition const*>(&it.second->declaration()))
		{
			Json::Value method(devDocumentation(fun->annotation().docTags));
			// add the function, only if we have any documentation to add
			Json::Value jsonReturn = extractReturnParameterDocs(
				fun->annotation().docTags,
				fun->functionType(false)->returnParameterNames()
			);

			if (!jsonReturn.empty())
				method["returns"] = std::move(jsonReturn);

			if (!method.empty())
				doc["methods"][it.second->externalSignature()] = std::move(method);
		}
	}

	for (VariableDeclaration const* varDecl: _contractDef.stateVariables())
	{
		if (auto devDoc = devDocumentation(varDecl->annotation().docTags); !devDoc.empty())
			doc["stateVariables"][varDecl->name()] = devDoc;

		auto const assignIfNotEmpty = [&](std::string const& _name, Json::Value const& _content)
		{
			if (!_content.empty())
				doc["stateVariables"][varDecl->name()][_name] = _content;
		};

		if (varDecl->annotation().docTags.count("return") == 1)
			assignIfNotEmpty("return", extractDoc(varDecl->annotation().docTags, "return"));

		if (FunctionTypePointer functionType = varDecl->functionType(false))
			assignIfNotEmpty("returns", extractReturnParameterDocs(
				varDecl->annotation().docTags,
				functionType->returnParameterNames()
			));
	}

	for (auto const& event: uniqueInterfaceEvents(_contractDef))
		if (auto devDoc = devDocumentation(event->annotation().docTags); !devDoc.empty())
			doc["events"][event->functionType(true)->externalSignature()] = devDoc;
	for (auto const& error: _contractDef.interfaceErrors())
		if (auto devDoc = devDocumentation(error->annotation().docTags); !devDoc.empty())
			doc["errors"][error->functionType(true)->externalSignature()].append(devDoc);

	return doc;
}

Json::Value Natspec::extractReturnParameterDocs(std::multimap<std::string, DocTag> const& _tags, std::vector<std::string> const& _returnParameterNames)
{
	Json::Value jsonReturn{Json::objectValue};
	auto returnDocs = _tags.equal_range("return");

	if (!_returnParameterNames.empty())
	{
		size_t n = 0;
		for (auto i = returnDocs.first; i != returnDocs.second; i++)
		{
			std::string paramName = _returnParameterNames.at(n);
			std::string content = i->second.content;

			if (paramName.empty())
				paramName = "_" + std::to_string(n);
			else
			{
				//check to make sure the first word of the doc str is the same as the return name
				auto nameEndPos = content.find_first_of(" \t");
				solAssert(content.substr(0, nameEndPos) == paramName, "No return param name given: " + paramName);
				content = content.substr(nameEndPos+1);
			}

			jsonReturn[paramName] = Json::Value(content);
			n++;
		}
	}

	return jsonReturn;
}

std::string Natspec::extractDoc(std::multimap<std::string, DocTag> const& _tags, std::string const& _name)
{
	std::string value;
	auto range = _tags.equal_range(_name);
	for (auto i = range.first; i != range.second; i++)
		value += i->second.content;
	return value;
}

Json::Value Natspec::extractCustomDoc(std::multimap<std::string, DocTag> const& _tags)
{
	std::map<std::string, std::string> concatenated;
	for (auto const& [tag, value]: _tags)
		if (boost::starts_with(tag, "custom"))
			concatenated[tag] += value.content;
	// We do not want to create an object if there are no custom tags found.
	if (concatenated.empty())
		return Json::nullValue;
	Json::Value result{Json::objectValue};
	for (auto& [tag, value]: concatenated)
		result[tag] = std::move(value);
	return result;
}

Json::Value Natspec::devDocumentation(std::multimap<std::string, DocTag> const& _tags)
{
	Json::Value json = extractCustomDoc(_tags);
	auto dev = extractDoc(_tags, "dev");
	if (!dev.empty())
		json["details"] = Json::Value(dev);

	auto author = extractDoc(_tags, "author");
	if (!author.empty())
		json["author"] = author;

	Json::Value params(Json::objectValue);
	auto paramRange = _tags.equal_range("param");
	for (auto i = paramRange.first; i != paramRange.second; ++i)
		params[i->second.paramName] = Json::Value(i->second.content);

	if (!params.empty())
		json["params"] = params;

	return json;
}

std::vector<EventDefinition const*>  Natspec::uniqueInterfaceEvents(ContractDefinition const& _contract)
{
	auto eventSignature = [](EventDefinition const* _event) -> std::string {
		FunctionType const* functionType = _event->functionType(true);
		solAssert(functionType, "");
		return functionType->externalSignature();
	};
	auto compareBySignature =
		[&](EventDefinition const* _lhs, EventDefinition const* _rhs) -> bool {
			return eventSignature(_lhs) < eventSignature(_rhs);
		};

	std::set<EventDefinition const*, decltype(compareBySignature)> uniqueEvents{compareBySignature};
	// Insert events defined in the contract first so that in case of a conflict
	// they're the ones that get selected.
	uniqueEvents += _contract.definedInterfaceEvents();

	std::set<EventDefinition const*, decltype(compareBySignature)> filteredUsedEvents{compareBySignature};
	std::set<std::string> usedSignatures;
	for (EventDefinition const* event: _contract.usedInterfaceEvents())
	{
		auto&& [eventIt, eventInserted] = filteredUsedEvents.insert(event);
		auto&& [signatureIt, signatureInserted] = usedSignatures.insert(eventSignature(event));
		if (!signatureInserted)
			filteredUsedEvents.erase(eventIt);
	}

	uniqueEvents += filteredUsedEvents;
	return util::convertContainer<std::vector<EventDefinition const*>>(std::move(uniqueEvents));
}
