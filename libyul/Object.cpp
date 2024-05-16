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

#include <range/v3/view/drop.hpp>
#include <range/v3/view/transform.hpp>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;


Object const* Object::at(YulString _name) const
{
	yulAssert(
		this->subIndexByName.count(_name),
		"Assembly object <" + _name.str() + "> not found."
	);
	size_t subIndex = this->subIndexByName.at(_name);
	return dynamic_cast<Object const*>(this->subObjects[subIndex].get());
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

	std::string useSrcComment;

	if (debugData->sourceNames)
		useSrcComment =
			"/// @use-src " +
			joinHumanReadable(ranges::views::transform(*debugData->sourceNames, [](auto&& _pair) {
				return std::to_string(_pair.first) + ":" + util::escapeAndQuoteString(*_pair.second);
			})) +
			"\n";

	std::string inner = "code " + AsmPrinter(
		_dialect,
		debugData->sourceNames,
		_debugInfoSelection,
		_soliditySourceProvider
	)(*code);

	for (auto const& obj: subObjects)
		inner += "\n" + obj->toString(_dialect, _debugInfoSelection, _soliditySourceProvider);

	return useSrcComment + "object \"" + name.str() + "\" {\n" + indent(inner) + "\n}";
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

void Object::visitPath(YulString _qualifiedName, std::function<bool(Object const*)> const& _visitor) const
{
	if (boost::algorithm::starts_with(_qualifiedName.str(), name.str() + "."))
		_qualifiedName = YulString{_qualifiedName.str().substr(name.str().length() + 1)};
	yulAssert(!_qualifiedName.empty(), "");

	std::vector<std::string> subObjectPathComponents;
	boost::algorithm::split(subObjectPathComponents, _qualifiedName.str(), boost::is_any_of("."));

	Object const* object = this;
	for (std::string const& currentSubObjectName: subObjectPathComponents)
	{
		yulAssert(!currentSubObjectName.empty(), "");

		YulString subObjectName = YulString{currentSubObjectName};
		object = object->at(subObjectName);
		if (object && _visitor(object))
			break;
	}
}

std::vector<size_t> Object::pathToSubObject(YulString _qualifiedName) const
{
	std::vector<size_t> path;
	yulAssert(_qualifiedName != name, "");
	yulAssert(subIndexByName.count(name) == 0, "");

	this->visitPath(_qualifiedName, [&](Object const* _object) -> bool {
		yulAssert(_object->subId != std::numeric_limits<size_t>::max(), "");
		path.push_back(_object->subId);
		return false;
	});
	return path;
}

Object const* Object::subObjectAt(YulString _qualifiedName)
{
	yulAssert(!_qualifiedName.empty(), "");

	// If there is no `.` in the given `_qualifiedName`, the target
	// object name is considered to be equal to the `_qualifiedName`, otherwise,
	// the target object name is the last element in the path given by `_qualifiedName`.
	YulString targetObjectName = _qualifiedName;
	size_t targetObjectPos = _qualifiedName.str().find_last_of(".");
	if (targetObjectPos != std::string::npos)
		targetObjectName = YulString(_qualifiedName.str().substr(targetObjectPos + 1));

	Object const* foundObject = nullptr;
	this->visitPath(_qualifiedName, [&](Object const* _subObject) -> bool {
		if (targetObjectName != _subObject->name)
			return false;
		foundObject = _subObject;
		return true;
	});

	return foundObject;
}
