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
#include <libsolidity/lsp/ReferenceCollector.h>

#include <liblangutil/SourceReferenceExtractor.h>
#include <liblangutil/CharStream.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/JSON.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <fmt/format.h>

#include <ostream>
#include <string>

using namespace std;
using namespace std::placeholders;

using namespace solidity::langutil;
using namespace solidity::frontend;

namespace solidity::lsp
{

namespace // {{{ helpers
{

string toFileURI(boost::filesystem::path const& _path)
{
	return "file://" + _path.generic_string();
}

optional<string> extractPathFromFileURI(string const& _uri)
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

constexpr int toDiagnosticSeverity(Error::Type _errorType)
{
	// 1=Error, 2=Warning, 3=Info, 4=Hint
	switch (_errorType)
	{
		case Error::Type::CodeGenerationError:
		case Error::Type::DeclarationError:
		case Error::Type::DocstringParsingError:
		case Error::Type::ParserError:
		case Error::Type::SyntaxError:
		case Error::Type::TypeError:
			return 1;
		case Error::Type::Warning:
			return 2;
	}
	return 1;
}

vector<Declaration const*> allAnnotatedDeclarations(Identifier const* _identifier)
{
	vector<Declaration const*> output;
	output.push_back(_identifier->annotation().referencedDeclaration);
	output += _identifier->annotation().candidateDeclarations;
	return output;
}

} // }}} end helpers

LanguageServer::LanguageServer(Logger _logger, unique_ptr<Transport> _transport):
	m_client{move(_transport)},
	m_handlers{
		{"$/cancelRequest", {} }, // Don't do anything for now, as we're synchronous.
		{"cancelRequest", {} }, // Don't do anything for now, as we're synchronous.
		{"initialize", bind(&LanguageServer::handleInitialize, this, _1, _2)},
		{"initialized", {} },
		{"shutdown", [this](auto, auto) { m_shutdownRequested = true; }},
		{"textDocument/definition", [this](auto _id, auto _args) { handleGotoDefinition(_id, _args); }},
		{"textDocument/didChange", bind(&LanguageServer::handleTextDocumentDidChange, this, _1, _2)},
		{"textDocument/didClose", [](auto, auto) {/*nothing for now*/}},
		{"textDocument/didOpen", bind(&LanguageServer::handleTextDocumentDidOpen, this, _1, _2)},
		{"textDocument/documentHighlight", bind(&LanguageServer::handleTextDocumentHighlight, this, _1, _2)},
		{"textDocument/hover", bind(&LanguageServer::handleTextDocumentHover, this, _1, _2)},
		{"textDocument/implementation", [this](auto _id, auto _args) { handleGotoDefinition(_id, _args); }},
		{"textDocument/references", bind(&LanguageServer::handleTextDocumentReferences, this, _1, _2)},
		{"workspace/didChangeConfiguration", bind(&LanguageServer::handleWorkspaceDidChangeConfiguration, this, _1, _2)},
	},
	m_logger{move(_logger)}
{
}

DocumentPosition LanguageServer::extractDocumentPosition(Json::Value const& _json) const
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
	m_settingsObject = _settings;
}

bool LanguageServer::compile(string const& _path)
{
	// TODO: optimize! do not recompile if nothing has changed (file(s) not flagged dirty).

	auto const i = m_fileReader->sourceCodes().find(_path);
	if (i == m_fileReader->sourceCodes().end())
	{
		log("source code not found for path: " + _path);
		log(fmt::format("Available: {}", m_fileReader->sourceCodes().size()));
		for (auto const& x: m_fileReader->sourceCodes())
		{
			log(fmt::format(" - file: {}", x.first));
		}
		return false;
	}

	// Json::Value jsonInput;
	// jsonInput.copy(m_settingsObject);

	m_compilerStack.reset();
	m_compilerStack = make_unique<CompilerStack>(bind(&FileReader::readFile, ref(*m_fileReader), _1, _2));

	StandardCompiler::InputsAndSettings inputsAndSettings = m_inputsAndSettings;
	inputsAndSettings.sources = m_fileReader->sourceCodes();
	StandardCompiler::configure(inputsAndSettings, *m_compilerStack);

	m_compilerStack->compile(CompilerStack::State::AnalysisPerformed);

	return true;
}

