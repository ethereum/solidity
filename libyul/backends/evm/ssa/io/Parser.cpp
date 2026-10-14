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

#include <libyul/backends/evm/ssa/io/Parser.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>
#include <libyul/backends/evm/ssa/SSACFG.h>
#include <libyul/backends/evm/ssa/io/Keywords.h>

#include <libyul/Utilities.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/Common.h>
#include <liblangutil/Scanner.h>

#include <libsolutil/Numeric.h>
#include <libsolutil/StringUtils.h>
#include <libsolutil/Visitor.h>

#include <fmt/format.h>

#include <range/v3/algorithm/sort.hpp>

#include <algorithm>
#include <concepts>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <variant>
#include <vector>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::io;

namespace
{

/// Throws the `ParseError` that `io::parse` returns.
[[noreturn]] void fail(SourceRange const _location, std::string _message)
{
	throw ParseError{_location, std::move(_message)};
}

template<typename Arg, typename... Args>
[[noreturn]] void fail(
	SourceRange const _location,
	fmt::format_string<Arg, Args...> const _format,
	Arg&& _arg,
	Args&&... _args
)
{
	fail(_location, fmt::format(_format, std::forward<Arg>(_arg), std::forward<Args>(_args)...));
}

enum class TokenKind
{
	Keyword,  // e.g. `builtin`, `jump` or `preds`
	ValueRef,  // v<N>
	PhiRef,  // ^v<N>
	BlockRef,  // #<N>
	Symbol,  // @<identifier>
	Number,  // decimal integer
	HexNumber,  // 0x<hex digits>
	String,  // quoted string literal
	Equal,
	Comma,
	Colon,
	LParen,
	RParen,
	LBrace,
	RBrace,
	Arrow,
	Newline,
	EndOfInput
};

struct Token
{
	TokenKind kind;
	SourceRange location;
	/// Set for and only for tokens of kind `TokenKind::Keyword`.
	std::optional<Keyword> keyword = std::nullopt;

	std::string_view lexeme(std::string_view const _source) const
	{
		yulAssert(location.end >= location.begin);
		return _source.substr(location.begin, location.end - location.begin);
	}
};

std::string_view describe(TokenKind const _kind)
{
	switch (_kind)
	{
	case TokenKind::Keyword: return "keyword";
	case TokenKind::ValueRef: return "value reference";
	case TokenKind::PhiRef: return "phi reference";
	case TokenKind::BlockRef: return "block reference";
	case TokenKind::Symbol: return "function or builtin name";
	case TokenKind::Number: return "number";
	case TokenKind::HexNumber: return "hex number";
	case TokenKind::String: return "string literal";
	case TokenKind::Equal: return "'='";
	case TokenKind::Comma: return "','";
	case TokenKind::Colon: return "':'";
	case TokenKind::LParen: return "'('";
	case TokenKind::RParen: return "')'";
	case TokenKind::LBrace: return "'{'";
	case TokenKind::RBrace: return "'}'";
	case TokenKind::Arrow: return "'->'";
	case TokenKind::Newline: return "end of line";
	case TokenKind::EndOfInput: return "end of input";
	}
	std::unreachable();
}

bool isYulIdentifierPart(char const _c) { return langutil::isIdentifierPart(_c) || _c == '.'; }

class Tokenizer
{
public:
	explicit Tokenizer(std::string_view _source): m_source(_source) {}

	std::vector<Token> tokenize()
	{
		std::vector<Token> tokens;
		while (true)
		{
			skipWhitespace();
			if (atEnd())
			{
				tokens.push_back({.kind = TokenKind::EndOfInput, .location = {.begin = m_source.size(), .end = m_source.size()}});
				return tokens;
			}
			tokens.push_back(next());
		}
	}

private:
	bool atEnd() const { return m_position >= m_source.size(); }
	char peek() const
	{
		return m_position < m_source.size() ? m_source[m_position] : '\0';
	}

	void skipWhitespace()
	{
		skipWhile([](char const _c) { return _c != '\n' && langutil::isWhiteSpace(_c); });
	}

	template<std::predicate<char> Predicate>
	bool skipWhile(Predicate const& _predicate)
	{
		std::size_t const start = m_position;
		while (!atEnd() && _predicate(peek()))
			++m_position;
		return m_position > start;
	}

