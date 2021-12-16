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
#pragma once

#include <json/value.h>

#include <functional>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace solidity::lsp
{

using MessageID = Json::Value;

enum class ErrorCode
{
	// Defined by JSON RPC
	ParseError = -32700,
	MethodNotFound = -32601,
	InvalidParams = -32602,
	InternalError = -32603,

	// Defined by the protocol.
	ServerNotInitialized = -32002,
	RequestFailed = -32803
};

/**
 * Transport layer API
 *
 * The transport layer API is abstracted to make LSP more testable as well as
 * this way it could be possible to support other transports (HTTP for example) easily.
 */
class Transport
{
public:
	virtual ~Transport() = default;

	virtual bool closed() const noexcept = 0;
	virtual std::optional<Json::Value> receive() = 0;
	virtual void notify(std::string _method, Json::Value _params) = 0;
	virtual void reply(MessageID _id, Json::Value _result) = 0;
	virtual void error(MessageID _id, ErrorCode _code, std::string _message) = 0;
};

/**
 * LSP Transport using JSON-RPC over iostreams.
 */
class IOStreamTransport: public Transport
{
public:
	/// Constructs a standard stream transport layer.
	///
	/// @param _in for example std::cin (stdin)
	/// @param _out for example std::cout (stdout)
	IOStreamTransport(std::istream& _in, std::ostream& _out);

	// Constructs a JSON transport using standard I/O streams.
	IOStreamTransport();

	bool closed() const noexcept override;
	std::optional<Json::Value> receive() override;
	void notify(std::string _method, Json::Value _params) override;
	void reply(MessageID _id, Json::Value _result) override;
	void error(MessageID _id, ErrorCode _code, std::string _message) override;

protected:
	/// Sends an arbitrary raw message to the client.
	///
	/// Used by the notify/reply/error function family.
	virtual void send(Json::Value _message, MessageID _id = Json::nullValue);

	/// Parses header section from the client including message-delimiting empty line.
	std::optional<std::map<std::string, std::string>> parseHeaders();

private:
	std::istream& m_input;
	std::ostream& m_output;
};

}
