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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Formatting functions for errors referencing positions and locations in the source.
 */

#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/interface/Exceptions.h>

using namespace std;

namespace dev
{
namespace solidity
{

void SourceReferenceFormatter::printSourceLocation(
	ostream& _stream,
	SourceLocation const* _location,
	function<Scanner const&(string const&)> const& _scannerFromSourceName
)
{
	if (!_location || !_location->sourceName)
		return; // Nothing we can print here
	auto const& scanner = _scannerFromSourceName(*_location->sourceName);
	int startLine;
	int startColumn;
	tie(startLine, startColumn) = scanner.translatePositionToLineColumn(_location->start);
	int endLine;
	int endColumn;
	tie(endLine, endColumn) = scanner.translatePositionToLineColumn(_location->end);
	if (startLine == endLine)
	{
		string line = scanner.lineAtPosition(_location->start);
		_stream << line << endl;
		for_each(
			line.cbegin(),
			line.cbegin() + startColumn,
			[&_stream](char const& ch) { _stream << (ch == '\t' ? '\t' : ' '); }
		);
		_stream << "^";
		if (endColumn > startColumn + 2)
			_stream << string(endColumn - startColumn - 2, '-');
		if (endColumn > startColumn + 1)
			_stream << "^";
		_stream << endl;
	}
	else
		_stream <<
			scanner.lineAtPosition(_location->start) <<
			endl <<
			string(startColumn, ' ') <<
			"^\n" <<
			"Spanning multiple lines.\n";
}

void SourceReferenceFormatter::printSourceName(
	ostream& _stream,
	SourceLocation const* _location,
	function<Scanner const&(string const&)> const& _scannerFromSourceName
)
{
	if (!_location || !_location->sourceName)
		return; // Nothing we can print here
	auto const& scanner = _scannerFromSourceName(*_location->sourceName);
	int startLine;
	int startColumn;
	tie(startLine, startColumn) = scanner.translatePositionToLineColumn(_location->start);
	_stream << *_location->sourceName << ":" << (startLine + 1) << ":" << (startColumn + 1) << ": ";
}

void SourceReferenceFormatter::printExceptionInformation(
	ostream& _stream,
	Exception const& _exception,
	string const& _name,
	function<Scanner const&(string const&)> const& _scannerFromSourceName
)
{
	SourceLocation const* location = boost::get_error_info<errinfo_sourceLocation>(_exception);
	auto secondarylocation = boost::get_error_info<errinfo_secondarySourceLocation>(_exception);

	printSourceName(_stream, location, _scannerFromSourceName);

	_stream << _name;
	if (string const* description = boost::get_error_info<errinfo_comment>(_exception))
		_stream << ": " << *description << endl;

	printSourceLocation(_stream, location, _scannerFromSourceName);

	if (secondarylocation && !secondarylocation->infos.empty())
	{
		for (auto info: secondarylocation->infos)
		{
			_stream << info.first << " ";
			printSourceName(_stream, &info.second, _scannerFromSourceName);
			_stream << endl;
			printSourceLocation(_stream, &info.second, _scannerFromSourceName);
		}
		_stream << endl;
	}
}

}
}