	Token next()
	{
		std::size_t const start = m_position;
		auto const make = [&](TokenKind const _kind) {
			return Token{.kind = _kind, .location = {.begin = start, .end = m_position}};
		};
		auto const failHere = [&](std::string _message) { failFrom(start, std::move(_message)); };

		char const c = peek();
		++m_position;
		switch (c)
		{
		case '\n': return make(TokenKind::Newline);
		case '=': return make(TokenKind::Equal);
		case ',': return make(TokenKind::Comma);
		case ':': return make(TokenKind::Colon);
		case '(': return make(TokenKind::LParen);
		case ')': return make(TokenKind::RParen);
		case '{': return make(TokenKind::LBrace);
		case '}': return make(TokenKind::RBrace);
		case '-':
			if (peek() != '>')
				failHere("Expected '->'.");
			++m_position;
			return make(TokenKind::Arrow);
		case '#':
			if (!skipWhile(langutil::isDecimalDigit))
				failHere("Expected digits after '#'.");
			return make(TokenKind::BlockRef);
		case '^':
			if (peek() != 'v')
				failHere("Expected '^v<N>'.");
			++m_position;
			if (!skipWhile(langutil::isDecimalDigit))
				failHere("Expected digits after '^v'.");
			return make(TokenKind::PhiRef);
		case '@':
			if (!langutil::isIdentifierStart(peek()))
				failHere("Expected an identifier after '@'.");
			skipWhile(isYulIdentifierPart);
			return make(TokenKind::Symbol);
		case '"':
			while (peek() != '"')
			{
				if (atEnd() || peek() == '\n')
					failHere("Unterminated string literal.");
				m_position += peek() == '\\' ? 2u : 1u;
			}
			++m_position;
			return make(TokenKind::String);
		default:
			break;
		}

		if (c == '0' && peek() == 'x')
		{
			++m_position;
			if (!skipWhile(langutil::isHexDigit))
				failHere("Expected hex digits after '0x'.");
			return make(TokenKind::HexNumber);
		}
		if (langutil::isDecimalDigit(c))
		{
			skipWhile(langutil::isDecimalDigit);
			return make(TokenKind::Number);
		}
		if (langutil::isIdentifierStart(c))
		{
			skipWhile(isYulIdentifierPart);
			Token token = make(TokenKind::Keyword);
			std::string_view const lexeme = token.lexeme(m_source);
			std::string_view const digits = lexeme.substr(1);
			bool const allDigits = ranges::all_of(digits, langutil::isDecimalDigit);
			if (lexeme[0] == 'v' && !digits.empty() && allDigits)
				token.kind = TokenKind::ValueRef;
			else
			{
				token.keyword = keywordFromString(lexeme);
				if (!token.keyword)
					failHere(fmt::format("Unknown keyword '{}'.", lexeme));
			}
			return token;
		}
		failFrom(start, fmt::format("Unexpected character '{}'.", c));
	}

	/// Fails with the text from `_start` to the current position
	[[noreturn]] void failFrom(std::size_t const _start, std::string _message) const
	{
		yulAssert(m_position > _start);
		fail({.begin = _start, .end = std::min(m_position, m_source.size())}, std::move(_message));
	}

	std::string_view m_source;
	std::size_t m_position = 0;
};

/// The (per graph) number of a value (`v<N>`) or block (`#<N>`) as written in the source
using Label = InstId::ValueType;
using Operand = Label;

struct ConstInst { u256 value; };
struct PhiInst {};
struct ArgInst { std::size_t index; };
struct ProjectionInst { Label producer; std::size_t index; };
struct IdentityInst { Operand forward; };
struct NopInst {};
struct MemoryGuardInst {};
struct BuiltinInst
{
	std::string_view name;
	std::vector<std::variant<Operand, std::string>> operands;  // value arguments or literals
};
struct CallInst
{
	std::string_view callee;
	std::vector<Operand> arguments;
	bool canContinue;
};
struct UpsilonInst { Operand value; Label phi; };
struct Line
{
	using Instruction = std::variant<
		ConstInst,
		PhiInst,
		ArgInst,
		ProjectionInst,
		IdentityInst,
		NopInst,
		MemoryGuardInst,
		BuiltinInst,
		CallInst,
		UpsilonInst
	>;
	SourceRange location;
	std::optional<Label> result;
	Instruction instruction;
};
struct JumpExit { Label target; };
struct BranchExit { Operand condition; Label nonZero; Label zero; };
struct ReturnExit { std::vector<Operand> values; };
struct MainExit {};
struct TerminatedExit {};
using Exit = std::variant<JumpExit, BranchExit, ReturnExit, MainExit, TerminatedExit>;
struct Block
{
	/// `#<N>:` including the predecessor list
	SourceRange headerLocation;
	Label label;
	std::optional<std::vector<Label>> predecessors;
	std::vector<Line> lines;
	SourceRange exitLocation;
	Exit exit;
};
struct FunctionHeader
{
	std::string_view name;
	std::vector<Label> arguments;
	std::size_t numReturns;
	bool canContinue;
};
struct Graph
{
	SourceRange headerLocation;  // the function header or the entry block label of the main graph
	std::vector<Block> blocks;
};
struct Function
{
	FunctionHeader header;
	Graph graph;
};
struct Module
{
	std::optional<u256> memoryGuard;
	Graph main;
	std::vector<Function> functions;
};

/// Inverts `util::escapeAndQuoteString` on a string literal token.
std::string unescape(std::string_view const _source, Token const& _token)
{
	langutil::CharStream stream(std::string(_token.lexeme(_source)), "");
	langutil::Scanner const scanner(stream, langutil::ScannerKind::Yul);
	if (scanner.currentToken() != langutil::Token::StringLiteral)
		fail(_token.location, langutil::to_string(scanner.currentError()));
	return scanner.currentLiteral();
}

class SyntaxParser
{
public:
	/// `_tokens` must end with `EndOfInput` and refer to `_source`.
	SyntaxParser(std::string_view const _source, std::span<Token const> const _tokens):
		m_source(_source),
		m_tokens(_tokens)
	{}

