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
/**
 * Formatting functions for errors referencing positions and locations in the source.
 */

#include <liblangutil/SourceReferenceFormatter.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/CharStream.h>
#include <liblangutil/CharStreamProvider.h>
#include <libsolutil/UTF8.h>
#include <iomanip>
#include <string_view>
#include <variant>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::util::formatting;

namespace
{

std::string replaceNonTabs(std::string_view _utf8Input, char _filler)
{
	std::string output;
	for (char const c: _utf8Input)
		if ((c & 0xc0) != 0x80)
			output.push_back(c == '\t' ? '\t' : _filler);
	return output;
}

}

std::string SourceReferenceFormatter::formatErrorInformation(Error const& _error, CharStream const& _charStream)
{
	return formatErrorInformation(
		_error,
		SingletonCharStreamProvider(_charStream)
	);
}

char const* SourceReferenceFormatter::errorTextColor(Error::Severity _severity)
{
	switch (_severity)
	{
	case Error::Severity::Error: return RED;
	case Error::Severity::Warning: return YELLOW;
	case Error::Severity::Info: return WHITE;
	}
	util::unreachable();
}

char const* SourceReferenceFormatter::errorHighlightColor(Error::Severity _severity)
{
	switch (_severity)
	{
	case Error::Severity::Error: return RED_BACKGROUND;
	case Error::Severity::Warning: return ORANGE_BACKGROUND_256;
	case Error::Severity::Info: return GRAY_BACKGROUND;
	}
	util::unreachable();
}

AnsiColorized SourceReferenceFormatter::normalColored() const
{
	return AnsiColorized(m_stream, m_colored, {WHITE});
}

AnsiColorized SourceReferenceFormatter::frameColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, BLUE});
}

AnsiColorized SourceReferenceFormatter::errorColored(std::ostream& _stream, bool _colored, Error::Severity _severity)
{
	return AnsiColorized(_stream, _colored, {BOLD, errorTextColor(_severity)});
}

AnsiColorized SourceReferenceFormatter::messageColored(std::ostream& _stream, bool _colored)
{
	return AnsiColorized(_stream, _colored, {BOLD, WHITE});
}

AnsiColorized SourceReferenceFormatter::secondaryColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, CYAN});
}

AnsiColorized SourceReferenceFormatter::highlightColored() const
{
	return AnsiColorized(m_stream, m_colored, {YELLOW});
}

AnsiColorized SourceReferenceFormatter::diagColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, YELLOW});
}

void SourceReferenceFormatter::printSourceLocation(SourceReference const& _ref)
{
	if (_ref.sourceName.empty())
		return; // Nothing we can print here

	if (_ref.position.line < 0)
	{
		frameColored() << "-->";
		m_stream << ' ' << _ref.sourceName << '\n';
		return; // No line available, nothing else to print
	}

	std::string line = std::to_string(_ref.position.line + 1); // one-based line number as string
	std::string leftpad = std::string(line.size(), ' ');

	// line 0: source name
	m_stream << leftpad;
	frameColored() << "-->";
	m_stream << ' ' << _ref.sourceName << ':' << line << ':' << (_ref.position.column + 1) << ":\n";

	std::string_view text = _ref.text;

	if (m_charStreamProvider.charStream(_ref.sourceName).isImportedFromAST())
		return;

	if (!_ref.multiline)
	{
		size_t const locationLength = static_cast<size_t>(_ref.endColumn - _ref.startColumn);

		// line 1:
		m_stream << leftpad << ' ';
		frameColored() << '|';
		m_stream << '\n';

		// line 2:
		frameColored() << line << " |";

		m_stream << ' ' << text.substr(0, static_cast<size_t>(_ref.startColumn));
		highlightColored() << text.substr(static_cast<size_t>(_ref.startColumn), locationLength);
		m_stream << text.substr(static_cast<size_t>(_ref.endColumn)) << '\n';

		// line 3:
		m_stream << leftpad << ' ';
		frameColored() << '|';

		m_stream << ' ' << replaceNonTabs(text.substr(0, static_cast<size_t>(_ref.startColumn)), ' ');
		diagColored() << (
			locationLength == 0 ?
			"^" :
			replaceNonTabs(text.substr(static_cast<size_t>(_ref.startColumn), locationLength), '^')
		);
		m_stream << '\n';
	}
	else
	{
		// line 1:
		m_stream << leftpad << ' ';
		frameColored() << '|';
		m_stream << '\n';

		// line 2:
		frameColored() << line << " |";
		m_stream << ' ' << text.substr(0, static_cast<size_t>(_ref.startColumn));
		highlightColored() << text.substr(static_cast<size_t>(_ref.startColumn)) << '\n';

		// line 3:
		m_stream << leftpad << ' ';
		frameColored() << '|';
		m_stream << ' ' << replaceNonTabs(text.substr(0, static_cast<size_t>(_ref.startColumn)), ' ');
		diagColored() << "^ (Relevant source part starts here and spans across multiple lines).";
		m_stream << '\n';
	}
}

void SourceReferenceFormatter::printPrimaryMessage(
	std::ostream& _stream,
	std::string _message,
	std::variant<Error::Type, Error::Severity> _typeOrSeverity,
	std::optional<ErrorId> _errorId,
	bool _colored,
	bool _withErrorIds
)
{
	errorColored(_stream, _colored, Error::errorSeverityOrType(_typeOrSeverity)) << Error::formatTypeOrSeverity(_typeOrSeverity);

	if (_withErrorIds && _errorId.has_value())
		errorColored(_stream, _colored, Error::errorSeverityOrType(_typeOrSeverity)) << " (" << _errorId.value().error << ")";

	messageColored(_stream, _colored) << ": " << _message << '\n';
}

void SourceReferenceFormatter::printExceptionInformation(SourceReferenceExtractor::Message const& _msg)
{
	printPrimaryMessage(m_stream, _msg.primary.message, _msg._typeOrSeverity, _msg.errorId, m_colored, m_withErrorIds);
	printSourceLocation(_msg.primary);

	for (auto const& secondary: _msg.secondary)
	{
		secondaryColored() << "Note";
		messageColored() << ":" << (secondary.message.empty() ? "" : (" " + secondary.message)) << '\n';
		printSourceLocation(secondary);
	}

	m_stream << '\n';
}

void SourceReferenceFormatter::printExceptionInformation(util::Exception const& _exception, Error::Type _type)
{
	printExceptionInformation(SourceReferenceExtractor::extract(m_charStreamProvider, _exception, _type));
}

void SourceReferenceFormatter::printExceptionInformation(util::Exception const& _exception, Error::Severity _severity)
{
	printExceptionInformation(SourceReferenceExtractor::extract(m_charStreamProvider, _exception, _severity));
}

void SourceReferenceFormatter::printErrorInformation(ErrorList const& _errors)
{
	for (auto const& error: _errors)
		printErrorInformation(*error);
}

void SourceReferenceFormatter::printErrorInformation(Error const& _error)
{
	SourceReferenceExtractor::Message message =
		SourceReferenceExtractor::extract(
			m_charStreamProvider,
			_error,
			Error::errorSeverity(_error.type())
		);
	printExceptionInformation(message);
}
