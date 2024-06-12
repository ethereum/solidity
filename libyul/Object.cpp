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
 * Yul code and data object container.
 */

#include <libyul/Object.h>

#include <libyul/AsmPrinter.h>
#include <libyul/AsmJsonConverter.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string.hpp>

#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

std::string Data::toString(Dialect const&, AsmPrinter::TypePrinting, DebugInfoSelection const&, CharStreamProvider const*) const
{
	return "data \"" + name + "\" hex\"" + util::toHex(data) + "\"";
}

std::string Object::toString(
	Dialect const& _dialect,
	AsmPrinter::TypePrinting const _printingMode,
	DebugInfoSelection const& _debugInfoSelection,
	CharStreamProvider const* _soliditySourceProvider
) const
{
	yulAssert(hasCode(), "No code");
	yulAssert(debugData, "No debug data");

	std::string inner = "code " + AsmPrinter(
		_printingMode,
		_dialect,
		debugData->sourceNames,
		_debugInfoSelection,
		_soliditySourceProvider
	)(code()->root());

	for (auto const& obj: subObjects)
		inner += "\n" + obj->toString(_dialect, _printingMode, _debugInfoSelection, _soliditySourceProvider);

	return
		debugData->formatUseSrcComment() +
		"object \"" + name + "\" {\n" +
		indent(inner) + "\n" +
		"}";
}

Json Data::toJson() const
{
	Json ret;
	ret["nodeType"] = "YulData";
	ret["value"] = util::toHex(data);
	return ret;
}

std::string ObjectDebugData::formatUseSrcComment() const
{
	if (!sourceNames)
		return "";

	auto formatIdNamePair = [](auto&& _pair) {
		return std::to_string(_pair.first) + ":" + util::escapeAndQuoteString(*_pair.second);
	};

	std::string serializedSourceNames = joinHumanReadable(
		ranges::views::transform(*sourceNames, formatIdNamePair)
	);
	return "/// @use-src " + serializedSourceNames + "\n";
}

Json Object::toJson() const
{
	yulAssert(hasCode(), "No code");

	Json codeJson;
	codeJson["nodeType"] = "YulCode";
	codeJson["block"] = AsmJsonConverter(0 /* sourceIndex */)(code()->root());

	Json subObjectsJson = Json::array();
	for (std::shared_ptr<ObjectNode> const& subObject: subObjects)
		subObjectsJson.emplace_back(subObject->toJson());

	Json ret;
	ret["nodeType"] = "YulObject";
	ret["name"] = name;
	ret["code"] = codeJson;
	ret["subObjects"] = subObjectsJson;
	return ret;
}

std::set<std::string> Object::qualifiedDataNames() const
{
	std::set<std::string> qualifiedNames =
		name.empty() || util::contains(name, '.') ?
		std::set<std::string>{} :
		std::set<std::string>{name};
	for (std::shared_ptr<ObjectNode> const& subObjectNode: subObjects)
	{
		yulAssert(qualifiedNames.count(subObjectNode->name) == 0, "");
		if (util::contains(subObjectNode->name, '.'))
			continue;
		qualifiedNames.insert(subObjectNode->name);
		if (auto const* subObject = dynamic_cast<Object const*>(subObjectNode.get()))
			for (auto const& subSubObj: subObject->qualifiedDataNames())
				if (subObject->name != subSubObj)
				{
					yulAssert(qualifiedNames.count(subObject->name + "." + subSubObj) == 0, "");
					qualifiedNames.insert(subObject->name + "." + subSubObj);
				}
	}

	yulAssert(qualifiedNames.count("") == 0, "");
	return qualifiedNames;
}

std::vector<size_t> Object::pathToSubObject(std::string_view _qualifiedName) const
{
	yulAssert(_qualifiedName != name, "");
	yulAssert(subIndexByName.count(name) == 0, "");

	if (boost::algorithm::starts_with(_qualifiedName, name + "."))
		_qualifiedName = _qualifiedName.substr(name.length() + 1);
	yulAssert(!_qualifiedName.empty(), "");

	std::vector<std::string> subObjectPathComponents;
	boost::algorithm::split(subObjectPathComponents, _qualifiedName, boost::is_any_of("."));

	std::vector<size_t> path;
	Object const* object = this;
	for (std::string const& currentSubObjectName: subObjectPathComponents)
	{
		yulAssert(!currentSubObjectName.empty(), "");
		auto subIndexIt = object->subIndexByName.find(currentSubObjectName);
		yulAssert(
			subIndexIt != object->subIndexByName.end(),
			"Assembly object <" + std::string(_qualifiedName) + "> not found or does not contain code."
		);
		object = dynamic_cast<Object const*>(object->subObjects[subIndexIt->second].get());
		yulAssert(object, "Assembly object <" + std::string(_qualifiedName) + "> not found or does not contain code.");
		yulAssert(object->subId != std::numeric_limits<size_t>::max(), "");
		path.push_back({object->subId});
	}

	return path;
}

std::shared_ptr<AST const> Object::code() const
{
	return m_code;
}

bool Object::hasCode() const { return code() != nullptr; }

void Object::setCode(std::shared_ptr<AST const> const& _ast, std::shared_ptr<yul::AsmAnalysisInfo> _analysisInfo)
{
	m_code = _ast;
	analysisInfo = std::move(_analysisInfo);
}

void Object::collectSourceIndices(std::map<std::string, unsigned>& _indices) const
{
	if (debugData && debugData->sourceNames.has_value())
		for (auto const& [sourceIndex, sourceName]: debugData->sourceNames.value())
		{
			solAssert(_indices.count(*sourceName) == 0 || _indices[*sourceName] == sourceIndex);
			_indices[*sourceName] = sourceIndex;
		}
	for (std::shared_ptr<ObjectNode> const& subNode: subObjects)
		if (auto subObject = dynamic_cast<Object*>(subNode.get()))
			subObject->collectSourceIndices(_indices);
}

bool Object::hasContiguousSourceIndices() const
{
	std::map<std::string, unsigned> sourceIndices;
	collectSourceIndices(sourceIndices);

	unsigned maxSourceIndex = 0;
	std::set<unsigned> indices;
	for (auto const& [sources, sourceIndex]: sourceIndices)
	{
		maxSourceIndex = std::max(sourceIndex, maxSourceIndex);
		indices.insert(sourceIndex);
	}

	solAssert(maxSourceIndex + 1 >= indices.size());
	return indices.size() == 0 || indices.size() == maxSourceIndex + 1;
}