void LanguageServer::compileSourceAndReport(string const& _path)
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
		jsonDiag["severity"] = toDiagnosticSeverity(error->type());
		jsonDiag["message"] = message.primary.message;
		jsonDiag["range"] = toJsonRange(
			message.primary.position.line, message.primary.startColumn,
			message.primary.position.line, message.primary.endColumn
		);
		if (message.errorId.has_value())
			jsonDiag["code"] = Json::UInt64{message.errorId.value().error};

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

ASTNode const* LanguageServer::requestASTNode(DocumentPosition _filePos)
{
	if (!m_compilerStack)
		compile(_filePos.path);

	auto file = m_fileReader->sourceCodes().find(_filePos.path);
	if (file == m_fileReader->sourceCodes().end())
		return nullptr;

	if (!m_compilerStack || m_compilerStack->state() < CompilerStack::AnalysisPerformed)
		return nullptr;

	SourceUnit const& sourceUnit = m_compilerStack->ast(_filePos.path);
	auto const sourcePos = sourceUnit.location().source->translateLineColumnToPosition(_filePos.position.line, _filePos.position.column);
	if (!sourcePos.has_value())
		return nullptr;

	return locateASTNode(sourcePos.value(), sourceUnit);
}

optional<SourceLocation> LanguageServer::declarationPosition(Declaration const* _declaration)
{
	if (!_declaration)
		return nullopt;

	if (_declaration->nameLocation().isValid())
		return _declaration->nameLocation();

	if (_declaration->location().isValid())
		return _declaration->location();

	return nullopt;
}

vector<SourceLocation> LanguageServer::findAllReferences(
	Declaration const* _declaration,
	string const& _sourceIdentifierName,
	SourceUnit const& _sourceUnit
)
{
	vector<SourceLocation> output;
	for (DocumentHighlight& highlight: ReferenceCollector::collect(_declaration, _sourceUnit, _sourceIdentifierName))
		output.emplace_back(move(highlight.location));
	return output;
}

void LanguageServer::findAllReferences(
	Declaration const* _declaration,
	string const& _sourceIdentifierName,
	SourceUnit const& _sourceUnit,
	vector<SourceLocation>& _output
)
{
	for (DocumentHighlight& highlight: ReferenceCollector::collect(_declaration, _sourceUnit, _sourceIdentifierName))
		_output.emplace_back(move(highlight.location));
}

vector<SourceLocation> LanguageServer::references(DocumentPosition _documentPosition)
{
	ASTNode const* sourceNode = requestASTNode(_documentPosition);
	if (!sourceNode)
		return {};

	SourceUnit const& sourceUnit = m_compilerStack->ast(_documentPosition.path);
	vector<SourceLocation> output;
	if (auto const* identifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		for (auto const* declaration: allAnnotatedDeclarations(identifier))
			output += findAllReferences(declaration, declaration->name(), sourceUnit);
	}
	else if (auto const* declaration = dynamic_cast<Declaration const*>(sourceNode))
	{
		output += findAllReferences(declaration, declaration->name(), sourceUnit);
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(sourceNode))
	{
		if (Declaration const* decl = memberAccess->annotation().referencedDeclaration)
			output += findAllReferences(decl, memberAccess->memberName(), sourceUnit);
	}
	return output;
}

