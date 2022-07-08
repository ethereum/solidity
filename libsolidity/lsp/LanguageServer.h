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
#include <libsolidity/lsp/FileRepository.h>
#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/FileReader.h>

#include <json/value.h>

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace solidity::lsp
{

class RenameSymbol;
enum class ErrorCode;

/**
 * Solidity Language Server, managing one LSP client.
 * This implements a subset of LSP version 3.16 that can be found at:
 * https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/
 */
class LanguageServer
{
public:
	/// @param _transport Customizable transport layer.
	explicit LanguageServer(Transport& _transport);

	/// Re-compiles the project and updates the diagnostics pushed to the client.
	void compileAndUpdateDiagnostics();

	/// Loops over incoming messages via the transport layer until shutdown condition is met.
	///
	/// The standard shutdown condition is when the maximum number of consecutive failures
	/// has been exceeded.
	///
	/// @return boolean indicating normal or abnormal termination.
	bool run();

	FileRepository& fileRepository() noexcept { return m_fileRepository; }
	Transport& client() noexcept { return m_client; }
	frontend::ASTNode const* astNodeAtSourceLocation(std::string const& _sourceUnitName, langutil::LineColumn const& _filePos);
	frontend::CompilerStack const& compilerStack() const noexcept { return m_compilerStack; }

private:
	/// Checks if the server is initialized (to be used by messages that need it to be initialized).
	/// Reports an error and returns false if not.
	void requireServerInitialized();
	void handleInitialize(MessageID _id, Json::Value const& _args);
	void handleWorkspaceDidChangeConfiguration(Json::Value const& _args);
	void setTrace(Json::Value const& _args);
	void handleTextDocumentDidOpen(Json::Value const& _args);
	void handleTextDocumentDidChange(Json::Value const& _args);
	void handleTextDocumentDidClose(Json::Value const& _args);
	void handleRename(Json::Value const& _args);
	void handleGotoDefinition(MessageID _id, Json::Value const& _args);
	void semanticTokensFull(MessageID _id, Json::Value const& _args);

	/// Invoked when the server user-supplied configuration changes (initiated by the client).
	void changeConfiguration(Json::Value const&);

	/// Compile everything until after analysis phase.
	void compile();
	using MessageHandler = std::function<void(MessageID, Json::Value const&)>;

	Json::Value toRange(langutil::SourceLocation const& _location);
	Json::Value toJson(langutil::SourceLocation const& _location);

	// LSP related member fields

	enum class State { Started, Initialized, ShutdownRequested, ExitRequested, ExitWithoutShutdown };
	State m_state = State::Started;

	Transport& m_client;
	std::map<std::string, MessageHandler> m_handlers;

	/// Set of files (names in URI form) known to be open by the client.
	std::set<std::string> m_openFiles;
	/// Set of source unit names for which we sent diagnostics to the client in the last iteration.
	std::set<std::string> m_nonemptyDiagnostics;
	FileRepository m_fileRepository;

	frontend::CompilerStack m_compilerStack;

	/// User-supplied custom configuration settings (such as EVM version).
	Json::Value m_settingsObject;
};

}
