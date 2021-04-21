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
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/ReadFile.h>
#include <libsolidity/lsp/LanguageServer.h>
#include <libsolidity/lsp/ReferenceCollector.h>

#include <liblangutil/SourceReferenceExtractor.h>
#include <liblangutil/CharStream.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/JSON.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <fmt/format.h>

#include <ostream>

#include <iostream>
#include <string>

using namespace std;
using namespace std::placeholders;

using namespace solidity::langutil;
using namespace solidity::frontend;

namespace solidity::lsp {

namespace // {{{ helpers
{

string toFileURI(boost::filesystem::path const& _path)
{
	return "file://" + _path.generic_string();
}

class ASTNodeLocator: public ASTConstVisitor
{
public:
	static ASTNode const* locate(int _pos, SourceUnit const& _sourceUnit)
	{
		ASTNodeLocator locator{_pos};
		_sourceUnit.accept(locator);
		return locator.m_closestMatch;
	}

	bool visitNode(ASTNode const& _node) override
	{
		if (_node.location().contains(m_pos))
		{
			m_closestMatch = &_node;
			return true;
		}
		return false;
	}

private:
	explicit ASTNodeLocator(int _pos): m_pos{_pos}, m_closestMatch{nullptr} {}

	int m_pos;
	ASTNode const* m_closestMatch;
};

optional<string> extractPathFromFileURI(std::string const& _uri)
{
	if (!boost::starts_with(_uri, "file://"))
		return nullopt;

	return _uri.substr(7);
}

Json::Value toJson(LineColumn _pos)
{
	Json::Value json = Json::objectValue;
	json["line"] = max(_pos.line, 0);
	json["character"] = max(_pos.column, 0);

	return json;
}

Json::Value toJsonRange(int _startLine, int _startColumn, int _endLine, int _endColumn)
{
	Json::Value json;
	json["start"] = toJson({_startLine, _startColumn});
	json["end"] = toJson({_endLine, _endColumn});
	return json;
}

Json::Value toJsonRange(SourceLocation const& _location)
{
	solAssert(_location.source, "");
	auto const [startLine, startColumn] = _location.source->translatePositionToLineColumn(_location.start);
	auto const [endLine, endColumn] = _location.source->translatePositionToLineColumn(_location.end);
	return toJsonRange(startLine, startColumn, endLine, endColumn);
}

Json::Value toJson(boost::filesystem::path const& _basePath, SourceLocation const& _location)
{
	solAssert(_location.source.get() != nullptr, "");
	Json::Value item = Json::objectValue;
	item["uri"] = toFileURI(_basePath / boost::filesystem::path(_location.source->name()));
	item["range"] = toJsonRange(_location);
	return item;
}

std::pair<size_t, size_t> offsetsOf(std::string const& _text, LineColumnRange _range) noexcept
{
	auto const start = CharStream::translateLineColumnToPosition(_text, _range.start.line, _range.start.column);
	auto const end = CharStream::translateLineColumnToPosition(_text, _range.end.line, _range.end.column);
	solAssert(start.has_value(), "");
	solAssert(end.has_value(), "");
	return std::pair{ size_t(start.value()), size_t(end.value()) };
}

} // }}} end helpers

LanguageServer::LanguageServer(Logger _logger):
	LanguageServer(
		_logger,
		make_unique<JSONTransport>(std::cin, std::cout, std::move(_logger))
	)
{
}

LanguageServer::LanguageServer(Logger _logger, std::unique_ptr<Transport> _transport):
	m_client{std::move(_transport)},
	m_handlers{
		{"cancelRequest", [](auto, auto) {/*don't do anything for now, as we're synchronous*/}},
		{"$/cancelRequest", [](auto, auto) {/*don't do anything for now, as we're synchronous*/}},
		{"initialize", bind(&LanguageServer::handle_initialize, this, _1, _2)},
		{"initialized", {} },
		{"shutdown", [this](auto, auto) { m_shutdownRequested = true; }},
		{"workspace/didChangeConfiguration", bind(&LanguageServer::handle_workspace_didChangeConfiguration, this, _1, _2)},
		{"textDocument/didOpen", bind(&LanguageServer::handle_textDocument_didOpen, this, _1, _2)},
		{"textDocument/didChange", bind(&LanguageServer::handle_textDocument_didChange, this, _1, _2)},
		{"textDocument/didClose", [](auto, auto) {/*nothing for now*/}},
		{"textDocument/definition", bind(&LanguageServer::handle_textDocument_definition, this, _1, _2)},
		{"textDocument/documentHighlight", bind(&LanguageServer::handle_textDocument_highlight, this, _1, _2)},
		{"textDocument/hover", bind(&LanguageServer::handle_textDocument_hover, this, _1, _2)},
		{"textDocument/references", bind(&LanguageServer::handle_textDocument_references, this, _1, _2)},
	},
	m_logger{std::move(_logger)}
{
}

DocumentPosition LanguageServer::extractDocumentPosition(Json::Value const& _json)
{
	DocumentPosition dpos{};

	dpos.path = extractPathFromFileURI(_json["textDocument"]["uri"].asString()).value();

	if (boost::algorithm::starts_with(dpos.path, m_basePath.generic_string()))
		dpos.path = dpos.path.substr(m_basePath.generic_string().size());

	dpos.position.line = _json["position"]["line"].asInt();
	dpos.position.column = _json["position"]["character"].asInt();

	return dpos;
}

void LanguageServer::changeConfiguration(Json::Value const& _settings)
{
	if (_settings["evm"].isString())
		if (auto const evmVersionOpt = EVMVersion::fromString(_settings["evm"].asString()); evmVersionOpt.has_value())
			m_evmVersion = evmVersionOpt.value();

	if (_settings["remapping"].isArray())
	{
		for (auto const& element: _settings["remapping"])
		{
			if (element.isString())
			{
				if (auto remappingOpt = ImportRemapper::parseRemapping(element.asString()); remappingOpt.has_value())
					m_remappings.emplace_back(move(remappingOpt.value()));
				else
					trace("Failed to parse remapping: '"s + element.asString() + "'");
			}
		}
	}
}

void LanguageServer::documentContentUpdated(string const& _path, LineColumnRange _range, string const& _replacementText)
{
	// TODO: all this info is actually unrelated to solidity/lsp specifically except knowing that
	// the file has updated, so we can  abstract that away and only do the re-validation here.
	auto file = m_fileReader->sourceCodes().find(_path);
	if (file == m_fileReader->sourceCodes().end())
	{
		log("LanguageServer: File to be modified not opened \"{}\"", _path);
		return;
	}

	string buffer = file->second;

	auto const [start, end] = offsetsOf(buffer, _range);
	buffer.replace(start, end - start, _replacementText);

	m_fileReader->setSource(_path, buffer);
	m_pathMappings[_path] = _path;
}

void LanguageServer::documentContentUpdated(string const& _path, string const& _replacementText)
{
	auto file = m_fileReader->sourceCodes().find(_path);
	if (file == m_fileReader->sourceCodes().end())
	{
		log("LanguageServer: File to be modified not opened \"" + _path + "\"");
		return;
	}

	m_fileReader->setSource(_path, _replacementText);
	m_pathMappings[_path] = _path;

	compileSource(_path);
}

frontend::ReadCallback::Result LanguageServer::readFile(string const& _kind, string const& _path)
{
	log("readFile: " + _path);
	return m_fileReader->readFile(_kind, _path);
}

constexpr DiagnosticSeverity toDiagnosticSeverity(Error::Type _errorType)
{
	using Type = Error::Type;
	using Severity = DiagnosticSeverity;
	switch (_errorType)
	{
		case Type::CodeGenerationError:
		case Type::DeclarationError:
		case Type::DocstringParsingError:
		case Type::ParserError:
		case Type::SyntaxError:
		case Type::TypeError:
			return Severity::Error;
		case Type::Warning:
			return Severity::Warning;
	}
	// Should never be reached.
	return Severity::Error;
}

bool LanguageServer::compile(std::string const& _path)
{
	// TODO: optimize! do not recompile if nothing has changed (file(s) not flagged dirty).

	auto const i = m_fileReader->sourceCodes().find(_path);
	if (i == m_fileReader->sourceCodes().end())
	{
		log("source code not found for path: " + _path);
		return false;
	}
	log("compile: " + _path + " (" + i->second + ")");

	m_compilerStack.reset();
	m_compilerStack = make_unique<CompilerStack>(bind(&FileReader::readFile, ref(*m_fileReader), _1, _2));

	// TODO: configure all compiler flags like in CommandLineInterface (TODO: refactor to share logic!)
	OptimiserSettings settings = OptimiserSettings::standard(); // TODO: get from config
	m_compilerStack->setOptimiserSettings(settings);
	m_compilerStack->setParserErrorRecovery(false);
	m_compilerStack->setRevertStringBehaviour(RevertStrings::Default); // TODO get from config
	m_compilerStack->setSources(m_fileReader->sourceCodes());
	m_compilerStack->setRemappings(m_remappings);

	m_compilerStack->setEVMVersion(m_evmVersion);

	trace("compile: using EVM "s + m_evmVersion.name());

	m_compilerStack->compile(CompilerStack::State::AnalysisPerformed);
	return true;
}

void LanguageServer::compileSource(std::string const& _path)
{
	compile(_path);

	Json::Value params;
	params["uri"] = toFileURI(m_basePath / _path);

	params["diagnostics"] = Json::arrayValue;
	for (shared_ptr<Error const> const& error: m_compilerStack->errors())
	{
		SourceReferenceExtractor::Message const message = SourceReferenceExtractor::extract(*error);

		Json::Value jsonDiag;
		jsonDiag["source"] = "solc";
		jsonDiag["severity"] = int(toDiagnosticSeverity(error->type()));
		jsonDiag["message"] = message.primary.message;

		if (message.errorId.has_value())
			jsonDiag["code"] = Json::UInt64{message.errorId.value().error};

		jsonDiag["range"] = toJsonRange(
			message.primary.position.line, message.primary.startColumn,
			message.primary.position.line, message.primary.endColumn
		);

		for (SourceReference const& secondary: message.secondary)
		{
			Json::Value jsonRelated;
			jsonRelated["message"] = secondary.message;
			jsonRelated["location"]["uri"] = toFileURI(m_basePath / boost::filesystem::path(secondary.sourceName));
			jsonRelated["location"]["range"] = toJsonRange(
				secondary.position.line, secondary.startColumn,
				secondary.position.line, secondary.endColumn
			);
			jsonDiag["relatedInformation"].append(jsonRelated);
		}

		params["diagnostics"].append(jsonDiag);
	}

	m_client->notify("textDocument/publishDiagnostics", params);
}

frontend::ASTNode const* LanguageServer::findASTNode(LineColumn _position, std::string const& _fileName)
{
	if (!m_compilerStack)
	{
		log("findASTNode: {}:{}:{}: No compiler stack.", _fileName, _position.line, _position.column);
		return nullptr;
	}

	if (m_compilerStack->state() < frontend::CompilerStack::AnalysisPerformed)
	{
		log("findASTNode: {}:{}:{}: Compilation failed.", _fileName, _position.line, _position.column);
		return nullptr;
	}

	frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(_fileName);
	log("findASTNode: {}:{}:{}", _fileName, _position.line, _position.column);
	auto const sourcePos = sourceUnit.location().source->translateLineColumnToPosition(_position.line, _position.column);

	if (!sourcePos.has_value())
	{
		trace(
			"findASTNode: could not locate offset for "s +
			to_string(_position.line) + ":" +
			to_string(_position.column)
		);
		return nullptr;
	}

	ASTNode const* closestMatch = ASTNodeLocator::locate(sourcePos.value(), sourceUnit);

	if (!closestMatch)
		trace(
			"findASTNode not found for "s +
			to_string(sourcePos.value()) + ":" +
			to_string(_position.line) + ":" +
			to_string(_position.column)
		);
	else
		trace(
			"findASTNode found for "s +
			to_string(sourcePos.value()) + ":" +
			to_string(_position.line) + ":" +
			to_string(_position.column) + ": " +
			closestMatch->location().text() + " (" +
			typeid(*closestMatch).name() +
			")"s
		);


	return closestMatch;
}

optional<SourceLocation> LanguageServer::declarationPosition(frontend::Declaration const* _declaration)
{
	if (!_declaration)
		return nullopt;

	if (_declaration->nameLocation().isValid())
		return _declaration->nameLocation();

	if (_declaration->location().isValid())
		return _declaration->location();

	return nullopt;
}

std::vector<SourceLocation> LanguageServer::findAllReferences(
	frontend::Declaration const* _declaration,
	string const& _sourceIdentifierName,
	frontend::SourceUnit const& _sourceUnit
)
{
	std::vector<SourceLocation> output;
	for (DocumentHighlight& highlight: ReferenceCollector::collect(_declaration, _sourceUnit, _sourceIdentifierName))
		output.emplace_back(move(highlight.location));
	return output;
}

void LanguageServer::findAllReferences(
	frontend::Declaration const* _declaration,
	string const& _sourceIdentifierName,
	frontend::SourceUnit const& _sourceUnit,
	std::vector<SourceLocation>& _output
)
{
	for (DocumentHighlight& highlight: ReferenceCollector::collect(_declaration, _sourceUnit, _sourceIdentifierName))
		_output.emplace_back(move(highlight.location));
}

vector<SourceLocation> LanguageServer::references(DocumentPosition _documentPosition)
{
	auto const file = m_fileReader->sourceCodes().find(_documentPosition.path);
	if (file == m_fileReader->sourceCodes().end())
	{
		trace("File does not exist. " + _documentPosition.path);
		return {};
	}

	if (!m_compilerStack)
		compile(_documentPosition.path);

	solAssert(m_compilerStack.get() != nullptr, "");

	string const& sourceName = _documentPosition.path;

	ASTNode const* sourceNode = findASTNode(_documentPosition.position, sourceName);
	if (!sourceNode)
	{
		trace("references: AST node not found");
		return {};
	}

	if (auto const sourceIdentifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		string const& sourceName = _documentPosition.path;
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(sourceName);

		vector<SourceLocation> output;
		if (auto decl = sourceIdentifier->annotation().referencedDeclaration)
			output += findAllReferences(decl, decl->name(), sourceUnit);

		for (auto const decl: sourceIdentifier->annotation().candidateDeclarations)
			output += findAllReferences(decl, decl->name(), sourceUnit);
		return output;
	}
	else if (auto const decl = dynamic_cast<VariableDeclaration const*>(sourceNode))
	{
		auto const sourceName = _documentPosition.path;
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(sourceName);
		return findAllReferences(decl, decl->name(), sourceUnit);
	}
	else if (auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(sourceNode))
	{
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(sourceName);
		return findAllReferences(functionDefinition, functionDefinition->name(), sourceUnit);
	}
	else if (auto const* enumDef = dynamic_cast<EnumDefinition const*>(sourceNode))
	{
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(sourceName);
		return findAllReferences(enumDef, enumDef->name(), sourceUnit);
	}
	else if (auto const memberAccess = dynamic_cast<MemberAccess const*>(sourceNode))
	{
		if (Declaration const* decl = memberAccess->annotation().referencedDeclaration)
		{
			auto const sourceName = _documentPosition.path;
			frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(sourceName);
			return findAllReferences(decl, memberAccess->memberName(), sourceUnit);
		}
		return {};
	}
	else if (auto const* importDef = dynamic_cast<ImportDirective const*>(sourceNode))
	{
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(sourceName);
		return findAllReferences(importDef, importDef->name(), sourceUnit);
	}

	trace("references: not an identifier: "s + typeid(*sourceNode).name());
	return {};
}

vector<DocumentHighlight> LanguageServer::semanticHighlight(DocumentPosition _documentPosition)
{
	solAssert(m_compilerStack.get() != nullptr, "");
	log("highlight: {}", _documentPosition.path);

	if (m_compilerStack->state() < frontend::CompilerStack::AnalysisPerformed)
	{
		log("semanticHighlight: no compiler stack.");
		return {};
	}

	frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(_documentPosition.path);
	ASTNode const* sourceNode = findASTNode(_documentPosition.position, _documentPosition.path);
	if (!sourceNode)
	{
		trace("semanticHighlight: AST node not found");
		return {};
	}

	trace(
		"semanticHighlight: Source Node("s + typeid(*sourceNode).name() + "): " +
		sourceNode->location().text()
	);

	// TODO: ImportDirective: hovering a symbol of an import directive should highlight all uses of that symbol.
	if (auto const* sourceIdentifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		vector<DocumentHighlight> output;
		if (sourceIdentifier->annotation().referencedDeclaration)
			output += ReferenceCollector::collect(sourceIdentifier->annotation().referencedDeclaration, sourceUnit, sourceIdentifier->name());

		for (Declaration const* declaration: sourceIdentifier->annotation().candidateDeclarations)
			output += ReferenceCollector::collect(declaration, sourceUnit, sourceIdentifier->name());

		for (Declaration const* declaration: sourceIdentifier->annotation().overloadedDeclarations)
			output += ReferenceCollector::collect(declaration, sourceUnit, sourceIdentifier->name());
		return output;
	}
	else if (auto const* varDecl = dynamic_cast<VariableDeclaration const*>(sourceNode))
		return ReferenceCollector::collect(varDecl, sourceUnit, varDecl->name());
	else if (auto const* structDef = dynamic_cast<StructDefinition const*>(sourceNode))
		return ReferenceCollector::collect(structDef, sourceUnit, structDef->name());
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(sourceNode))
	{
		Type const* type = memberAccess->expression().annotation().type;
		if (auto const ttype = dynamic_cast<TypeType const*>(type))
		{
			auto const memberName = memberAccess->memberName();

			if (auto const* enumType = dynamic_cast<EnumType const*>(ttype->actualType()))
			{
				// find the definition
				vector<DocumentHighlight> output;
				for (auto const& enumMember: enumType->enumDefinition().members())
					if (enumMember->name() == memberName)
						output += ReferenceCollector::collect(enumMember.get(), sourceUnit, enumMember->name());

				// TODO: find uses of the enum value
				return output;
			}
		}
		else
		{
			// TODO: StructType, ...
			trace("semanticHighlight: member type is: "s + (type ? typeid(*type).name() : "NULL"));
		}

		// TODO: If the cursor os positioned on top of a type name, then all other symbols matching
		// this type should be highlighted (clangd does so, too).
		//
		// if (auto const tt = dynamic_cast<TypeType const*>(type))
		// {
		// 	output = findAllReferences(declaration, sourceUnit);
		// }
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(sourceNode))
	{
		solAssert(!identifierPath->path().empty(), "");
		return ReferenceCollector::collect(identifierPath->annotation().referencedDeclaration, sourceUnit, identifierPath->path().back());
	}
	else if (auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(sourceNode))
		return ReferenceCollector::collect(functionDefinition, sourceUnit, functionDefinition->name());
	else if (auto const* enumDef = dynamic_cast<EnumDefinition const*>(sourceNode))
		return ReferenceCollector::collect(enumDef, sourceUnit, enumDef->name());
	else if (auto const* importDef = dynamic_cast<ImportDirective const*>(sourceNode))
		return ReferenceCollector::collect(importDef, sourceUnit, importDef->name());
	else
		trace("semanticHighlight: not an identifier. "s + typeid(*sourceNode).name());

