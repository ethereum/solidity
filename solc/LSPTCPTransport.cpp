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
#include <solc/LSPTCPTransport.h>

#include <optional>
#include <string>
#include <iostream>

namespace solidity::lsp {

using std::function;
using std::nullopt;
using std::optional;
using std::string_view;
using std::to_string;

using namespace std::string_literals;

LSPTCPTransport::LSPTCPTransport(unsigned short _port, Trace _traceLevel, function<void(string_view)> _trace):
	m_io_service(),
	m_endpoint(boost::asio::ip::make_address("127.0.0.1"), _port),
	m_acceptor(m_io_service),
	m_stream(),
	m_jsonTransport(),
	m_traceLevel{_traceLevel},
	m_trace(_trace ? std::move(_trace) : [](string_view) {})
{
	m_acceptor.open(m_endpoint.protocol());
	m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	m_acceptor.bind(m_endpoint);
	m_acceptor.listen();

	if (m_traceLevel >= Trace::Messages)
		m_trace("Listening on tcp://127.0.0.1:"s + to_string(_port));
}

void LSPTCPTransport::setTraceLevel(Trace _traceLevel)
{
	m_traceLevel = _traceLevel;

	if (m_jsonTransport)
		m_jsonTransport->setTraceLevel(_traceLevel);
}

bool LSPTCPTransport::closed() const noexcept
{
	return !m_acceptor.is_open();
}

optional<Json::Value> LSPTCPTransport::receive()
{
	auto const clientClosed = [&]() { return !m_stream || !m_stream.value().good() || m_stream.value().eof(); };

	if (clientClosed())
	{
		m_stream.emplace(m_acceptor.accept());
		if (clientClosed())
			return nullopt;

		auto const remoteAddr = m_stream.value().socket().remote_endpoint().address().to_string();
		auto const remotePort = m_stream.value().socket().remote_endpoint().port();
		m_trace("New client connected from "s + remoteAddr + ":" + to_string(remotePort) + ".");
		m_jsonTransport.emplace(m_stream.value(), m_stream.value(), m_traceLevel, [this](auto msg) { m_trace(msg); });
	}
	if (auto value = m_jsonTransport.value().receive(); value.has_value())
		return value;

	if (clientClosed())
	{
		m_trace("Client disconnected.");
		m_jsonTransport.reset();
		m_stream.reset();
	}
	return nullopt;
}

void LSPTCPTransport::notify(std::string const& _method, Json::Value const& _params)
{
	if (m_jsonTransport.has_value())
		m_jsonTransport.value().notify(_method, _params);
}

void LSPTCPTransport::reply(MessageID _id, Json::Value const& _result)
{
	if (m_jsonTransport.has_value())
		m_jsonTransport.value().reply(_id, _result);
}

void LSPTCPTransport::error(MessageID _id, ErrorCode _code, std::string const& _message)
{
	if (m_jsonTransport.has_value())
		m_jsonTransport.value().error(_id, _code, _message);
}

}
