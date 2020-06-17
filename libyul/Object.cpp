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
 * Yul code and data object container.
 */

#include <libyul/Object.h>

#include <libyul/AsmPrinter.h>
#include <libyul/Exceptions.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/CommonData.h>

#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

namespace
{

string indent(std::string const& _input)
{
	if (_input.empty())
		return _input;
	return boost::replace_all_copy("    " + _input, "\n", "\n    ");
}

}

string Data::toString(Dialect const*) const
{
	return "data \"" + name.str() + "\" hex\"" + util::toHex(data) + "\"";
}

string Object::toString(Dialect const* _dialect) const
{
	yulAssert(code, "No code");
	string inner = "code " + (_dialect ? AsmPrinter{*_dialect} : AsmPrinter{})(*code);

	for (auto const& obj: subObjects)
		inner += "\n" + obj->toString(_dialect);

	return "object \"" + name.str() + "\" {\n" + indent(inner) + "\n}";
}

set<YulString> Object::qualifiedDataNames() const
{
	set<YulString> qualifiedNames = {name};
	for (shared_ptr<ObjectNode> const& subObjectNode: subObjects)
	{
		qualifiedNames.insert(subObjectNode->name);
		if (auto const* subObject = dynamic_cast<Object const*>(subObjectNode.get()))
			for (YulString const& subSubObj: subObject->qualifiedDataNames())
				qualifiedNames.insert(YulString{subObject->name.str() + "." + subSubObj.str()});
	}

	// The empty name is not valid
	qualifiedNames.erase(YulString{});

	return qualifiedNames;
}

tuple<bool, vector<size_t>> Object::pathToSubObject(string _qualifiedName) const
{
	if (_qualifiedName == name.str())
		return {};
	if (_qualifiedName.rfind(name.str() + ".", 0) == 0)
		_qualifiedName.erase(0, name.str().length() + 1);

	vector<size_t> path;

	Object const* currentObject = this;
	while (!_qualifiedName.empty())
	{
		size_t dotPos = _qualifiedName.find('.');
		auto subIndexIt = currentObject->subIndexByName.find(YulString{_qualifiedName.substr(0, dotPos)});
		if (subIndexIt == currentObject->subIndexByName.end())
			return {};
		path.push_back({subIndexIt->second});
		currentObject = dynamic_cast<Object const*>(currentObject->subObjects[subIndexIt->second].get());
		_qualifiedName.erase(0, dotPos + (dotPos == string::npos ? 0 : 1));
		if (!currentObject)
		{
			yulAssert(_qualifiedName.empty(), "");
			return {true, {}};
		}
	}

	return {false, path};
}
