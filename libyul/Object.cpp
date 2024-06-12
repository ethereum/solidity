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
#include <libyul/Utilities.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string.hpp>

#include <range/v3/algorithm/none_of.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

namespace
{

std::string formatUseSrcComment(ObjectDebugData const& _debugData)
{
	if (!_debugData.sourceNames)
		return "";

	static auto formatIdNamePair = [](auto&& _pair) {
		return std::to_string(_pair.first) + ":" + util::escapeAndQuoteString(*_pair.second);
	};

	std::string serializedSourceNames = joinHumanReadable(
		ranges::views::transform(*_debugData.sourceNames, formatIdNamePair)
	);
	return "/// @use-src " + serializedSourceNames + "\n";
}

}

std::string Data::toString(Dialect const*, DebugInfoSelection const&, CharStreamProvider const*) const
{
	return "data \"" + name.str() + "\" hex\"" + util::toHex(data) + "\"";
}

std::string Object::toString(
	Dialect const* _dialect,
	DebugInfoSelection const& _debugInfoSelection,
	CharStreamProvider const* _soliditySourceProvider
) const
{
	yulAssert(code, "No code");
	yulAssert(debugData, "No debug data");

	std::string inner = "code " + AsmPrinter(
		_dialect,
		debugData->sourceNames,
		_debugInfoSelection,
		_soliditySourceProvider
	)(*code);

	for (auto const& obj: subObjects)
		inner += "\n" + obj->toString(_dialect, _debugInfoSelection, _soliditySourceProvider);

	return
		formatUseSrcComment(*debugData) +
		"object \"" + name.str() + "\" {\n" +
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

Json Object::toJson() const
{
	yulAssert(code, "No code");

	Json codeJson;
	codeJson["nodeType"] = "YulCode";
	codeJson["block"] = AsmJsonConverter(0 /* sourceIndex */)(*code);

	Json subObjectsJson = Json::array();
	for (std::shared_ptr<ObjectNode> const& subObject: subObjects)
		subObjectsJson.emplace_back(subObject->toJson());

	Json ret;
	ret["nodeType"] = "YulObject";
	ret["name"] = name.str();
	ret["code"] = codeJson;
	ret["subObjects"] = subObjectsJson;
	return ret;
}

std::set<YulString> Object::qualifiedDataNames() const
{
	std::set<YulString> qualifiedNames =
		name.empty() || util::contains(name.str(), '.') ?
		std::set<YulString>{} :
		std::set<YulString>{name};
	for (std::shared_ptr<ObjectNode> const& subObjectNode: subObjects)
	{
		yulAssert(qualifiedNames.count(subObjectNode->name) == 0, "");
		if (util::contains(subObjectNode->name.str(), '.'))
			continue;
		qualifiedNames.insert(subObjectNode->name);
		if (auto const* subObject = dynamic_cast<Object const*>(subObjectNode.get()))
			for (YulString const& subSubObj: subObject->qualifiedDataNames())
				if (subObject->name != subSubObj)
				{
					yulAssert(qualifiedNames.count(YulString{subObject->name.str() + "." + subSubObj.str()}) == 0, "");
					qualifiedNames.insert(YulString{subObject->name.str() + "." + subSubObj.str()});
				}
	}

	yulAssert(qualifiedNames.count(YulString{}) == 0, "");
	qualifiedNames.erase(YulString{});
	return qualifiedNames;
}

std::vector<size_t> Object::pathToSubObject(YulString _qualifiedName) const
{
	yulAssert(_qualifiedName != name, "");
	yulAssert(subIndexByName.count(name) == 0, "");

	if (boost::algorithm::starts_with(_qualifiedName.str(), name.str() + "."))
		_qualifiedName = YulString{_qualifiedName.str().substr(name.str().length() + 1)};
	yulAssert(!_qualifiedName.empty(), "");

	std::vector<std::string> subObjectPathComponents;
	boost::algorithm::split(subObjectPathComponents, _qualifiedName.str(), boost::is_any_of("."));

	std::vector<size_t> path;
	Object const* object = this;
	for (std::string const& currentSubObjectName: subObjectPathComponents)
	{
		yulAssert(!currentSubObjectName.empty(), "");
		auto subIndexIt = object->subIndexByName.find(YulString{currentSubObjectName});
		yulAssert(
			subIndexIt != object->subIndexByName.end(),
			"Assembly object <" + _qualifiedName.str() + "> not found or does not contain code."
		);
		object = dynamic_cast<Object const*>(object->subObjects[subIndexIt->second].get());
		yulAssert(object, "Assembly object <" + _qualifiedName.str() + "> not found or does not contain code.");
		yulAssert(object->subId != std::numeric_limits<size_t>::max(), "");
		path.push_back({object->subId});
	}

	return path;
}

std::string ObjectSource::toString() const
{
	// Whitespace here was chosen to match what IRGenerator::run() used to do.
	// TODO: Remove whitespace tweaks and update test expectations.

	// NOTE: Debug info selection and source provider only matter for Object. In ObjectSource the
	// debug data (if present at all) is hard-coded in the source.
	return "\n" + reindent(toString(nullptr, DebugInfoSelection::None(), nullptr)) + "\n";
}

std::string ObjectSource::toString(
	Dialect const* _dialect,
	DebugInfoSelection const& _debugInfoSelection,
	CharStreamProvider const* _soliditySourceProvider
) const
{
	// These parameters make no sense for SourceObject but are present in ObjectNode.
	// None of them will affect how the object is printed since these aspects are all hard-coded in the source.
	yulAssert(!_dialect);
	yulAssert(_debugInfoSelection == DebugInfoSelection::None());
	yulAssert(!_soliditySourceProvider);
	yulAssert(debugData);

	std::string serializedSubObjects;
	for (std::shared_ptr<ObjectNode> const& subObject: subObjects)
	{
		yulAssert(subObject, "Incomplete objects cannot be printed.");
		yulAssert(
			dynamic_cast<ObjectSource const*>(subObject.get()) ||
			dynamic_cast<Data const*>(subObject.get()),
			"ObjectSource tree may be composed only of Data and SourceObjects."
		);
		serializedSubObjects += subObject->toString(_dialect, _debugInfoSelection, _soliditySourceProvider) + "\n";
	}

	// Whitespace here was chosen to match what IRGenerator::run() used to do.
	// TODO: Remove whitespace tweaks and update test expectations.
	if (subObjects.size() > 0 && subObjects.back()->name == YulString{yul::Object::metadataName()})
		serializedSubObjects = "\n" + serializedSubObjects;
	else
		serializedSubObjects += "\n";

	return
		formatUseSrcComment(*debugData) +
		"object \"" + name.str() + "\" {\n" +
		"code " + code + "\n" +
		serializedSubObjects +
		"}";
}

void ObjectSource::addSubObject(std::shared_ptr<ObjectNode> _subObject)
{
	yulAssert(_subObject);
	yulAssert(
		!dynamic_cast<Object const*>(_subObject.get()),
		"Mixing ObjectSource and Object in the same tree not supported."
	);

	subIndexByName[_subObject->name] = subObjects.size();
	subObjects.push_back(std::move(_subObject));
}

void ObjectSource::addSubObjectPlaceholder(YulString _name)
{
	yulAssert(subIndexByName.count(_name) == 0);
	subIndexByName[_name] = subObjects.size();
	subObjects.push_back(nullptr);
}

void ObjectSource::addMetadata(bytes const& _cborMetadata)
{
	YulString subName = YulString{yul::Object::metadataName()};
	yulAssert(subIndexByName.count(subName) == 0);

	auto metadata = std::make_shared<Data>(subName, _cborMetadata);
	subIndexByName[subName] = subObjects.size();
	subObjects.push_back(std::move(metadata));
}

std::shared_ptr<ObjectSource> ObjectSource::clone() const
{
	std::vector<std::shared_ptr<ObjectNode>> clonedSubObjects;
	for (std::shared_ptr<ObjectNode> subNode: subObjects)
		if (!subNode)
			clonedSubObjects.push_back(nullptr);
		else if (dynamic_cast<Data const*>(subNode.get()))
			clonedSubObjects.push_back(subNode);
		else if (auto const& subObjectSource = dynamic_cast<ObjectSource const*>(subNode.get()))
			clonedSubObjects.push_back(subObjectSource->clone());
		else
			yulAssert(false);

	// TMP: Add constructors
	auto clonedObject = std::make_shared<ObjectSource>();
	clonedObject->name = name;
	clonedObject->code = code;
	clonedObject->subObjects = std::move(clonedSubObjects);
	clonedObject->subIndexByName = subIndexByName;
	clonedObject->debugData = debugData;
	return clonedObject;
}

bool ObjectSource::fillPlaceholders(std::map<std::string, std::shared_ptr<ObjectSource>> const& _availableSources)
{
	auto isCurrentObject = [this](auto const& _nameAndObject) { return _nameAndObject.second.get() == this; };
	solAssert(ranges::none_of(_availableSources, isCurrentObject));

	bool allFilled = true;
	for (auto const& [subName, subIndex]: subIndexByName)
	{
		yulAssert(subObjects.size() > subIndex);
		if (!subObjects[subIndex])
		{
			if (_availableSources.count(subName.str()))
				subObjects[subIndex] = _availableSources.at(subName.str());
			else
				allFilled = false;
		}
		else if (auto subObjectSource = std::dynamic_pointer_cast<ObjectSource>(subObjects[subIndex]))
			subObjectSource->fillPlaceholders(_availableSources);
	}

	return allFilled;
}
