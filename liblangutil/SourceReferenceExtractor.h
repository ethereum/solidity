// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <liblangutil/Exceptions.h>

#include <iosfwd>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace solidity::langutil
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
	static SourceReference MessageOnly(std::string _msg, std::string _sourceName = {})
	{
		SourceReference sref;
		sref.message = std::move(_msg);
		sref.sourceName = std::move(_sourceName);
		return sref;
	}
};

namespace SourceReferenceExtractor
{
	struct Message
	{
		SourceReference primary;
		std::string category; // "Error", "Warning", ...
		std::vector<SourceReference> secondary;
		std::optional<ErrorId> errorId;
	};

	Message extract(util::Exception const& _exception, std::string _category);
	Message extract(Error const& _error);
	SourceReference extract(SourceLocation const* _location, std::string message = "");
}

}
