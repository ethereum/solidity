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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Formatting functions for errors referencing positions and locations in the source.
 */

#include <liblangutil/SourceReferenceFormatter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/Exceptions.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;

void SourceReferenceFormatter::printSourceLocation(SourceLocation const* _location)
{
	printSourceLocation(SourceReferenceExtractor::extract(_location));
}

void SourceReferenceFormatter::printSourceLocation(SourceReference const& _ref)
{
	if (_ref.position.line < 0)
		return; // Nothing we can print here

	if (!_ref.multiline)
	{
		m_stream << _ref.text << endl;

		// mark the text-range like this: ^-----^
		for_each(
			_ref.text.cbegin(),
			_ref.text.cbegin() + _ref.startColumn,
			[this](char ch) { m_stream << (ch == '\t' ? '\t' : ' '); }
		);
		m_stream << "^";
		if (_ref.endColumn > _ref.startColumn + 2)
			m_stream << string(static_cast<size_t>(_ref.endColumn - _ref.startColumn - 2), '-');
		if (_ref.endColumn > _ref.startColumn + 1)
			m_stream << "^";
		m_stream << endl;
	}
	else
		m_stream <<
			_ref.text <<
			endl <<
			string(static_cast<size_t>(_ref.startColumn), ' ') <<
			"^ (Relevant source part starts here and spans across multiple lines)." <<
			endl;
}

void SourceReferenceFormatter::printSourceName(SourceReference const& _ref)
{
	if (_ref.position.line != -1)
		m_stream << _ref.sourceName << ":" << (_ref.position.line + 1) << ":" << (_ref.position.column + 1) << ": ";
	else if (!_ref.sourceName.empty())
		m_stream << _ref.sourceName << ": ";
}

void SourceReferenceFormatter::printExceptionInformation(util::Exception const& _exception, std::string const& _category)
{
	printExceptionInformation(SourceReferenceExtractor::extract(_exception, _category));
}

void SourceReferenceFormatter::printErrorInformation(Error const& _error)
{
	printExceptionInformation(SourceReferenceExtractor::extract(_error));
}

void SourceReferenceFormatter::printExceptionInformation(SourceReferenceExtractor::Message const& _msg)
{
	printSourceName(_msg.primary);

	m_stream << _msg.category << ": " << _msg.primary.message << endl;

	printSourceLocation(_msg.primary);

	for (auto const& ref: _msg.secondary)
	{
		printSourceName(ref);
		m_stream << ref.message << endl;
		printSourceLocation(ref);
	}
}
