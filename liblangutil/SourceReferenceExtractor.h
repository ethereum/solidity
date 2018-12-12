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
#pragma once

#include <iosfwd>
#include <string>
#include <tuple>
#include <vector>

namespace dev
{
struct Exception;
}

namespace langutil
{

struct LineColumn
{
	int line = {-1};
	int column = {-1};

	LineColumn() = default;
	LineColumn(std::tuple<int, int> const& _t): line{std::get<0>(_t)}, column{std::get<1>(_t)} {}
};

struct SourceReference
{
	std::string message;      ///< A message that relates to this source reference (such as a warning or an error message).
	std::string sourceName;   ///< Underlying source name (for example the filename).
	LineColumn position;      ///< Actual (error) position this source reference is surrounding.
	bool multiline = {false}; ///< Indicates whether the actual SourceReference is truncated to one line.
	std::string text;         ///< Extracted source code text (potentially truncated if multiline or too long).
	int startColumn = {-1};   ///< Highlighting range-start of text field.
	int endColumn = {-1};     ///< Highlighting range-end of text field.

	/// Constructs a SourceReference containing a message only.
	static SourceReference MessageOnly(std::string _msg)
	{
		SourceReference sref;
		sref.message = std::move(_msg);
		return sref;
	}
};

struct SourceLocation;

namespace SourceReferenceExtractor
{
	struct Message
	{
		SourceReference primary;
		std::string category; // "Error", "Warning", ...
		std::vector<SourceReference> secondary;
	};

	Message extract(dev::Exception const& _exception, std::string _category);
	SourceReference extract(SourceLocation const* _location, std::string message = "");
}

}
