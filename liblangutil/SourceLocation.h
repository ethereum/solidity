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
 * @author Lefteris Karapetsas <lefteris@ethdev.com>
 * @date 2015
 * Represents a location in a source file
 */

#pragma once

#include <libdevcore/Common.h> // defines noexcept macro for MSVC
#include <liblangutil/CharStream.h>
#include <memory>
#include <string>
#include <ostream>
#include <tuple>

namespace langutil
{

/**
 * Representation of an interval of source positions.
 * The interval includes start and excludes end.
 */
struct SourceLocation
{
	SourceLocation(): start(-1), end(-1), source{nullptr} { }
	SourceLocation(int _start, int _end, std::shared_ptr<CharStream> _source):
		start(_start), end(_end), source{std::move(_source)} { }

	bool operator==(SourceLocation const& _other) const
	{
		return start == _other.start && end == _other.end &&
			((!source.get() && !_other.source.get()) ||
			  (source.get() && _other.source.get() && source->name() == _other.source->name()));
	}
	bool operator!=(SourceLocation const& _other) const { return !operator==(_other); }
	inline bool operator<(SourceLocation const& _other) const;
	inline bool contains(SourceLocation const& _other) const;
	inline bool intersects(SourceLocation const& _other) const;

	bool isEmpty() const { return start == -1 && end == -1; }

	int start;
	int end;
	std::shared_ptr<CharStream> source;
};

/// Stream output for Location (used e.g. in boost exceptions).
inline std::ostream& operator<<(std::ostream& _out, SourceLocation const& _location)
{
	if (_location.isEmpty())
		return _out << "NO_LOCATION_SPECIFIED";

	if (_location.source)
		_out << _location.source->name();

	_out << "[" << _location.start << "," << _location.end << ")";

	return _out;
}

bool SourceLocation::operator<(SourceLocation const& _other) const
{
	if (!source|| !_other.source)
		return std::make_tuple(int(!!source), start, end) < std::make_tuple(int(!!_other.source), _other.start, _other.end);
	else
		return std::make_tuple(source->name(), start, end) < std::make_tuple(_other.source->name(), _other.start, _other.end);
}

bool SourceLocation::contains(SourceLocation const& _other) const
{
	if (isEmpty() || _other.isEmpty() || ((!source || !_other.source || source->name() != _other.source->name()) && (source || _other.source)))
		return false;
	return start <= _other.start && _other.end <= end;
}

bool SourceLocation::intersects(SourceLocation const& _other) const
{
	if (isEmpty() || _other.isEmpty() || ((!source || !_other.source || source->name() != _other.source->name()) && (source || _other.source)))
		return false;
	return _other.start < end && start < _other.end;
}

}
