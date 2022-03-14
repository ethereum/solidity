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
#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTUtils.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/ReadFile.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/lsp/LanguageServer.h>
#include <libsolidity/lsp/HandlerBase.h>
#include <libsolidity/lsp/Utils.h>

// LSP feature implementations
#include <libsolidity/lsp/GotoDefinition.h>

#include <liblangutil/SourceReferenceExtractor.h>
#include <liblangutil/CharStream.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/JSON.h>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <fmt/format.h>

#include <ostream>
#include <string>

using namespace std;
using namespace std::string_literals;
using namespace std::placeholders;

using namespace solidity::lsp;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace
{

int toDiagnosticSeverity(Error::Type _errorType)
{
	// 1=Error, 2=Warning, 3=Info, 4=Hint
	switch (Error::errorSeverity(_errorType))
	{
	case Error::Severity::Error: return 1;
	case Error::Severity::Warning: return 2;
	case Error::Severity::Info: return 3;
	}
	solAssert(false);
	return -1;
}

}

LanguageServer::LanguageServer(Transport& _transport):
	m_client{_transport},
	m_handlers{
		{"$/cancelRequest", [](auto, auto) {/*nothing for now as we are synchronous */}},
		{"cancelRequest", [](auto, auto) {/*nothing for now as we are synchronous */}},
		{"exit", [this](auto, auto) { m_state = (m_state == State::ShutdownRequested ? State::ExitRequested : State::ExitWithoutShutdown); }},
		{"initialize", bind(&LanguageServer::handleInitialize, this, _1, _2)},
		{"initialized", [](auto, auto) {}},
		{"shutdown", [this](auto, auto) { m_state = State::ShutdownRequested; }},
		{"textDocument/definition", GotoDefinition(*this) },
		{"textDocument/didOpen", bind(&LanguageServer::handleTextDocumentDidOpen, this, _2)},
		{"textDocument/didChange", bind(&LanguageServer::handleTextDocumentDidChange, this, _2)},
		{"textDocument/didClose", bind(&LanguageServer::handleTextDocumentDidClose, this, _2)},
		{"textDocument/implementation", GotoDefinition(*this) },
		{"workspace/didChangeConfiguration", bind(&LanguageServer::handleWorkspaceDidChangeConfiguration, this, _2)},
	},
	m_fileRepository("/" /* basePath */),
	m_compilerStack{m_fileRepository.reader()}
{
}

Json::Value LanguageServer::toRange(SourceLocation const& _location)
{
	return HandlerBase(*this).toRange(_location);
}

Json::Value LanguageServer::toJson(SourceLocation const& _location)
{
	return HandlerBase(*this).toJson(_location);
}

void LanguageServer::changeConfiguration(Json::Value const& _settings)
{
	m_settingsObject = _settings;
}

void LanguageServer::compile()
{
	// For files that are not open, we have to take changes on disk into account,
	// so we just remove all non-open files.

	FileRepository oldRepository(m_fileRepository.basePath());
	swap(oldRepository, m_fileRepository);

	for (string const& fileName: m_openFiles)
		m_fileRepository.setSourceByClientPath(
			fileName,
			oldRepository.sourceUnits().at(oldRepository.clientPathToSourceUnitName(fileName))
		);

	// TODO: optimize! do not recompile if nothing has changed (file(s) not flagged dirty).

	m_compilerStack.reset(false);
	m_compilerStack.setSources(m_fileRepository.sourceUnits());
	m_compilerStack.compile(CompilerStack::State::AnalysisPerformed);
}

