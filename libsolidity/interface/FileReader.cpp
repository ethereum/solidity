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
#include <libsolidity/interface/FileReader.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/Exceptions.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string/predicate.hpp>
#include <range/v3/view/transform.hpp>

#include <range/v3/range/conversion.hpp>

#include <functional>

using solidity::frontend::ReadCallback;
using solidity::langutil::InternalCompilerError;
using solidity::util::errinfo_comment;
using solidity::util::readFileAsString;
using solidity::util::joinHumanReadable;

namespace solidity::frontend
{

FileReader::FileReader(
	boost::filesystem::path _basePath,
	std::vector<boost::filesystem::path> const& _includePaths,
	FileSystemPathSet _allowedDirectories
):
	m_allowedDirectories(std::move(_allowedDirectories)),
	m_sourceCodes()
{
	setBasePath(_basePath);
	for (boost::filesystem::path const& includePath: _includePaths)
		addIncludePath(includePath);

	for (boost::filesystem::path const& allowedDir: m_allowedDirectories)
		solAssert(!allowedDir.empty(), "");
}

void FileReader::setBasePath(boost::filesystem::path const& _path)
{
	if (_path.empty())
	{
		// Empty base path is a special case that does not make sense when include paths are used.
		solAssert(m_includePaths.empty(), "");
		m_basePath = "";
	}
	else
		m_basePath = normalizeCLIPathForVFS(_path);
}

void FileReader::addIncludePath(boost::filesystem::path const& _path)
{
	solAssert(!m_basePath.empty(), "");
	solAssert(!_path.empty(), "");
	m_includePaths.push_back(normalizeCLIPathForVFS(_path));
}

void FileReader::allowDirectory(boost::filesystem::path _path)
{
	solAssert(!_path.empty(), "");
	m_allowedDirectories.insert(std::move(_path));
}

void FileReader::addOrUpdateFile(boost::filesystem::path const& _path, SourceCode _source)
{
	m_sourceCodes[cliPathToSourceUnitName(_path)] = std::move(_source);
}

void FileReader::setStdin(SourceCode _source)
{
	m_sourceCodes["<stdin>"] = std::move(_source);
}

void FileReader::setSourceUnits(StringMap _sources)
{
	m_sourceCodes = std::move(_sources);
}

ReadCallback::Result FileReader::readFile(std::string const& _kind, std::string const& _sourceUnitName)
{
	try
	{
		if (_kind != ReadCallback::kindString(ReadCallback::Kind::ReadFile))
			solAssert(false, "ReadFile callback used as callback kind " + _kind);
		std::string strippedSourceUnitName = _sourceUnitName;
		if (strippedSourceUnitName.find("file://") == 0)
			strippedSourceUnitName.erase(0, 7);

		std::vector<boost::filesystem::path> candidates;
		std::vector<std::reference_wrapper<boost::filesystem::path>> prefixes = {m_basePath};
		prefixes += (m_includePaths | ranges::to<std::vector<std::reference_wrapper<boost::filesystem::path>>>);

		for (auto const& prefix: prefixes)
		{
			boost::filesystem::path canonicalPath = normalizeCLIPathForVFS(prefix / strippedSourceUnitName, SymlinkResolution::Enabled);
			if (boost::filesystem::exists(canonicalPath))
				candidates.push_back(std::move(canonicalPath));
		}

		auto pathToQuotedString = [](boost::filesystem::path const& _path){ return "\"" + _path.string() + "\""; };

		if (candidates.empty())
			return ReadCallback::Result{
				false,
				"File not found. Searched the following locations: " +
				joinHumanReadable(prefixes | ranges::views::transform(pathToQuotedString), ", ") +
				"."
			};

		if (candidates.size() >= 2)
			return ReadCallback::Result{
				false,
				"Ambiguous import. "
				"Multiple matching files found inside base path and/or include paths: " +
				joinHumanReadable(candidates | ranges::views::transform(pathToQuotedString), ", ") +
				"."
			};

		FileSystemPathSet allowedPaths =
			m_allowedDirectories +
			decltype(allowedPaths){m_basePath.empty() ? "." : m_basePath} +
			m_includePaths;

		bool isAllowed = false;
		for (boost::filesystem::path const& allowedDir: allowedPaths)
			if (isPathPrefix(normalizeCLIPathForVFS(allowedDir, SymlinkResolution::Enabled), candidates[0]))
			{
				isAllowed = true;
				break;
			}

		if (!isAllowed)
			return ReadCallback::Result{
				false,
				"File outside of allowed directories. The following are allowed: " +
				joinHumanReadable(allowedPaths | ranges::views::transform(pathToQuotedString), ", ") +
				"."
			};

		if (!boost::filesystem::is_regular_file(candidates[0]))
			return ReadCallback::Result{false, "Not a valid file."};

		// NOTE: we ignore the FileNotFound exception as we manually check above
		auto contents = readFileAsString(candidates[0]);
		solAssert(m_sourceCodes.count(_sourceUnitName) == 0, "");
		m_sourceCodes[_sourceUnitName] = contents;
		return ReadCallback::Result{true, contents};
	}
	catch (util::Exception const& _exception)
	{
		return ReadCallback::Result{false, "Exception in read callback: " + boost::diagnostic_information(_exception)};
	}
	catch (std::exception const& _exception)
	{
		return ReadCallback::Result{false, "Exception in read callback: " + boost::diagnostic_information(_exception)};
	}
	catch (...)
	{
		return ReadCallback::Result{false, "Unknown exception in read callback: " + boost::current_exception_diagnostic_information()};
	}
}

std::string FileReader::cliPathToSourceUnitName(boost::filesystem::path const& _cliPath) const
{
	std::vector<boost::filesystem::path> prefixes = {m_basePath.empty() ? normalizeCLIPathForVFS(".") : m_basePath};
	prefixes += m_includePaths;

	boost::filesystem::path normalizedPath = normalizeCLIPathForVFS(_cliPath);
	for (boost::filesystem::path const& prefix: prefixes)
		if (isPathPrefix(prefix, normalizedPath))
		{
			// Multiple prefixes can potentially match the path. We take the first one.
			normalizedPath = stripPrefixIfPresent(prefix, normalizedPath);
			break;
		}

	return normalizedPath.generic_string();
}

std::map<std::string, FileReader::FileSystemPathSet> FileReader::detectSourceUnitNameCollisions(FileSystemPathSet const& _cliPaths) const
{
	std::map<std::string, FileReader::FileSystemPathSet> nameToPaths;
	for (boost::filesystem::path const& cliPath: _cliPaths)
	{
		std::string sourceUnitName = cliPathToSourceUnitName(cliPath);
		boost::filesystem::path normalizedPath = normalizeCLIPathForVFS(cliPath);
		nameToPaths[sourceUnitName].insert(normalizedPath);
	}

	std::map<std::string, FileReader::FileSystemPathSet> collisions;
	for (auto&& [sourceUnitName, cliPaths]: nameToPaths)
		if (cliPaths.size() >= 2)
			collisions[sourceUnitName] = std::move(cliPaths);

	return collisions;
}

boost::filesystem::path FileReader::normalizeCLIPathForVFS(
	boost::filesystem::path const& _path,
	SymlinkResolution _symlinkResolution
)
{
	// Detailed normalization rules:
	// - Makes the path either be absolute or have slash as root (note that on Windows paths with
	//   slash as root are not considered absolute by Boost). If it is empty, it becomes
	//   the current working directory.
	// - Collapses redundant . and .. segments.
	// - Removes leading .. segments from an absolute path (i.e. /../../ becomes just /).
	// - Squashes sequences of multiple path separators into one.
	// - Ensures that forward slashes are used as path separators on all platforms.
	// - Removes the root name (e.g. drive letter on Windows) when it matches the root name in the
	//   path to the current working directory.
	//
	// Also note that this function:
	// - Does NOT resolve symlinks (except for symlinks in the path to the current working directory)
	//   unless explicitly requested.
	// - Does NOT check if the path refers to a file or a directory. If the path ends with a slash,
	//   the slash is preserved even if it's a file.
	//   - The only exception are paths where the file name is a dot (e.g. '.' or 'a/b/.'). These
	//     always have a trailing slash after normalization.
	// - Preserves case. Even if the filesystem is case-insensitive but case-preserving and the
	//   case differs, the actual case from disk is NOT detected.

	boost::filesystem::path canonicalWorkDir = boost::filesystem::weakly_canonical(boost::filesystem::current_path());

	// NOTE: On UNIX systems the path returned from current_path() has symlinks resolved while on
	// Windows it does not. To get consistent results we resolve them on all platforms.
	boost::filesystem::path absolutePath = boost::filesystem::absolute(_path, canonicalWorkDir);

	boost::filesystem::path normalizedPath;
	if (_symlinkResolution == SymlinkResolution::Enabled)
	{
		// NOTE: weakly_canonical() will not convert a relative path into an absolute one if no
		// directory included in the path actually exists.
		normalizedPath = boost::filesystem::weakly_canonical(absolutePath);

		// The three corner cases in which lexically_normal() includes a trailing slash in the
		// normalized path but weakly_canonical() does not. Note that the trailing slash is not
		// ignored when comparing paths with ==.
		if ((_path == "." || _path == "./" || _path == "../") && !boost::ends_with(normalizedPath.generic_string(), "/"))
			normalizedPath = normalizedPath.parent_path() / (normalizedPath.filename().string() + "/");
	}
	else
	{
		solAssert(_symlinkResolution == SymlinkResolution::Disabled, "");

		// NOTE: boost path preserves certain differences that are ignored by its operator ==.
		// E.g. "a//b" vs "a/b" or "a/b/" vs "a/b/.". lexically_normal() does remove these differences.
		normalizedPath = absolutePath.lexically_normal();
	}
	solAssert(normalizedPath.is_absolute() || normalizedPath.root_path() == "/", "");

	// If the path is on the same drive as the working dir, for portability we prefer not to
	// include the root name. Do this only for non-UNC paths - my experiments show that on Windows
	// when the working dir is an UNC path, / does not not actually refer to the root of the UNC path.

	boost::filesystem::path normalizedRootPath = normalizeCLIRootPathForVFS(normalizedPath, canonicalWorkDir);

	// lexically_normal() will not squash paths like "/../../" into "/". We have to do it manually.
	boost::filesystem::path dotDotPrefix = absoluteDotDotPrefix(normalizedPath);

	boost::filesystem::path normalizedPathNoDotDot = normalizedPath;
	if (dotDotPrefix.empty())
		normalizedPathNoDotDot = normalizedRootPath / normalizedPath.relative_path();
	else
		normalizedPathNoDotDot = normalizedRootPath / normalizedPath.lexically_relative(normalizedPath.root_path() / dotDotPrefix);
	solAssert(!hasDotDotSegments(normalizedPathNoDotDot), "");

	// NOTE: On Windows lexically_normal() converts all separators to forward slashes. Convert them back.
	// Separators do not affect path comparison but remain in internal representation returned by native().
	// This will also normalize the root name to start with // in UNC paths.
	normalizedPathNoDotDot = normalizedPathNoDotDot.generic_string();

	// For some reason boost considers "/." different than "/" even though for other directories
	// the trailing dot is ignored.
	if (normalizedPathNoDotDot == "/.")
		return "/";

	return normalizedPathNoDotDot;
}

boost::filesystem::path FileReader::normalizeCLIRootPathForVFS(
	boost::filesystem::path const& _path,
	boost::filesystem::path const& _workDir
)
{
	solAssert(_workDir.is_absolute(), "");

	boost::filesystem::path absolutePath = boost::filesystem::absolute(_path, _workDir);
	boost::filesystem::path rootPath = absolutePath.root_path();
	boost::filesystem::path baseRootPath = _workDir.root_path();

	if (isUNCPath(absolutePath))
		return rootPath;

	// Ignore drive letter case on Windows (C:\ <=> c:\).
	if (boost::filesystem::equivalent(rootPath, baseRootPath))
		return "/";

	return rootPath;
}

bool FileReader::isPathPrefix(boost::filesystem::path const& _prefix, boost::filesystem::path const& _path)
{
	solAssert(!_prefix.empty() && !_path.empty(), "");
	// NOTE: On Windows paths starting with a slash (rather than a drive letter) are considered relative by boost.
	solAssert(_prefix.is_absolute() || isUNCPath(_prefix) || _prefix.root_path() == "/", "");
	solAssert(_path.is_absolute() || isUNCPath(_path) || _path.root_path() == "/", "");
	// NOTE: On Windows before Boost 1.78 lexically_normal() would not replace the `//` UNC prefix with `\\\\`.
	// Later versions do. Use generic_path() to normalize all slashes to `/` and ignore that difference.
	// This does not make the assert weaker because == ignores slash type anyway.
	solAssert(_prefix == _prefix.lexically_normal().generic_string() && _path == _path.lexically_normal().generic_string(), "");
	solAssert(!hasDotDotSegments(_prefix) && !hasDotDotSegments(_path), "");

	boost::filesystem::path strippedPath = _path.lexically_relative(
		// Before 1.72.0 lexically_relative() was not handling paths with empty, dot and dot dot segments
		// correctly (see https://github.com/boostorg/filesystem/issues/76). The only case where this
		// is possible after our normalization is a directory name ending in a slash (filename is a dot).
		_prefix.filename_is_dot() ? _prefix.parent_path() : _prefix
	);
	return !strippedPath.empty() && *strippedPath.begin() != "..";
}

boost::filesystem::path FileReader::stripPrefixIfPresent(boost::filesystem::path const& _prefix, boost::filesystem::path const& _path)
{
	if (!isPathPrefix(_prefix, _path))
		return _path;

	boost::filesystem::path strippedPath = _path.lexically_relative(
		_prefix.filename_is_dot() ? _prefix.parent_path() : _prefix
	);
	solAssert(strippedPath.empty() || *strippedPath.begin() != "..", "");
	return strippedPath;
}

boost::filesystem::path FileReader::absoluteDotDotPrefix(boost::filesystem::path const& _path)
{
	solAssert(_path.is_absolute() || _path.root_path() == "/", "");

	boost::filesystem::path _pathWithoutRoot = _path.relative_path();
	boost::filesystem::path prefix;
	for (boost::filesystem::path const& segment: _pathWithoutRoot)
		if (segment.filename_is_dot_dot())
			prefix /= segment;

	return prefix;
}

bool FileReader::hasDotDotSegments(boost::filesystem::path const& _path)
{
	for (boost::filesystem::path const& segment: _path)
		if (segment.filename_is_dot_dot())
			return true;

	return false;
}

bool FileReader::isUNCPath(boost::filesystem::path const& _path)
{
	std::string rootName = _path.root_name().string();

	return (
		rootName.size() == 2 ||
		(rootName.size() > 2 && rootName[2] != rootName[1])
	) && (
		(rootName[0] == '/' && rootName[1] == '/')
#if defined(_WIN32)
		|| (rootName[0] == '\\' && rootName[1] == '\\')
#endif
	);
}

}