	Module parseModule()
	{
		Module module;
		skipNewlines();
		if (current().keyword == Keyword::MemoryGuard)
		{
			advance();
			expect(TokenKind::Equal);
			module.memoryGuard = parseWord(expect(TokenKind::HexNumber));
			expectEndOfLine();
		}
		if (current().kind != TokenKind::BlockRef)
			failAtCurrent("Expected the entry block of the main graph.");
		module.main = Graph{.headerLocation = current().location, .blocks = parseBlocks()};
		while (current().keyword == Keyword::Func)
			module.functions.push_back(parseFunction());
		if (current().kind != TokenKind::EndOfInput)
			failAtCurrent("Expected 'func' or end of input.");
		return module;
	}

private:
	Token const& current() const { return m_tokens[m_position]; }

	Token const& advance()
	{
		Token const& token = m_tokens[m_position];
		if (token.kind != TokenKind::EndOfInput)
			++m_position;
		return token;
	}

	/// The source from `_begin` to the end of the last consumed token.
	SourceRange locationFrom(std::size_t const _begin) const
	{
		return {.begin = _begin, .end = m_tokens[m_position - 1].location.end};
	}

	[[noreturn]] void failAtCurrent(std::string const& _message) const
	{
		Token const& token = current();
		std::string const found =
			token.kind == TokenKind::Newline || token.kind == TokenKind::EndOfInput ?
			std::string(describe(token.kind)) :
			fmt::format("{} '{}'", describe(token.kind), token.lexeme(m_source));
		fail(token.location, "{} Found {}.", _message, found);
	}

	Token const& expect(TokenKind const _kind)
	{
		if (current().kind != _kind)
			failAtCurrent(fmt::format("Expected {}.", describe(_kind)));
		return advance();
	}

	void expectWord(Keyword const _word)
	{
		if (current().keyword != _word)
			failAtCurrent(fmt::format("Expected '{}'.", _word));
		advance();
	}

	bool acceptWord(Keyword const _word)
	{
		if (current().keyword != _word)
			return false;
		advance();
		return true;
	}

	void skipNewlines()
	{
		while (current().kind == TokenKind::Newline)
			advance();
	}

	void expectEndOfLine()
	{
		if (current().kind != TokenKind::EndOfInput)
			expect(TokenKind::Newline);
		skipNewlines();
	}

	/// Parses the decimal number in `_token`, skipping the first `_prefixLength` characters.
	template<typename T>
	T parseInteger(Token const& _token, std::size_t const _prefixLength = 0) const
	{
		std::string_view const digits = _token.lexeme(m_source).substr(_prefixLength);
		std::optional<T> const value = solidity::util::parseArithmetic<T>(digits);
		if (!value)
			fail(_token.location, "Number '{}' couldn't be parsed.", digits);
		return *value;
	}

	u256 parseWord(Token const& _token) const
	{
		std::string const text(_token.lexeme(m_source));
		if (bigint(text) > std::numeric_limits<u256>::max())
			fail(_token.location, "Number '{}' does not fit into 256 bits.", text);
		return u256(text);
	}

	/// Parses `v<N>`, `^v<N>`, or `#<N>` into `<N>`.
	Label parseLabel(TokenKind const _kind)
	{
		return parseInteger<Label>(expect(_kind), _kind == TokenKind::PhiRef ? 2 : 1);
	}

	std::vector<Label> parseLabelList(TokenKind const _kind)
	{
		std::vector labels{parseLabel(_kind)};
		while (current().kind == TokenKind::Comma)
		{
			advance();
			labels.push_back(parseLabel(_kind));
		}
		return labels;
	}

	bool atOperand() const
	{
		return current().kind == TokenKind::ValueRef || current().keyword == Keyword::Unreachable;
	}

	Operand parseOperand()
	{
		if (current().keyword == Keyword::Unreachable)
			failAtCurrent("Unreachable values are never referenced.");
		return parseLabel(TokenKind::ValueRef);
	}

	std::vector<Operand> parseOperandList()
	{
		std::vector operands{parseOperand()};
		while (current().kind == TokenKind::Comma)
		{
			advance();
			operands.push_back(parseOperand());
		}
		return operands;
	}

	/// Parses `@<name>` into `<name>`.
	std::string_view parseSymbol() { return expect(TokenKind::Symbol).lexeme(m_source).substr(1); }

