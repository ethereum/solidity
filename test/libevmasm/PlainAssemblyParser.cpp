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

#include <test/libevmasm/PlainAssemblyParser.h>

#include <test/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/Common.h>

#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string/find.hpp>

#include <fmt/format.h>

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::test;
using namespace solidity::evmasm;
using namespace solidity::evmasm::test;
using namespace solidity::langutil;

Json PlainAssemblyParser::parse(std::string _sourceName, std::string const& _source)
{
	m_sourceStream = std::istringstream(_source);
	m_sourceName = std::move(_sourceName);
	m_lineNumber = 0;

	advanceLine();
	return parseAssembly(0);
}

Json PlainAssemblyParser::parseAssembly(size_t _nestingLevel)
{
	Json assemblyJSON = {{".code", Json::array()}};
	Json& codeJSON = assemblyJSON[".code"];

	while (m_line.has_value())
	{
		if (m_lineTokens.empty())
		{
			advanceLine();
			continue;
		}

		size_t newLevel = parseNestingLevel();
		if (newLevel > _nestingLevel)
			BOOST_THROW_EXCEPTION(std::runtime_error(formatError("Indentation does not match the current subassembly nesting level.")));

		if (newLevel < _nestingLevel)
			return assemblyJSON;

		if (currentToken().value == ".sub")
		{
			advanceLine();

			std::string nextDataIndex = std::to_string(assemblyJSON[".data"].size());
			assemblyJSON[".data"][nextDataIndex] = parseAssembly(_nestingLevel + 1);
			continue;
		}
		else if (assemblyJSON.contains(".data"))
			BOOST_THROW_EXCEPTION(std::runtime_error(formatError("The code of an assembly must be specified before its subassemblies.")));

		if (c_instructions.contains(currentToken().value) || currentToken().value == "PUSHSIZE")
		{
			std::string_view name = currentToken().value;
			if (name == "SWAPN" || name == "DUPN")
			{
				if (
					std::optional<uint8_t> const depth = solidity::util::parseArithmetic<uint8_t>(expectArgument());
					depth.has_value() && isValidDupSwapNDepth(*depth)
				)
					codeJSON.push_back({{"name", name}, {"value", currentToken().value}});
				else
					BOOST_THROW_EXCEPTION(
						std::runtime_error(
							formatError(
								"Invalid " + std::string(name) + " depth: " + std::string(currentToken().value)
							)
						)
					);

				expectNoMoreArguments();
			}
			else
			{
				expectNoMoreArguments();
				codeJSON.push_back({{"name", currentToken().value}});
			}
		}
		else if (currentToken().value == "PUSH")
		{
			if (hasMoreTokens() && nextToken().value == "[tag]")
			{
				advanceToken();
				std::string_view tagID = expectArgument();
				expectNoMoreArguments();
				codeJSON.push_back({{"name", "PUSH [tag]"}, {"value", tagID}});
			}
			else if (hasMoreTokens() && (nextToken().value == "[$]" || nextToken().value == "#[$]"))
			{
				std::string pushType = std::string(nextToken().value);
				advanceToken();
				std::string_view subassemblyID = expectArgument();
				expectNoMoreArguments();

				if (!subassemblyID.starts_with("0x"))
					BOOST_THROW_EXCEPTION(std::runtime_error(formatError("The subassembly ID must be a hex number prefixed with '0x'.")));

				subassemblyID.remove_prefix("0x"s.size());
				codeJSON.push_back({{"name", "PUSH " + pushType}, {"value", subassemblyID}});
			}
			else
			{
				std::string_view immediateArgument = expectArgument();
				expectNoMoreArguments();

				if (!immediateArgument.starts_with("0x"))
					BOOST_THROW_EXCEPTION(std::runtime_error(formatError("The immediate argument to PUSH must be a hex number prefixed with '0x'.")));

				immediateArgument.remove_prefix("0x"s.size());
				codeJSON.push_back({{"name", "PUSH"}, {"value", immediateArgument}});
			}
		}
		else if (currentToken().value == "tag")
		{
			std::string_view tagID = expectArgument();
			expectNoMoreArguments();

			codeJSON.push_back({{"name", "tag"}, {"value", tagID}});
			codeJSON.push_back({{"name", "JUMPDEST"}});
		}
		else
			BOOST_THROW_EXCEPTION(std::runtime_error(formatError("Unknown instruction.")));

		advanceLine();
	}

	return assemblyJSON;
}

