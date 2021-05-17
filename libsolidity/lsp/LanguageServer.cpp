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
#include <libsolidity/lsp/SemanticTokensBuilder.h>

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

using namespace solidity::lsp;
using namespace solidity::langutil;
using namespace solidity::frontend;

namespace
{

void log(string const& _message)
{
#if 0
	static ofstream logFile("/tmp/solc.lsp.log", std::ios::app);
	logFile << _message << endl;
#else
	(void)_message;
#endif
}

struct MarkdownBuilder
{
	std::stringstream result;

	MarkdownBuilder& code(std::string const& _code)
	{
		// TODO: Use solidity as language Id as soon as possible.
		auto constexpr SolidityLanguageId = "javascript";
		result << "```" << SolidityLanguageId << '\n' << _code << "\n```\n\n";
		return *this;
	}

	MarkdownBuilder& text(std::string const& _text)
	{
		if (_text.empty())
		{
			result << _text << '\n';
			if (_text.back() != '\n') // We want double-LF to ensure constructing a paragraph.
				result << '\n';
		}
		return *this;
	}
};

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

constexpr int toDiagnosticSeverity(Error::Type _errorType)
{
	// 1=Error, 2=Warning, 3=Info, 4=Hint
	switch (Error::errorSeverity(_errorType))
	{
	case Error::Severity::Error: return 1;
	case Error::Severity::Warning: return 2;
	case Error::Severity::Info: return 3;
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

Json::Value semanticTokensLegend()
{
	Json::Value legend = Json::objectValue;

	// NOTE! The (alphabetical) order and items must match exactly the items of
	//       their respective enum class members.

	Json::Value tokenTypes = Json::arrayValue;
	tokenTypes.append("class");
	tokenTypes.append("comment");
	tokenTypes.append("enum");
	tokenTypes.append("enumMember");
	tokenTypes.append("event");
	tokenTypes.append("function");
	tokenTypes.append("interface");
	tokenTypes.append("keyword");
	tokenTypes.append("macro");
	tokenTypes.append("method");
	tokenTypes.append("modifier");
	tokenTypes.append("number");
	tokenTypes.append("operator");
	tokenTypes.append("parameter");
	tokenTypes.append("property");
	tokenTypes.append("string");
	tokenTypes.append("struct");
	tokenTypes.append("type");
	tokenTypes.append("typeParameter");
	tokenTypes.append("variable");
	legend["tokenTypes"] = tokenTypes;

	Json::Value tokenModifiers = Json::arrayValue;
	tokenModifiers.append("abstract");
	tokenModifiers.append("declaration");
	tokenModifiers.append("definition");
	tokenModifiers.append("deprecated");
	tokenModifiers.append("documentation");
	tokenModifiers.append("modification");
	tokenModifiers.append("readonly");
	legend["tokenModifiers"] = tokenModifiers;

	return legend;
}

}

LanguageServer::LanguageServer(unique_ptr<Transport> _transport):
	m_client{move(_transport)},
	m_handlers{
		{"$/cancelRequest", [](auto, auto) {/*nothing for now as we are synchronous */} },
		{"cancelRequest", [](auto, auto) {/*nothing for now as we are synchronous */} },
		{"exit", [this](auto, auto) { terminate(); }},
		{"initialize", bind(&LanguageServer::handleInitialize, this, _1, _2)},
		{"initialized", [](auto, auto) {} },
		{"shutdown", [this](auto, auto) { m_shutdownRequested = true; }},
		{"textDocument/definition", [this](auto _id, auto _args) { handleGotoDefinition(_id, _args); }},
		{"textDocument/didChange", bind(&LanguageServer::handleTextDocumentDidChange, this, _1, _2)},
		{"textDocument/didClose", [](auto, auto) {/*nothing for now*/}},
		{"textDocument/didOpen", bind(&LanguageServer::handleTextDocumentDidOpen, this, _1, _2)},
		{"textDocument/documentHighlight", bind(&LanguageServer::handleTextDocumentHighlight, this, _1, _2)},
		{"textDocument/hover", bind(&LanguageServer::handleTextDocumentHover, this, _1, _2)},
		{"textDocument/implementation", [this](auto _id, auto _args) { handleGotoDefinition(_id, _args); }},
		{"textDocument/references", bind(&LanguageServer::handleTextDocumentReferences, this, _1, _2)},
		{"textDocument/semanticTokens/full", bind(&LanguageServer::semanticTokensFull, this, _1, _2)},
		{"workspace/didChangeConfiguration", bind(&LanguageServer::handleWorkspaceDidChangeConfiguration, this, _1, _2)},
	},
	m_fileReader{"/"},
	m_compilerStack{bind(&FileReader::readFile, ref(m_fileReader), _1, _2)}

{
}

DocumentPosition LanguageServer::extractDocumentPosition(Json::Value const& _json) const
{
	DocumentPosition dpos{};

	dpos.path = _json["textDocument"]["uri"].asString();
	dpos.position.line = _json["position"]["line"].asInt();
	dpos.position.column = _json["position"]["character"].asInt();

	return dpos;
}

Json::Value LanguageServer::toRange(SourceLocation const& _location) const
{
	solAssert(_location.sourceName, "");
	CharStream const& stream = m_compilerStack.charStream(*_location.sourceName);
	auto const [startLine, startColumn] = stream.translatePositionToLineColumn(_location.start);
	auto const [endLine, endColumn] = stream.translatePositionToLineColumn(_location.end);
	return toJsonRange(startLine, startColumn, endLine, endColumn);
}

Json::Value LanguageServer::toJson(SourceLocation const& _location) const
{
	solAssert(_location.sourceName);
	Json::Value item = Json::objectValue;
	item["uri"] = m_fileMappings.at(*_location.sourceName);
	item["range"] = toRange(_location);
	return item;
}

string LanguageServer::clientPathToSourceUnitName(string const& _path) const
{
	return m_fileReader.cliPathToSourceUnitName(_path);
}

bool LanguageServer::clientPathSourceKnown(string const& _path) const
{
	return m_fileReader.sourceCodes().count(clientPathToSourceUnitName(_path));
}

void LanguageServer::changeConfiguration(Json::Value const& _settings)
{
	m_settingsObject = _settings;
}

bool LanguageServer::compile(string const& _path)
{
	// TODO: optimize! do not recompile if nothing has changed (file(s) not flagged dirty).

	if (!clientPathSourceKnown(_path))
		return false;

	m_compilerStack.reset(false);
	m_compilerStack.setSources(m_fileReader.sourceCodes());
	m_compilerStack.compile(CompilerStack::State::AnalysisPerformed);

	return true;
}

void LanguageServer::compileSourceAndReport(string const& _path)
{
	compile(_path);

	Json::Value params;
	params["uri"] = _path;

	params["diagnostics"] = Json::arrayValue;
	for (shared_ptr<Error const> const& error: m_compilerStack.errors())
	{
		SourceReferenceExtractor::Message const message = SourceReferenceExtractor::extract(m_compilerStack, *error);

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
			// TODO translate back?
			jsonRelated["location"]["uri"] = secondary.sourceName;
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
	if (m_compilerStack.state() < CompilerStack::AnalysisPerformed)
		return nullptr;

	if (!clientPathSourceKnown(_filePos.path))
		return nullptr;

	string sourceUnitName = clientPathToSourceUnitName(_filePos.path);

	optional<int> sourcePos = m_compilerStack.charStream(sourceUnitName)
		.translateLineColumnToPosition(_filePos.position.line, _filePos.position.column);
	if (!sourcePos.has_value())
		return nullptr;

	return locateInnermostASTNode(*sourcePos, m_compilerStack.ast(sourceUnitName));
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

	SourceUnit const& sourceUnit = m_compilerStack.ast(clientPathToSourceUnitName(_documentPosition.path));
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

	SourceUnit const& sourceUnit = m_compilerStack.ast(clientPathToSourceUnitName(_path));

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
			//log("semanticHighlight: member type is: "s + (type ? typeid(*type).name() : "NULL"));
		}
	}
	return output;
}

bool LanguageServer::run()
{
	while (!m_exitRequested && !m_client->closed())
	{
		optional<Json::Value> const jsonMessage = m_client->receive();
		if (!jsonMessage)
			continue;

		try
		{
			string const methodName = (*jsonMessage)["method"].asString();
			MessageID const id = (*jsonMessage)["id"];

			if (auto handler = valueOrDefault(m_handlers, methodName))
				handler(id, (*jsonMessage)["params"]);
			else
				m_client->error(id, ErrorCode::MethodNotFound, "Unknown method " + methodName);
		}
		catch (exception const& e)
		{
			log("Unhandled exception caught when handling message. "s + e.what());
		}
	}
	return m_shutdownRequested;
}

void LanguageServer::handleInitialize(MessageID _id, Json::Value const& _args)
{
	// The default of FileReader is to use `.`, but the path from where the LSP was started
	// should not matter.
	string rootPath("/");
	if (Json::Value uri = _args["rootUri"])
		rootPath = uri.asString();
	else if (Json::Value rootPath = _args["rootPath"])
		rootPath = rootPath.asString();

	//log("root path: " + rootPath);
	m_fileReader.setBasePath(boost::filesystem::path(rootPath));
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
	replyArgs["capabilities"]["semanticTokensProvider"]["legend"] = semanticTokensLegend();
	replyArgs["capabilities"]["semanticTokensProvider"]["range"] = true;
	replyArgs["capabilities"]["semanticTokensProvider"]["full"] = true; // XOR requests.full.delta = true

	m_client->reply(_id, replyArgs);
}

void LanguageServer::handleWorkspaceDidChangeConfiguration(MessageID, Json::Value const& _args)
{
	if (_args["settings"].isObject())
		changeConfiguration(_args["settings"]);
}

void LanguageServer::handleTextDocumentDidOpen(MessageID /*_id*/, Json::Value const& _args)
{
	if (!_args["textDocument"])
		return;

	auto const text = _args["textDocument"]["text"].asString();
	auto uri = _args["textDocument"]["uri"].asString();
	m_fileMappings[clientPathToSourceUnitName(uri)] = uri;
	m_fileReader.setSource(uri, text);
	compileSourceAndReport(uri);
}

void LanguageServer::handleTextDocumentDidChange(MessageID /*_id*/, Json::Value const& _args)
{
	auto const uri = _args["textDocument"]["uri"].asString();
	auto const contentChanges = _args["contentChanges"];

	for (Json::Value jsonContentChange: contentChanges)
	{
		if (!jsonContentChange.isObject()) // Protocol error, will only happen on broken clients, so silently ignore it.
			continue;

		if (!clientPathSourceKnown(uri))
			// should be an error as well
			continue;

		string text = jsonContentChange["text"].asString();
		if (!jsonContentChange["range"].isObject()) // full content update
		{
			m_fileReader.setSource(uri, move(text));
			continue;
		}

		Json::Value const jsonRange = jsonContentChange["range"];
		// TODO could use a general helper to read line/characer json objects into int pairs or whateveer
		int const startLine = jsonRange["start"]["line"].asInt();
		int const startColumn = jsonRange["start"]["character"].asInt();
		int const endLine = jsonRange["end"]["line"].asInt();
		int const endColumn = jsonRange["end"]["character"].asInt();

		string buffer = m_fileReader.sourceCodes().at(clientPathToSourceUnitName(uri));
		optional<int> const startOpt = CharStream::translateLineColumnToPosition(buffer, startLine, startColumn);
		optional<int> const endOpt = CharStream::translateLineColumnToPosition(buffer, endLine, endColumn);
		if (!startOpt || !endOpt)
			continue;

		size_t const start = static_cast<size_t>(startOpt.value());
		size_t const count = static_cast<size_t>(endOpt.value()) - start; // TODO: maybe off-by-1 bug? +1 missing?
		buffer.replace(start, count, move(text));
		m_fileReader.setSource(uri, move(buffer));
	}

	if (!contentChanges.empty())
		compileSourceAndReport(uri);
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
		if (m_fileReader.sourceCodes().count(path))
			locations.emplace_back(SourceLocation{0, 0, make_shared<string const>(path)});
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
		reply.append(toJson(location));
	m_client->reply(_id, reply);
}

string LanguageServer::symbolHoverInformation(ASTNode const* _sourceNode)
{
	MarkdownBuilder markdown{};

	// Try getting the type definition of the underlying AST node, if available.
	if (auto const* expression = dynamic_cast<Expression const*>(_sourceNode))
	{
		if (expression->annotation().type)
			markdown.code(expression->annotation().type->toString(false));
		if (auto const* declaration = ASTNode::referencedDeclaration(*expression))
			if (declaration->type())
				markdown.code(declaration->type()->toString(false));
	}
	else if (auto const* declaration = dynamic_cast<Declaration const*>(_sourceNode))
	{
		if (declaration->type())
			markdown.code(declaration->type()->toString(false));
	}
	else if (auto const* identifierPath = dynamic_cast<IdentifierPath const*>(_sourceNode))
	{
		Declaration const* decl = identifierPath->annotation().referencedDeclaration;
		if (decl && decl->type())
			markdown.code(decl->type()->toString(false));
		if (auto const* node = dynamic_cast<StructurallyDocumented const*>(decl))
			if (node->documentation()->text())
				markdown.text(*node->documentation()->text());
	}
	else if (auto const* expression = dynamic_cast<Expression const*>(_sourceNode))
	{
		if (auto const* declaration = ASTNode::referencedDeclaration(*expression))
			if (declaration->type())
				markdown.code(declaration->type()->toString(false));
	}
	else
	{
		markdown.text(fmt::format("Unhandled AST node type in hover: {}\n", typeid(*_sourceNode).name()));
		log(fmt::format("Unhandled AST node type in hover: {}\n", typeid(*_sourceNode).name()));
	}

	// If this AST node contains documentation itself, append it.
	if (auto const* documented = dynamic_cast<StructurallyDocumented const*>(_sourceNode))
	{
		if (documented->documentation())
			markdown.text(*documented->documentation()->text());
	}

	return markdown.result.str();
}

void LanguageServer::handleTextDocumentHover(MessageID _id, Json::Value const& _args)
{
	auto const sourceNode = requestASTNode(extractDocumentPosition(_args));
	string tooltipText = symbolHoverInformation(sourceNode);
	if (tooltipText.empty())
	{
		m_client->reply(_id, Json::nullValue);
		return;
	}

	Json::Value reply = Json::objectValue;
	reply["range"] = toRange(sourceNode->location());
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
		item["range"] = toRange(highlight.location);
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
	string sourceUnitName = clientPathToSourceUnitName(dpos.path);
	SourceUnit const& sourceUnit = m_compilerStack.ast(sourceUnitName);

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
		jsonReply.append(toJson(location));
	log("Sending reply");
	m_client->reply(_id, jsonReply);
}

void LanguageServer::semanticTokensFull(MessageID _id, Json::Value const& _args)
{
	auto uri = _args["textDocument"]["uri"];

	if (!compile(uri.as<string>()))
		return;

	auto const sourceName = clientPathToSourceUnitName(uri.as<string>());
	SourceUnit const& ast = m_compilerStack.ast(sourceName);
	m_compilerStack.charStream(sourceName);
	SemanticTokensBuilder semanticTokensBuilder;
	Json::Value data = semanticTokensBuilder.build(ast, m_compilerStack.charStream(sourceName));

	Json::Value reply = Json::objectValue;
	reply["data"] = data;

	m_client->reply(_id, reply);
}

void LanguageServer::terminate()
{
	m_exitRequested = true;
}