	Function parseFunction()
	{
		std::size_t const begin = current().location.begin;
		expectWord(Keyword::Func);
		FunctionHeader header;
		header.name = parseSymbol();
		expect(TokenKind::LParen);
		expectWord(Keyword::Args);
		expect(TokenKind::Colon);
		expect(TokenKind::LParen);
		if (current().kind == TokenKind::ValueRef)
			header.arguments = parseLabelList(TokenKind::ValueRef);
		expect(TokenKind::RParen);
		expect(TokenKind::RParen);
		expect(TokenKind::Arrow);
		header.numReturns = parseInteger<std::size_t>(expect(TokenKind::Number));
		header.canContinue = !acceptWord(Keyword::NoContinue);
		expect(TokenKind::LBrace);
		SourceRange const headerLocation = locationFrom(begin);
		expectEndOfLine();
		if (current().kind != TokenKind::BlockRef)
			failAtCurrent("Expected the entry block of the function.");
		std::vector<Block> blocks = parseBlocks();
		expect(TokenKind::RBrace);
		expectEndOfLine();
		return Function{std::move(header), Graph{headerLocation, std::move(blocks)}};
	}

	std::vector<Block> parseBlocks()
	{
		std::vector<Block> blocks;
		while (current().kind == TokenKind::BlockRef)
			blocks.push_back(parseBlock());
		return blocks;
	}

	Block parseBlock()
	{
		Block block;
		std::size_t const begin = current().location.begin;
		block.label = parseLabel(TokenKind::BlockRef);
		expect(TokenKind::Colon);
		if (acceptWord(Keyword::Preds))
		{
			expect(TokenKind::Colon);
			block.predecessors = parseLabelList(TokenKind::BlockRef);
		}
		block.headerLocation = locationFrom(begin);
		expectEndOfLine();
		while (!atExit())
			block.lines.push_back(parseLine());
		std::size_t const exitBegin = current().location.begin;
		block.exit = parseExit();
		block.exitLocation = locationFrom(exitBegin);
		expectEndOfLine();
		return block;
	}

	bool atExit() const
	{
		return
			current().keyword == Keyword::Jump ||
			current().keyword == Keyword::Branch ||
			current().keyword == Keyword::Return ||
			current().keyword == Keyword::MainExit ||
			current().keyword == Keyword::Terminated;
	}

	Line parseLine()
	{
		std::size_t const begin = current().location.begin;
		Line line;
		if (current().kind == TokenKind::ValueRef)
		{
			line.result = parseLabel(TokenKind::ValueRef);
			expect(TokenKind::Equal);
			line.instruction = parseValueInstruction();
		}
		else if (current().keyword == Keyword::Builtin)
			line.instruction = parseBuiltin();
		else if (current().keyword == Keyword::Call)
			line.instruction = parseCall();
		else if (acceptWord(Keyword::Upsilon))
		{
			Operand const value = parseOperand();
			expect(TokenKind::Arrow);
			line.instruction = UpsilonInst{value, parseLabel(TokenKind::PhiRef)};
		}
		else
			failAtCurrent("Expected an instruction or a block exit.");
		line.location = locationFrom(begin);
		expectEndOfLine();
		return line;
	}

	Line::Instruction parseValueInstruction()
	{
		if (current().keyword == Keyword::Builtin)
			return parseBuiltin();
		if (current().keyword == Keyword::Call)
			return parseCall();
		if (acceptWord(Keyword::Const))
			return ConstInst{parseWord(expect(TokenKind::HexNumber))};
		if (acceptWord(Keyword::Phi))
			return PhiInst{};
		if (acceptWord(Keyword::Arg))
			return ArgInst{parseInteger<std::size_t>(expect(TokenKind::Number))};
		if (acceptWord(Keyword::Proj))
		{
			Label const producer = parseLabel(TokenKind::ValueRef);
			expect(TokenKind::Comma);
			return ProjectionInst{producer, parseInteger<std::size_t>(expect(TokenKind::Number))};
		}
		if (acceptWord(Keyword::Identity))
			return IdentityInst{parseOperand()};
		if (acceptWord(Keyword::Nop))
			return NopInst{};
		if (acceptWord(Keyword::MemoryGuard))
			return MemoryGuardInst{};
		if (current().keyword == Keyword::Unreachable)
			failAtCurrent("Unreachables should never be in a block.");
		if (current().keyword == Keyword::Tombstone)
			failAtCurrent("Tombstones should never be instantiated.");
		failAtCurrent("Expected an instruction.");
	}

	BuiltinInst parseBuiltin()
	{
		expectWord(Keyword::Builtin);
		BuiltinInst builtin{.name = parseSymbol(), .operands = {}};
		if (current().kind == TokenKind::Newline || current().kind == TokenKind::EndOfInput)
			return builtin;
		while (true)
		{
			if (atOperand())
				builtin.operands.emplace_back(parseOperand());
			else if (current().kind == TokenKind::String)
				builtin.operands.emplace_back(unescape(m_source, advance()));
			else
				failAtCurrent("Expected a value reference or a string literal.");
			if (current().kind != TokenKind::Comma)
				return builtin;
			advance();
		}
	}

