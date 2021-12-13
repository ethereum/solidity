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

#include <libsolidity/interface/ImportRemapper.h>
#include <libsolidity/interface/ReadFile.h>

#include <boost/filesystem.hpp>

#include <map>
#include <set>

namespace solidity::frontend
{

/// FileReader - used for progressively loading source code.
///
/// It is used in solc to load files from CLI parameters, stdin, or from JSON and
/// also used in the solc language server where solc is a long running process.
class FileReader
{
public:
	using StringMap = std::map<SourceUnitName, SourceCode>;
	using PathMap = std::map<SourceUnitName, boost::filesystem::path>;
	using FileSystemPathSet = std::set<boost::filesystem::path>;

	enum SymlinkResolution {
		Disabled, ///< Do not resolve symbolic links in the path.
		Enabled,  ///< Follow symbolic links. The path should contain no symlinks.
	};

	/// Constructs a FileReader with a base path and sets of include paths and allowed directories
	/// that will be used when requesting files from this file reader instance.
	explicit FileReader(
		boost::filesystem::path _basePath = {},
		std::vector<boost::filesystem::path> const& _includePaths = {},
		FileSystemPathSet _allowedDirectories = {}
	);

	void setBasePath(boost::filesystem::path const& _path);
	boost::filesystem::path const& basePath() const noexcept { return m_basePath; }

	void addIncludePath(boost::filesystem::path const& _path);
	std::vector<boost::filesystem::path> const& includePaths() const noexcept { return m_includePaths; }

	void allowDirectory(boost::filesystem::path _path);
	FileSystemPathSet const& allowedDirectories() const noexcept { return m_allowedDirectories; }

	/// @returns all sources by their internal source unit names.
	StringMap const& sourceUnits() const noexcept { return m_sourceCodes; }

	/// Resets all sources to the given map of source unit name to source codes.
	/// Does not enforce @a allowedDirectories().
	void setSourceUnits(StringMap _sources);

	/// Adds the source code under a source unit name created by normalizing the file path
	/// or changes an existing source.
	/// Does not enforce @a allowedDirectories().
	void addOrUpdateFile(boost::filesystem::path const& _path, SourceCode _source);

	/// Adds the source code under the source unit name of @a <stdin>.
	/// Does not enforce @a allowedDirectories().
	void setStdin(SourceCode _source);

	/// Receives a @p _sourceUnitName that refers to a source unit in compiler's virtual filesystem
	/// and attempts to interpret it as a path and read the corresponding file from disk.
	/// The read will only succeed if the canonical path of the file is within one of the @a allowedDirectories().
	/// @param _kind must be equal to "source". Other values are not supported.
	/// @return Content of the loaded file or an error message. If the operation succeeds, a copy of
	/// the content is retained in @a sourceUnits() under the key of @a _sourceUnitName. If the key
	/// already exists, previous content is discarded.
	frontend::ReadCallback::Result readFile(std::string const& _kind, std::string const& _sourceUnitName);

	frontend::ReadCallback::Callback reader()
	{
		return [this](std::string const& _kind, std::string const& _path) { return readFile(_kind, _path); };
	}

	/// Creates a source unit name by normalizing a path given on the command line and, if possible,
	/// making it relative to base path or one of the include directories.
	std::string cliPathToSourceUnitName(boost::filesystem::path const& _cliPath) const;

	/// Checks if a set contains any paths that lead to different files but would receive identical
	/// source unit names. Files are considered the same if their paths are exactly the same after
	/// normalization (without following symlinks).
	/// @returns a map containing all the conflicting source unit names and the paths that would
	/// receive them. The returned paths are normalized.
	std::map<std::string, FileSystemPathSet> detectSourceUnitNameCollisions(FileSystemPathSet const& _cliPaths) const;

	/// Normalizes a filesystem path to make it include all components up to the filesystem root,
	/// remove small, inconsequential differences that do not affect the meaning and make it look
	/// the same on all platforms (if possible).
	/// The resulting path uses forward slashes as path separators, has no redundant separators,
	/// has no redundant . or .. segments and has no root name if removing it does not change the meaning.
	/// The path does not have to actually exist.
	/// @param _path Path to normalize.
	/// @param _symlinkResolution If @a Disabled, any symlinks present in @a _path are preserved.
	static boost::filesystem::path normalizeCLIPathForVFS(
		boost::filesystem::path const& _path,
		SymlinkResolution _symlinkResolution = SymlinkResolution::Disabled
	);

	/// @returns true if all the path components of @a _prefix are present at the beginning of @a _path.
	/// Both paths must be absolute (or have slash as root) and normalized (no . or .. segments, no
	/// multiple consecutive slashes).
	/// Paths are treated as case-sensitive. Does not require the path to actually exist in the
	/// filesystem and does not follow symlinks. Only considers whole segments, e.g. /abc/d is not
	/// considered a prefix of /abc/def. Both paths must be non-empty.
	/// Ignores the trailing slash, i.e. /a/b/c.sol/ is treated as a valid prefix of /a/b/c.sol.
	static bool isPathPrefix(boost::filesystem::path const& _prefix, boost::filesystem::path const& _path);

	/// If @a _prefix is actually a prefix of @p _path, removes it from @a _path to make it relative.
	/// @returns The path without the prefix or unchanged path if there is not prefix.
	/// If @a _path and @_prefix are identical, the result is '.'.
	static boost::filesystem::path stripPrefixIfPresent(boost::filesystem::path const& _prefix, boost::filesystem::path const& _path);

	/// @returns true if the specified path is an UNC path.
	/// UNC paths start with // followed by a name (on Windows they can also start with \\).
	/// They are used for network shares on Windows. On UNIX systems they do not have the same
	/// functionality but usually they are still recognized and treated in a special way.
	static bool isUNCPath(boost::filesystem::path const& _path);

private:
	/// If @a _path starts with a number of .. segments, returns a path consisting only of those
	/// segments (root name is not included). Otherwise returns an empty path. @a _path must be
	/// absolute (or have slash as root).
	static boost::filesystem::path absoluteDotDotPrefix(boost::filesystem::path const& _path);

	/// @returns true if the path contains any .. segments.
	static bool hasDotDotSegments(boost::filesystem::path const& _path);

	/// Base path, used for resolving relative paths in imports.
	boost::filesystem::path m_basePath;

	/// Additional directories used for resolving relative paths in imports.
	std::vector<boost::filesystem::path> m_includePaths;

	/// list of allowed directories to read files from
	FileSystemPathSet m_allowedDirectories;

	/// map of input files to source code strings
	StringMap m_sourceCodes;
};

}
