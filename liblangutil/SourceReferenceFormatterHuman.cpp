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
/**
 * Formatting functions for errors referencing positions and locations in the source.
 */

#include <liblangutil/SourceReferenceFormatterHuman.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/Exceptions.h>
#include <cmath>
#include <iomanip>

using namespace std;
using namespace dev;
using namespace dev::formatting;
using namespace langutil;

AnsiColorized SourceReferenceFormatterHuman::normalColored() const
{
	return AnsiColorized(m_stream, m_colored, {WHITE});
}

AnsiColorized SourceReferenceFormatterHuman::frameColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, BLUE});
}

AnsiColorized SourceReferenceFormatterHuman::errorColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, RED});
}

AnsiColorized SourceReferenceFormatterHuman::messageColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, WHITE});
}

AnsiColorized SourceReferenceFormatterHuman::secondaryColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, CYAN});
}

AnsiColorized SourceReferenceFormatterHuman::highlightColored() const
{
	return AnsiColorized(m_stream, m_colored, {YELLOW});
}

AnsiColorized SourceReferenceFormatterHuman::diagColored() const
{
	return AnsiColorized(m_stream, m_colored, {BOLD, YELLOW});
}

void SourceReferenceFormatterHuman::printSourceLocation(SourceReference const& _ref)
{
	if (_ref.position.line < 0)
		return; // Nothing we can print here

	int const leftpad = static_cast<int>(log10(max(_ref.position.line, 1))) + 1;

	// line 0: source name
	frameColored() << string(leftpad, ' ') << "--> ";
	m_stream << _ref.sourceName << ":" << (_ref.position.line + 1) << ":" << (_ref.position.column + 1) << ":" << '\n';

	if (!_ref.multiline)
	{
		int const locationLength = _ref.endColumn - _ref.startColumn;

		// line 1:
		m_stream << string(leftpad, ' ');
		frameColored() << " |" << '\n';

		// line 2:
		frameColored() << (_ref.position.line + 1) << " | ";
		m_stream << _ref.text.substr(0, _ref.startColumn);
		highlightColored() << _ref.text.substr(_ref.startColumn, locationLength);
		m_stream << _ref.text.substr(_ref.endColumn) << '\n';

		// line 3:
		m_stream << string(leftpad, ' ');
		frameColored() << " | ";
		for_each(
			_ref.text.cbegin(),
			_ref.text.cbegin() + _ref.startColumn,
			[this](char ch) { m_stream << (ch == '\t' ? '\t' : ' '); }
		);
		diagColored() << string(locationLength, '^') << '\n';
	}
	else
	{
		// line 1:
		m_stream << string(leftpad, ' ');
		frameColored() << " |" << '\n';

		// line 2:
		frameColored() << (_ref.position.line + 1) << " | ";
		m_stream << _ref.text.substr(0, _ref.startColumn);
		highlightColored() << _ref.text.substr(_ref.startColumn) << '\n';

		// line 3:
		frameColored() << string(leftpad, ' ') << " | ";
		m_stream << string(_ref.startColumn, ' ');
		diagColored() << "^ (Relevant source part starts here and spans across multiple lines).\n";
	}
}

void SourceReferenceFormatterHuman::printExceptionInformation(SourceReferenceExtractor::Message const& _msg)
{
	// exception header line
	errorColored() << _msg.category;
	messageColored() << ": " << _msg.primary.message << '\n';

	printSourceLocation(_msg.primary);

	for (auto const& secondary: _msg.secondary)
	{
		secondaryColored() << "Note";
		messageColored() << ": " << secondary.message << '\n';
		printSourceLocation(secondary);
	}

	m_stream << '\n';
}
