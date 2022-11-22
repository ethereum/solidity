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
 * Utilities for creating temporary directories and temporarily changing the working directory
 * for use in tests.
 */

#pragma once

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

namespace solidity::util
{

/**
 * An object that creates a unique temporary directory and automatically deletes it and its
 * content upon being destroyed.
 *
 * The directory is guaranteed to be newly created and empty. Directory names are generated
 * randomly. If a directory with the same name already exists (very unlikely but possible) the
 * object won't reuse it and will fail with an exception instead.
 */
class TemporaryDirectory
{
public:
	TemporaryDirectory(std::string const& _prefix = "solidity");
	TemporaryDirectory(
		std::vector<boost::filesystem::path> const& _subdirectories,
		std::string const& _prefix = "solidity"
	);
	~TemporaryDirectory();

	boost::filesystem::path const& path() const { return m_path; }
	operator boost::filesystem::path() const { return m_path; }

private:
	boost::filesystem::path m_path;
};

/**
 * An object that changes current working directory and restores it upon destruction.
 */
class TemporaryWorkingDirectory
{
public:
	TemporaryWorkingDirectory(boost::filesystem::path const& _newDirectory);
	~TemporaryWorkingDirectory();

	boost::filesystem::path const& originalWorkingDirectory() const { return m_originalWorkingDirectory; }
	operator boost::filesystem::path() const { return boost::filesystem::current_path(); }

private:
	boost::filesystem::path m_originalWorkingDirectory;
};

}