	CallInst parseCall()
	{
		expectWord(Keyword::Call);
		CallInst call{parseSymbol(), {}, true};
		if (atOperand())
			call.arguments = parseOperandList();
		if (current().kind == TokenKind::LBrace)
		{
			advance();
			expectWord(Keyword::NoContinue);
			expect(TokenKind::Equal);
			expectWord(Keyword::True);
			expect(TokenKind::RBrace);
			call.canContinue = false;
		}
		return call;
	}

	Exit parseExit()
	{
		if (acceptWord(Keyword::Jump))
			return JumpExit{parseLabel(TokenKind::BlockRef)};
		if (acceptWord(Keyword::Branch))
		{
			BranchExit branch;
			branch.condition = parseOperand();
			expect(TokenKind::Comma);
			branch.nonZero = parseLabel(TokenKind::BlockRef);
			expect(TokenKind::Comma);
			branch.zero = parseLabel(TokenKind::BlockRef);
			return branch;
		}
		if (acceptWord(Keyword::Return))
		{
			ReturnExit ret;
			if (atOperand())
				ret.values = parseOperandList();
			return ret;
		}
		if (acceptWord(Keyword::MainExit))
			return MainExit{};
		expectWord(Keyword::Terminated);
		return TerminatedExit{};
	}

	std::string_view m_source;
	std::span<Token const> m_tokens;
	std::size_t m_position = 0;
};

struct Callee
{
	FunctionGraphID id;
	FunctionHeader const& header;
};
using Callees = std::map<std::string, Callee, std::less<>>;

class GraphBuilder
{
public:
	GraphBuilder(
		ControlFlowGraphs const& _cfgs,
		Callees const& _callees,
		Graph const& _graph,
		FunctionHeader const* _function,
		SSACFG& _cfg
	):
		m_cfgs(_cfgs),
		m_callees(_callees),
		m_graph(_graph),
		m_function(_function),
		m_cfg(_cfg)
	{}

	void build()
	{
		createBlocks();
		allocateInstructions();
		bindArguments();
		resolveInstructions();
		resolveExits();
		connectBlocks();
	}

private:
	void createBlocks()
	{
		for (Block const& block: m_graph.blocks)
			if (!m_blocks.emplace(block.label, m_cfg.makeBlock(nullptr)).second)
				fail(block.headerLocation, "Duplicate block #{}.", block.label);
		m_cfg.entry = m_blocks.at(m_graph.blocks.front().label);
	}

	void allocateInstructions()
	{
		if (m_function)
			m_argumentValues.resize(m_function->arguments.size());
		for (std::size_t blockIndex = 0; blockIndex < m_graph.blocks.size(); ++blockIndex)
		{
			BlockId const blockId{static_cast<BlockId::ValueType>(blockIndex)};
			std::vector<InstId>& schedule = m_schedule.emplace_back();
			for (Line const& line: m_graph.blocks[blockIndex].lines)
			{
				InstId const id = allocate(line, blockId);
				if (line.result && !m_values.emplace(*line.result, id).second)
					fail(line.location, "Duplicate definition of v{}.", *line.result);
				m_scheduled.insert(id);
				schedule.push_back(id);
			}
		}
	}

