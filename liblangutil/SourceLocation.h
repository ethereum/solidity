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

#include <libsolutil/Assertions.h>
#include <libsolutil/Exceptions.h>

#include <liblangutil/CharStream.h>

#include <memory>
#include <string>

namespace solidity::langutil
{
struct SourceLocationError: virtual util::Exception {};

/**
 * Representation of an interval of source positions.
 * The interval includes start and excludes end.
 */
struct SourceLocation
{
	bool operator==(SourceLocation const& _other) const
	{
		return source.get() == _other.source.get() && start == _other.start && end == _other.end;
	}
	bool operator!=(SourceLocation const& _other) const { return !operator==(_other); }

	inline bool operator<(SourceLocation const& _other) const
	{
		if (!source|| !_other.source)
			return std::make_tuple(int(!!source), start, end) < std::make_tuple(int(!!_other.source), _other.start, _other.end);
		else
			return std::make_tuple(source->name(), start, end) < std::make_tuple(_other.source->name(), _other.start, _other.end);
	}

	inline bool contains(SourceLocation const& _other) const
	{
		if (!hasText() || !_other.hasText() || source.get() != _other.source.get())
			return false;
		return start <= _other.start && _other.end <= end;
	}

	inline bool intersects(SourceLocation const& _other) const
	{
		if (!hasText() || !_other.hasText() || source.get() != _other.source.get())
			return false;
		return _other.start < end && start < _other.end;
	}

	bool isValid() const { return source || start != -1 || end != -1; }

	bool hasText() const
	{
		return
			source &&
			0 <= start &&
			start <= end &&
			end <= int(source->source().length());
	}

	std::string text() const
	{
		assertThrow(source, SourceLocationError, "Requested text from null source.");
		assertThrow(0 <= start, SourceLocationError, "Invalid source location.");
		assertThrow(start <= end, SourceLocationError, "Invalid source location.");
		assertThrow(end <= int(source->source().length()), SourceLocationError, "Invalid source location.");
		return source->source().substr(start, end - start);
	}

	/// @returns the smallest SourceLocation that contains both @param _a and @param _b.
	/// Assumes that @param _a and @param _b refer to the same source (exception: if the source of either one
	/// is unset, the source of the other will be used for the result, even if that is unset as well).
	/// Invalid start and end positions (with value of -1) are ignored (if start or end are -1 for both @param _a and
	/// @param _b, then start resp. end of the result will be -1 as well).
	static SourceLocation smallestCovering(SourceLocation _a, SourceLocation const& _b)
	{
		if (!_a.source)
			_a.source = _b.source;

		if (_a.start < 0)
			_a.start = _b.start;
		else if (_b.start >= 0 && _b.start < _a.start)
			_a.start = _b.start;
		if (_b.end > _a.end)
			_a.end = _b.end;

		return _a;
	}

	int start = -1;
	int end = -1;
	std::shared_ptr<CharStream> source;
};

SourceLocation const parseSourceLocation(std::string const& _input, std::string const& _sourceName, size_t _maxIndex = -1);

/// Stream output for Location (used e.g. in boost exceptions).
inline std::ostream& operator<<(std::ostream& _out, SourceLocation const& _location)
{
	if (!_location.isValid())
		return _out << "NO_LOCATION_SPECIFIED";

	if (_location.source)
		_out << _location.source->name();

	_out << "[" << _location.start << "," << _location.end << "]";

	return _out;
}

}
