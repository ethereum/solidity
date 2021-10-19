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
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <range/v3/view/transform.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

namespace
{

string indent(std::string const& _input)
{
	if (_input.empty())
		return _input;
	return boost::replace_all_copy("    " + _input, "\n", "\n    ");
}

}

string Data::toString(Dialect const*, DebugInfoSelection const&, CharStreamProvider const*) const
{
	return "data \"" + name.str() + "\" hex\"" + util::toHex(data) + "\"";
}

string Object::toString(
	Dialect const* _dialect,
	DebugInfoSelection const& _debugInfoSelection,
	CharStreamProvider const* _soliditySourceProvider
) const
{
	yulAssert(code, "No code");
	yulAssert(debugData, "No debug data");

	string useSrcComment;

	if (debugData->sourceNames)
		useSrcComment =
			"/// @use-src " +
			joinHumanReadable(ranges::views::transform(*debugData->sourceNames, [](auto&& _pair) {
				return to_string(_pair.first) + ":" + util::escapeAndQuoteString(*_pair.second);
			})) +
			"\n";

	string inner = "code " + AsmPrinter(
		_dialect,
		debugData->sourceNames,
		_debugInfoSelection,
		_soliditySourceProvider
	)(*code);

	for (auto const& obj: subObjects)
		inner += "\n" + obj->toString(_dialect, _debugInfoSelection, _soliditySourceProvider);

	return useSrcComment + "object \"" + name.str() + "\" {\n" + indent(inner) + "\n}";
}

set<YulString> Object::qualifiedDataNames() const
{
	set<YulString> qualifiedNames =
		name.empty() || contains(name.str(), '.') ?
		set<YulString>{} :
		set<YulString>{name};
	for (shared_ptr<ObjectNode> const& subObjectNode: subObjects)
	{
		yulAssert(qualifiedNames.count(subObjectNode->name) == 0, "");
		if (contains(subObjectNode->name.str(), '.'))
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

vector<size_t> Object::pathToSubObject(YulString _qualifiedName) const
{
	yulAssert(_qualifiedName != name, "");
	yulAssert(subIndexByName.count(name) == 0, "");

	if (boost::algorithm::starts_with(_qualifiedName.str(), name.str() + "."))
		_qualifiedName = YulString{_qualifiedName.str().substr(name.str().length() + 1)};
	yulAssert(!_qualifiedName.empty(), "");

	vector<string> subObjectPathComponents;
	boost::algorithm::split(subObjectPathComponents, _qualifiedName.str(), boost::is_any_of("."));

	vector<size_t> path;
	Object const* object = this;
	for (string const& currentSubObjectName: subObjectPathComponents)
	{
		yulAssert(!currentSubObjectName.empty(), "");
		auto subIndexIt = object->subIndexByName.find(YulString{currentSubObjectName});
		yulAssert(
			subIndexIt != object->subIndexByName.end(),
			"Assembly object <" + _qualifiedName.str() + "> not found or does not contain code."
		);
		object = dynamic_cast<Object const*>(object->subObjects[subIndexIt->second].get());
		yulAssert(object, "Assembly object <" + _qualifiedName.str() + "> not found or does not contain code.");
		yulAssert(object->subId != numeric_limits<size_t>::max(), "");
		path.push_back({object->subId});
	}

	return path;
}