	/// Allocates the instruction of `_line` in `_block`.
	InstId allocate(Line const& _line, BlockId const _block)
	{
		return std::visit(GenericVisitor{
			[&](ConstInst const& _const)
			{
				if (_block != m_cfg.entry)
					fail(_line.location, "Constants must be defined in the entry block.");
				InstId const id = m_cfg.newLiteral(nullptr, _const.value);
				if (m_scheduled.contains(id))
				{
					std::string const value = toCompactHexWithPrefix(_const.value);
					fail(_line.location, "Duplicate constant {}.", value);
				}
				return id;
			},
			[&](PhiInst const&)
			{
				InstId const id = m_cfg.newPhi(_block);
				m_phis.insert(id);
				return id;
			},
			[&](ArgInst const& _arg)
			{
				if (!m_function)
					fail(_line.location, "Function arguments are not allowed in the main graph.");
				if (_block != m_cfg.entry)
					fail(_line.location, "Function arguments must be defined in the entry block.");
				if (_arg.index >= m_argumentValues.size())
					fail(
						_line.location,
						"Argument index {} is out of range for a function with {} arguments.",
						_arg.index,
						m_argumentValues.size()
					);
				if (m_argumentValues[_arg.index])
					fail(_line.location, "Duplicate definition of argument {}.", _arg.index);
				InstId const id = m_cfg.newFunctionArgument();
				m_argumentValues[_arg.index] = id;
				return id;
			},
			[&](ProjectionInst const& _projection)
			{
				auto const producerIt = m_values.find(_projection.producer);
				if (producerIt == m_values.end())
					fail(
						_line.location,
						"Projection of v{} must follow it.",
						_projection.producer
					);
				InstId const producer = producerIt->second;
				if (!m_cfg.isOperation(producer) || m_cfg.numReturnsOf(producer) < 2)
					fail(
						_line.location,
						"v{} does not have multiple returns.",
						_projection.producer
					);
				if (_projection.index >= m_cfg.numReturnsOf(producer))
					fail(
						_line.location,
						"Projection index {} is out of range for v{} with {} returns.",
						_projection.index,
						_projection.producer,
						m_cfg.numReturnsOf(producer)
					);
				if (m_cfg.inst(producer).block != _block)
					fail(
						_line.location,
						"Projections must be in the same block as their producer."
					);
				auto const offset = static_cast<InstId::ValueType>(1 + _projection.index);
				InstId const id{producer.value + offset};
				if (m_scheduled.contains(id))
					fail(
						_line.location,
						"Duplicate projection {} of v{}.",
						_projection.index,
						_projection.producer
					);
				return id;
			},
			[&](IdentityInst const&)
			{
				// Placeholder, turned into an identity once its input is known.
				return m_cfg.newPhi(_block);
			},
			[&](NopInst const&)
			{
				InstId const id = m_cfg.newPhi(_block);
				m_cfg.replaceWithNop(id);
				return id;
			},
			[&](MemoryGuardInst const&)
			{
				if (!m_cfgs.memoryGuard)
					fail(_line.location, "memoryguard requires a 'memoryguard = <value>' header.");
				return m_cfg.makeMemoryGuard(_block);
			},
			[&](BuiltinInst const& _builtin)
			{
				if (_builtin.name == "memoryguard")
					fail(_line.location, "memoryguard must be written as 'v<N> = memoryguard'.");
				auto const handle = m_cfg.evmDialect.findBuiltin(_builtin.name);
				if (!handle)
					fail(_line.location, "Unknown builtin @{}.", _builtin.name);
				auto const& builtin = m_cfg.evmDialect.builtin(*handle);
				if (_builtin.operands.size() != builtin.numParameters)
					fail(
						_line.location,
						"@{} expects {} arguments, but {} were given.",
						_builtin.name,
						builtin.numParameters,
						_builtin.operands.size()
					);
				std::vector<Literal> literalArguments;
				std::size_t numValueArguments = 0;
				for (std::size_t i = 0; i < _builtin.operands.size(); ++i)
				{
					std::optional<LiteralKind> const literalKind = builtin.literalArgument(i);
					std::string const* literal = std::get_if<std::string>(&_builtin.operands[i]);
					if (literalKind.has_value() != (literal != nullptr))
						fail(
							_line.location,
							"Argument {} of @{} must be a {}.",
							i,
							_builtin.name,
							literalKind ? "string literal" : "value"
						);
					if (literal)
					{
						yulAssert(*literalKind == LiteralKind::String);
						literalArguments.push_back(Literal{
							nullptr,
							LiteralKind::String,
							valueOfBuiltinStringLiteralArgument(*literal)
						});
					}
					else
						++numValueArguments;
				}
				checkCallResult(_line, builtin.numReturns, fmt::format("@{}", _builtin.name));
				return m_cfg.makeBuiltinCallWithProjections(
					_block,
					SSACFG::BuiltinCall{*handle, std::move(literalArguments)},
					std::vector<InstId>(numValueArguments),
					static_cast<InstructionStore::NumReturnsSizeType>(builtin.numReturns)
				);
			},
			[&](CallInst const& _call)
			{
				auto const calleeIt = m_callees.find(_call.callee);
				if (calleeIt == m_callees.end())
					fail(_line.location, "Call to undefined function @{}.", _call.callee);
				FunctionHeader const& callee = calleeIt->second.header;
				if (_call.arguments.size() != callee.arguments.size())
					fail(
						_line.location,
						"@{} expects {} arguments, but {} were given.",
						_call.callee,
						callee.arguments.size(),
						_call.arguments.size()
					);
				if (_call.canContinue != callee.canContinue)
					fail(
						_line.location,
						"Call to @{} disagrees with its definition on whether it can continue.",
						_call.callee
					);
				checkCallResult(_line, callee.numReturns, fmt::format("@{}", _call.callee));
				return m_cfg.makeCallWithProjections(
					_block,
					SSACFG::Call{calleeIt->second.id, _call.canContinue, callee.numReturns},
					std::vector<InstId>(_call.arguments.size()),
					static_cast<InstructionStore::NumReturnsSizeType>(callee.numReturns)
				);
			},
			[&](UpsilonInst const&)
			{
				// Placeholder, turned into an upsilon once its input and phi are known.
				return m_cfg.unreachableValue();
			}
		}, _line.instruction);
	}

	static void checkCallResult(
		Line const& _line,
		std::size_t const _numReturns,
		std::string const& _what
	)
	{
		if (_numReturns == 0 && _line.result)
			fail(
				_line.location,
				"{} returns nothing, but is assigned to v{}.",
				_what,
				*_line.result
			);
		if (_numReturns > 0 && !_line.result)
			fail(_line.location, "The result of {} must be assigned to a value.", _what);
	}

