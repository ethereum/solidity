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

#include <libsolidity/lsp/LSPTypes.h>

#include <json/value.h>

#include <functional>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace solidity::lsp {

using MessageID = Json::Value;

enum class ErrorCode
{
	// Defined by JSON RPC
	ParseError = -32700,
	InvalidRequest = -32600,
	MethodNotFound = -32601,
	InvalidParams = -32602,
	InternalError = -32603,
	serverErrorStart = -32099,
	serverErrorEnd = -32000,
	ServerNotInitialized = -32002,
	UnknownErrorCode = -32001,

	// Defined by the protocol.
	RequestCancelled = -32800,
	ContentModified = -32801,
};

/// Transport layer API
///
/// The transport layer API is abstracted so it users become more testable as well as
/// this way it could be possible to support other transports (HTTP for example) easily.
class Transport
{
public:
	virtual ~Transport() = default;

	virtual void setTraceLevel(Trace /*_traceLevel*/) {}
	virtual bool closed() const noexcept = 0;
	virtual std::optional<Json::Value> receive() = 0;
	virtual void notify(std::string const& _method, Json::Value const& _params) = 0;
	virtual void reply(MessageID _id, Json::Value const& _result) = 0;
	virtual void error(MessageID _id, ErrorCode _code, std::string const& _message) = 0;
};

/// LSP Transport using JSON-RPC over iostreams.
class JSONTransport : public Transport
{
public:
	/// Constructs a standard stream transport layer.
	///
	/// @param _in for example std::cin (stdin)
	/// @param _out for example std::cout (stdout)
	/// @param _traceLevel sets the initial trace logging level
	/// @param _trace special logger used for debugging the LSP messages.
	JSONTransport(std::istream& _in, std::ostream& _out, Trace _traceLevel, std::function<void(std::string_view)> _trace);

	// Constructs a JSON transport using standard I/O streams.
	JSONTransport(Trace _traceLevel, std::function<void(std::string_view)> _trace);

	void setTraceLevel(Trace _traceLevel) override;
	bool closed() const noexcept override;
	std::optional<Json::Value> receive() override;
	void notify(std::string const& _method, Json::Value const& _params) override;
	void reply(MessageID _id, Json::Value const& _result) override;
	void error(MessageID _id, ErrorCode _code, std::string const& _message) override;

protected:
	using HeaderMap = std::map<std::string, std::string>;

	/// Reads given number of bytes from the client.
	virtual std::string readBytes(int _n);

	/// Sends an arbitrary raw message to the client.
	///
	/// Used by the notify/reply/error function family.
	virtual void send(Json::Value _message, MessageID _id = Json::nullValue);

	/// Parses a single text line from the client ending with CRLF (or just LF).
	std::string readLine();

	/// Parses header section from the client including message-delimiting empty line.
	std::optional<HeaderMap> parseHeaders();

	/// Appends given JSON message to the trace log.
	void traceMessage(Json::Value const& _message, std::string_view _title);

private:
	std::istream& m_input;
	std::ostream& m_output;
	Trace m_traceLevel;
	std::function<void(std::string_view)> m_trace;
};

} // end namespace
