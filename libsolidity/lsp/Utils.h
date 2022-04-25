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

#include <libsolidity/ast/ASTForward.h>

#include <libsolutil/JSON.h>

#include <optional>
#include <vector>

#if !defined(NDEBUG)
#include <fstream>
#define lspDebug(message) (std::ofstream("/tmp/solc.log", std::ios::app) << (message) << std::endl)
#else
#define lspDebug(message) do {} while (0)
#endif

namespace solidity::langutil
{
class CharStreamProvider;
}

namespace solidity::lsp
{

class FileRepository;

std::optional<langutil::LineColumn> parseLineColumn(Json::Value const& _lineColumn);
Json::Value toJson(langutil::LineColumn const& _pos);
Json::Value toJsonRange(langutil::LineColumn const& _start, langutil::LineColumn const& _end);

/// @returns the source location given a source unit name and an LSP Range object,
/// or nullopt on failure.
std::optional<langutil::SourceLocation> parsePosition(
	FileRepository const& _fileRepository,
	std::string const& _sourceUnitName,
	Json::Value const& _position
);

/// @returns the source location given a source unit name and an LSP Range object,
/// or nullopt on failure.
std::optional<langutil::SourceLocation> parseRange(
	FileRepository const& _fileRepository,
	std::string const& _sourceUnitName,
	Json::Value const& _range
);

/// Strips the file:// URI prefix off the given path, if present,
/// also taking special care of Windows-drive-letter paths.
///
/// So file:///path/to/some/file.txt returns /path/to/some/file.txt, as well as,
/// file:///C:/file.txt will return C:/file.txt (forward-slash is okay on Windows).
std::string stripFileUriSchemePrefix(std::string const& _path);

/// Extracts the resolved declaration of the given expression AST node.
///
/// This may for example be the type declaration of an identifier,
/// or the type declaration of a structured member identifier.
///
/// @returns the resolved type declaration if found, or nullptr otherwise.
frontend::Declaration const* referencedDeclaration(frontend::Expression const* _expression);

/// @returns the location of the declaration's name, if present, or the location of the complete
/// declaration otherwise. If the input declaration is nullptr, std::nullopt is returned instead.
std::optional<langutil::SourceLocation> declarationLocation(frontend::Declaration const* _declaration);

}