vector<DocumentHighlight> LanguageServer::semanticHighlight(ASTNode const* _sourceNode, string const& _path)
{
	ASTNode const* sourceNode = _sourceNode; // TODO
	if (!sourceNode)
		return {};

	SourceUnit const& sourceUnit = m_compilerStack->ast(_path);

	vector<DocumentHighlight> output;
	if (auto const* declaration = dynamic_cast<Declaration const*>(sourceNode))
	{
		output += ReferenceCollector::collect(declaration, sourceUnit, declaration->name());
	}
	else if (auto const* identifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		for (auto const* declaration: allAnnotatedDeclarations(identifier))
			output += ReferenceCollector::collect(declaration, sourceUnit, identifier->name());
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(sourceNode))
	{
		solAssert(!identifierPath->path().empty(), "");
		output += ReferenceCollector::collect(identifierPath->annotation().referencedDeclaration, sourceUnit, identifierPath->path().back());
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(sourceNode))
	{
		Type const* type = memberAccess->expression().annotation().type;
		if (auto const* ttype = dynamic_cast<TypeType const*>(type))
		{
			auto const memberName = memberAccess->memberName();

			if (auto const* enumType = dynamic_cast<EnumType const*>(ttype->actualType()))
			{
				// find the definition
				vector<DocumentHighlight> output;
				for (ASTPointer<EnumValue> const& enumMember: enumType->enumDefinition().members())
					if (enumMember->name() == memberName)
						output += ReferenceCollector::collect(enumMember.get(), sourceUnit, enumMember->name());

				// TODO: find uses of the enum value
			}
		}
		else if (auto const* structType = dynamic_cast<StructType const*>(type))
		{
			(void) structType; // TODO
			// TODO: highlight all struct member occurrences.
			// memberAccess->memberName()
			// structType->
		}
		else
		{
			// TODO: EnumType, ...
			trace("semanticHighlight: member type is: "s + (type ? typeid(*type).name() : "NULL"));
		}
	}
	return output;
}

void LanguageServer::logNotImplemented(string_view _message)
{
	if (m_trace > Trace::Off && m_logger)
		m_logger(fmt::format("Not implemented. {}", _message));
}

bool LanguageServer::run()
{
	while (!m_exitRequested && !m_client->closed())
	{
		if (optional<Json::Value> const jsonMessage = m_client->receive(); jsonMessage.has_value())
		{
			try
			{
				handleMessage(jsonMessage.value());
			}
			catch (exception const& e)
			{
				log("Unhandled exception caught when handling message. "s + e.what());
			}
		}
	}
	return m_shutdownRequested;
}

void LanguageServer::handleInitialize(MessageID _id, Json::Value const& _args)
{
	string rootPath;
	if (Json::Value uri = _args["rootUri"])
		rootPath = extractPathFromFileURI(uri.asString()).value();
	else if (Json::Value rootPath = _args["rootPath"]; rootPath)
		rootPath = rootPath.asString();

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

	m_basePath = boost::filesystem::path(rootPath);
	m_fileReader = make_unique<FileReader>(m_basePath, FileReader::FileSystemPathSet{m_basePath});
	if (_args["initializationOptions"].isObject())
		changeConfiguration(_args["initializationOptions"]);

	Json::Value replyArgs;
	replyArgs["serverInfo"]["name"] = "solc";
	replyArgs["serverInfo"]["version"] = string(VersionNumber);
	replyArgs["hoverProvider"] = true;
	replyArgs["capabilities"]["hoverProvider"] = true;
	replyArgs["capabilities"]["textDocumentSync"]["openClose"] = true;
	replyArgs["capabilities"]["textDocumentSync"]["change"] = 2; // 0=none, 1=full, 2=incremental
	replyArgs["capabilities"]["definitionProvider"] = true;
	replyArgs["capabilities"]["implementationProvider"] = true;
	replyArgs["capabilities"]["documentHighlightProvider"] = true;
	replyArgs["capabilities"]["referencesProvider"] = true;
	m_client->reply(_id, replyArgs);
}

void LanguageServer::handleWorkspaceDidChangeConfiguration(MessageID, Json::Value const& _args)
{
	if (_args["settings"].isObject())
		changeConfiguration(_args["settings"]);
}

void LanguageServer::handleExit(MessageID _id, Json::Value const& /*_args*/)
{
	m_exitRequested = true;
	Json::Value replyArgs = Json::intValue;
	replyArgs = m_shutdownRequested ? 0 : 1;
	m_client->reply(_id, replyArgs);
}

void LanguageServer::handleTextDocumentDidOpen(MessageID /*_id*/, Json::Value const& _args)
{
	if (!_args["textDocument"])
		return;

	auto const text = _args["textDocument"]["text"].asString();
	auto path = extractPathFromFileURI(_args["textDocument"]["uri"].asString()).value();
	if (boost::algorithm::starts_with(path, m_basePath.generic_string()))
		path = path.substr(m_basePath.generic_string().size());

	m_fileReader->setSource(path, text);
	compileSourceAndReport(path);
}

