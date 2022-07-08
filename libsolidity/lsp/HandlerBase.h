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

#include <libsolidity/lsp/FileRepository.h>
#include <libsolidity/lsp/LanguageServer.h>

#include <liblangutil/SourceLocation.h>
#include <liblangutil/CharStreamProvider.h>

#include <optional>

namespace solidity::lsp
{

class Transport;

/**
 * Helper base class for implementing handlers.
 */
class HandlerBase
{
public:
	explicit HandlerBase(LanguageServer& _server): m_server{_server} {}

	Json::Value toRange(langutil::SourceLocation const& _location) const;
	Json::Value toJson(langutil::SourceLocation const& _location) const;

	/// @returns source unit name and the line column position as extracted
	/// from the JSON-RPC parameters.
	std::pair<std::string, langutil::LineColumn> extractSourceUnitNameAndLineColumn(Json::Value const& _params) const;

	langutil::CharStreamProvider const& charStreamProvider() const noexcept { return m_server.compilerStack(); }
	FileRepository& fileRepository() const noexcept { return m_server.fileRepository(); }
	Transport& client() const noexcept { return m_server.client(); }

protected:
	LanguageServer& m_server;
};

}