	return {};
}

// {{{ LSP internals
bool LanguageServer::run()
{
	while (!m_exitRequested && !m_client->closed())
	{
		optional<Json::Value> const jsonMessage = m_client->receive();
		if (!jsonMessage.has_value())
		{
			m_client->error(Json::nullValue, ErrorCode::InvalidRequest, "Could not parse request.");
			continue;
		}

		try
		{
			handleMessage(*jsonMessage);
		}
		catch (std::exception const& e)
		{
			log("Unhandled exception caught when handling message. "s + e.what());
		}
	}

	if (m_shutdownRequested)
		return true;
	else
		return false;
}

#define MYDEBUG 0
void LanguageServer::handle_initialize(MessageID _id, Json::Value const& _args)
{
	string rootPath;
	if (Json::Value uri = _args["rootUri"])
		rootPath = extractPathFromFileURI(uri.asString()).value();
	else if (Json::Value rootPath = _args["rootPath"]; rootPath)
		rootPath = rootPath.asString();

	trace("initialize: root path: '"s + rootPath + "'");

	if (Json::Value value = _args["trace"]; value)
	{
		string const name = value.asString();
		if (name == "messages")
			m_trace = Trace::Messages;
		else if (name == "verbose")
			m_trace = Trace::Verbose;
		else if (name == "off")
			m_trace = Trace::Off;
	}

#if MYDEBUG // Currently not used.
	// At least VScode supports more than one workspace.
	// This is the list of initial configured workspace folders
	struct WorkspaceFolder { std::string name; std::string path; };
	std::vector<WorkspaceFolder> workspaceFolders;
	if (Json::Value folders = _args["workspaceFolders"]; folders)
	{
		for (Json::Value folder: folders)
		{
			WorkspaceFolder wsFolder{};
			wsFolder.name = folder["name"].asString();
			wsFolder.path = extractPathFromFileURI(folder["uri"].asString()).value();
			workspaceFolders.emplace_back(move(wsFolder));
		}
	}
#endif

	auto const fspath = boost::filesystem::path(rootPath);

	m_basePath = fspath;
	log("basepath: " + m_basePath.generic_string());
	m_fileReader = make_unique<FileReader>(m_basePath, FileReader::FileSystemPathSet{fspath});

	if (_args["initializationOptions"].isObject())
		changeConfiguration(_args["initializationOptions"]);

	// {{{ encoding
	Json::Value replyArgs;

	replyArgs["serverInfo"]["name"] = "solc";
	replyArgs["serverInfo"]["version"] = string(solidity::frontend::VersionNumber);
	replyArgs["hoverProvider"] = true;
	replyArgs["capabilities"]["hoverProvider"] = true;
	replyArgs["capabilities"]["textDocumentSync"]["openClose"] = true;
	replyArgs["capabilities"]["textDocumentSync"]["change"] = 2; // 0=none, 1=full, 2=incremental
	replyArgs["capabilities"]["definitionProvider"] = true;
	replyArgs["capabilities"]["documentHighlightProvider"] = true;
	replyArgs["capabilities"]["referencesProvider"] = true;

	m_client->reply(_id, replyArgs);
	// }}}
}

