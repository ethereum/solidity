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
 * Helpers for common filesystem operations used in multiple tests.
 */

#pragma once

#include <boost/filesystem.hpp>

#include <set>
#include <string>

namespace solidity::test
{

/// Creates all the specified files and fills them with the specifiedcontent. Creates their parent
/// directories if they do not exist. Throws an exception if any part of the operation does not succeed.
void createFilesWithParentDirs(std::set<boost::filesystem::path> const& _paths, std::string const& _content = "");

/// Creates a file with the exact content specified in the second argument.
/// Throws an exception if the file already exists or if the parent directory of the file does not.
void createFileWithContent(boost::filesystem::path const& _path, std::string const& _content);

/// Creates a symlink between two paths.
/// The target does not have to exist.
/// If @p directorySymlink is true, indicate to the operating system that this is a directory
/// symlink. On some systems (e.g. Windows) it's possible to create a non-directory symlink pointing
/// at a directory, which makes such a symlinks unusable.
/// @returns true if the symlink has been successfully created, false if the filesystem does not
/// support symlinks.
/// Throws an exception of the operation fails for a different reason.
bool createSymlinkIfSupportedByFilesystem(
	boost::filesystem::path _targetPath,
	boost::filesystem::path const& _linkName,
	bool _directorySymlink
);

}
