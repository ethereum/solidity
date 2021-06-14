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

#include <libsolidity/ast/AST.h>
#include <libsolidity/formal/ModelCheckerSettings.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/FileReader.h>
#include <libsolidity/interface/ImportRemapper.h>
#include <libsolidity/lsp/LSPTypes.h>
#include <libsolidity/lsp/ReferenceCollector.h>
#include <libsolidity/lsp/Transport.h>

#include <liblangutil/SourceReferenceExtractor.h>

#include <fmt/format.h>

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

namespace solidity::lsp
{

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
	/// @param _transport Customizable transport layer.
	LanguageServer(Logger _logger, std::unique_ptr<Transport> _transport);

	/// Compiles the source behind path @p _file and updates the diagnostics pushed to the client.
	///
	/// update diagnostics and also pushes any updates to the client.
	void compileSourceAndReport(std::string const& _file);

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
	void handleInitialize(MessageID _id, Json::Value const& _args);
	void handleExit(MessageID _id, Json::Value const& _args);
	void handleWorkspaceDidChangeConfiguration(MessageID _id, Json::Value const& _args);
	void handleTextDocumentDidOpen(MessageID _id, Json::Value const& _args);
	void handleTextDocumentDidChange(MessageID _id, Json::Value const& _args);
	void handleTextDocumentHover(MessageID _id, Json::Value const& _args);
	void handleTextDocumentHighlight(MessageID _id, Json::Value const& _args);
	void handleTextDocumentReferences(MessageID _id, Json::Value const& _args);
	void handleGotoDefinition(MessageID _id, Json::Value const& _args);

	/**
	 * Constructs some tooltip (hover) text.
	 *
	 * The resulting text string should be in markdown format.
	 */
	std::string symbolHoverInformation(frontend::ASTNode const* _node);

	// {{{ Client-to-Server messages
	/// Invoked when the server user-supplied configuration changes (initiated by the client).
	void changeConfiguration(Json::Value const&);

	/// Find all semantically equivalent occurrences of the symbol the current cursor is located at.
	///
	/// @returns a list of ranges to highlight as well as their use kind (read fraom, written to, other text).
	std::vector<DocumentHighlight> semanticHighlight(frontend::ASTNode const* _node, std::string const& _path);

	/// Finds all references of the current symbol at the given document position.
	///
	/// @returns all references as document ranges as well as their use kind (read fraom, written to, other text).
	std::vector<langutil::SourceLocation> references(DocumentPosition _documentPosition);
	// }}}

	/// Logs a message (should be used for logging messages that are informationally useful to the client).
	void log(std::string _message);

	template <typename... Args>
	void log(std::string_view _msg, Args... _args)
	{
		log(fmt::format(_msg, std::forward<Args>(_args)...));
	}

	/// Logs a verbose trace message (should used for logging messages that are helpful to the client).
	void trace(std::string const& _message);

	template <typename... Args>
	void trace(std::string_view _msg, Args... _args)
	{
		trace(fmt::format(_msg, std::forward<Args>(_args)...));
	}

	void logNotImplemented(std::string_view _message);

	bool compile(std::string const& _path);

	frontend::ASTNode const* requestASTNode(DocumentPosition _filePos);

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

	DocumentPosition extractDocumentPosition(Json::Value const& _json) const;

	// {{{ LSP related member fields
	using Handler = std::function<void(MessageID, Json::Value const&)>;
	using HandlerMap = std::unordered_map<std::string, Handler>;

	std::unique_ptr<Transport> m_client;
	HandlerMap m_handlers;
	bool m_shutdownRequested = false;
	bool m_exitRequested = false;
	Trace m_trace = Trace::Off;
	std::function<void(std::string_view)> m_logger;
	// }}}

	/// FileReader is used for reading files during comilation phase but is also used as VFS for the LSP.
	std::unique_ptr<frontend::FileReader> m_fileReader;

	/// Workspace root directory
	boost::filesystem::path m_basePath;

	std::unique_ptr<frontend::CompilerStack> m_compilerStack;
	Json::Value m_settingsObject;
	frontend::StandardCompiler::InputsAndSettings m_inputsAndSettings;
};

} // namespace solidity