void LanguageServer::handle_workspace_didChangeConfiguration(MessageID, Json::Value const& _args)
{
	if (_args["settings"].isObject())
		changeConfiguration(_args["settings"]);
}

void LanguageServer::handle_exit(MessageID _id, Json::Value const& /*_args*/)
{
	m_exitRequested = true;
	auto const exitCode = m_shutdownRequested ? 0 : 1;

	Json::Value replyArgs = Json::intValue;
	replyArgs = exitCode;

	m_client->reply(_id, replyArgs);
}

void LanguageServer::handle_textDocument_didOpen(MessageID /*_id*/, Json::Value const& _args)
{
	// decoding
	if (!_args["textDocument"])
		return;

	auto const fullPath = extractPathFromFileURI(_args["textDocument"]["uri"].asString()).value();
	auto path = fullPath;
	if (boost::algorithm::starts_with(path, m_basePath.generic_string()))
		path = path.substr(m_basePath.generic_string().size());

	auto const text = _args["textDocument"]["text"].asString();
	// auto const version = _args["textDocument"]["version"].asInt();
	// auto const languageId = _args["textDocument"]["languageId"].asString();

	log("LanguageServer: Opening document: " + path);

	m_fileReader->setSource(path, text);
	m_pathMappings[fullPath] = path;

	compileSource(path);

	// no encoding
}