size_t PlainAssemblyParser::parseNestingLevel() const
{
	std::string_view indentationString = indentation();

	if (indentationString != std::string(indentationString.size(), ' '))
		BOOST_THROW_EXCEPTION(std::runtime_error(formatError("Non-space characters used for indentation.")));

	if (indentationString.size() % 4 != 0)
		BOOST_THROW_EXCEPTION(std::runtime_error(formatError("Each indentation level must consist of 4 spaces.")));

	return indentationString.size() / 4;
}

PlainAssemblyParser::Token const& PlainAssemblyParser::currentToken() const
{
	soltestAssert(m_tokenIndex < m_lineTokens.size());
	return m_lineTokens[m_tokenIndex];
}

PlainAssemblyParser::Token const& PlainAssemblyParser::nextToken() const
{
	soltestAssert(m_tokenIndex + 1 < m_lineTokens.size());
	return m_lineTokens[m_tokenIndex + 1];
}

std::string_view PlainAssemblyParser::indentation() const
{
	soltestAssert(m_line.has_value());

	if (m_lineTokens.empty())
		return *m_line;

	return std::string_view(*m_line).substr(0, m_lineTokens.at(0).position);
}

bool PlainAssemblyParser::advanceToken()
{
	if (!hasMoreTokens())
		return false;

	++m_tokenIndex;
	return true;
}

std::string_view PlainAssemblyParser::expectArgument()
{
	bool hasArgument = advanceToken();
	if (!hasArgument)
		BOOST_THROW_EXCEPTION(std::runtime_error(formatError("Missing argument(s).")));

	return currentToken().value;
}

void PlainAssemblyParser::expectNoMoreArguments()
{
	bool hasArgument = advanceToken();
	if (hasArgument)
		BOOST_THROW_EXCEPTION(std::runtime_error(formatError("Too many arguments.")));
}

bool PlainAssemblyParser::advanceLine()
{
	std::string line;
	if (!getline(m_sourceStream, line))
	{
		m_line = std::nullopt;
		return false;
	}

	++m_lineNumber;
	m_line = std::move(line);
	m_lineTokens = tokenizeLine(*m_line);
	m_tokenIndex = 0;
	return true;
}

std::vector<PlainAssemblyParser::Token> PlainAssemblyParser::tokenizeLine(std::string_view _line)
{
	auto const notWhiteSpace = [](char _c) { return !isWhiteSpace(_c); };

	std::vector<Token> tokens;
	auto tokenLocation = boost::find_token(_line, notWhiteSpace, boost::token_compress_on);
	while (!tokenLocation.empty())
	{
		std::string_view value{tokenLocation.begin(), tokenLocation.end()};
		if (value.starts_with("//"))
			break;

		tokens.push_back({
			.value = value,
			.position = static_cast<size_t>(std::distance(_line.begin(), tokenLocation.begin())),
		});
		soltestAssert(!value.empty());
		soltestAssert(tokens.back().position < _line.size());
		soltestAssert(tokens.back().position + value.size() <= _line.size());

		std::string_view tail{tokenLocation.end(), _line.end()};
		tokenLocation = boost::find_token(tail, notWhiteSpace, boost::token_compress_on);
	}

	return tokens;
}

std::string PlainAssemblyParser::formatError(std::string_view _message) const
{
	soltestAssert(m_line.has_value());
	soltestAssert(!m_lineTokens.empty());

	std::string lineNumberString = std::to_string(m_lineNumber);
	std::string padding(lineNumberString.size(), ' ');
	std::string underline = std::string(currentToken().position, ' ') + std::string(currentToken().value.size(), '^');
	return fmt::format(
		"Error while parsing plain assembly: {}\n"
		"{}--> {}\n"
		"{} | \n"
		"{} | {}\n"
		"{} | {}\n",
		_message,
		padding, m_sourceName,
		padding,
		m_lineNumber, *m_line,
		padding, underline
	);
}
