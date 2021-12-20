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

private:
	/// Checks if the server is initialized (to be used by messages that need it to be initialized).
	/// Reports an error and returns false if not.
	bool checkServerInitialized(MessageID _id);
	void handleInitialize(MessageID _id, Json::Value const& _args);
	void handleWorkspaceDidChangeConfiguration(MessageID _id, Json::Value const& _args);
	void handleTextDocumentDidOpen(MessageID _id, Json::Value const& _args);
	void handleTextDocumentDidChange(MessageID _id, Json::Value const& _args);
	void handleTextDocumentDidClose(MessageID _id, Json::Value const& _args);

	/// Invoked when the server user-supplied configuration changes (initiated by the client).
	void changeConfiguration(Json::Value const&);

	/// Compile everything until after analysis phase.
	void compile();

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
	Json::Value toRange(langutil::SourceLocation const& _location) const;
	Json::Value toJson(langutil::SourceLocation const& _location) const;

	// LSP related member fields
	using MessageHandler = std::function<void(MessageID, Json::Value const&)>;

	enum class State { Started, Initialized, ShutdownRequested, ExitRequested, ExitWithoutShutdown };
	State m_state = State::Started;

	Transport& m_client;
	std::map<std::string, MessageHandler> m_handlers;

	/// Set of files known to be open by the client.
	std::set<std::string> m_openFiles;
	/// Set of source unit names for which we sent diagnostics to the client in the last iteration.
	std::set<std::string> m_nonemptyDiagnostics;
	FileRepository m_fileRepository;

	frontend::CompilerStack m_compilerStack;

	/// User-supplied custom configuration settings (such as EVM version).
	Json::Value m_settingsObject;
};

}
