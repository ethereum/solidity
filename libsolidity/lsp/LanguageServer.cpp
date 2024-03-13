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
#include <libsolidity/lsp/DocumentHoverHandler.h>
#include <libsolidity/lsp/GotoDefinition.h>
#include <libsolidity/lsp/RenameSymbol.h>
#include <libsolidity/lsp/SemanticTokensBuilder.h>

#include <liblangutil/SourceReferenceExtractor.h>
#include <liblangutil/CharStream.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/JSON.h>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <ostream>
#include <string>

#include <fmt/format.h>

using namespace std::string_literals;
using namespace std::placeholders;

using namespace solidity::lsp;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity;

namespace fs = boost::filesystem;

namespace
{

bool resolvesToRegularFile(boost::filesystem::path _path, int maxRecursionDepth = 10)
{
	fs::file_status fileStatus = fs::status(_path);

	while (fileStatus.type() == fs::file_type::symlink_file && maxRecursionDepth > 0)
	{
		_path = boost::filesystem::read_symlink(_path);
		fileStatus = fs::status(_path);
		maxRecursionDepth--;
	}

	return fileStatus.type() == fs::file_type::regular_file;
}

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

Json semanticTokensLegend()
{
	Json legend;

	// NOTE! The (alphabetical) order and items must match exactly the items of
	//       their respective enum class members.

	Json tokenTypes = Json::array();
	tokenTypes.emplace_back("class");
	tokenTypes.emplace_back("comment");
	tokenTypes.emplace_back("enum");
	tokenTypes.emplace_back("enumMember");
	tokenTypes.emplace_back("event");
	tokenTypes.emplace_back("function");
	tokenTypes.emplace_back("interface");
	tokenTypes.emplace_back("keyword");
	tokenTypes.emplace_back("macro");
	tokenTypes.emplace_back("method");
	tokenTypes.emplace_back("modifier");
	tokenTypes.emplace_back("number");
	tokenTypes.emplace_back("operator");
	tokenTypes.emplace_back("parameter");
	tokenTypes.emplace_back("property");
	tokenTypes.emplace_back("std::string");
	tokenTypes.emplace_back("struct");
	tokenTypes.emplace_back("type");
	tokenTypes.emplace_back("typeParameter");
	tokenTypes.emplace_back("variable");
	legend["tokenTypes"] = tokenTypes;

	Json tokenModifiers = Json::array();
	tokenModifiers.emplace_back("abstract");
	tokenModifiers.emplace_back("declaration");
	tokenModifiers.emplace_back("definition");
	tokenModifiers.emplace_back("deprecated");
	tokenModifiers.emplace_back("documentation");
	tokenModifiers.emplace_back("modification");
	tokenModifiers.emplace_back("readonly");
	legend["tokenModifiers"] = tokenModifiers;

	return legend;
}

}

LanguageServer::LanguageServer(Transport& _transport):
	m_client{_transport},
	m_handlers{
		{"$/cancelRequest", [](auto, auto) {/*nothing for now as we are synchronous */}},
		{"cancelRequest", [](auto, auto) {/*nothing for now as we are synchronous */}},
		{"exit", [this](auto, auto) { m_state = (m_state == State::ShutdownRequested ? State::ExitRequested : State::ExitWithoutShutdown); }},
		{"initialize", std::bind(&LanguageServer::handleInitialize, this, _1, _2)},
		{"initialized", std::bind(&LanguageServer::handleInitialized, this, _1, _2)},
		{"$/setTrace", [this](auto, Json const& args) { setTrace(args["value"]); }},
		{"shutdown", [this](auto, auto) { m_state = State::ShutdownRequested; }},
		{"textDocument/definition", GotoDefinition(*this) },
		{"textDocument/didOpen", std::bind(&LanguageServer::handleTextDocumentDidOpen, this, _2)},
		{"textDocument/didChange", std::bind(&LanguageServer::handleTextDocumentDidChange, this, _2)},
		{"textDocument/didClose", std::bind(&LanguageServer::handleTextDocumentDidClose, this, _2)},
		{"textDocument/hover", DocumentHoverHandler(*this) },
		{"textDocument/rename", RenameSymbol(*this) },
		{"textDocument/implementation", GotoDefinition(*this) },
		{"textDocument/semanticTokens/full", std::bind(&LanguageServer::semanticTokensFull, this, _1, _2)},
		{"workspace/didChangeConfiguration", std::bind(&LanguageServer::handleWorkspaceDidChangeConfiguration, this, _2)},
	},
	m_fileRepository("/" /* basePath */, {} /* no search paths */),
	m_compilerStack{m_fileRepository.reader()}
{
}

