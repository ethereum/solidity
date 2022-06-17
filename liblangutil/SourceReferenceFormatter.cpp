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

using namespace std;
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

AnsiColorized SourceReferenceFormatter::normalColored() const
{
	return AnsiColorized(m_stream, m_colored, {WHITE});
}

AnsiColorized SourceReferenceFormatter::frameColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, BLUE});
}

AnsiColorized SourceReferenceFormatter::errorColored(Error::Severity _severity) const
{
	// We used to color messages of any severity as errors so this seems like a good default
	// for cases where severity cannot be determined.
	char const* textColor = RED;

	switch (_severity)
	{
	case Error::Severity::Error: textColor = RED; break;
	case Error::Severity::Warning: textColor = YELLOW; break;
	case Error::Severity::Info: textColor = WHITE; break;
	}

	return AnsiColorized(m_stream, m_colored, {BOLD, textColor});
}

AnsiColorized SourceReferenceFormatter::messageColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, WHITE});
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

	string line = std::to_string(_ref.position.line + 1); // one-based line number as string
	string leftpad = string(line.size(), ' ');

	// line 0: source name
	m_stream << leftpad;
	frameColored() << "-->";
	m_stream << ' ' << _ref.sourceName << ':' << line << ':' << (_ref.position.column + 1) << ":\n";

	string_view text = _ref.text;

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

void SourceReferenceFormatter::printExceptionInformation(SourceReferenceExtractor::Message const& _msg, bool _printFullType)
{
	errorColored(Error::errorSeverity(_msg.type)) << (
		_printFullType ?
		Error::formatErrorType(_msg.type) :
		Error::formatErrorSeverity(Error::errorSeverity(_msg.type))
	);

	if (m_withErrorIds && _msg.errorId.has_value())
		errorColored(Error::errorSeverity(_msg.type)) << " (" << _msg.errorId.value().error << ")";

	messageColored() << ": " << _msg.primary.message << '\n';
	printSourceLocation(_msg.primary);

	for (auto const& secondary: _msg.secondary)
	{
		secondaryColored() << "Note";
		messageColored() << ":" << (secondary.message.empty() ? "" : (" " + secondary.message)) << '\n';
		printSourceLocation(secondary);
	}

	m_stream << '\n';
}

void SourceReferenceFormatter::printExceptionInformation(util::Exception const& _exception, Error::Type _type, bool _printFullType)
{
	printExceptionInformation(SourceReferenceExtractor::extract(m_charStreamProvider, _exception, _type), _printFullType);
}

void SourceReferenceFormatter::printErrorInformation(ErrorList const& _errors)
{
	for (auto const& error: _errors)
		printErrorInformation(*error);
}

void SourceReferenceFormatter::printErrorInformation(Error const& _error)
{
	printExceptionInformation(SourceReferenceExtractor::extract(m_charStreamProvider, _error));
}
