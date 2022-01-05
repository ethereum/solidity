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

	std::optional<langutil::SourceLocation> parsePosition(
		std::string const& _sourceUnitName,
		Json::Value const& _position
	) const;

	/// @returns the source location given a source unit name and an LSP Range object,
	/// or nullopt on failure.
	std::optional<langutil::SourceLocation> parseRange(
		std::string const& _sourceUnitName,
		Json::Value const& _range
	) const;

	langutil::CharStreamProvider const& charStreamProvider() const noexcept { return m_server.charStreamProvider(); };
	FileRepository const& fileRepository() const noexcept { return m_server.fileRepository(); };
	Transport& client() const noexcept { return m_server.client(); };

	LanguageServer& m_server;
};

}
