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
/** @file JSON.cpp
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2018
 */

#include <libsolutil/JSON.h>

#include <libsolutil/CommonIO.h>

#include <boost/algorithm/string/replace.hpp>

#include <sstream>
#include <map>
#include <memory>

using namespace std;

static_assert(
	(NLOHMANN_JSON_VERSION_MAJOR == 3) && (NLOHMANN_JSON_VERSION_MINOR == 10) && (NLOHMANN_JSON_VERSION_PATCH == 2),
	"Unexpected nlohmann-json version. Expecting 3.10.2."
);

namespace solidity::util
{

namespace
{

#if 0
/// Takes a JSON value (@ _json) and removes all its members with value 'null' recursively.
void removeNullMembersHelper(Json& _json)
{
	if (_json.type() == Json::ValueType::arrayValue)
		for (auto& child: _json)
			removeNullMembersHelper(child);
	else if (_json.type() == Json::ValueType::objectValue)
		for (auto const& key: _json.getMemberNames())
		{
			Json::Value& value = _json[key];
			if (value.isNull())
				_json.removeMember(key);
			else
				removeNullMembersHelper(value);
		}
}
#endif

} // end anonymous namespace

Json removeNullMembers(Json _json)
{
	// TODO: Support this.
	// removeNullMembersHelper(_json);
	return _json;
}

string jsonPrettyPrint(Json const& _input)
{
	return jsonPrint(_input, JsonFormat{ JsonFormat::Pretty });
}

string jsonCompactPrint(Json const& _input)
{
	return jsonPrint(_input, JsonFormat{ JsonFormat::Compact });
}

string jsonPrint(Json const& _input, JsonFormat const& _format)
{
	// NOTE: -1 here means no new lines (it is also the default setting)
	return _input.dump(
		/* indent */ (_format.format == JsonFormat::Pretty) ? static_cast<int>(_format.indent) : -1,
		/* indent_char */ ' ',
		/* ensure_ascii */ true
	);
}

bool jsonParseStrict(string const& _input, Json& _json, string* _errs /* = nullptr */)
{
	try
	{
		_json = Json::parse(_input);
		_errs = {};
		return true;
	}
	catch (Json::parse_error const& e)
	{
		// NOTE: e.id() gives the code and e.byte() gives the byte offset
		if (_errs)
		{
			*_errs = e.what();
		}
		return false;
	}
}

} // namespace solidity::util