	void bindArguments()
	{
		if (!m_function)
			return;
		std::vector<Label> const& arguments = m_function->arguments;
		for (std::size_t i = 0; i < arguments.size(); ++i)
		{
			if (!m_argumentValues[i])
				fail(m_graph.headerLocation, "Argument {} is not defined in the entry block.", i);
			auto const it = m_values.find(arguments[i]);
			if (it == m_values.end() || it->second != *m_argumentValues[i])
				fail(
					m_graph.headerLocation,
					"Argument {} is listed as v{}, which is not 'arg {}'.",
					i,
					arguments[i],
					i
				);
			m_cfg.arguments.push_back(*m_argumentValues[i]);
		}
	}

	void resolveInstructions()
	{
		for (std::size_t blockIndex = 0; blockIndex < m_graph.blocks.size(); ++blockIndex)
		{
			BlockId const blockId{static_cast<BlockId::ValueType>(blockIndex)};
			std::vector<Line> const& lines = m_graph.blocks[blockIndex].lines;
			for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
			{
				Line const& line = lines[lineIndex];
				InstId const id = m_schedule[blockIndex][lineIndex];
				std::visit(GenericVisitor{
					[&](BuiltinInst const& _builtin)
					{
						std::size_t input = 0;
						for (auto const& operand: _builtin.operands)
							if (Operand const* value = std::get_if<Operand>(&operand))
								m_cfg.inst(id).inputs[input++] = resolve(*value, line.location);
					},
					[&](CallInst const& _call)
					{
						for (std::size_t i = 0; i < _call.arguments.size(); ++i)
							m_cfg.inst(id).inputs[i] = resolve(_call.arguments[i], line.location);
					},
					[&](IdentityInst const& _identity)
					{
						if (_identity.forward == *line.result)
							fail(line.location, "v{} cannot forward to itself.", *line.result);
						m_cfg.replaceWithIdentity(id, resolve(_identity.forward, line.location));
					},
					[&](UpsilonInst const& _upsilon)
					{
						auto const phiIt = m_values.find(_upsilon.phi);
						if (phiIt == m_values.end() || !m_phis.contains(phiIt->second))
							fail(line.location, "Upsilon target v{} is not a phi.", _upsilon.phi);
						InstId const value = resolve(_upsilon.value, line.location);
						m_cfg.replaceWithUpsilon(id, blockId, value, phiIt->second);
					},
					[](auto const&) {}
				}, line.instruction);
			}
		}
	}

	void resolveExits()
	{
		for (std::size_t blockIndex = 0; blockIndex < m_graph.blocks.size(); ++blockIndex)
		{
			Block const& block = m_graph.blocks[blockIndex];
			BlockId const blockId{static_cast<BlockId::ValueType>(blockIndex)};
			SSACFG::BasicBlock& basicBlock = m_cfg.block(blockId);
			basicBlock.exit = std::visit(GenericVisitor{
				[&](JumpExit const& _jump) -> decltype(basicBlock.exit)
				{
					return SSACFG::BasicBlock::Jump{blockOf(_jump.target, block.exitLocation)};
				},
				[&](BranchExit const& _branch) -> decltype(basicBlock.exit)
				{
					return SSACFG::BasicBlock::ConditionalJump{
						resolve(_branch.condition, block.exitLocation),
						blockOf(_branch.nonZero, block.exitLocation),
						blockOf(_branch.zero, block.exitLocation)
					};
				},
				[&](ReturnExit const& _return) -> decltype(basicBlock.exit)
				{
					if (!m_function)
						fail(block.exitLocation, "'return' is not allowed in the main graph.");
					if (_return.values.size() != m_function->numReturns)
						fail(
							block.exitLocation,
							"@{} returns {} values, but {} were given.",
							m_function->name,
							m_function->numReturns,
							_return.values.size()
						);
					SSACFG::BasicBlock::FunctionReturn functionReturn;
					for (Operand const value: _return.values)
						functionReturn.returnValues.push_back(resolve(value, block.exitLocation));
					return functionReturn;
				},
				[&](MainExit const&) -> decltype(basicBlock.exit)
				{
					if (m_function)
						fail(block.exitLocation, "'main_exit' is only allowed in the main graph.");
					return SSACFG::BasicBlock::MainExit{};
				},
				[&](TerminatedExit const&) -> decltype(basicBlock.exit)
				{
					return SSACFG::BasicBlock::Terminated{};
				}
			}, block.exit);
			basicBlock.instructions = m_schedule[blockIndex];
		}
	}