void LanguageServer::compileAndUpdateDiagnostics()
{
	compile();

	// These are the source units we will sent diagnostics to the client for sure,
	// even if it is just to clear previous diagnostics.
	map<string, Json::Value> diagnosticsBySourceUnit;
	for (string const& sourceUnitName: m_fileRepository.sourceUnits() | ranges::views::keys)
		diagnosticsBySourceUnit[sourceUnitName] = Json::arrayValue;
	for (string const& sourceUnitName: m_nonemptyDiagnostics)
		diagnosticsBySourceUnit[sourceUnitName] = Json::arrayValue;

	for (shared_ptr<Error const> const& error: m_compilerStack.errors())
	{
		SourceLocation const* location = error->sourceLocation();
		if (!location || !location->sourceName)
			// LSP only has diagnostics applied to individual files.
			continue;

		Json::Value jsonDiag;
		jsonDiag["source"] = "solc";
		jsonDiag["severity"] = toDiagnosticSeverity(error->type());
		jsonDiag["code"] = Json::UInt64{error->errorId().error};
		string message = error->typeName() + ":";
		if (string const* comment = error->comment())
			message += " " + *comment;
		jsonDiag["message"] = move(message);
		jsonDiag["range"] = toRange(*location);

		if (auto const* secondary = error->secondarySourceLocation())
			for (auto&& [secondaryMessage, secondaryLocation]: secondary->infos)
			{
				Json::Value jsonRelated;
				jsonRelated["message"] = secondaryMessage;
				jsonRelated["location"] = toJson(secondaryLocation);
				jsonDiag["relatedInformation"].append(jsonRelated);
			}

		diagnosticsBySourceUnit[*location->sourceName].append(jsonDiag);
	}

	m_nonemptyDiagnostics.clear();
	for (auto&& [sourceUnitName, diagnostics]: diagnosticsBySourceUnit)
	{
		Json::Value params;
		params["uri"] = m_fileRepository.sourceUnitNameToClientPath(sourceUnitName);
		if (!diagnostics.empty())
			m_nonemptyDiagnostics.insert(sourceUnitName);
		params["diagnostics"] = move(diagnostics);
		m_client.notify("textDocument/publishDiagnostics", move(params));
	}
}

bool LanguageServer::run()
{
	while (m_state != State::ExitRequested && m_state != State::ExitWithoutShutdown && !m_client.closed())
	{
		MessageID id;
		try
		{
			optional<Json::Value> const jsonMessage = m_client.receive();
			if (!jsonMessage)
				continue;

			if ((*jsonMessage)["method"].isString())
			{
				string const methodName = (*jsonMessage)["method"].asString();
				id = (*jsonMessage)["id"];

				if (auto handler = util::valueOrDefault(m_handlers, methodName))
					handler(id, (*jsonMessage)["params"]);
				else
					m_client.error(id, ErrorCode::MethodNotFound, "Unknown method " + methodName);
			}
			else
				m_client.error({}, ErrorCode::ParseError, "\"method\" has to be a string.");
		}
		catch (RequestError const& error)
		{
			m_client.error(id, error.code(), error.comment() ? *error.comment() : ""s);
		}
		catch (...)
		{
			m_client.error(id, ErrorCode::InternalError, "Unhandled exception: "s + boost::current_exception_diagnostic_information());
		}
	}
	return m_state == State::ExitRequested;
}

void LanguageServer::requireServerInitialized()
{
	lspAssert(
		m_state == State::Initialized,
		ErrorCode::ServerNotInitialized,
		"Server is not properly initialized."
	);
}

