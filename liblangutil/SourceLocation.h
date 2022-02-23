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
 * @author Lefteris Karapetsas <lefteris@ethdev.com>
 * @date 2015
 * Represents a location in a source file
 */

#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace solidity::langutil
{

/**
 * Representation of an interval of source positions.
 * The interval includes start and excludes end.
 */
struct SourceLocation
{
	bool operator==(SourceLocation const& _other) const
	{
		return start == _other.start && end == _other.end && equalSources(_other);
	}
	bool operator!=(SourceLocation const& _other) const { return !operator==(_other); }

	bool operator<(SourceLocation const& _other) const
	{
		if (!sourceName || !_other.sourceName)
			return std::make_tuple(int(!!sourceName), start, end) < std::make_tuple(int(!!_other.sourceName), _other.start, _other.end);
		else
			return std::make_tuple(*sourceName, start, end) < std::make_tuple(*_other.sourceName, _other.start, _other.end);
	}

	bool contains(SourceLocation const& _other) const
	{
		if (!hasText() || !_other.hasText() || !equalSources(_other))
			return false;
		return start <= _other.start && _other.end <= end;
	}

	bool containsOffset(int _pos) const
	{
		if (!hasText() || _pos < 0)
			return false;
		return start <= _pos && _pos < end;
	}

	bool intersects(SourceLocation const& _other) const
	{
		if (!hasText() || !_other.hasText() || !equalSources(_other))
			return false;
		return _other.start < end && start < _other.end;
	}

	bool equalSources(SourceLocation const& _other) const
	{
		if (!!sourceName != !!_other.sourceName)
			return false;
		if (sourceName && *sourceName != *_other.sourceName)
			return false;
		return true;
	}

	bool isValid() const { return sourceName || start != -1 || end != -1; }

	bool hasText() const { return sourceName && 0 <= start && start <= end; }

	/// @returns the smallest SourceLocation that contains both @param _a and @param _b.
	/// Assumes that @param _a and @param _b refer to the same source (exception: if the source of either one
	/// is unset, the source of the other will be used for the result, even if that is unset as well).
	/// Invalid start and end positions (with value of -1) are ignored (if start or end are -1 for both @param _a and
	/// @param _b, then start resp. end of the result will be -1 as well).
	static SourceLocation smallestCovering(SourceLocation _a, SourceLocation const& _b)
	{
		if (!_a.sourceName)
			_a.sourceName = _b.sourceName;

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
	std::shared_ptr<std::string const> sourceName;
};

SourceLocation parseSourceLocation(
	std::string const& _input,
	std::vector<std::shared_ptr<std::string const>> const& _sourceNames
);

/// Stream output for Location (used e.g. in boost exceptions).
std::ostream& operator<<(std::ostream& _out, SourceLocation const& _location);


/**
 * Alternative, line-column-based representation for source locations.
 * Both line and column are zero-based.
 * If used as a range, the second location is considered exclusive.
 * Negative values are invalid.
 */
struct LineColumn
{
	/// Line value, can be between zero and number of `\n` characters in the source file.
	int line = -1;
	/// Column value, can be between zero and number of characters in the line (inclusive).
	int column = -1;

	LineColumn() = default;
	explicit LineColumn(int _line, int _column): line(_line), column(_column) {}
};


}
