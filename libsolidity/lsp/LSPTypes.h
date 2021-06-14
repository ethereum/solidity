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
#pragma once

#include <liblangutil/SourceLocation.h>
#include <liblangutil/SourceReferenceExtractor.h> // LineColumn

#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace solidity::lsp
{

struct LineColumnRange
{
	langutil::LineColumn start;
	langutil::LineColumn end;
};

enum class Trace { Off, Messages, Verbose };

struct DocumentPosition
{
	std::string path;
	langutil::LineColumn position;
};

enum class DocumentHighlightKind
{
	Unspecified,
	Text,           //!< a textual occurrence
	Read,           //!< read access to a variable
	Write,          //!< write access to a variable
};

// Represents a symbol / AST node that is to be highlighted, with some context associated.
struct DocumentHighlight
{
	langutil::SourceLocation location;
	// std::string sourceName;
	// LineColumnRange range;
	DocumentHighlightKind kind = DocumentHighlightKind::Unspecified;
};

/// Represents a related message and source code location for a diagnostic. This should be
/// used to point to code locations that cause or related to a diagnostics, e.g when duplicating
/// a symbol in a scope.
struct DiagnosticRelatedInformation
{
	langutil::SourceLocation location;   // The location of this related diagnostic information.
	std::string message; // The message of this related diagnostic information.
};

}