void LanguageServer::handleTextDocumentDidChange(MessageID /*_id*/, Json::Value const& _args)
{
	auto const path = extractPathFromFileURI(_args["textDocument"]["uri"].asString()).value();
	auto const contentChanges = _args["contentChanges"];

	for (Json::Value jsonContentChange: contentChanges)
	{
		if (!jsonContentChange.isObject()) // Protocol error, will only happen on broken clients, so silently ignore it.
			continue;

		auto file = m_fileReader->sourceCodes().find(path);
		if (file == m_fileReader->sourceCodes().end())
			continue;

		string text = jsonContentChange["text"].asString();
		if (!jsonContentChange["range"].isObject()) // full content update
		{
			m_fileReader->setSource(path, move(text));
			continue;
		}

		Json::Value const jsonRange = jsonContentChange["range"];
		int const startLine = jsonRange["start"]["line"].asInt();
		int const startColumn = jsonRange["start"]["character"].asInt();
		int const endLine = jsonRange["end"]["line"].asInt();
		int const endColumn = jsonRange["end"]["character"].asInt();

		string buffer = file->second;
		optional<int> const startOpt = CharStream::translateLineColumnToPosition(buffer, startLine, startColumn);
		optional<int> const endOpt = CharStream::translateLineColumnToPosition(buffer, endLine, endColumn);
		if (!startOpt || !endOpt)
			continue;

		size_t const start = static_cast<size_t>(startOpt.value());
		size_t const count = static_cast<size_t>(endOpt.value()) - start; // TODO: maybe off-by-1 bug? +1 missing?
		buffer.replace(start, count, move(text));
		m_fileReader->setSource(path, move(buffer));
	}

	if (!contentChanges.empty())
		compileSourceAndReport(path);
}

void LanguageServer::handleGotoDefinition(MessageID _id, Json::Value const& _args)
{
	ASTNode const* sourceNode = requestASTNode(extractDocumentPosition(_args));
	vector<SourceLocation> locations;
	if (auto const* identifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		for (auto const* declaration: allAnnotatedDeclarations(identifier))
			if (auto location = declarationPosition(declaration); location.has_value())
				locations.emplace_back(move(location.value()));
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(sourceNode))
	{
		if (auto const* declaration = identifierPath->annotation().referencedDeclaration)
			if (auto location = declarationPosition(declaration); location.has_value())
				locations.emplace_back(move(location.value()));
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(sourceNode))
	{
		auto const location = declarationPosition(memberAccess->annotation().referencedDeclaration);
		if (location.has_value())
			locations.emplace_back(location.value());
	}
	else if (auto const* importDirective = dynamic_cast<ImportDirective const*>(sourceNode))
	{
		auto const& path = *importDirective->annotation().absolutePath;
		auto const i = m_fileReader->sourceCodes().find(path);
		if (i != m_fileReader->sourceCodes().end())
			locations.emplace_back(SourceLocation{0, 0, make_shared<CharStream>("", path)});
	}
	else if (auto const* declaration = dynamic_cast<Declaration const*>(sourceNode))
	{
		if (auto location = declarationPosition(declaration); location.has_value())
			locations.emplace_back(move(location.value()));
	}
	else if (sourceNode)
	{
		log(fmt::format("Could not infer def of {}", typeid(*sourceNode).name()));
	}

	Json::Value reply = Json::arrayValue;
	for (SourceLocation const& location: locations)
		reply.append(toJson(m_basePath, location));
	m_client->reply(_id, reply);
}

string LanguageServer::symbolHoverInformation(ASTNode const* _sourceNode)
{
	if (auto const* documented = dynamic_cast<StructurallyDocumented const*>(_sourceNode))
	{
		if (documented->documentation())
			return *documented->documentation()->text();
	}
	else if (auto const* identifier = dynamic_cast<Identifier const*>(_sourceNode))
	{
		if (Type const* type = identifier->annotation().type)
			return type->toString(false);
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(_sourceNode))
	{
		Declaration const* decl = identifierPath->annotation().referencedDeclaration;
		if (decl && decl->type())
			return decl->type()->toString(false);
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(_sourceNode))
	{
		if (memberAccess->annotation().type)
			return memberAccess->annotation().type->toString(false);
	}

	return {};
}

