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

#include <libsolutil/Exceptions.h>

#include <json/value.h>

#include <boost/filesystem/path.hpp>

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

enum class TraceValue
{
	Off,
	Messages,
	Verbose
};

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
 * Error exception used to bail out on errors in the LSP function-call handlers.
 */
class RequestError: public util::Exception
{
public:
	explicit RequestError(ErrorCode _code):
		m_code{_code}
	{
	}

	ErrorCode code() const noexcept { return m_code; }

private:
	ErrorCode m_code;
};

#define lspAssert(condition, errorCode, errorMessage) \
	if (!(condition)) \
	{ \
		BOOST_THROW_EXCEPTION( \
			RequestError(errorCode) << \
			util::errinfo_comment(errorMessage) \
		); \
	}

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

	std::optional<Json::Value> receive();
	void notify(std::string _method, Json::Value _params);
	void reply(MessageID _id, Json::Value _result);
	void error(MessageID _id, ErrorCode _code, std::string _message);

	virtual bool closed() const noexcept = 0;

	void trace(std::string _message, Json::Value _extra = Json::nullValue);

	TraceValue traceValue() const noexcept { return m_logTrace; }
	void setTrace(TraceValue _value) noexcept { m_logTrace = _value; }

	/// Sets path to a local log file (in addition to reporting the trace log to the client)
	/// to be written to if trace value is set to verbose.
	void setTraceLogFile(std::optional<boost::filesystem::path> _pathToLogfile);

private:
	TraceValue m_logTrace = TraceValue::Off;
	std::optional<boost::filesystem::path> m_traceLogFilePath;

protected:
	/// Reads from the transport and parses the headers until the beginning
	/// of the contents.
	std::optional<std::map<std::string, std::string>> parseHeaders();

	/// Consumes exactly @p _byteCount bytes, as needed for consuming
	/// the message body from the transport line.
	virtual std::string readBytes(size_t _byteCount) = 0;

	// Mimmicks std::getline() on this Transport API.
	virtual std::string getline() = 0;

	/// Writes the given payload @p _data to transport.
	/// This call may or may not buffer.
	virtual void writeBytes(std::string_view _data) = 0;

	/// Ensures transport output is flushed.
	virtual void flushOutput() = 0;

	/// Sends an arbitrary raw message to the client.
	///
	/// Used by the notify/reply/error function family.
	virtual void send(Json::Value _message, MessageID _id = Json::nullValue);
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

	bool closed() const noexcept override;

protected:
	std::string readBytes(size_t _byteCount) override;
	std::string getline() override;
	void writeBytes(std::string_view _data) override;
	void flushOutput() override;

private:
	std::istream& m_input;
	std::ostream& m_output;
};

/**
 * Standard I/O transport Layer utilizing stdin/stdout for communication.
 */
class StdioTransport: public Transport
{
public:
	StdioTransport();

	bool closed() const noexcept override;

protected:
	std::string readBytes(size_t _byteCount) override;
	std::string getline() override;
	void writeBytes(std::string_view _data) override;
	void flushOutput() override;
};

}