void LanguageServer::handle_textDocument_didChange(MessageID /*_id*/, Json::Value const& _args)
{
	//auto const version = _args["textDocument"]["version"].asInt();
	auto const path = extractPathFromFileURI(_args["textDocument"]["uri"].asString()).value();

	// TODO: in the longer run, I'd like to try moving the VFS handling into Server class, so
	// the specific Solidity LSP implementation doesn't need to care about that.

	auto const contentChanges = _args["contentChanges"];
	for (Json::Value jsonContentChange: contentChanges)
	{
		if (!jsonContentChange.isObject())
			// Protocol error, will only happen on broken clients, so silently ignore it.
			continue;

		auto const text = jsonContentChange["text"].asString();

		if (jsonContentChange["range"].isObject())
		{
			Json::Value jsonRange = jsonContentChange["range"];
			LineColumnRange range{};
			range.start.line = jsonRange["start"]["line"].asInt();
			range.start.column = jsonRange["start"]["character"].asInt();
			range.end.line = jsonRange["end"]["line"].asInt();
			range.end.column = jsonRange["end"]["character"].asInt();

			documentContentUpdated(path, range, text);
		}
		else
		{
			// full content update
			documentContentUpdated(path, text);
		}
	}

	if (!contentChanges.empty())
		compileSource(path);
}

