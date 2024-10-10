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

#include <libsolutil/CommonData.h>

#include <boost/algorithm/string.hpp>

#include <sstream>

namespace solidity::util
{

namespace
{

/// Takes a JSON value (@ _json) and removes all its members with value 'null' recursively.
void removeNullMembersHelper(Json& _json)
{
	if (_json.is_array())
	{
		for (auto& child: _json)
			removeNullMembersHelper(child);
	}
	else if (_json.is_object())
	{
		for (auto it = _json.begin(); it != _json.end();)
		{
			if (it->is_null())
				it = _json.erase(it);
			else
			{
				removeNullMembersHelper(*it);
				++it;
			}
		}
	}
}

std::string escapeNewlinesAndTabsWithinStringLiterals(std::string const& _json)
{
	std::stringstream fixed;
	bool inQuotes = false;
	for (size_t i = 0; i < _json.size(); ++i)
	{
		char c = _json[i];

		// Originally we had just this here:
		// if (c == '"' && (i == 0 || _json[i - 1] != '\\'))
		//    inQuotes = !inQuotes;
		// However, this is not working if the escape character itself was escaped. e.g. "\n\r'\"\\".

		if (c == '"')
		{
			size_t backslashCount = 0;
			size_t j = i;
			while (j > 0 && _json[j - 1] == '\\')
			{
				backslashCount++;
				j--;
			}
			if (backslashCount % 2 == 0)
			{
				inQuotes = !inQuotes;
				fixed << c;
				continue;
			}
		}

		if (inQuotes)
		{
			if (c == '\n')
				fixed << "\\n";
			else if (c == '\t')
				fixed << "\\t";
			else
				fixed << c;
		}
		else
			fixed << c;
	}
	return fixed.str();
}

} // end anonymous namespace

Json removeNullMembers(Json _json)
{
	removeNullMembersHelper(_json);
	return _json;
}

std::string removeNlohmannInternalErrorIdentifier(std::string const& _input)
{
	std::string result = _input;
	std::size_t startPos = result.find('[');
	std::size_t endPos = result.find(']', startPos);

	if (startPos != std::string::npos && endPos != std::string::npos)
		result.erase(startPos, endPos - startPos + 1);

	return boost::trim_copy(result);
}

std::string jsonPrettyPrint(Json const& _input) { return jsonPrint(_input, JsonFormat{JsonFormat::Pretty}); }

std::string jsonCompactPrint(Json const& _input) { return jsonPrint(_input, JsonFormat{JsonFormat::Compact}); }

std::string jsonPrint(Json const& _input, JsonFormat const& _format)
{
	// NOTE: -1 here means no new lines (it is also the default setting)
	std::string dumped = _input.dump(
		/* indent */ (_format.format == JsonFormat::Pretty) ? static_cast<int>(_format.indent) : -1,
		/* indent_char */ ' ',
		/* ensure_ascii */ true
	);

	return dumped;
}

bool jsonParseStrict(std::string const& _input, Json& _json, std::string* _errs /* = nullptr */)
{
	try
	{
		_json = Json::parse(
			// TODO: remove this in the next breaking release?
			escapeNewlinesAndTabsWithinStringLiterals(_input),
			/* callback */ nullptr,
			/* allow exceptions */ true,
			/* ignore_comments */true
		);
		_errs = {};
		return true;
	}
	catch (Json::parse_error const& e)
	{
		if (_errs)
		{
			std::stringstream escaped;
			for (char c: removeNlohmannInternalErrorIdentifier(e.what()))
				if (std::isprint(c))
					escaped << c;
				else
					escaped << "\\x" + toHex(static_cast<uint8_t>(c));
			*_errs = escaped.str();
		}
		return false;
	}
}

std::optional<Json> jsonValueByPath(Json const& _node, std::string_view _jsonPath)
{
	if (!_node.is_object() || _jsonPath.empty())
		return {};

	std::string memberName = std::string(_jsonPath.substr(0, _jsonPath.find_first_of('.')));
	if (!_node.contains(memberName))
		return {};

	if (memberName == _jsonPath)
		return _node[memberName];

	return jsonValueByPath(_node[memberName], _jsonPath.substr(memberName.size() + 1));
}

} // namespace solidity::util
