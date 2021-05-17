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

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <sstream>

using namespace std;
using namespace solidity::lsp;

JSONTransport::JSONTransport(istream& _in, ostream& _out):
	m_input{_in},
	m_output{_out}
{
}

JSONTransport::JSONTransport(): JSONTransport(cin, cout)
{
}

bool JSONTransport::closed() const noexcept
{
	return m_input.eof();
}

optional<Json::Value> JSONTransport::receive()
{
	auto const headers = parseHeaders();
	if (!headers)
		return nullopt;

	if (!headers->count("content-length"))
		return nullopt;

	string const data = readBytes(stoi(headers->at("content-length")));

	Json::Value jsonMessage;
	string errs;
	solidity::util::jsonParseStrict(data, jsonMessage, &errs);
	if (!errs.empty())
		return nullopt; // JsonParseError

	//traceMessage(jsonMessage, "Request");

	return {jsonMessage};
}

void JSONTransport::notify(string const& _method, Json::Value const& _message)
{
	Json::Value json;
	json["method"] = _method;
	json["params"] = _message;
	send(json);
}

void JSONTransport::reply(MessageID _id, Json::Value const& _message)
{
	Json::Value json;
	json["result"] = _message;
	send(json, _id);
}

void JSONTransport::error(MessageID _id, ErrorCode _code, string const& _message)
{
	Json::Value json;
	json["error"]["code"] = static_cast<int>(_code);
	json["error"]["message"] = _message;
	send(json, _id);
}

void JSONTransport::send(Json::Value _json, MessageID _id)
{
	_json["jsonrpc"] = "2.0";
	if (!_id.isNull())
		_json["id"] = _id;

	string const jsonString = solidity::util::jsonCompactPrint(_json);

	m_output << "Content-Length: " << jsonString.size() << "\r\n";
	m_output << "\r\n";
	m_output << jsonString;

	m_output.flush();
	//traceMessage(_json, "Response");
}

optional<map<string, string>> JSONTransport::parseHeaders()
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

		auto const name = boost::to_lower_copy(line.substr(0, delimiterPos));
		auto const value = boost::trim_copy(line.substr(delimiterPos + 1));
		headers[move(name)] = value;
	}
	return {move(headers)};
}

string JSONTransport::readBytes(int _n)
{
	if (_n < 0)
		return {};

	string data;
	data.resize(static_cast<string::size_type>(_n));
	m_input.read(data.data(), _n);
	return data;
}
