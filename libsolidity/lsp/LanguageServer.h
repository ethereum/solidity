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

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/FileReader.h>
#include <libsolidity/interface/FileRemapper.h>
#include <libsolidity/lsp/LSPTypes.h>
#include <libsolidity/lsp/ReferenceCollector.h>
#include <libsolidity/lsp/Transport.h>

#include <libsolidity/ast/AST.h>

#include <liblangutil/SourceReferenceExtractor.h>

#include <json/value.h>

#include <boost/filesystem/path.hpp>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace solidity::frontend {
	class Declaration;
}

namespace solidity::lsp {

enum class ErrorCode;

/// Solidity Language Server, managing one LSP client.
///
/// This implements a subset of LSP version 3.16 that can be found at:
///     https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/
class LanguageServer
{
public:
	using Logger = std::function<void(std::string_view)>;

	/// @param _logger special logger used for debugging the LSP.
	LanguageServer(Logger _logger, std::istream& _in, std::ostream& _out);
	explicit LanguageServer(Logger _logger): LanguageServer(std::move(_logger), std::cin, std::cout) {}

	/// Compiles the source behind path @p _file and updates the diagnostics pushed to the client.
	///
	/// update diagnostics and also pushes any updates to the client.
	void compileSource(std::string const& _file);

	/// Loops over incoming messages via the transport layer until shutdown condition is met.
	///
	/// The standard shutdown condition is when the maximum number of consecutive failures
	/// has been exceeded.
	///
	/// @return boolean indicating normal or abnormal termination.
	bool run();

	/// Handles a single JSON-RPC message in string form.
	void handleMessage(std::string const& _message);

	/// Handles a single JSON-RPC message.
	void handleMessage(Json::Value const& _jsonMessage);

protected:
	void handle_initialize(MessageId _id, Json::Value const& _args);
	void handle_exit(MessageId _id, Json::Value const& _args);
	void handle_workspace_didChangeConfiguration(MessageId _id, Json::Value const& _args);
	void handle_textDocument_didOpen(MessageId _id, Json::Value const& _args);
	void handle_textDocument_didChange(MessageId _id, Json::Value const& _args);
	void handle_textDocument_definition(MessageId _id, Json::Value const& _args);
	void handle_textDocument_hover(MessageId _id, Json::Value const& _args);
	void handle_textDocument_highlight(MessageId _id, Json::Value const& _args);
	void handle_textDocument_references(MessageId _id, Json::Value const& _args);

	// {{{ Client-to-Server messages
	/// Invoked when the server user-supplied configuration changes (initiated by the client).
	void changeConfiguration(Json::Value const&);
	void documentContentUpdated(std::string const& _path, std::string const& _fullContentChange);
	void documentContentUpdated(std::string const& _path, LineColumnRange _range, std::string const& _text);

	/// Find all semantically equivalent occurrences of the symbol the current cursor is located at.
	///
	/// @returns a list of ranges to highlight as well as their use kind (read fraom, written to, other text).
	std::vector<DocumentHighlight> semanticHighlight(DocumentPosition _documentPosition);

	/// Finds all references of the current symbol at the given document position.
	///
	/// @returns all references as document ranges as well as their use kind (read fraom, written to, other text).
	std::vector<langutil::SourceLocation> references(DocumentPosition _documentPosition);
	// }}}

	/// Logs a message (should be used for logging messages that are informationally useful to the client).
	void log(std::string const& _message);

	/// Logs a verbose trace message (should used for logging messages that are helpful to the client).
	void trace(std::string const& _message);

	frontend::ReadCallback::Result readFile(std::string const&, std::string const&);

	bool compile(std::string const& _path);

	frontend::ASTNode const* findASTNode(langutil::LineColumn _position, std::string const& _fileName);

	std::optional<langutil::SourceLocation> declarationPosition(frontend::Declaration const* _declaration);

	std::vector<langutil::SourceLocation> findAllReferences(
		frontend::Declaration const* _declaration,
		std::string const& _sourceIdentifierName,
		frontend::SourceUnit const& _sourceUnit
	);

	void findAllReferences(
		frontend::Declaration const* _declaration,
		std::string const& _sourceIdentifierName,
		frontend::SourceUnit const& _sourceUnit,
		std::vector<langutil::SourceLocation>& _output
	);

	// {{{ LSP related member fields
	using Handler = std::function<void(MessageId, Json::Value const&)>;
	using HandlerMap = std::unordered_map<std::string, Handler>;

	std::unique_ptr<JSONTransport> m_client;
	HandlerMap m_handlers;
	bool m_shutdownRequested = false;
	bool m_exitRequested = false;
	Trace m_trace = Trace::Off;
	std::function<void(std::string_view)> m_logger;
	// }}}

	/// FileReader is used for reading files during comilation phase but is also used as VFS for the LSP.
	std::unique_ptr<FileReader> m_fileReader;

	/// Workspace root directory
	boost::filesystem::path m_basePath;

	/// map of input files to source code strings
	///
	/// The key must be a fully qualified path to the file.
	// TODO (dead) std::map<std::string, std::string> m_sourceCodes;

	std::unique_ptr<frontend::CompilerStack> m_compilerStack;
	std::vector<FileRemapper::Remapping> m_remappings;

	/// Configured EVM version that is being used in compilations.
	langutil::EVMVersion m_evmVersion = langutil::EVMVersion::berlin();
};

} // namespace solidity

