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
#include <libsolidity/lsp/Transport.h>

#include <libsolutil/JSON.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/CommonIO.h>
#include <liblangutil/Exceptions.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <sstream>

using namespace std;
using namespace solidity::lsp;

namespace
{
template <typename T>
optional<T> popFromFront(std::list<T>& _queue)
{
	if (_queue.empty())
		return nullopt;
	Json::Value message = _queue.front();
	_queue.pop_front();
	return message;
}
}

bool MockTransport::closed() const noexcept
{
	return m_closed;
}

void MockTransport::appendInput(Json::Value _message)
{
	solAssert(!m_closed, "");
	m_input.emplace_back(move(_message));
}

optional<Json::Value> MockTransport::receive()
{
	return popFromFront(m_input);
}

optional<Json::Value> MockTransport::popOutput()
{
	return popFromFront(m_output);
}

void MockTransport::notify(string _method, Json::Value _message)
{
	Json::Value json;
	json["method"] = move(_method);
	json["params"] = move(_message);
	send(move(json));
}

void MockTransport::reply(MessageID _id, Json::Value _message)
{
	Json::Value json;
	json["result"] = move(_message);
	send(move(json), _id);
}

void MockTransport::error(MessageID _id, ErrorCode _code, string _message)
{
	Json::Value json;
	json["error"]["code"] = static_cast<int>(_code);
	json["error"]["message"] = move(_message);
	send(move(json), _id);
}

void MockTransport::send(Json::Value _json, MessageID _id)
{
	_json["jsonrpc"] = "2.0";
	if (_id != Json::nullValue)
		_json["id"] = _id;

	m_output.push_back(_json);
}

IOStreamTransport::IOStreamTransport(istream& _in, ostream& _out):
	m_input{_in},
	m_output{_out}
{
}

IOStreamTransport::IOStreamTransport():
	IOStreamTransport(cin, cout)
{
}

bool IOStreamTransport::closed() const noexcept
{
	return m_input.eof();
}

optional<Json::Value> IOStreamTransport::receive()
{
	auto const headers = parseHeaders();
	if (!headers)
	{
		error({}, ErrorCode::ParseError, "Could not parse RPC headers.");
		return nullopt;
	}

	if (!headers->count("content-length"))
	{
		error({}, ErrorCode::ParseError, "No content-length header found.");
		return nullopt;
	}

	string const data = util::readBytes(m_input, stoul(headers->at("content-length")));

	Json::Value jsonMessage;
	string jsonParsingErrors;
	solidity::util::jsonParseStrict(data, jsonMessage, &jsonParsingErrors);
	if (!jsonParsingErrors.empty() || !jsonMessage || !jsonMessage.isObject())
	{
		error({}, ErrorCode::ParseError, "Could not parse RPC JSON payload. " + jsonParsingErrors);
		return nullopt;
	}

	return {move(jsonMessage)};
}

void IOStreamTransport::notify(string _method, Json::Value _message)
{
	Json::Value json;
	json["method"] = move(_method);
	json["params"] = move(_message);
	send(move(json));
}

void IOStreamTransport::reply(MessageID _id, Json::Value _message)
{
	Json::Value json;
	json["result"] = move(_message);
	send(move(json), _id);
}

void IOStreamTransport::error(MessageID _id, ErrorCode _code, string _message)
{
	Json::Value json;
	json["error"]["code"] = static_cast<int>(_code);
	json["error"]["message"] = move(_message);
	send(move(json), _id);
}

void IOStreamTransport::send(Json::Value _json, MessageID _id)
{
	solAssert(_json.isObject());
	_json["jsonrpc"] = "2.0";
	if (_id != Json::nullValue)
		_json["id"] = _id;

	string const jsonString = solidity::util::jsonCompactPrint(_json);

	m_output << "Content-Length: " << jsonString.size() << "\r\n";
	m_output << "\r\n";
	m_output << jsonString;

	m_output.flush();
}

optional<map<string, string>> IOStreamTransport::parseHeaders()
{
	map<string, string> headers;

	while (true)
	{
		string line;
		getline(m_input, line);
		if (boost::trim_copy(line).empty())
			break;

		auto const delimiterPos = line.find(':');
		if (delimiterPos == string::npos)
			return nullopt;

		string name = boost::to_lower_copy(line.substr(0, delimiterPos));
		string value = line.substr(delimiterPos + 1);
		if (!headers.emplace(
			boost::trim_copy(name),
			boost::trim_copy(value)
		).second)
			return nullopt;
	}
	return {move(headers)};
}