void LanguageServer::handleTextDocumentHover(MessageID _id, Json::Value const& _args)
{
	auto const sourceNode = requestASTNode(extractDocumentPosition(_args));
	string tooltipText = symbolHoverInformation(sourceNode);
	if (!tooltipText.empty())
	{
		m_client->reply(_id, Json::nullValue);
		return;
	}

	Json::Value reply = Json::objectValue;
	reply["range"] = toJsonRange(sourceNode->location());
	reply["contents"]["kind"] = "markdown";
	reply["contents"]["value"] = move(tooltipText);
	m_client->reply(_id, reply);
}

void LanguageServer::handleTextDocumentHighlight(MessageID _id, Json::Value const& _args)
{
	auto const dpos = extractDocumentPosition(_args);
	ASTNode const* sourceNode = requestASTNode(dpos);
	Json::Value jsonReply = Json::arrayValue;
	for (DocumentHighlight const& highlight: semanticHighlight(sourceNode, dpos.path))
	{
		Json::Value item = Json::objectValue;
		item["range"] = toJsonRange(highlight.location);
		if (highlight.kind != DocumentHighlightKind::Unspecified)
			item["kind"] = int(highlight.kind);
		jsonReply.append(item);
	}
	m_client->reply(_id, jsonReply);
}

void LanguageServer::handleTextDocumentReferences(MessageID _id, Json::Value const& _args)
{
	auto const dpos = extractDocumentPosition(_args);

	auto const sourceNode = requestASTNode(dpos);
	if (!sourceNode)
	{
		Json::Value emptyResponse = Json::arrayValue;
		m_client->reply(_id, emptyResponse); // reply with "No references".
		return;
	}
	SourceUnit const& sourceUnit = m_compilerStack->ast(dpos.path);

	auto output = vector<SourceLocation>{};
	if (auto const* identifier = dynamic_cast<Identifier const*>(sourceNode))
	{
		for (auto const* declaration: allAnnotatedDeclarations(identifier))
			output += findAllReferences(declaration, declaration->name(), sourceUnit);
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(sourceNode))
	{
		if (auto decl = identifierPath->annotation().referencedDeclaration)
			output += findAllReferences(decl, decl->name(), sourceUnit);
	}
	else if (auto const* memberAccess = dynamic_cast<MemberAccess const*>(sourceNode))
	{
		output += findAllReferences(memberAccess->annotation().referencedDeclaration, memberAccess->memberName(), sourceUnit);
	}
	else if (auto const* declaration = dynamic_cast<Declaration const*>(sourceNode))
	{
		output += findAllReferences(declaration, declaration->name(), sourceUnit);
	}

	Json::Value jsonReply = Json::arrayValue;
	for (SourceLocation const& location: output)
		jsonReply.append(toJson(m_basePath, location));
	m_client->reply(_id, jsonReply);
}

void LanguageServer::log(string _message)
{
	if (m_trace >= Trace::Messages && m_logger)
		m_logger(_message);
}

void LanguageServer::trace(string const& _message)
{
	if (m_trace >= Trace::Verbose && m_logger)
		m_logger(_message);
}

void LanguageServer::handleMessage(Json::Value const& _jsonMessage)
{
	string const methodName = _jsonMessage["method"].asString();

	MessageID const id = _jsonMessage["id"].isInt() ?
		MessageID{to_string(_jsonMessage["id"].asInt())} :
		_jsonMessage["id"].isString() ?
			MessageID{_jsonMessage["id"].asString()} :
			MessageID{};

	auto const handler = m_handlers.find(methodName);
	if (handler == m_handlers.end())
		m_client->error(id, ErrorCode::MethodNotFound, "Unknown method " + methodName);
	else if (handler->second)
	{
		Json::Value const& jsonArgs = _jsonMessage["params"];
		handler->second(id, jsonArgs);
	}
}

} // namespace solidity