	/// Assigns the predecessors of each block. Printed predecessor lists are kept in their order
	/// (which is otherwise not recoverable) after checking them against the jumps.
	void connectBlocks()
	{
		std::vector<std::vector<BlockId>> jumpsInto(m_graph.blocks.size());
		for (std::size_t blockIndex = 0; blockIndex < m_graph.blocks.size(); ++blockIndex)
		{
			BlockId const blockId{static_cast<BlockId::ValueType>(blockIndex)};
			m_cfg.block(blockId).forEachExit([&](BlockId const _successor) {
				jumpsInto[_successor.value].push_back(blockId);
			});
		}
		if (!jumpsInto[m_cfg.entry.value].empty())
			fail(
				m_graph.blocks.front().headerLocation,
				"The entry block must not have predecessors."
			);
		for (std::size_t blockIndex = 0; blockIndex < m_graph.blocks.size(); ++blockIndex)
		{
			Block const& block = m_graph.blocks[blockIndex];
			BlockId const blockId{static_cast<BlockId::ValueType>(blockIndex)};
			std::vector<BlockId>& entries = m_cfg.block(blockId).entries;
			if (!block.predecessors)
			{
				if (!jumpsInto[blockIndex].empty())
					fail(
						block.headerLocation,
						"#{} is jumped to from #{}, but declares no predecessors.",
						block.label,
						m_graph.blocks[jumpsInto[blockIndex].front().value].label
					);
				continue;
			}
			for (Label const predecessor: *block.predecessors)
				entries.push_back(blockOf(predecessor, block.headerLocation));
			std::vector<BlockId> declared = entries;
			ranges::sort(declared);
			if (declared != jumpsInto[blockIndex])
				fail(
					block.headerLocation,
					"Predecessors of #{} differ from jumps to it.",
					block.label
				);
		}
	}

	BlockId blockOf(Label const _label, SourceRange const _location) const
	{
		auto const it = m_blocks.find(_label);
		if (it == m_blocks.end())
			fail(_location, "Reference to undefined block #{}.", _label);
		return it->second;
	}

	InstId resolve(Operand const _operand, SourceRange const _location)
	{
		auto const it = m_values.find(_operand);
		if (it == m_values.end())
			fail(_location, "Reference to undefined value v{}.", _operand);
		if (m_cfg.isOperation(it->second) && m_cfg.numReturnsOf(it->second) >= 2)
			fail(_location, "v{} has multiple returns and must be referenced through 'proj'.", _operand);
		return it->second;
	}

	ControlFlowGraphs const& m_cfgs;
	Callees const& m_callees;
	Graph const& m_graph;
	FunctionHeader const* m_function;
	SSACFG& m_cfg;

	std::map<Label, BlockId> m_blocks;
	std::map<Label, InstId> m_values;
	std::set<InstId> m_phis;
	std::set<InstId> m_scheduled;
	std::vector<std::vector<InstId>> m_schedule;
	std::vector<std::optional<InstId>> m_argumentValues;
};

std::unique_ptr<ControlFlowGraphs> buildGraphs(Module const& _module, EVMDialect const& _dialect)
{
	auto cfgs = std::make_unique<ControlFlowGraphs>();
	cfgs->memoryGuard = _module.memoryGuard;
	cfgs->functionGraphs.emplace_back(std::make_unique<SSACFG>(_dialect));

	Callees callees;
	for (std::size_t functionIndex = 0; functionIndex < _module.functions.size(); ++functionIndex)
	{
		Function const& function = _module.functions[functionIndex];
		SSACFG& cfg = *cfgs->functionGraphs.emplace_back(std::make_unique<SSACFG>(_dialect));
		using NumReturnsSizeType = InstructionStore::NumReturnsSizeType;
		if (function.header.numReturns > std::numeric_limits<NumReturnsSizeType>::max())
			fail(
				function.graph.headerLocation,
				"@{} has too many return values.",
				function.header.name
			);
		auto const graphId = static_cast<FunctionGraphID>(functionIndex + 1);
		Callee const callee{graphId, function.header};
		if (!callees.emplace(std::string(function.header.name), callee).second)
			fail(
				function.graph.headerLocation,
				"Duplicate function @{}. Names must be unique (stem from a disambiguated AST).",
				function.header.name
			);
		cfg.name = std::string(function.header.name);
		cfg.numReturns = function.header.numReturns;
		cfg.canContinue = function.header.canContinue;
	}

	GraphBuilder(*cfgs, callees, _module.main, nullptr, *cfgs->functionGraphs.front()).build();
	for (std::size_t functionIndex = 0; functionIndex < _module.functions.size(); ++functionIndex)
	{
		Function const& function = _module.functions[functionIndex];
		SSACFG& cfg = *cfgs->functionGraphs[functionIndex + 1];
		GraphBuilder(*cfgs, callees, function.graph, &function.header, cfg).build();
	}
	return cfgs;
}

}

std::expected<std::unique_ptr<ControlFlowGraphs>, ParseError> io::parse(
	std::string_view const _source,
	EVMDialect const& _dialect
)
{
	try
	{
		std::vector<Token> const tokens = Tokenizer(_source).tokenize();
		Module const module = SyntaxParser(_source, tokens).parseModule();
		return buildGraphs(module, _dialect);
	}
	catch (ParseError& _error)
	{
		return std::unexpected(std::move(_error));
	}
}
