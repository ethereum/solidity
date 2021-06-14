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

#include <libsolidity/lsp/Transport.h>

#include <boost/asio.hpp>
#include <optional>

namespace solidity::lsp {

class LSPTCPTransport: public lsp::Transport {
public:
	LSPTCPTransport(unsigned short _port, Trace _traceLevel, std::function<void(std::string_view)>);

	void setTraceLevel(Trace _traceLevel) override;
	bool closed() const noexcept override;
	std::optional<Json::Value> receive() override;
	void notify(std::string const& _method, Json::Value const& _params) override;
	void reply(lsp::MessageID _id, Json::Value const& _result) override;
	void error(lsp::MessageID _id, lsp::ErrorCode _code, std::string const& _message) override;

private:
	boost::asio::io_service m_io_service;
	boost::asio::ip::tcp::endpoint m_endpoint;
	boost::asio::ip::tcp::acceptor m_acceptor;
	std::optional<boost::asio::ip::tcp::iostream> m_stream;
	std::optional<lsp::JSONTransport> m_jsonTransport;
	Trace m_traceLevel;
	std::function<void(std::string_view)> m_trace;
};

} // end namespace