void LanguageServer::handle_textDocument_definition(MessageID _id, Json::Value const& _args)
{
	DocumentPosition const dpos = extractDocumentPosition(_args);

	// source should be compiled already
	solAssert(m_compilerStack.get() != nullptr, "");

	log("definition: dpos.path: " + dpos.path);

	auto const file = m_fileReader->sourceCodes().find(dpos.path);
	if (file == m_fileReader->sourceCodes().end())
	{
		Json::Value emptyResponse = Json::arrayValue;
		m_client->reply(_id, emptyResponse);
		return;
	}

	auto const sourceNode = findASTNode(dpos.position, dpos.path);
	if (!sourceNode)
	{
		trace("gotoDefinition: AST node not found for "s + to_string(dpos.position.line) + ":" + to_string(dpos.position.column));
		// Could not infer AST node from given source location.
		Json::Value emptyResponse = Json::arrayValue;
		m_client->reply(_id, emptyResponse);
		return;
	}

	vector<SourceLocation> locations;
	if (auto const importDirective = dynamic_cast<ImportDirective const*>(sourceNode))
	{
		// When cursor is on an import directive, then we want to jump to the actual file that
		// is being imported.
		auto const fpm = m_pathMappings.find(importDirective->path());
		if (fpm != m_pathMappings.end())
			locations.emplace_back(SourceLocation{0, 0, make_shared<CharStream>("", fpm->second)});
		else
			trace("gotoDefinition: (importDirective) full path mapping not found\n");
	}
	else if (auto const n = dynamic_cast<frontend::MemberAccess const*>(sourceNode))
	{
		// For scope members, jump to the naming symbol of the referencing declaration of this member.
		auto const declaration = n->annotation().referencedDeclaration;
		auto const location = declarationPosition(declaration);
		if (location.has_value())
			locations.emplace_back(location.value());
		else
			trace("gotoDefinition: declaration not found.");
	}
	else if (auto const sourceIdentifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		// For identifiers, jump to the naming symbol of the definition of this identifier.
		if (Declaration const* decl = sourceIdentifier->annotation().referencedDeclaration)
			if (auto location = declarationPosition(decl); location.has_value())
				locations.emplace_back(move(location.value()));
		// if (auto location = declarationPosition(sourceIdentifier->annotation().referencedDeclaration); location.has_value())
		// 	locations.emplace_back(move(location.value()));

		for (auto const declaration: sourceIdentifier->annotation().candidateDeclarations)
			if (auto location = declarationPosition(declaration); location.has_value())
				locations.emplace_back(move(location.value()));
	}
	else
		trace("gotoDefinition: Symbol is not an identifier. "s + typeid(*sourceNode).name());

	Json::Value reply = Json::arrayValue;
	for (SourceLocation const& location: locations)
		reply.append(toJson(m_basePath, location));
	m_client->reply(_id, reply);
}