Json LanguageServer::toRange(SourceLocation const& _location)
{
	return HandlerBase(*this).toRange(_location);
}

Json LanguageServer::toJson(SourceLocation const& _location)
{
	return HandlerBase(*this).toJson(_location);
}

void LanguageServer::changeConfiguration(Json const& _settings)
{
	// The settings item: "file-load-strategy" (enum) defaults to "project-directory" if not (or not correctly) set.
	// It can be overridden during client's handshake or at runtime, as usual.
	//
	// If this value is set to "project-directory" (default), all .sol files located inside the project directory or reachable through symbolic links will be subject to operations.
	//
	// Operations include compiler analysis, but also finding all symbolic references or symbolic renaming.
	//
	// If this value is set to "directly-opened-and-on-import", then only currently directly opened files and
	// those files being imported directly or indirectly will be included in operations.
	if (_settings.contains("file-load-strategy"))
	{
		auto const text = _settings["file-load-strategy"].get<std::string>();
		if (text == "project-directory")
			m_fileLoadStrategy = FileLoadStrategy::ProjectDirectory;
		else if (text == "directly-opened-and-on-import")
			m_fileLoadStrategy = FileLoadStrategy::DirectlyOpenedAndOnImported;
		else
			lspRequire(false, ErrorCode::InvalidParams, "Invalid file load strategy: " + text);
	}

	m_settingsObject = _settings;
	Json jsonIncludePaths = _settings.contains("include-paths") ? _settings["include-paths"] : Json::object();

	if (!jsonIncludePaths.empty())
	{
		int typeFailureCount = 0;
		if (jsonIncludePaths.is_array())
		{
			std::vector<boost::filesystem::path> includePaths;
			for (Json const& jsonPath: jsonIncludePaths)
			{
				if (jsonPath.is_string())
					includePaths.emplace_back(jsonPath.get<std::string>());
				else
					typeFailureCount++;
			}
			m_fileRepository.setIncludePaths(std::move(includePaths));
		}
		else
			++typeFailureCount;

		if (typeFailureCount)
			m_client.trace("Invalid JSON configuration passed. \"include-paths\" must be an array of strings.");
	}
}

std::vector<boost::filesystem::path> LanguageServer::allSolidityFilesFromProject() const
{
	std::vector<fs::path> collectedPaths{};

	// We explicitly decided against including all files from include paths but leave the possibility
	// open for a future PR to enable such a feature to be optionally enabled (default disabled).
	// Note: Newer versions of boost have deprecated symlink_option::recurse
#if (BOOST_VERSION < 107200)
	auto directoryIterator = fs::recursive_directory_iterator(m_fileRepository.basePath(), fs::symlink_option::recurse);
#else
	auto directoryIterator = fs::recursive_directory_iterator(m_fileRepository.basePath(), fs::directory_options::follow_directory_symlink);
#endif
	for (fs::directory_entry const& dirEntry: directoryIterator)
		if (
			dirEntry.path().extension() == ".sol" &&
			(dirEntry.status().type() == fs::file_type::regular_file || resolvesToRegularFile(dirEntry.path()))
		)
			collectedPaths.push_back(dirEntry.path());

	return collectedPaths;
}

