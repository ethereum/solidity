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
	(JSONCPP_VERSION_MAJOR == 1) && (JSONCPP_VERSION_MINOR == 9) && (JSONCPP_VERSION_PATCH == 3),
	"Unexpected jsoncpp version: " JSONCPP_VERSION_STRING ". Expecting 1.9.3."
);

namespace solidity::util
{

namespace
{

/// StreamWriterBuilder that can be constructed with specific settings
class StreamWriterBuilder: public Json::StreamWriterBuilder
{
public:
	explicit StreamWriterBuilder(map<string, Json::Value> const& _settings)
	{
		for (auto const& iter: _settings)
			this->settings_[iter.first] = iter.second;
	}
};

/// CharReaderBuilder with strict-mode settings
class StrictModeCharReaderBuilder: public Json::CharReaderBuilder
{
public:
	StrictModeCharReaderBuilder()
	{
		Json::CharReaderBuilder::strictMode(&this->settings_);
	}
};

/// Serialise the JSON object (@a _input) with specific builder (@a _builder)
/// \param _input JSON input string
/// \param _builder StreamWriterBuilder that is used to create new Json::StreamWriter
/// \return serialized json object
string print(Json::Value const& _input, Json::StreamWriterBuilder const& _builder)
{
	stringstream stream;
	unique_ptr<Json::StreamWriter> writer(_builder.newStreamWriter());
	writer->write(_input, &stream);
	return stream.str();
}

/// Parse a JSON string (@a _input) with specified builder (@ _builder) and writes resulting JSON object to (@a _json)
/// \param _builder CharReaderBuilder that is used to create new Json::CharReaders
/// \param _input JSON input string
/// \param _json [out] resulting JSON object
/// \param _errs [out] Formatted error messages
/// \return \c true if the document was successfully parsed, \c false if an error occurred.
bool parse(Json::CharReaderBuilder& _builder, string const& _input, Json::Value& _json, string* _errs)
{
	unique_ptr<Json::CharReader> reader(_builder.newCharReader());
	return reader->parse(_input.c_str(), _input.c_str() + _input.length(), &_json, _errs);
}

/// Takes a JSON value (@ _json) and removes all its members with value 'null' recursively.
void removeNullMembersHelper(Json::Value& _json)
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

} // end anonymous namespace

Json::Value removeNullMembers(Json::Value _json)
{
	removeNullMembersHelper(_json);
	return _json;
}

string jsonPrettyPrint(Json::Value const& _input)
{
	return jsonPrint(_input, JsonFormat{ JsonFormat::Pretty });
}

string jsonCompactPrint(Json::Value const& _input)
{
	return jsonPrint(_input, JsonFormat{ JsonFormat::Compact });
}

string jsonPrint(Json::Value const& _input, JsonFormat const& _format)
{
	map<string, Json::Value> settings;
	if (_format.format == JsonFormat::Pretty)
	{
		settings["indentation"] = string(_format.indent, ' ');
		settings["enableYAMLCompatibility"] = true;
	}
	else
		settings["indentation"] = "";
	StreamWriterBuilder writerBuilder(settings);
	string result = print(_input, writerBuilder);
	if (_format.format == JsonFormat::Pretty)
		boost::replace_all(result, " \n", "\n");
	return result;
}

bool jsonParseStrict(string const& _input, Json::Value& _json, string* _errs /* = nullptr */)
{
	static StrictModeCharReaderBuilder readerBuilder;
	return parse(readerBuilder, _input, _json, _errs);
}

} // namespace solidity::util