void LanguageServer::handle_textDocument_hover(MessageID _id, Json::Value const& _args)
{
	auto const dpos = extractDocumentPosition(_args);

	auto const sourceNode = findASTNode(dpos.position, dpos.path);
	if (!sourceNode)
	{
		Json::Value emptyResponse = Json::arrayValue;
		m_client->reply(_id, emptyResponse); // reply with "No references".
		return;
	}

	Json::Value reply = Json::objectValue;

	reply["range"] = toJsonRange(sourceNode->location());
	reply["contents"]["kind"] = "markdown";
	reply["contents"]["value"] = ""; // Will be the Natspec documentation (converted to markdown) later.

	m_client->reply(_id, reply);
}

void LanguageServer::handle_textDocument_highlight(MessageID _id, Json::Value const& _args)
{
	auto const dpos = extractDocumentPosition(_args);
	log("highlight: {}:{}:{}", dpos.path, dpos.position.line, dpos.position.column);

	if (!m_compilerStack)
		compile(dpos.path);

	Json::Value jsonReply = Json::arrayValue;
	for (DocumentHighlight const& highlight: semanticHighlight(dpos))
	{
		Json::Value item = Json::objectValue;
		item["range"] = toJsonRange(highlight.location);
		if (highlight.kind != DocumentHighlightKind::Unspecified)
			item["kind"] = static_cast<int>(highlight.kind);

		jsonReply.append(item);
	}
	m_client->reply(_id, jsonReply);
}

