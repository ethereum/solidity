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
 * @author Lefteris <lefteris@ethdev.com>
 * @date 2014
 * Takes the parsed AST and produces the Natspec documentation:
 * https://github.com/ethereum/wiki/wiki/Ethereum-Natural-Specification-Format
 *
 * Can generally deal with JSON files
 */

#include <libsolidity/interface/Natspec.h>
#include <boost/range/irange.hpp>
#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;

Json::Value Natspec::userDocumentation(ContractDefinition const& _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	string notice = extractDoc(_contractDef.annotation().docTags, "notice");
	if (!notice.empty())
		doc["notice"] = Json::Value(notice);

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

Json::Value Natspec::devDocumentation(ContractDefinition const& _contractDef)
{
	Json::Value doc;
	Json::Value methods(Json::objectValue);

	auto author = extractDoc(_contractDef.annotation().docTags, "author");
	if (!author.empty())
		doc["author"] = author;
	auto title = extractDoc(_contractDef.annotation().docTags, "title");
	if (!title.empty())
		doc["title"] = title;
	auto dev = extractDoc(_contractDef.annotation().docTags, "dev");
	if (!dev.empty())
		doc["details"] = Json::Value(dev);

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

string Natspec::extractDoc(multimap<string, DocTag> const& _tags, string const& _name)
{
	string value;
	auto range = _tags.equal_range(_name);
	for (auto i = range.first; i != range.second; i++)
		value += i->second.content;
	return value;
}