void LanguageServer::handleInitialize(MessageID _id, Json::Value const& _args)
{
	lspAssert(
		m_state == State::Started,
		ErrorCode::RequestFailed,
		"Initialize called at the wrong time."
	);

	m_state = State::Initialized;

	// The default of FileReader is to use `.`, but the path from where the LSP was started
	// should not matter.
	string rootPath("/");
	if (Json::Value uri = _args["rootUri"])
	{
		rootPath = uri.asString();
		lspAssert(
			boost::starts_with(rootPath, "file://"),
			ErrorCode::InvalidParams,
			"rootUri only supports file URI scheme."
		);

		rootPath = rootPath.substr(7);
	}
	else if (Json::Value rootPath = _args["rootPath"])
		rootPath = rootPath.asString();

	m_fileRepository = FileRepository(boost::filesystem::path(rootPath));
	if (_args["initializationOptions"].isObject())
		changeConfiguration(_args["initializationOptions"]);

	Json::Value replyArgs;
	replyArgs["serverInfo"]["name"] = "solc";
	replyArgs["serverInfo"]["version"] = string(VersionNumber);
	replyArgs["capabilities"]["definitionProvider"] = true;
	replyArgs["capabilities"]["implementationProvider"] = true;
	replyArgs["capabilities"]["textDocumentSync"]["change"] = 2; // 0=none, 1=full, 2=incremental
	replyArgs["capabilities"]["textDocumentSync"]["openClose"] = true;

	m_client.reply(_id, move(replyArgs));
}

void LanguageServer::handleWorkspaceDidChangeConfiguration(Json::Value const& _args)
{
	requireServerInitialized();

	if (_args["settings"].isObject())
		changeConfiguration(_args["settings"]);
}

void LanguageServer::handleTextDocumentDidOpen(Json::Value const& _args)
{
	requireServerInitialized();

	lspAssert(
		_args["textDocument"],
		ErrorCode::RequestFailed,
		"Text document parameter missing."
	);

	string text = _args["textDocument"]["text"].asString();
	string uri = _args["textDocument"]["uri"].asString();
	m_openFiles.insert(uri);
	m_fileRepository.setSourceByClientPath(uri, move(text));
	compileAndUpdateDiagnostics();
}

void LanguageServer::handleTextDocumentDidChange(Json::Value const& _args)
{
	requireServerInitialized();

	string const uri = _args["textDocument"]["uri"].asString();

	for (Json::Value jsonContentChange: _args["contentChanges"])
	{
		lspAssert(
			jsonContentChange.isObject(),
			ErrorCode::RequestFailed,
			"Invalid content reference."
		);

		string const sourceUnitName = m_fileRepository.clientPathToSourceUnitName(uri);
		lspAssert(
			m_fileRepository.sourceUnits().count(sourceUnitName),
			ErrorCode::RequestFailed,
			"Unknown file: " + uri
		);

		string text = jsonContentChange["text"].asString();
		if (jsonContentChange["range"].isObject()) // otherwise full content update
		{
			optional<SourceLocation> change = parseRange(m_fileRepository, sourceUnitName, jsonContentChange["range"]);
			lspAssert(
				change && change->hasText(),
				ErrorCode::RequestFailed,
				"Invalid source range: " + util::jsonCompactPrint(jsonContentChange["range"])
			);

			string buffer = m_fileRepository.sourceUnits().at(sourceUnitName);
			buffer.replace(static_cast<size_t>(change->start), static_cast<size_t>(change->end - change->start), move(text));
			text = move(buffer);
		}
		m_fileRepository.setSourceByClientPath(uri, move(text));
	}

	compileAndUpdateDiagnostics();
}

void LanguageServer::handleTextDocumentDidClose(Json::Value const& _args)
{
	requireServerInitialized();

	lspAssert(
		_args["textDocument"],
		ErrorCode::RequestFailed,
		"Text document parameter missing."
	);

	string uri = _args["textDocument"]["uri"].asString();
	m_openFiles.erase(uri);

	compileAndUpdateDiagnostics();
}

ASTNode const* LanguageServer::astNodeAtSourceLocation(std::string const& _sourceUnitName, LineColumn const& _filePos)
{
	if (m_compilerStack.state() < CompilerStack::AnalysisPerformed)
		return nullptr;

	if (!m_fileRepository.sourceUnits().count(_sourceUnitName))
		return nullptr;

	if (optional<int> sourcePos =
		m_compilerStack.charStream(_sourceUnitName).translateLineColumnToPosition(_filePos))
		return locateInnermostASTNode(*sourcePos, m_compilerStack.ast(_sourceUnitName));
	else
		return nullptr;
}