void LanguageServer::handle_textDocument_references(MessageID _id, Json::Value const& _args)
{
	auto const dpos = extractDocumentPosition(_args);

	trace(
		"find all references: " +
		dpos.path + ":" +
		to_string(dpos.position.line) + ":" +
		to_string(dpos.position.column)
	);

	auto file = m_fileReader->sourceCodes().find(dpos.path);
	if (file == m_fileReader->sourceCodes().end())
	{
		Json::Value emptyResponse = Json::arrayValue;
		m_client->reply(_id, emptyResponse); // reply with "No references".
		return;
	}

	if (!m_compilerStack)
		compile(dpos.path);
	solAssert(m_compilerStack.get() != nullptr, "");

	auto const sourceNode = findASTNode(dpos.position, dpos.path);
	if (!sourceNode)
	{
		Json::Value emptyResponse = Json::arrayValue;
		m_client->reply(_id, emptyResponse); // reply with "No references".
		return;
	}

	auto locations = vector<SourceLocation>{};
	if (auto const sourceIdentifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(dpos.path);

		if (auto decl = sourceIdentifier->annotation().referencedDeclaration)
			findAllReferences(decl, decl->name(), sourceUnit, locations);

		for (auto const decl: sourceIdentifier->annotation().candidateDeclarations)
			findAllReferences(decl, decl->name(), sourceUnit, locations);
	}
	else if (auto const decl = dynamic_cast<VariableDeclaration const*>(sourceNode))
	{
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(dpos.path);
		findAllReferences(decl, decl->name(), sourceUnit, locations);
	}
	else if (auto const* functionDefinition = dynamic_cast<FunctionDefinition const*>(sourceNode))
	{
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(dpos.path);
		findAllReferences(functionDefinition, functionDefinition->name(), sourceUnit, locations);
	}
	else if (auto const* enumDef = dynamic_cast<EnumDefinition const*>(sourceNode))
	{
		frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(dpos.path);
		findAllReferences(enumDef, enumDef->name(), sourceUnit, locations);
	}
	else if (auto const memberAccess = dynamic_cast<MemberAccess const*>(sourceNode))
	{
		if (Declaration const* decl = memberAccess->annotation().referencedDeclaration)
		{
			frontend::SourceUnit const& sourceUnit = m_compilerStack->ast(dpos.path);
			findAllReferences(decl, memberAccess->memberName(), sourceUnit, locations);
		}
	}
	else
		trace("references: not an identifier: "s + typeid(*sourceNode).name());

	Json::Value jsonReply = Json::arrayValue;
	for (SourceLocation const& location: locations)
		jsonReply.append(toJson(m_basePath, location));

	m_client->reply(_id, jsonReply);
}