void LanguageServer::compile()
{
	// For files that are not open, we have to take changes on disk into account,
	// so we just remove all non-open files.

	FileRepository oldRepository(m_fileRepository.basePath(), m_fileRepository.includePaths());
	std::swap(oldRepository, m_fileRepository);

	// Load all solidity files from project.
	if (m_fileLoadStrategy == FileLoadStrategy::ProjectDirectory)
		for (auto const& projectFile: allSolidityFilesFromProject())
		{
			lspDebug(fmt::format("adding project file: {}", projectFile.generic_string()));
			m_fileRepository.setSourceByUri(
				m_fileRepository.sourceUnitNameToUri(projectFile.generic_string()),
				util::readFileAsString(projectFile)
			);
		}

	// Overwrite all files as opened by the client, including the ones which might potentially have changes.
	for (std::string const& fileName: m_openFiles)
		m_fileRepository.setSourceByUri(
			fileName,
			oldRepository.sourceUnits().at(oldRepository.uriToSourceUnitName(fileName))
		);

	// TODO: optimize! do not recompile if nothing has changed (file(s) not flagged dirty).

	m_compilerStack.reset(false);
	m_compilerStack.setSources(m_fileRepository.sourceUnits());
	m_compilerStack.compile(CompilerStack::State::AnalysisSuccessful);
}

void LanguageServer::compileAndUpdateDiagnostics()
{
	compile();

	// These are the source units we will sent diagnostics to the client for sure,
	// even if it is just to clear previous diagnostics.
	std::map<std::string, Json> diagnosticsBySourceUnit;
	for (std::string const& sourceUnitName: m_fileRepository.sourceUnits() | ranges::views::keys)
		diagnosticsBySourceUnit[sourceUnitName] = Json::array();
	for (std::string const& sourceUnitName: m_nonemptyDiagnostics)
		diagnosticsBySourceUnit[sourceUnitName] = Json::array();

	for (std::shared_ptr<Error const> const& error: m_compilerStack.errors())
	{
		SourceLocation const* location = error->sourceLocation();
		if (!location || !location->sourceName)
			// LSP only has diagnostics applied to individual files.
			continue;

		Json jsonDiag;
		jsonDiag["source"] = "solc";
		jsonDiag["severity"] = toDiagnosticSeverity(error->type());
		jsonDiag["code"] = Json(error->errorId().error);
		std::string message = Error::formatErrorType(error->type()) + ":";
		if (std::string const* comment = error->comment())
			message += " " + *comment;
		jsonDiag["message"] = std::move(message);
		jsonDiag["range"] = toRange(*location);

		if (auto const* secondary = error->secondarySourceLocation())
			for (auto&& [secondaryMessage, secondaryLocation]: secondary->infos)
			{
				Json jsonRelated;
				jsonRelated["message"] = secondaryMessage;
				jsonRelated["location"] = toJson(secondaryLocation);
				jsonDiag["relatedInformation"].emplace_back(jsonRelated);
			}

		diagnosticsBySourceUnit[*location->sourceName].emplace_back(jsonDiag);
	}

	if (m_client.traceValue() != TraceValue::Off)
	{
		Json extra;
		extra["openFileCount"] = Json(diagnosticsBySourceUnit.size());
		m_client.trace("Number of currently open files: " + std::to_string(diagnosticsBySourceUnit.size()), extra);
	}

	m_nonemptyDiagnostics.clear();
	for (auto&& [sourceUnitName, diagnostics]: diagnosticsBySourceUnit)
	{
		Json params;
		params["uri"] = m_fileRepository.sourceUnitNameToUri(sourceUnitName);
		if (!diagnostics.empty())
			m_nonemptyDiagnostics.insert(sourceUnitName);
		params["diagnostics"] = std::move(diagnostics);
		m_client.notify("textDocument/publishDiagnostics", std::move(params));
	}
}