void LanguageServer::log(string _message)
{
	if (m_trace < Trace::Messages)
		return;

	if (m_logger)
		m_logger(_message);

	// Json::Value json = Json::objectValue;
	// json["type"] = static_cast<int>(Trace::Messages);
	// json["message"] = move(_message);
	// m_client->notify("window/logMessage", json);
}

void LanguageServer::trace(string const& _message)
{
	// if (m_trace < Trace::Verbose)
	// 	return;

	Json::Value json = Json::objectValue;
	json["type"] = static_cast<int>(Trace::Verbose);
	json["message"] = _message;

	m_client->notify("window/logMessage", json);

	if (m_logger)
		m_logger(_message);
}

void LanguageServer::handleMessage(Json::Value const& _jsonMessage)
{
	string const methodName = _jsonMessage["method"].asString();

	MessageID const id = _jsonMessage["id"].isInt()
		? MessageID{to_string(_jsonMessage["id"].asInt())}
		: _jsonMessage["id"].isString()
			? MessageID{_jsonMessage["id"].asString()}
			: MessageID{};

	auto const handler = m_handlers.find(methodName);
	if (handler == m_handlers.end())
		m_client->error(id, ErrorCode::MethodNotFound, "Unknown method " + methodName);
	else if (handler->second)
	{
		Json::Value const& jsonArgs = _jsonMessage["params"];
		handler->second(id, jsonArgs);
	}
}
// }}}

} // namespace solidity