bool LanguageServer::run()
{
	while (m_state != State::ExitRequested && m_state != State::ExitWithoutShutdown && !m_client.closed())
	{
		MessageID id;
		try
		{
			std::optional<Json> const jsonMessage = m_client.receive();
			if (!jsonMessage)
				continue;

			if ((*jsonMessage).contains("method") && (*jsonMessage)["method"].is_string())
			{
				std::string const methodName = (*jsonMessage)["method"].get<std::string>();
				if ((*jsonMessage).contains("id"))
					id = (*jsonMessage)["id"];
				lspDebug(fmt::format("received method call: {}", methodName));

				if (auto handler = util::valueOrDefault(m_handlers, methodName))
					handler(id, (*jsonMessage)["params"]);
				else
					m_client.error(id, ErrorCode::MethodNotFound, "Unknown method " + methodName);
			}
			else
				m_client.error({}, ErrorCode::ParseError, "\"method\" has to be a string.");
		}
		catch (Json::exception const&)
		{
			m_client.error(id, ErrorCode::InvalidParams, "JSON object access error. Most likely due to a badly formatted JSON request message."s);
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
	lspRequire(
		m_state == State::Initialized,
		ErrorCode::ServerNotInitialized,
		"Server is not properly initialized."
	);
}

void LanguageServer::handleInitialize(MessageID _id, Json const& _args)
{
	lspRequire(
		m_state == State::Started,
		ErrorCode::RequestFailed,
		"Initialize called at the wrong time."
	);

	m_state = State::Initialized;

	// The default of FileReader is to use `.`, but the path from where the LSP was started
	// should not matter.
	std::string rootPath("/");
	if (_args.contains("rootUri") && _args["rootUri"].is_string())
	{
		rootPath = _args["rootUri"].get<std::string>();
		lspRequire(
			boost::starts_with(rootPath, "file://"),
			ErrorCode::InvalidParams,
			"rootUri only supports file URI scheme."
		);
		rootPath = stripFileUriSchemePrefix(rootPath);
	}
	else if (_args.contains("rootPath"))
		rootPath = _args["rootPath"].get<std::string>();

	if (_args.contains("trace"))
		setTrace(_args["trace"]);

	m_fileRepository = FileRepository(rootPath, {});
	if (_args.contains("initializationOptions") && _args["initializationOptions"].is_object())
		changeConfiguration(_args["initializationOptions"]);

	Json replyArgs;
	replyArgs["serverInfo"]["name"] = "solc";
	replyArgs["serverInfo"]["version"] = std::string(VersionNumber);
	replyArgs["capabilities"]["definitionProvider"] = true;
	replyArgs["capabilities"]["implementationProvider"] = true;
	replyArgs["capabilities"]["textDocumentSync"]["change"] = 2; // 0=none, 1=full, 2=incremental
	replyArgs["capabilities"]["textDocumentSync"]["openClose"] = true;
	replyArgs["capabilities"]["semanticTokensProvider"]["legend"] = semanticTokensLegend();
	replyArgs["capabilities"]["semanticTokensProvider"]["range"] = false;
	replyArgs["capabilities"]["semanticTokensProvider"]["full"] = true; // XOR requests.full.delta = true
	replyArgs["capabilities"]["renameProvider"] = true;
	replyArgs["capabilities"]["hoverProvider"] = true;

	m_client.reply(_id, std::move(replyArgs));
}

void LanguageServer::handleInitialized(MessageID, Json const&)
{
	if (m_fileLoadStrategy == FileLoadStrategy::ProjectDirectory)
		compileAndUpdateDiagnostics();
}

void LanguageServer::semanticTokensFull(MessageID _id, Json const& _args)
{
	if (_args.contains("textDocument") && _args["textDocument"].contains("uri"))
	{
		auto uri = _args["textDocument"]["uri"];

		compile();

		auto const sourceName = m_fileRepository.uriToSourceUnitName(uri.get<std::string>());
		SourceUnit const& ast = m_compilerStack.ast(sourceName);
		m_compilerStack.charStream(sourceName);
		Json data = SemanticTokensBuilder().build(ast, m_compilerStack.charStream(sourceName));

		Json reply;
		reply["data"] = data;

		m_client.reply(_id, std::move(reply));
	}
	else
		m_client.error(_id, ErrorCode::InvalidParams, "Invalid parameter: textDocument.uri expected.");
}

void LanguageServer::handleWorkspaceDidChangeConfiguration(Json const& _args)
{
	requireServerInitialized();

	if (_args.contains("settings") && _args["settings"].is_object())
		changeConfiguration(_args["settings"]);
}

void LanguageServer::setTrace(Json const& _args)
{
	if (!_args.is_string())
		// Simply ignore invalid parameter.
		return;

	std::string const stringValue = _args.get<std::string>();
	if (stringValue == "off")
		m_client.setTrace(TraceValue::Off);
	else if (stringValue == "messages")
		m_client.setTrace(TraceValue::Messages);
	else if (stringValue == "verbose")
		m_client.setTrace(TraceValue::Verbose);
}

void LanguageServer::handleTextDocumentDidOpen(Json const& _args)
{
	requireServerInitialized();

	lspRequire(
		_args.contains("textDocument"),
		ErrorCode::RequestFailed,
		"Text document parameter missing."
	);

	if (_args["textDocument"].contains("text") && _args["textDocument"].contains("uri"))
	{
		std::string text = _args["textDocument"]["text"].get<std::string>();
		std::string uri = _args["textDocument"]["uri"].get<std::string>();
		m_openFiles.insert(uri);
		m_fileRepository.setSourceByUri(uri, std::move(text));
		compileAndUpdateDiagnostics();
	}
}

void LanguageServer::handleTextDocumentDidChange(Json const& _args)
{
	requireServerInitialized();

	if (_args.contains("textDocument") && _args["textDocument"].contains("uri"))
	{
		std::string const uri = _args["textDocument"]["uri"].get<std::string>();

		if (_args.contains("contentChanges"))
			for (auto const& [_, jsonContentChange]: _args["contentChanges"].items())
			{
				lspRequire(jsonContentChange.is_object(), ErrorCode::RequestFailed, "Invalid content reference.");

				std::string const sourceUnitName = m_fileRepository.uriToSourceUnitName(uri);
				lspRequire(
					m_fileRepository.sourceUnits().count(sourceUnitName),
					ErrorCode::RequestFailed,
					"Unknown file: " + uri);

				if (jsonContentChange.contains("text"))
				{
					std::string text = jsonContentChange["text"].get<std::string>();
					if (jsonContentChange.contains("range")
						&& jsonContentChange["range"].is_object()) // otherwise full content update
					{
						std::optional<SourceLocation> change
							= parseRange(m_fileRepository, sourceUnitName, jsonContentChange["range"]);
						lspRequire(
							change && change->hasText(),
							ErrorCode::RequestFailed,
							"Invalid source range: " + util::jsonCompactPrint(jsonContentChange["range"]));

						std::string buffer = m_fileRepository.sourceUnits().at(sourceUnitName);
						buffer.replace(
							static_cast<size_t>(change->start),
							static_cast<size_t>(change->end - change->start),
							std::move(text));
						text = std::move(buffer);
					}
					m_fileRepository.setSourceByUri(uri, std::move(text));
				}
			}

		compileAndUpdateDiagnostics();
	}
}

void LanguageServer::handleTextDocumentDidClose(Json const& _args)
{
	requireServerInitialized();

	lspRequire(
		_args.contains("textDocument"),
		ErrorCode::RequestFailed,
		"Text document parameter missing."
	);

	if (_args["textDocument"].contains("uri"))
	{
		std::string uri = _args["textDocument"]["uri"].get<std::string>();
		m_openFiles.erase(uri);

		compileAndUpdateDiagnostics();
	}
}

ASTNode const* LanguageServer::astNodeAtSourceLocation(std::string const& _sourceUnitName, LineColumn const& _filePos)
{
	return std::get<ASTNode const*>(astNodeAndOffsetAtSourceLocation(_sourceUnitName, _filePos));
}

std::tuple<ASTNode const*, int> LanguageServer::astNodeAndOffsetAtSourceLocation(std::string const& _sourceUnitName, LineColumn const& _filePos)
{
	if (m_compilerStack.state() < CompilerStack::AnalysisSuccessful)
		return {nullptr, -1};
	if (!m_fileRepository.sourceUnits().count(_sourceUnitName))
		return {nullptr, -1};

	std::optional<int> sourcePos = m_compilerStack.charStream(_sourceUnitName).translateLineColumnToPosition(_filePos);
	if (!sourcePos)
		return {nullptr, -1};

	return {locateInnermostASTNode(*sourcePos, m_compilerStack.ast(_sourceUnitName)), *sourcePos};
}
